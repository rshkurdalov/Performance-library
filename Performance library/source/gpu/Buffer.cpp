// Copyright (c) 2017-2018, Roman Shkurdalov
// This file is under The Clear BSD License, see LICENSE.txt

#include "gpu\Buffer.h"
#include "gpu\GpuDevice.h"

namespace gpu
{
	Buffer::Buffer(
		GpuDevice *device,
		VkBuffer vkBuffer,
		VkBufferCreateInfo vkBufferInfo,
		uint32 vkMemoryTypeBits,
		VkDeviceMemory vkBufferMemory,
		VkDeviceSize bufferSize)
	{
		device->AddRef();
		this->device = device;
		this->vkBuffer = vkBuffer;
		this->vkBufferInfo = vkBufferInfo;
		this->vkMemoryTypeBits = vkMemoryTypeBits;
		this->vkBufferMemory = vkBufferMemory;
		this->bufferSize = bufferSize;
	}
	Buffer::~Buffer()
	{
		vkFreeMemory(device->vkDevice, vkBufferMemory, nullptr);
		vkDestroyBuffer(device->vkDevice, vkBuffer, nullptr);
		device->Release();
	}
	msize Buffer::GetSize()
	{
		return (msize)bufferSize;
	}
	HResult Buffer::Resize(msize newSize)
	{
		vkFreeMemory(device->vkDevice, vkBufferMemory, nullptr);
		vkDestroyBuffer(device->vkDevice, vkBuffer, nullptr);

		vkBufferInfo.size = newSize;
		CheckReturnFail(vkCreateBuffer(
			device->vkDevice,
			&vkBufferInfo,
			nullptr,
			&vkBuffer));

		VkMemoryRequirements vkMemReqs;
		vkGetBufferMemoryRequirements(device->vkDevice, vkBuffer, &vkMemReqs);
		VkMemoryAllocateInfo allocInfo;
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.pNext = nullptr;
		CheckReturn(VkGetMemoryTypeFromRequirements(
			device->vkPhysicalDevice,
			vkMemReqs.memoryTypeBits,
			vkMemoryTypeBits,
			&allocInfo.memoryTypeIndex));
		allocInfo.allocationSize = vkMemReqs.size;
		bufferSize = vkMemReqs.size;
		CheckReturnFail(vkAllocateMemory(
			device->vkDevice,
			&allocInfo,
			nullptr,
			&vkBufferMemory));
		CheckReturnFail(vkBindBufferMemory(
			device->vkDevice,
			vkBuffer,
			vkBufferMemory,
			0));

		return HResultSuccess;
	}
	HResult Buffer::MapMemory(
		msize offset,
		msize size,
		void **ppData)
	{
		CheckReturnFail(vkMapMemory(
			device->vkDevice,
			vkBufferMemory,
			(VkDeviceSize)offset,
			(VkDeviceSize)size,
			0,
			ppData));
		return HResultSuccess;
	}
	void Buffer::UnmapMemory()
	{
		vkUnmapMemory(device->vkDevice, vkBufferMemory);
	}
}
