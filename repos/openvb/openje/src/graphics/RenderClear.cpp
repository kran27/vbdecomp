#include "RenderManager.hpp"

void Gfx_RenderManager::Clear()
{
	staticGroups[0].clear();
	dynamicGroups[0].clear();
	skinMeshes.clear();
	// The original empties each list before clearing its containing map. Keep
	// this forward pass: list-node releases can call back into the renderer.
	std::map<DWORD, std::list<Gfx_RenderSubmission16> >::iterator entry = skinSubmissions.begin();
	while (entry != skinSubmissions.end())
	{
		entry->second.clear();
		++entry;
	}
	skinSubmissions.clear();
}
