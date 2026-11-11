#include "EngineServer.hpp"
#include "entity/World.hpp"
#include "startup/NetworkComponent.hpp"
#include "model/CommonException.hpp"
#include <cstring>

namespace JE
{
	void Server::SetGlobal(std::string name, JE::Net::Int32 value)
	{
		if (!(F3::g_pNetworkInstance->GetMode()&1))
			throw ::Common::Exception("Invalid attempt to retrieve world server instance.");
		IWorldServer* world=g_World;
		world->GetGlobalParameters()->SetInteger(name,value,std::string(""));
		SendGlobal(name,value); //556100 runs after mutation, even when unchanged.
	}
	void Server::SendGlobal(std::string name, Net::Int32 value) //556100.
	{
		Net::Packet notification;
		notification.field0x30=65;
		const char* text=name.c_str();
		notification.Write(text,static_cast<JE::Net::UInt32>(std::strlen(text)+1));
		notification.WriteSignedDword(value);
		Broadcast(notification,1);
	}
}
