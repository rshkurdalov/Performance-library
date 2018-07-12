// Copyright (c) 2017-2018, Roman Shkurdalov
// This file is under The Clear BSD License, see LICENSE.txt

#pragma once
#include "kernel\kernel.h"
#include "kernel\SharedObject.h"
#include "gpu\VulkanAPI.h"

namespace gpu
{
	class DescriptorSet : public SharedObject
	{
		friend class DescriptorPool;
		friend class GpuDevice;
		friend class CommandBuffer;
		friend class Buffer;
		friend class RenderTarget;
	protected:
		DescriptorPool *descPool;
		VkDescriptorSet vkDescSet;

		DescriptorSet(
			DescriptorPool *descPool,
			VkDescriptorSet vkDescSet);
		~DescriptorSet();
	public:
		void UpdateBuffer(Buffer *buffer);
	};
}
