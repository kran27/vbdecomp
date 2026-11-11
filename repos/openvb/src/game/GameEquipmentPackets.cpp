#include <winsock2.h>
#include "GameEquipmentPackets.hpp"
#include "GameEntity.hpp"
#include "startup/GameServer.hpp"
#include "network/EngineServer.hpp"
#include "diagnostics/DebugOutput.hpp"
#include <cstring>

namespace F3
{
	void GameServer::SendPrimaryAmmo(DWORD entityID, LONG modifier, DWORD count)
	{
		JE::Net::Packet packet; packet.field0x30=102;
		packet.WriteDword(entityID); packet.WriteSignedDword(modifier); packet.WriteDword(count);
		Broadcast(packet,1);
	}
	void GameServer::SendSecondaryAmmo(DWORD entityID, LONG modifier, DWORD count)
	{
		JE::Net::Packet packet; packet.field0x30=103;
		packet.WriteDword(entityID); packet.WriteSignedDword(modifier); packet.WriteDword(count);
		Broadcast(packet,1);
	}
	void GameServer::SendNewAmmoStack(DWORD ownerID, DWORD stackID, LONG caliber, LONG modifier)
	{
		JE::Net::Packet packet; packet.field0x30=92;
		packet.WriteDword(ownerID); packet.WriteDword(stackID);
		packet.WriteSignedDword(caliber); packet.WriteSignedDword(modifier);
		Broadcast(packet,1);
	}
	namespace
	{
		DWORD FloatWord(float value)
		{
			DWORD bits;
			std::memcpy(&bits,&value,sizeof(bits));
			return bits;
		}
	}
	void GameServer::SetMoveMode(DWORD entityID, DWORD mode, float speed)
	{
		GameEntity* entity = dynamic_cast<GameEntity*>(JE::GetEntityPtr(entityID));
		if (!entity) return;
		entity->value90 = mode;
		std::memcpy(&entity->value94, &speed, sizeof(speed));
		SendMoveMode(entityID, mode, speed);
	}
	void GameServer::SendMoveMode(DWORD entityID, DWORD mode, float speed)
	{
		JE::Net::Packet notification;
		notification.field0x30=86;
		notification.WriteDword(entityID);
		notification.WriteDword(mode);
		notification.WriteDword(FloatWord(speed));
		Broadcast(notification,1);
	}
	void GameServer::SendEntityDamage(DWORD entityID, DWORD damage, DWORD damageType)
	{
		JE::Net::Packet notification;
		notification.field0x30=85;
		notification.WriteDword(entityID);
		notification.WriteDword(damage);
		notification.WriteDword(damageType);
		Broadcast(notification,1);
		JE::LogDebugString("Server - Sending GAME_ID_ENTITY_DAMAGE.\n");
	}
	void GameServer::SendEquippedItem(DWORD ownerID, DWORD itemID,
		DWORD sourceID, DWORD slot, BYTE add)
	{
		JE::Net::Packet packet;
		packet.field0x30=95;
		packet.WriteByte(add);
		packet.WriteDword(ownerID);
		packet.WriteDword(itemID);
		packet.WriteDword(sourceID);
		packet.WriteDword(slot);
		Broadcast(packet,1);
	}
	void GameServer::SendUnequippedItem(DWORD ownerID, DWORD itemID, BYTE add)
	{
		JE::Net::Packet packet;
		packet.field0x30=96;
		packet.WriteByte(add);
		packet.WriteDword(ownerID);
		packet.WriteDword(itemID);
		Broadcast(packet,1);
	}
	void GameServer::SendActiveWeapon(DWORD ownerID, DWORD slot)
	{
		JE::Net::Packet packet;
		packet.field0x30=89;
		packet.WriteDword(ownerID);
		packet.WriteDword(slot);
		Broadcast(packet,1);
	}
}
