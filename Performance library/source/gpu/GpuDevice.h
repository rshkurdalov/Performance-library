// Copyright (c) 2017-2018, Roman Shkurdalov
// This file is under The Clear BSD License, see LICENSE.txt

#pragma once
#pragma comment(lib, "dependencies\\vulkan\\lib32\\vulkan-1.lib")
#include "kernel\kernel.h"
#define VK_USE_PLATFORM_WIN32_KHR
#include "vulkan\vulkan.h"
#include "kernel\SharedObject.h"
#include "gpu\GpuManager.h"
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
		friend class Geometry;
		friend HResult GpuInitialize();
	protected:
		VkInstance vkInstance;
		VkPhysicalDevice vkPhysicalDevice;
		VkPhysicalDeviceProperties vkDeviceProp;
		VkDevice vkDevice;
		std::vector<VkQueueFamilyProperties> familyProp;
		VkCommandPool vkCmdPool;
		VkDescriptorPool vkDescPool;
		VkDescriptorSetLayout descSetLayout;
		VkDescriptorSet vkDescSet;
		VkPipelineLayout vkPipelineLayout;
		uint32 graphicsQueueFamilyIndex;
		VkQueue vkGraphicsQueue;
		Buffer *storageBuffer;
		GpuMemoryManager *memManager;
		concurrency::critical_section memorySection;
		VkSampleCountFlagBits msaa;

		GpuDevice(
			VkInstance vkInstance,
			VkPhysicalDevice vkPhysicalDevice,
			VkPhysicalDeviceProperties vkDeviceProp,
			VkDevice vkDevice,
			std::vector<VkQueueFamilyProperties> &familyProp,
			VkCommandPool vkCmdPool,
			VkDescriptorPool vkDescPool,
			VkDescriptorSetLayout descSetLayout,
			VkDescriptorSet vkDescSet,
			VkPipelineLayout vkPipelineLayout,
			uint32 graphicsQueueFamilyIndex,
			VkQueue vkGraphicsQueue);
		~GpuDevice();
		uint32 AllocateMemory(uint32 size);
		void DeallocateMemory(uint32 offset);
		void MapMemory(uint32 offset, uint32 size, void **ppData);
		void UnmapMemory();
		void UpdateStorageBuffer();
		bool GetMemoryTypeFromRequirements(
			VkPhysicalDevice vkPhysicalDevice,
			uint32 typeBits,
			VkFlags requirementsMask,
			uint32 *typeIndex);
		void GetAvailableSampleCount(
			VkPhysicalDeviceProperties vkDeviceProp,
			VkSampleCountFlagBits desiredSampleCount,
			VkSampleCountFlagBits *availableSampleCount);
		HResult CreateShader(
			uint32 *code,
			uint32 size,
			Shader **ppShader);
		HResult CreateSwapChain(
			uint32 width,
			uint32 height,
			Surface *surface,
			SwapChain **ppSwapChain);
		HResult CreatePipeline(
			SwapChain* swapChain,
			Shader *vertexShader,
			Shader *fragmentShader,
			Pipeline **ppPipeline);
		HResult CreateCommandBuffer(CommandBuffer **ppCommandBuffer);
		HResult CreateBuffer(
			VkBufferUsageFlags vkBufferType,
			uint32 memoryTypeBits,
			uint32 size,
			Buffer **ppBuffer);
	public:
		VkInstance GetVkInstance();
		HResult CreateRenderTarget(
			uint32 width,
			uint32 height,
			Surface *surface,
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
