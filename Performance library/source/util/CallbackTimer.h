// Copyright (c) 2017-2018, Roman Shkurdalov
// This file is under The Clear BSD License, see LICENSE.txt

#pragma once
#include "util\Time.h"

namespace util
{
    class CallbackTimer : public SharedObject
    {
        friend class Time;
    protected:
        int64 period;
		int64 nextAlarm;
        std::function<void(TimerEvent &)> callback;
		aligned(TimerState) state;

        CallbackTimer() {}
        ~CallbackTimer();
	public:
        void Start();
        void Pause();
        void Resume();
        void Refresh();
        void Stop();
        TimerState GetTimerState();
    };
}
