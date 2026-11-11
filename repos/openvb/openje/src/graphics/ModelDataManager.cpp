#include "ModelDataManager.hpp"
#include "ModelData.hpp"

Gfx_ModelDataManager* g_ModelDataManager;

Gfx_ModelDataManager::Gfx_ModelDataManager()
{
	// Both actual containers finish construction before the original scalar write.
	nextHandle = 1;
}

Gfx_ModelDataManager::~Gfx_ModelDataManager()
{
	for (std::map<DWORD, Gfx_ModelData526620*>::iterator entry = entries.begin();
		entry != entries.end(); ++entry)
	{
		Gfx_ModelData526620* capturedPayload = entry->second;
		if (capturedPayload)
		{
			delete capturedPayload;
		}
	}
	entries.clear();
	// Normal map/deque destruction precedes publication-base destruction.
}


void Gfx_ModelDataManager::RecycleHandle(DWORD* handle)
{
	if (*handle) { freeHandles.push_back(*handle); *handle = 0; }
}

DWORD Gfx_ModelDataManager::AllocateHandle()
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
