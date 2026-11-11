#include "EngineServer.hpp"
#include "entity/Entity.hpp"
#include <cstring>

namespace JE
{
	namespace
	{
		JE::Net::UInt32 FloatBits(float value)
		{
			JE::Net::UInt32 bits;
			std::memcpy(&bits,&value,sizeof bits);
			return bits;
		}
	}

	void Entity::WriteUpdate(Net::Packet& output) //54A060.
	{
		output.WriteDword(FloatBits(position.x));
		output.WriteDword(FloatBits(position.y));
		output.WriteDword(FloatBits(position.z));
		output.WriteDword(FloatBits(rotation.x));
		output.WriteDword(FloatBits(rotation.y));
		output.WriteDword(FloatBits(rotation.z));
		output.WriteDword(currentAnimationGroup);
		output.WriteDword(currentAnimationIndex);
		//54A0F6 reads the live animation clock only after the preceding writes.
		output.WriteDword(GetAnimationTime());
	}

	void Server::PrepareUpdate(JE::Net::UInt32 index,Net::Packet& output) //5552C0.
	{
		packet.Reset();
		Net::EngineServerClientConnection* connection=
			static_cast<Net::EngineServerClientConnection*>(connections[index]);
		for (std::list<Entity*>::iterator entry=g_createdEntities.begin();
			entry!=g_createdEntities.end(); ++entry)
		{
			Entity* entity=*entry;
			if (entity->mapHandle==connection->field0x416C && entity->model)
			{
				packet.WriteDword(entity->entityID);
				entity->WriteUpdate(packet);
			}
		}
		packet.WriteSignedDword(0);
		const JE::Net::UInt32 length=packet.field0x10;
		packet.field0x0C=0;
		output.CopyFrom(packet,length);
		//CreateDialogue552A60 supplies the actual manager; this call is direct
		//55E110 in the original, not a successful fallback on ServerDialogue.
		static_cast<DialogueManager*>(dialogue)->WriteUpdate(output);
	}
}
