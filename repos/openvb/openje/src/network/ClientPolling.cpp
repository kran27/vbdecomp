#include "Client.hpp"
#include "ClientBinding.hpp"
#include "Connection.hpp"
#include "DebugPrint.hpp"
#include <cstdio>

namespace
{
	void PrintPacket(JE::Net::Client& client,JE::Net::Packet& packet,JE::Net::UInt32 flag,const char* text)
	{
		char peer[64];
		std::sprintf(peer,"%u.%u.%u.%u:%u",packet.address.bytes[0],packet.address.bytes[1],
			packet.address.bytes[2],packet.address.bytes[3],packet.address.port);
		JE::Net::DebugPrint(client.debugStream.debugFlags,client.debugStream.logger,flag,text,client.address.port,peer);
	}
	void PrintUserPacket(JE::Net::Client& client,JE::Net::Packet& packet,const char* text)
	{
		char peer[64];
		std::sprintf(peer,"%u.%u.%u.%u:%u",packet.address.bytes[0],packet.address.bytes[1],
			packet.address.bytes[2],packet.address.bytes[3],packet.address.port);
		JE::Net::DebugPrint(client.debugStream.debugFlags,client.debugStream.logger,4096,text,client.address.port,packet.field0x30,peer);
	}
}

namespace JE
{
	namespace Net
	{
		JE::Net::UInt8 Client::Poll()
		{
			JE::Net::UInt8 result=Base::Poll();
			SendKeepalive();
			return result;
		}
		void Client::ResetPollingClock()
		{
			Base::ResetPollingClock();
			connection->ResetClock(field0x34,field0x44);
		}
		void Client::PrepareUpdate(Packet&) {}
		void Client::PrepareStartRequest(Packet&) {} //Original+48,5A9320.
		void Client::SendStartRequest()
		{
			Packet packet;
			packet.address = connection->address;
			packet.field0x30 = 5;
			PrintPacket(*this,packet,32,"[client %u] Sending start request to server at %s\n");
			PrepareStartRequest(packet);
			Send(packet,1);
		}
		void Client::PrepareDisconnect(Packet&) {}
		JE::Net::UInt8 Client::OnServerStart(Packet&) { return 1; }
		JE::Net::UInt8 Client::OnServerUpdate(Packet&) { return 1; }
		JE::Net::UInt8 Client::OnServerDisconnect(Packet&) { return 1; }
		JE::Net::UInt8 Client::OnServerTimeout(JE::Net::UInt32) { return 1; }
		void Client::UnusedPacketHook3C(Packet&) {} //5A9320.
		void Client::PrepareChallengeRequest(Packet&) {} //5A9320.
		void Client::PrepareConnectRequest(Packet&) {} //5A9320.
		JE::Net::UInt8 Client::OnDiscoveryResponse(Packet&) { return 1; } //409BB0.
		JE::Net::UInt8 Client::OnServerChallenge(Packet&) { return 1; } //409BB0.
		JE::Net::UInt8 Client::OnServerConnect(Packet&) { return 1; } //409BB0.
		bool Client::DispatchGamePacketSink(Packet& packet)
		{
			return DispatchClientPacketSink(*this, packet);
		}
		void Client::OnGamePacket(Packet&) {} //Original5A9320 is unconditionally empty.
		void Client::SendDisconnectRequest()
		{
			Packet packet;
			packet.address=connection->address;
			packet.field0x30=8;
			PrintPacket(*this,packet,16,"[client %u] Sending disconnect to server at %s\n");
			PrepareDisconnect(packet);
			Send(packet,2);
		}
		void Client::DisconnectProtocol()
		{
			//The caller59C8A0 has already tested Client state>1. Connection state
			//does not gate this packet construction/callback in that original path.
			SendDisconnectRequest();
			connection->Reset();
			state=0;
		}
		void Client::ResetConnection(JE::Net::UInt8 notify)
		{
			if (static_cast<JE::Net::Int32>(connection->state)<=0) return;
			if (notify)
			{
				Packet packet;
				packet.address = connection->address;
				packet.field0x30 = 8;
				char peer[64];
				debugStream.Print(16,"[client %u] Sending disconnect to server at %s\n",
					address.port,packet.address.Format(peer,sizeof(peer),1));
				PrepareDisconnect(packet);
				Send(packet,2);
			}
			connection->Reset(); //59D040 does not write Client::state.
		}
		void Client::CheckServerTimeout()
		{
			if (static_cast<JE::Net::Int32>(connection->state)>0)
			{
				UInt32 interval = rate;
				UInt32 limit = timeout;
				UInt32 tick = field0x44;
				if (connection->HasTimedOut(tick,limit,interval))
				{
					debugStream.Print(0x1000000,"[client %u] Server timed out after %u milliseconds.\n",
						address.port,connection->value44);
					if (OnServerTimeout(connection->value44)) ResetConnection(1);
				}
			}
		}

		void Client::SendUpdate()
		{
			if (static_cast<JE::Net::Int32>(connection->state)>0)
			{
				Packet packet;
				connection->PrepareUpdate(packet,field0x44);
				char peer[64];
				debugStream.Print(64,"[client %u] Sending update to server at %s\n",
					address.port,packet.address.Format(peer,sizeof(peer),1));
				PrepareUpdate(packet);
				connection->SendUpdate(packet);
			}
		}

