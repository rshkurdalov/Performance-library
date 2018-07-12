// Copyright (c) 2017-2018, Roman Shkurdalov
// This file is under The Clear BSD License, see LICENSE.txt

#include "gpu\GpuMemoryManager.h"
#include "gpu\Buffer.h"
#include "gpu\DescriptorSet.h"
#include "gpu\GpuDevice.h"

namespace gpu
{
	GpuMemoryManager::GpuMemoryManager(
		Buffer *storageBuffer,
		DescriptorSet *descSet)
	{
		this->storageBuffer = storageBuffer;
		this->descSet = descSet;
		root = new Node(InitialHeapSize);
		heapSize = InitialHeapSize;
	}
	GpuMemoryManager::~GpuMemoryManager()
	{
		delete root;
	}
	void GpuMemoryManager::AllocBlock(Node *node, msize size)
	{
		rangeDiv2 = range >> 1;
		if (rangeDiv2 < size)
		{
			if (range == size)
			{
				node->maxAlloc = 0;
				node->allocated = size;
			}
			else
			{
				node->left = new Node(rangeDiv2);
				node->left->allocated = size;
				node->left->maxAlloc = 0;
				node->right = new Node(rangeDiv2);
				range = rangeDiv2;
				AllocBlock(node->right, size - rangeDiv2);
				node->maxAlloc = node->right->maxAlloc;
			}
		}
		else
		{
			range = rangeDiv2;
			if (node->left == nullptr)
			{
				node->left = new Node(rangeDiv2);
				node->right = new Node(rangeDiv2);
			}
			if (size <= node->left->maxAlloc)
				AllocBlock(node->left, size);
			else
			{
				offset += rangeDiv2;
				AllocBlock(node->right, size);
			}
			node->maxAlloc = Max(node->left->maxAlloc, node->right->maxAlloc);
		}
	}
	void GpuMemoryManager::DeallocBlock(Node *node, msize offset, msize range, msize addr)
	{
		msize rangeDiv2 = range >> 1;
		if (offset == addr)
		{
			if (node->allocated != 0)
			{
				node->allocated = 0;
				node->maxAlloc = range;
			}
			else if (node->left->allocated != 0)
			{
				if (node->left->allocated != rangeDiv2)
					DeallocBlock(node->right, offset + rangeDiv2, rangeDiv2, offset + rangeDiv2);
				node->left->allocated = 0;
				node->left->maxAlloc = rangeDiv2;
			}
			else DeallocBlock(node->left, offset, rangeDiv2, addr);
		}
		else if (addr < offset + rangeDiv2)
			DeallocBlock(node->left, offset, rangeDiv2, addr);
		else
			DeallocBlock(node->right, offset + rangeDiv2, rangeDiv2, addr);
		if (node->left != nullptr)
		{
			if (node->left->maxAlloc == rangeDiv2
				&& node->right->maxAlloc == rangeDiv2)
			{
				delete node->left;
				node->left = nullptr;
				delete node->right;
				node->right = nullptr;
				node->maxAlloc = range;
			}
			else node->maxAlloc = Max(node->left->maxAlloc, node->right->maxAlloc);
		}
		else node->maxAlloc = range;
	}
	msize GpuMemoryManager::Allocate(msize size)
	{
		if (root->maxAlloc < size)
		{
			while (root->maxAlloc < size)
			{
				Node *node = new Node(heapSize << 1);
				node->left = root;
				node->right = new Node(heapSize);
				root = node;
				if (root->left->maxAlloc == heapSize) root->maxAlloc = heapSize << 1;
				else root->maxAlloc = Max(root->left->maxAlloc, root->right->maxAlloc);
				heapSize <<= 1;
			}
			storageBuffer->Resize(heapSize);
			descSet->UpdateBuffer(storageBuffer);
		}
		offset = 0;
		range = heapSize;
		AllocBlock(root, size);
		return offset;
	}
	void GpuMemoryManager::Deallocate(msize addr)
	{
		DeallocBlock(root, 0, heapSize, addr);
	}
}
