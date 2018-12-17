// Copyright (c) 2017-2018, Roman Shkurdalov
// This file is under The Clear BSD License, see LICENSE.txt

#pragma once
#include "kernel\kernel.h"
#include "kernel\SharedObject.h"
#include "gpu\GpuDevice.h"

namespace gpu
{
	class Pipeline : public SharedObject
	{
		friend class GpuDevice;
		friend class CommandBuffer;
		friend class SwapChain;
	protected:
		GpuDevice *device;
		SwapChain *swapChain;
		VkPipeline vkPipeline;
		VkPipelineLayout vkPipelineLayout;

		Pipeline(
			GpuDevice *device,
			SwapChain *swapChain,
			VkPipelineLayout vkPipelineLayout,
			VkPipeline vkPipeline);
		~Pipeline();
	};
}
