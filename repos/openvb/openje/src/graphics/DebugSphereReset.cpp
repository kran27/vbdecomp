#include "DebugSphere.hpp"

void Gfx_DebugSphere505880::Reset()
{
	Gfx_BasePacketData* capturedPacket = packet;
	capturedPacket->scale.x = 1.0f;
	capturedPacket->scale.y = 1.0f;
	capturedPacket->scale.z = 1.0f;
	radius = 1.0f;
	D3DXMatrixIdentity(&transform);
}
