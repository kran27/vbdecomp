#include "network/Types.hpp"
#include "ClientUseRequests.hpp"
#include "network/Client.hpp"
#include "network/EngineClient.hpp"
#include "network/Packet.hpp"
#include "entity/Entity.hpp"

namespace JE
{
	void Client::RequestLook(DWORD actorID,DWORD targetID)
	{
		JE::Net::Packet packet;
		packet.field0x30=81;
		packet.WriteDword(actorID);
		packet.WriteDword(targetID);
		Send(packet,1);
	}

	void Client::RequestUse(DWORD actorID,DWORD targetID)
	{
		JE::Net::Packet packet;
		packet.field0x30=72;
		packet.WriteDword(actorID);
		packet.WriteDword(targetID);
		Send(packet,1);
	}

	void Client::RequestContext75(DWORD actorID,DWORD targetID) //45F790.
	{
		JE::Net::Packet packet;
		packet.field0x30=75;
		packet.WriteDword(actorID);
		packet.WriteDword(targetID);
		Send(packet,1);
	}

	void Client::RequestContext76(DWORD actorID,DWORD targetID) //45F870.
	{
		JE::Net::Packet packet;
		packet.field0x30=76;
		packet.WriteDword(actorID);
		packet.WriteDword(targetID);
		Send(packet,1);
	}

	void Client::RequestContext77(DWORD actorID,DWORD targetID) //45F950.
	{
		JE::Net::Packet packet;
		packet.field0x30=77;
		packet.WriteDword(actorID);
		packet.WriteDword(targetID);
		Send(packet,1);
	}

	void Client::RequestContext79(DWORD actorID,DWORD targetID) //45FB10.
	{
		JE::Net::Packet packet;
		packet.field0x30=79;
		packet.WriteDword(actorID);
		packet.WriteDword(targetID);
		Send(packet,1);
	}

	void Client::RequestContext80(DWORD actorID,DWORD targetID) //45FBF0.
	{
		JE::Net::Packet packet;
		packet.field0x30=80;
		packet.WriteDword(actorID);
		packet.WriteDword(targetID);
		Send(packet,1);
	}

	void Client::RequestGive(DWORD actorID,DWORD targetID,DWORD itemID) //45FE90.
	{
		JE::Net::Packet packet;
		packet.field0x30=83;
		packet.WriteDword(actorID);
		packet.WriteDword(targetID);
		packet.WriteDword(itemID);
		Send(packet,1);
	}

}

namespace F3
{
	BYTE ReceiveGameServerUseRequest(JE::Net::Packet& packet)
	{
		if (packet.field0x30!=72) return 0;
		JE::Net::UInt32 actorID,targetID;
		packet.ReadDword(actorID);
		packet.ReadDword(targetID);
		//Original46DFDC..46E00B resolves both IDs and returns. It has no
		//action allocation, queue, ClearActions, target test or mutation.
		JE::GetEntityPtr(actorID);
		JE::GetEntityPtr(targetID);
		return 1; //Adapter consumption only, not a successful world operation.
	}
}
