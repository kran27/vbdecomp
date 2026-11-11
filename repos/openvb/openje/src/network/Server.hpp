#pragma once

#include "Base.hpp"

namespace JE
{
	namespace Net
	{
		class Connection;
		struct ServerSlot
		{
			Address address;
			JE::Net::UInt32 field0x0C;
			JE::Net::UInt32 field0x10;
		};

		class Server : public Base
		{
		public:
			typedef void (*UserPacketObserver)(void* context,JE::Net::UInt32 index,const Packet& packet);
			explicit Server(JE::Net::UInt16 port); // 0x5A0B40.
			~Server(); // 0x5A1DC0.
			void Initialize(); // 0x552980; calls virtual Configure(16).
			void Cleanup(); // 0x5A1B80.
			void SetLogger(Logger callback); // 0x5A07D0.
			void SetDebugFlags(JE::Net::UInt32 flags); // 0x5A0800.
			JE::Net::UInt8 Poll(); //5A19C0, captures Base result before keepalives.
			void ResetPollingClock(); //5A0990, shared clock then connection clocks.
			void UpdatePolling(); //5A1BD0, dispatch ->timeouts ->latency ->updates.
			void ReceivePacket(Packet& packet); //5A1C00, outer acceptance then unpacked dispatch.
			virtual void Configure(JE::Net::UInt32 count); //0x5A0BC0; original+2C.
			virtual void CreateConnections(); //0x5A0900; original+30.
			virtual void PrepareDiscoveryResponse(Packet& packet); //Original+34, shared5A9320 empty.
			virtual void PrepareChallengeResponse(Packet& packet); //Original+38, shared5A9320 empty.
			virtual void PrepareConnectResponse(Packet& packet); //Original+3C, shared5A9320 empty.
			virtual void PrepareStart(JE::Net::UInt32 index,Packet& packet); //Original+40, shared408090 empty.
			virtual void PrepareUpdate(JE::Net::UInt32 index,Packet& packet); //Original+44, shared408090 empty.
			virtual void PrepareKeepalive(JE::Net::UInt32 index,Packet& packet); //Original+48, shared408090 empty.
			virtual void PrepareDisconnect(JE::Net::UInt32 index,Packet& packet); //Original+4C, shared408090 empty.
			virtual JE::Net::UInt8 OnDiscoveryRequest(Packet& packet); //Original+50, shared409BB0 returns1.
			virtual JE::Net::UInt8 OnChallengeRequest(Packet& packet); //Original+54, shared409BB0 returns1.
			virtual JE::Net::UInt8 OnConnectRequest(Packet& packet); //Original+58, shared409BB0 returns1.
			virtual JE::Net::UInt8 AllowStart(JE::Net::UInt32 index,Packet& packet); //Original+5C,5A0BB0 returns1.
			virtual JE::Net::UInt8 OnClientUpdate(JE::Net::UInt32 index,Packet& packet); //Original+60,5A0BB0 returns1.
			virtual JE::Net::UInt8 OnDisconnectRequest(JE::Net::UInt32 index,Packet& packet); //Original+64,5A0BB0 returns1.
			virtual void OnUserPacket(JE::Net::UInt32 index,Packet& packet); //Original+68, shared408090 empty.
			virtual void OnOutOfBandPacket(Packet& packet); //Original+6C, shared5A9320 empty.
			virtual JE::Net::UInt8 OnClientTimeout(JE::Net::UInt32 index,JE::Net::UInt32 elapsed); //Original+70,5A0BB0 returns1.
			JE::Net::Int32 FindClient(const Address& address,JE::Net::UInt16 remoteID); //5A0AD0, live-count lookup.
			void Process(); //5A08D0, enabled-only live connection-count pump.
			void SendKeepalive(JE::Net::UInt32 index); //5A1410, actual sequenced message7.
			void SendKeepalives(); //5A18C0, live-count sweep; outer Poll owns enabled check.
			void ResizeConnections(JE::Net::UInt32 count); // 0x5A0830.
			void DisconnectClient(JE::Net::UInt32 index, JE::Net::UInt8 notify); //5A1550, optional packet8 then Reset/count.
			void SendDisconnect(JE::Net::UInt32 index); //5A15AD..5A165A, actual packet8 child.
			void SetEnabled(JE::Net::UInt8 enabled); // 0x5A19A0; incidental AL excluded.
			void DisconnectAll(); // 0x5A18F0.
			void Broadcast(Packet& packet, JE::Net::UInt32 channel); //5A0A00 via per-slot5A09D0.

