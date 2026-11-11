#include <winsock2.h>
#include "GameEntity.hpp"
#include "../startup/GameServer.hpp"
#include "diagnostics/DebugOutput.hpp"

namespace F3
{
	void GameEntity::SetHealth(LONG health)
	{
		currentHealth = health;
		if (health > maximumHealth) currentHealth = maximumHealth;
	}

	void GameServer::SendEntityHealth(DWORD id, LONG health)
	{
		JE::Net::Packet notification;
		notification.field0x30 = 88;
		notification.WriteDword(id);
		notification.WriteDword(static_cast<DWORD>(health));
		Broadcast(notification, 1);
		JE::LogDebugString("Server - Sending GAME_ID_ENTITY_SETCURRENTHP");
	}
}
