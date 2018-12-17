// Copyright (c) 2017-2018, Roman Shkurdalov
// This file is under The Clear BSD License, see LICENSE.txt

#pragma once
#include "kernel\kernel.h"
#include "kernel\SharedObject.h"
#include "gpu\GpuDevice.h"

namespace gpu
{
	class Buffer : public SharedObject
	{
		friend class GpuDevice;
		friend class GpuMemoryManager;
		friend class CommandBuffer;
		friend class DescriptorSet;
		friend class RenderTarget;
	protected:
		GpuDevice *device;
		VkBuffer vkBuffer;
		VkBufferCreateInfo vkBufferInfo;
		uint32 vkMemoryTypeBits;
		VkDeviceMemory vkBufferMemory;
		VkDeviceSize bufferSize;

		Buffer(
			GpuDevice *device,
			VkBuffer vkBuffer,
			VkBufferCreateInfo vkBufferInfo,
			uint32 vkMemoryTypeBits,
			VkDeviceMemory vkBufferMemory,
			VkDeviceSize bufferSize);
		~Buffer();
	public:
		uint32 GetSize();
		HResult Resize(uint32 newSize);
		HResult MapMemory(
			uint32 offset,
			uint32 size,
			void **ppData);
		void UnmapMemory();
	};
}
