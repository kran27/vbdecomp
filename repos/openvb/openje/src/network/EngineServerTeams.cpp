#include "EngineServer.hpp"
#include "startup/NetworkComponent.hpp"
#include "entity/World.hpp"
#include "model/CommonException.hpp"

namespace JE
{
	JE::Net::UInt8 Server::AssignEntityTeam(JE::Net::UInt32 id, JE::Net::UInt32 team)
	{
		if (!(F3::g_pNetworkInstance->GetMode() & 1))
			throw ::Common::Exception("Invalid attempt to retrieve world server instance.");
		IWorldServer* world=g_World;
		if (world->GetTeams()->AddEntityTeam(id,team))
		{
			SendEntityTeam(id,team);
			return 1;
		}
		return 0;
	}
	void Server::SendEntityTeam(DWORD id,DWORD team) //554300.
	{
		Net::Packet notification;
		notification.field0x30=22;
		notification.WriteDword(id);
		notification.WriteDword(team);
		Broadcast(notification,1);
	}
}
