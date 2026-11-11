#include "EngineServer.hpp"
#include "entity/Entity.hpp"
#include <cstring>

namespace JE
{
	JE::Net::UInt32 Server::AttachVisualEffect(Entity* entity,const char* name) //555790.
	{
		DWORD id=AllocateVisualEffectID();
		entity->AttachVisualEffect(name,id,1); //Actual local child completes before publication.
		SendVisualEffectAttach(entity->entityID,name,id);
		return id; //Allocation result is returned even when the child returns NULL normally.
	}
	void Server::SendVisualEffectAttach(DWORD entityID,const char* name,DWORD id) //553E90.
	{
		Net::Packet packet;
		packet.field0x30=18;
		packet.WriteDword(entityID);
		packet.Write(name,JE::Net::UInt32(std::strlen(name)+1));
		packet.WriteDword(id);
		Broadcast(packet,1);
	}
	void Server::RemoveVisualEffect(Entity* entity,JE::Net::UInt32 id,JE::Net::UInt8 force) //5557D0.
	{
		entity->RemoveVisualEffectByID(id,force); //Original ignores whether any binding matched.
		SendVisualEffectRemove(entity->entityID,id,force);
	}
	void Server::SendVisualEffectRemove(DWORD entityID,DWORD id,BYTE force) //553F90.
	{
		Net::Packet packet;
		packet.field0x30=19;
		packet.WriteDword(entityID);
		packet.WriteDword(id);
		packet.WriteByte(force);
		Broadcast(packet,1);
	}
}
