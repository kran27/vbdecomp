#pragma once

#include "Address.hpp"
#include "Buffer.hpp"

namespace JE
{
	namespace Net
	{
		class Packet : public Buffer
		{
		public:
			Packet(); // 0x45F300.
			virtual ~Packet() {} //45F280; actual Address/Buffer destruction is inline-visible.
			Packet(const Packet& source); // 0x5ABCE0.
			Packet& operator=(const Packet& source); // 0x5AC630.
			void Reset(); // 0x5AC5B0; preserves Buffer storage/ownership and payload.
			UInt32 SerializedSize(UInt8 fields) const; //5AC5E0, excludes fields BYTE.
			void Serialize(Buffer& output, UInt8 fields) const; //5AC690.
			void Deserialize(Buffer& input); //5AC7B0.

			Address address;
			UInt16 field0x24;
			UInt32 field0x28; //Sequence, high bit is reliable marker.
			UInt32 field0x2C; //Acknowledgment sequence/reliable bit.
			UInt32 field0x30; //Message type (movement59/60).
			UInt32 field0x34; //Receive timestamp; ctor-unwritten, Reset zero.
			// The constructor does not write payload bytes. This is inline storage,
			// not an original-layout object or a guessed serialized packet header.
			UInt8 payload[8192];
		};
	}
}
