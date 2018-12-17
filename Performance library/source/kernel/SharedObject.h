// Copyright (c) 2017-2018, Roman Shkurdalov
// This file is under The Clear BSD License, see LICENSE.txt

#pragma once
#include "kernel\kernel.h"

namespace kernel
{
    class SharedObject
    {
    private:
        uint32 referenceCounter;

		SharedObject(SharedObject &) {}
		SharedObject(SharedObject &&) {}
    protected:
        SharedObject();
		virtual ~SharedObject() {}
	public:
        void AddRef();
        void Unref();
    };
}
