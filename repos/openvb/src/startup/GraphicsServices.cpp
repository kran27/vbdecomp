#include <winsock2.h>
#include "NetworkComponent.hpp"
#include "ConfigurationObject.hpp"
#include "EngineSettings.hpp"
#include "EngineStringSystem.hpp"
#include "Window.hpp"
#include "graphics/GraphicsServices.hpp"
#include "network/EngineClient.hpp"

namespace JE
{
	namespace EngineClientServices
	{
		Client* CurrentClient()
		{
			return static_cast<Client*>(F3::g_pNetworkInstance->client);
		}
	}

	namespace GraphicsServices
	{
		Configuration::Settings& MainSettings() { return *F3::GetSettingsObject(); }
		Configuration::Settings& EngineSettings() { return *F3::GetEngineSettings(); }
		HWND Window() { return F3::g_window; }
		BYTE __cdecl ResolveString(DWORD reference, std::string& output)
		{
			return static_cast<BYTE>(F3::GetString(reference, output));
		}
	}
}
