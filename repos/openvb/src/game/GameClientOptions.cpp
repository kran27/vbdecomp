#include "startup/GameClient.hpp"
#include "startup/NetworkComponent.hpp"
#include "game/OptionsInterface.hpp"
#include "game/InventoryInterface.hpp"
#include "game/PipBoyInterface.hpp"

namespace F3
{
	void GameClient::ToggleOptionsWindow() //45EB70.
	{
		GameInterfaces* interfaces = static_cast<GameInterfaces*>(
			static_cast<JE::Client*>(g_pNetworkInstance->client)->child0xA0);
		InventoryInterface* inventory = interfaces->inventory;
		OptionsInterface* options = interfaces->options;
		PipBoyInterface* pipBoy = interfaces->pipBoy;
		if (options->GetHidden())
		{
			options->SetHidden(0);
			options->Open(1);
			if (pipBoy) pipBoy->SetHidden(1);
			if (inventory) inventory->SetHidden(1);
		}
		else
		{
			options->CloseChildren();
			options->SetHidden(1);
		}
	}
	void GameClient::ToggleOptionsPage(LONG page) //45ED10, wrappers45EDB0..45EDF0.
	{
		GameInterfaces* interfaces = static_cast<GameInterfaces*>(
			static_cast<JE::Client*>(g_pNetworkInstance->client)->child0xA0);
		InventoryInterface* inventory = interfaces->inventory;
		OptionsInterface* options = interfaces->options;
		PipBoyInterface* pipBoy = interfaces->pipBoy;
		if (options->GetHidden())
		{
			options->SetHidden(0);
			options->Open(page);
			if (pipBoy) pipBoy->SetHidden(1);
			if (inventory) inventory->SetHidden(1);
		}
		else if (options->CurrentPage() == page)
		{
			options->CloseChildren();
			options->SetHidden(1);
		}
		else options->SwitchPage(page);
	}
}
