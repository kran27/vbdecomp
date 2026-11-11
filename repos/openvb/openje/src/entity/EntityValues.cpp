#include "Entity.hpp"

namespace JE
{
	DWORD Entity::GetScriptValue(LONG key) const
	{
		std::map<LONG,DWORD>::const_iterator i = scriptValues.find(key);
		if (i != scriptValues.end()) return i->second;
		return 0;
	}

	void Entity::SetScriptValue(LONG key, DWORD value)
	{
		scriptValues[key] = value;
	}
}
