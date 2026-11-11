#pragma once

#include "Base.hpp"

namespace JE
{
	namespace Net
	{
		class Connection;

		class Client : public Base
		{
		public:
			explicit Client(JE::Net::UInt16 port); // 0x59C2E0.
			~Client(); // 0x59C790; misleading database CRT name.
			void Initialize(); // 0x59C340.
			void Cleanup(); // 0x59C360.
			void SetLogger(Logger callback); // 0x59C320.
			void SetDebugFlags(JE::Net::UInt32 flags); // 0x59C330.
			JE::Net::UInt8 Poll(); //59D180, captures Base result then SendKeepalive.
			void ResetPollingClock(); //59C4D0, Base clock then actual Connection clock reset.
			void UpdatePolling(); //59D2D0, dispatch/timeout/stats/challenge/update.
			void ReceivePacket(Packet& packet); //59D300, OOB/sequenced + packed dispatch.
			void Process(); //59C440, actual Connection queue pump.
			void SendKeepalive(); //59CF10; signed live state, strict idle>100ms, sequenced message7.
			void SendStartRequest(); //59CC3F..59CCAE factor after established state2; not full connect approval.
			//VC7 groups overloads in reverse declaration order in the vtable.
			virtual void Connect(const char* host); //59C380, original+30.
			virtual void Connect(const Address& address); //59C410, original+2C.
			virtual void Disconnect(); //59C8A0, original+34.
			virtual void CreateConnection(); //59C450, original+38.
			virtual void UnusedPacketHook3C(Packet& packet); //Original+3C,5A9320.
			virtual void PrepareChallengeRequest(Packet& packet); //Original+40,5A9320.
			virtual void PrepareConnectRequest(Packet& packet); //Original+44,5A9320.
			virtual void PrepareStartRequest(Packet& packet); //Original+48,5A9320.
			virtual void PrepareUpdate(Packet& packet); //Original+4C,5A9320.
			virtual void PrepareKeepalive(Packet& packet); //Original+50,5A9320.
			virtual void PrepareDisconnect(Packet& packet); //Original+54,5A9320.
			virtual JE::Net::UInt8 OnDiscoveryResponse(Packet& packet); //Original+58,409BB0.
			virtual JE::Net::UInt8 OnServerChallenge(Packet& packet); //Original+5C,409BB0.
			virtual JE::Net::UInt8 OnServerConnect(Packet& packet); //Original+60,409BB0.
			virtual JE::Net::UInt8 OnServerStart(Packet& packet); //Original+64,409BB0.
			virtual JE::Net::UInt8 OnServerUpdate(Packet& packet); //Original+68,409BB0.
			virtual JE::Net::UInt8 OnServerDisconnect(Packet& packet); //Original+6C,409BB0.
			virtual void OnGamePacket(Packet& packet); //Original+70,5A9320 empty.
			virtual JE::Net::UInt8 OnServerTimeout(JE::Net::UInt32 elapsed); //Original+74,409BB0.
			typedef void (*GamePacketSink)(Client& client,Packet& packet,void* context);
			bool DispatchGamePacketSink(Packet& packet); //Explicit external host binding; not an original slot.
			void DisconnectProtocol(); //Factored59C8E7 prefix for actual default ClientServices.
			void Send(Packet& packet, JE::Net::UInt32 channel); //59C4F0.

			Connection* connection;
			JE::Net::UInt32 state;
			Address serverAddress;
			JE::Net::UInt32 field0x68, field0x6C;
		private:
			void SendChallengeRequest(); //59CCF0, pending-state retry every3000ms.
			void CheckServerTimeout(); //59D260.
			void RefreshServerLatency(); //59C780.
			void SendUpdate(); //59CE10, tail of original UpdatePolling scope.
			void ReceiveDiscoveryResponse(Packet& packet); //59C560.
			void ReceiveChallengeResponse(Packet& packet); //59C9C0.
			void ReceiveConnectResponse(Packet& packet); //59CB40, approval then reliable start5.
			void ReceiveOutOfBand(Packet& packet); //59D1A0.
			void ReceiveEstablished(Packet& packet); //59C7F0.
			void ReceiveServerStart(Packet& packet); //59C5D0.
			void ReceiveServerUpdate(Packet& packet); //59C670.
			void ReceiveServerDisconnect(Packet& packet); //59C6F0.
			void SendDisconnectRequest(); //Shared source factor59C8EB/59D096.
			void ResetConnection(JE::Net::UInt8 notify); //59D040, does not assign Client state.
		};

#if defined(_MSC_VER) && _MSC_VER == 1300
		typedef char NetClientSize[(sizeof(Client) == 0x70) ? 1 : -1];
		typedef char NetClientConnection[(offsetof(Client, connection) == 0x54) ? 1 : -1];
		typedef char NetClientState[(offsetof(Client, state) == 0x58) ? 1 : -1];
		typedef char NetClientServerAddress[(offsetof(Client, serverAddress) == 0x5C) ? 1 : -1];
		typedef char NetClientChallengeTime[(offsetof(Client, field0x68) == 0x68) ? 1 : -1];
		typedef char NetClientChallengeValue[(offsetof(Client, field0x6C) == 0x6C) ? 1 : -1];
#endif

		namespace ClientServices
		{
			Connection* CreateServerConnection(); // Actual 0x5ABF30 child and derived defaults.
			void DestroyConnection(Connection* connection);
			void SetConnectionLogger(Connection* connection, Logger callback);
			void SetConnectionDebugFlags(Connection* connection, JE::Net::UInt32 flags);
			void DisconnectConnected(Client& client);
		}
	}
}
