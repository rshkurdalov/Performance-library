// Copyright (c) 2017-2018, Roman Shkurdalov
// This file is under The Clear BSD License, see LICENSE.txt

#pragma once
#include "util\Time.h"

namespace util
{
    class AsyncTimer : public SharedObject
    {
		friend class Time;
    protected:
        int64 period;
        int64 lastTimestamp;
        int64 timeExcess;
        aligned(TimerState) state;

        AsyncTimer();
        ~AsyncTimer();
    public:
        void Start();
        void Refresh();
        void Stop();
        uint64 GetTicks();
        TimerState GetTimerState();
    };
}
