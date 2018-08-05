// Copyright (c) 2017-2018, Roman Shkurdalov
// This file is under The Clear BSD License, see LICENSE.txt

#include "gpu\GpuDevice.h"
#include "gpu\Surface.h"
#include "gpu\GpuMemoryManager.h"
#include "gpu\CommandPool.h"
#include "gpu\CommandBuffer.h"
#include "gpu\DescriptorPool.h"
#include "gpu\DescriptorSet.h"
#include "gpu\Shader.h"
#include "gpu\Pipeline.h"
#include "gpu\SwapChain.h"
#include "gpu\Buffer.h"
#include "gpu\RenderTarget.h"
#include "gpu\Bitmap.h"
#include "gpu\VulkanAPI.h"
#include "math\VectorMath.h"
#include <vector>

namespace gpu
{
	GpuDevice::GpuDevice(
		VkPhysicalDevice vkPhysicalDevice,
		VkPhysicalDeviceProperties vkDeviceProp,
		VkDevice vkDevice,
		std::vector<VkQueueFamilyProperties> &familyProp)
	{
		this->vkPhysicalDevice = vkPhysicalDevice;
		this->vkDeviceProp = vkDeviceProp;
		this->vkDevice = vkDevice;
		this->familyProp = familyProp;

		DescriptorPool *descPool;
		CreateDescriptorPool(&descPool);
		VkDescriptorSetLayoutBinding descSetBinding;
		descSetBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		descSetBinding.descriptorCount = 1;
		descSetBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		descSetBinding.binding = 0;
		descSetBinding.pImmutableSamplers = nullptr;
		VkDescriptorSetLayout descSetLayout;
		VkDescriptorSetLayoutCreateInfo descSetCreateInfo;
		descSetCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		descSetCreateInfo.pNext = nullptr;
		descSetCreateInfo.bindingCount = 1;
		descSetCreateInfo.pBindings = &descSetBinding;
		descSetCreateInfo.flags = 0;
		vkCreateDescriptorSetLayout(vkDevice, &descSetCreateInfo, nullptr, &descSetLayout);
		descPool->CreateDescriptorSet(1, &descSetLayout, &descSet);

		CreateBuffer(
			VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			GpuMemoryManager::InitialHeapSize,
			&storageBuffer);
		descSet->UpdateBuffer(storageBuffer);

		memManager = new GpuMemoryManager(storageBuffer, descSet);

		CreateBuffer(
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			RenderTarget::vertexBufferSize,
			&vertexBuffer);

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
		vkCreatePipelineLayout(
			vkDevice,
			&pipelineLayoutCreateInfo,
			nullptr,
			&vkPipelineLayout);

		for (uint32 i = 0; i < familyProp.size(); i++)
		{
			if (familyProp[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
				graphicsQueueFamilyIndex = i;
		}
		vkGetDeviceQueue(vkDevice, graphicsQueueFamilyIndex, 0, &vkGraphicsQueue);
	}
	GpuDevice::~GpuDevice()
	{
		vkDestroyPipelineLayout(vkDevice, vkPipelineLayout, nullptr);
		vkDestroyDevice(vkDevice, nullptr);
	}
	msize GpuDevice::AllocateMemory(msize size)
	{
		Lock();
		msize offset = memManager->Allocate(size);
		Unlock();
		return offset;
	}
	void GpuDevice::DeallocateMemory(msize offset)
	{
		Lock();
		memManager->Deallocate(offset);
		Unlock();
	}
	void GpuDevice::MapMemory(msize offset, msize size, void **ppData)
	{
		csMemory.lock();
		VkResult res = vkMapMemory(vkDevice, storageBuffer->vkBufferMemory, offset, size, 0, ppData);
	}
	void GpuDevice::UnmapMemory()
	{
		vkUnmapMemory(vkDevice, storageBuffer->vkBufferMemory);
		csMemory.unlock();
	}
	HResult GpuDevice::CreateCommandPool(CommandPool **ppCommandPool)
	{
		VkCommandPoolCreateInfo cmdPoolInfo;
		cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		cmdPoolInfo.pNext = nullptr;
		cmdPoolInfo.queueFamilyIndex = graphicsQueueFamilyIndex;
		cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT
			| VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
		VkCommandPool vkCmdPool;
		vkCreateCommandPool(vkDevice, &cmdPoolInfo, nullptr, &vkCmdPool);

		*ppCommandPool = new CommandPool(this, vkCmdPool);

		return HResultSuccess;
	}
	HResult GpuDevice::CreateDescriptorPool(DescriptorPool **ppDescPool)
	{
		VkDescriptorPoolSize descPoolSize[1];
		descPoolSize[0].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		descPoolSize[0].descriptorCount = 1;

		VkDescriptorPoolCreateInfo descPoolCreateInfo;
		descPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		descPoolCreateInfo.pNext = nullptr;
		descPoolCreateInfo.maxSets = 1;
		descPoolCreateInfo.poolSizeCount = 1;
		descPoolCreateInfo.pPoolSizes = descPoolSize;
		descPoolCreateInfo.flags = 0;
		VkDescriptorPool vkDescPool;
		vkCreateDescriptorPool(
			vkDevice,
			&descPoolCreateInfo,
			nullptr,
			&vkDescPool);

		*ppDescPool = new DescriptorPool(this, vkDescPool);

		return HResultSuccess;
	}
	HResult GpuDevice::CreateSwapChain(
		uint32 width,
		uint32 height,
		Surface *surface,
		SwapChain **ppSwapChain)
	{
		uint32 graphicsQueueFamilyIndex = UINT32_MAX,
			presentQueueFamilyIndex = UINT32_MAX;
		VkBool32 presentSupport;
		for (uint32 i = 0; i < familyProp.size(); i++)
		{
			if (familyProp[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
				graphicsQueueFamilyIndex = i;
			CheckReturnFail(vkGetPhysicalDeviceSurfaceSupportKHR(
				vkPhysicalDevice,
				i,
				surface->vkSurface,
				&presentSupport));
			if (presentSupport == VK_TRUE)
			{
				presentQueueFamilyIndex = i;
				if (graphicsQueueFamilyIndex == presentQueueFamilyIndex) break;
			}
		}
		if (graphicsQueueFamilyIndex == UINT32_MAX
			|| presentQueueFamilyIndex == UINT32_MAX)
			return HResultNoSuitableGPUQueue;

		VkQueue vkGraphicsQueue, vkPresentQueue;
		vkGetDeviceQueue(
			vkDevice,
			graphicsQueueFamilyIndex,
			0,
			&vkGraphicsQueue);
		vkGetDeviceQueue(
			vkDevice,
			presentQueueFamilyIndex,
			0,
			&vkPresentQueue);

		uint32 formatCount;
		CheckReturnFail(vkGetPhysicalDeviceSurfaceFormatsKHR(
			vkPhysicalDevice,
			surface->vkSurface,
			&formatCount,
			nullptr));
		if (formatCount == 0) return HResultSurfaceFormatNotFound;
		std::vector<VkSurfaceFormatKHR> surfaceFormats(formatCount);
		CheckReturnFail(vkGetPhysicalDeviceSurfaceFormatsKHR(
			vkPhysicalDevice,
			surface->vkSurface,
			&formatCount,
			surfaceFormats.data()));
		VkFormat vkFormat = surfaceFormats[0].format;
		if (formatCount == 1 && surfaceFormats[0].format == VK_FORMAT_UNDEFINED)
			vkFormat = VK_FORMAT_B8G8R8A8_UNORM;
		else
		{
			for (msize iter = 0; iter < surfaceFormats.size(); iter++)
				if (surfaceFormats[iter].format == VK_FORMAT_B8G8R8A8_UNORM)
				{
					vkFormat = VK_FORMAT_B8G8R8A8_UNORM;
					break;
				}
		}

		uint32 presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(
			vkPhysicalDevice,
			surface->vkSurface,
			&presentModeCount,
			nullptr);
		if (presentModeCount == 0) return HResultPresentModeNotFound;
		std::vector<VkPresentModeKHR> presentModes(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(
			vkPhysicalDevice,
			surface->vkSurface,
			&presentModeCount,
			presentModes.data());

		VkSurfaceCapabilitiesKHR surfaceCapabilities;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
			vkPhysicalDevice,
			surface->vkSurface,
			&surfaceCapabilities);
		VkExtent2D swapChainExtent;
		if (surfaceCapabilities.currentExtent.width == UINT32_MAX)
		{
			swapChainExtent.width = width;
			swapChainExtent.height = height;
			if (swapChainExtent.width < surfaceCapabilities.minImageExtent.width)
				swapChainExtent.width = surfaceCapabilities.minImageExtent.width;
			else if (swapChainExtent.width > surfaceCapabilities.maxImageExtent.width)
				swapChainExtent.width = surfaceCapabilities.maxImageExtent.width;
			if (swapChainExtent.height < surfaceCapabilities.minImageExtent.height)
				swapChainExtent.height = surfaceCapabilities.minImageExtent.height;
			else if (swapChainExtent.height > surfaceCapabilities.maxImageExtent.height)
				swapChainExtent.height = surfaceCapabilities.maxImageExtent.height;
		}
		else swapChainExtent = surfaceCapabilities.currentExtent;

		VkSurfaceTransformFlagBitsKHR preTransform;
		if (surfaceCapabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
			preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
		else preTransform = surfaceCapabilities.currentTransform;

		VkCompositeAlphaFlagBitsKHR compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		VkCompositeAlphaFlagBitsKHR compositeAlphaFlags[4] =
		{
			VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
			VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
			VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
			VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
		};
		for (uint32 i = 0; i < sizeof(compositeAlphaFlags); i++)
		{
			if (surfaceCapabilities.supportedCompositeAlpha & compositeAlphaFlags[i])
			{
				compositeAlpha = compositeAlphaFlags[i];
				break;
			}
		}

		VkSwapchainCreateInfoKHR vkSwapChainCreateInfo;
		vkSwapChainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		vkSwapChainCreateInfo.pNext = nullptr;
		vkSwapChainCreateInfo.surface = surface->vkSurface;
		vkSwapChainCreateInfo.minImageCount = surfaceCapabilities.minImageCount;
		vkSwapChainCreateInfo.imageFormat = vkFormat;
		vkSwapChainCreateInfo.imageExtent.width = swapChainExtent.width;
		vkSwapChainCreateInfo.imageExtent.height = swapChainExtent.height;
		vkSwapChainCreateInfo.preTransform = preTransform;
		vkSwapChainCreateInfo.compositeAlpha = compositeAlpha;
		vkSwapChainCreateInfo.imageArrayLayers = 1;
		vkSwapChainCreateInfo.presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR; //!!!
		vkSwapChainCreateInfo.oldSwapchain = VK_NULL_HANDLE;
		vkSwapChainCreateInfo.clipped = true;
		vkSwapChainCreateInfo.imageColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
		vkSwapChainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		vkSwapChainCreateInfo.flags = 0;
		uint32 queueFamilyIndices[2] = { graphicsQueueFamilyIndex, presentQueueFamilyIndex };
		if (graphicsQueueFamilyIndex == presentQueueFamilyIndex)
		{
			vkSwapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			vkSwapChainCreateInfo.queueFamilyIndexCount = 0;
			vkSwapChainCreateInfo.pQueueFamilyIndices = nullptr;
		}
		else
		{
			vkSwapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			vkSwapChainCreateInfo.queueFamilyIndexCount = 2;
			vkSwapChainCreateInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
		VkSwapchainKHR vkSwapChain;
		CheckReturnFail(vkCreateSwapchainKHR(
			vkDevice,
			&vkSwapChainCreateInfo,
			nullptr,
			&vkSwapChain));
		uint32 vkImageCount;
		CheckReturnFail(vkGetSwapchainImagesKHR(
			vkDevice,
			vkSwapChain,
			&vkImageCount,
			nullptr));
		std::vector<VkImage> vkImages(vkImageCount);
		std::vector<VkImageView> vkViews(vkImageCount);
		CheckReturnFail(vkGetSwapchainImagesKHR(
			vkDevice,
			vkSwapChain,
			&vkImageCount,
			vkImages.data()));

		for (uint32 i = 0; i < vkImageCount; i++)
		{
			VkImageViewCreateInfo viewCreateInfo;
			viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			viewCreateInfo.pNext = nullptr;
			viewCreateInfo.flags = 0;
			viewCreateInfo.image = vkImages[i];
			viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			viewCreateInfo.vkFormat = vkFormat;
			viewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_R;
			viewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_G;
			viewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_B;
			viewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_A;
			viewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			viewCreateInfo.subresourceRange.baseMipLevel = 0;
			viewCreateInfo.subresourceRange.levelCount = 1;
			viewCreateInfo.subresourceRange.baseArrayLayer = 0;
			viewCreateInfo.subresourceRange.layerCount = 1;
			CheckReturnFail(vkCreateImageView(
				vkDevice,
				&viewCreateInfo,
				nullptr,
				&vkViews[i]));
		}

		VkFormat vkDepthFormat = VK_FORMAT_D16_UNORM;
		VkImageCreateInfo imageCreateInfo;
		VkFormatProperties formatProps;
		vkGetPhysicalDeviceFormatProperties(
			vkPhysicalDevice,
			vkDepthFormat,
			&formatProps);
		if (formatProps.linearTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
			imageCreateInfo.tiling = VK_IMAGE_TILING_LINEAR;
		else imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;

		imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageCreateInfo.pNext = nullptr;
		imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
		imageCreateInfo.vkFormat = vkDepthFormat;
		imageCreateInfo.extent.width = swapChainExtent.width;
		imageCreateInfo.extent.height = swapChainExtent.height;
		imageCreateInfo.extent.depth = 1;
		imageCreateInfo.mipLevels = 1;
		imageCreateInfo.arrayLayers = 1;
		imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageCreateInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		imageCreateInfo.queueFamilyIndexCount = 0;
		imageCreateInfo.pQueueFamilyIndices = nullptr;
		imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageCreateInfo.flags = 0;
		VkImage vkDepthImage;
		CheckReturnFail(vkCreateImage(
			vkDevice,
			&imageCreateInfo,
			nullptr,
			&vkDepthImage));

		VkMemoryRequirements memReqs;
		vkGetImageMemoryRequirements(vkDevice, vkDepthImage, &memReqs);

		VkMemoryAllocateInfo memAlloc;
		memAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		memAlloc.pNext = nullptr;
		CheckReturn(VkGetMemoryTypeFromRequirements(
			vkPhysicalDevice,
			memReqs.memoryTypeBits,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			&memAlloc.memoryTypeIndex));
		memAlloc.allocationSize = memReqs.size;
		VkDeviceMemory vkDepthMemory;
		CheckReturnFail(vkAllocateMemory(
			vkDevice,
			&memAlloc,
			nullptr,
			&vkDepthMemory));
		CheckReturnFail(vkBindImageMemory(
			vkDevice,
			vkDepthImage,
			vkDepthMemory,
			0));

		VkImageViewCreateInfo viewCreateInfo;
		viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewCreateInfo.pNext = nullptr;
		viewCreateInfo.flags = 0;
		viewCreateInfo.image = vkDepthImage;
		viewCreateInfo.vkFormat = vkDepthFormat;
		viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_R;
		viewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_G;
		viewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_B;
		viewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_A;
		viewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		viewCreateInfo.subresourceRange.baseMipLevel = 0;
		viewCreateInfo.subresourceRange.levelCount = 1;
		viewCreateInfo.subresourceRange.baseArrayLayer = 0;
		viewCreateInfo.subresourceRange.layerCount = 1;
		VkImageView vkDepthView;
		CheckReturnFail(vkCreateImageView(
			vkDevice,
			&viewCreateInfo,
			nullptr,
			&vkDepthView));

		// Multisampling
		VkImage msaaColorImage;
		VkDeviceMemory msaaColorMemory;
		VkImageView msaaImageView;
		VkImage msaaDepthImage;
		VkDeviceMemory msaaDepthMemory;
		VkImageView msaaDepthView;
		VkGetAvailableSampleCount(vkDeviceProp, VK_SAMPLE_COUNT_8_BIT, &msaa);

		imageCreateInfo = {};
		imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageCreateInfo.pNext = nullptr;
		imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
		imageCreateInfo.vkFormat = vkSwapChainCreateInfo.imageFormat;
		imageCreateInfo.extent.width = width;
		imageCreateInfo.extent.height = height;
		imageCreateInfo.extent.depth = 1;
		imageCreateInfo.mipLevels = 1;
		imageCreateInfo.arrayLayers = 1;
		imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageCreateInfo.samples = msaa;
		imageCreateInfo.usage = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		CheckReturnFail(vkCreateImage(
			vkDevice,
			&imageCreateInfo,
			nullptr,
			&msaaColorImage));

		vkGetImageMemoryRequirements(vkDevice, msaaColorImage, &memReqs);
		CheckReturn(VkGetMemoryTypeFromRequirements(
			vkPhysicalDevice,
			memReqs.memoryTypeBits,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			&memAlloc.memoryTypeIndex));
		memAlloc.allocationSize = memReqs.size;

		CheckReturnFail(vkAllocateMemory(
			vkDevice,
			&memAlloc,
			nullptr,
			&msaaColorMemory));
		CheckReturnFail(vkBindImageMemory(
			vkDevice,
			msaaColorImage,
			msaaColorMemory,
			0));

		viewCreateInfo.image = msaaColorImage;
		viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewCreateInfo.vkFormat = vkSwapChainCreateInfo.imageFormat;
		viewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_R;
		viewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_G;
		viewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_B;
		viewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_A;
		viewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		viewCreateInfo.subresourceRange.levelCount = 1;
		viewCreateInfo.subresourceRange.layerCount = 1;
		CheckReturnFail(vkCreateImageView(
			vkDevice,
			&viewCreateInfo,
			nullptr,
			&msaaImageView));

		imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
		imageCreateInfo.vkFormat = vkDepthFormat;
		imageCreateInfo.extent.width = width;
		imageCreateInfo.extent.height = height;
		imageCreateInfo.extent.depth = 1;
		imageCreateInfo.mipLevels = 1;
		imageCreateInfo.arrayLayers = 1;
		imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageCreateInfo.samples = msaa;
		imageCreateInfo.usage = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		CheckReturnFail(vkCreateImage(
			vkDevice,
			&imageCreateInfo,
			nullptr,
			&msaaDepthImage));

		vkGetImageMemoryRequirements(
			vkDevice,
			msaaDepthImage,
			&memReqs);
		CheckReturn(VkGetMemoryTypeFromRequirements(
			vkPhysicalDevice,
			memReqs.memoryTypeBits,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			&memAlloc.memoryTypeIndex));
		memAlloc.allocationSize = memReqs.size;

		CheckReturnFail(vkAllocateMemory(
			vkDevice,
			&memAlloc,
			nullptr,
			&msaaDepthMemory));
		CheckReturnFail(vkBindImageMemory(
			vkDevice,
			msaaDepthImage,
			msaaDepthMemory,
			0));

		viewCreateInfo.image = msaaDepthImage;
		viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewCreateInfo.vkFormat = vkDepthFormat;
		viewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_R;
		viewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_G;
		viewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_B;
		viewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_A;
		viewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
		viewCreateInfo.subresourceRange.levelCount = 1;
		viewCreateInfo.subresourceRange.layerCount = 1;
		CheckReturnFail(vkCreateImageView(
			vkDevice,
			&viewCreateInfo,
			nullptr,
			&msaaDepthView));

		VkAttachmentDescription attachments[4];
		attachments[0].vkFormat = vkFormat;
		attachments[0].samples = msaa;
		attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachments[0].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		attachments[0].flags = 0;

		attachments[1].vkFormat = vkDepthFormat;
		attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
		attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachments[1].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		attachments[1].flags = 0;

		attachments[2].vkFormat = vkDepthFormat;
		attachments[2].samples = msaa;
		attachments[2].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachments[2].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachments[2].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachments[2].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachments[2].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachments[2].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		attachments[2].flags = 0;

		attachments[3].vkFormat = vkDepthFormat;
		attachments[3].samples = VK_SAMPLE_COUNT_1_BIT;
		attachments[3].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachments[3].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachments[3].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachments[3].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachments[3].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachments[3].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		attachments[3].flags = 0;

		VkAttachmentReference colorReference;
		colorReference.attachment = 0;
		colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		VkAttachmentReference depthReference;
		depthReference.attachment = 2;
		depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		VkAttachmentReference resolveReference;
		resolveReference.attachment = 1;
		resolveReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass;
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.flags = 0;
		subpass.inputAttachmentCount = 0;
		subpass.pInputAttachments = nullptr;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorReference;
		subpass.pResolveAttachments = &resolveReference;
		subpass.pDepthStencilAttachment = &depthReference;
		subpass.preserveAttachmentCount = 0;
		subpass.pPreserveAttachments = nullptr;

		VkSubpassDependency dependencies[2];
		dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[0].dstSubpass = 0;
		dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		dependencies[1].srcSubpass = 0;
		dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		VkRenderPassCreateInfo renderPassCreateInfo;
		renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassCreateInfo.pNext = nullptr;
		renderPassCreateInfo.attachmentCount = 4;
		renderPassCreateInfo.pAttachments = attachments;
		renderPassCreateInfo.subpassCount = 1;
		renderPassCreateInfo.pSubpasses = &subpass;
		renderPassCreateInfo.dependencyCount = 2;
		renderPassCreateInfo.pDependencies = dependencies;
		renderPassCreateInfo.flags = 0;
		VkRenderPass vkRenderPass;
		CheckReturnFail(vkCreateRenderPass(
			vkDevice,
			&renderPassCreateInfo,
			nullptr,
			&vkRenderPass));
		
		VkImageView imageViewAttachments[4];
		imageViewAttachments[0] = msaaImageView;
		imageViewAttachments[2] = msaaDepthView;
		imageViewAttachments[3] = vkDepthView;

		VkFramebufferCreateInfo fbCreateInfo;
		fbCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		fbCreateInfo.pNext = nullptr;
		fbCreateInfo.renderPass = vkRenderPass;
		fbCreateInfo.attachmentCount = 4;
		fbCreateInfo.pAttachments = imageViewAttachments;
		fbCreateInfo.width = vkSwapChainCreateInfo.imageExtent.width;
		fbCreateInfo.height = vkSwapChainCreateInfo.imageExtent.height;
		fbCreateInfo.layers = 1;
		fbCreateInfo.flags = 0;

		std::vector<VkFramebuffer> vkFramebuffers(vkImageCount);
		for (uint32 i = 0; i <vkImageCount; i++)
		{
			imageViewAttachments[1] = vkViews[i];
			vkCreateFramebuffer(
				vkDevice,
				&fbCreateInfo,
				nullptr,
				&vkFramebuffers[i]);
		}

		/*VkImage colorImage;
		VkDeviceMemory colorMemory;
		VkImageView imageView;
		VkImage depthImage;
		VkDeviceMemory depthMemory;
		VkImageView depthView;

		imageCreateInfo = {};
		imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageCreateInfo.pNext = nullptr;
		imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
		imageCreateInfo.vkFormat = vkSwapChainCreateInfo.imageFormat;
		imageCreateInfo.extent.width = width;
		imageCreateInfo.extent.height = height;
		imageCreateInfo.extent.depth = 1;
		imageCreateInfo.mipLevels = 1;
		imageCreateInfo.arrayLayers = 1;
		imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageCreateInfo.usage = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		CheckReturnFail(vkCreateImage(
			vkDevice,
			&imageCreateInfo,
			nullptr,
			&colorImage));

		vkGetImageMemoryRequirements(vkDevice, colorImage, &memReqs);
		CheckReturn(VkGetMemoryTypeFromRequirements(
			vkPhysicalDevice,
			memReqs.memoryTypeBits,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			&memAlloc.memoryTypeIndex));
		memAlloc.allocationSize = memReqs.size;

		CheckReturnFail(vkAllocateMemory(
			vkDevice,
			&memAlloc,
			nullptr,
			&colorMemory));
		CheckReturnFail(vkBindImageMemory(
			vkDevice,
			colorImage,
			colorMemory,
			0));

		viewCreateInfo.image = colorImage;
		viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewCreateInfo.vkFormat = vkSwapChainCreateInfo.imageFormat;
		viewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_R;
		viewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_G;
		viewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_B;
		viewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_A;
		viewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		viewCreateInfo.subresourceRange.levelCount = 1;
		viewCreateInfo.subresourceRange.layerCount = 1;
		CheckReturnFail(vkCreateImageView(
			vkDevice,
			&viewCreateInfo,
			nullptr,
			&imageView));

		imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
		imageCreateInfo.vkFormat = vkDepthFormat;
		imageCreateInfo.extent.width = width;
		imageCreateInfo.extent.height = height;
		imageCreateInfo.extent.depth = 1;
		imageCreateInfo.mipLevels = 1;
		imageCreateInfo.arrayLayers = 1;
		imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageCreateInfo.usage = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		CheckReturnFail(vkCreateImage(
			vkDevice,
			&imageCreateInfo,
			nullptr,
			&depthImage));

		vkGetImageMemoryRequirements(
			vkDevice,
			depthImage,
			&memReqs);
		CheckReturn(VkGetMemoryTypeFromRequirements(
			vkPhysicalDevice,
			memReqs.memoryTypeBits,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			&memAlloc.memoryTypeIndex));
		memAlloc.allocationSize = memReqs.size;

		CheckReturnFail(vkAllocateMemory(
			vkDevice,
			&memAlloc,
			nullptr,
			&depthMemory));
		CheckReturnFail(vkBindImageMemory(
			vkDevice,
			depthImage,
			depthMemory,
			0));

		viewCreateInfo.image = depthImage;
		viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewCreateInfo.vkFormat = vkDepthFormat;
		viewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_R;
		viewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_G;
		viewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_B;
		viewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_A;
		viewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
		viewCreateInfo.subresourceRange.levelCount = 1;
		viewCreateInfo.subresourceRange.layerCount = 1;
		CheckReturnFail(vkCreateImageView(
			vkDevice,
			&viewCreateInfo,
			nullptr,
			&depthView));

		VkAttachmentDescription attachments[2];
		attachments[0].vkFormat = vkFormat;
		attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
		attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		attachments[0].flags = 0;

		attachments[1].vkFormat = vkDepthFormat;
		attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
		attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		attachments[1].flags = 0;

		VkAttachmentReference colorReference;
		colorReference.attachment = 0;
		colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		VkAttachmentReference depthReference;
		depthReference.attachment = 1;
		depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass;
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.flags = 0;
		subpass.inputAttachmentCount = 0;
		subpass.pInputAttachments = nullptr;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorReference;
		subpass.pResolveAttachments = nullptr;
		subpass.pDepthStencilAttachment = &depthReference;
		subpass.preserveAttachmentCount = 0;
		subpass.pPreserveAttachments = nullptr;

		VkRenderPassCreateInfo renderPassCreateInfo;
		renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassCreateInfo.pNext = nullptr;
		renderPassCreateInfo.attachmentCount = 2;
		renderPassCreateInfo.pAttachments = attachments;
		renderPassCreateInfo.subpassCount = 1;
		renderPassCreateInfo.pSubpasses = &subpass;
		renderPassCreateInfo.dependencyCount = 0;
		renderPassCreateInfo.pDependencies = nullptr;
		renderPassCreateInfo.flags = 0;
		VkRenderPass vkRenderPass;
		CheckReturnFail(vkCreateRenderPass(
			vkDevice,
			&renderPassCreateInfo,
			nullptr,
			&vkRenderPass));
		
		VkImageView imageViewAttachments[2];
		imageViewAttachments[0] = imageView;
		imageViewAttachments[1] = depthView;

		VkFramebufferCreateInfo fbCreateInfo;
		fbCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		fbCreateInfo.pNext = nullptr;
		fbCreateInfo.renderPass = vkRenderPass;
		fbCreateInfo.attachmentCount = 2;
		fbCreateInfo.pAttachments = imageViewAttachments;
		fbCreateInfo.width = vkSwapChainCreateInfo.imageExtent.width;
		fbCreateInfo.height = vkSwapChainCreateInfo.imageExtent.height;
		fbCreateInfo.layers = 1;
		fbCreateInfo.flags = 0;

		std::vector<VkFramebuffer> vkFramebuffers(vkImageCount);
		for (uint32 i = 0; i < vkImageCount; i++)
		{
			imageViewAttachments[0] = vkViews[i];
			vkCreateFramebuffer(
				vkDevice,
				&fbCreateInfo,
				nullptr,
				&vkFramebuffers[i]);
		}*/

		uint32 currentBuffer = 0;
		CheckReturnFail(vkAcquireNextImageKHR(
			vkDevice,
			vkSwapChain,
			UINT64_MAX,
			VK_NULL_HANDLE,
			VK_NULL_HANDLE,
			&currentBuffer));
		vkDeviceWaitIdle(vkDevice);

		*ppSwapChain = new SwapChain(
			this,
			vkSwapChainCreateInfo,
			vkSwapChain,
			vkGraphicsQueue,
			vkPresentQueue,
			vkFormat,
			vkImageCount,
			vkImages,
			vkViews,
			vkDepthFormat,
			vkDepthImage,
			vkDepthView,
			vkDepthMemory,
			vkRenderPass,
			vkFramebuffers,
			currentBuffer);

		return HResultSuccess;
	}
	HResult GpuDevice::CreateShader(
		uint32 *code,
		uint32 size,
		Shader **ppShader)
	{
		VkShaderModuleCreateInfo moduleCreateInfo;
		moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		moduleCreateInfo.pNext = nullptr;
		moduleCreateInfo.flags = 0;
		moduleCreateInfo.codeSize = size;
		moduleCreateInfo.pCode = code;
		VkShaderModule vkShader;
		CheckReturnFail(vkCreateShaderModule(
			vkDevice,
			&moduleCreateInfo,
			nullptr,
			&vkShader));

		*ppShader = new Shader(this, vkShader);

		return HResultSuccess;
	}
	HResult GpuDevice::CreatePipeline(
		SwapChain* swapChain,
		Shader *vertexShader,
		Shader *fragmentShader,
		Pipeline **ppPipeline)
	{
		VkDynamicState dynamicStateEnables[VK_DYNAMIC_STATE_RANGE_SIZE];
		VkPipelineDynamicStateCreateInfo dynamicState;
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.pNext = nullptr;
		dynamicState.pDynamicStates = dynamicStateEnables;
		dynamicState.dynamicStateCount = 0;
		dynamicState.flags = 0;

		VkVertexInputBindingDescription viBindDesc;
		viBindDesc.binding = 0;
		viBindDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		viBindDesc.stride = 32;

		VkVertexInputAttributeDescription viAttrDesc[2];
		viAttrDesc[0].binding = 0;
		viAttrDesc[0].location = 0;
		viAttrDesc[0].vkFormat = VK_FORMAT_R32G32B32A32_SFLOAT;
		viAttrDesc[0].offset = 0;
		viAttrDesc[1].binding = 0;
		viAttrDesc[1].location = 1;
		viAttrDesc[1].vkFormat = VK_FORMAT_R32G32B32A32_SFLOAT;
		viAttrDesc[1].offset = 16;

		VkPipelineVertexInputStateCreateInfo vi;
		vi.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vi.pNext = nullptr;
		vi.flags = 0;
		vi.vertexBindingDescriptionCount = 1;
		vi.pVertexBindingDescriptions = &viBindDesc;
		vi.vertexAttributeDescriptionCount = 2;
		vi.pVertexAttributeDescriptions = viAttrDesc;

		VkPipelineInputAssemblyStateCreateInfo ia;
		ia.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		ia.pNext = nullptr;
		ia.flags = 0;
		ia.primitiveRestartEnable = VK_FALSE;
		ia.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

		VkPipelineRasterizationStateCreateInfo rs;
		rs.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rs.pNext = nullptr;
		rs.flags = 0;
		rs.polygonMode = VK_POLYGON_MODE_FILL;
		rs.cullMode = VK_CULL_MODE_BACK_BIT;
		rs.frontFace = VK_FRONT_FACE_CLOCKWISE;
		rs.depthClampEnable = VK_FALSE;
		rs.rasterizerDiscardEnable = VK_FALSE;
		rs.depthBiasEnable = VK_FALSE;
		rs.depthBiasConstantFactor = 0;
		rs.depthBiasClamp = 0;
		rs.depthBiasSlopeFactor = 0;
		rs.lineWidth = 1.0f;

		VkPipelineColorBlendStateCreateInfo cb;
		cb.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		cb.pNext = nullptr;
		cb.flags = 0;
		VkPipelineColorBlendAttachmentState attState[1];
		attState[0].colorWriteMask = VK_COLOR_COMPONENT_R_BIT
			| VK_COLOR_COMPONENT_G_BIT
			| VK_COLOR_COMPONENT_B_BIT
			| VK_COLOR_COMPONENT_A_BIT;
		attState[0].blendEnable = VK_TRUE;
		attState[0].colorBlendOp = VK_BLEND_OP_ADD;
		attState[0].srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		attState[0].dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		attState[0].alphaBlendOp = VK_BLEND_OP_MAX;
		attState[0].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		attState[0].dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		cb.attachmentCount = 1;
		cb.pAttachments = attState;
		cb.logicOpEnable = VK_FALSE;
		cb.logicOp = VK_LOGIC_OP_NO_OP;
		cb.blendConstants[0] = 0.0f;
		cb.blendConstants[1] = 0.0f;
		cb.blendConstants[2] = 0.0f;
		cb.blendConstants[3] = 0.0f;

		VkPipelineViewportStateCreateInfo vp;
		vp.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		vp.pNext = nullptr;
		vp.flags = 0;
		vp.viewportCount = 1;
		dynamicStateEnables[dynamicState.dynamicStateCount++] = VK_DYNAMIC_STATE_VIEWPORT;
		vp.scissorCount = 1;
		dynamicStateEnables[dynamicState.dynamicStateCount++] = VK_DYNAMIC_STATE_SCISSOR;
		vp.pScissors = nullptr;
		vp.pViewports = nullptr;

		VkPipelineDepthStencilStateCreateInfo ds;
		ds.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		ds.pNext = nullptr;
		ds.flags = 0;
		ds.depthTestEnable = VK_TRUE;
		ds.depthWriteEnable = VK_TRUE;
		ds.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
		ds.depthBoundsTestEnable = VK_FALSE;
		ds.minDepthBounds = 0;
		ds.maxDepthBounds = 0;
		ds.stencilTestEnable = VK_FALSE;
		ds.back.failOp = VK_STENCIL_OP_KEEP;
		ds.back.passOp = VK_STENCIL_OP_KEEP;
		ds.back.compareOp = VK_COMPARE_OP_ALWAYS;
		ds.back.compareMask = 0;
		ds.back.reference = 0;
		ds.back.depthFailOp = VK_STENCIL_OP_KEEP;
		ds.back.writeMask = 0;
		ds.front = ds.back;

		VkPipelineMultisampleStateCreateInfo ms;
		ms.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		ms.pNext = nullptr;
		ms.flags = 0;
		ms.pSampleMask = nullptr;
		ms.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		ms.sampleShadingEnable = VK_FALSE;
		ms.alphaToCoverageEnable = VK_FALSE;
		ms.alphaToOneEnable = VK_FALSE;
		ms.minSampleShading = 0.0;

		VkDescriptorSetLayoutBinding layoutBinding;
		layoutBinding.binding = 0;
		layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		layoutBinding.descriptorCount = 1;
		layoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		layoutBinding.pImmutableSamplers = nullptr;

		VkGraphicsPipelineCreateInfo pipelineCreateInfo;
		pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineCreateInfo.pNext = nullptr;
		pipelineCreateInfo.layout = vkPipelineLayout;
		pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
		pipelineCreateInfo.basePipelineIndex = 0;
		pipelineCreateInfo.flags = 0;
		pipelineCreateInfo.pVertexInputState = &vi;
		pipelineCreateInfo.pInputAssemblyState = &ia;
		pipelineCreateInfo.pRasterizationState = &rs;
		pipelineCreateInfo.pColorBlendState = &cb;
		pipelineCreateInfo.pTessellationState = nullptr;
		pipelineCreateInfo.pMultisampleState = &ms;
		pipelineCreateInfo.pDynamicState = &dynamicState;
		pipelineCreateInfo.pViewportState = &vp;
		pipelineCreateInfo.pDepthStencilState = &ds;
		VkPipelineShaderStageCreateInfo shaderStages[2];
		shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStages[0].pNext = nullptr;
		shaderStages[0].pSpecializationInfo = nullptr;
		shaderStages[0].flags = 0;
		shaderStages[0].module = vertexShader->vkShader;
		shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
		shaderStages[0].pName = "main";
		shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStages[1].pNext = nullptr;
		shaderStages[1].pSpecializationInfo = nullptr;
		shaderStages[1].flags = 0;
		shaderStages[1].module = fragmentShader->vkShader;
		shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		shaderStages[1].pName = "main";
		pipelineCreateInfo.pStages = shaderStages;
		pipelineCreateInfo.stageCount = 2;
		pipelineCreateInfo.renderPass = swapChain->vkRenderPass;
		pipelineCreateInfo.subpass = 0;
		VkPipeline vkPipeline;
		CheckReturnFail(vkCreateGraphicsPipelines(
			vkDevice,
			VK_NULL_HANDLE,
			1,
			&pipelineCreateInfo,
			nullptr,
			&vkPipeline));

		*ppPipeline = new Pipeline(
			this,
			swapChain,
			vkPipelineLayout,
			vkPipeline);

		return HResultSuccess;
	}
	HResult GpuDevice::CreateRenderTargetPipelines(
		SwapChain *swapChain,
		Shader *vertexShader,
		Shader *fragmentShader,
		Pipeline **ppPipeline)
	{
		VkDynamicState dynamicStateEnables[VK_DYNAMIC_STATE_RANGE_SIZE];
		VkPipelineDynamicStateCreateInfo dynamicState;
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.pNext = nullptr;
		dynamicState.pDynamicStates = dynamicStateEnables;
		dynamicState.dynamicStateCount = 0;
		dynamicState.flags = 0;

		VkVertexInputBindingDescription viBindDesc;
		viBindDesc.binding = 0;
		viBindDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		viBindDesc.stride = sizeof(Vector2f);

		VkVertexInputAttributeDescription viAttrDesc[2];
		viAttrDesc[0].binding = 0;
		viAttrDesc[0].location = 0;
		viAttrDesc[0].vkFormat = VK_FORMAT_R32G32_SFLOAT;
		viAttrDesc[0].offset = 0;

		VkPipelineVertexInputStateCreateInfo vi;
		vi.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vi.pNext = nullptr;
		vi.flags = 0;
		vi.vertexBindingDescriptionCount = 1;
		vi.pVertexBindingDescriptions = &viBindDesc;
		vi.vertexAttributeDescriptionCount = 1;
		vi.pVertexAttributeDescriptions = viAttrDesc;

		VkPipelineInputAssemblyStateCreateInfo ia;
		ia.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		ia.pNext = nullptr;
		ia.flags = 0;
		ia.primitiveRestartEnable = VK_FALSE;
		ia.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;

		VkPipelineRasterizationStateCreateInfo rs;
		rs.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rs.pNext = nullptr;
		rs.flags = 0;
		rs.polygonMode = VK_POLYGON_MODE_FILL;
		rs.cullMode = VK_CULL_MODE_NONE;
		rs.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rs.depthClampEnable = VK_FALSE;
		rs.rasterizerDiscardEnable = VK_FALSE;
		rs.depthBiasEnable = VK_FALSE;
		rs.depthBiasConstantFactor = 0;
		rs.depthBiasClamp = 0;
		rs.depthBiasSlopeFactor = 0;
		rs.lineWidth = 1.0f;

		VkPipelineColorBlendStateCreateInfo cb;
		cb.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		cb.pNext = nullptr;
		cb.flags = 0;
		VkPipelineColorBlendAttachmentState attState[1];
		attState[0].colorWriteMask = VK_COLOR_COMPONENT_R_BIT
			| VK_COLOR_COMPONENT_G_BIT
			| VK_COLOR_COMPONENT_B_BIT
			| VK_COLOR_COMPONENT_A_BIT;
		attState[0].blendEnable = VK_TRUE;
		attState[0].colorBlendOp = VK_BLEND_OP_ADD;
		attState[0].srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		attState[0].dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		attState[0].alphaBlendOp = VK_BLEND_OP_MAX;
		attState[0].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		attState[0].dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		cb.attachmentCount = 1;
		cb.pAttachments = attState;
		cb.logicOpEnable = VK_FALSE;
		cb.logicOp = VK_LOGIC_OP_NO_OP;
		cb.blendConstants[0] = 0.0f;
		cb.blendConstants[1] = 0.0f;
		cb.blendConstants[2] = 0.0f;
		cb.blendConstants[3] = 0.0f;

		VkPipelineViewportStateCreateInfo vp;
		vp.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		vp.pNext = nullptr;
		vp.flags = 0;
		vp.viewportCount = 1;
		dynamicStateEnables[dynamicState.dynamicStateCount++] = VK_DYNAMIC_STATE_VIEWPORT;
		vp.scissorCount = 1;
		dynamicStateEnables[dynamicState.dynamicStateCount++] = VK_DYNAMIC_STATE_SCISSOR;
		vp.pScissors = nullptr;
		vp.pViewports = nullptr;

		VkPipelineDepthStencilStateCreateInfo ds;
		ds.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		ds.pNext = nullptr;
		ds.flags = 0;
		ds.depthTestEnable = VK_FALSE;
		ds.depthWriteEnable = VK_FALSE;
		ds.depthCompareOp = VK_COMPARE_OP_NEVER;
		ds.depthBoundsTestEnable = VK_FALSE;
		ds.minDepthBounds = 0;
		ds.maxDepthBounds = 0;
		ds.stencilTestEnable = VK_FALSE;
		ds.back.failOp = VK_STENCIL_OP_KEEP;
		ds.back.passOp = VK_STENCIL_OP_KEEP;
		ds.back.compareOp = VK_COMPARE_OP_ALWAYS;
		ds.back.compareMask = 0;
		ds.back.reference = 0;
		ds.back.depthFailOp = VK_STENCIL_OP_KEEP;
		ds.back.writeMask = 0;
		ds.front = ds.back;

		VkPipelineMultisampleStateCreateInfo ms;
		ms.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		ms.pNext = nullptr;
		ms.flags = 0;
		ms.pSampleMask = nullptr;
		ms.rasterizationSamples = msaa;
		ms.sampleShadingEnable = VK_TRUE;
		ms.alphaToCoverageEnable = VK_FALSE;
		ms.alphaToOneEnable = VK_FALSE;
		ms.minSampleShading = 1.0f;

		VkGraphicsPipelineCreateInfo pipelineCreateInfo;
		pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineCreateInfo.pNext = nullptr;
		pipelineCreateInfo.layout = vkPipelineLayout;
		pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
		pipelineCreateInfo.basePipelineIndex = 0;
		pipelineCreateInfo.flags = 0;
		pipelineCreateInfo.pVertexInputState = &vi;
		pipelineCreateInfo.pInputAssemblyState = &ia;
		pipelineCreateInfo.pRasterizationState = &rs;
		pipelineCreateInfo.pColorBlendState = &cb;
		pipelineCreateInfo.pTessellationState = nullptr;
		pipelineCreateInfo.pMultisampleState = &ms;
		pipelineCreateInfo.pDynamicState = &dynamicState;
		pipelineCreateInfo.pViewportState = &vp;
		pipelineCreateInfo.pDepthStencilState = &ds;
		VkPipelineShaderStageCreateInfo shaderStages[2];
		shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStages[0].pNext = nullptr;
		shaderStages[0].pSpecializationInfo = nullptr;
		shaderStages[0].flags = 0;
		shaderStages[0].module = vertexShader->vkShader;
		shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
		shaderStages[0].pName = "main";
		shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStages[1].pNext = nullptr;
		shaderStages[1].pSpecializationInfo = nullptr;
		shaderStages[1].flags = 0;
		shaderStages[1].module = fragmentShader->vkShader;
		shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		shaderStages[1].pName = "main";
		pipelineCreateInfo.pStages = shaderStages;
		pipelineCreateInfo.stageCount = 2;
		pipelineCreateInfo.renderPass = swapChain->vkRenderPass;
		pipelineCreateInfo.subpass = 0;

		VkPipeline vkPipeline;
		CheckReturnFail(vkCreateGraphicsPipelines(
			vkDevice,
			VK_NULL_HANDLE,
			1,
			&pipelineCreateInfo,
			nullptr,
			&vkPipeline));

		*ppPipeline = new Pipeline(
			this,
			swapChain,
			vkPipelineLayout,
			vkPipeline);

		return HResultSuccess;
	}
	HResult GpuDevice::CreateBuffer(
		VkBufferUsageFlags vkBufferType,
		uint32 memoryTypeBits,
		msize size,
		Buffer **ppBuffer)
	{
		VkBuffer vkBuffer;
		VkBufferCreateInfo vkBufferInfo;
		vkBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		vkBufferInfo.pNext = nullptr;
		vkBufferInfo.usage = vkBufferType;
		vkBufferInfo.size = (VkDeviceSize)size;
		vkBufferInfo.pQueueFamilyIndices = nullptr;
		vkBufferInfo.queueFamilyIndexCount = 0;
		vkBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		vkBufferInfo.flags = 0;
		CheckReturnFail(vkCreateBuffer(
			vkDevice,
			&vkBufferInfo,
			nullptr,
			&vkBuffer));

		VkDeviceMemory vkBufferMemory;
		VkMemoryRequirements vkMemReqs;
		vkGetBufferMemoryRequirements(vkDevice, vkBuffer, &vkMemReqs);
		VkMemoryAllocateInfo allocInfo;
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.pNext = nullptr;
		CheckReturn(VkGetMemoryTypeFromRequirements(
			vkPhysicalDevice,
			vkMemReqs.memoryTypeBits,
			memoryTypeBits,
			&allocInfo.memoryTypeIndex));
		allocInfo.allocationSize = vkMemReqs.size;
		CheckReturnFail(vkAllocateMemory(
			vkDevice,
			&allocInfo,
			nullptr,
			&vkBufferMemory));
		CheckReturnFail(vkBindBufferMemory(
			vkDevice,
			vkBuffer,
			vkBufferMemory,
			0));

		*ppBuffer = new Buffer(
			this,
			vkBuffer,
			vkBufferInfo,
			memoryTypeBits,
			vkBufferMemory,
			vkMemReqs.size);

		return HResultSuccess;
	}
	HResult GpuDevice::CreateRenderTarget(
		SwapChain *swapChain,
		CommandBuffer *cmdBuffer,
		RenderTarget **ppRenderTarget)
	{
		
		Shader *vertexShader, *fragmentShader;
		std::vector<int8> vertexSPV, fragmentSPV;
		std::fstream file;
		file.open(L"shaders\\general vertex shader.spv", std::ios::in | std::ios::binary);
		file.seekg(0, std::ios::end);
		vertexSPV.resize(file.tellg());
		file.seekg(0, std::ios::beg);
		file.read(vertexSPV.data(), vertexSPV.size());
		file.close();
		file.open(L"shaders\\general fragment shader.spv", std::ios::in | std::ios::binary);
		file.seekg(0, std::ios::end);
		fragmentSPV.resize(file.tellg());
		file.seekg(0, std::ios::beg);
		file.read(fragmentSPV.data(), fragmentSPV.size());
		file.close();
		CreateShader(
			reinterpret_cast<uint32 *>(vertexSPV.data()),
			vertexSPV.size(),
			&vertexShader);
		CreateShader(
			reinterpret_cast<uint32 *>(fragmentSPV.data()),
			fragmentSPV.size(),
			&fragmentShader);

		Pipeline *pipeline;
		CheckReturn(CreateRenderTargetPipelines(
			swapChain,
			vertexShader,
			fragmentShader,
			&pipeline));

		*ppRenderTarget = new RenderTarget(
			swapChain,
			vertexBuffer,
			cmdBuffer,
			pipeline);

		return HResultSuccess;
	}
	HResult GpuDevice::CreateBitmap(
		uint32 width,
		uint32 height,
		Bitmap **ppBitmap)
	{
		*ppBitmap = new Bitmap(this, width, height);

		return HResultSuccess;
	}
	HResult GpuDevice::CreateGradientCollection(
		GradientStop *stops,
		uint32 count,
		GradientCollection **ppGradientCollection)
	{
		msize memOffset = AllocateMemory(count * sizeof(GradientStop));
		void *mapped;
		MapMemory(memOffset, count * sizeof(GradientStop), &mapped);
		memcpy(mapped, stops, count * sizeof(GradientStop));
		UnmapMemory();
		*ppGradientCollection = new GradientCollection(this, memOffset, count);
		return HResultSuccess;
	}
}
