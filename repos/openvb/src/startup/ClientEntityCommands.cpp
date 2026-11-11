#include <winsock2.h>
#include "ClientEntityCommands.hpp"
#include "NetworkComponent.hpp"
#include "network/EngineClient.hpp"
#include "entity/Entity.hpp"
#include "entity/Map.hpp"
#include "entity/World.hpp"
#include "graphics/Octree.hpp"
#include "gui/ConsoleInterface.hpp"
#include "diagnostics/DebugOutput.hpp"
#include "../game/GameAnimationCache.hpp"
#include <cctype>
#include <cstdlib>
#include <cstring>

namespace F3
{
	void CommandKill() //58A7B0.
	{
		JE::Client* client = static_cast<JE::Client*>(g_pNetworkInstance->client);
		DWORD entity = client->field0xDC;
		if (entity) client->RequestKill(entity);
	}

	void __cdecl CommandForceEntityStep(CommandArguments* arguments) //460720.
	{
		if (arguments->size() < 3)
		{
			JE::LogDebugString("GameClient::CommandForceEntityStep - Incorrect parameters");
			return;
		}
		DWORD selected = static_cast<JE::Client*>(g_pNetworkInstance->client)->field0xE0;
		if (!selected)
		{
			JE::LogDebugString("GameClient::CommandForceEntityStep - Invalid entity ID");
			return;
		}
		JE::Entity* entity = JE::GetEntityPtr(selected);
		if (!entity)
		{
			JE::LogDebugString("GameClient::CommandForceEntityStep - Failed to find entity");
			return;
		}
		JE::IWorld* world = JE::g_World;
		JE::Map* map = world->GetMap(entity->mapHandle);
		if (!map)
		{
			JE::LogDebugString("GameClient::CommandForceEntityStep - Failed to get map");
			return;
		}
		float x = entity->position.x, z = entity->position.z;
		if (!_stricmp("up", (*arguments)[1])) z = static_cast<float>(std::atof((*arguments)[2]) + z);
		else if (!_stricmp("down", (*arguments)[1])) z = static_cast<float>(z - std::atof((*arguments)[2]));
		else if (!_stricmp("left", (*arguments)[1])) x = static_cast<float>(x - std::atof((*arguments)[2]));
		else if (!_stricmp("right", (*arguments)[1])) x = static_cast<float>(std::atof((*arguments)[2]) + x);
		//Unknown directions retain X/Z and still perform the original floor query.
		float point[3] = {x, 10.0f, z};
		float height;
		if (map->GetOctree()->QueryFloorHeight(point, &height, 1, 0, NULL) == 1)
			static_cast<JE::Client*>(g_pNetworkInstance->client)->RequestForceEntityStep(selected, D3DXVECTOR3(x, height, z));
	}

	void __cdecl CommandTestAnim(CommandArguments* arguments) //460920.
	{
		JE::Client* client = static_cast<JE::Client*>(g_pNetworkInstance->client);
		JE::ConsoleInterface* console = client->child0x88;
		DWORD selected = client->field0xE0;
		JE::Entity* entity;
		if (!selected || (entity = JE::GetEntityPtr(selected)) == NULL)
		{
			console->Print("No entity selected.");
			return;
		}
		if (arguments->size() < 2)
		{
			console->Print("Usage: TestAnim number|name [variation [loopcount [backwards]]]");
			DWORD animation = entity->currentAnimationGroup;
			if (animation < 516)
			{
				const char* direction = entity->animationReverse ? "backwards" : "forwards";
				DWORD variation = entity->currentAnimationIndex;
				const char* suffix = g_animationGroups[animation].c_str();
				console->Print("The selected entities current animation is %d - %s, variation %d, running %s.",
					animation, suffix, variation, direction);
			}
			else console->Print("The selected entities animation is invalid.");
			return;
		}
		DWORD animation;
		if ((*arguments)[1][0] == '_')
		{
			for (animation = 0; animation < 516; ++animation)
			{
				const char* suffix = g_animationGroups[animation].c_str();
				const char* name = (*arguments)[1];
				if (!_stricmp(name, suffix)) break;
			}
			if (animation == 516)
			{
			invalidAnimation:
				console->Print("Invalid animation requested.  Anims can be specified by their enumeration value or group string.");
				return;
			}
		}
		else
		{
			if (!std::isdigit((*arguments)[1][0])) goto invalidAnimation;
			animation = std::atol((*arguments)[1]);
		}
		if (animation >= 516) goto invalidAnimation;
		DWORD variation = 0;
		if (arguments->size() > 2) variation = std::atol((*arguments)[2]);
		WORD loops = 0;
		if (arguments->size() > 3) loops = static_cast<WORD>(std::atol((*arguments)[3]));
		LONG backwards = 0;
		if (arguments->size() > 4) backwards = std::atol((*arguments)[4]);
		BYTE reverse = backwards != 0;
		client->RequestTestAnimation(selected, animation, variation, loops, reverse);
	}

	unsigned char ExecuteClientEntityCommand(JE::Net::UInt32 originalVA, CommandArguments* arguments)
	{
		switch (originalVA)
		{
		case 0x58A7B0: CommandKill(); return 1;
		case 0x460720: CommandForceEntityStep(arguments); return 1;
		case 0x460920: CommandTestAnim(arguments); return 1;
		default: return 0;
		}
	}
}
