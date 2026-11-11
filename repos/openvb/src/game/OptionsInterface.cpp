#include "startup/GameClient.hpp"
#include "startup/NetworkComponent.hpp"
#include "game/OptionsInterface.hpp"
#include "game/OptionsServices.hpp"
#include "game/GameOptionsInterface.hpp"
#include "game/HotKeysOptionsInterface.hpp"
#include "game/OptionsLoadInterface.hpp"
#include "game/OptionsMultiplayerInterface.hpp"
#include "game/OptionsVideoInterface.hpp"
#include "gui/Event.hpp"
#include "gui/Label.hpp"
#include "audio/Sound.hpp"
#include <cstring>

namespace F3
{
	namespace
	{
		const DWORD normalColor = DWORD(-1688187372);
		const DWORD selectedColor = DWORD(-933212652);
	}
	OptionsInterface::OptionsInterface(const std::string& name) : JE::EngineInterface(name), GameInterface(name)
	{
		LoadResource("Options.INT");
		rootWindow = FindWindow("picBackground");
		//Each original lookup owns a separate temporary/unwind state.
		labels[0][0] = static_cast<::GUI::Label*>(FindWindow("lblGame"));
		labels[0][1] = static_cast<::GUI::Label*>(FindWindow("lblGameOpt"));
		labels[1][0] = static_cast<::GUI::Label*>(FindWindow("lblHotkeys"));
		labels[1][1] = static_cast<::GUI::Label*>(FindWindow("lblHotkeysOpt"));
		labels[2][0] = static_cast<::GUI::Label*>(FindWindow("lblLoad"));
		labels[2][1] = static_cast<::GUI::Label*>(FindWindow("lblLoadOpt"));
		labels[3][0] = static_cast<::GUI::Label*>(FindWindow("lblMultiplayer"));
		labels[3][1] = static_cast<::GUI::Label*>(FindWindow("lblMultiplayerOpt"));
		labels[4][0] = static_cast<::GUI::Label*>(FindWindow("lblVideo"));
		labels[4][1] = static_cast<::GUI::Label*>(FindWindow("lblVideoOpt"));
	}
	OptionsInterface::~OptionsInterface() {}
	void OptionsInterface::Reset() //442BF0.
	{
		JE::Client* client = static_cast<JE::Client*>(g_pNetworkInstance->client);
		GameInterfaces* interfaces = static_cast<GameInterfaces*>(client->child0xA0);
		GameOptionsInterface* game = interfaces->optionsGame;
		HotKeysOptionsInterface* hotKeys = interfaces->optionsHotKeys;
		OptionsLoadInterface* load = interfaces->optionsLoad;
		OptionsMultiplayerInterface* multiplayer = interfaces->optionsMultiplayer;
		OptionsVideoInterface* video = interfaces->optionsVideo;
		game->SetHidden(1);
		hotKeys->SetHidden(1);
		load->SetHidden(1);
		multiplayer->SetHidden(1);
		video->SetHidden(1);
		labels[0][0]->SetTextColor(normalColor);
		labels[0][1]->SetTextColor(normalColor);
		labels[1][0]->SetTextColor(normalColor);
		labels[1][1]->SetTextColor(normalColor);
		labels[2][0]->SetTextColor(normalColor);
		labels[2][1]->SetTextColor(normalColor);
		labels[3][0]->SetTextColor(normalColor);
		labels[3][1]->SetTextColor(normalColor);
		labels[4][0]->SetTextColor(normalColor);
		labels[4][1]->SetTextColor(normalColor);
	}
	LONG OptionsInterface::CurrentPage() const //442D10.
	{
		JE::Client* client = static_cast<JE::Client*>(g_pNetworkInstance->client);
		GameInterfaces* interfaces = static_cast<GameInterfaces*>(client->child0xA0);
		GameOptionsInterface* game = interfaces->optionsGame;
		HotKeysOptionsInterface* hotKeys = interfaces->optionsHotKeys;
		OptionsLoadInterface* load = interfaces->optionsLoad;
		OptionsMultiplayerInterface* multiplayer = interfaces->optionsMultiplayer;
		OptionsVideoInterface* video = interfaces->optionsVideo;
		if (!game->GetHidden()) return 1;
		if (!hotKeys->GetHidden()) return 2;
		if (!load->GetHidden()) return 3;
		if (!multiplayer->GetHidden()) return 4;
		return !video->GetHidden() ? 5 : 0;
	}
	BYTE OptionsInterface::SwitchPage(LONG target) //442DC0.
	{
		JE::Client* client = static_cast<JE::Client*>(g_pNetworkInstance->client);
		GameInterfaces* interfaces = static_cast<GameInterfaces*>(client->child0xA0);
		OptionsVideoInterface* video = interfaces->optionsVideo;
		HotKeysOptionsInterface* hotKeys = interfaces->optionsHotKeys;
		OptionsLoadInterface* load = interfaces->optionsLoad;
		OptionsMultiplayerInterface* multiplayer = interfaces->optionsMultiplayer;
		GameOptionsInterface* game = interfaces->optionsGame;
		if (!game->GetHidden())
		{
			if (target == 1) return 0;
			game->SetHidden(1);
			labels[0][0]->SetTextColor(normalColor);
			labels[0][1]->SetTextColor(normalColor);
		}
		if (!hotKeys->GetHidden())
		{
			if (target == 2) return 0;
			hotKeys->Commit();
			hotKeys->SetHidden(1);
			labels[1][0]->SetTextColor(normalColor);
			labels[1][1]->SetTextColor(normalColor);
		}
		if (!load->GetHidden())
		{
			if (target == 3) return 0;
			load->SetHidden(1);
			labels[2][0]->SetTextColor(normalColor);
			labels[2][1]->SetTextColor(normalColor);
		}
		if (!multiplayer->GetHidden())
		{
			if (target == 4) return 0;
			multiplayer->SetHidden(1);
			labels[3][0]->SetTextColor(normalColor);
			labels[3][1]->SetTextColor(normalColor);
		}
		if (!video->GetHidden())
		{
			if (target == 5) return 0;
			video->SetHidden(1);
			labels[4][0]->SetTextColor(normalColor);
			labels[4][1]->SetTextColor(normalColor);
		}
		switch (target)
		{
		case 1:
			labels[0][0]->SetTextColor(selectedColor);
			labels[0][1]->SetTextColor(selectedColor);
			game->SetHidden(0);
			game->BringToFront();
			break;
		case 2:
			labels[1][0]->SetTextColor(selectedColor);
			labels[1][1]->SetTextColor(selectedColor);
			hotKeys->PrepareForOpen();
			hotKeys->SetHidden(0);
			hotKeys->BringToFront();
			break;
		case 3:
			labels[2][0]->SetTextColor(selectedColor);
			labels[2][1]->SetTextColor(selectedColor);
			load->ClearSelection();
			load->SetHidden(0);
			load->BringToFront();
			break;
		case 4:
			labels[3][0]->SetTextColor(selectedColor);
			labels[3][1]->SetTextColor(selectedColor);
			multiplayer->SetHidden(0);
			multiplayer->BringToFront();
			break;
		case 5:
			labels[4][0]->SetTextColor(selectedColor);
			labels[4][1]->SetTextColor(selectedColor);
			video->PrepareForOpen();
			video->SetHidden(0);
			video->BringToFront();
			break;
		}
		return 1;
	}

