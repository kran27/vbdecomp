// Original WinMM calls use import-library thunks.
#define _WINMM_
#include "Base.hpp"
#include "Connection.hpp"
#include <mmsystem.h>

namespace JE
{
	namespace Net
	{
		void Base::ResetPollingClock() //59D5D0.
		{
			field0x44=0;
			field0x38=timeGetTime();
			field0x3C=0;
		}

		void Base::ReceivePackets() //59D7F0.
		{
			while (socket.Receive(packets[field0x50]))
			{
				packets[field0x50].field0x34=field0x34;
				JE::Net::UInt32 oldCount=packetCount;
				++field0x50;
				if (field0x50>=oldCount)
				{
					Packet* oldPackets=packets;
					packetCount=2*oldCount;
					Packet* replacement=new Packet[packetCount];
					packets=replacement;
					for (JE::Net::UInt32 index=0;index<oldCount;++index)
						packets[index]=oldPackets[index];
					//Actual assignment5AC630 resets but does not copy receive time+34.
					if (oldPackets) delete[] oldPackets;
				}
			}
		}

		void Base::DispatchPackets() //59D5F0.
		{
			for (JE::Net::UInt32 index=0;index<field0x50;++index)
				ReceivePacket(packets[index]);
			field0x50=0;
		}

		JE::Net::UInt8 Base::Poll() //59D930.
		{
			if (field0x2E)
			{
				field0x2E=0;
				ResetPollingClock();
			}
			field0x34=timeGetTime();
			field0x3C+=TimeDifference(field0x38,field0x34);
			field0x38=field0x34;
			ReceivePackets();
			JE::Net::UInt32 interval=rate;
			if (!interval)
			{
				++field0x44;
				UpdatePolling();
				return 1;
			}
			JE::Net::UInt32 updates=field0x3C/interval;
			JE::Net::UInt8 result;
			if (updates)
			{
				result = 1;
				if (updates>10) updates=10;
				do
				{
					if (field0x3C<=interval) break;
					--updates;
					++field0x44;
					UpdatePolling();
					interval=rate;
					field0x3C-=interval;
				}
				while (updates);
			}
			else result = 0;
			return result; //Nonzero quotient returns1 even if accumulator==interval.
		}
	}
}
