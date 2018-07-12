// Copyright (c) 2017-2018, Roman Shkurdalov
// This file is under The Clear BSD License, see LICENSE.txt

#include "util\Time.h"
#include "util\AsyncTimer.h"
#include "util\CallbackTimer.h"

using namespace std;
using namespace chrono;

namespace util
{
    concurrency::critical_section Time::criticalSection;
    condition_variable Time::waitPoint;
    multimap<int64, CallbackTimer *> Time::timers;
    thread _timerThread(Time::timerProcessThread);

	void Time::CreateAsyncTimer(
		int64 periodNanoseconds,
		AsyncTimer **ppTimer)
	{
		*ppTimer = new AsyncTimer();
		(*ppTimer)->period = periodNanoseconds;
	}

	void Time::CreateCallbackTimer(
		std::function<void(TimerEvent &)> callback,
		int64 periodMicroseconds,
		CallbackTimer **ppTimer)
	{
		CallbackTimer *timer = new CallbackTimer();
		timer->period = periodMicroseconds;
		timer->callback = callback;
		timer->state = TimerStateInactive;
		*ppTimer = timer;
	}

	void Time::DoCallbackIn(
		std::function<void()> callback,
		int64 delayMicroseconds)
	{
		CallbackTimer *timer = new CallbackTimer();
		timer->period = delayMicroseconds;
		timer->callback = [timer, callback](TimerEvent &e) -> void
		{
			callback();
			timer->Release();
			e.stop = true;
		};
		timer->state = TimerStateInactive;
		timer->Start();
	}

	void Time::DoCallbackAt(
		std::function<void()> callback,
		int64 timestampMicroseconds)
	{
		CallbackTimer *timer = new CallbackTimer();
		timer->callback = [timer, callback](TimerEvent &e) -> void
		{
			callback();
			timer->Release();
			e.stop = true;
		};
		Time::criticalSection.lock();
		Time::timers.insert(pair<int64, CallbackTimer *>(
			timestampMicroseconds, timer));
		Time::criticalSection.unlock();
	}

	void Time::timerProcessThread()
    {
        _timerThread.detach();
        multimap<int64, CallbackTimer *>::const_iterator iter;
        CallbackTimer *alarmedTimer;
        mutex timerMutex;
        unique_lock<mutex> locker(timerMutex);
		int64 timePoint = duration_cast<microseconds>(high_resolution_clock::now().time_since_epoch()).count();
        CallbackTimer defaultTimer;
        defaultTimer.period = 1000000000;
        defaultTimer.callback = [] (TimerEvent &e) -> void {};
        timers.insert(pair<int64, CallbackTimer *>(
            timePoint + defaultTimer.period,
            &defaultTimer));
        while(true)
        {
            criticalSection.lock();
            timePoint = duration_cast<microseconds>
                (high_resolution_clock::now().time_since_epoch()).count();
            iter = timers.begin();
            while(timePoint >= iter->first)
            {
                alarmedTimer = iter->second;
                alarmedTimer->AddRef();
                timers.erase(iter);
                alarmedTimer->Lock();
                criticalSection.unlock();
				TimerEvent e;
				e.nextPeriod = alarmedTimer->period;
				alarmedTimer->callback(e);
                if(!e.stop)
                {
					alarmedTimer->period = e.nextPeriod;
                    alarmedTimer->nextAlarm += alarmedTimer->period;
                    timers.insert(
                        std::pair<int64, CallbackTimer *>(
                            alarmedTimer->nextAlarm, alarmedTimer));
                }
                else alarmedTimer->state = TimerStateInactive;
                alarmedTimer->Unlock();
                alarmedTimer->Release();
                criticalSection.lock();
                iter = timers.begin();
            }
            criticalSection.unlock();
            waitPoint.wait_until(
                locker,
                time_point<steady_clock>(microseconds(iter->first)));
        }
    }
}
