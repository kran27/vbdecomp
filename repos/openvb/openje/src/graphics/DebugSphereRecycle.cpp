#include "RenderManager.hpp"
#include "DebugSphere.hpp"

void Gfx_RenderManager::RecycleDebugSpheres()
{
	std::list<Gfx_DebugSphere505880*>::iterator entry = usedDebugSpheres.begin();
	while (entry != usedDebugSpheres.end())
	{
		Gfx_DebugSphere505880* sphere = *entry;
		sphere->Reset();
		// Append uses the captured payload. Erase frees only the used list node;
		// the pointed object has moved to the free pool and stays owned there.
		freeDebugSpheres.push_back(sphere);
		entry = usedDebugSpheres.erase(entry);
	}
}
