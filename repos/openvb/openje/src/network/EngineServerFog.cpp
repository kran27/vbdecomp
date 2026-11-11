#include "EngineServer.hpp"
#include "entity/World.hpp"
#include "entity/Map.hpp"
#include <cstring>

namespace JE
{
	namespace
	{
		DWORD FloatWord(float value)
		{
			DWORD bits;
			std::memcpy(&bits,&value,sizeof(bits));
			return bits;
		}

	}

	void Server::SetCameraFog(DWORD map,float nearLimit,float farLimit,DWORD color,
		float density,BYTE enabled) //555F20, mutate before publishing captured arguments.
	{
		IWorld* world=g_World;
		world->GetMap(map)->SetCameraFog(nearLimit,farLimit,color,density,enabled);
		SendCameraFog(map,nearLimit,farLimit,color,density,enabled);
	}
	void Server::SetHeightFog(DWORD map,float upperLimit,float lowerLimit,DWORD color,
		float density,BYTE enabled) //555F90.
	{
		IWorld* world=g_World;
		world->GetMap(map)->SetHeightFog(upperLimit,lowerLimit,color,density,enabled);
		SendHeightFog(map,upperLimit,lowerLimit,color,density,enabled);
	}
	void Server::SendCameraFog(DWORD map,float nearLimit,float farLimit,DWORD color,float density,BYTE enabled) //553010.
	{
		Net::Packet packet;
		packet.field0x30=73;
		packet.WriteDword(map);
		packet.WriteDword(FloatWord(nearLimit));
		packet.WriteDword(FloatWord(farLimit));
		packet.WriteDword(color);
		packet.WriteDword(FloatWord(density));
		packet.WriteByte(enabled);
		Broadcast(packet,1);
	}
	void Server::SendHeightFog(DWORD map,float upperLimit,float lowerLimit,DWORD color,float density,BYTE enabled) //553180.
	{
		Net::Packet packet;
		packet.field0x30=74;
		packet.WriteDword(map);
		packet.WriteDword(FloatWord(upperLimit));
		packet.WriteDword(FloatWord(lowerLimit));
		packet.WriteDword(color);
		packet.WriteDword(FloatWord(density));
		packet.WriteByte(enabled);
		Broadcast(packet,1);
	}
	void Server::SendCameraFogReset(DWORD map) //5532F0.
	{
		Net::Packet packet;
		packet.field0x30=75;
		packet.WriteDword(map);
		Broadcast(packet,1);
	}
	void Server::SendHeightFogReset(DWORD map) //5533B0.
	{
		Net::Packet packet;
		packet.field0x30=76;
		packet.WriteDword(map);
		Broadcast(packet,1);
	}
	void Server::ResetCameraFog(DWORD map) //556040.
	{
		IWorld* world=g_World;
		Map* selected=world->GetMap(map);
		selected->ResetCameraFog();
		SendCameraFogReset(map);
	}
	void Server::ResetHeightFog(DWORD map) //556000.
	{
		IWorld* world=g_World;
		Map* selected=world->GetMap(map);
		selected->ResetHeightFog();
		SendHeightFogReset(map);
	}
}
