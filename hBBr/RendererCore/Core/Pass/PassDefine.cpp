﻿#include "PassDefine.h"
#include "VulkanRenderer.h"
#include "DescriptorSet.h"
#include "VertexFactory.h"
#include "Buffer.h"
#include "Primitive.h"
#include "Pass/PassType.h"
#include "Texture.h"

/*
	Opaque pass
*/
#pragma region OpaquePass
BasePass::~BasePass()
{
	VulkanManager::GetManager()->DestroyPipelineLayout(_pipelineLayout);
}

void BasePass::PassInit()
{
	//Swapchain
	AddAttachment(VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE, _renderer->GetSurfaceFormat().format, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
	//SceneDepth
	AddAttachment(VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE, GetSceneTexture((uint32_t)SceneTextureDesc::SceneDepth)->GetFormat() , VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
	AddSubpass({}, { 0 }, 1);
	CreateRenderPass();
	//DescriptorSet
	_opaque_descriptorSet_pass.reset(new DescriptorSet(_renderer, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1, sizeof(PassUniformBuffer)));
	_opaque_descriptorSet_obj.reset(new DescriptorSet(_renderer, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1));
	_opaque_descriptorSet_mat.reset(new DescriptorSet(_renderer, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1));
	_opaque_vertexBuffer.reset(new Buffer(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT));
	_opaque_indexBuffer.reset(new Buffer(VK_BUFFER_USAGE_INDEX_BUFFER_BIT));
	VulkanManager::GetManager()->CreatePipelineLayout(
		{
			_opaque_descriptorSet_pass->GetDescriptorSetLayout() ,
			_opaque_descriptorSet_obj->GetDescriptorSetLayout(),
			_opaque_descriptorSet_mat->GetDescriptorSetLayout()
		}
	, _pipelineLayout);
	//Pass Uniform总是一尘不变的,并且我们用的是Dynamic uniform buffer ,所以只需要更新一次所有的DescriptorSet即可。
	_opaque_descriptorSet_pass->UpdateDescriptorSetAll(sizeof(PassUniformBuffer));

	_passName = "Opaque Render Pass";
}

void BasePass::PassUpdate()
{
	const auto manager = VulkanManager::GetManager();
	const auto cmdBuf = _renderer->GetCommandBuffer();
	COMMAND_MAKER(cmdBuf, BasePass, _passName.c_str(), _markColor);
	//Update FrameBuffer
	ResetFrameBuffer(_renderer->GetSurfaceSize(), { GetSceneTexture((uint32_t)SceneTextureDesc::SceneDepth)->GetTextureView() });
	SetViewport(_currentFrameBufferSize);
	BeginRenderPass({ 0,0,0,0 });
	//Opaque Pass
	SetupBasePassAndDraw(
		Pass::OpaquePass,
		_opaque_descriptorSet_pass.get(),
		_opaque_descriptorSet_obj.get(),
		_opaque_descriptorSet_mat.get(),
		_opaque_vertexBuffer.get(),
		_opaque_indexBuffer.get()
	);
	
	//manager->CmdNextSubpass(cmdBuf);
	//Translucent pass
	//...

	
	EndRenderPass();
}

void BasePass::SetupBasePassAndDraw(Pass p, DescriptorSet* pass, DescriptorSet* obj, DescriptorSet* mat, Buffer* vb, Buffer* ib)
{
	const auto manager = VulkanManager::GetManager();
	const auto cmdBuf = _renderer->GetCommandBuffer();
	VkDeviceSize vbOffset = 0;
	VkDeviceSize ibOffset = 0;
	VkDeviceSize matOffset = 0;
	uint32_t objectUboOffset = 0;
	std::vector<uint32_t> matBufferOffset;
	std::vector<uint32_t> matBufferSize;
	PipelineObject* pipelineObj = NULL;
	//Reset buffer
	{
		bool bUpdateObjUb = false;
		//Update pass uniform buffers
		{
			PassUniformBuffer passUniformBuffer = _renderer->GetPassUniformBufferCache();
			pass->BufferMapping(&passUniformBuffer, 0, sizeof(PassUniformBuffer));
		}
		uint32_t objectCount = 0;
		for (auto m : PrimitiveProxy::GetMaterialPrimitives((uint32_t)p))
		{
			//Get Pipeline
			pipelineObj = PipelineManager::GetGraphicsPipelineMap(m->graphicsIndex);
			if (pipelineObj == NULL)
			{
				auto vsCache = Shader::_vsShader[m->vsShader];
				auto psCache = Shader::_psShader[m->psShader];
				VkGraphicsPipelineCreateInfoCache pipelineCreateInfo = {};
				PipelineManager::SetColorBlend(pipelineCreateInfo, false);
				PipelineManager::SetColorBlend(pipelineCreateInfo, false);
				PipelineManager::SetRenderRasterizer(pipelineCreateInfo);
				PipelineManager::SetRenderDepthStencil(pipelineCreateInfo);
				PipelineManager::SetVertexInput(pipelineCreateInfo, m->inputLayout);
				PipelineManager::SetVertexShaderAndPixelShader(pipelineCreateInfo, vsCache, psCache);
				//Setting pipeline end
				pipelineObj = PipelineManager::CreatePipelineObject(pipelineCreateInfo, _pipelineLayout,
					_renderPass, m->graphicsIndex, (uint32_t)_subpassDescs.size());
			}
			if (m->uniformBufferSize != 0)
			{
				////Test
				//m->uniformBuffer[2] = glm::vec4(1,0,0,1);
				mat->BufferMapping(m->uniformBuffer.data(), 0, m->uniformBufferSize);
				matBufferOffset.push_back((uint32_t)matOffset);
				matBufferSize.push_back((uint32_t)m->uniformBufferSize);
				matOffset += m->uniformBufferSize;
			}
			auto prims = PrimitiveProxy::GetModelPrimitives(m);
			for (size_t m = 0; m < prims.size(); m++)
			{
				ModelPrimitive* prim = prims[m];
				//Vb,Ib
				bool bRefreshBuffer = false;
				if (vbOffset != prim->vbPos) //偏移信息不相同时,重新映射
				{
					vb->BufferMapping(prim->vertexData.data(), vbOffset, prim->vbSize);
					prim->vbPos = vbOffset;
					bRefreshBuffer = true;
				}
				if (ibOffset != prim->ibPos || bRefreshBuffer) //偏移信息不相同时,重新映射
				{
					ib->BufferMapping(prim->vertexIndices.data(), ibOffset, prim->ibSize);
					prim->ibPos = ibOffset;
					bRefreshBuffer = true;
				}
				vbOffset += prim->vbSize;
				ibOffset += prim->ibSize;
				//Object uniform
				if (prim->transform &&
					(prim->transform->NeedUpdateUb()
						|| bRefreshBuffer //当顶点发生改变的时候,UniformBuffer也应该一起更新。
						))
				{
					ObjectUniformBuffer objectUniformBuffer = {};
					objectUniformBuffer.WorldMatrix = prim->transform->GetWorldMatrix();
					obj->BufferMapping(&objectUniformBuffer, (uint64_t)objectUboOffset, sizeof(ObjectUniformBuffer));
					bUpdateObjUb = true;
				}
				objectCount++;
				objectUboOffset += (uint32_t)sizeof(ObjectUniformBuffer);
			}
		}
		if (bUpdateObjUb)
		{
			obj->ResizeDescriptorBuffer(sizeof(ObjectUniformBuffer) * (objectCount + 1));
		}
		//obj->NeedUpdate();
		obj->UpdateDescriptorSet(sizeof(ObjectUniformBuffer));
		//mat->NeedUpdate();
		mat->ResizeDescriptorBuffer(matOffset);
		mat->UpdateDescriptorSet(matBufferSize, matBufferOffset);
	}

	//Update Render
	vbOffset = 0;
	ibOffset = 0;
	objectUboOffset = 0;
	uint32_t dynamicOffset[] = { 0 };
	uint32_t matIndex = 0;
	vkCmdBindDescriptorSets(cmdBuf, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, _pipelineLayout, 0, 1, &pass->GetDescriptorSet(), 1, dynamicOffset);
	for (auto m : PrimitiveProxy::GetMaterialPrimitives((uint32_t)Pass::OpaquePass))
	{
		manager->CmdCmdBindPipeline(cmdBuf, pipelineObj->pipeline);
		vkCmdBindDescriptorSets(cmdBuf, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, _pipelineLayout, 2, 1, &mat->GetDescriptorSet(), 1, &matBufferOffset[matIndex]);
		auto prims = PrimitiveProxy::GetModelPrimitives(m);
		for (size_t m = 0; m < prims.size(); m++)
		{
			ModelPrimitive* prim = prims[m];
			{
				vkCmdBindDescriptorSets(cmdBuf, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, _pipelineLayout, 1, 1, &obj->GetDescriptorSet(), 1, &objectUboOffset);
				objectUboOffset += sizeof(ObjectUniformBuffer);

				VkBuffer verBuf[] = { vb->GetBuffer() };
				vkCmdBindVertexBuffers(cmdBuf, 0, 1, verBuf, &vbOffset);
				vkCmdBindIndexBuffer(cmdBuf, ib->GetBuffer(), ibOffset, VK_INDEX_TYPE_UINT32);
				vkCmdDrawIndexed(cmdBuf, (uint32_t)prim->vertexIndices.size(), 1, 0, 0, 0);

				vbOffset += prim->vbSize;
				ibOffset += prim->ibSize;
			}
		}
		matIndex++;
	}
}
#pragma endregion OpaquePass

PreCommandPass::~PreCommandPass()
{
}

void PreCommandPass::PassInit()
{
	_passName = "Precommand Pass";
	_markColor = glm::vec4(1, 1, 0, 0.5);
}

void PreCommandPass::PassUpdate()
{
	//Image data CPU to GPU
	for (auto i : Texture::GetUploadTextures())
	{
		
	}
}
