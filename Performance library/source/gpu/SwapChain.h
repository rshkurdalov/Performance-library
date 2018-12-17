// Copyright (c) 2017-2018, Roman Shkurdalov
// This file is under The Clear BSD License, see LICENSE.txt

#pragma once
#include "kernel\kernel.h"
#include "kernel\SharedObject.h"
#include "gpu\GpuDevice.h"
#include <vector>

namespace gpu
{
	class SwapChain : public SharedObject
	{
		friend class GpuDevice;
		friend class CommandBuffer;
	protected:
		GpuDevice *device;
		Surface *surface;
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
		VkImage msaaColorImage;
		VkDeviceMemory msaaColorMemory;
		VkImageView msaaImageView;
		VkImage msaaDepthImage;
		VkDeviceMemory msaaDepthMemory;
		VkImageView msaaDepthView;
		VkRenderPass vkRenderPass;
		std::vector<VkFramebuffer> vkFramebuffers;
		uint32 currentBuffer;

		SwapChain(
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
			uint32 currentBuffer);
		~SwapChain();
	public:
		uint32 GetWidth();
		uint32 GetHeight();
		uint32 GetCurrentBuffer();
		HResult Resize(uint32 width, uint32 height);
		void Present();
	};
}
