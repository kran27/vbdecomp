#include "ModelDataManager.hpp"
#include "ModelData.hpp"

namespace
{
	DWORD __cdecl LoadResourceModelData(const char* name, DWORD kind, DWORD textures,
		DWORD shadow, float scale, DWORD fvf) //4B8630, actual initial callback.
	{
		const std::string resourceName(name);
		const DWORD handle = Gfx_LoadModelData(resourceName, kind, textures, shadow, scale, fvf);
		return handle; //The owned name dies after the handle is captured.
	}
}

//Original6FAFF4 contains4B8630 in the PE data image; no dynamic registrar.
Gfx_ModelDataLoadCallback g_GfxModelDataLoadCallback = LoadResourceModelData;

DWORD Gfx_ModelDataManager::Register(Gfx_ModelData526620* data) // 0x5CB2A0.
{
	typedef std::map<DWORD, Gfx_ModelData526620*> ModelMap;
	if (!data->handle)
	{
		DWORD handle = AllocateHandle();
		entries.insert(ModelMap::value_type(handle, NULL)).first->second = data;
		data->handle = handle;
		return handle;
	}
	DWORD originalHandle = data->handle;
	ModelMap::iterator i = entries.find(originalHandle);
	if (i == entries.end())
	{
		DWORD handle = AllocateHandle();
		entries.insert(ModelMap::value_type(handle, NULL)).first->second = data;
		data->handle = handle;
		return handle;
	}
	if (i->second != data) return 0;
	return originalHandle;
}

DWORD Gfx_ModelDataManager::References(DWORD* handle) // 5CABC0; Engine wrapper5C08B0.
{
	DWORD references = 0;
	if (*handle)
	{
		std::map<DWORD, Gfx_ModelData526620*>::iterator i = entries.find(*handle);
		if (i != entries.end()) references = i->second->references;
	}
	return references;
}

DWORD Gfx_ModelDataManager::Release(DWORD* handle) // 0x5CB6C0/5C08C0.
{
	DWORD references = 0;
	if (*handle)
	{
		std::map<DWORD, Gfx_ModelData526620*>::iterator i = entries.find(*handle);
		if (i != entries.end())
		{
			Gfx_ModelData526620* data = i->second;
			references = data->references;
			if (!references)
			{
				delete data;
				entries.erase(i);
				RecycleHandle(handle);
			}
		}
	}
	return references;
}

Gfx_ModelInstance* Gfx_ModelDataManager::CreateInstance(DWORD handle) // 5CAC00; Engine wrapper5C08D0.
{
	Gfx_ModelInstance* instance = NULL;
	if (handle)
	{
		std::map<DWORD, Gfx_ModelData526620*>::iterator i = entries.find(handle);
		if (i != entries.end()) instance = i->second->CreateModelInstance();
	}
	return instance;
}
