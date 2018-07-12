// Copyright (c) 2017-2018, Roman Shkurdalov
// This file is under The Clear BSD License, see LICENSE.txt

#include "gpu\DescriptorPool.h"
#include "gpu\GpuDevice.h"
#include "gpu\DescriptorSet.h"

namespace gpu
{
	DescriptorPool::DescriptorPool(
		GpuDevice *device,
		VkDescriptorPool vkDescPool)
	{
		device->AddRef();
		this->device = device;
		this->vkDescPool = vkDescPool;
	}
	DescriptorPool::~DescriptorPool()
	{
		vkDestroyDescriptorPool(
			device->vkDevice,
			vkDescPool,
			nullptr);
		device->Release();
	}
	HResult DescriptorPool::CreateDescriptorSet(
		uint32 count,
		VkDescriptorSetLayout *vkDescLayouts,
		DescriptorSet **ppDescSet)
	{
		VkDescriptorSetAllocateInfo descAllocInfo;
		descAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		descAllocInfo.pNext = nullptr;
		descAllocInfo.descriptorPool = vkDescPool;
		descAllocInfo.descriptorSetCount = count;
		descAllocInfo.pSetLayouts = vkDescLayouts;
		VkDescriptorSet vkDescSet(count);
		CheckReturnFail(vkAllocateDescriptorSets(
			device->vkDevice,
			&descAllocInfo,
			&vkDescSet));

		*ppDescSet = new DescriptorSet(this, vkDescSet);

		return HResultSuccess;
	}
}
