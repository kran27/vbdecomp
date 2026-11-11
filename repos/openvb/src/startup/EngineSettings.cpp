#include "EngineSettings.hpp"

namespace F3
{
	JE::Configuration::Settings g_EngineSettings;

	JE::Configuration::Settings* __cdecl GetEngineSettings()
	{
		return &g_EngineSettings;
	}
}
