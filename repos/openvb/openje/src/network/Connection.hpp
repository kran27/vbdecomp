#pragma once

#include "DebugStream.hpp"
#include "Address.hpp"
#include "Packet.hpp"
#include <deque>
#include <cstddef>

namespace JE
{
	namespace Net
	{
		class Socket;
		UInt32 TimeDifference(UInt32 first, UInt32 second); //5B8D00, wrap quirk.
		struct ConnectionException : public Exception
		{
		public:
			explicit ConnectionException(const char* message) : Exception(message) {}
		};
		struct ConnectionTrafficCounters
		{
			ConnectionTrafficCounters() : receivedPackets(0),sentPackets(0),receivedBytes(0),sentBytes(0) {}
			void Reset()
			{
				receivedPackets = 0;
				sentPackets = 0;
				receivedBytes = 0;
				sentBytes = 0;
			}
			UInt32 receivedPackets,sentPackets,receivedBytes,sentBytes;
		};
		struct ConnectionStatistics
		{
			ConnectionStatistics(); //5ABD80, including redundant counter reset.
			UInt32 meanLatency,echoedTick,latencySamples[16],sequenceGap;
			ConnectionTrafficCounters traffic[3];
		};

		class Connection
		{
		public:
			Connection(); // 0x5ABF30.
			virtual ~Connection(); // 0x5ABF90.
			void Reset(); // 0x5AB790; state only.
			void Open(Socket* socket, UInt16 localID, const Address& peer,
				UInt16 remoteID, UInt32 now); //5ABC00, actual channel establishment.
			void Send(Packet& packet, UInt32 channel); //5AC450.
			void Process(); //5AC060, reliable queue/ack/retransmit.
			bool MatchesPeer(const Address& peer, UInt16 remoteID); //5AB8E0.
			bool Accept(Packet& packet, UInt32 now); //5AB7A0, sequence/ack policy.
			void ResetClock(UInt32 physicalNow,UInt32 tick); //5AB770.
			bool HasTimedOut(UInt32 tick,UInt32 timeout,UInt32 rate); //5AB930.
			void ReadUpdate(Packet& packet); //5AB960, preserves partial reads and latency ring.
			UInt32 RefreshLatency(); //5ABA20, mean of nonzero16 samples.
			void PrepareUpdate(Packet& packet,UInt32 tick); //5ABC70, control6 header.
			void SendUpdate(Packet& packet); //5AC500, patches physical clocks then Send0.

			DebugStream debugStream; //Original+04; flags+08/logger+0C.
			UInt32 state;
			Socket* socket; //+14, borrowed; established by Open.
			UInt16 localID; //+18.
			Address address;
			UInt16 remoteID; //+28.
			UInt32 receivedSequence, acknowledgedSequence; //+2C/+30.
			UInt32 receivedReliableBit, acknowledgedReliableBit; //+34/+38.
			UInt32 value3C; //Original-unwritten.
			UInt32 lastReceiveTime, value44; //+40/+44, value44 set by Accept.
			UInt32 sentSequence, sendReliableBit, reliableSequence; //+48/+4C/+50.
			UInt32 lastSendTime; //+54.
			std::deque<Packet> queue;
			Packet packet0;
			Packet packet1;
			//5ABD80 zeros these31 words at40DC: mean latency, echoed tick,
			//16 latency samples, then existing packet counters/byte totals.
			ConnectionStatistics tail;
			UInt32 updateTick,updateSentTime,updateReceivedTime; //Original4158/415C/4160, ctor-UNWRITTEN.
		private:
			Connection(const Connection&);
			Connection& operator=(const Connection&);
			UInt32 SendUnsequenced(Packet& packet); //5ABAB0.
			UInt32 SendSequenced(Packet& packet); //5ABAF0.
			UInt32 SendReliable(Packet& packet); //5ABB60.
		};

