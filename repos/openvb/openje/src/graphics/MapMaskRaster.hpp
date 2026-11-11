#pragma once
#include <d3d8.h>
class Gfx_Texture;
namespace JE { class AreaMap; struct AreaMapBits; }
namespace MapMaskRaster
{
	void SetBit(JE::AreaMapBits& bits, LONG index, BYTE value); //Convenience forwarder to AreaMapBits5AD6D0.
	BYTE GetBit(const JE::AreaMapBits& bits, LONG index); //Convenience forwarder to actual AreaMapBits5AD760.
}
