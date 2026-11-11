#include <winsock2.h>
#include "ClientWorldCommands.hpp"
#include "NetworkComponent.hpp"
#include "network/EngineClient.hpp"
#include "network/EngineServer.hpp"
#include "entity/World.hpp"
#include "entity/WorldCollections.hpp"
#include "gui/ConsoleInterface.hpp"
#include "gui/FadeManager.hpp"
#include "graphics/WaterSampling.hpp"
#include <cstdlib>
#include <cstring>
#include <list>
#include <string>


namespace F3
{
	void __cdecl CommandStartDialogue(CommandArguments*) //58A7D0.
	{
		static_cast<JE::Client*>(g_pNetworkInstance->client)->StartTestDialogue();
	}

	void __cdecl CommandEndDialogue(CommandArguments*) //58A7E0.
	{
		static_cast<JE::Client*>(g_pNetworkInstance->client)->EndDialogue();
	}

	void __cdecl CommandWarp(CommandArguments* arguments) //58B880.
	{
		if (arguments->size() == 2)
			static_cast<JE::Client*>(g_pNetworkInstance->client)->Warp((*arguments)[1]);
	}

	void __cdecl CommandFadeIn(CommandArguments* arguments) //58BA10.
	{
		const int count = static_cast<int>(arguments->size());
		LONG red = 0;
		LONG green = 0, blue = 0;
		DWORD duration;
		if (count == 2) duration = std::atol((*arguments)[1]);
		else if (count == 5)
		{
			duration = std::atol((*arguments)[1]);
			red = std::atol((*arguments)[2]);
			green = std::atol((*arguments)[3]);
			blue = std::atol((*arguments)[4]);
		}
		else return;
		static_cast<JE::Client*>(g_pNetworkInstance->client)->FadeIn(duration, red, green, blue);
	}

	void __cdecl CommandFadeOut(CommandArguments* arguments) //58BAA0.
	{
		const int count = static_cast<int>(arguments->size());
		LONG red = 0;
		LONG green = 0, blue = 0;
		DWORD duration;
		if (count == 2) duration = std::atol((*arguments)[1]);
		else if (count == 5)
		{
			duration = std::atol((*arguments)[1]);
			red = std::atol((*arguments)[2]);
			green = std::atol((*arguments)[3]);
			blue = std::atol((*arguments)[4]);
		}
		else return;
		static_cast<JE::Client*>(g_pNetworkInstance->client)->FadeOut(duration, red, green, blue);
	}

	void __cdecl CommandStartMap(CommandArguments* arguments) //58DB80.
	{
		if ((g_pNetworkInstance->GetMode() & 1) && arguments->size() >= 2)
		{
			JE::Server* server = static_cast<JE::Server*>(g_pNetworkInstance->server);
			server->startMap.assign((*arguments)[1], std::strlen((*arguments)[1]));
			server->entryPoint = 0;
		}
	}

	void __cdecl CommandCreateEntity(CommandArguments* arguments) //58DDA0.
	{
		if (arguments->size() == 5 || arguments->size() == 2)
		{
			D3DXVECTOR3 position(0, 0, 0);
			if (arguments->size() == 2)
			{
				JE::Client* client = static_cast<JE::Client*>(g_pNetworkInstance->client);
				if (!client->reserved0xE8[0]) return;
				std::memcpy(&position, &client->field0xEC, sizeof(position));
			}
			else
			{
				position.x = static_cast<float>(std::atof((*arguments)[2]));
				position.y = static_cast<float>(std::atof((*arguments)[3]));
				position.z = static_cast<float>(std::atof((*arguments)[4]));
			}
			JE::IWorldClient* world = JE::g_World;
			std::string resource((*arguments)[1]);
			JE::Client* client = static_cast<JE::Client*>(g_pNetworkInstance->client);
			client->RequestCreateEntity(resource, world->GetCurrentMapID(), position, 0);
		}
	}

	void __cdecl CommandSetScript(CommandArguments* arguments) //58DF10.
	{
		if (arguments->size() == 2)
		{
			DWORD entity = static_cast<JE::Client*>(g_pNetworkInstance->client)->field0xDC;
			std::string name((*arguments)[1]);
			static_cast<JE::Client*>(g_pNetworkInstance->client)->RequestSetScript(entity, name);
		}
	}

	void __cdecl CommandToggleFOWLOS(CommandArguments*) //58E340.
	{
		static_cast<JE::Client*>(g_pNetworkInstance->client)->ToggleFOWLOS();
		JE::Client* client = static_cast<JE::Client*>(g_pNetworkInstance->client);
		client->child0x88->Print("Fog of war line of sight: %s", client->field0x100 ? "On" : "Off");
	}

	unsigned char ExecuteClientWorldCommand(JE::Net::UInt32 originalVA, CommandArguments* arguments)
	{
		switch (originalVA)
		{
		case 0x58A7D0: CommandStartDialogue(arguments); return 1;
		case 0x58A7E0: CommandEndDialogue(arguments); return 1;
		case 0x58B880: CommandWarp(arguments); return 1;
		case 0x58BA10: CommandFadeIn(arguments); return 1;
		case 0x58BAA0: CommandFadeOut(arguments); return 1;
		case 0x58DB80: CommandStartMap(arguments); return 1;
		case 0x58DDA0: CommandCreateEntity(arguments); return 1;
		case 0x58DF10: CommandSetScript(arguments); return 1;
		case 0x58E340: CommandToggleFOWLOS(arguments); return 1;
		case 0x58E7E0: CommandToggleWater(arguments); return 1;
		default: return 0;
		}
	}
}
