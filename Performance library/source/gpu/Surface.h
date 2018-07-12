// Copyright (c) 2017-2018, Roman Shkurdalov
// This file is under The Clear BSD License, see LICENSE.txt

#pragma once
#include "kernel\kernel.h"
#include "kernel\SharedObject.h"
#include "kernel\OperatingSystemAPI.h"
#include "gpu\VulkanAPI.h"

namespace gpu
{
	class Surface : public SharedObject
	{
		friend class GpuDevice;
		friend class SwapChain;
		friend HResult kernel::OSCreateSurface(
			ui::Window *window,
			Surface **ppSurface);
	protected:
		VkSurfaceKHR vkSurface;

		Surface(VkSurfaceKHR surface);
		~Surface();
	};
}
