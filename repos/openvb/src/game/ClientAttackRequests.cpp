#include "network/Types.hpp"
#include "ClientAttackRequests.hpp"
#include "GameActionAttack.hpp"
#include "entity/Entity.hpp"
#include "network/Client.hpp"
#include "network/EngineClient.hpp"
#include "network/Packet.hpp"
#include "diagnostics/DebugOutput.hpp"
#include <cstring>
#include <new>

namespace JE
{
	namespace
	{
		JE::Net::UInt32 FloatBits(float value)
		{
			JE::Net::UInt32 bits;
			std::memcpy(&bits,&value,sizeof bits);
			return bits;
		}
	}

	void Client::RequestAttack(DWORD attackerID, DWORD targetID, LONG calledShotPart,
		const D3DXVECTOR3* point)
	{
		JE::Net::Packet packet;
		packet.field0x30=91;
		packet.WriteDword(attackerID);
		packet.WriteDword(targetID);
		packet.WriteSignedDword(calledShotPart);
		packet.WriteByte(point!=NULL);
		if (point)
		{
			packet.WriteDword(FloatBits(point->x));
			packet.WriteDword(FloatBits(point->y));
			packet.WriteDword(FloatBits(point->z));
		}
		Send(packet,1); //Actual reliable queue; caller's real Process flushes it.
	}

}

namespace F3
{
	BYTE ReceiveGameServerAttackRequest(JE::Net::Packet& packet)
	{
		if (packet.field0x30!=91) return 0;
		JE::Net::UInt32 attackerID, targetID, calledShotPart;
		JE::Net::UInt8 pointTarget;
		packet.ReadDword(attackerID);
		packet.ReadDword(targetID);
		packet.ReadDword(calledShotPart);
		packet.ReadByte(pointTarget);
		D3DXVECTOR3 point(0,0,0);
		if (pointTarget)
		{
			float* values[3]={&point.x,&point.y,&point.z};
			for (unsigned index=0; index!=3; ++index)
			{
				JE::Net::UInt32 bits;
				packet.ReadDword(bits);
				std::memcpy(values[index],&bits,4);
			}
		}
		JE::Entity* attacker=JE::GetEntityPtr(attackerID);
		JE::Entity* target=JE::GetEntityPtr(targetID);
		if (attacker && (pointTarget || target))
		{
			GameActionAttack* action=new GameActionAttack;
			action->SetTarget(target);
			action->calledShotPart=calledShotPart;
			//401100 writes the raw request BYTE, then copies the point only for
			//a nonzero request. It does not reset targetID or prior actions.
			action->pointTarget=pointTarget;
			if (pointTarget) action->targetPosition=point;
			attacker->QueueAction(action);
		}
		else JE::LogDebugString("GameServer::recv_packet() - Entities for attack action not found.\n");
		return 1;
	}
}
