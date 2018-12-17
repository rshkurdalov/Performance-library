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
		static constexpr uint32 InitialHeapSize = 2 << 15;
		struct Node
		{
			uint32 allocated;
			uint32 maxAlloc;
			Node *left;
			Node *right;
			Node(uint32 size)
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
		uint32 heapSize;
		uint32 offset;
		uint32 range;
		uint32 rangeDiv2;
		GpuDevice *device;
		Buffer *storageBuffer;

		GpuMemoryManager(
			GpuDevice *device,
			Buffer *storageBuffer);
		~GpuMemoryManager();
		void AllocBlock(Node *node, uint32 size);
		void DeallocBlock(Node *node, uint32 offset, uint32 range, uint32 addr);
	public:
		uint32 Allocate(uint32 size);
		void Deallocate(uint32 addr);
	};
}
