#include "network/Types.hpp"
#include "ClientPickupRequests.hpp"
#include "network/Client.hpp"
#include "network/EngineClient.hpp"
#include "network/Packet.hpp"
#include "entity/Entity.hpp"
#include "entity/World.hpp"
#include "entity/ActionPickUp.hpp"

namespace JE
{
	void Client::RequestPickup(DWORD actorID,DWORD targetID)
	{
		JE::Net::Packet packet;
		packet.field0x30=82;
		packet.WriteDword(actorID);
		packet.WriteDword(targetID);
		Send(packet,1);
	}

}

namespace F3
{
	BYTE ReceiveGameServerPickupRequest(JE::Net::Packet& packet)
	{
		if (packet.field0x30!=82) return 0;
		JE::Net::UInt32 actorID,targetID;
		packet.ReadDword(actorID);
		packet.ReadDword(targetID);
		JE::Entity* actor=JE::GetEntityPtr(actorID);
		JE::Entity* target=JE::GetEntityPtr(targetID);
		if (actor && target)
		{
			//Actual46E328..46E343: factory3, target setter, queue. Unlike
			//nearby packet handlers, this branch does not clear old actions.
			JE::ActionPickUp* action=static_cast<JE::ActionPickUp*>(JE::g_World->NewAction(3));
			action->SetTarget(target);
			actor->QueueAction(action);
		}
		return 1; //Factor consumption flag, not a game pickup-success result.
	}
}
