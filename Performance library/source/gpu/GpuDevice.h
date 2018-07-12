// Copyright (c) 2017-2018, Roman Shkurdalov
// This file is under The Clear BSD License, see LICENSE.txt

#pragma once
#include "kernel\kernel.h"
#include "kernel\SharedObject.h"
#include "gpu\VulkanAPI.h"
#include "graphics\Color.h"
#include "gpu\GradientCollection.h"
#include <vector>
#include <concrt.h>

#include <fstream>

namespace gpu
{
	class GpuDevice : public SharedObject
	{
		friend class GpuMemoryManager;
		friend class CommandPool;
		friend class CommandBuffer;
		friend class DescriptorPool;
		friend class DescriptorSet;
		friend class Shader;
		friend class Pipeline;
		friend class SwapChain;
		friend class Buffer;
		friend class RenderTarget;
		friend class Bitmap;
		friend class GradientCollection;
		friend HResult VkInitialize();
	protected:
		VkPhysicalDevice vkPhysicalDevice;
		VkPhysicalDeviceProperties vkDeviceProp;
		VkDevice vkDevice;
		std::vector<VkQueueFamilyProperties> familyProp;
		uint32 graphicsQueueFamilyIndex;
		VkQueue vkGraphicsQueue;
		VkSampleCountFlagBits msaa;
		VkPipelineLayout vkPipelineLayout;
		DescriptorSet *descSet;
		Buffer *storageBuffer;
		Buffer *vertexBuffer;
		msize geometryMemory;
		GpuMemoryManager *memManager;
		concurrency::critical_section csMemory;
	protected:
		GpuDevice(
			VkPhysicalDevice vkPhysicalDevice,
			VkPhysicalDeviceProperties vkDeviceProp,
			VkDevice vkDevice,
			std::vector<VkQueueFamilyProperties> &familyProp);
		~GpuDevice();
		msize AllocateMemory(msize size);
		void DeallocateMemory(msize offset);
		void MapMemory(msize offset, msize size, void **ppData);
		void UnmapMemory();
	public:
		HResult CreateCommandPool(CommandPool **ppCommandPool);
		HResult CreateDescriptorPool(DescriptorPool **ppDescPool);
		HResult CreateSwapChain(
			uint32 width,
			uint32 height,
			Surface *surface,
			SwapChain **ppSwapChain);
		HResult CreateShader(
			uint32 *code,
			uint32 size,
			Shader **ppShader);
		HResult CreatePipeline(
			SwapChain* swapChain,
			Shader *vertexShader,
			Shader *fragmentShader,
			Pipeline **ppPipeline);
		HResult CreateRenderTargetPipelines(
			SwapChain* swapChain,
			Shader *vertexShader,
			Shader *fragmentShader,
			Pipeline **ppPipeline);
		HResult CreateBuffer(
			VkBufferUsageFlags vkBufferType,
			uint32 memoryTypeBits,
			msize size,
			Buffer **ppBuffer);
		HResult CreateRenderTarget(
			SwapChain *swapChain,
			CommandBuffer *cmdBuffer,
			RenderTarget **ppRenderTarget);
		HResult CreateBitmap(
			uint32 width,
			uint32 height,
			Bitmap **ppBitmap);
		HResult CreateGradientCollection(
			GradientStop *stops,
			uint32 count,
			GradientCollection **ppGradientCollection);
	};
}
