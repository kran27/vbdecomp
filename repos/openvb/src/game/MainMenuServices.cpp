#include <winsock2.h>
#include "startup/NetworkComponent.hpp"
#include "game/MainMenuServices.hpp"
#include "network/CommandProcessor.hpp"
#include "network/EngineClient.hpp"
#include "gui/Manager.hpp"
#include "game/MainMenuInterface.hpp"
#include "game/GameInterfaces.hpp"
#include "startup/GameClient.hpp"
#include "game/ActionbarInterface.hpp"
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
	OptionsInterface* GetOptionsInterface()
	{
		GameClient* client = static_cast<GameClient*>(g_pNetworkInstance->client);
		return client->Options();
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
	void QueueCommand(const char* command) //580D10, queued rather than dispatched now.
	{
		JE::Client* client = static_cast<JE::Client*>(g_pNetworkInstance->client);
		client->child0x7C->Queue(command);
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
		InventoryInterface* inventory = client->Inventory(); //+18.
		void* auxiliary24 = interfaces->auxiliary24;
		::GUI::Interface* auxiliary30 = interfaces->auxiliary30;
		EscapeMenuInterface* escape = client->EscapeMenu(); //+1C.
		::GUI::Interface* auxiliary3C = interfaces->auxiliary3C;
		StoreInventoryScreen* auxiliary40 = interfaces->auxiliary40;
		::GUI::Interface* auxiliary14 = interfaces->auxiliary14;
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
