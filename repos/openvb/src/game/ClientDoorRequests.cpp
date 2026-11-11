#include "network/Types.hpp"
#include "ClientDoorRequests.hpp"
#include "GameActionOperateDoor.hpp"
#include "GameActionOperateContainer.hpp"
#include "GameDoor.hpp"
#include "network/Client.hpp"
#include "network/EngineClient.hpp"
#include "network/Packet.hpp"
#include "entity/Entity.hpp"
#include <new>

namespace JE
{
	void Client::RequestOperate(DWORD actorID,DWORD targetID)
	{
		JE::Net::Packet packet;
		packet.field0x30=78;
		packet.WriteDword(actorID);
		packet.WriteDword(targetID);
		Send(packet,1);
	}

}

namespace F3
{
	BYTE ReceiveGameServerOperateRequest(JE::Net::Packet& packet)
	{
		if (packet.field0x30!=78) return 0;
		JE::Net::UInt32 actorID,targetID;
		packet.ReadDword(actorID);
		packet.ReadDword(targetID);
		JE::Entity* actor=JE::GetEntityPtr(actorID);
		JE::Entity* target=JE::GetEntityPtr(targetID);
		if (actor && target)
		{
			if (target->entityClass==3)
			{
				GameDoor* door=dynamic_cast<GameDoor*>(target);
				GameActionOperateDoor* action=new GameActionOperateDoor;
				action->SetDoor(door);
				actor->QueueAction(action);
			}
			else if (target->entityClass==4 || target->entityClass==1)
			{
				GameActionOperateContainer* action=new GameActionOperateContainer;
				action->SetTarget(target);
				actor->QueueAction(action);
			}
		}
		//Original case78 does not clear actions or add an ownership check.
		return 1; //Adapter consumption flag; actual branch returns void.
	}
}
