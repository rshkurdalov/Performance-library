// Copyright (c) 2017-2018, Roman Shkurdalov
// This file is under The Clear BSD License, see LICENSE.txt

#pragma once
#pragma comment(lib, "dependencies\\vulkan\\lib32\\vulkan-1.lib")

#define VK_USE_PLATFORM_WIN32_KHR

#include "vulkan\vulkan.h"
#include "kernel\kernel.h"

namespace gpu
{
	HResult VkInitialize();

	HResult VkQueryInstance(VkInstance *vkInstance);

	HResult VkQueryGpuDevice(GpuDevice **ppGpuDevice);

	HResult VkGetMemoryTypeFromRequirements(
		VkPhysicalDevice vkPhysicalDevice,
		uint32 typeBits,
		VkFlags requirementsMask,
		uint32 *typeIndex);

	HResult VkGetAvailableSampleCount(
		VkPhysicalDeviceProperties vkDeviceProp,
		VkSampleCountFlagBits desiredSampleCount,
		VkSampleCountFlagBits *availableSampleCount);
}
