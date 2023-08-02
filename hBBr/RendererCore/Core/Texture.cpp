﻿#include "Texture.h"
#include "VulkanRenderer.h"
#include "VulkanManager.h"

SceneTexture::SceneTexture(VulkanRenderer* renderer)
{
	_renderer = renderer;
	auto sceneColor = Texture::CreateTexture2D(1, 1, VK_FORMAT_R16G16B16A16_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, "SceneColor");
	auto sceneDepth = Texture::CreateTexture2D(1, 1, VK_FORMAT_D32_SFLOAT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, "SceneDepth");
	//Transition
	VkCommandBuffer cmdbuf;
	VulkanManager::GetManager()->AllocateCommandBuffer(VulkanManager::GetManager()->GetCommandPool(), cmdbuf);
	VulkanManager::GetManager()->BeginCommandBuffer(cmdbuf);
	{
		sceneColor->Transition(cmdbuf, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
		sceneDepth->Transition(cmdbuf, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);
	}
	VulkanManager::GetManager()->EndCommandBuffer(cmdbuf);
	VulkanManager::GetManager()->SubmitQueueImmediate({ cmdbuf });
	vkQueueWaitIdle(VulkanManager::GetManager()->GetGraphicsQueue());
	VulkanManager::GetManager()->FreeCommandBuffers(VulkanManager::GetManager()->GetCommandPool(), { cmdbuf });
	//
	_sceneTexture.insert(std::make_pair(SceneTextureDesc::SceneColor, sceneColor));
	_sceneTexture.insert(std::make_pair(SceneTextureDesc::SceneDepth, sceneDepth));
}

void SceneTexture::UpdateTextures()
{
	auto sceneDepth = _sceneTexture[SceneTextureDesc::SceneDepth];
	if (sceneDepth->GetImageSize().width != _renderer->GetSurfaceSize().width ||
		sceneDepth->GetImageSize().height != _renderer->GetSurfaceSize().height)
	{
		_sceneTexture[SceneTextureDesc::SceneDepth]->Resize(_renderer->GetSurfaceSize().width, _renderer->GetSurfaceSize().height);
	}
}

Texture::~Texture()
{
	VulkanManager::GetManager()->DestroyImageMemory(_imageViewMemory);
	VulkanManager::GetManager()->DestroyImageView(_imageView);
	VulkanManager::GetManager()->DestroyImage(_image);
}

void Texture::Transition(VkCommandBuffer cmdBuffer, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevelBegin, uint32_t mipLevelCount)
{
	VulkanManager::GetManager()->Transition(cmdBuffer, _image, _imageAspectFlags, oldLayout, newLayout, mipLevelBegin, mipLevelCount);
	_imageLayout = newLayout;
}

void Texture::TransitionImmediate(VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevelBegin, uint32_t mipLevelCount)
{
	VkCommandBuffer cmdbuf;
	VulkanManager::GetManager()->AllocateCommandBuffer(VulkanManager::GetManager()->GetCommandPool(), cmdbuf);
	VulkanManager::GetManager()->BeginCommandBuffer(cmdbuf);
	{
		Transition(cmdbuf, oldLayout, newLayout);
	}
	VulkanManager::GetManager()->EndCommandBuffer(cmdbuf);
	VulkanManager::GetManager()->SubmitQueueImmediate({ cmdbuf });
	vkQueueWaitIdle(VulkanManager::GetManager()->GetGraphicsQueue());
	VulkanManager::GetManager()->FreeCommandBuffers(VulkanManager::GetManager()->GetCommandPool(), { cmdbuf });
}

void Texture::Resize(uint32_t width, uint32_t height)
{
	if (_image == VK_NULL_HANDLE)
		return;

	if (_imageViewMemory != VK_NULL_HANDLE)
		VulkanManager::GetManager()->FreeBufferMemory(_imageViewMemory);
	VulkanManager::GetManager()->DestroyImageView(_imageView);
	VulkanManager::GetManager()->DestroyImage(_image);
	//
	VulkanManager::GetManager()->CreateImage(width, height, _format, _usageFlags, _image);
	if (!_bNoMemory)
		VulkanManager::GetManager()->CreateImageMemory(_image, _imageViewMemory);
	VulkanManager::GetManager()->CreateImageView(_image, _format, _imageAspectFlags, _imageView);
	//
	VkCommandBuffer cmdbuf;
	VulkanManager::GetManager()->AllocateCommandBuffer(VulkanManager::GetManager()->GetCommandPool(), cmdbuf);
	VulkanManager::GetManager()->BeginCommandBuffer(cmdbuf);
	{
		Transition(cmdbuf, VK_IMAGE_LAYOUT_UNDEFINED, _imageLayout);
	}
	VulkanManager::GetManager()->EndCommandBuffer(cmdbuf);
	VulkanManager::GetManager()->SubmitQueueImmediate({ cmdbuf });
	vkQueueWaitIdle(VulkanManager::GetManager()->GetGraphicsQueue());
	VulkanManager::GetManager()->FreeCommandBuffers(VulkanManager::GetManager()->GetCommandPool(), { cmdbuf });

	_imageSize = { width,height };
}

std::shared_ptr<Texture> Texture::CreateTexture2D(uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usageFlags, HString textureName, bool noMemory)
{
	std::shared_ptr<Texture> newTexture = std::make_shared<Texture>();
	newTexture->_bNoMemory = noMemory;
	newTexture->_textureName = textureName;
	newTexture->_imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	newTexture->_imageSize = {width,height};
	VulkanManager::GetManager()->CreateImage(width,height,format , usageFlags, newTexture->_image);
	if (format == VK_FORMAT_R32_SFLOAT || format == VK_FORMAT_D32_SFLOAT)
		newTexture->_imageAspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT;
	else if (format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT || format == VK_FORMAT_D16_UNORM_S8_UINT)
		newTexture->_imageAspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
	else
		newTexture->_imageAspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
	if (!newTexture->_bNoMemory)
	{
		VulkanManager::GetManager()->CreateImageMemory(newTexture->_image, newTexture->_imageViewMemory);
	}
	VulkanManager::GetManager()->CreateImageView(newTexture->_image, format, newTexture->_imageAspectFlags, newTexture->_imageView);
	newTexture->_format = format;
	newTexture->_usageFlags = usageFlags;
	return std::move(newTexture);
}
