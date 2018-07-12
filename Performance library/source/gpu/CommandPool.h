// Copyright (c) 2017-2018, Roman Shkurdalov
// This file is under The Clear BSD License, see LICENSE.txt

#pragma once
#include "kernel\kernel.h"
#include "kernel\SharedObject.h"
#include "gpu\VulkanAPI.h"

namespace gpu
{
	class CommandPool : public SharedObject
	{
		friend class GpuDevice;
		friend class CommandBuffer;
		friend class RenderTarget;
	protected:
		GpuDevice *device;
		VkCommandPool vkCmdPool;

		CommandPool(
			GpuDevice *device,
			VkCommandPool vkCmdPool);
		~CommandPool();
	public:
		HResult CreateCommandBuffer(CommandBuffer **ppCommandBuffer);
	};
}
