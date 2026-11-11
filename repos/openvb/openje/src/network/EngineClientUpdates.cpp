#include "EngineClient.hpp"
#include "entity/Entity.hpp"
#include "gui/DialogueInterface.hpp"

namespace JE
{
	void Entity::SkipUpdate(Net::Packet& packet) //549250.
	{
		JE::Net::UInt32 ignored;
		packet.ReadDword(ignored);
		packet.ReadDword(ignored);
		packet.ReadDword(ignored);
		packet.ReadDword(ignored);
		packet.ReadDword(ignored);
		packet.ReadDword(ignored);
		packet.ReadDword(ignored);
		packet.ReadDword(ignored);
		packet.ReadDword(ignored);
	}

	JE::Net::UInt8 Client::OnServerUpdate(Net::Packet& packet) //5880C0.
	{
		JE::Net::UInt32 id;
		packet.ReadDword(id);
		while (id)
		{
			Entity* entity=GetEntityPtr(id);
			if (entity) entity->ReadUpdate(packet);
			else Entity::SkipUpdate(packet);
			packet.ReadDword(id);
		}
		child0x9C->ReadUpdate(packet);
		return 1;
	}
}
