#include "SkinManager.hpp"

void Gfx_SkinMeshManager::Visit()
{
	for (std::map<DWORD, Gfx_SkinMesh*>::iterator entry = entries.begin();
		entry != entries.end(); ++entry)
	{
		Gfx_SkinMeshDeviceChange(entry->second);
	}
}
