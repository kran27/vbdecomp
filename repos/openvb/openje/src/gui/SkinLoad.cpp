#include "Skin.hpp"
#include "configuration/Configuration.hpp"
#include "configuration/ResourceConfigurationRead.hpp"

namespace GUI
{
	BYTE Skin::LoadFile(const std::string& name)
	{
		Clear();
		JE::Configuration::Settings settings;
		if (!JE::Configuration::ReadFile(name, settings))
		{
			return 0;
		}
		return Apply(settings);
	}

	BYTE Skin::LoadResource(const std::string& name)
	{
		Clear();
		JE::Configuration::Settings settings;
		if (!JE::Configuration::ReadResourceConfigurationFile(name, settings))
		{
			return 0;
		}
		return Apply(settings);
	}
}
