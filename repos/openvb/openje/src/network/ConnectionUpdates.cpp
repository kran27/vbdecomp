// Original WinMM calls use import-library thunks.
#define _WINMM_
#include "Connection.hpp"
#include <windows.h>
#include <mmsystem.h>

namespace JE
{
	namespace Net
	{
		void Connection::ResetClock(JE::Net::UInt32 physicalNow,JE::Net::UInt32 tick)
		{
			lastReceiveTime=tick;
			lastSendTime=physicalNow;
		}
		bool Connection::HasTimedOut(JE::Net::UInt32 tick,JE::Net::UInt32 limit,JE::Net::UInt32 interval)
		{
			JE::Net::UInt32 elapsed=interval*(tick-lastReceiveTime);
			if (elapsed>limit)
			{
				lastReceiveTime=tick;
				value44+=elapsed;
				return true;
			}
			return false;
		}
		void Connection::ReadUpdate(Packet& packet)
		{
			packet.ReadDword(updateTick);
			packet.ReadDword(updateSentTime);
			updateReceivedTime=packet.field0x34;
			JE::Net::UInt32 tick,sent,delay;
			packet.ReadDword(tick);
			packet.ReadDword(sent);
			packet.ReadDword(delay);
			if (tick!=tail.echoedTick)
			{
				tail.echoedTick=tick;
				JE::Net::UInt32 now=timeGetTime();
				JE::Net::UInt32 elapsed=TimeDifference(sent,now)-TimeDifference(packet.field0x34,now);
				if (delay>elapsed) elapsed=0;
				else elapsed-=delay;
				tail.latencySamples[tick&15]=elapsed;
			}
		}
		JE::Net::UInt32 Connection::RefreshLatency()
		{
			JE::Net::UInt32 count=0,sum=0;
			for (JE::Net::UInt32 i=0;i<16;++i)
				if (tail.latencySamples[i]>0) { sum+=tail.latencySamples[i];++count; }
			if (count) { sum/=count;tail.meanLatency=sum; }
			else tail.meanLatency=0;
			return sum;
		}
		void Connection::PrepareUpdate(Packet& packet,JE::Net::UInt32 tick)
		{
			packet.Reset();
			packet.address=address;
			packet.field0x30=6;
			packet.WriteDword(tick);
			packet.WriteDword(0);
			packet.WriteDword(updateTick);
			packet.WriteDword(updateSentTime);
			packet.WriteDword(0);
		}
		void Connection::SendUpdate(Packet& packet)
		{
			packet.field0x0C=0;
			if (packet.field0x10<4) throw Exception("Invalid seek.");
			packet.field0x0C=4;
			packet.WriteDword(timeGetTime());
			JE::Net::UInt32 cursor=packet.field0x0C+8;
			if (cursor>packet.field0x10) throw Exception("Invalid seek.");
			packet.field0x0C=cursor;
			packet.WriteDword(TimeDifference(updateReceivedTime,timeGetTime()));
			Send(packet,0);
		}
	}
}
