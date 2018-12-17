// Copyright (c) 2017-2018, Roman Shkurdalov
// This file is under The Clear BSD License, see LICENSE.txt

#pragma once

namespace kernel
{
	enum HResult
	{
		HResultSuccess = 0,
		HResultFail = 1,
		HResultInvalidArgument = 2,
		HResultCannotOpenFile = 3,

		/*GpuDevice*/

		HResultNoSuitableGPU = 10000,
		HResultNoSuitableGPUQueue = 10001,
		HResultSurfaceFormatNotFound = 10002,
		HResultPresentModeNotFound = 10003,
	};
}

#define CheckReturn(result) {decltype(result) hr = result; if(hr != 0) return (decltype(result))hr; }
#define CheckReturnFail(result) if(result != 0) return HResultFail
