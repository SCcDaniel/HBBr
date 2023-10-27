﻿#include "ShaderCompiler.h"
#include "FileSystem.h"
#if IS_EDITOR
#include <fstream>
#include "ConsoleDebug.h"
#include "glm/glm.hpp"
#include "ThirdParty/shaderc/shaderc.hpp"
#include "Component/Material.h"

#include <string>  
#include <iostream> 

#pragma comment(lib,"shaderc/shaderc_shared.lib")

void Shaderc::ShaderCompiler::CompileAllShaders(const char* srcShaderPath)
{
	auto allFxShaders = FileSystem::GetFilesBySuffix(srcShaderPath , "fx");
	//不打算单独编译顶点或者像素着色器
	//auto allvFxShaders = FileSystem::GetFilesBySuffix(srcShaderPath, "vfx");
	//auto allpFxShaders = FileSystem::GetFilesBySuffix(srcShaderPath, "pfx");
	auto allcFxShaders = FileSystem::GetFilesBySuffix(srcShaderPath, "cfx");
	//fx
	if (allFxShaders.size() > 0)
	{
		for (auto i : allFxShaders) 
		{
			CompileShader(i.absPath.c_str(), "VSMain", CompileShaderType::VertexShader);
			CompileShader(i.absPath.c_str(), "PSMain", CompileShaderType::PixelShader);
		}
	}
	////vfx
	//if (allvFxShaders.size() > 0)
	//{
	//	for (auto i : allvFxShaders)
	//	{
	//		CompileShader(i.absPath.c_str(), "VSMain", CompileShaderType::VertexShader);
	//	}
	//}
	////pfx
	//if (allpFxShaders.size() > 0)
	//{
	//	for (auto i : allpFxShaders)
	//	{
	//		CompileShader(i.absPath.c_str(), "PSMain", CompileShaderType::PixelShader);
	//	}
	//}
	//cfx
	if (allcFxShaders.size() > 0)
	{
		for (auto i : allcFxShaders)
		{
			CompileShader(i.absPath.c_str(), "CSMain", CompileShaderType::ComputeShader);
		}
	}
}

class ShaderIncluder : public  shaderc::CompileOptions::IncluderInterface 
{
public:
	std::vector<HString> samePath;
	std::vector<std::string>contents;
	std::vector<std::string>source_names;
	//这是个循环函数,content必须要存起来才能正常处理嵌套关系
	shaderc_include_result* GetInclude(const char* requested_source,
		shaderc_include_type type,
		const char* requesting_source,
		size_t include_depth) override {
		auto* result = new shaderc_include_result;
		// 读取文件内容。
		HString IncludePath = FileSystem::GetShaderIncludeAbsPath() + requested_source;
		IncludePath.CorrectionPath();
		std::ifstream file(IncludePath.c_str());
		if (file.good()) {
			contents.push_back(std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>()).c_str());
			source_names.push_back(requested_source);
			result->content = contents[contents.size() - 1].c_str();
			result->content_length = contents[contents.size() - 1].size();
			result->source_name = source_names[source_names.size() - 1].c_str();
			result->source_name_length = source_names[source_names.size() - 1].size();
			result->user_data = nullptr;
			if (std::find(samePath.begin(), samePath.end(), IncludePath) != samePath.end())
			{
				MessageOut(HString(HString(requesting_source) + (": Shader Include exist.")).c_str(), false, false, "255,255,0");
				result->content = " \n\n ";
				result->content_length = strlen(result->content);
			}
			else
			{
				samePath.push_back(IncludePath);
			}
		}
		//MessageOut(contents[contents.size() - 1].c_str(), false, false);
		return result;
	}

	void ReleaseInclude(shaderc_include_result* data) override {
		// 释放我们在 GetInclude 中分配的内存。
		delete data;
	}
};

