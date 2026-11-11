#include "PacketData.hpp"

DWORD Gfx_BasePacketData::LightCount(DWORD primitiveID)
{
	std::map<DWORD, Gfx_LightList>::iterator found = primitiveLights.find(primitiveID);
	if (found != primitiveLights.end()) return static_cast<DWORD>(found->second.size());
	return 0;
}

Gfx_LightList* Gfx_BasePacketData::FindLights(DWORD primitiveID)
{
	std::map<DWORD, Gfx_LightList>::iterator found = primitiveLights.find(primitiveID);
	if (found != primitiveLights.end()) return &found->second;
	return 0;
}
