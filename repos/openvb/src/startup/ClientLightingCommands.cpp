#include "ClientLightingCommands.hpp"
#include "NetworkComponent.hpp"
#include "network/EngineClient.hpp"
#include "graphics/Graphics.hpp"

namespace F3
{
	void __cdecl CommandToggleLighting(CommandArguments*) //5882C0; includes original shared587880 tail.
	{
		static_cast<JE::Client*>(g_pNetworkInstance->client)->ToggleLighting();
	}

	unsigned char ExecuteClientLightingCommand(JE::Net::UInt32 originalVA, CommandArguments*)
	{
		if (originalVA != 0x5882C0) return 0;
		CommandToggleLighting(NULL);
		return 1;
	}
}
