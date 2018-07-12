// Copyright (c) 2017-2018, Roman Shkurdalov
// This file is under The Clear BSD License, see LICENSE.txt

#pragma once
#include "kernel\kernel.h"
#include "kernel\SharedObject.h"
#include <map>
#include <thread>
#include <condition_variable>
#include <chrono>
#include <concrt.h>

namespace util
{
	enum TimerState
	{
		TimerStateActive,
		TimerStatePaused,
		TimerStateInactive,
	};

	class TimerEvent
	{
	public:
		bool stop;
		int64 nextPeriod;
		TimerEvent() : stop(false) {}
	};

    class Time
    {
		friend class AsyncTimer;
        friend class CallbackTimer;
    private:
        static concurrency::critical_section criticalSection;
        static int64 timePoint;
        static std::condition_variable waitPoint;
        static std::multimap<int64, CallbackTimer*> timers;
    public:
		static void CreateAsyncTimer(
			int64 periodNanoseconds,
			AsyncTimer **ppTimer);
		static void CreateCallbackTimer(
			std::function<void(TimerEvent &)> callback,
			int64 periodMicroseconds,
			CallbackTimer **ppTimer);
		static void DoCallbackIn(
			std::function<void()> callback,
			int64 delayMicroseconds);
		static void DoCallbackAt(
			std::function<void()> callback,
			int64 timestampMicroseconds);
        static void __stdcall timerProcessThread();
    };
}
