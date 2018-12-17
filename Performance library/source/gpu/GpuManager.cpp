// Copyright (c) 2017-2018, Roman Shkurdalov
// This file is under The Clear BSD License, see LICENSE.txt

#include "gpu\GpuManager.h"
#include "gpu\GpuDevice.h"
#include "gpu\GpuMemoryManager.h"
#include <vector>

namespace gpu
{
	GpuDevice *gpuDevice;

	HResult GpuInitialize()
	{
		VkInstance vkInstance;
		VkApplicationInfo appInfo = {};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pNext = nullptr;
		appInfo.pApplicationName = "";
		appInfo.applicationVersion = 1;
		appInfo.pEngineName = "";
		appInfo.engineVersion = 1;
		appInfo.apiVersion = VK_API_VERSION_1_1;
		VkInstanceCreateInfo instInfo = {};
		instInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		instInfo.pNext = nullptr;
		instInfo.flags = 0;
		instInfo.pApplicationInfo = &appInfo;
		char8 *extensions[] = { VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_WIN32_SURFACE_EXTENSION_NAME };
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
		for (uint32 iter = 0; iter < queueCreateInfo.size(); iter++)
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
		char8 *deviceExtensions[] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
		deviceInfo.enabledExtensionCount = ARRAYSIZE(deviceExtensions);
		deviceInfo.ppEnabledExtensionNames = deviceExtensions;
		deviceInfo.enabledLayerCount = 0;
		deviceInfo.ppEnabledLayerNames = nullptr;
		deviceInfo.pEnabledFeatures = nullptr;
		deviceInfo.flags = 0;
		if (vkCreateDevice(vkPhysicalDevice, &deviceInfo, nullptr, &vkDevice) != VK_SUCCESS)
			return HResultNoSuitableGPU;

		uint32 graphicsQueueFamilyIndex;
		VkQueue vkGraphicsQueue;
		for (uint32 i = 0; i < familyProps.size(); i++)
		{
			if (familyProps[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
				graphicsQueueFamilyIndex = i;
		}
		vkGetDeviceQueue(vkDevice, graphicsQueueFamilyIndex, 0, &vkGraphicsQueue);

		VkCommandPoolCreateInfo cmdPoolInfo;
		cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		cmdPoolInfo.pNext = nullptr;
		cmdPoolInfo.queueFamilyIndex = graphicsQueueFamilyIndex;
		cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT
			| VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
		VkCommandPool vkCmdPool;
		vkCreateCommandPool(vkDevice, &cmdPoolInfo, nullptr, &vkCmdPool);

		VkDescriptorPoolSize descPoolSize[1];
		descPoolSize[0].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		descPoolSize[0].descriptorCount = 1;

		VkDescriptorPoolCreateInfo descPoolCreateInfo;
		descPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		descPoolCreateInfo.pNext = nullptr;
		descPoolCreateInfo.maxSets = 1;
		descPoolCreateInfo.pPoolSizes = descPoolSize;
		descPoolCreateInfo.poolSizeCount = 1;
		descPoolCreateInfo.flags = 0;
		VkDescriptorPool vkDescPool;
		vkCreateDescriptorPool(
			vkDevice,
			&descPoolCreateInfo,
			nullptr,
			&vkDescPool);

		VkDescriptorSetLayoutBinding descSetBinding;
		descSetBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		descSetBinding.descriptorCount = 1;
		descSetBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		descSetBinding.binding = 0;
		descSetBinding.pImmutableSamplers = nullptr;
		VkDescriptorSetLayoutCreateInfo descSetCreateInfo;
		descSetCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		descSetCreateInfo.pNext = nullptr;
		descSetCreateInfo.pBindings = &descSetBinding;
		descSetCreateInfo.bindingCount = 1;
		descSetCreateInfo.flags = 0;
		VkDescriptorSetLayout descSetLayout;
		vkCreateDescriptorSetLayout(vkDevice, &descSetCreateInfo, nullptr, &descSetLayout);

		VkDescriptorSetAllocateInfo descAllocInfo;
		descAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		descAllocInfo.pNext = nullptr;
		descAllocInfo.descriptorPool = vkDescPool;
		descAllocInfo.pSetLayouts = &descSetLayout;
		descAllocInfo.descriptorSetCount = 1;
		VkDescriptorSet vkDescSet;
		vkAllocateDescriptorSets(
			vkDevice,
			&descAllocInfo,
			&vkDescSet);

		VkPushConstantRange vkPushConstantRange;
		vkPushConstantRange.offset = 0;
		vkPushConstantRange.size = 128;
		vkPushConstantRange.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo;
		pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutCreateInfo.pNext = nullptr;
		pipelineLayoutCreateInfo.pushConstantRangeCount = 1;
		pipelineLayoutCreateInfo.pPushConstantRanges = &vkPushConstantRange;
		pipelineLayoutCreateInfo.setLayoutCount = 1;
		pipelineLayoutCreateInfo.pSetLayouts = &descSetLayout;
		pipelineLayoutCreateInfo.flags = 0;
		VkPipelineLayout vkPipelineLayout;
		vkCreatePipelineLayout(
			vkDevice,
			&pipelineLayoutCreateInfo,
			nullptr,
			&vkPipelineLayout);

		gpuDevice = new GpuDevice(
			vkInstance,
			vkPhysicalDevice,
			deviceProperties,
			vkDevice,
			familyProps,
			vkCmdPool,
			vkDescPool,
			descSetLayout,
			vkDescSet,
			vkPipelineLayout,
			graphicsQueueFamilyIndex,
			vkGraphicsQueue);

		return HResultSuccess;
	}

	void QueryGpuDevice(GpuDevice **ppGpuDevice)
	{
		gpuDevice->AddRef();
		*ppGpuDevice = gpuDevice;
	}
}