		class ServerClientConnection : public Connection
		{
		public:
			ServerClientConnection() : field0x4164(0) {} // Derived role in 0x5A0900.
			virtual ~ServerClientConnection() {}
			UInt8 field0x4164;
		};
		class ClientServerConnection : public Connection
		{
		public:
			ClientServerConnection() : field0x4164(0) {} // Derived role in 0x59C450.
			virtual ~ClientServerConnection() {}
			UInt8 field0x4164;
		};
		class EngineServerClientConnection : public ServerClientConnection
		{
		public:
			EngineServerClientConnection() : field0x4168(0) {} // Derived role in 0x552AC0.
			virtual ~EngineServerClientConnection() {}
			UInt32 field0x4168;
			UInt32 field0x416C; //Map ID from client packet64; original552AC0 leaves it unwritten.
		};
		class EngineClientServerConnection : public ClientServerConnection
		{
		public:
			EngineClientServerConnection() {} // Distinct Client::Server role in 0x588F50.
			virtual ~EngineClientServerConnection() {}
		};

#if defined(_MSC_VER) && _MSC_VER == 1300
		typedef char NetTrafficCountersSize[(sizeof(ConnectionTrafficCounters)==0x10)?1:-1];
		typedef char NetStatisticsSize[(sizeof(ConnectionStatistics)==0x7C)?1:-1];
		typedef char NetStatisticsSamples[(offsetof(ConnectionStatistics,latencySamples)==0x08)?1:-1];
		typedef char NetStatisticsGap[(offsetof(ConnectionStatistics,sequenceGap)==0x48)?1:-1];
		typedef char NetStatisticsTraffic[(offsetof(ConnectionStatistics,traffic)==0x4C)?1:-1];
		typedef char NetConnectionSize[(sizeof(Connection) == 0x4164) ? 1 : -1];
		typedef char NetConnectionDebugStream[(offsetof(Connection, debugStream) == 0x04) ? 1 : -1];
		typedef char NetConnectionState[(offsetof(Connection, state) == 0x10) ? 1 : -1];
		typedef char NetConnectionSocket[(offsetof(Connection, socket) == 0x14) ? 1 : -1];
		typedef char NetConnectionLocalID[(offsetof(Connection, localID) == 0x18) ? 1 : -1];
		typedef char NetConnectionAddress[(offsetof(Connection, address) == 0x1C) ? 1 : -1];
		typedef char NetConnectionRemoteID[(offsetof(Connection, remoteID) == 0x28) ? 1 : -1];
		typedef char NetConnectionReceiveSequence[(offsetof(Connection, receivedSequence) == 0x2C) ? 1 : -1];
		typedef char NetConnectionAckSequence[(offsetof(Connection, acknowledgedSequence) == 0x30) ? 1 : -1];
		typedef char NetConnectionReceiveReliable[(offsetof(Connection, receivedReliableBit) == 0x34) ? 1 : -1];
		typedef char NetConnectionAckReliable[(offsetof(Connection, acknowledgedReliableBit) == 0x38) ? 1 : -1];
		typedef char NetConnectionUnknownWord[(offsetof(Connection, value3C) == 0x3C) ? 1 : -1];
		typedef char NetConnectionLastReceiveTime[(offsetof(Connection, lastReceiveTime) == 0x40) ? 1 : -1];
		typedef char NetConnectionTimeoutValue[(offsetof(Connection, value44) == 0x44) ? 1 : -1];
		typedef char NetConnectionSendSequence[(offsetof(Connection, sentSequence) == 0x48) ? 1 : -1];
		typedef char NetConnectionSendReliable[(offsetof(Connection, sendReliableBit) == 0x4C) ? 1 : -1];
		typedef char NetConnectionReliableSequence[(offsetof(Connection, reliableSequence) == 0x50) ? 1 : -1];
		typedef char NetConnectionLastSendTime[(offsetof(Connection, lastSendTime) == 0x54) ? 1 : -1];
		typedef char NetConnectionQueue[(offsetof(Connection, queue) == 0x58) ? 1 : -1];
		typedef char NetConnectionQueueSize[(sizeof(std::deque<Packet>) == 0x14) ? 1 : -1];
		typedef char NetConnectionPacketSize[(sizeof(Packet) == 0x2038) ? 1 : -1];
		typedef char NetConnectionPacket0[(offsetof(Connection, packet0) == 0x6C) ? 1 : -1];
		typedef char NetConnectionPacket1[(offsetof(Connection, packet1) == 0x20A4) ? 1 : -1];
		typedef char NetConnectionStatistics[(offsetof(Connection, tail) == 0x40DC) ? 1 : -1];
		typedef char NetConnectionStatisticsSize[(sizeof(((Connection*)0)->tail) == 0x7C) ? 1 : -1];
		typedef char NetConnectionUpdateTick[(offsetof(Connection, updateTick) == 0x4158) ? 1 : -1];
		typedef char NetConnectionUpdateSent[(offsetof(Connection, updateSentTime) == 0x415C) ? 1 : -1];
		typedef char NetConnectionUpdateReceived[(offsetof(Connection, updateReceivedTime) == 0x4160) ? 1 : -1];
		typedef char NetServerClientConnectionSize[(sizeof(ServerClientConnection) == 0x4168) ? 1 : -1];
		typedef char NetServerClientConnectionRole[(offsetof(ServerClientConnection, field0x4164) == 0x4164) ? 1 : -1];
		typedef char NetClientServerConnectionSize[(sizeof(ClientServerConnection) == 0x4168) ? 1 : -1];
		typedef char NetClientServerConnectionRole[(offsetof(ClientServerConnection, field0x4164) == 0x4164) ? 1 : -1];
		typedef char NetEngineServerConnectionSize[(sizeof(EngineServerClientConnection) == 0x4170) ? 1 : -1];
		typedef char NetEngineServerConnectionID[(offsetof(EngineServerClientConnection, field0x4168) == 0x4168) ? 1 : -1];
		typedef char NetEngineServerConnectionMap[(offsetof(EngineServerClientConnection, field0x416C) == 0x416C) ? 1 : -1];
		typedef char NetEngineClientConnectionSize[(sizeof(EngineClientServerConnection) == 0x4168) ? 1 : -1];
#endif
	}
}
