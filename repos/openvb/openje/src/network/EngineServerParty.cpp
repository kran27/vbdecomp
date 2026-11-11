#include "EngineServer.hpp"
#include "entity/World.hpp"
#include "entity/WorldCollections.hpp"

namespace JE
{
	JE::Net::UInt8 Server::JoinParty(Entity* entity) //556370.
	{
		IWorld* world = g_World;
		Party* party = world->GetParty();
		BYTE joined = party->Add(entity);
		if (joined)
		{
			SendPartyJoin(entity->entityID);
			joined = AssignEntityTeam(entity->entityID, 1);
		}
		return joined;
	}
	void Server::SendPartyJoin(DWORD entityID) //553DD0.
	{
		Net::Packet notification;
		notification.field0x30 = 16;
		notification.WriteDword(entityID);
		Broadcast(notification, 1);
	}
}
