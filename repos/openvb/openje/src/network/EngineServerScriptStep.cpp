#include "EngineServer.hpp"
#include "EngineClient.hpp"
#include "gui/DebugWindow.hpp"
#include "startup/NetworkComponent.hpp"
#include <string>

namespace JE
{
	void Server::SetScriptStep(BYTE value) //5567F0.
	{
		if (!value && field0x70E6)
		{
			Client* client = static_cast<Client*>(F3::g_pNetworkInstance->client);
			DebugWindow* window = client->child0x8C;
			field0x70E6 = 0;
			window->Write(std::string("~Done~"), 0, 128, 0);
		}
		else field0x70E6 = value;
	}
}
