#include "ModelInstance.hpp"
#include "ModelCache.hpp"
#include "graphics/ModelInstance.hpp"

namespace JE
{
	BYTE ModelInstance::AddLight(Gfx_Light* light) //5640E0.
	{
		BYTE result = 0;
		const DWORD count = g_pModelCache->slotCount;
		for (DWORD i = 0; i < count; ++i)
			if (instances[i]) result = instances[i]->AddLight(light) || result;
		for (std::list<ModelInstance*>::iterator i = attached.begin(); i != attached.end(); ++i)
			result = (*i)->AddLight(light) || result; // Call first; normalize the accumulated result.
		return result;
	}
	void ModelInstance::RemoveLight(Gfx_Light* light) //564160.
	{
		const DWORD count = g_pModelCache->slotCount;
		for (DWORD i = 0; i < count; ++i)
			if (instances[i]) instances[i]->RemoveLight(light);
		for (std::list<ModelInstance*>::iterator i = attached.begin(); i != attached.end(); ++i)
			(*i)->RemoveLight(light);
	}
	void ModelInstance::FinishLights() //5641C0.
	{
		const DWORD count = g_pModelCache->slotCount;
		for (DWORD i = 0; i < count; ++i)
			if (instances[i]) instances[i]->FinishLights();
		for (std::list<ModelInstance*>::iterator i = attached.begin(); i != attached.end(); ++i)
			(*i)->FinishLights();
	}
	void ModelInstance::ClearLights() //564220.
	{
		const DWORD count = g_pModelCache->slotCount;
		for (DWORD i = 0; i < count; ++i)
			if (instances[i]) instances[i]->ClearLights();
		for (std::list<ModelInstance*>::iterator i = attached.begin(); i != attached.end(); ++i)
			(*i)->ClearLights();
	}
}
