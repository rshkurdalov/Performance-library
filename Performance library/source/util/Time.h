// Copyright (c) 2017-2018, Roman Shkurdalov
// This file is under The Clear BSD License, see LICENSE.txt

#pragma once
#include "kernel\kernel.h"
#include "kernel\SharedObject.h"
#include "atc\Function.h"

namespace util
{
	enum TimerState
	{
		TimerStateActive,
		TimerStatePaused,
		TimerStateInactive,
	};

	struct TimerEvent
	{
		void *param;
		// False by default
		// Change this value to stop timer
		bool stop;
		// Initialized with current timer period
		// Change to assign new period
		int64 period;
	};

	// Time is measured in nanoseconds
    class Time
    {
    public:
		static int64 Now();
		static void CreateAsyncTimer(
			int64 period,
			AsyncTimer **ppTimer);
		// Callback must not change state of its calling timer
		// Event args can stop timer or change its period
		static void CreateCallbackTimer(
			Function<void(TimerEvent *)> callback,
			void *param,
			int64 period,
			CallbackTimer **ppTimer);
		static void DoCallbackIn(
			Function<void(void *)> callback,
			void *param,
			int64 delay);
		static void DoCallbackAt(
			Function<void(void *)> callback,
			void *param,
			int64 timestamp);
    };
}
