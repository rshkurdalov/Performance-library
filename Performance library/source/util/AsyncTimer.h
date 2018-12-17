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
        int64 startTimestamp;
        uint64 completePeriods;
        TimerState state;

        AsyncTimer();
    public:
        void Start();
        void Refresh();
        void Stop();
		int64 GetPeriod();
		// Get number of complete periods
		// accumulated from start time to now
		// minus number of complete periods
		// accumulated from start time to last time
		// this function has been called
		// Result is undefined if timer is not running
        uint64 GetTicks();
		// Get number of complete periods
		// accumulated from start time to now
		// Result is undefined if timer is not running
		uint64 GetCompletePeriods();
		// Get time elapsed from beginning of current period
		// Result is undefined if timer is not running
		int64 GetPeriodProgress();
        TimerState GetState();
    };
}
