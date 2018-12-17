// Copyright (c) 2017-2018, Roman Shkurdalov
// This file is under The Clear BSD License, see LICENSE.txt

#include "util\AsyncTimer.h"

namespace util
{
	AsyncTimer::AsyncTimer()
	{
		state = TimerStateInactive;
	}
	void AsyncTimer::Start()
	{
		if (state == TimerStateActive) return;
		startTimestamp = Time::Now();
		completePeriods = 0;
		state = TimerStateActive;
	}
	void AsyncTimer::Refresh()
	{
		Stop();
		Start();
	}
	void AsyncTimer::Stop()
	{
		state = TimerStateInactive;
	}
	int64 AsyncTimer::GetPeriod()
	{
		return period;
	}
	uint64 AsyncTimer::GetTicks()
	{
		uint64 ticks = ((Time::Now() - startTimestamp) / period) - completePeriods;
		completePeriods += ticks;
		return ticks;
	}
	uint64 AsyncTimer::GetCompletePeriods()
	{
		return (Time::Now() - startTimestamp) / period;
	}
	int64 AsyncTimer::GetPeriodProgress()
	{
		return (Time::Now() - startTimestamp) % period;
	}
	TimerState AsyncTimer::GetState()
	{
		return state;
	}
}
