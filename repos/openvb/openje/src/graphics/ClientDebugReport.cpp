#include <winsock2.h>
#include "ClientDebugReport.hpp"
#include "entity/Entity.hpp"
#include "game/GameEntity.hpp"
#include "gui/DebugWindow.hpp"
#include "network/EngineClient.hpp"
#include "startup/NetworkComponent.hpp"
#include <cstdio>

namespace F3
{
	BYTE __stdcall DebugPrintEntityInfo(LONG, LONG) //437030; original RET8.
	{
		JE::Entity* entity = JE::GetEntityPtr(static_cast<JE::Client*>(g_pNetworkInstance->client)->field0xDC);
		GameEntity* gameEntity = dynamic_cast<GameEntity*>(entity);
		JE::DebugWindow* debugWindow = static_cast<JE::Client*>(g_pNetworkInstance->client)->child0x8C;
		if (!entity || !debugWindow) return 0;
		char buffer[260];
		std::sprintf(buffer, "**Entity Info**");
		debugWindow->Write(std::string(buffer), 128, 0, 0);
		std::sprintf(buffer, "Instance Name: %s", entity->displayName.c_str());
		debugWindow->Write(std::string(buffer), 128, 128, 128);
		std::sprintf(buffer, "Entity ID #: %d", entity->entityID);
		debugWindow->Write(std::string(buffer), 128, 128, 128);
		std::sprintf(buffer, "Script File name: %s", entity->scriptName.c_str());
		debugWindow->Write(std::string(buffer), 128, 128, 128);
		std::sprintf(buffer, "Team ID and Rank: %d and %d", entity->value368, entity->value36C);
		debugWindow->Write(std::string(buffer), 128, 128, 128);
		std::sprintf(buffer, "Squad ID and Rank: %d and %d", entity->squadID, entity->squadRank);
		debugWindow->Write(std::string(buffer), 128, 128, 128);
		std::sprintf(buffer, "Dialogue File Name: %s", entity->roleName.c_str());
		debugWindow->Write(std::string(buffer), 128, 128, 128);
		std::sprintf(buffer, "Entity File Name: %s", entity->resourceName.c_str());
		debugWindow->Write(std::string(buffer), 128, 128, 128);
		std::sprintf(buffer, "Entity Icon File: %s", entity->inventoryTexture.c_str());
		debugWindow->Write(std::string(buffer), 128, 128, 128);
		std::sprintf(buffer, "Current Map ID: %d", entity->mapHandle);
		debugWindow->Write(std::string(buffer), 128, 128, 128);
		std::sprintf(buffer, "Owner's ID: %d", entity->inventoryOwnerID);
		debugWindow->Write(std::string(buffer), 128, 128, 128);
		if (entity->visibilityOwnerFlag)
			std::sprintf(buffer, "This entity is client controlled.");
		else
			std::sprintf(buffer, "This entity is not client controlled.");
		if (gameEntity)
		{
			//Original overwrites the control-status text and passes three unused
			//sprintf arguments. Both game lines still use ordinary gray output.
			std::sprintf(buffer, "**GameEntity Level Info**", 128, 128, 0);
			debugWindow->Write(std::string(buffer), 128, 128, 128);
			const LONG hitpoints = gameEntity->currentHealth;
			const LONG maximum = gameEntity->GetMaximumHealth();
			std::sprintf(buffer, "Current Hitpoints %d/%d", hitpoints, maximum);
			debugWindow->Write(std::string(buffer), 128, 128, 128);
		}
		//For GameEntity this repeats the hitpoint line. Other entities print
		//the control-status text here; there is no intervening formatting call.
		debugWindow->Write(std::string(buffer), 128, 128, 128);
		std::sprintf(buffer, "**End of Entity Info**");
		debugWindow->Write(std::string(buffer), 128, 0, 0);
		return 1;
	}
}
