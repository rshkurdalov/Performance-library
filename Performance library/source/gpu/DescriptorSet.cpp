// Copyright (c) 2017-2018, Roman Shkurdalov
// This file is under The Clear BSD License, see LICENSE.txt

#include "gpu\DescriptorSet.h"
#include "gpu\GpuDevice.h"
#include "gpu\DescriptorPool.h"
#include "gpu\Buffer.h"

namespace gpu
{
	DescriptorSet::DescriptorSet(
		DescriptorPool *descPool,
		VkDescriptorSet vkDescSet)
	{
		descPool->AddRef();
		this->descPool = descPool;
		this->vkDescSet = vkDescSet;
	}
	DescriptorSet::~DescriptorSet()
	{
		vkFreeDescriptorSets(
			descPool->device->vkDevice,
			descPool->vkDescPool,
			1,
			&vkDescSet);
		descPool->Release();
	}
	void DescriptorSet::UpdateBuffer(Buffer *buffer)
	{
		VkDescriptorBufferInfo descBufferInfo;
		descBufferInfo.buffer = buffer->vkBuffer;
		descBufferInfo.offset = 0;
		descBufferInfo.range = buffer->bufferSize;
		VkWriteDescriptorSet descWrite;
		descWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descWrite.pNext = nullptr;
		descWrite.dstSet = vkDescSet;
		descWrite.descriptorCount = 1;
		descWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		descWrite.pBufferInfo = &descBufferInfo;
		descWrite.dstArrayElement = 0;
		descWrite.dstBinding = 0;
		vkUpdateDescriptorSets(descPool->device->vkDevice, 1, &descWrite, 0, nullptr);
	}
}
