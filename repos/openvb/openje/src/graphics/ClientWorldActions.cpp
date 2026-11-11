#include "ClientWorldActions.hpp"
#include "network/Client.hpp"
#include "network/Packet.hpp"
#include "entity/Entity.hpp"
#include "entity/World.hpp"
#include "entity/ActionMoveTo.hpp"
#include <cstring>
#include <stdexcept>

namespace JE
{
	void SendClientWorldAction(Net::Client& client, DWORD message, DWORD entityID,
		const D3DXVECTOR3& position)
	{
		if (message != 59 && message != 60)
			throw std::logic_error("ClientWorldAction: only actual position/move builders589FD0/58A0E0 recovered");
		Net::Packet packet;
		packet.field0x30 = message;
		packet.WriteDword(entityID);
		Net::UInt32 coordinates[3];
		std::memcpy(coordinates,&position,sizeof(coordinates));
		for (unsigned i = 0; i != 3; ++i) packet.WriteDword(coordinates[i]);
		client.Send(packet,0);
	}

	BYTE ReceiveServerWorldAction(Net::Packet& packet)
	{
		// Original GameServer46DD70 passes these engine messages to556EA0.
		// Client identity is unused by these two branches; there is no ownership
		// authorization check to add here. Unknown messages remain with the caller.
		if (packet.field0x30 != 59 && packet.field0x30 != 60) return 0;
		Net::UInt32 entityID;
		packet.ReadDword(entityID);
		D3DXVECTOR3 position(0,0,0);
		float* values[3] = {&position.x,&position.y,&position.z};
		for (unsigned i = 0; i != 3; ++i)
		{
			Net::UInt32 bits;
			packet.ReadDword(bits);
			std::memcpy(values[i],&bits,sizeof(bits));
		}
		Entity* entity = GetEntityPtr(entityID);
		if (!entity) return 1;
		entity->ClearActions();
		if (packet.field0x30 == 59)
		{
			entity->SetPosition(position);
			entity->QueueAction(g_World->NewAction(1));
		}
		else
		{
			ActionMoveTo* action = static_cast<ActionMoveTo*>(g_World->NewAction(2));
			action->destination = position;
			action->flag34 = 1;
			entity->QueueAction(action);
		}
		return 1;
	}
}
