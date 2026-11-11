#include "BufferManager.hpp"
#include "BufferRecord.hpp"

void Gfx_BufferManager::ReleaseStoredBuffers(BYTE destroy)
{
	if (!destroy) return;
	for (std::map<DWORD, Gfx_BufferRecord*>::iterator entry = records.begin();
		entry != records.end(); ++entry)
	{
		Gfx_BufferRecord* record = entry->second;
		vertexBytes -= record->vertices ? record->vertices->capacity : 0;
		indexBytes -= record->indices ? record->indices->capacity : 0;
		delete record;
	}
	records.clear();
	while (!freeHandles.empty()) freeHandles.pop_back();
	nextHandle = 1;
}
