#include "PacketData.hpp"
#include "SystemInterface.hpp"

void Gfx_BasePacketData::ClearLights()
{
	lights.clear();
	for (std::map<DWORD,Gfx_LightList>::iterator i=primitiveLights.begin();
		i!=primitiveLights.end();++i) i->second.clear();
	primitiveLights.clear();
}

BYTE Gfx_BasePacketData::AddLight(Gfx_Light* light,DWORD primitiveID)
{
	BYTE result = 0;
	Gfx_System_Interface* system = g_GfxSystem;
	const DWORD count = LightCount(primitiveID);
	if (count < system->MaximumActiveLights())
	{
		primitiveLights[primitiveID].push_back(light);
		perPrimitiveLights=1;
		result = 1;
	}
	else
		perPrimitiveLights=1;
	return result;
}