		void Client::UpdatePolling()
		{
			DispatchPackets();
			CheckServerTimeout();
			RefreshServerLatency();
			SendChallengeRequest();
			SendUpdate();
		}
		void Client::ReceivePacket(Packet& packet)
		{
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
			if (static_cast<JE::Net::Int32>(state)<2 || !connection->MatchesPeer(packet.address,packet.field0x24)
				|| !connection->Accept(packet,field0x44)) return;
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
					ReceiveEstablished(child);
				}
			}
			else ReceiveEstablished(packet);
		}
		void Client::ReceiveServerStart(Packet& packet) //59C5D0.
		{
			if (state!=2) return;
			char source[64];
			debugStream.Print(0x10000,"[client %u] Receiving requested start from server at %s\n",
				address.port,packet.address.Format(source,sizeof(source),1));
			if (!OnServerStart(packet))
			{
				Disconnect();
				return;
			}
			state=3;
			static_cast<ClientServerConnection*>(connection)->field0x4164=1;
		}

		void Client::ReceiveServerUpdate(Packet& packet) //59C670.
		{
			if (static_cast<JE::Net::Int32>(state)<2
				|| !static_cast<ClientServerConnection*>(connection)->field0x4164) return;
			char source[64];
			debugStream.Print(0x20000,"[client %u] Receiving update from server at %s\n",
				address.port,packet.address.Format(source,sizeof(source),1));
			connection->ReadUpdate(packet);
			OnServerUpdate(packet);
		}

		void Client::ReceiveEstablished(Packet& packet) //59C7F0.
		{
			switch (packet.field0x30)
			{
			case 5: ReceiveServerStart(packet); break;
			case 6: ReceiveServerUpdate(packet); break;
			default:
				char source[64];
				debugStream.Print(4096,"[client %u] Receiving sequenced user packet %u from %s\n",
					address.port,packet.field0x30,packet.address.Format(source,sizeof(source),1));
				OnGamePacket(packet);
			}
		}
		void Client::SendChallengeRequest() //59CCF0.
		{
			if (state != 1) return;
			const UInt32 now = field0x34;
			if (TimeDifference(field0x68,now) < 3000) return;
			field0x68 = now;
			Packet request;
			request.address = serverAddress;
			request.field0x30 = 3;
			char peer[64];
			debugStream.Print(8,"[client %u] Sending challenge request to server at %s\n",
				address.port,request.address.Format(peer,sizeof(peer),1));
			PrepareChallengeRequest(request);
			SendOutOfBand(request);
		}
		void Client::ReceiveDiscoveryResponse(Packet& packet) //59C560.
		{
			char peer[64];
			debugStream.Print(0x2000,"[client %u] Receiving requested info from server at %s\n",
				address.port,packet.address.Format(peer,sizeof(peer),1));
			OnDiscoveryResponse(packet);
		}
		void Client::ReceiveChallengeResponse(Packet& packet) //59C9C0.
		{
			if (state != 1 || !UInt8(packet.address.Compare(serverAddress,0))) return;
			packet.field0x0C = 0;
			if (packet.field0x10 < 4) return;
			UInt32 challenge;
			packet.ReadDword(challenge);
			char peer[64];
			debugStream.Print(0x4000,"[client %u] Receiving requested challenge from server at %s\n",
				address.port,packet.address.Format(peer,sizeof(peer),1));
			if (!OnServerChallenge(packet)) return;
			field0x6C = challenge;
			Packet request;
			request.address = packet.address;
			request.field0x30 = 4;
			request.WriteDword(1);
			request.WriteDword(field0x6C);
			debugStream.Print(16,"[client %u] Sending connect request to server at %s\n",
				address.port,packet.address.Format(peer,sizeof(peer),1));
			PrepareConnectRequest(request);
			SendOutOfBand(request);
		}
		void Client::ReceiveConnectResponse(Packet& packet) //59CB40.
		{
			if (state != 1 || !UInt8(packet.address.Compare(serverAddress,0))) return;
			if (packet.field0x10 - packet.field0x0C < 4)
			{
				state = 0;
				return;
			}
			UInt32 status;
			packet.ReadDword(status);
			if (status > 0)
			{
				state = 0;
				return;
			}
			char peer[64];
			debugStream.Print(0x8000,"[client %u] Receiving requested connect from server at %s\n",
				address.port,packet.address.Format(peer,sizeof(peer),1));
			if (!OnServerConnect(packet))
			{
				state = 0;
				return;
			}
			connection->Open(&socket,field0x2C,packet.address,packet.field0x24,field0x44);
			static_cast<ClientServerConnection*>(connection)->field0x4164 = 0;
			state = 2;
			Packet request;
			request.address = connection->address;
			request.field0x30 = 5;
			debugStream.Print(32,"[client %u] Sending start request to server at %s\n",
				address.port,packet.address.Format(peer,sizeof(peer),1));
			PrepareStartRequest(request);
			Send(request,1);
		}
		void Client::ReceiveServerDisconnect(Packet& packet) //59C6F0.
		{
			if (static_cast<JE::Net::Int32>(state)>=2)
			{
				Address& peer=packet.address;
				if (connection->MatchesPeer(peer,packet.field0x24))
				{
					char source[64];
					debugStream.Print(0x80000,"[client %u] Receiving disconnect from server at %s\n",
						address.port,peer.Format(source,sizeof(source),1));
					OnServerDisconnect(packet);
					connection->Reset();
					state=0;
				}
			}
		}

		void Client::ReceiveOutOfBand(Packet& packet) //59D1A0.
		{
			switch (packet.field0x30)
			{
			case 2: ReceiveDiscoveryResponse(packet); break;
			case 3: ReceiveChallengeResponse(packet); break;
			case 4: ReceiveConnectResponse(packet); break;
			case 8: ReceiveServerDisconnect(packet); break;
			default:
				char source[64];
				debugStream.Print(4096,"[client %u] Receiving out-of-band user packet %u from %s\n",
					address.port,packet.field0x30,packet.address.Format(source,sizeof(source),1));
				OnGamePacket(packet);
			}
		}
	}
}
