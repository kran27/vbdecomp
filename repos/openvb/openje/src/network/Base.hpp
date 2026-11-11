#pragma once

#include "Logger.hpp"
#include "Socket.hpp"
#include "Packet.hpp"
#include "CriticalSection.hpp"
#include "DebugStream.hpp"
#include <cstddef>

namespace JE
{
	namespace Net
	{
		class Base
		{
		public:
			explicit Base(JE::Net::UInt16 port); // 0x59D6D0.
			virtual ~Base(); //0x59D530; original slot+00.
			virtual void Initialize() = 0; //Original+04.
			virtual void Cleanup() = 0;
			virtual void SetLogger(Logger callback); //0x59C2C0, original+0C.
			virtual void SetDebugFlags(JE::Net::UInt32 flags); //0x59C2D0,+10.
			virtual JE::Net::UInt32 GetDebugFlags(); //55BDB0, original+14.
			virtual JE::Net::UInt8 Poll(); //59D930, original+18.
			virtual void Process(); //404B70, original+1C empty hook.
			virtual void ResetPollingClock(); //59D5D0, original slot+20.
			virtual void UpdatePolling() = 0; //Original slot+24, pure on Base.
			virtual void ReceivePacket(Packet& packet) = 0; //Original slot+28, pure on Base.
			void DispatchPackets(); //59D5F0; clear pending count after all callbacks return.
			void SendOutOfBand(Packet& packet); //59D630; own ID, unsequenced marker, actual Socket send.

			CriticalSection lock; //Original+04,8B polymorphic owner.
			DebugStream debugStream; //Original+0C,flags+10/logger+14.
			Socket socket;
			Address address;
			JE::Net::UInt16 field0x2C; // Written only after timeGetTime returns.
			JE::Net::UInt8 field0x2E;
			JE::Net::UInt32 timeout; //Original+30.
			JE::Net::UInt32 field0x34, field0x38, field0x3C; //Current/previous clock, accumulator.
			JE::Net::UInt32 rate; //Original+40.
			JE::Net::UInt32 field0x44; //Update count.
			Packet* packets;
			JE::Net::UInt32 packetCount, field0x50; //Capacity and pending receive count.
		private:
			void ReceivePackets(); //59D7F0; normal Packet array growth, no dispatch.
		};

#if defined(_MSC_VER) && _MSC_VER == 1300
		typedef char NetCriticalSectionSize[(sizeof(CriticalSection) == 0x08) ? 1 : -1];
		typedef char NetCriticalSectionPointer[(offsetof(CriticalSection, section) == 0x04) ? 1 : -1];
		typedef char NetDebugStreamSize[(sizeof(DebugStream) == 0x0C) ? 1 : -1];
		typedef char NetDebugStreamFlags[(offsetof(DebugStream, debugFlags) == 0x04) ? 1 : -1];
		typedef char NetDebugStreamLogger[(offsetof(DebugStream, logger) == 0x08) ? 1 : -1];
		typedef char NetSocketSize[(sizeof(Socket) == 0x08) ? 1 : -1];
		typedef char NetAddressSize[(sizeof(Address) == 0x0C) ? 1 : -1];
		typedef char NetBaseSize[(sizeof(Base) == 0x54) ? 1 : -1];
		typedef char NetBaseLock[(offsetof(Base, lock) == 0x04) ? 1 : -1];
		typedef char NetBaseDebugStream[(offsetof(Base, debugStream) == 0x0C) ? 1 : -1];
		typedef char NetBaseSocket[(offsetof(Base, socket) == 0x18) ? 1 : -1];
		typedef char NetBaseAddress[(offsetof(Base, address) == 0x20) ? 1 : -1];
		typedef char NetBaseID[(offsetof(Base, field0x2C) == 0x2C) ? 1 : -1];
		typedef char NetBaseResetFlag[(offsetof(Base, field0x2E) == 0x2E) ? 1 : -1];
		typedef char NetBaseTimeout[(offsetof(Base, timeout) == 0x30) ? 1 : -1];
		typedef char NetBaseCurrentTime[(offsetof(Base, field0x34) == 0x34) ? 1 : -1];
		typedef char NetBasePreviousTime[(offsetof(Base, field0x38) == 0x38) ? 1 : -1];
		typedef char NetBaseAccumulator[(offsetof(Base, field0x3C) == 0x3C) ? 1 : -1];
		typedef char NetBaseRate[(offsetof(Base, rate) == 0x40) ? 1 : -1];
		typedef char NetBaseUpdateCount[(offsetof(Base, field0x44) == 0x44) ? 1 : -1];
		typedef char NetBasePackets[(offsetof(Base, packets) == 0x48) ? 1 : -1];
		typedef char NetBasePacketCount[(offsetof(Base, packetCount) == 0x4C) ? 1 : -1];
		typedef char NetBasePendingCount[(offsetof(Base, field0x50) == 0x50) ? 1 : -1];
#endif

		namespace BaseServices
		{
			int Setup(Socket& socket, JE::Net::UInt16 port);
			JE::Net::UInt16 GetAddress(Socket& socket, Address& address);
			JE::Net::UInt32 Time();
			Packet* AllocatePackets(JE::Net::UInt32 count);
			void DestroyPackets(Packet* packets);
		}
	}
}
