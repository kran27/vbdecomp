#include "startup/GameClient.hpp"
#include "startup/NetworkComponent.hpp"
#include "game/EscapeMenuInterface.hpp"
#include "graphics/ClientCursor.hpp"
#include "gui/Manager.hpp"
#include <stdexcept>

namespace F3
{
	void GameClient::ToggleEscapeMenu() //45EC00.
	{
		GameClient* client = static_cast<GameClient*>(g_pNetworkInstance->client);
		EscapeMenuInterface* menu = client->Interfaces()->escapeMenu;
		GameCursorManager* cursor = static_cast<GameCursorManager*>(client->child0xA4);
		if (cursor && cursor->CancelSpecialContext()) return;
		if (menu->GetHidden())
		{
			menu->SetHidden(0);
			menu->BringToFront();
			::GUI::g_Manager.PushModal(menu);
		}
		else menu->SetHidden(1); //Original command does not RemoveModal here.
	}
}
