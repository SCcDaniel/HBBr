﻿#include "Shader.h"
#include "FileSystem.h"
#include "VulkanManager.h"
#include <fstream>
std::map<HString, ShaderCache> Shader::_vsShader;
std::map<HString, ShaderCache> Shader::_psShader;
std::map<HString, ShaderCache> Shader::_csShader;

void Shader::LoadShaderCache(const char* cachePath)
{
	auto allCacheFiles = FileSystem::GetFilesBySuffix(cachePath, "spv");
	uint64_t cacheIndex = 0;
	for (auto i : allCacheFiles)
	{
		HString fileName = i.baseName;
		auto split = fileName.Split("@");
		ShaderCache cache = {};
		cache.shaderCacheIndex = cacheIndex;
		//
		//auto shaderData = FileSystem::ReadBinaryFile(i.absPath.c_str());
		std::ifstream file(i.absPath.c_str(), std::ios::ate | std::ios::binary);
		size_t fileSize = static_cast<size_t>(file.tellg());
		file.seekg(0);
		//header
		file.read((char*)&cache.header, sizeof(ShaderCacheHeader));
		//shader parameter infos
		for (int i = 0; i < cache.header.shaderParameterCount; i++)
		{
			ShaderParameterInfo newInfo;
			file.read((char*)&newInfo, sizeof(ShaderParameterInfo));
			cache.params.push_back(newInfo);
		}
		//cache
		std::vector<char> shaderData(fileSize - sizeof(ShaderCacheHeader) - (sizeof(ShaderParameterInfo) * cache.header.shaderParameterCount));
		file.read(shaderData.data(), fileSize - sizeof(ShaderCacheHeader) - (sizeof(ShaderParameterInfo) * cache.header.shaderParameterCount));
		VulkanManager::GetManager()->CreateShaderModule(shaderData, cache.shaderModule);
		file.close();
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
		cacheIndex++;
	}

}

void Shader::DestroyAllShaderCache()
{
	for (auto& i : _vsShader)
	{
		vkDestroyShaderModule(VulkanManager::GetManager()->GetDevice(), i.second.shaderModule, VK_NULL_HANDLE);
	}
	for (auto& i : _psShader)
	{
		vkDestroyShaderModule(VulkanManager::GetManager()->GetDevice(), i.second.shaderModule, VK_NULL_HANDLE);
	}
	for (auto& i : _csShader)
	{
		vkDestroyShaderModule(VulkanManager::GetManager()->GetDevice(), i.second.shaderModule, VK_NULL_HANDLE);
	}
	_vsShader.clear();
	_psShader.clear();
	_csShader.clear();
}
