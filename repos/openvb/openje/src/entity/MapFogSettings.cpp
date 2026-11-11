#include "Map.hpp"
#include "World.hpp"
#include "graphics/SystemInterface.hpp"
#include <cstring>

namespace
{
	DWORD ActiveMapID()
	{
		JE::IWorld* world = JE::g_World;
		return world->GetCurrentMapID();
	}


}

namespace JE
{
	void Map::GetCameraFog(float& nearDistance, float& farDistance, DWORD& color,
		float& density, BYTE& enabled) const
	{
		enabled = flag1B1;
		nearDistance = fogNear;
		farDistance = fogFar;
		color = fogColor;
		density = fogScale;
	}

	void Map::GetHeightFog(float& totalHeight, float& baseHeight, DWORD& color,
		float& maximumDensity, BYTE& enabled) const
	{
		totalHeight = value18C;
		baseHeight = value180;
		color = value17C;
		maximumDensity = value19C;
		enabled = flag178;
	}

	void Map::SetCameraFog(float nearDistance, float farDistance, DWORD color,
		float density, BYTE enabled)
	{
		flag1B1 = enabled;
		fogNear = nearDistance;
		fogFar = farDistance;
		fogColor = color;
		fogScale = density;
		const DWORD activeMapID = ActiveMapID();
		if (activeMapID == idB4)
		{
			g_GfxSystem->SetFogEnabled(flag1B1, NULL);
			g_GfxSystem->SetFogColor(&fogColor, NULL);
			g_GfxSystem->SetFogRange(fogNear, fogFar, NULL);
			g_GfxSystem->SetFogDensity(fogScale, NULL);
		}
	}

	void Map::ResetCameraFog()
	{
		flag1B1 = fogFlagCopy;
		fogNear = fogNearCopy;
		fogFar = fogFarCopy;
		//56D579/56D5A4: FLD then real CRT truncation, retaining low32 bits.
		fogColor = static_cast<DWORD>(static_cast<__int64>(fogColorCopy));
		fogScale = fogScaleCopy;
		const DWORD activeMapID = ActiveMapID();
		if (activeMapID == idB4)
		{
			g_GfxSystem->SetFogEnabled(flag1B1, NULL);
			g_GfxSystem->SetFogColor(&fogColor, NULL);
			g_GfxSystem->SetFogRange(fogNear, fogFar, NULL);
			g_GfxSystem->SetFogDensity(fogScale, NULL);
		}
	}

	void Map::SetHeightFog(float totalHeight, float baseHeight, DWORD color,
		float maximumDensity, BYTE enabled)
	{
		value18C = totalHeight;
		value19C = maximumDensity;
		value180 = baseHeight;
		value17C = color;
		flag178 = enabled;
		//The original publishes this backend enable even for an inactive map.
		g_GfxSystem->EnableHeightFog(enabled, NULL);
		const DWORD activeMapID = ActiveMapID();
		if (activeMapID == idB4 && flag178)
			g_GfxSystem->CreateHeightFog(value17C, value180,
				*reinterpret_cast<const DWORD*>(&value184),
				*reinterpret_cast<const DWORD*>(&value188), value18C,
				*reinterpret_cast<const DWORD*>(&value190),
				*reinterpret_cast<const DWORD*>(&value194),
				*reinterpret_cast<const DWORD*>(&value198), value19C, NULL);
	}

	void Map::ResetHeightFog()
	{
		value18C = value1A8;
		value180 = value1A4;
		value17C = value1A0;
		value19C = value1AC;
		flag178 = flag1B0;
		//Unlike SetHeightFog, reset neither enables/disables nor destroys fog.
		const DWORD activeMapID = ActiveMapID();
		if (activeMapID == idB4 && flag178)
			g_GfxSystem->CreateHeightFog(value17C, value180,
				*reinterpret_cast<const DWORD*>(&value184),
				*reinterpret_cast<const DWORD*>(&value188), value18C,
				*reinterpret_cast<const DWORD*>(&value190),
				*reinterpret_cast<const DWORD*>(&value194),
				*reinterpret_cast<const DWORD*>(&value198), value19C, NULL);
	}
}
