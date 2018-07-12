// Copyright (c) 2017-2018, Roman Shkurdalov
// This file is under The Clear BSD License, see LICENSE.txt

#include "kernel\kernel.h"
#include "gpu\VulkanAPI.h"
#include "graphics\Font.h"

namespace kernel
{
	HResult EngineInitialize()
	{
		CheckReturn(gpu::VkInitialize());
		CheckReturn(graphics::FtInitialize());
		return HResultSuccess;
	}
}
