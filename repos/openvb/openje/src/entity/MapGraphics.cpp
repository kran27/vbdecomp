#include "Map.hpp"
#include "graphics/OctreeRender.hpp"
#include "graphics/SystemInterface.hpp"

namespace JE
{
	void Map::CopyHiddenMaterials(std::set<int>& output) const
	{
		output.clear();
		for (std::map<int, std::set<DWORD> >::const_iterator i = hiddenMaterialOwners.begin();
			i != hiddenMaterialOwners.end(); ++i)
		{
			output.insert(i->first);
		}
	}

	void Map::ApplyHeightFog(Octree2Gfx* renderer)
	{
		if (flag178)
		{
			// These parameter slots carry the original float words unchanged.
			g_GfxSystem->CreateHeightFog(value17C, value180,
				*reinterpret_cast<const DWORD*>(&value184),
				*reinterpret_cast<const DWORD*>(&value188), value18C,
				*reinterpret_cast<const DWORD*>(&value190),
				*reinterpret_cast<const DWORD*>(&value194),
				*reinterpret_cast<const DWORD*>(&value198), value19C, NULL);
		}
		else
		{
			g_GfxSystem->DestroyHeightFog(NULL);
		}
		renderer->SetHeightFogFlag(flag178);
		g_GfxSystem->EnableHeightFog(flag178, NULL);
	}

	void Map::ApplyFogSettings()
	{
		g_GfxSystem->SetFogRange(fogNear, fogFar, NULL);
		g_GfxSystem->SetFogColor(&fogColor, NULL);
		g_GfxSystem->SetFogDensity(fogScale, NULL);
		g_GfxSystem->SetFogEnabled(flag1B1, NULL);
	}
}
