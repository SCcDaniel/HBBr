﻿#pragma once
//Pass基类
#include "Common.h"
#include "Pipeline.h"
#include <vector>
#include <memory>
#include <map>
#include "HString.h"
#include "DescriptorSet.h"
#include "VertexFactory.h"
#include "Primitive.h"
class Texture;
class VulkanRenderer;

class PassBase
{
	friend class VulkanManager;
	friend class PassManager;
public:
	PassBase(VulkanRenderer* renderer);
	virtual ~PassBase();
	HBBR_INLINE HString GetName()const { return _passName; }
protected:
	virtual void PassInit() {}
	virtual void PassUpdate() {}
	virtual void PassReset() {}
	std::shared_ptr<Texture> GetSceneTexture(uint32_t descIndex);
	VulkanRenderer* _renderer = NULL;
	HString _passName = "PassBase";
};

class GraphicsPass : public PassBase
{
public:
	GraphicsPass(VulkanRenderer* renderer) :PassBase(renderer) {}
	virtual ~GraphicsPass();
	//Step 1 , Can add multiple attachments.
	virtual void AddAttachment(VkAttachmentLoadOp loadOp, VkAttachmentStoreOp storeOp, VkFormat attachmentFormat, VkImageLayout initLayout, VkImageLayout finalLayout);
	//Step 2 , Setup subpass by attachments.
	virtual void AddSubpass(std::vector<uint32_t> inputAttachments, std::vector<uint32_t> colorAttachments, int depthStencilAttachments = -1);
	//Step the last,custom.
	virtual void PassInit()override {}
	virtual void PassReset()override { _currentFrameBufferSize = { 999999 , 999999 }; }
	virtual void ResetFrameBuffer(VkExtent2D size,std::vector<VkImageView> imageViews);
	void CreateRenderPass();
	HBBR_INLINE VkRenderPass GetRenderPass()const
	{
		return _renderPass;
	}
protected:
	void BeginRenderPass(std::array<float, 4> clearColor = { 0.0f, 0.0f, 0.0f, 0.0f });
	void EndRenderPass();
	void SetViewport(VkExtent2D viewportSize);
	VkFramebuffer GetFrameBuffer()const;
	VkRenderPass _renderPass = VK_NULL_HANDLE;
	VkPipelineLayout _pipelineLayout = VK_NULL_HANDLE;
	std::vector<VkAttachmentDescription>_attachmentDescs;
	std::vector<VkSubpassDependency>_subpassDependencys;
	std::vector<VkSubpassDescription>_subpassDescs;
	//
	std::vector<VkAttachmentReference> _input_ref;
	std::vector<VkAttachmentReference> _color_ref;
	VkAttachmentReference	_depthStencil_ref;
	std::vector<VkFramebuffer> _framebuffers;
	VkExtent2D _currentFrameBufferSize;
};

class ComputePass : public PassBase
{
public:
	ComputePass(VulkanRenderer* renderer) :PassBase(renderer) {}
};
