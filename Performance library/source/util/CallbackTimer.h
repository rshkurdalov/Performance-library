// Copyright (c) 2017-2018, Roman Shkurdalov
// This file is under The Clear BSD License, see LICENSE.txt

#pragma once
#include "util\Time.h"

namespace util
{
	HResult TimerProcessInitialize();

    class CallbackTimer : public SharedObject
    {
        friend class Time;
		friend HResult TimerProcessInitialize();
		friend void timerProcessFunction();
    protected:
		Function<void(TimerEvent *)> callback;
		void *param;
        int64 period;
		int64 nextAlarm;
		TimerState state;

        CallbackTimer() {}
        ~CallbackTimer();
	public:
		// Has no effect if timer is not inactive
        void Start();
		// Has no effect if timer is not active
        void Pause();
		// Has no effect if timer is not paused
        void Resume();
		// Stops and starts timer
        void Refresh();
		// Paused timers also will become inactive
        void Stop();
        TimerState GetState();
    };
}
