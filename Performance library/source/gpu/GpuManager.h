// Copyright (c) 2017-2018, Roman Shkurdalov
// This file is under The Clear BSD License, see LICENSE.txt

#pragma once
#include "kernel\kernel.h"

namespace gpu
{
	HResult GpuInitialize();

	void QueryGpuDevice(GpuDevice **ppGpuDevice);
}
