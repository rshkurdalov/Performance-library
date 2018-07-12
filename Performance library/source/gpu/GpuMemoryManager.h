// Copyright (c) 2017-2018, Roman Shkurdalov
// This file is under The Clear BSD License, see LICENSE.txt

#pragma once
#include "kernel\kernel.h"
#include "atc\StaticOperators.h"

namespace gpu
{
	class GpuMemoryManager
	{
		friend class GpuDevice;
	protected:
		static const msize InitialHeapSize = 2 << 15;
		struct Node
		{
			msize allocated;
			msize maxAlloc;
			Node *left;
			Node *right;
			Node(msize size)
			{
				this->allocated = 0;
				this->maxAlloc = size;
				left = nullptr;
				right = nullptr;
			}
			~Node()
			{
				if (left != nullptr)
				{
					delete left;
					delete right;
				}
			}
		};
		Node *root;
		msize heapSize;
		msize offset;
		msize range;
		msize rangeDiv2;
		Buffer *storageBuffer;
		DescriptorSet *descSet;

		GpuMemoryManager(
			Buffer *storageBuffer,
			DescriptorSet *descSet);
		~GpuMemoryManager();
		void AllocBlock(Node *node, msize size);
		void DeallocBlock(Node *node, msize offset, msize range, msize addr);
	public:
		msize Allocate(msize size);
		void Deallocate(msize addr);
	};
}
