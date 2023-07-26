﻿#include "PassBase.h"
#include "VulkanRenderer.h"
#include "VulkanManager.h"
#include "Texture.h"
#include "PassManager.h"
#include "VertexFactory.h"
#include "DescriptorSet.h"

void GraphicsPass::ResetFrameBuffer(VkExtent2D size, std::vector<VkImageView> imageViews)
{
	if (_currentFrameBufferSize.width != size.width || _currentFrameBufferSize.height != size.height)
	{
		_currentFrameBufferSize = size;
		const auto manager = VulkanManager::GetManager();
		if (_framebuffers.size() > 0)
		{
			vkQueueWaitIdle(manager->GetGraphicsQueue());
			manager->DestroyFrameBuffer(_framebuffers[VulkanRenderer::GetCurrentFrameIndex()]);
		}
		else
		{
			_framebuffers.resize(manager->GetSwapchainBufferCount());
		}
		VulkanManager::GetManager()->CreateFrameBuffer(size.width, size.height, _pipeline->GetRenderPass(), imageViews, _framebuffers[VulkanRenderer::GetCurrentFrameIndex()]);
	}
}

void GraphicsPass::PassBuild()
{
	VulkanManager::GetManager()->CreateRenderPass(_attachmentDescs, _subpassDependencys, _subpassDescs, _renderPass);
	_pipeline.reset(new Pipeline());
	//Setting pipeline start
	//.....
	//Setting pipeline end
	_pipeline->CreatePipelineObject(_renderPass, (uint32_t)_subpassDescs.size());
}

void GraphicsPass::AddAttachment(VkAttachmentLoadOp loadOp, VkAttachmentStoreOp storeOp,VkFormat attachmentFormat , VkImageLayout initLayout,VkImageLayout finalLayout)
{
	//attachment
	VkAttachmentDescription attachmentDesc = {} ;
	attachmentDesc.flags = 0;
	attachmentDesc.format = attachmentFormat;
	attachmentDesc.samples = VK_SAMPLE_COUNT_1_BIT;
	attachmentDesc.loadOp = loadOp;
	attachmentDesc.storeOp = storeOp;
	attachmentDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachmentDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachmentDesc.initialLayout = initLayout;
	attachmentDesc.finalLayout = finalLayout;
	_attachmentDescs.push_back(attachmentDesc);
}

void GraphicsPass::AddSubpass(std::vector<uint32_t> inputIndexes, std::vector<uint32_t> colorIndexes, int depthStencilIndex)
{
	_input_ref.resize(inputIndexes.size());
	for (int i = 0; i < inputIndexes.size(); i++)
	{
		_input_ref[i].attachment = inputIndexes[i];
		//_input_ref[i].layout = _attachmentDescs[inputIndexes[i]].finalLayout;
		_input_ref[i].layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	}
	_color_ref.resize(colorIndexes.size());
	for (int i = 0; i < colorIndexes.size(); i++)
	{
		_color_ref[i].attachment = colorIndexes[i];
		//_color_ref[i].layout = _attachmentDescs[colorIndexes[i]].finalLayout;
		_color_ref[i].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	}
	if (depthStencilIndex >= 0)
	{
		_depthStencil_ref.attachment = depthStencilIndex;
		//_depthStencil_ref.layout = _attachmentDescs[depthStencilIndex].finalLayout;
		_depthStencil_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	}
	VkSubpassDescription subpassDesc = {};
	subpassDesc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpassDesc.flags = 0;
	subpassDesc.inputAttachmentCount = (uint32_t)_input_ref.size();
	subpassDesc.pInputAttachments = _input_ref.data();
	subpassDesc.colorAttachmentCount = (uint32_t)_color_ref.size();
	subpassDesc.pColorAttachments = _color_ref.data();
	subpassDesc.pResolveAttachments = NULL;
	subpassDesc.pDepthStencilAttachment = depthStencilIndex >= 0 ? &_depthStencil_ref : NULL;
	subpassDesc.preserveAttachmentCount = 0;
	subpassDesc.pPreserveAttachments = NULL;
	//
	_subpassDescs.push_back(subpassDesc);

	VkSubpassDependency depen = {};
	if (_subpassDependencys.size() <= 0)
	{
		depen.srcSubpass = VK_SUBPASS_EXTERNAL;//pass out side
		depen.dstSubpass = 0;//Next subpass,into the first subpass.	
	}
	else
	{
		depen.srcSubpass = _subpassDependencys.size() - 1;
		depen.dstSubpass = _subpassDependencys.size() ;//Next subpass.	
	}
	depen.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
	if (depthStencilIndex >= 0)
	{
		depen.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		depen.dstAccessMask =
			VK_ACCESS_INPUT_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
			VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	}
	else
	{
		depen.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		depen.dstAccessMask =
			VK_ACCESS_INPUT_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	}
	depen.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	depen.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	_subpassDependencys.push_back(depen);
}

