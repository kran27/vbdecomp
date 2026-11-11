#include "PacketStorage.hpp"
#include "PacketBuffer.hpp"
#include <cstdio>
#include <new>

// Real std::list, allocation, formatting and Win32
// supply the dependencies. Constructor cleanup uses ordinary C++ member lifetime.

namespace GUI
{
	cls_0x4b6910::cls_0x4b6910(DWORD bytesPerBlock)
	{
		// Both list members have already been constructed before this first write.
		blockBytes = bytesPerBlock;
		blocks.clear();
		batches.clear();
		BYTE* block = static_cast<BYTE*>(::operator new(blockBytes));
		blocks.push_back(block);
		// If push_back throws the raw block is leaked, as in the original.
		usedBytes = 0;
		nextBatch = 0;
		currentBlock = block;
	}

	cls_0x4b6910::~cls_0x4b6910()
	{
		char buffer[256];
		DWORD batchCount = static_cast<DWORD>(batches.size());
		std::sprintf(buffer, "GUI memory -> %d total blocks of size %d\n",
			static_cast<LONG>(blocks.size()), static_cast<LONG>(blockBytes));
		OutputDebugStringA(buffer);
		// Original cached the batch count before the first formatting/debug boundary.
		std::sprintf(buffer, "              %d total mesh objects allocated\n",
			static_cast<LONG>(batchCount));
		OutputDebugStringA(buffer);
		for (std::list<BYTE*>::iterator block = blocks.begin(); block != blocks.end(); ++block)
		{
			if (*block != 0)
			{
				::operator delete(*block);
			}
		}
		blocks.clear();
		for (std::list<Gfx_UIPacketBuffer*>::iterator batch = batches.begin();
			batch != batches.end(); ++batch)
		{
			if (*batch != 0)
			{
				delete *batch;
			}
		}
		batches.clear();
		// Real list members then destroy batches followed by blocks.
		// Scalar words remain unchanged, including dangling currentBlock.
	}

	void cls_0x4b6910::Reset()
	{
		currentBlock = *blocks.begin();
		usedBytes = 0;
		nextBatch = 0;
	}

	BYTE* cls_0x4b6910::Allocate(DWORD bytes)
	{
		if (static_cast<LONG>(usedBytes + bytes) > static_cast<LONG>(blockBytes))
		{
			std::list<BYTE*>::iterator block = blocks.begin();
			std::list<BYTE*>::iterator end = blocks.end();
			BYTE* previous = currentBlock;
			for (; block != blocks.end(); ++block)
			{
				if (*block == previous)
				{
					++block;
					break;
				}
			}
			if (block == end)
			{
				BYTE* allocated = static_cast<BYTE*>(::operator new(blockBytes));
				blocks.push_back(allocated);
				currentBlock = allocated;
			}
			else
			{
				currentBlock = *block;
			}
			usedBytes = 0;
		}
		BYTE* result = currentBlock + usedBytes;
		usedBytes += bytes;
		return result;
	}

	Gfx_UIPacketBuffer* cls_0x4b6910::AcquireBatch()
	{
		if (static_cast<LONG>(nextBatch) < static_cast<LONG>(batches.size()))
		{
			std::list<Gfx_UIPacketBuffer*>::iterator batch = batches.begin();
			for (LONG index = 0; index < static_cast<LONG>(nextBatch); ++index)
				++batch;
			Gfx_UIPacketBuffer* result = *batch;
			++nextBatch;
			return result;
		}
		Gfx_UIPacketBuffer* batch = new Gfx_UIPacketBuffer;
		// The new expression alone owns constructor-failure deallocation. If the
		// later list insertion throws, the successfully constructed batch leaks.
		batches.push_back(batch);
		++nextBatch;
		return batch;
	}
}
