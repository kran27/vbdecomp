#define _WINMM_
#include "Base.hpp"

#include <mmsystem.h>
#include <stdexcept>

namespace JE
{
	namespace Net
	{
		Base::Base(JE::Net::UInt16 port)
		{
			field0x2E = 1;
			timeout = 5000;
			field0x34 = 0;
			field0x38 = 0;
			field0x3C = 0;
			rate = 20;
			field0x44 = 0;
			packets = NULL;
			packetCount = 32;
			field0x50 = 0;
			socket.Setup(port);
			socket.GetAddress(address);
			const JE::Net::UInt32 now = timeGetTime();
			const JE::Net::UInt32 count = packetCount;
			field0x2C = static_cast<JE::Net::UInt16>(now);
			packets = new Packet[count];
		}

		Base::~Base()
		{
			if (packets) BaseServices::DestroyPackets(packets);
			socket.Close();
			// Address, Socket (including its second Close), and lock follow in
			// normal member destruction order. Do not clear packets here.
		}

		void Base::SetLogger(Logger callback) { debugStream.logger = callback; }
		void Base::SetDebugFlags(JE::Net::UInt32 flags) { debugStream.debugFlags = flags; }
		JE::Net::UInt32 Base::GetDebugFlags() { return debugStream.debugFlags; }
		void Base::Process() { }

		void Base::SendOutOfBand(Packet& packet)
		{
			packet.field0x24 = field0x2C;
			packet.field0x28 = 0xFFFFFFFF;
			socket.Send(packet);
		}

		namespace BaseServices
		{
			int Setup(Socket& socket, JE::Net::UInt16 port) { return socket.Setup(port); }
			JE::Net::UInt16 GetAddress(Socket& socket, Address& address) { return socket.GetAddress(address); }
			JE::Net::UInt32 Time() { return timeGetTime(); }
			Packet* AllocatePackets(JE::Net::UInt32 count) { return new Packet[count]; }
			void DestroyPackets(Packet* packets) { delete[] packets; }
		}
	}
}
