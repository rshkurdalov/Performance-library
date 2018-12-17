// Copyright (c) 2017-2018, Roman Shkurdalov
// This file is under The Clear BSD License, see LICENSE.txt

#include "kernel\SharedObject.h"

namespace kernel
{
    SharedObject::SharedObject()
    {
		referenceCounter = 1;
    }
    void SharedObject::AddRef()
    {
		referenceCounter++;
    }
    void SharedObject::Unref()
    {
		if (--referenceCounter == 0)
			delete this;
    }
}
