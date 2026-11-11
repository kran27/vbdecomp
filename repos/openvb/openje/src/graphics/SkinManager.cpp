#include "SkinManager.hpp"

DWORD Gfx_SkinMeshManager::Register(Gfx_SkinMesh* mesh)
{
	DWORD handle = AllocateHandle();
	if (handle)
	{
		std::map<DWORD, Gfx_SkinMesh*>::iterator entry =
			entries.insert(std::make_pair(handle, static_cast<Gfx_SkinMesh*>(NULL))).first;
		entry->second = mesh;
	}
	return handle;
}

Gfx_SkinMesh* Gfx_SkinMeshManager::Lookup(DWORD handle)
{
	Gfx_SkinMesh* result = NULL;
	std::map<DWORD, Gfx_SkinMesh*>::iterator entry = entries.find(handle);
	if (entry != entries.end()) result = entry->second;
	return result;
}

void Gfx_SkinMeshManager::Remove(DWORD handle)
{
	std::map<DWORD, Gfx_SkinMesh*>::iterator entry = entries.find(handle);
	if (entry != entries.end())
	{
		entries.erase(entry);
		RecycleHandle(handle);
	}
}