			Connection** connections; //Original+54.
			JE::Net::UInt8 enabled; //Original+58; preserve byte values.
			JE::Net::UInt8 field0x59;
			JE::Net::UInt32 connectionCount; //Original+5C.
			JE::Net::UInt32 field0x60;
			ServerSlot slots[1024];
		protected:
			void SendToClient(JE::Net::UInt32 index,Packet& packet,JE::Net::UInt32 channel); //5A09D0.
		private:
			void CheckClientTimeouts(); //5A1B50.
			void RefreshClientLatencies(); //5A0BF0.
			void SendClientUpdates(); //5A1890; owned by UpdatePolling catalog scope.
			void ReceiveOutOfBand(Packet& packet); //5A19F0.
			void ReceiveDiscoveryRequest(Packet& packet); //5A0C20; preserves request/reply alias defect.
			void ReceiveChallengeRequest(Packet& packet); //5A0D60; cached address challenge.
			void ReceiveConnectRequest(Packet& packet); //5A0F30; protocol/challenge admission.
			void ReceiveEstablished(JE::Net::UInt32 index,Packet& packet); //5A1AB0.
			void ReceiveStartRequest(JE::Net::UInt32 index,Packet& packet); //5A16A0.
			void ReceiveUpdate(JE::Net::UInt32 index,Packet& packet); //5A0A30.
			void ReceiveDisconnect(Packet& packet); //5A1800.
			void CheckClientTimeout(JE::Net::UInt32 index); //5A1920.
			void RefreshClientLatency(JE::Net::UInt32 index); //5A0B20.
			void SendClientUpdate(JE::Net::UInt32 index); //5A12F0.
		};

#if defined(_MSC_VER) && _MSC_VER == 1300
		typedef char NetServerSlotSize[(sizeof(ServerSlot) == 0x14) ? 1 : -1];
		typedef char NetServerConnections[(offsetof(Server, connections) == 0x54) ? 1 : -1];
		typedef char NetServerEnabled[(offsetof(Server, enabled) == 0x58) ? 1 : -1];
		typedef char NetServerInfoEnabled[(offsetof(Server, field0x59) == 0x59) ? 1 : -1];
		typedef char NetServerConnectionCount[(offsetof(Server, connectionCount) == 0x5C) ? 1 : -1];
		typedef char NetServerLiveCount[(offsetof(Server, field0x60) == 0x60) ? 1 : -1];
		typedef char NetServerSlots[(offsetof(Server, slots) == 0x64) ? 1 : -1];
		typedef char NetServerSize[(sizeof(Server) == 0x5064) ? 1 : -1];
		typedef char ServerOriginalSlotAddress[(offsetof(ServerSlot, address) == 0x00) ? 1 : -1];
		typedef char ServerOriginalSlotChallenge[(offsetof(ServerSlot, field0x0C) == 0x0C) ? 1 : -1];
		typedef char ServerOriginalSlotClock[(offsetof(ServerSlot, field0x10) == 0x10) ? 1 : -1];
#endif

		namespace ServerServices
		{
			void DestroyConnection(Connection* connection);
			void FreeConnections(Connection** connections);
			void Disconnect(Server& server, JE::Net::UInt32 index, JE::Net::UInt8 notify);
			void SetConnectionLogger(Connection* connection, Logger callback);
			void SetConnectionDebugFlags(Connection* connection, JE::Net::UInt32 flags);
			void ResizeConnections(Server& server, JE::Net::UInt32 count);
			Connection** AllocateConnections(JE::Net::UInt32 count);
			Connection* CreateClientConnection();
			void NotifyDisconnect(Server& server, JE::Net::UInt32 index);
		}
	}
}
