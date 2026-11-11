#include "EngineServer.hpp"
#include "startup/NetworkComponent.hpp"
#include "entity/World.hpp"
#include "model/CommonException.hpp"
#include "diagnostics/DebugOutput.hpp"

namespace JE
{
	JE::Net::UInt8 Server::RemoveEntityTeam(JE::Net::UInt32 id)
	{
		if (!(F3::g_pNetworkInstance->GetMode() & 1))
			throw ::Common::Exception("Invalid attempt to retrieve world server instance.");
		IWorldServer* world=g_World;
		if (world->GetTeams()->RemoveEntityTeam(id))
		{
			SendTeamRemoval(id);
			return 1;
		}
		return 0;
	}
	void Server::SendTeamRemoval(DWORD id) //5543E0.
	{
		Net::Packet packet;
		packet.field0x30 = 23;
		packet.WriteDword(id);
		Broadcast(packet, 1);
	}
	void Server::SetEntityTeamRank(JE::Net::UInt32 id, JE::Net::UInt32 rank)
	{
		Entity* entity = GetEntityPtr(id);
		if (!entity)
		{
			LogDebugString("EntitySetTeamRank() - Invalid Entity ID %d.\n", id);
			return;
		}
		entity->value36C = rank;
		SendTeamRank(id,rank);
	}
	void Server::SendTeamRank(DWORD id,DWORD rank) //5544A0.
	{
		Net::Packet packet;
		packet.field0x30 = 24;
		packet.WriteDword(id);
		packet.WriteDword(rank);
		Broadcast(packet, 1);
	}
	JE::Net::UInt8 Server::SetTeamAttitude(JE::Net::UInt32 source, JE::Net::UInt32 target, JE::Net::UInt32 attitude)
	{
		if (!(F3::g_pNetworkInstance->GetMode() & 1))
			throw ::Common::Exception("Invalid attempt to retrieve world server instance.");
		IWorldServer* world=g_World;
		if (world->GetTeams()->SetTeamAttitude(source,target,attitude))
		{
			SendTeamAttitude(source,target,attitude);
			return 1;
		}
		return 0;
	}
	void Server::SendTeamAttitude(DWORD source,DWORD target,DWORD attitude) //554580.
	{
		Net::Packet packet;
		packet.field0x30=27;
		packet.WriteDword(source);
		packet.WriteDword(target);
		packet.WriteDword(attitude);
		Broadcast(packet,1);
	}
	JE::Net::UInt32 Server::CreateTeam()
	{
		if (!(F3::g_pNetworkInstance->GetMode() & 1))
			throw ::Common::Exception("Invalid attempt to retrieve world server instance.");
		IWorldServer* world=g_World;
		DWORD id=world->GetTeams()->CreateGroup();
		SendTeamCreation();
		return id;
	}
	void Server::SendTeamCreation() //554670.
	{
		Net::Packet packet;
		packet.field0x30 = 28;
		Broadcast(packet, 1);
	}
	JE::Net::UInt32 Server::CreateSquad()
	{
		if (!(F3::g_pNetworkInstance->GetMode() & 1))
			throw ::Common::Exception("Invalid attempt to retrieve world server instance.");
		IWorldServer* world=g_World;
		DWORD id=world->GetSquads()->CreateGroup();
		SendSquadCreation();
		return id;
	}
	void Server::SendSquadCreation() //554720.
	{
		Net::Packet packet;
		packet.field0x30 = 29;
		Broadcast(packet, 1);
	}
	JE::Net::UInt8 Server::RemoveEntitySquad(JE::Net::UInt32 id)
	{
		if (!(F3::g_pNetworkInstance->GetMode() & 1))
			throw ::Common::Exception("Invalid attempt to retrieve world server instance.");
		IWorldServer* world=g_World;
		if (world->GetSquads()->RemoveEntitySquad(id))
		{
			SendSquadRemoval(id);
			return 1;
		}
		return 0;
	}
	void Server::SendSquadRemoval(DWORD id) //5547D0.
	{
		Net::Packet packet;
		packet.field0x30 = 30;
		packet.WriteDword(id);
		Broadcast(packet, 1);
	}
	void Server::SetEntitySquadRank(JE::Net::UInt32 id, JE::Net::UInt32 rank)
	{
		Entity* entity = GetEntityPtr(id);
		if (!entity)
		{
			LogDebugString("EntitySetSquadRank() - Invalid Entity ID %d\n", id);
			return;
		}
		entity->squadRank = rank;
		SendSquadRank(id,rank);
	}
	void Server::SendSquadRank(DWORD id,DWORD rank) //554890.
	{
		Net::Packet packet;
		packet.field0x30 = 31;
		packet.WriteDword(id);
		packet.WriteSignedDword(static_cast<Net::Int32>(rank));
		Broadcast(packet, 1);
	}
	JE::Net::UInt8 Server::AssignEntitySquad(JE::Net::UInt32 id, JE::Net::UInt32 squad)
	{
		if (!(F3::g_pNetworkInstance->GetMode() & 1))
			throw ::Common::Exception("Invalid attempt to retrieve world server instance.");
		IWorldServer* world=g_World;
		if (world->GetSquads()->AddEntitySquad(id,squad))
		{
			SendEntitySquad(id,squad);
			return 1;
		}
		return 0;
	}
	void Server::SendEntitySquad(DWORD id,DWORD squad) //554970.
	{
		Net::Packet packet;
		packet.field0x30=32;
		packet.WriteDword(id);
		packet.WriteDword(squad);
		Broadcast(packet,1);
	}
	JE::Net::UInt8 Server::SetSquadAttitude(JE::Net::UInt32 source, JE::Net::UInt32 target, JE::Net::UInt32 attitude)
	{
		if (!(F3::g_pNetworkInstance->GetMode() & 1))
			throw ::Common::Exception("Invalid attempt to retrieve world server instance.");
		IWorldServer* world=g_World;
		if (world->GetSquads()->SetSquadAttitude(source,target,attitude))
		{
			SendSquadAttitude(source,target,attitude);
			return 1;
		}
		return 0;
	}
	void Server::SendSquadAttitude(DWORD source,DWORD target,DWORD attitude) //554A50.
	{
		Net::Packet packet;
		packet.field0x30=33;
		packet.WriteDword(source);
		packet.WriteDword(target);
		packet.WriteDword(attitude);
		Broadcast(packet,1);
	}
}
