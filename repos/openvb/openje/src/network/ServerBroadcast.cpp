#include "Server.hpp"
#include "Connection.hpp"

namespace JE
{
	namespace Net
	{
		void Server::SendToClient(JE::Net::UInt32 index,Packet& packet,JE::Net::UInt32 channel)
		{
			if (enabled) connections[index]->Send(packet,channel);
		}

		void Server::Broadcast(Packet& packet, JE::Net::UInt32 channel)
		{
			//5A0A00 reloads connectionCount after each slot. The per-slot helper
			//5A09D0 checks enabled, then delegates all connection-state policy to
			//the actual Connection5AC450 Send implementation.
			for (JE::Net::UInt32 i = 0; i < connectionCount; ++i)
				SendToClient(i,packet,channel);
		}

		void Server::CheckClientTimeouts()
		{
			for (JE::Net::UInt32 index=0;index<connectionCount;++index) CheckClientTimeout(index);
		}

		void Server::RefreshClientLatencies()
		{
			for (JE::Net::UInt32 index=0;index<connectionCount;++index) RefreshClientLatency(index);
		}

		void Server::SendClientUpdates()
		{
			for (JE::Net::UInt32 index=0;index<connectionCount;++index) SendClientUpdate(index);
		}
	}
}
