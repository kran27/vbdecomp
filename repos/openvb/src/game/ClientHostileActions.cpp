#include "ClientHostileActions.hpp"
#include "network/Client.hpp"
#include "network/EngineClient.hpp"
#include "network/Packet.hpp"

namespace JE
{
	void Client::RequestHostile(DWORD actorID, DWORD targetID) //45F6B0.
	{
		JE::Net::Packet packet;
		packet.field0x30 = 74;
		packet.WriteDword(actorID);
		packet.WriteDword(targetID);
		Send(packet, 1);
	}
}
