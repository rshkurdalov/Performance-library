// Copyright (c) 2017-2018, Roman Shkurdalov
// This file is under The Clear BSD License, see LICENSE.txt

#include "gpu\Shader.h"
#include "gpu\GpuDevice.h"

namespace gpu
{
	Shader::Shader(
		GpuDevice *device,
		VkShaderModule vkShader)
	{
		device->AddRef();
		this->device = device;
		this->vkShader = vkShader;
	}
	Shader::~Shader()
	{
		vkDestroyShaderModule(device->vkDevice, vkShader, nullptr);
		device->Release();
	}
}
