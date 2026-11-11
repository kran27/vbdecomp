#include <winsock2.h>
#include "startup/NetworkComponent.hpp"
#include "game/MainMenuServices.hpp"
#include "network/CommandProcessor.hpp"
#include "network/EngineClient.hpp"
#include "gui/FadeManager.hpp"
#include "gui/Manager.hpp"
#include "game/MainMenuInterface.hpp"
#include "game/GameInterfaces.hpp"
#include "game/LoadingBarInterface.hpp"
#include "game/CharacterGenerationInterface.hpp"
#include "graphics/CameraControl.hpp"
#include "startup/GameClient.hpp"
#include "startup/GameClientBinding.hpp"
#include "graphics/ClientCursor.hpp"
#include "game/ActionbarInterface.hpp"
#include "game/InventoryInputServices.hpp"
#include "game/InventoryInterface.hpp"
#include "game/EscapeMenuInterface.hpp"
#include "game/FeedbackInterface.hpp"
#include "game/PipBoyInterface.hpp"
#include "game/OptionsInterface.hpp"
#include "graphics/FloatingTextInterface.hpp"
#include "entity/World.hpp"
#include "resources/ResourceSystem.hpp"
#include "configuration/ConfigurationGetters.hpp"
#include "startup/EngineSettings.hpp"
#include <stdexcept>

namespace F3 { namespace MainMenuServices
{
	namespace { Binding* binding = NULL; }
	Binding* SetBinding(Binding* value)
	{
		Binding* previous = binding;
		binding = value;
		return previous;
	}
	Binding& GetBinding()
	{
		if (!binding) throw std::logic_error("Mainmenu: actual standalone screen binding required");
		return *binding;
	}
	OptionsInterface* GetOptionsInterface()
	{
		GameClient* client = dynamic_cast<GameClient*>(g_pNetworkInstance->client);
		if (!client)
			throw std::logic_error("CharacterGeneration: actual GameInterfaces Options binding unresolved");
		return client->Options();
	}
	void ShowMainMenu()
	{
		JE::Client* client = static_cast<JE::Client*>(g_pNetworkInstance->client);
		JE::InitializeMainmenuCamera(*client->child0x84);
		GameInterfaces* interfaces = static_cast<GameClient*>(client)->Interfaces();
		MainMenuInterface* menu = interfaces ? interfaces->mainMenu : GetBinding().mainMenu;
		menu->SetHidden(0);
		::GUI::g_Manager.PushModal(menu);
	}
	void HideGameInterfaces() //Standalone convenience adapter; actual45E6C0 is the member.
	{
		static_cast<GameClient*>(g_pNetworkInstance->client)->HideInterfaces();
	}
	void ReturnToMainMenu() //433BE0, original existing-owner path.
	{
		std::string map("none.map");
		const JE::Configuration::Settings* settings = GetEngineSettings();
		JE::Configuration::Settings::const_iterator section = settings->find(std::string("Mainmenu"));
		if (section != settings->end())
		{
			const JE::Configuration::Values& entries = section->second;
			JE::Configuration::Values::const_iterator value = entries.find(std::string("Map Name"));
			if (value != entries.end()) map = value->second;
		}
		GameClient* client = static_cast<GameClient*>(g_pNetworkInstance->client);
		client->child0xAC->Clear();
		if (JE::Resources::g_pResourceSystem->Contains(map.c_str()))
		{
			JE::IWorldClient* world = JE::g_World;
			world->Reset();
			world = JE::g_World;
			world->LoadMap(map.c_str()); //Original does not inspect the return.
			{
				std::string activeMap(map.c_str());
				world = JE::g_World;
				world->SetActiveMap(activeMap);
			}
			world = JE::g_World;
			client->PublishMapID(world->GetCurrentMapID());
		}
		client->HideInterfaces();
		client->InitializeMainmenuCamera();
		GameInterfaces* interfaces = static_cast<GameInterfaces*>(client->child0xA0);
		MainMenuInterface* menu = interfaces->mainMenu;
		menu->SetHidden(0);
		::GUI::g_Manager.PushModal(menu);
	}
	void SetLoadingVisible(BYTE visible)
	{
		GameInterfaces* interfaces = static_cast<GameClient*>(g_pNetworkInstance->client)->Interfaces();
		if (interfaces) { interfaces->SetLoadingVisible(visible); return; }
		LoadingBarInterface* loading = GetBinding().loading;
		if (loading) loading->SetHidden(visible ? 0 : 1);
		//The original following hook is empty.
	}
	BYTE HandleSceneTransition(JE::Client& client, DWORD address)
	{
		GameInterfaces* interfaces = static_cast<GameClient&>(client).Interfaces();
		if (!interfaces && !binding) return 0;
		switch (address)
		{
		case 0x589F78: SetLoadingVisible(1); return 1;
		case 0x58C51F:
		case 0x58C5BD:
		case 0x58D42F: SetLoadingVisible(0); return 1;
		case 0x58C52A: client.child0xA4->OnUnused(); return 1;
		case 0x58DADC: //Actual GameInterfaces439810, virtual+1C selection refresh.
		{
			if (interfaces) { interfaces->RefreshSelection(); return 1; }
			ActionbarInterface* actionbar = GetGameClientBinding(static_cast<GameClient&>(client)).borrowedActionbar;
			if (actionbar && !actionbar->GetHidden())
				actionbar->SelectEntity(static_cast<JE::Client*>(g_pNetworkInstance->client)->field0xE0);
			return 1;
		}
		case 0x58C53C:
		{
			if (interfaces) { interfaces->ShowGameplay(); return 1; }
			//439770 reloads the live GameClient before its required cursor call.
			GameClient* game = dynamic_cast<GameClient*>(g_pNetworkInstance->client);
			InventoryInputServices::SetContext(*static_cast<GameCursorManager*>(game->child0xA4),1);
			ActionbarInterface* actionbar = GetGameClientBinding(static_cast<GameClient&>(client)).borrowedActionbar;
			if (actionbar)
			{
				const DWORD selected = static_cast<JE::Client*>(g_pNetworkInstance->client)->field0xE0;
				actionbar->SetHidden(0);
				actionbar->RefreshEntity(selected);
			}
			return 1;
		}
		default: return 0;
		}
	}
	void QueueCommand(const char* command) //580D10, queued rather than dispatched now.
	{
		JE::Client* client = static_cast<JE::Client*>(g_pNetworkInstance->client);
		client->child0x7C->Queue(command);
	}

