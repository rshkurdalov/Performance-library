// Copyright (c) 2017-2018, Roman Shkurdalov
// This file is under The Clear BSD License, see LICENSE.txt

#include "gpu\SwapChain.h"
#include "gpu\Surface.h"
#include "gpu\GpuDevice.h"
#include "gpu\CommandBuffer.h"
#include "gpu\Pipeline.h"

namespace gpu
{
	SwapChain::SwapChain(
		GpuDevice *device,
		Surface *surface,
		VkSwapchainCreateInfoKHR &vkSwapChainCreateInfo,
		VkSwapchainKHR vkSwapChain,
		VkQueue vkGraphicsQueue,
		VkQueue vkPresentQueue,
		VkFormat vkFormat,
		uint32 vkImageCount,
		std::vector<VkImage> &vkImages,
		std::vector<VkImageView> &vkViews,
		VkFormat vkDepthFormat,
		VkImage vkDepthImage,
		VkImageView vkDepthView,
		VkDeviceMemory vkDepthMemory,
		VkImage msaaColorImage,
		VkDeviceMemory msaaColorMemory,
		VkImageView msaaImageView,
		VkImage msaaDepthImage,
		VkDeviceMemory msaaDepthMemory,
		VkImageView msaaDepthView,
		VkRenderPass vkRenderPass,
		std::vector<VkFramebuffer> vkFramebuffers,
		uint32 currentBuffer)
	{
		device->AddRef();
		this->device = device;
		surface->AddRef();
		this->surface = surface;
		this->vkSwapChainCreateInfo = vkSwapChainCreateInfo;
		this->vkSwapChain = vkSwapChain;
		this->vkGraphicsQueue = vkGraphicsQueue;
		this->vkPresentQueue = vkPresentQueue;
		this->vkFormat = vkFormat;
		this->imageCount = vkImageCount;
		this->vkImages = vkImages;
		this->vkViews = vkViews;
		this->vkDepthFormat = vkDepthFormat;
		this->vkDepthImage = vkDepthImage;
		this->vkDepthView = vkDepthView;
		this->vkDepthMemory = vkDepthMemory;
		this->msaaColorImage = msaaColorImage;
		this->msaaColorMemory = msaaColorMemory;
		this->msaaImageView = msaaImageView;
		this->msaaDepthImage = msaaDepthImage;
		this->msaaDepthMemory = msaaDepthMemory;
		this->msaaDepthView = msaaDepthView;
		this->vkRenderPass = vkRenderPass;
		this->vkFramebuffers = vkFramebuffers;
		this->currentBuffer = currentBuffer;
	}
	SwapChain::~SwapChain()
	{
		for (uint32 i = 0; i < imageCount; i++)
			vkDestroyFramebuffer(device->vkDevice, vkFramebuffers[i], nullptr);
		vkDestroyRenderPass(device->vkDevice, vkRenderPass, nullptr);
		vkDestroyImageView(device->vkDevice, msaaDepthView, nullptr);
		vkFreeMemory(device->vkDevice, msaaDepthMemory, nullptr);
		vkDestroyImage(device->vkDevice, msaaDepthImage, nullptr);
		vkFreeMemory(device->vkDevice, msaaColorMemory, nullptr);
		vkDestroyImage(device->vkDevice, msaaColorImage, nullptr);
		vkDestroyImageView(device->vkDevice, vkDepthView, nullptr);
		vkDestroyImage(device->vkDevice, vkDepthImage, nullptr);
		vkFreeMemory(device->vkDevice, vkDepthMemory, nullptr);
		for (uint32 i = 0; i < imageCount; i++)
			vkDestroyImageView(device->vkDevice, vkViews[i], nullptr);
		vkDestroySwapchainKHR(device->vkDevice, vkSwapChain, nullptr);
		surface->Unref();
		device->Unref();
	}
	uint32 SwapChain::GetWidth()
	{
		return vkSwapChainCreateInfo.imageExtent.width;
	}
	uint32 SwapChain::GetHeight()
	{
		return vkSwapChainCreateInfo.imageExtent.height;
	}
	uint32 SwapChain::GetCurrentBuffer()
	{
		return currentBuffer;
	}
	HResult SwapChain::Resize(uint32 width, uint32 height)
	{
		for (uint32 i = 0; i < imageCount; i++)
			vkDestroyFramebuffer(device->vkDevice, vkFramebuffers[i], nullptr);
		vkDestroyRenderPass(device->vkDevice, vkRenderPass, nullptr);
		vkDestroyImageView(device->vkDevice, msaaDepthView, nullptr);
		vkFreeMemory(device->vkDevice, msaaDepthMemory, nullptr);
		vkDestroyImage(device->vkDevice, msaaDepthImage, nullptr);
		vkFreeMemory(device->vkDevice, msaaColorMemory, nullptr);
		vkDestroyImage(device->vkDevice, msaaColorImage, nullptr);
		vkDestroyImageView(device->vkDevice, vkDepthView, nullptr);
		vkDestroyImage(device->vkDevice, vkDepthImage, nullptr);
		vkFreeMemory(device->vkDevice, vkDepthMemory, nullptr);
		for (uint32 i = 0; i < imageCount; i++)
			vkDestroyImageView(device->vkDevice, vkViews[i], nullptr);

		VkSurfaceCapabilitiesKHR surfaceCapabilities;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
			device->vkPhysicalDevice,
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
		vkSwapChainCreateInfo.imageExtent = swapChainExtent;
		vkSwapChainCreateInfo.oldSwapchain = vkSwapChain;
		VkSwapchainKHR vkNewSwapChain;
		CheckReturnFail(vkCreateSwapchainKHR(
			device->vkDevice,
			&vkSwapChainCreateInfo,
			nullptr,
			&vkNewSwapChain));
		vkDestroySwapchainKHR(device->vkDevice, vkSwapChain, nullptr);
		vkSwapChain = vkNewSwapChain;

		uint32 vkImageCount = vkImages.size();
		CheckReturnFail(vkGetSwapchainImagesKHR(
			device->vkDevice,
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
			viewCreateInfo.format = vkFormat;
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
				device->vkDevice,
				&viewCreateInfo,
				nullptr,
				&vkViews[i]));
		}

