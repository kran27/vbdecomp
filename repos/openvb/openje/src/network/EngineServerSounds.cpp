#include "EngineServer.hpp"
#include "EngineServerSoundServices.hpp"
#include "entity/Entity.hpp"
#include <cstring>

namespace JE
{
	namespace EngineServerSoundServices
	{
		UnconsumedRequests* g_UnconsumedRequests=NULL;
		void AttachSound(Entity* entity, const char* filename, BYTE looping, DWORD id)
		{
			if (g_UnconsumedRequests)
				g_UnconsumedRequests->CaptureAttachSound(entity,filename,looping,id);
			else entity->AttachSound(filename,looping,id);
		}
	}
	JE::Net::UInt32 Server::AttachSound(Entity* entity, const char* filename, JE::Net::UInt8 looping)
	{
		DWORD id=AllocateSoundID(); //555816: allocation precedes required local child.
		entity->AttachSound(filename,looping,id);
		SendSoundAttach(entity->entityID,filename,looping,id);
		return id;
	}
	void Server::SendSoundAttach(DWORD entityID,const char* filename,BYTE looping,DWORD id) //5540C0.
	{
		Net::Packet notification;
		notification.field0x30=20;
		notification.WriteDword(entityID);
		notification.Write(filename,static_cast<JE::Net::UInt32>(std::strlen(filename)+1));
		notification.WriteSignedDword(static_cast<Net::Int8>(looping!=0));
		notification.WriteDword(id);
		Broadcast(notification,1);
	}
	void Server::RemoveSound(Entity* entity, JE::Net::UInt32 id, JE::Net::UInt8 force) //555850.
	{
		entity->RemoveSound(id,force); //Required local child precedes publication; result ignored.
		SendSoundRemove(entity->entityID,id,force);
	}
	void Server::SendSoundRemove(DWORD entityID,DWORD id,BYTE force) //5541D0.
	{
		Net::Packet notification;
		notification.field0x30=21;
		notification.WriteDword(entityID);
		notification.WriteDword(id);
		notification.WriteByte(force);
		Broadcast(notification,1);
	}
}
