#include <winsock2.h>
#include "network/EngineClient.hpp"
#include "NetworkComponent.hpp"
#include <cstring>
#include <d3dx8math.h>

namespace JE
{
	void Client::RequestKill(DWORD entity) //589980.
	{
		if (!(F3::g_pNetworkInstance->GetMode() & 1)) return;
		Net::Packet packet;
		packet.field0x30 = 11;
		packet.WriteDword(entity);
		Send(packet, 1);
	}

	void Client::RequestForceEntityStep(DWORD entity, D3DXVECTOR3 position) //589FD0.
	{
		Net::Packet packet;
		packet.field0x30 = 59;
		packet.WriteDword(entity);
		//The original reuses one actual float local, then reads its raw word.
		float value;
		value = position.x; packet.WriteDword(*reinterpret_cast<Net::UInt32*>(&value));
		value = position.y; packet.WriteDword(*reinterpret_cast<Net::UInt32*>(&value));
		value = position.z; packet.WriteDword(*reinterpret_cast<Net::UInt32*>(&value));
		Send(packet, 0);
	}

	void Client::RequestMoveTo(DWORD entity, D3DXVECTOR3 position) //58A0E0.
	{
		Net::Packet packet;
		packet.field0x30 = 60;
		packet.WriteDword(entity);
		//The original reuses one actual float local, then reads its raw word.
		float value;
		value = position.x; packet.WriteDword(*reinterpret_cast<Net::UInt32*>(&value));
		value = position.y; packet.WriteDword(*reinterpret_cast<Net::UInt32*>(&value));
		value = position.z; packet.WriteDword(*reinterpret_cast<Net::UInt32*>(&value));
		Send(packet, 0);
	}

	void Client::RequestTestAnimation(DWORD entity, DWORD animation, DWORD variation,
		WORD loops, BYTE backwards) //589B60.
	{
		Net::Packet packet;
		packet.field0x30 = 26;
		packet.WriteDword(entity);
		packet.WriteDword(animation);
		packet.WriteDword(variation);
		packet.WriteWord(loops);
		packet.WriteByte(backwards);
		Send(packet, 1);
	}
}
