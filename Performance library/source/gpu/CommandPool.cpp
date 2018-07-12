// Copyright (c) 2017-2018, Roman Shkurdalov
// This file is under The Clear BSD License, see LICENSE.txt

#include "gpu\CommandPool.h"
#include "gpu\GpuDevice.h"
#include "gpu\CommandBuffer.h"

namespace gpu
{
	CommandPool::CommandPool(
		GpuDevice *device,
		VkCommandPool vkCmdPool)
	{
		device->AddRef();
		this->device = device;
		this->vkCmdPool = vkCmdPool;
	}
	CommandPool::~CommandPool()
	{
		vkDestroyCommandPool(
			device->vkDevice,
			vkCmdPool,
			nullptr);
		device->Release();
	}
	HResult CommandPool::CreateCommandBuffer(CommandBuffer **ppCommandBuffer)
	{
		VkCommandBufferAllocateInfo cmd;
		cmd.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		cmd.pNext = nullptr;
		cmd.commandPool = vkCmdPool;
		cmd.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		cmd.commandBufferCount = 1;
		VkCommandBuffer vkCmdBuffer;
		CheckReturnFail(vkAllocateCommandBuffers(
			device->vkDevice,
			&cmd,
			&vkCmdBuffer));

		*ppCommandBuffer = new CommandBuffer(this, vkCmdBuffer);

		return HResultSuccess;
	}
}
