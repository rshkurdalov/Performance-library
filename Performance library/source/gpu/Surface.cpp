// Copyright (c) 2017-2018, Roman Shkurdalov
// This file is under The Clear BSD License, see LICENSE.txt

#include "gpu\Surface.h"

namespace gpu
{
	Surface::Surface(VkSurfaceKHR surface)
	{
		this->vkSurface = surface;
	}
	Surface::~Surface()
	{
		VkInstance vkInstance;
		VkQueryInstance(&vkInstance);
		vkDestroySurfaceKHR(
			vkInstance,
			vkSurface,
			nullptr);
	}
}
