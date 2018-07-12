// Copyright (c) 2017-2018, Roman Shkurdalov
// This file is under The Clear BSD License, see LICENSE.txt

#pragma once
#include "kernel\kernel.h"
#include "kernel\SharedObject.h"
#include "gpu\VulkanAPI.h"
#include <vector>

namespace gpu
{
	class SwapChain : public SharedObject
	{
		friend class GpuDevice;
		friend class CommandBuffer;
	protected:
		GpuDevice *device;
		CommandBuffer *cmdBuffer;
		VkSwapchainCreateInfoKHR vkSwapChainCreateInfo;
		VkSwapchainKHR vkSwapChain;
		VkQueue vkGraphicsQueue;
		VkQueue vkPresentQueue;
		VkFormat vkFormat;
		uint32 imageCount;
		std::vector<VkImage> vkImages;
		std::vector<VkImageView> vkViews;
		VkFormat vkDepthFormat;
		VkImage vkDepthImage;
		VkImageView vkDepthView;
		VkDeviceMemory vkDepthMemory;
		VkRenderPass vkRenderPass;
		std::vector<VkFramebuffer> vkFramebuffers;
		uint32 currentBuffer;

		SwapChain(
			GpuDevice *device,
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
			VkRenderPass vkRenderPass,
			std::vector<VkFramebuffer> vkFramebuffers,
			uint32 currentBuffer);
		~SwapChain();
	public:
		uint32 GetWidth();
		uint32 GetHeight();
		uint32 GetCurrentBuffer();
		void Present();
	};
}
