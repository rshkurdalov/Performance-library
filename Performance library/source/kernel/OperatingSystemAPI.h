// Copyright (c) 2017-2018, Roman Shkurdalov
// This file is under The Clear BSD License, see LICENSE.txt

#pragma once
#include "kernel\kernel.h"
#ifdef _WIN32
#include <Windows.h>
#endif

namespace kernel
{
	HResult OSCreateWindow(
		const wchar *name,
		int32 x,
		int32 y,
		uint32 width,
		uint32 height,
		ui::Window **window);

	HResult OSEnterMsgLoop(
		ui::Window *window,
		ui::UIObject *layout);

	HResult OSReleaseWindowHandler(ui::Window *window);

	HResult OSCreateSurface(
		ui::Window *window,
		gpu::Surface **ppSurface);
}
