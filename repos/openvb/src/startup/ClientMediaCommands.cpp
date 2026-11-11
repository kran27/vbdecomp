#include <winsock2.h>
#include "ClientMediaCommands.hpp"
#include "NetworkComponent.hpp"
#include "network/EngineClient.hpp"
#include "graphics/Video.hpp"

namespace F3
{
	void __cdecl CommandMovieTest(CommandArguments*) //588D00.
	{
		static_cast<JE::Client*>(g_pNetworkInstance->client)->child0xB4->Play(
			"movies\\BISlogo.mve", 30, 418, 640, 320, 0, 0, NULL);
	}

	bool ExecuteClientMediaCommand(JE::Net::UInt32 address, CommandArguments* arguments)
	{
		if (address != 0x588D00) return false;
		CommandMovieTest(arguments);
		return true;
	}
}
