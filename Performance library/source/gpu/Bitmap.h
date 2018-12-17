// Copyright (c) 2017-2018, Roman Shkurdalov
// This file is under The Clear BSD License, see LICENSE.txt

#pragma once
#include "kernel\kernel.h"
#include "kernel\SharedObject.h"
#include "math\VectorMath.h"

namespace gpu
{
	class Bitmap : public SharedObject
	{
		friend class GpuDevice;
		friend class RenderTarget;
	protected:
		GpuDevice *device;
		uint32 width;
		uint32 height;
		Matrix3x2f transform;
		uint32 memOffset;
		Bitmap(
			GpuDevice *device,
			uint32 width,
			uint32 height);
		~Bitmap();
	public:
		uint32 GetWidth();
		uint32 GetHeight();
		void SetTransform(Matrix3x2f &transform);
		Matrix3x2f GetTransform();
		void MapMemory(void **ppData);
		void UnmapMempory();
	};
}
