#include "SkinManager.hpp"

DWORD Gfx_SkinMeshManager::AllocateHandle()
{
	DWORD result = 0;
	if (!freeHandles.empty())
	{
		result = freeHandles.back();
		freeHandles.pop_back();
	}
	else if (nextHandle != 0xFFFFFFFF) result = nextHandle++;
	return result;
}

void Gfx_SkinMeshManager::RecycleHandle(DWORD& handle)
{
	if (handle)
	{
		freeHandles.push_back(handle);
		handle = 0;
	}
}
