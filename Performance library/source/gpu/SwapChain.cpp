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
		uint32 currentBuffer)
	{
		device->AddRef();
		this->device = device;
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
		this->vkRenderPass = vkRenderPass;
		this->vkFramebuffers = vkFramebuffers;
		this->currentBuffer = currentBuffer;
	}
	SwapChain::~SwapChain()
	{
		for (uint32 i = 0; i < imageCount; i++)
			vkDestroyFramebuffer(device->vkDevice, vkFramebuffers[i], nullptr);
		vkDestroyRenderPass(device->vkDevice, vkRenderPass, nullptr);
		vkDestroyImageView(device->vkDevice, vkDepthView, nullptr);
		vkDestroyImage(device->vkDevice, vkDepthImage, nullptr);
		vkFreeMemory(device->vkDevice, vkDepthMemory, nullptr);
		for (uint32 i = 0; i < imageCount; i++)
			vkDestroyImageView(device->vkDevice, vkViews[i], nullptr);
		vkDestroySwapchainKHR(device->vkDevice, vkSwapChain, nullptr);
		device->Release();
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
