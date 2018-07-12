// Copyright (c) 2017-2018, Roman Shkurdalov
// This file is under The Clear BSD License, see LICENSE.txt

#include "gpu\Pipeline.h"
#include "gpu\GpuDevice.h"
#include "gpu\SwapChain.h"

namespace gpu
{
	Pipeline::Pipeline(
		GpuDevice *device,
		SwapChain *swapChain,
		VkPipelineLayout vkPipelineLayout,
		VkPipeline vkPipeline)
	{
		device->AddRef();
		this->device = device;
		swapChain->AddRef();
		this->swapChain = swapChain;
		this->vkPipelineLayout = vkPipelineLayout;
		this->vkPipeline = vkPipeline;
	}
	Pipeline::~Pipeline()
	{
		vkDestroyPipeline(
			device->vkDevice,
			vkPipeline,
			nullptr);
		device->Release();
		swapChain->Release();
	}
}
