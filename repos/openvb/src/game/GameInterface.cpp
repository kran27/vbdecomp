#include <winsock2.h>
#include "game/GameInterface.hpp"
#include "startup/NetworkComponent.hpp"
#include "network/EngineClient.hpp"
#include "graphics/ClientCursor.hpp"
#include "gui/Window.hpp"

namespace F3
{
	std::string g_EmptyInterfaceText(" *  "); //6F34FC; original four-character row-label prefix.
	GameInterface::GameInterface(const std::string& name) : JE::EngineInterface(name) {}
	GameInterface::~GameInterface() {}
	BYTE GameInterface::OnCursor(::GUI::Window* window, LONG, LONG)
	{
		JE::Client* client=static_cast<JE::Client*>(g_pNetworkInstance->client);
		if (window)
		{
			if (window->type==4)
				client->child0xA4->RequestImage("GUI_BeamCursor.tga",16,16,0);
			else
			{
				JE::CursorManager* cursor=client->child0xA4;
				DWORD context=static_cast<GameCursorManager*>(cursor)->context;
				if (context!=1 && context!=0) return 0;
				cursor->RequestImage("GUI_DefaultCursor.tga",2,3,0);
			}
			return 1;
		}
		return 0;
	}
}
