// Copyright (c) 2017-2018, Roman Shkurdalov
// This file is under The Clear BSD License, see LICENSE.txt

#pragma once
#include "kernel\kernel.h"
#include "kernel\SharedObject.h"
#include "gpu\VulkanAPI.h"

namespace gpu
{
	class DescriptorPool : public SharedObject
	{
		friend class GpuDevice;
		friend class DescriptorSet;
	protected:
		GpuDevice *device;
		VkDescriptorPool vkDescPool;

		DescriptorPool(
			GpuDevice *device,
			VkDescriptorPool vkDescPool);
		~DescriptorPool();
	public:
		HResult CreateDescriptorSet(
			uint32 count,
			VkDescriptorSetLayout *vkDescLayouts,
			DescriptorSet **ppDescSet);
	};
}
