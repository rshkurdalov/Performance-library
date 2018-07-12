// Copyright (c) 2017-2018, Roman Shkurdalov
// This file is under The Clear BSD License, see LICENSE.txt

#pragma once
#include "kernel\kernel.h"
#include "kernel\SharedObject.h"
#include "graphics\Color.h"

namespace gpu
{
	enum ColorInterpolationMode
	{
		ColorInterpolationModeFlat = 0,
		ColorInterpolationModeLinear = 1,
		ColorInterpolationModeSmooth = 2,
	};

	struct GradientStop
	{
		float32 offset;
		Color color;
	};

	class GradientCollection : public SharedObject
	{
		friend class GpuDevice;
		friend class RenderTarget;
	protected:
		GpuDevice *device;
		msize memOffset;
		uint32 count;
		GradientCollection(
			GpuDevice *device,
			msize memOffset,
			uint32 count);
		~GradientCollection();
	};
}
