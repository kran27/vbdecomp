#include <winsock2.h>
#include "network/EngineServer.hpp"
#include "network/EngineClient.hpp"
#include "gui/DebugWindow.hpp"
#include "startup/NetworkComponent.hpp"
#include "entity/Entity.hpp"
#include "scripting/ScriptSmall.hpp"

namespace JE
{
	void Server::SetBreakpoints(BYTE enabled)
	{
		Client* client = static_cast<Client*>(F3::g_pNetworkInstance->client);
		DebugWindow* window = client->child0x8C;
		if (!enabled)
		{
			field0x70EC = 0;
			if (window) window->Write(std::string("*Breakpoints disabled*"), 128, 0, 0);
		}
		else
		{
			field0x70EC = 1;
			if (window) window->Write(std::string("*Breakpoints enabled*"), 0, 128, 0);
		}
	}

	void Server::SetLocalVariables(BYTE enabled)
	{
		Client* client = static_cast<Client*>(F3::g_pNetworkInstance->client);
		DebugWindow* window = client->child0x8C;
		if (!enabled)
		{
			field0x70ED = 0;
			if (window) window->Write(std::string("*Now Displaying Global & Static Variables*"), 0, 0, 128);
		}
		else
		{
			field0x70ED = 1;
			if (window) window->Write(std::string("*Now Displaying Local Variables*"), 0, 0, 128);
		}
		if (field0x70E8)
		{
			Entity* entity = GetEntityPtr(field0x70E8);
			if (entity)
			{
				Script_Small* script = entity->GetScript();
				if (script) script->ShowDebugState();
			}
		}
	}
}
