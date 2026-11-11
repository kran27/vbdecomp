#include "network/EngineClient.hpp"
#include "startup/NetworkComponent.hpp"
#include "gui/EngineInterface.hpp"
#include "gui/Window.hpp"
#include "gui/Event.hpp"
#include "graphics/ClientCursor.hpp"
#include <stdexcept>

namespace JE
{
	EngineInterface::EngineInterface(const std::string& name) : ::GUI::Interface(name) {}
	EngineInterface::~EngineInterface() {}
	BYTE EngineInterface::OnCursor(::GUI::Window* window, LONG, LONG)
	{
		Client* client=static_cast<Client*>(F3::g_pNetworkInstance->client);
		if (window)
		{
			if (window->type==4) client->child0xA4->RequestImage("GUI_BeamCursor.tga",16,16,0);
			else client->child0xA4->RequestImage("GUI_DefaultCursor.tga",2,3,0);
			return 1;
		}
		return 0;
	}
}
