﻿#include "Shader.h"
#include "FileSystem.h"
#include "VulkanManager.h"

std::map<HString, ShaderCache> Shader::_vsShader;
std::map<HString, ShaderCache> Shader::_psShader;
std::map<HString, ShaderCache> Shader::_csShader;

void Shader::LoadShaderCache(const char* cachePath)
{
	auto allCacheFiles = FileSystem::GetFilesBySuffix(cachePath, "spv");
	for (auto i : allCacheFiles)
	{
		HString fileName = i.baseName;
		auto split = fileName.Split("@");
		ShaderCache cache = {};
		//
		auto shaderData = FileSystem::ReadBinaryFile(i.absPath.c_str());
		VkShaderModuleCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		info.codeSize = shaderData.size();
		info.pCode = reinterpret_cast<const uint32_t*>(shaderData.data());
		vkCreateShaderModule(VulkanManager::GetManager()->GetDevice(),&info,nullptr, &cache.shaderModule);
		//
		cache.shaderPath = i.relativePath;
		cache.shaderName = split[0];
		cache.shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		cache.shaderStageInfo.module = cache.shaderModule;
		
		if (split[1] == "vs")
		{
			cache.shaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
			cache.shaderStageInfo.pName = "VSMain";
			cache.shaderType = ShaderType::VertexShader;
			_vsShader.emplace(std::make_pair(cache.shaderName, cache));
		}
		else if (split[1] == "ps")
		{
			cache.shaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
			cache.shaderStageInfo.pName = "PSMain";
			cache.shaderType = ShaderType::PixelShader;
			_psShader.emplace(std::make_pair(cache.shaderName, cache));
		}
		else if (split[1] == "cs")
		{
			cache.shaderStageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
			cache.shaderStageInfo.pName = "CSMain";
			cache.shaderType = ShaderType::ComputeShader;
			_csShader.emplace(std::make_pair(cache.shaderName, cache));
		}
	}

}
