#include "BufferManager.hpp"

DWORD Gfx_BufferManager::AllocateHandle()
{
	DWORD result = 0;
	if (!freeHandles.empty())
	{
		result = freeHandles.back();
		freeHandles.pop_back();
	}
	else if (nextHandle != 0xFFFFFFFE) result = nextHandle++;
	return result;
}
