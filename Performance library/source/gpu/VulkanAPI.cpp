// Copyright (c) 2017-2018, Roman Shkurdalov
// This file is under The Clear BSD License, see LICENSE.txt

#include "gpu\VulkanAPI.h"
#include "gpu\GpuDevice.h"
#include <vector>

namespace gpu
{
	VkInstance vkInstance = VK_NULL_HANDLE;
	GpuDevice *gpuDevice = nullptr;

	HResult VkInitialize()
	{
		VkApplicationInfo appInfo = {};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pNext = nullptr;
		appInfo.pApplicationName = "";
		appInfo.applicationVersion = 1;
		appInfo.pEngineName = "";
		appInfo.engineVersion = 1;
		appInfo.apiVersion = VK_API_VERSION_1_0;
		VkInstanceCreateInfo instInfo = {};
		instInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		instInfo.pNext = nullptr;
		instInfo.flags = 0;
		instInfo.pApplicationInfo = &appInfo;
		char8 *extensions[] = {VK_KHR_WIN32_SURFACE_EXTENSION_NAME, VK_KHR_SURFACE_EXTENSION_NAME};
		instInfo.enabledExtensionCount = ARRAYSIZE(extensions);
		instInfo.ppEnabledExtensionNames = extensions;
		instInfo.enabledLayerCount = 0;
		instInfo.ppEnabledLayerNames = nullptr;
		CheckReturnFail(vkCreateInstance(&instInfo, nullptr, &vkInstance));

		VkPhysicalDevice vkPhysicalDevice;
		VkDevice vkDevice;
		uint32 gpuCount = 1;
		CheckReturnFail(vkEnumeratePhysicalDevices(vkInstance, &gpuCount, nullptr));
		std::vector<VkPhysicalDevice> gpuDevices(gpuCount);
		CheckReturnFail(vkEnumeratePhysicalDevices(vkInstance, &gpuCount, gpuDevices.data()));
		VkPhysicalDeviceProperties deviceProperties;
		VkPhysicalDeviceFeatures deviceFeatures;
		uint32 bestGpuIndex, bestGpuScore = 0;
		for (uint32 i = 0, score; i < gpuCount; i++)
		{
			vkGetPhysicalDeviceProperties(gpuDevices[i], &deviceProperties);
			vkGetPhysicalDeviceFeatures(gpuDevices[i], &deviceFeatures);
			score = 0;
			if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
				score += 1000;
			score += deviceProperties.limits.maxImageDimension2D;
			if (!deviceFeatures.geometryShader)
				score = 0;
			if (score > bestGpuScore)
			{
				bestGpuIndex = i;
				bestGpuScore = score;
			}
		}
		if (bestGpuScore == 0) return HResultNoSuitableGPU;
		vkPhysicalDevice = gpuDevices[bestGpuIndex];
		vkGetPhysicalDeviceProperties(vkPhysicalDevice, &deviceProperties);

		uint32 queueFamilyCount;
		vkGetPhysicalDeviceQueueFamilyProperties(
			vkPhysicalDevice,
			&queueFamilyCount,
			nullptr);
		std::vector<VkQueueFamilyProperties> familyProps(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(
			vkPhysicalDevice,
			&queueFamilyCount,
			familyProps.data());
		float32 queuePriority = 1.0f;
		std::vector<VkDeviceQueueCreateInfo> queueCreateInfo(familyProps.size());
		for (msize iter = 0; iter < queueCreateInfo.size(); iter++)
		{
			queueCreateInfo[iter].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo[iter].pNext = nullptr;
			queueCreateInfo[iter].queueFamilyIndex = iter;
			queueCreateInfo[iter].queueCount = 1;
			queueCreateInfo[iter].pQueuePriorities = &queuePriority;
			queueCreateInfo[iter].flags = 0;
		}

		VkDeviceCreateInfo deviceInfo;
		deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		deviceInfo.pNext = nullptr;
		deviceInfo.pQueueCreateInfos = queueCreateInfo.data();
		deviceInfo.queueCreateInfoCount = queueCreateInfo.size();
		deviceInfo.enabledExtensionCount = 0;
		deviceInfo.ppEnabledExtensionNames = nullptr;
		deviceInfo.enabledLayerCount = 0;
		deviceInfo.ppEnabledLayerNames = nullptr;
		deviceInfo.pEnabledFeatures = nullptr;
		deviceInfo.flags = 0;
		if (vkCreateDevice(vkPhysicalDevice, &deviceInfo, nullptr, &vkDevice) != VK_SUCCESS)
			return HResultNoSuitableGPU;

		gpuDevice = new GpuDevice(
			vkPhysicalDevice,
			deviceProperties,
			vkDevice,
			familyProps);

		return HResultSuccess;
	}

	HResult VkQueryInstance(VkInstance *pVkInstance)
	{
		if (vkInstance == VK_NULL_HANDLE) return HResultFail;
		*pVkInstance = vkInstance;
		return HResultSuccess;
	}

	HResult VkQueryGpuDevice(GpuDevice **ppGpuDevice)
	{
		if (gpuDevice == nullptr)
			return HResultFail;
		gpuDevice->AddRef();
		*ppGpuDevice = gpuDevice;
		return HResultSuccess;
	}

	HResult VkGetMemoryTypeFromRequirements(
		VkPhysicalDevice vkPhysicalDevice,
		uint32 typeBits,
		VkFlags requirementsMask,
		uint32 *typeIndex)
	{
		VkPhysicalDeviceMemoryProperties memProps;
		vkGetPhysicalDeviceMemoryProperties(vkPhysicalDevice, &memProps);
		for (uint32 i = 0; i < memProps.memoryTypeCount; i++)
		{
			if (typeBits & 1)
			{
				if ((memProps.memoryTypes[i].propertyFlags & requirementsMask) == requirementsMask)
				{
					*typeIndex = i;
					return HResultSuccess;
				}
			}
			typeBits >>= 1;
		}
		return HResultFail;
	}

	HResult VkGetAvailableSampleCount(
		VkPhysicalDeviceProperties vkDeviceProp,
		VkSampleCountFlagBits desiredSampleCount,
		VkSampleCountFlagBits *availableSampleCount)
	{
		VkSampleCountFlagBits maxSamples = min(
			(VkSampleCountFlagBits)vkDeviceProp.limits.framebufferColorSampleCounts,
			(VkSampleCountFlagBits)vkDeviceProp.limits.framebufferDepthSampleCounts);
		if (desiredSampleCount <= maxSamples)
			*availableSampleCount = desiredSampleCount;
		else *availableSampleCount = (VkSampleCountFlagBits)(maxSamples ^ (maxSamples >> 1));
		return HResultSuccess;
	}
}
