// Copyright (c) 2017-2018, Roman Shkurdalov
// This file is under The Clear BSD License, see LICENSE.txt

#include "util\Time.h"
#include "util\AsyncTimer.h"
#include "util\CallbackTimer.h"
#include <chrono>

namespace util
{
	int64 Time::Now()
	{
		return std::chrono::high_resolution_clock::now().time_since_epoch().count();
	}
	void Time::CreateAsyncTimer(
		int64 period,
		AsyncTimer **ppTimer)
	{
		*ppTimer = new AsyncTimer();
		(*ppTimer)->period = period;
	}
	void Time::CreateCallbackTimer(
		Function<void(TimerEvent *)> callback,
		void *param,
		int64 period,
		CallbackTimer **ppTimer)
	{
		CallbackTimer *timer = new CallbackTimer();
		timer->callback = callback;
		timer->param = param;
		timer->period = period;
		timer->state = TimerStateInactive;
		*ppTimer = timer;
	}
	void Time::DoCallbackIn(
		Function<void(void *)> callback,
		void *param,
		int64 delay)
	{
		CallbackTimer *timer = new CallbackTimer();
		timer->period = delay;
		struct OverlayParams
		{
			Function<void(void *)> callback;
			void *callbackParam;
			CallbackTimer *timer;
		} op;
		op.callback = callback;
		op.callbackParam = param;
		op.timer = timer;
		static void(*overlayCallback)(TimerEvent *) = [](TimerEvent *e) -> void
		{
			OverlayParams *op = (OverlayParams *)e->param;
			op->callback(op->callbackParam);
			op->timer->Unref();
			e->stop = true;
		};
		timer->callback = overlayCallback;
		timer->state = TimerStateInactive;
		timer->Start();
		timer->Unref();
	}
	void Time::DoCallbackAt(
		Function<void(void *)> callback,
		void *param,
		int64 timestamp)
	{
		DoCallbackIn(
			callback,
			param,
			timestamp - Now());
	}
}
