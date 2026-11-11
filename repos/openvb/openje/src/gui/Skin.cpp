#include "Skin.hpp"
#include "graphics/SystemInterface.hpp"

namespace GUI
{
	Skin::Skin()
	{
	}

	Skin::~Skin()
	{
		// Fallback then map records/head follow normal reverse member lifetime.
	}

	void Skin::Clear()
	{
		fragments.clear();
	}

	SkinFragment* Skin::Find(std::string name)
	{
		std::map<std::string, SkinFragment>::iterator found;
		found = fragments.find(name);
		if (found != fragments.end()) return &found->second;
		return &fallback;
	}

	BYTE Skin::Apply(const JE::Configuration::Settings& settings)
	{
		for (JE::Configuration::Settings::const_iterator current = settings.begin();
			current != settings.end(); ++current)
		{
			SkinFragment& fragment = fragments[current->first];
			fragment.Parse(current->second);
		}
		return 1;
	}
}
