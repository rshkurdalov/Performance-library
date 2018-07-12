// Copyright (c) 2017-2018, Roman Shkurdalov
// This file is under The Clear BSD License, see LICENSE.txt

#pragma once
#include "kernel\kernel.h"
#include <atomic>
#include <concrt.h>

namespace kernel
{
    class SharedObject
    {
    private:
        std::atomic<uint32> refCounter;
		concurrency::critical_section criticalSection;
    protected:
        SharedObject();
		virtual ~SharedObject() {}
	public:
        void AddRef();
        void Release();
        bool TryLock();
        void Lock();
        void Unlock();
    };
}

