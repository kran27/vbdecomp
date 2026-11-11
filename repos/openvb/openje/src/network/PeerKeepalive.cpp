// Original WinMM calls use import-library thunks.
#define _WINMM_
#include "Client.hpp"
#include "Server.hpp"
#include "Connection.hpp"
#include "DebugPrint.hpp"
#include <mmsystem.h>
#include <cstdio>

namespace JE
{
	namespace Net
	{
		void Client::PrepareKeepalive(Packet&) {} //5A9320, shared RET4 hook.
		void Server::PrepareKeepalive(JE::Net::UInt32,Packet&) {} //408090, shared RET8 hook.

		void Client::SendKeepalive() //59CF10, the post-Base::Poll suffix.
		{
			if (static_cast<JE::Net::Int32>(connection->state)<=0) return;
			JE::Net::UInt32 lastSend=connection->lastSendTime;
			if (TimeDifference(lastSend,timeGetTime())<=100) return;
			Packet packet;
			packet.address=connection->address;
			packet.field0x30=7;
			char destination[64];
			debugStream.Print(128,"[client %u] Sending keepalive to server at %s\n",
				address.port,packet.address.Format(destination,sizeof(destination),1));
			PrepareKeepalive(packet);
			Send(packet,0); //Real Connection::SendSequenced publishes sequence and ACK fields.
		}

		void Server::SendKeepalive(JE::Net::UInt32 index) //5A1410.
		{
			if (static_cast<JE::Net::Int32>(connections[index]->state)<=0) return;
			JE::Net::UInt32 lastSend=connections[index]->lastSendTime;
			if (TimeDifference(lastSend,timeGetTime())<=100) return;
			Packet packet;
			packet.address=connections[index]->address;
			packet.field0x30=7;
			char destination[64];
			debugStream.Print(128,"[server] Sending keepalive to client %u at %s\n",
				index,packet.address.Format(destination,sizeof(destination),1));
			PrepareKeepalive(index,packet);
			connections[index]->Send(packet,0); //Reloads connection after the virtual hook.
		}

	}
}
