﻿#pragma once
//Pass基类
#include "Common.h"
#include "Pipeline.h"
#include <vector>
#include <memory>
#include <map>
#include "VulkanRenderer.h"
class Texture;

class PassBase
{
	friend class PassManager;
public:
	PassBase(VulkanRenderer* renderer) { _renderer = renderer; }
	~PassBase() {}
	virtual void BuildPass() {}
protected:
	virtual void PassInit() {}
	virtual void PassUpdate() {}
	virtual void PassReset() {}
	std::shared_ptr<Texture> GetSceneTexture(uint32_t descIndex);
	std::unique_ptr<Pipeline> _pipeline;
	VulkanRenderer* _renderer = NULL;
	HString _passName = "PassBase";
};

class GraphicsPass : public PassBase
{
public:
	GraphicsPass(VulkanRenderer* renderer) :PassBase(renderer) {}
	//Step 1 , Can add multiple attachments.
	virtual void AddAttachment(VkAttachmentLoadOp loadOp, VkAttachmentStoreOp storeOp, std::shared_ptr<Texture> texture);
	//Step 2 , Setup subpass by attachments.
	virtual void AddSubpass(std::vector<uint32_t> inputAttachments, std::vector<uint32_t> colorAttachments, int depthStencilAttachments = -1);
	//Step the last
	virtual void BuildPass()override;

	__forceinline VkRenderPass GetRenderPass()const
	{
		return _renderPass;
	}
protected:
	VkRenderPass _renderPass = VK_NULL_HANDLE;
	std::vector<VkAttachmentDescription>_attachmentDescs;
	std::vector<VkSubpassDependency>_subpassDependencys;
	std::vector<VkSubpassDescription>_subpassDescs;
	//
	std::vector<VkAttachmentReference> _input_ref;
	std::vector<VkAttachmentReference> _color_ref;
	VkAttachmentReference	_depthStencil_ref;
};

class ComputePass : public PassBase
{
public:
	ComputePass(VulkanRenderer* renderer) :PassBase(renderer) {}
};

class OpaquePass :public GraphicsPass
{
public:
	OpaquePass(VulkanRenderer* renderer) :GraphicsPass(renderer) {}
	virtual void PassInit()override;
	virtual void BuildPass()override;
};