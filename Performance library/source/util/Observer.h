// Copyright (c) 2017-2018, Roman Shkurdalov
// This file is under The Clear BSD License, see LICENSE.txt

#pragma once
#include "kernel\kernel.h"
#include "atc\Function.h"
#include <vector>

namespace util
{
	template<typename ...Args> class Observer
	{
	protected:
		struct Listener
		{
			Function<void(Args..., void *)> callback;
			void *param;
		};
		std::vector<Listener> callbacks;
	public:
		Observer() {}
		void AddCallback(Function<void(Args..., void *)> callback, void *param = nullptr)
		{
			callbacks.push_back({ callback, param });
		}
		void RemoveCallback(Function<void(Args..., void *)> callback)
		{
			for (uint32 i = 0; i < callbacks.size(); i++)
			{
				if (callbacks[i].callback.fptr == callback.fptr)
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
		void Notify(Args... args)
		{
			for (uint32 iter = 0; iter < callbacks.size(); iter++)
				callbacks[iter].callback(
					AppendRValue<Args...>(args)...,
					callbacks[iter].param);
		}
	};
}
