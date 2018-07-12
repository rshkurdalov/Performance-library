// Copyright (c) 2017-2018, Roman Shkurdalov
// This file is under The Clear BSD License, see LICENSE.txt

#pragma once
#include "kernel\kernel.h"
#include "atc\Function.h"
#include <vector>

namespace util
{
	template<typename EventArgsType>
	class Observer
	{
	protected:
		struct Listener
		{
			Function<bool(EventArgsType *, void *)> callback;
			void *param;
		};
		std::vector<Listener> callbacks;
	public:
		Observer() {}
		void AddCallback(Function<bool(EventArgsType *, void *)> callback, void *param)
		{
			callbacks.push_back({callback, param});
		}
		void RemoveCallback(Function<bool(EventArgsType *, void *)> callback)
		{
			for (msize i = 0; i < callbacks.size(); i++)
			{
				if (callbacks[i].Ptr == callback.Ptr)
				{
					callbacks.erase(callbacks.begin() + i);
					return;
				}
			}
		}
		void RemoveAll()
		{
			callbacks.clear();
		}
		void Notify(EventArgsType *e)
		{
			msize iter = 0;
			while (iter < callbacks.size())
				if (!callbacks[iter].callback(e, callbacks[iter].param))
					callbacks.erase(callbacks.begin() + iter);
				else iter++;
		}
	};
}
