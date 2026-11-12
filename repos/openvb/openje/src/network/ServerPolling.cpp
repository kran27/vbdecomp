#include "Server.hpp"
#include <new>
#include "Connection.hpp"
#include "DebugPrint.hpp"
#include <cstdio>
#include <stdexcept>

namespace JE
{
	namespace Net
	{
		void Server::Configure(JE::Net::UInt32 count)
		{
			Cleanup();
			rate = 100;
			timeout = 3000;
			ResizeConnections(count);
		}

		void Server::DisconnectAll()
		{
			for (JE::Net::UInt32 index = 0; index < connectionCount; ++index)
				DisconnectClient(index, 1);
		}

		void Server::Cleanup()
		{
			SetEnabled(0);
			if (connections)
			{
				for (JE::Net::UInt32 index = 0; index < connectionCount; ++index)
				{
					Connection* connection = connections[index];
					if (connection) delete connection;
				}
				::operator delete[](connections);
				connections = NULL;
			}
		}

		namespace
		{
			const char* AddressText(const Address& address,char* text)
			{
				std::sprintf(text,"%u.%u.%u.%u:%u",address.bytes[0],address.bytes[1],
					address.bytes[2],address.bytes[3],address.port);
				return text;
			}
		}

		void Server::PrepareStart(JE::Net::UInt32,Packet&) {} //408090.
		void Server::PrepareUpdate(JE::Net::UInt32,Packet&) {} //408090.
		void Server::PrepareDisconnect(JE::Net::UInt32,Packet&) {} //408090.
		JE::Net::UInt8 Server::AllowStart(JE::Net::UInt32,Packet&) { return 1; } //5A0BB0.
		JE::Net::UInt8 Server::OnClientUpdate(JE::Net::UInt32,Packet&) { return 1; } //5A0BB0.
		JE::Net::UInt8 Server::OnDisconnectRequest(JE::Net::UInt32,Packet&) { return 1; } //5A0BB0.
		void Server::OnUserPacket(JE::Net::UInt32,Packet&) {} //408090.
		void Server::OnOutOfBandPacket(Packet&) {} //5A9320.
		JE::Net::UInt8 Server::OnClientTimeout(JE::Net::UInt32,JE::Net::UInt32) { return 1; } //5A0BB0.

		JE::Net::UInt8 Server::Poll()
		{
			if (!enabled) return 0;
			JE::Net::UInt8 result=Base::Poll();
			SendKeepalives();
			return result;
		}

		void Server::ResetPollingClock()
		{
			Base::ResetPollingClock();
			if (!enabled) return;
			JE::Net::UInt32 now=field0x34,tick=field0x44;
			for (JE::Net::UInt32 index=0;index<connectionCount;++index)
				connections[index]->ResetClock(now,tick);
		}

		void Server::UpdatePolling()
		{
			if (!enabled) return;
			DispatchPackets();
			CheckClientTimeouts();
			RefreshClientLatencies();
			SendClientUpdates();
		}

		JE::Net::Int32 Server::FindClient(const Address& source,JE::Net::UInt16 id)
		{
			for (JE::Net::UInt32 index=0;index<connectionCount;++index)
				if (connections[index]->MatchesPeer(source,id)) return JE::Net::Int32(index);
			return -1;
		}

		void Server::ReceivePacket(Packet& packet)
		{
			if (!enabled) return;
			packet.field0x0C=0;
			if (packet.field0x28==0xFFFFFFFF)
			{
				if (packet.field0x30==1)
				{
					Packet child;
					while (packet.field0x10!=packet.field0x0C)
					{
						child.Deserialize(packet);
						child.address=packet.address;
						child.field0x24=packet.field0x24;
						child.field0x34=packet.field0x34;
						child.field0x0C=0;
						ReceiveOutOfBand(child);
					}
				}
				else ReceiveOutOfBand(packet);
				return;
			}
			JE::Net::Int32 index=FindClient(packet.address,packet.field0x24);
			if (index==-1 || !connections[index]->Accept(packet,field0x44)) return;
			if (packet.field0x30==1)
			{
				Packet child;
				while (packet.field0x10!=packet.field0x0C)
				{
					child.Deserialize(packet);
					child.address=packet.address;
					child.field0x24=packet.field0x24;
					child.field0x34=packet.field0x34;
					child.field0x0C=0;
					ReceiveEstablished(JE::Net::UInt32(index),child);
				}
			}
			else ReceiveEstablished(JE::Net::UInt32(index),packet);
		}

