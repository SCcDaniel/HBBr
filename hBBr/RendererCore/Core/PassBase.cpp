﻿#include "PassBase.h"
#include "VulkanRenderer.h"
#include "VulkanManager.h"
#include "Texture.h"
#include "PassManager.h"
#include "VertexFactory.h"
#include "DescriptorSet.h"

PassBase::PassBase(VulkanRenderer* renderer)
{
	_renderer = renderer;
	//we will collect all passes we need, direct execute them by once.So need not create VkFence for the every alone pass.
	//_fence.resize(VulkanManager::GetManager()->GetSwapchainBufferCount());
}

PassBase::~PassBase()
{
	_pipeline.reset();
}

std::shared_ptr<Texture> PassBase::GetSceneTexture(uint32_t descIndex)
{
	return _renderer->GetPassManager()->GetSceneTexture()->GetTexture(SceneTextureDesc(descIndex));
}

void GraphicsPass::ResetFrameBuffer(VkExtent2D size, std::vector<VkImageView> swapchainImageViews, std::vector<VkImageView> imageViews)
{
	if (_currentFrameBufferSize.width != size.width || _currentFrameBufferSize.height != size.height)
	{
		_currentFrameBufferSize = size;
		const auto manager = VulkanManager::GetManager();
		const auto frameIndex = VulkanRenderer::GetCurrentFrameIndex();
		//Insert swapchain imageView to first.
		if (_framebuffers.size() > 0)
		{
			vkQueueWaitIdle(manager->GetGraphicsQueue());
			manager->DestroyFrameBuffers(_framebuffers);
		}
		_framebuffers.resize(manager->GetSwapchainBufferCount());
		//VulkanManager::GetManager()->CreateFrameBuffer(size.width, size.height, _renderPass, imageViews, _framebuffers[frameIndex]);
		//VulkanManager::GetManager()->CreateFrameBuffers({ size.width, size.height }, _renderPass, imageViews, _framebuffers);
		for (int i = 0; i < (int)VulkanManager::GetManager()->GetSwapchainBufferCount(); i++)
		{
			std::vector<VkImageView> ivs = { swapchainImageViews[i] };
			if (imageViews.size() > 0)
			{
				ivs.insert(ivs.end(), imageViews.begin(), imageViews.end());
			}
			VulkanManager::GetManager()->CreateFrameBuffer(size.width, size.height, _renderPass, ivs, _framebuffers[i]);
		}
	}
}

void GraphicsPass::CreateRenderPass()
{
	VulkanManager::GetManager()->CreateRenderPass(_attachmentDescs, _subpassDependencys, _subpassDescs, _renderPass);
}

GraphicsPass::~GraphicsPass()
{
	VulkanManager::GetManager()->DestroyFrameBuffers(_framebuffers);
	VulkanManager::GetManager()->DestroyRenderPass(_renderPass);
}

void GraphicsPass::AddAttachment(VkAttachmentLoadOp loadOp, VkAttachmentStoreOp storeOp, VkFormat attachmentFormat, VkImageLayout initLayout, VkImageLayout finalLayout)
{
	//attachment
	VkAttachmentDescription attachmentDesc = {};
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
		depen.srcSubpass = (uint32_t)_subpassDependencys.size() - 1;
		depen.dstSubpass = (uint32_t)_subpassDependencys.size();//Next subpass.	
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

VkFramebuffer GraphicsPass::GetFrameBuffer()const
{
	return _framebuffers[_renderer->GetCurrentFrameIndex()];
}

OpaquePass::~OpaquePass()
{
	_descriptorSet_pass.reset();
	_descriptorSet_obj.reset();
}

void OpaquePass::PassInit()
{
	//Swapchain
	AddAttachment(VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE, _renderer->GetSurfaceFormat().format, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR , VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
	AddSubpass({}, {0} , -1);
	CreateRenderPass();
}

void OpaquePass::PassBuild()
{
	_descriptorSet_pass.reset(new DescriptorSet(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1));
	_descriptorSet_pass->CreateBuffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
	_descriptorSet_obj.reset(new DescriptorSet(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1));
	_descriptorSet_obj->CreateBuffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

	_pipeline.reset(new Pipeline());
	//Setting pipeline start
	_pipeline->SetColorBlend(false);
	_pipeline->SetRenderRasterizer();
	_pipeline->SetRenderDepthStencil();
	_pipeline->SetVertexInput(VertexFactory::VertexInputBase::BuildLayout());
	//
	_pipeline->SetPipelineLayout(
		{ 
			_descriptorSet_pass->GetDescriptorSetLayout() ,
			_descriptorSet_obj->GetDescriptorSetLayout() 
		});
	_pipeline->SetVertexShaderAndPixelShader(Shader::_vsShader["TestShader"], Shader::_psShader["TestShader"]);
	//Setting pipeline end
	_pipeline->CreatePipelineObject(_renderPass, (uint32_t)_subpassDescs.size());
}

void OpaquePass::PassUpdate()
{
	const auto manager = VulkanManager::GetManager();
	const auto cmdBuf = _renderer->GetCommandBuffer();
	//Update FrameBuffer
	ResetFrameBuffer(_renderer->GetSurfaceSize(), _renderer->GetSwapchainImageViews() , {});
	manager->CmdSetViewport(cmdBuf, { _currentFrameBufferSize });
	manager->BeginRenderPass(cmdBuf, GetFrameBuffer(), _renderPass, _currentFrameBufferSize, _attachmentDescs, { 0,0,0.0,0 });

	//vkCmdBindPipeline(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline->GetPipelineObject());



	manager->EndRenderPass(cmdBuf);
}

#include "imgui.h"
#include "backends/imgui_impl_vulkan.h"

void ImguiPass::PassInit()
{
	//Swapchain
	AddAttachment(VK_ATTACHMENT_LOAD_OP_LOAD, VK_ATTACHMENT_STORE_OP_STORE, _renderer->GetSurfaceFormat().format, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
	AddSubpass({}, { 0 }, -1);
	CreateRenderPass();
}
void ImguiPass::PassBuild()
{
	const auto manager = VulkanManager::GetManager();
	manager->InitImgui(_renderer->GetWindowHandle(), _renderPass);
}

ImguiPass::~ImguiPass()
{
	VulkanManager::GetManager()->ShutdownImgui();
}

void ImguiPass::PassUpdate()
{
	const auto manager = VulkanManager::GetManager();
	const auto cmdBuf = _renderer->GetCommandBuffer();
	ImGui_ImplVulkan_SetMinImageCount(manager->GetSwapchainBufferCount());
	//Update FrameBuffer
	ResetFrameBuffer(_renderer->GetSurfaceSize(), _renderer->GetSwapchainImageViews(), {});
	manager->CmdSetViewport(cmdBuf, { _currentFrameBufferSize });
	manager->BeginRenderPass(cmdBuf, GetFrameBuffer(), _renderPass, _currentFrameBufferSize, _attachmentDescs, { 0,0,0,0 });
	//Begin
	manager->ImguiNewFrame();
	ImGui::Text( HString(HString::FromInt(ImGui::GetMousePos().x) + "," + HString::FromInt(ImGui::GetMousePos().y )).c_str());
	//End
	manager->ImguiEndFrame(cmdBuf);
	manager->EndRenderPass(cmdBuf);
}