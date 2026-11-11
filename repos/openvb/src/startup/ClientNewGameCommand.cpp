#include "startup/ClientNewGameCommand.hpp"
#include "startup/NetworkComponent.hpp"
#include "network/EngineClient.hpp"
#include "network/EngineServer.hpp"
#include <cstring>

namespace F3
{
	void RequestNewGame(CommandArguments* arguments) //58DBE0.
	{
		//The original token count includes the command itself. An optional map
		//argument changes only the actual local server's startMap/entryPoint.
		if (arguments->size() == 2 && (g_pNetworkInstance->GetMode() & 1))
		{
			JE::Server* server = static_cast<JE::Server*>(g_pNetworkInstance->server);
			server->startMap.assign((*arguments)[1], std::strlen((*arguments)[1]));
			server->entryPoint = 0;
		}
		static_cast<JE::Client*>(g_pNetworkInstance->client)->RequestNewGame();
	}
}