		void Server::ReceiveEstablished(JE::Net::UInt32 index,Packet& packet)
		{
			switch (packet.field0x30)
			{
			case 5:
				ReceiveStartRequest(index,packet);
				break;
			case 6:
				ReceiveUpdate(index,packet);
				break;
			default:
				char source[64];
				debugStream.Print(4096,
					"[server] Receiving sequenced user packet %u from client %u at %s\n",
					packet.field0x30,index,packet.address.Format(source,sizeof(source),1));
				OnUserPacket(index,packet); //Includes actual control7; engine/game owns its policy.
			}
		}

		void Server::ReceiveOutOfBand(Packet& packet)
		{
			switch (packet.field0x30)
			{
			case 2:
				ReceiveDiscoveryRequest(packet);
				return;
			case 3:
				ReceiveChallengeRequest(packet);
				return;
			case 4:
				ReceiveConnectRequest(packet);
				return;
			case 8:
				ReceiveDisconnect(packet);
				return;
			default:
				char source[64];
				debugStream.Print(4096,
					"[server] Receiving out-of-band user packet %u from %s\n",
					packet.field0x30,packet.address.Format(source,sizeof(source),1));
				OnOutOfBandPacket(packet);
			}
		}

		void Server::ReceiveStartRequest(JE::Net::UInt32 index,Packet& request)
		{
			char source[64];
			debugStream.Print(0x10000,
				"[server] Receiving a start request from client %u at %s\n",
				index,request.address.Format(source,sizeof(source),1));
			if (!AllowStart(index,request)) { DisconnectClient(index,1); return; }
			Packet reply;
			reply.address=connections[index]->address;
			reply.field0x30=5;
			debugStream.Print(32,"[server] Sending start to client %u at %s\n",
				index,request.address.Format(source,sizeof(source),1));
			PrepareStart(index,reply);
			connections[index]->Send(reply,1);
		}

		void Server::ReceiveUpdate(JE::Net::UInt32 index,Packet& packet)
		{
			if (JE::Net::Int32(connections[index]->state)<=0) return;
			char source[64];
			debugStream.Print(0x20000,
				"[server] Receiving update from client %u at %s [%u]\n",
				index,packet.address.Format(source,sizeof(source),1),connections[index]->tail.meanLatency);
			connections[index]->ReadUpdate(packet);
			OnClientUpdate(index,packet);
			static_cast<ServerClientConnection*>(connections[index])->field0x4164=1;
		}

		void Server::ReceiveDisconnect(Packet& packet)
		{
			JE::Net::Int32 index=FindClient(packet.address,packet.field0x24);
			if (index==-1) return;
			char source[64];
			debugStream.Print(0x80000,
				"[server] Receiving disconnect request from client %u at %s\n",
				JE::Net::UInt32(index),packet.address.Format(source,sizeof(source),1));
			OnDisconnectRequest(JE::Net::UInt32(index),packet);
			DisconnectClient(JE::Net::UInt32(index),0);
		}

		void Server::SendDisconnect(JE::Net::UInt32 index)
		{
			Packet packet;
			packet.address=connections[index]->address;
			packet.field0x30=8;
			char destination[64];
			DebugPrint(debugStream.debugFlags,debugStream.logger,16,"[server] Sending disconnect to client %u at %s\n",
				index,AddressText(packet.address,destination));
			PrepareDisconnect(index,packet);
			connections[index]->Send(packet,2);
		}

		void Server::CheckClientTimeout(JE::Net::UInt32 index)
		{
			Connection* current=connections[index];
			if (JE::Net::Int32(current->state)>0)
			{
				JE::Net::UInt32 interval=rate;
				JE::Net::UInt32 limit=timeout;
				JE::Net::UInt32 tick=field0x44;
				if (current->HasTimedOut(tick,limit,interval))
				{
					debugStream.Print(0x1000000,
						"[server] Client %u timed out after %u milliseconds\n",index,connections[index]->value44);
					if (OnClientTimeout(index,connections[index]->value44)) DisconnectClient(index,1);
				}
			}
		}

		void Server::RefreshClientLatency(JE::Net::UInt32 index)
		{
			if (JE::Net::Int32(connections[index]->state)>0) connections[index]->RefreshLatency();
		}

		void Server::SendClientUpdate(JE::Net::UInt32 index)
		{
			if (JE::Net::Int32(connections[index]->state)<=0
				|| !static_cast<ServerClientConnection*>(connections[index])->field0x4164) return;
			Packet packet;
			connections[index]->PrepareUpdate(packet,field0x44);
			char destination[64];
			debugStream.Print(64,"[server] Sending update to client %u at %s\n",
				index,packet.address.Format(destination,sizeof(destination),1));
			PrepareUpdate(index,packet);
			connections[index]->SendUpdate(packet);
		}
	}
}
