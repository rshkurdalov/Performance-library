// Copyright (c) 2017-2018, Roman Shkurdalov
// This file is under The Clear BSD License, see LICENSE.txt

#pragma once
#include "kernel\kernel.h"
#include "kernel\SharedObject.h"
#include "gpu\VulkanAPI.h"

namespace gpu
{
	class Shader : public SharedObject
	{
		friend class GpuDevice;
	protected:
		GpuDevice *device;
		VkShaderModule vkShader;

		Shader(
			GpuDevice *device,
			VkShaderModule vkShader);
		~Shader();
	};
}