	void FadeOut(DWORD duration, DWORD red, DWORD green, DWORD blue)
	{
		JE::Client* client = static_cast<JE::Client*>(g_pNetworkInstance->client);
		client->child0xA8->FadeOut(duration, LONG(red), green, blue);
	}

	void OpenCharacterSelection()
	{
		GameInterfaces* interfaces = static_cast<GameClient*>(g_pNetworkInstance->client)->Interfaces();
		CharacterGenerationInterface* screen = interfaces ? interfaces->characterGeneration : GetBinding().characterGeneration;
		if (!screen) throw std::logic_error("Mainmenu: actual character-generation owner required");
		screen->Open();
	}

	void FadeIn(DWORD duration, DWORD red, DWORD green, DWORD blue)
	{
		JE::Client* client = static_cast<JE::Client*>(g_pNetworkInstance->client);
		client->child0xA8->FadeIn(duration, LONG(red), green, blue);
	}
} }

namespace F3
{
	void EscapeMenuInterface::ReturnToMainMenu()
	{
		//The original caller supplies this receiver. The existing policy factor
		//still has an unresolved Settings find boundary and earns no exact credit.
		MainMenuServices::ReturnToMainMenu();
	}
}

namespace F3
{
	void GameClient::HideInterfaces() //45E6C0; incoming receiver is unused in native.
	{
		GameClient* client = static_cast<GameClient*>(g_pNetworkInstance->client);
		GameInterfaces* interfaces = client->Interfaces();
		if (!interfaces && !MainMenuServices::GetBinding().freshAuxiliaries)
			throw std::logic_error("GameClient45E6C0: populated standalone auxiliary interfaces unresolved");
		InventoryInterface* inventory = client->Inventory(); //+18.
		void* auxiliary24 = interfaces ? interfaces->auxiliary24 : NULL;
		::GUI::Interface* auxiliary30 = interfaces ? interfaces->auxiliary30 : NULL;
		EscapeMenuInterface* escape = client->EscapeMenu(); //+1C.
		::GUI::Interface* auxiliary3C = interfaces ? interfaces->auxiliary3C : NULL;
		StoreInventoryScreen* auxiliary40 = interfaces ? interfaces->auxiliary40 : NULL;
		::GUI::Interface* auxiliary14 = interfaces ? interfaces->auxiliary14 : NULL;
		ActionbarInterface* actionbar = client->Actionbar(); //+10.
		FeedbackInterface* feedback = client->Feedback(); //+0C.
		PipBoyInterface* pipboy = client->PipBoy(); //+44.
		OptionsInterface* options = client->Options(); //+4C.
		if (inventory) inventory->SetHidden(1);
		if (auxiliary24)
			throw std::logic_error("GameClient45E6C0: populated auxiliary24 texture owner430760 unresolved");
		if (auxiliary30) auxiliary30->SetHidden(1);
		if (escape) escape->SetHidden(1);
		if (auxiliary3C) auxiliary3C->SetHidden(1);
		if (auxiliary40) auxiliary40->SetHidden(1);
		if (auxiliary14) auxiliary14->SetHidden(1);
		if (actionbar) actionbar->SetHidden(1);
		if (feedback) feedback->SetHidden(1);
		if (pipboy) pipboy->SetHidden(1);
		if (options)
		{
			options->CloseChildren();
			options->SetHidden(1);
		}
	}
}