	BYTE OptionsInterface::Open(LONG page) { Reset(); return SwitchPage(page); }
	BYTE OptionsInterface::CloseChildren() { return SwitchPage(0); }
	void OptionsInterface::PlayTabSound() //443130.
	{
		JE::CSound* sound = new JE::CSound;
		if (sound)
		{
			sound->Load("f3_button_down_02.wav", 0);
			sound->Play(2, 0, 0);
		}
	}
	int OptionsInterface::OnEvent(::GUI::Window* window, ::GUI::Event* event)
	{
		if (event->type == 4)
		{
			if (!_stricmp(window->GetName(), "picGameTab"))
			{ if (SwitchPage(1)) PlayTabSound(); }
			else if (!_stricmp(window->GetName(), "picHotKeysTab"))
			{ if (SwitchPage(2)) PlayTabSound(); }
			else if (!_stricmp(window->GetName(), "picLoadTab"))
			{ if (SwitchPage(3)) PlayTabSound(); }
			else if (!_stricmp(window->GetName(), "picMultiplayerTab"))
			{ if (SwitchPage(4)) PlayTabSound(); }
			else if (!_stricmp(window->GetName(), "picVideoTab"))
			{ if (SwitchPage(5)) PlayTabSound(); }
		}
		return JE::EngineInterface::OnEvent(window, event);
	}
}
