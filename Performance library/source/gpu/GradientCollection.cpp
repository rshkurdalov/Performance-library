// Copyright (c) 2017-2018, Roman Shkurdalov
// This file is under The Clear BSD License, see LICENSE.txt

#include "gpu\GradientCollection.h"
#include "gpu\GpuDevice.h"

namespace gpu
{
	GradientCollection::GradientCollection(
		GpuDevice *device,
		msize memOffset,
		uint32 count)
	{
		device->AddRef();
		this->device = device;
		this->memOffset = memOffset;
		this->count = count;
	}
	GradientCollection::~GradientCollection()
	{
		device->DeallocateMemory(memOffset);
		device->Release();
	}
}
