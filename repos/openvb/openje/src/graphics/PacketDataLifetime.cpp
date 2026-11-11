#include "PacketData.hpp"

Gfx_BasePacketData::Gfx_BasePacketData(DWORD value)
	: ambientOverride(0), unknown0C(value), scale(1, 1, 1),
	perPrimitiveLights(0)
{
	transformEnabled = 0;
	transform = 0;
	extraLight.x = 0;
	extraLight.y = 0;
	extraLight.z = 0;
	extraLight.w = 1;
	ambientColor = 0x808080;
}

Gfx_BasePacketData::~Gfx_BasePacketData()
{
	// Original destroys primitive-light maps before the shared borrowed-light list.
}

Gfx_PacketData::~Gfx_PacketData()
{
}

Gfx_PacketData* Gfx_CreatePacketData(DWORD meshWord)
{
	return new Gfx_PacketData;
}