PassBase::PassBase(VulkanRenderer* renderer)
{
	_renderer = renderer;
	//CommandBuffers
	_cmdBuf.resize(VulkanManager::GetManager()->GetSwapchainBufferCount());
	for (int i = 0; i < _cmdBuf.size(); i++)
	{
		VulkanManager::GetManager()->AllocateCommandBuffer(VulkanManager::GetManager()->GetCommandPool(), _cmdBuf[i]);
	}
}

PassBase::~PassBase()
{
	VulkanManager::GetManager()->FreeCommandBuffers(VulkanManager::GetManager()->GetCommandPool(), _cmdBuf);
}

VkCommandBuffer& PassBase::GetCommandBuffer()
{
	return _cmdBuf[_renderer->GetCurrentFrameIndex()];
}

std::shared_ptr<Texture> PassBase::GetSceneTexture(uint32_t descIndex)
{
	return _renderer->GetPassManager()->GetSceneTexture()->GetTexture(SceneTextureDesc(descIndex));
}


void OpaquePass::PassInit()
{
	//Swapchain
	AddAttachment(VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE, _renderer->GetSwapchainImage()->GetFormat(), _renderer->GetSwapchainImage()->GetLayout(), VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
	AddSubpass({}, {0} , -1);
	PassBuild();
}

void OpaquePass::PassBuild()
{
	_descriptorSet_pass.reset(new DescriptorSet(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1));
	_descriptorSet_pass->CreateBuffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
	_descriptorSet_obj.reset(new DescriptorSet(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1));
	_descriptorSet_obj->CreateBuffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

	VulkanManager::GetManager()->CreateRenderPass(_attachmentDescs, _subpassDependencys, _subpassDescs, _renderPass);
	_pipeline.reset(new Pipeline());
	//Setting pipeline start
	_pipeline->SetColorBlend(false);
	_pipeline->SetRenderRasterizer();
	_pipeline->SetRenderDepthStencil();
	_pipeline->SetVertexInput(VertexFactory::VertexInputBase::BuildLayout());
	//
	_pipeline->SetPipelineLayout(
		{ 
			_descriptorSet_pass ->GetDescriptorSetLayout() ,
			_descriptorSet_obj->GetDescriptorSetLayout() 
		});
	_pipeline->SetVertexShaderAndPixelShader(Shader::_vsShader["TestShader"], Shader::_psShader["TestShader"]);
	//Setting pipeline end
	_pipeline->CreatePipelineObject(_renderPass, (uint32_t)_subpassDescs.size() - 1 );
}

void OpaquePass::PassUpdate()
{
	const uint32_t frameIndex = _renderer->GetCurrentFrameIndex();
	const auto manager = VulkanManager::GetManager();
	//Update FrameBuffer
	ResetFrameBuffer(_renderer->GetSurfaceSize(), { _renderer->GetSwapchainImage()->GetTextureView()});
	manager->BeginCommandBuffer(_cmdBuf[frameIndex]);
	manager->BeginRenderPass(_cmdBuf[frameIndex], _framebuffers[frameIndex], _pipeline->GetRenderPass(), _currentFrameBufferSize, _attachmentDescs, { 1,0,0,1 });
	manager->EndRenderPass(_cmdBuf[frameIndex]);
	manager->EndCommandBuffer(_cmdBuf[frameIndex]);
}