// Copyright (c) 2017-2018, Roman Shkurdalov
// This file is under The Clear BSD License, see LICENSE.txt

#include "util\CallbackTimer.h"
#include "util\Time.h"

using namespace std;
using namespace chrono;

namespace util
{
    CallbackTimer::~CallbackTimer()
    {
        Stop();
    }
    void CallbackTimer::Start()
    {
        Time::criticalSection.lock();
        Lock();
        if(state != TimerStateInactive)
        {
            Unlock();
            Time::criticalSection.unlock();
            return;
        }
        nextAlarm = duration_cast<microseconds>(
            high_resolution_clock::now().time_since_epoch()).count() + period;
        if(nextAlarm<Time::timers.begin()->first)
            Time::waitPoint.notify_one();
        Time::timers.insert(
            std::pair<int64, CallbackTimer *>(
                nextAlarm,
                this));
        state = TimerStateActive;
        Unlock();
        Time::criticalSection.unlock();
    }
    void CallbackTimer::Pause()
    {
        Time::criticalSection.lock();
        Lock();
        if(state != TimerStateActive)
        {
            Unlock();
            Time::criticalSection.unlock();
            return;
        }
        multimap<int64, CallbackTimer *>::const_iterator iter
            = Time::timers.find(nextAlarm);
        while(iter->second != this) iter++;
        Time::timers.erase(iter);
        nextAlarm -= duration_cast<microseconds>(
            high_resolution_clock::now().time_since_epoch()).count();
        state = TimerStatePaused;
        Unlock();
        Time::criticalSection.unlock();
    }
    void CallbackTimer::Resume()
    {
        Time::criticalSection.lock();
        Lock();
        if(state != TimerStatePaused)
        {
            Unlock();
            Time::criticalSection.unlock();
            return;
        }
        nextAlarm += duration_cast<microseconds>(
            high_resolution_clock::now().time_since_epoch()).count();
        if(nextAlarm<Time::timers.begin()->first)
            Time::waitPoint.notify_one();
        Time::timers.insert(
            std::pair<int64, CallbackTimer *>(
                nextAlarm, this));
        state = TimerStateActive;
        Unlock();
        Time::criticalSection.unlock();
    }
    void CallbackTimer::Refresh()
    {
        Stop();
        Start();
    }
    void CallbackTimer::Stop()
    {
        Time::criticalSection.lock();
        Lock();
        if(state == TimerStateInactive)
        {
            Unlock();
            Time::criticalSection.unlock();
            return;
        }
        else if(state == TimerStatePaused)
        {
            state = TimerStateInactive;
            Unlock();
            Time::criticalSection.unlock();
            return;
        }
        multimap<int64, CallbackTimer *>::const_iterator iter
            = Time::timers.find(nextAlarm);
        while(iter->second != this) iter++;
        Time::timers.erase(iter);
        state = TimerStateInactive;
        Unlock();
        Time::criticalSection.unlock();
    }
    TimerState CallbackTimer::GetTimerState()
    {
        return state;
    }
}
