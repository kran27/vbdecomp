#include "Packet.hpp"
#include <cstring>

namespace JE
{
	namespace Net
	{
		Packet& Packet::operator=(const Packet& source)
		{
			Reset();
			address = source.address; // 0x5AC9E0: ordinary memberwise Address copy.
			field0x24 = source.field0x24;
			field0x28 = source.field0x28;
			field0x2C = source.field0x2C;
			field0x30 = source.field0x30;
			field0x0C = source.field0x0C;
			field0x10 = source.field0x10;
			// Preserve the native forward REP copy and preceding Reset/publication order.
			std::memcpy(payload, source.payload, sizeof(payload));
			return *this;
		}

		JE::Net::UInt32 Packet::SerializedSize(JE::Net::UInt8 fields) const
		{
			JE::Net::UInt32 size = 0;
			if (fields & 1) size = 6;
			if (fields & 2) size += 2;
			if (fields & 3) size += 4;
			if (fields & 4) size += 4;
			if (fields & 0x10) size += 4;
			if (fields & 0x20) size += 2;
			if (fields & 0x40) size += field0x10;
			return size; //Original deliberately omits the leading fields BYTE.
		}

		void Packet::Serialize(Buffer& output, JE::Net::UInt8 fields) const
		{
			output.WriteByte(fields);
			if (fields & 1)
			{
				output.WriteByte(address.bytes[0]);
				output.WriteByte(address.bytes[1]);
				output.WriteByte(address.bytes[2]);
				output.WriteByte(address.bytes[3]);
				output.WriteWord(address.port);
			}
			if (fields & 2) output.WriteWord(field0x24);
			if (fields & 3)
			{
				output.WriteDword(field0x28);
				if (field0x28 != 0xFFFFFFFF) output.WriteDword(field0x2C);
			}
			if (fields & 4) output.WriteDword(field0x30);
			if (fields & 0x10) output.WriteDword(field0x34);
			if (fields & 0x40)
			{
				if (fields & 0x20) output.WriteWord(static_cast<JE::Net::UInt16>(field0x10));
				output.Write(data, field0x10);
			}
		}

		void Packet::Deserialize(Buffer& input)
		{
			Reset();
			JE::Net::UInt8 fields;
			input.ReadByte(fields);
			if (fields & 1)
			{
				input.ReadByte(address.bytes[0]);
				input.ReadByte(address.bytes[1]);
				input.ReadByte(address.bytes[2]);
				input.ReadByte(address.bytes[3]);
				input.ReadWord(address.port);
			}
			if (fields & 2) input.ReadWord(field0x24);
			if (fields & 3)
			{
				input.ReadDword(field0x28);
				if (field0x28 != 0xFFFFFFFF) input.ReadDword(field0x2C);
			}
			if (fields & 4) input.ReadDword(field0x30);
			if (fields & 0x10) input.ReadDword(field0x34);
			if (fields & 0x40)
			{
				JE::Net::UInt16 count;
				if (fields & 0x20) { input.ReadWord(count); CopyFrom(input, count); }
				else CopyFrom(input, input.field0x10 - input.field0x0C);
			}
		}
	}
}
