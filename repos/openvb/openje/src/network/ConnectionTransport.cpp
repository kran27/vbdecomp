// Original WinMM calls use import-library thunks.
#define _WINMM_
#include "Connection.hpp"
#include "Socket.hpp"
#include "DebugPrint.hpp"
#include <mmsystem.h>
#include <cstring>

namespace JE
{
	namespace Net
	{
		JE::Net::UInt32 TimeDifference(JE::Net::UInt32 first, JE::Net::UInt32 second)
		{
			if (first > second)
				return second - first - 1;
			return second - first;
		}

		void Connection::Open(Socket* channel, JE::Net::UInt16 source, const Address& peer,
			JE::Net::UInt16 destination, JE::Net::UInt32 now)
		{
			Reset();
			state = 1;
			socket = channel;
			localID = source;
			address = peer;
			remoteID = destination;
			receivedSequence = 0;
			acknowledgedSequence = 0;
			receivedReliableBit = 0;
			acknowledgedReliableBit = 0;
			lastReceiveTime = now;
			sentSequence = 0;
			sendReliableBit = 0;
			reliableSequence = 0;
			lastSendTime = timeGetTime();
			tail.sequenceGap = 0;
		}

		JE::Net::UInt32 Connection::SendUnsequenced(Packet& packet)
		{
			socket->Send(packet);
			++tail.traffic[2].sentPackets;
			JE::Net::UInt32 size = static_cast<JE::Net::UInt16>(packet.SerializedSize(71));
			tail.traffic[2].sentBytes += size;
			return size;
		}

		JE::Net::UInt32 Connection::SendSequenced(Packet& packet)
		{
			packet.field0x28 = ++sentSequence & 0x7FFFFFFF;
			packet.field0x2C = (receivedSequence & 0x7FFFFFFF) | (receivedReliableBit << 31);
			socket->Send(packet);
			++tail.traffic[0].sentPackets;
			tail.traffic[0].sentBytes += static_cast<JE::Net::UInt16>(packet.SerializedSize(71));
			return tail.traffic[0].sentBytes;
		}

		JE::Net::UInt32 Connection::SendReliable(Packet& packet)
		{
			JE::Net::UInt32 previousBit = sendReliableBit;
			reliableSequence = ++sentSequence & 0x7FFFFFFF;
			sendReliableBit = previousBit ^ 1;
			JE::Net::UInt32 oldSequence = packet.field0x28;
			packet.field0x28 = reliableSequence | 0x80000000;
			packet.field0x2C = (receivedSequence & 0x7FFFFFFF) | (receivedReliableBit << 31);
			debugStream.Print( 0x02000000,
				"[connection] Sending reliable packet.               src: %u seq: %0x ack: %0x id: %u prev. seq: %u.\n",
				packet.field0x24, packet.field0x28, packet.field0x2C, packet.field0x30, oldSequence);
			socket->Send(packet);
			++tail.traffic[1].sentPackets;
			tail.traffic[1].sentBytes += static_cast<JE::Net::UInt16>(packet.SerializedSize(71));
			return tail.traffic[1].sentBytes;
		}

		void Connection::Send(Packet& packet, JE::Net::UInt32 channel)
		{
			if (static_cast<JE::Net::Int32>(state) <= 0) return;
			packet.address = address;
			packet.field0x24 = localID;
			lastSendTime = timeGetTime();
			if (channel == 2) { SendUnsequenced(packet); return; }
			if (!channel) { SendSequenced(packet); return; }
			if (channel == 1) queue.push_back(packet);
			else throw ConnectionException("Attempt to send packet via unknown channel.");
		}

		bool Connection::MatchesPeer(const Address& peer, JE::Net::UInt16 id)
		{
			if (static_cast<JE::Net::Int32>(state) <= 0) return false;
			//The original caller consumes Compare's low byte (TEST AL,AL).
			if (!static_cast<UInt8>(address.Compare(peer, 0)) || remoteID != id) return false;
			if (address.port != peer.port) address.port = peer.port;
			return true;
		}

		bool Connection::Accept(Packet& packet, JE::Net::UInt32 now)
		{
			if (static_cast<JE::Net::Int32>(state) <= 0) return false;
			if (packet.field0x28 == 0xFFFFFFFF)
			{
				++tail.traffic[2].receivedPackets;
				tail.traffic[2].receivedBytes += static_cast<JE::Net::UInt16>(packet.SerializedSize(71));
				return true;
			}
			JE::Net::UInt32 sequence = packet.field0x28;
			JE::Net::UInt32 reliable = sequence >> 31;
			sequence &= 0x7FFFFFFF;
			if (sequence <= receivedSequence) return false;
			tail.sequenceGap = sequence - receivedSequence - 1;
			JE::Net::UInt32 acknowledgement = packet.field0x2C;
			JE::Net::UInt32 ackBit = acknowledgement >> 31;
			acknowledgement &= 0x7FFFFFFF;
			if (ackBit == sendReliableBit)
			{
				if (packet1.field0x10)
					debugStream.Print(0x02000000,
						"[connection] Reliable packet has been acknowledged. src: %u seq: %0x id: %u.\n",
						packet1.field0x24, packet1.field0x28, packet1.field0x30);
				packet1.Reset();
			}
			acknowledgedSequence = acknowledgement;
			acknowledgedReliableBit = ackBit;
			receivedSequence = sequence;
			if (reliable)
			{
				receivedReliableBit ^= 1;
				++tail.traffic[1].receivedPackets;
				tail.traffic[1].receivedBytes += static_cast<JE::Net::UInt16>(packet.SerializedSize(71));
			}
			else
			{
				++tail.traffic[0].receivedPackets;
				tail.traffic[0].receivedBytes += static_cast<JE::Net::UInt16>(packet.SerializedSize(71));
			}
			lastReceiveTime = now;
			value44 = 0;
			return true;
		}

		void Connection::Process()
		{
			if (static_cast<JE::Net::Int32>(state) <= 0) return;
			if (acknowledgedSequence > reliableSequence && acknowledgedReliableBit != sendReliableBit
				&& packet1.field0x30)
			{
				debugStream.Print( 0x02000000,
					"[connection] Re-sending reliable packet.            src: %u seq: %0x id: %u.\n",
						packet1.field0x24, packet1.field0x28, packet1.field0x30);
				SendReliable(packet1);
				sendReliableBit ^= 1;
				return;
			}
			JE::Net::UInt32 count = static_cast<JE::Net::UInt32>(queue.size());
			if (!count || packet1.field0x30) return;
			if (count > 1)
			{
				debugStream.Print( 0x02000000,
					"[connection] Packing up reliable packets to send. id's are: ");
				packet1.field0x30 = 1;
				for (JE::Net::UInt32 index = 0; index < count; ++index)
				{
					Packet& outgoing = packet1;
					Packet& packet = queue.front();
					debugStream.Print( 0x02000000, "%u ", packet.field0x30);
					JE::Net::UInt32 available = outgoing.capacity - outgoing.field0x10;
					if (static_cast<JE::Net::UInt16>(packet.SerializedSize(100)) > available) break;
					packet.Serialize(outgoing, 100);
					queue.pop_front();
				}
				debugStream.Print( 0x02000000, "\n");
			}
			else
			{
				packet1 = queue[0];
				queue.clear();
			}
			packet1.address = address;
			packet1.field0x24 = localID;
			SendReliable(packet1);
		}
	}
}
