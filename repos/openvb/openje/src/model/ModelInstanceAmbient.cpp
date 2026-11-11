#include "ModelInstance.hpp"
#include "ModelCache.hpp"
#include "graphics/ModelInstance.hpp"

namespace JE
{
	void ModelInstance::SetAmbientOverride(BYTE value)
	{
		DWORD count=g_pModelCache->slotCount;
		for (DWORD slot=0; slot<count; ++slot)
			if (instances[slot]) instances[slot]->SetAmbientOverride(value);
		for (std::list<ModelInstance*>::iterator i=attached.begin(); i!=attached.end(); ++i)
			(*i)->SetAmbientOverride(value);
	}
	void ModelInstance::SetAmbientColor(DWORD value)
	{
		DWORD count=g_pModelCache->slotCount;
		for (DWORD slot=0; slot<count; ++slot)
			if (instances[slot]) instances[slot]->SetAmbientColor(value);
		for (std::list<ModelInstance*>::iterator i=attached.begin(); i!=attached.end(); ++i)
			(*i)->SetAmbientColor(value);
	}
}
