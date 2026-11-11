#pragma once

#include "MapFog.hpp"
#include "graphics/VfxValueOwners.hpp"

namespace JE
{
	struct MapEntryPoint
	{
		VFX_Rotation positions[6]; // 46CB30 constructs each float triple through 52C260.
		float values[6];
	};
}
