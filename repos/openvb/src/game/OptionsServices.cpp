#include "startup/GameClient.hpp"
#include "startup/NetworkComponent.hpp"
#include "game/OptionsServices.hpp"
#include "game/OptionsInterface.hpp"
#include "game/HotKeysOptionsInterface.hpp"
#include "game/ActionbarServices.hpp"
#include "game/PipBoyInterface.hpp"
#include "game/GameHotKeys.hpp"
#include "gui/EngineInterfaceServices.hpp"
#include <stdexcept>

namespace F3
{
	void __stdcall ReplaceCommandBinding(LONG scan, DWORD subtype, DWORD modifiers,
		const char* command) //Host compatibility transport; no native entry.
	{
		OptionsServices::GetBinding().pages.hotKeys->ReplaceCommandBinding(scan, subtype, modifiers, command);
	}
}

namespace F3 { namespace OptionsServices
{
	namespace
	{
		Binding* binding = NULL;
		Binding parentProjection; //Borrowed pointers only; never constructs child owners.
	}
	Binding* SetBinding(Binding* value)
	{
		Binding* previous = binding;
		binding = value;
		return previous;
	}
	const Binding& GetBinding()
	{
		GameClient* client = g_pNetworkInstance && g_pNetworkInstance->client
			? ActionbarServices::GetClient() : NULL;
		GameInterfaces* interfaces = client ? client->Interfaces() : NULL;
		if (interfaces)
		{
			//Keep real input owners from the external entry while projecting the
			//actual parent's already constructed page pointers on every lookup.
			parentProjection = binding ? *binding : Binding();
			parentProjection.pages.game = interfaces->optionsGame;
			parentProjection.pages.hotKeys = interfaces->optionsHotKeys;
			parentProjection.pages.load = interfaces->optionsLoad;
			parentProjection.pages.multiplayer = interfaces->optionsMultiplayer;
			parentProjection.pages.video = interfaces->optionsVideo;
			parentProjection.pipBoy = interfaces->pipBoy;
			return parentProjection;
		}
		if (!binding) throw std::logic_error("Options: actual sibling/input binding required");
		return *binding;
	}
	OptionsInterface* GetOptions()
	{
		GameClient* client = ActionbarServices::GetClient();
		OptionsInterface* options = client->Options();
		if (!options)
			throw std::logic_error("Options: actual interface owner required");
		return options;
	}
	JE::ClientHotKeys& HotKeys()
	{
		if (binding)
		{
			if (!GetBinding().keys) throw std::logic_error("Options: actual HotKeys owner required");
			return *GetBinding().keys;
		}
		return *static_cast<JE::Client*>(g_pNetworkInstance->client)->child0x80;
	}
	void ReplaceCommandBinding(LONG scan, DWORD subtype, DWORD modifiers,
		const char* command) //Host adapter; no native entry.
	{
		if (!binding)
		{
			F3::ReplaceCommandBinding(scan, subtype, modifiers, command);
			return;
		}
		JE::ClientHotKeys& keys = HotKeys();
		DWORD previousScan = DWORD(-1), previousSubtype, previousModifiers;
		while (keys.FindBinding(previousScan, previousSubtype, previousModifiers,
			command) == 1)
		{
			keys.Unbind(previousScan, previousSubtype, previousModifiers);
		}
		if (scan != -1)
			keys.Bind(scan, subtype, modifiers, command);
	}

	JE::ClientHotKeys* DefaultsHotKeys()
	{
		if (binding) return GetBinding().defaultKeys;
		return dynamic_cast<GameHotKeys*>(
			static_cast<JE::Client*>(g_pNetworkInstance->client)->child0x80);
	}
	const char* KeyName(DWORD scan)
	{
		if (binding) return HotKeys().input.KeyName(scan);
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
