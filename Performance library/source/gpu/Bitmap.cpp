// Copyright (c) 2017-2018, Roman Shkurdalov
// This file is under The Clear BSD License, see LICENSE.txt

#include "gpu\Bitmap.h"
#include "gpu\GpuDevice.h"
#include "graphics\Color.h"

namespace gpu
{
	Bitmap::Bitmap(
		GpuDevice *device,
		uint32 width,
		uint32 height)
	{
		device->AddRef();
		this->device = device;
		this->width = width;
		this->height = height;
		MatrixSetIdentity(&transform);
		memOffset = device->AllocateMemory(width*height * sizeof(Color));
	}
	Bitmap::~Bitmap()
	{
		device->DeallocateMemory(memOffset);
		device->Unref();
	}
	uint32 Bitmap::GetWidth()
	{
		return width;
	}
	uint32 Bitmap::GetHeight()
	{
		return height;
	}
	void Bitmap::SetTransform(Matrix3x2f &transform)
	{
		this->transform = transform;
	}
	Matrix3x2f Bitmap::GetTransform()
	{
		return transform;
	}
	void Bitmap::MapMemory(void **ppData)
	{
		device->MapMemory(memOffset, width*height * sizeof(Color), ppData);
	}
	void Bitmap::UnmapMempory()
	{
		device->UnmapMemory();
	}
}
