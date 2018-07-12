// Copyright (c) 2017-2018, Roman Shkurdalov
// This file is under The Clear BSD License, see LICENSE.txt

#include "util\AsyncTimer.h"

namespace util
{
    AsyncTimer::AsyncTimer()
    {
        state = TimerStateInactive;
    }
    AsyncTimer::~AsyncTimer()
    {

    }
    void AsyncTimer::Start()
    {
        Lock();
        if(state == TimerStateActive)
        {
            Unlock();
            return;
        }
        timeExcess = 0;
        state = TimerStateActive;
        lastTimestamp = std::chrono::high_resolution_clock::now().time_since_epoch().count();
        Unlock();
    }
    void AsyncTimer::Refresh()
    {
        Stop();
        Start();
    }
    void AsyncTimer::Stop()
    {
        Lock();
        if(state == TimerStateInactive)
        {
            Unlock();
            return;
        }
        state = TimerStateInactive;
        Unlock();
    }
    uint64 AsyncTimer::GetTicks()
    {
        Lock();
        if(state == TimerStateInactive)
        {
            Unlock();
            return 0;
        }
        int64 now = std::chrono::high_resolution_clock::now().time_since_epoch().count();
        uint64 ticks = (now - lastTimestamp + timeExcess) / period;
        timeExcess = (now - lastTimestamp + timeExcess) % period;
        lastTimestamp = now;
        Unlock();
        return ticks;
    }
    TimerState AsyncTimer::GetTimerState()
    {
        return state;
    }
}
