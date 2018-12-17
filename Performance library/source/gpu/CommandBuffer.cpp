// Copyright (c) 2017-2018, Roman Shkurdalov
// This file is under The Clear BSD License, see LICENSE.txt

#include "gpu\CommandBuffer.h"
#include "gpu\GpuDevice.h"
#include "gpu\SwapChain.h"
#include "gpu\Pipeline.h"
#include "gpu\Buffer.h"

namespace gpu
{
	CommandBuffer::CommandBuffer(
		GpuDevice *device,
		VkCommandBuffer vkCmdBuffer)
	{
		device->AddRef();
		this->device = device;
		this->vkCmdBuffer = vkCmdBuffer;

		currentPipeline = nullptr;
		clearValues[0].color.float32[0] = 0.0f;
		clearValues[0].color.float32[1] = 0.0f;
		clearValues[0].color.float32[2] = 0.0f;
		clearValues[0].color.float32[3] = 0.0f;
		clearValues[1].depthStencil.depth = 1.0f;
		clearValues[1].depthStencil.stencil = 0;
	}
	CommandBuffer::~CommandBuffer()
	{
		vkFreeCommandBuffers(
			device->vkDevice,
			device->vkCmdPool,
			1,
			&vkCmdBuffer);
		device->Unref();
	}
	void CommandBuffer::Begin()
	{
		VkCommandBufferBeginInfo vkCmdBufferBeginInfo;
		vkCmdBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		vkCmdBufferBeginInfo.pNext = nullptr;
		vkCmdBufferBeginInfo.pInheritanceInfo = nullptr;
		vkCmdBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		vkBeginCommandBuffer(vkCmdBuffer, &vkCmdBufferBeginInfo);
	}
	void CommandBuffer::End()
	{
		vkEndCommandBuffer(vkCmdBuffer);
		currentPipeline = nullptr;
		scissors.clear();
	}
	void CommandBuffer::BeginRenderPass(SwapChain *swapChain)
	{
		VkRenderPassBeginInfo renderPassBegin;
		renderPassBegin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBegin.pNext = nullptr;
		renderPassBegin.renderPass = swapChain->vkRenderPass;
		renderPassBegin.framebuffer = swapChain->vkFramebuffers[swapChain->GetCurrentBuffer()];
		renderPassBegin.renderArea.offset.x = 0;
		renderPassBegin.renderArea.offset.y = 0;
		renderPassBegin.renderArea.extent = swapChain->vkSwapChainCreateInfo.imageExtent;
		renderPassBegin.pClearValues = clearValues;
		renderPassBegin.clearValueCount = 2;

		vkCmdBeginRenderPass(
			vkCmdBuffer,
			&renderPassBegin,
			VK_SUBPASS_CONTENTS_INLINE);

		VkRect2D scissor = renderPassBegin.renderArea;
		scissors.clear();
		scissors.push_back(scissor);
		vkCmdSetScissor(vkCmdBuffer, 0, 1, &scissor);
	}
	void CommandBuffer::EndRenderPass()
	{
		vkCmdEndRenderPass(vkCmdBuffer);
	}
	void CommandBuffer::BindPipeline(Pipeline *pipeline)
	{
		currentPipeline = pipeline;
		vkCmdBindPipeline(
			vkCmdBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			pipeline->vkPipeline);
	}
	void CommandBuffer::BindDescriptorSet(VkDescriptorSet vkDescSet)
	{
		vkCmdBindDescriptorSets(
			vkCmdBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			currentPipeline->vkPipelineLayout,
			0,
			1,
			&vkDescSet,
			0,
			nullptr);
	}
	void CommandBuffer::SetViewport(
		float32 x,
		float32 y,
		float32 width,
		float32 height,
		float32 minDepth,
		float32 maxDepth)
	{
		vkViewport.x = x;
		vkViewport.y = y;
		vkViewport.width = width;
		vkViewport.height = height;
		vkViewport.minDepth = minDepth;
		vkViewport.maxDepth = maxDepth;
		vkCmdSetViewport(vkCmdBuffer, 0, 1, &vkViewport);
	}
	void CommandBuffer::PushScissor(
		float32 x,
		float32 y,
		float32 width,
		float32 height)
	{
		VkRect2D scissor;
		scissor.offset.x = Max((int32)floor(x), scissors.back().offset.x);
		scissor.offset.y = Max((int32)floor(y), scissors.back().offset.y);
		scissor.extent.width = (uint32)Max(
			0, Min((int32)ceil(x + width), scissors.back().offset.x + (int32)scissors.back().extent.width) - scissor.offset.x);
		scissor.extent.height = (uint32)Max(
			0, Min((int32)ceil(y + height), scissors.back().offset.y + (int32)scissors.back().extent.height) - scissor.offset.y);
		scissors.push_back(scissor);
		vkCmdSetScissor(vkCmdBuffer, 0, 1, &scissor);
	}
	void CommandBuffer::PopScissor()
	{
		if (scissors.size() == 1) return;
		scissors.pop_back();
		vkCmdSetScissor(vkCmdBuffer, 0, 1, &scissors.back());
	}
	void CommandBuffer::PushConstants(
		void *data,
		uint32 offset,
		uint32 size,
		VkShaderStageFlagBits shaderType)
	{
		vkCmdPushConstants(
			vkCmdBuffer,
			currentPipeline->vkPipelineLayout,
			shaderType,
			offset,
			size,
			(uint8 *)data + offset);
	}
	void CommandBuffer::BindVertexBuffer(
		uint32 bindingIndex,
		Buffer *buffer)
	{
		vkCmdBindVertexBuffers(
			vkCmdBuffer,
			bindingIndex,
			1,
			&buffer->vkBuffer,
			(VkDeviceSize *)&zeroOffset);
	}
	void CommandBuffer::Draw(
		uint32 vertexCount,
		uint32 instanceCount,
		uint32 firstVertex,
		uint32 firstInstance)
	{
		vkCmdDraw(
			vkCmdBuffer,
			vertexCount,
			instanceCount,
			firstVertex,
			firstInstance);
	}
	void CommandBuffer::Submit(SwapChain *swapChain)
	{
		VkPipelineStageFlags pipelineStageFlags =
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		VkSubmitInfo submitInfo;
		submitInfo.pNext = nullptr;
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.pWaitDstStageMask = &pipelineStageFlags;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &vkCmdBuffer;
		submitInfo.waitSemaphoreCount = 0;
		submitInfo.pWaitSemaphores = nullptr;
		submitInfo.signalSemaphoreCount = 0;
		submitInfo.pSignalSemaphores = nullptr;
		vkQueueSubmit(
			swapChain->vkGraphicsQueue,
			1,
			&submitInfo,
			VK_NULL_HANDLE);
		vkQueueWaitIdle(swapChain->vkGraphicsQueue);
	}
}
