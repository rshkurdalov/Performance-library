// Copyright (c) 2017-2018, Roman Shkurdalov
// This file is under The Clear BSD License, see LICENSE.txt

#pragma once
#include "kernel\kernel.h"
#include "kernel\SharedObject.h"
#include "math\VectorMath.h"
#include "gpu\VulkanAPI.h"
#include <vector>

namespace gpu
{
	class CommandBuffer : public SharedObject
	{
		friend class CommandPool;
		friend class GpuDevice;
		friend class RenderTarget;
	protected:
		static const uint64 zeroOffset = 0;
		CommandPool *cmdPool;
		VkCommandBuffer vkCmdBuffer;
		Pipeline *currentPipeline;
		VkClearValue clearValues[2];
		VkViewport vkViewport;
		std::vector<VkRect2D> scissors;

		CommandBuffer(
			CommandPool *cmdPool,
			VkCommandBuffer vkCmdBuffer);
		~CommandBuffer();
	public:
		void Begin();
		void End();
		void BeginRenderPass(SwapChain *swapChain);
		void EndRenderPass();
		void BindPipeline(Pipeline *pipeline);
		void BindDescriptorSets(
			Pipeline *pipeline,
			DescriptorSet *descSet);
		void SetViewport(
			float32 x,
			float32 y,
			float32 width,
			float32 height,
			float32 minDepth,
			float32 maxDepth);
		void PushScissor(
			int32 x,
			int32 y,
			uint32 width,
			uint32 height);
		void PopScissor();
		void PushConstants(
			void *data,
			uint32 offset,
			uint32 size,
			VkShaderStageFlagBits shaderType);
		void UpdateWorldTransform(
			Pipeline *pipeline,
			Matrix4x4f *transform);
		void BindVertexBuffer(
			uint32 bindingIndex,
			Buffer *buffer);
		void CopyBuffer(
			Buffer *srcBuffer,
			Buffer *dstBuffer,
			msize srcOffset,
			msize dstOffset,
			msize size);
		void Draw(
			uint32 vertexCount,
			uint32 instanceCount,
			uint32 firstVertex,
			uint32 firstInstance);
		void Submit(SwapChain *swapChain);
		void SubmitCopy(SwapChain *swapChain);
	};
}
