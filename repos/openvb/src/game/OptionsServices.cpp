#include "startup/GameClient.hpp"
#include "startup/NetworkComponent.hpp"
#include "game/OptionsServices.hpp"
#include "game/OptionsInterface.hpp"
#include "game/ActionbarServices.hpp"
#include "game/GameHotKeys.hpp"
#include "gui/EngineInterfaceServices.hpp"
#include <stdexcept>

namespace F3 { namespace OptionsServices
{
	OptionsInterface* GetOptions()
	{
		GameClient* client = ActionbarServices::GetClient();
		OptionsInterface* options = client->Options();
		return options;
	}
	JE::ClientHotKeys& HotKeys()
	{
		return *static_cast<JE::Client*>(g_pNetworkInstance->client)->child0x80;
	}

	JE::ClientHotKeys* DefaultsHotKeys()
	{
		return dynamic_cast<GameHotKeys*>(
			static_cast<JE::Client*>(g_pNetworkInstance->client)->child0x80);
	}
	const char* KeyName(DWORD scan)
	{
		return static_cast<JE::Client*>(g_pNetworkInstance->client)->child0x78->KeyName(scan);
	}
	void CloseOptions()
	{
		OptionsInterface* options = GetOptions();
		options->CloseChildren();
		options->SetHidden(1);
	}
	void PlayTabSound()
	{ JE::EngineInterfaceServices::RequestTransientSound("f3_button_down_02.wav", 0, 2, 0, 0); }
	void PlayToggleSound()
	{ JE::EngineInterfaceServices::RequestTransientSound("Toggle.wav", 0, 2, 0, 0); }
} }
