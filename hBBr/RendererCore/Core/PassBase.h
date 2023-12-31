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
	virtual void Reset() {}
	virtual void PassReset() {}
	std::shared_ptr<Texture> GetSceneTexture(uint32_t descIndex);
	VulkanRenderer* _renderer = nullptr;
	HString _passName = "PassBase";
	glm::vec4 _markColor = glm::vec4(1,1,1,0.5);
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
	virtual void AddSubpass(std::vector<uint32_t> inputAttachments, std::vector<uint32_t> colorAttachments, int depthStencilAttachments ,
		VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask,VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask);

	//Step the last,custom.
	virtual void PassInit()override {}
	virtual void PassUpdate()override;
	virtual void PassReset()override {}
	void Reset() override {
		_currentFrameBufferSize = { 999999 , 999999 };
		PassReset();
	}
	virtual void ResetFrameBuffer(VkExtent2D size,std::vector<VkImageView> imageViews);
	void CreateRenderPass();
	HBBR_INLINE VkRenderPass GetRenderPass()const
	{
		return _renderPass;
	}
	HString _passName = "Graphics Pass" ;
protected:
	virtual void PassRender() {}
	void BeginRenderPass(std::array<float, 4> clearColor = { 0.0f, 0.0f, 0.0f, 0.0f });
	void EndRenderPass();
	void SetViewport(VkExtent2D viewportSize);
	VkFramebuffer GetFrameBuffer()const;
	VkRenderPass _renderPass = VK_NULL_HANDLE;
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

class CommandPass : public PassBase
{
public:
	CommandPass(VulkanRenderer* renderer) :PassBase(renderer) {}
};


class ComputePass : public PassBase
{
public:
	ComputePass(VulkanRenderer* renderer) :PassBase(renderer) {}
};
