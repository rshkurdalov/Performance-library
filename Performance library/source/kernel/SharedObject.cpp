// Copyright (c) 2017-2018, Roman Shkurdalov
// This file is under The Clear BSD License, see LICENSE.txt

#include "kernel\SharedObject.h"

namespace kernel
{
    SharedObject::SharedObject()
    {
		refCounter = 1;
    }
    void SharedObject::AddRef()
    {
		refCounter++;
    }
    void SharedObject::Release()
    {
		if (--refCounter == 0)
			delete this;
    }
    bool SharedObject::TryLock()
    {
        return criticalSection.try_lock();
    }
    void SharedObject::Lock()
    {
        criticalSection.lock();
    }
    void SharedObject::Unlock()
    {
        criticalSection.unlock();
    }
}