		VkFormat vkDepthFormat = VK_FORMAT_D16_UNORM;
		VkImageCreateInfo imageCreateInfo;
		VkFormatProperties formatProps;
		vkGetPhysicalDeviceFormatProperties(
			device->vkPhysicalDevice,
			vkDepthFormat,
			&formatProps);
		if (formatProps.linearTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
			imageCreateInfo.tiling = VK_IMAGE_TILING_LINEAR;
		else imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;

		imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageCreateInfo.pNext = nullptr;
		imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
		imageCreateInfo.format = vkDepthFormat;
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
		CheckReturnFail(vkCreateImage(
			device->vkDevice,
			&imageCreateInfo,
			nullptr,
			&vkDepthImage));

		VkMemoryRequirements memReqs;
		vkGetImageMemoryRequirements(device->vkDevice, vkDepthImage, &memReqs);

		VkMemoryAllocateInfo memAlloc;
		memAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		memAlloc.pNext = nullptr;
		if (!device->GetMemoryTypeFromRequirements(
			device->vkPhysicalDevice,
			memReqs.memoryTypeBits,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			&memAlloc.memoryTypeIndex))
			return HResultFail;
		memAlloc.allocationSize = memReqs.size;
		CheckReturnFail(vkAllocateMemory(
			device->vkDevice,
			&memAlloc,
			nullptr,
			&vkDepthMemory));
		CheckReturnFail(vkBindImageMemory(
			device->vkDevice,
			vkDepthImage,
			vkDepthMemory,
			0));

		VkImageViewCreateInfo viewCreateInfo;
		viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewCreateInfo.pNext = nullptr;
		viewCreateInfo.flags = 0;
		viewCreateInfo.image = vkDepthImage;
		viewCreateInfo.format = vkDepthFormat;
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
		CheckReturnFail(vkCreateImageView(
			device->vkDevice,
			&viewCreateInfo,
			nullptr,
			&vkDepthView));

		VkSampleCountFlagBits msaa = VK_SAMPLE_COUNT_8_BIT;

		imageCreateInfo = {};
		imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageCreateInfo.pNext = nullptr;
		imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
		imageCreateInfo.format = vkSwapChainCreateInfo.imageFormat;
		imageCreateInfo.extent.width = swapChainExtent.width;
		imageCreateInfo.extent.height = swapChainExtent.height;
		imageCreateInfo.extent.depth = 1;
		imageCreateInfo.mipLevels = 1;
		imageCreateInfo.arrayLayers = 1;
		imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageCreateInfo.samples = msaa;
		imageCreateInfo.usage = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		CheckReturnFail(vkCreateImage(
			device->vkDevice,
			&imageCreateInfo,
			nullptr,
			&msaaColorImage));

		vkGetImageMemoryRequirements(device->vkDevice, msaaColorImage, &memReqs);
		if (!device->GetMemoryTypeFromRequirements(
			device->vkPhysicalDevice,
			memReqs.memoryTypeBits,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			&memAlloc.memoryTypeIndex))
			return HResultFail;
		memAlloc.allocationSize = memReqs.size;

		CheckReturnFail(vkAllocateMemory(
			device->vkDevice,
			&memAlloc,
			nullptr,
			&msaaColorMemory));
		CheckReturnFail(vkBindImageMemory(
			device->vkDevice,
			msaaColorImage,
			msaaColorMemory,
			0));

		viewCreateInfo.image = msaaColorImage;
		viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewCreateInfo.format = vkSwapChainCreateInfo.imageFormat;
		viewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_R;
		viewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_G;
		viewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_B;
		viewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_A;
		viewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		viewCreateInfo.subresourceRange.levelCount = 1;
		viewCreateInfo.subresourceRange.layerCount = 1;
		CheckReturnFail(vkCreateImageView(
			device->vkDevice,
			&viewCreateInfo,
			nullptr,
			&msaaImageView));

		imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
		imageCreateInfo.format = vkDepthFormat;
		imageCreateInfo.extent.width = swapChainExtent.width;
		imageCreateInfo.extent.height = swapChainExtent.height;
		imageCreateInfo.extent.depth = 1;
		imageCreateInfo.mipLevels = 1;
		imageCreateInfo.arrayLayers = 1;
		imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageCreateInfo.samples = msaa;
		imageCreateInfo.usage = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		CheckReturnFail(vkCreateImage(
			device->vkDevice,
			&imageCreateInfo,
			nullptr,
			&msaaDepthImage));

		vkGetImageMemoryRequirements(
			device->vkDevice,
			msaaDepthImage,
			&memReqs);
		if (!device->GetMemoryTypeFromRequirements(
			device->vkPhysicalDevice,
			memReqs.memoryTypeBits,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			&memAlloc.memoryTypeIndex))
			return HResultFail;
		memAlloc.allocationSize = memReqs.size;

		CheckReturnFail(vkAllocateMemory(
			device->vkDevice,
			&memAlloc,
			nullptr,
			&msaaDepthMemory));
		CheckReturnFail(vkBindImageMemory(
			device->vkDevice,
			msaaDepthImage,
			msaaDepthMemory,
			0));

		viewCreateInfo.image = msaaDepthImage;
		viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewCreateInfo.format = vkDepthFormat;
		viewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_R;
		viewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_G;
		viewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_B;
		viewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_A;
		viewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
		viewCreateInfo.subresourceRange.levelCount = 1;
		viewCreateInfo.subresourceRange.layerCount = 1;
		CheckReturnFail(vkCreateImageView(
			device->vkDevice,
			&viewCreateInfo,
			nullptr,
			&msaaDepthView));

		VkAttachmentDescription attachments[4];
		attachments[0].format = vkFormat;
		attachments[0].samples = msaa;
		attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachments[0].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		attachments[0].flags = 0;

		attachments[1].format = vkDepthFormat;
		attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
		attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachments[1].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		attachments[1].flags = 0;

		attachments[2].format = vkDepthFormat;
		attachments[2].samples = msaa;
		attachments[2].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachments[2].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachments[2].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachments[2].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachments[2].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachments[2].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		attachments[2].flags = 0;

		attachments[3].format = vkDepthFormat;
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
		CheckReturnFail(vkCreateRenderPass(
			device->vkDevice,
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

		for (uint32 i = 0; i < vkImageCount; i++)
		{
			imageViewAttachments[1] = vkViews[i];
			vkCreateFramebuffer(
				device->vkDevice,
				&fbCreateInfo,
				nullptr,
				&vkFramebuffers[i]);
		}

		CheckReturnFail(vkAcquireNextImageKHR(
			device->vkDevice,
			vkSwapChain,
			UINT64_MAX,
			VK_NULL_HANDLE,
			VK_NULL_HANDLE,
			&currentBuffer));
		vkDeviceWaitIdle(device->vkDevice);

		return HResultSuccess;
	}
	void SwapChain::Present()
	{
		vkQueueWaitIdle(vkPresentQueue);
		VkPresentInfoKHR present;
		present.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		present.pNext = nullptr;
		present.swapchainCount = 1;
		present.pSwapchains = &vkSwapChain;
		present.pImageIndices = &currentBuffer;
		present.waitSemaphoreCount = 0;
		present.pWaitSemaphores = nullptr;
		present.pResults = nullptr;
		vkQueuePresentKHR(vkPresentQueue, &present);
		vkAcquireNextImageKHR(
			device->vkDevice,
			vkSwapChain,
			UINT64_MAX,
			VK_NULL_HANDLE,
			VK_NULL_HANDLE,
			&currentBuffer);
	}
}
