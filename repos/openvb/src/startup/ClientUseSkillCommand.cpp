#include <winsock2.h>
#include "ClientUseSkillCommand.hpp"
#include "GameClient.hpp"
#include "NetworkComponent.hpp"
#include "graphics/ClientCursor.hpp"

namespace F3
{
	void __cdecl CommandUseSkill(CommandArguments*) //45EE30.
	{
		JE::Client* engineClient = static_cast<JE::Client*>(g_pNetworkInstance->client);
		GameClient* client = dynamic_cast<GameClient*>(engineClient);
		static_cast<GameCursorManager*>(client->child0xA4)->SetContext(16);
	}

	unsigned char ExecuteClientUseSkillCommand(JE::Net::UInt32 originalVA,
		CommandArguments* arguments)
	{
		if (originalVA != 0x45EE30) return 0;
		CommandUseSkill(arguments);
		return 1;
	}
}
