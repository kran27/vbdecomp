#include "Entity.hpp"
#include "graphics/VfxVeg.hpp"
#include <cstring>

namespace JE
{
	DWORD Entity::GetVisualEffectID(const char* name) const //54B450.
	{
		for (std::map<VFX_VEGInstance*,EntityEffectBinding>::const_iterator i=activeEffects.begin();
			i!=activeEffects.end();++i)
			if (!std::strcmp(i->second.instance->name.c_str(),name)) return i->second.id;
		return 0;
	}
}
