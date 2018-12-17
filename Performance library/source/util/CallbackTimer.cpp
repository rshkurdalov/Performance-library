// Copyright (c) 2017-2018, Roman Shkurdalov
// This file is under The Clear BSD License, see LICENSE.txt

#include "util\CallbackTimer.h"
#include "util\Time.h"
#include <chrono>
#include <map>
#include <thread>
#include <condition_variable>
#include <concrt.h>

using namespace std::chrono;

namespace util
{
	int64 timePoint;
	std::condition_variable waitPoint;
	std::multimap<int64, CallbackTimer *> timers;
	void timerProcessFunction()
	{
		std::multimap<int64, CallbackTimer *>::const_iterator iter;
		CallbackTimer *alarmedTimer;
		std::mutex timerMutex;
		std::unique_lock<std::mutex> locker(timerMutex);
		int64 timePoint;
		while (true)
		{
			EnterSharedSection();
			timePoint = Time::Now();
			iter = timers.begin();
			while (timePoint >= iter->first)
			{
				alarmedTimer = iter->second;
				alarmedTimer->AddRef();
				timers.erase(iter);
				TimerEvent e;
				e.param = alarmedTimer->param;
				e.period = alarmedTimer->period;
				e.stop = false;
				alarmedTimer->callback(&e);
				if (!e.stop)
				{
					alarmedTimer->period = e.period;
					alarmedTimer->nextAlarm += alarmedTimer->period;
					timers.insert(
						std::pair<int64, CallbackTimer *>(
							alarmedTimer->nextAlarm, alarmedTimer));
				}
				else alarmedTimer->state = TimerStateInactive;
				alarmedTimer->Unref();
				iter = timers.begin();
			}
			LeaveSharedSection();
			waitPoint.wait_until(locker, time_point<steady_clock>(nanoseconds(iter->first)));
		}
	}

	HResult TimerProcessInitialize()
	{
		void(*defaultCallback)(TimerEvent *) = [](TimerEvent *e)->void {};
		CallbackTimer *defaultTimer;
		Time::CreateCallbackTimer(defaultCallback, nullptr, 1000000000000, &defaultTimer);
		defaultTimer->nextAlarm = Time::Now() + 1000000000000;
		timers.insert(std::pair<int64, CallbackTimer *>(defaultTimer->nextAlarm, defaultTimer));
		std::thread timerThread(timerProcessFunction);
		timerThread.detach();
		return HResultSuccess;
	}

	CallbackTimer::~CallbackTimer()
	{
		Stop();
	}
	void CallbackTimer::Start()
	{
		if (state != TimerStateInactive) return;
		nextAlarm = Time::Now() + period;
		if (nextAlarm < timers.begin()->first)
			waitPoint.notify_one();
		timers.insert(std::pair<int64, CallbackTimer *>(nextAlarm, this));
		state = TimerStateActive;
	}
	void CallbackTimer::Pause()
	{
		if (state != TimerStateActive) return;
		std::multimap<int64, CallbackTimer *>::const_iterator iter = timers.find(nextAlarm);
		while (iter->second != this) iter++;
		timers.erase(iter);
		nextAlarm -= Time::Now();
		state = TimerStatePaused;
	}
	void CallbackTimer::Resume()
	{
		if (state != TimerStatePaused) return;
		nextAlarm += Time::Now();
		if (nextAlarm < timers.begin()->first)
			waitPoint.notify_one();
		timers.insert(std::pair<int64, CallbackTimer *>(nextAlarm, this));
		state = TimerStateActive;
	}
	void CallbackTimer::Refresh()
	{
		Stop();
		Start();
	}
	void CallbackTimer::Stop()
	{
		if (state == TimerStateInactive) return;
		else if (state == TimerStatePaused)
		{
			state = TimerStateInactive;
			return;
		}
		std::multimap<int64, CallbackTimer *>::const_iterator iter = timers.find(nextAlarm);
		while (iter->second != this) iter++;
		timers.erase(iter);
		state = TimerStateInactive;
	}
	TimerState CallbackTimer::GetState()
	{
		return state;
	}
}
