// Copyright (c) 2017-2018, Roman Shkurdalov
// This file is under The Clear BSD License, see LICENSE.txt

#include "kernel\kernel.h"
#include "gpu\GpuManager.h"
#include "graphics\Font.h"
#include "ui\UIManager.h"
#include "util\CallbackTimer.h"
#include <concrt.h>

namespace kernel
{
	concurrency::critical_section globalSharedSection;

	HResult LibraryInitialize()
	{
		CheckReturn(GpuInitialize());
		CheckReturn(FontInitialize());
		CheckReturn(UIInitialize());
		CheckReturn(TimerProcessInitialize());
		return HResultSuccess;
	}
	void EnterSharedSection()
	{
		globalSharedSection.lock();
	}
	void LeaveSharedSection()
	{
		globalSharedSection.unlock();
	}
}
