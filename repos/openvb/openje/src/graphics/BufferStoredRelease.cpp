#include "BufferManager.hpp"
#include "BufferRecord.hpp"

void Gfx_BufferManager::ReleaseStoredHandle(DWORD handle)
{
	if (handle == 0xFFFFFFFF) return;
	std::map<DWORD, Gfx_BufferRecord*>::iterator found = records.find(handle);
	if (found != records.end())
	{
		Gfx_BufferRecord* record = found->second;
		if (record)
		{
			vertexBytes -= record->vertices ? record->vertices->capacity : 0;
			indexBytes -= record->indices ? record->indices->capacity : 0;
			delete record;
			records.erase(handle);
		}
	}
}
