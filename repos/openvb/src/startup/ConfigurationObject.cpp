#include "ConfigurationObject.hpp"

namespace F3
{
	JE::Configuration::Settings g_SettingsObject;

	// 0x56AF30. Returns the shared settings object without first-use work.
	JE::Configuration::Settings* __cdecl GetSettingsObject()
	{
		return &g_SettingsObject;
	}
}