void Shaderc::ShaderCompiler::CompileShader(const char* srcShaderFileFullPath, const char* entryPoint, CompileShaderType shaderType)
{
	{
		//导入shader源码
		std::ifstream shaderSrcFile(srcShaderFileFullPath);
		std::string shaderSrcCode((std::istreambuf_iterator<char>(shaderSrcFile)), std::istreambuf_iterator<char>());
		HString _shaderSrcCode = shaderSrcCode.c_str();
		//
		shaderc::Compiler compiler;
		shaderc::CompileOptions options;
		ShaderCacheHeader header = {};
		std::vector<ShaderParameterInfo> shaderParamInfos;
		std::vector<ShaderTextureInfo> shaderTextureInfos;
		//自定义的信息进去
		{
			auto line = _shaderSrcCode.Split("\n");
			bool bSearchVertexInputLayout = false;
			bool bCollectMaterialParameter = false;
			int CollectMaterialParameterIndex = 0;
			bool bCollectMaterialTexture = false;
			int CollectMaterialTextureIndex = 0;
			for (auto s : line)
			{
				HString setting = s.ClearSpace();
				auto settings = setting.Split("]");
				if (settings[0].IsSame("//[Flags"))
				{
					auto flagStr = settings[1].Split(";");
					for (auto i : flagStr)
					{
						if (i.IsSame("EnableShaderDebug"))
						{
							header.flags |= EnableShaderDebug;
						}
					}
				}
				else if (settings[0].IsSame("//[InputLayout"))
				{
					bSearchVertexInputLayout = true;
				}
				else if (settings[0].IsSame("//[MP"))
				{
					bCollectMaterialParameter = true;
					shaderParamInfos.push_back(ShaderParameterInfo({}));
					//提取每个属性
					auto paramProperty = settings[1].Split(";");
					for (auto i : paramProperty)
					{
						//提取属性里的值
						auto value = i.Split("=");
						if (value[0].Contains("Default"))
						{
							auto values = value[1].Split(",");
							auto maxCount = (((values.size()) < (3)) ? (values.size()) : (3));
							for (int vv = 0; vv < maxCount; vv++)
								shaderParamInfos[shaderParamInfos.size()-1].defaultValue[vv] = (float)HString::ToDouble(values[vv]);
						}
						else if (value[0].Contains("Name"))
						{
							std::string name = value[1].c_str();
							name.copy(shaderParamInfos[shaderParamInfos.size() - 1].name, sizeof(shaderParamInfos[shaderParamInfos.size() - 1].name) - 1);
							shaderParamInfos[shaderParamInfos.size() - 1].name[sizeof(shaderParamInfos[shaderParamInfos.size() - 1].name) - 1] = '\0';
						}
					}
				}
				else if (settings[0].IsSame("//[MT"))
				{
					shaderTextureInfos.push_back(ShaderTextureInfo({}));
					bCollectMaterialTexture = true;
					//提取每个属性
					auto paramProperty = settings[1].Split(";");
					for (auto i : paramProperty)
					{
						//提取属性里的值
						auto value = i.Split("=");
						if (value[0].Contains("Default"))
						{
							std::string defaultValue = value[1].c_str();
							defaultValue.copy(shaderTextureInfos[shaderTextureInfos.size() - 1].defaultTexture, sizeof(shaderTextureInfos[shaderTextureInfos.size() - 1].defaultTexture) - 1);
							shaderTextureInfos[shaderTextureInfos.size() - 1].defaultTexture[sizeof(shaderTextureInfos[shaderTextureInfos.size() - 1].defaultTexture) - 1] = '\0';
						}
						else if (value[0].Contains("Name"))
						{
							std::string name = value[1].c_str();
							name.copy(shaderTextureInfos[shaderTextureInfos.size() - 1].name, sizeof(shaderTextureInfos[shaderTextureInfos.size() - 1].name) - 1);
							shaderTextureInfos[shaderTextureInfos.size() - 1].name[sizeof(shaderTextureInfos[shaderTextureInfos.size() - 1].name) - 1] = '\0';
						}
					}
				}
				else if (bSearchVertexInputLayout)
				{
					//查找类型符号
					//[0]pos:POSITION , [1]nor:NORMAL , [2]tar:TANGENT , [3]col:COLOR , [4]uv01:TEXCOORD0 , [5]uv23:TEXCOORD1
					bool bFind = false;
					int size = 0;
					if (s.Contains("float2"))
					{
						bFind = true;
						size = 2;
					}
					else if (s.Contains("float3"))
					{
						bFind = true;
						size = 3;
					}
					else if (s.Contains("float4"))
					{
						bFind = true;
						size = 4;
					}
					if (bFind)
					{
						auto sem = s.Split(":");
						if (sem.size() > 1)
						{
							if (sem[1].Contains("POSITION"))
								header.vertexInput[0] = size;
							else if (sem[1].Contains("NORMAL"))
								header.vertexInput[1] = size;
							else if (sem[1].Contains("TANGENT"))
								header.vertexInput[2] = size;
							else if (sem[1].Contains("COLOR"))
								header.vertexInput[3] = size;
							else if (sem[1].Contains("TEXCOORD0"))
								header.vertexInput[4] = size;
							else if (sem[1].Contains("TEXCOORD1"))
								header.vertexInput[5] = size;
						}
					}
					//结构体结束
					if (s.Contains("};"))
					{
						bSearchVertexInputLayout = false;
					}
				}
				else if (bCollectMaterialParameter)
				{
					bCollectMaterialParameter = false;
					//提取param
					std::stringstream param(s.c_str());
					std::string type, name;
					param >> type >> name;
					size_t shaderParamSize = shaderParamInfos.size();
					if (strlen(shaderParamInfos[shaderParamSize - 1].name) <= 0)
					{
						if (name[name.size() - 1] == ';')
						{
							name.erase(name.begin() + name.size() - 1);
						}
						name.copy(shaderParamInfos[shaderParamSize - 1].name, sizeof(shaderParamInfos[shaderParamSize - 1].name) - 1);
						shaderParamInfos[shaderParamSize - 1].name[sizeof(shaderParamInfos[shaderParamSize - 1].name) - 1] = '\0';
					}				
					//
					if (type.compare("float4") == 0)
					{
						shaderParamInfos[shaderParamSize - 1].type = MPType::Float4;
						shaderParamInfos[shaderParamSize - 1].index = CollectMaterialParameterIndex;
						CollectMaterialParameterIndex += 4;
					}

					else if (type.compare("float3") == 0)
					{
						shaderParamInfos[shaderParamSize - 1].type = MPType::Float3;
						shaderParamInfos[shaderParamSize - 1].index = CollectMaterialParameterIndex;
						CollectMaterialParameterIndex += 3;
					}
					else if (type.compare("float2") == 0)
					{
						shaderParamInfos[shaderParamSize - 1].type = MPType::Float2;
						shaderParamInfos[shaderParamSize - 1].index = CollectMaterialParameterIndex;
						CollectMaterialParameterIndex += 2;
					}
					else if (type.compare("float") == 0)
					{
						shaderParamInfos[shaderParamSize - 1].type = MPType::Float;
						shaderParamInfos[shaderParamSize - 1].index = CollectMaterialParameterIndex;
						CollectMaterialParameterIndex += 1;
					}
					header.shaderParameterCount++;
				}
				else if (bCollectMaterialTexture)
				{
					bCollectMaterialTexture = false;
					shaderTextureInfos[shaderTextureInfos.size() - 1].index = header.shaderTextureCount;
					header.shaderTextureCount++;
				}
			}
		}
		//
		if ((header.flags & EnableShaderDebug)) {
			options.SetOptimizationLevel(shaderc_optimization_level_zero);
			options.SetGenerateDebugInfo();
		}
		else {
			options.SetOptimizationLevel(shaderc_optimization_level_performance);
		}

		options.SetSourceLanguage(shaderc_source_language_hlsl);
		options.SetHlslIoMapping(true);
		//options.SetTargetSpirv(shaderc_spirv_version_1_3);
		HString fileName = srcShaderFileFullPath;
		HString filePath = fileName.GetFilePath();
		HString ResourcePath = FileSystem::GetShaderIncludeAbsPath();
		ResourcePath.CorrectionPath();
		options.SetIncluder(std::make_unique<ShaderIncluder>());
		HString dTarget;
		auto kind = shaderc_vertex_shader;
		if (shaderType == CompileShaderType::VertexShader)
		{
			dTarget = "vs_6_5";
			kind = shaderc_vertex_shader;
		}
		else if (shaderType == CompileShaderType::PixelShader)
		{
			dTarget = "ps_6_5";
			kind = shaderc_fragment_shader;
		}
		else if (shaderType == CompileShaderType::ComputeShader)
		{
			dTarget = "cs_6_5";
			kind = shaderc_compute_shader;
		}
		shaderc::SpvCompilationResult result = compiler.CompileGlslToSpv(_shaderSrcCode.c_str(), _shaderSrcCode.Length(), kind, fileName.GetBaseName().c_str(), entryPoint, options);
		auto resultStatus = result.GetCompilationStatus();
		if (resultStatus != shaderc_compilation_status_success)
		{
			std::cerr << result.GetErrorMessage();
			MessageOut(result.GetErrorMessage().c_str(), false, true, "255,0,0");
			return;
		}
		else
		{
			std::vector<uint32_t> resultChar = { result.cbegin(), result.cend() };
			HString cacheOnlyPath = (FileSystem::GetShaderCacheAbsPath()).c_str();
			HString cachePath = cacheOnlyPath + fileName.GetBaseName();
			if (shaderType == CompileShaderType::VertexShader)
			{
				cachePath += ("@vs");
			}
			else if (shaderType == CompileShaderType::PixelShader)
			{
				cachePath += ("@ps");
			}
			else if (shaderType == CompileShaderType::ComputeShader)
			{
				cachePath += ("@cs");
			}
			cachePath += TEXT(".spv");
			//Cache 路径不存在
			if (!FileSystem::FileExist(cacheOnlyPath.c_str()))
			{
				std::filesystem::create_directory(cacheOnlyPath.c_str());
			}
			std::ofstream out(cachePath.c_str(), std::ios::binary );
			if (out.is_open())
			{
				//Header 存入
				out.write((char*)&header, sizeof(ShaderCacheHeader));
				//Shader 参数信息存入
				for (auto s : shaderParamInfos)
				{
					out.write((char*)&s, sizeof(ShaderParameterInfo));
				}
				for (auto s : shaderTextureInfos)
				{
					out.write((char*)&s, sizeof(ShaderTextureInfo));
				}
				//ShaderCache
				out.write((char*)resultChar.data(), resultChar.size() * sizeof(glm::uint));
				out.close();
				//Log
				HString compileResultStr = TEXT("Compile shader [");
				compileResultStr += srcShaderFileFullPath;
				compileResultStr += TEXT("] successful.");
				ConsoleDebug::print_endl(compileResultStr, TEXT("0,255,50"));
			}
			else
			{
			}
		}
	}
}




#endif