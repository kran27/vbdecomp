#include "Manager.hpp"
#include "Skin.hpp"

namespace GUI
{
	Skin* Manager::GetSkin(const std::string& name)
	{
		std::map<std::string, Skin*>::iterator found = skins.find(name);
		if (found != skins.end())
		{
			return found->second;
		}
		Skin* created = new Skin;
		if (!created->LoadResource(name))
		{
			delete created;
			return NULL;
		}
		skins[name] = created;
		return created;
	}
}
