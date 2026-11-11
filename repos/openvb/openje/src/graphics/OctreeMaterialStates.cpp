#include "OctreeRender.hpp"
#include <mmsystem.h>

BYTE Octree2Gfx::GetMaterialState(int key)
{
	std::map<int, BYTE>::iterator found = materialStates.find(key);
	if (found == materialStates.end()) return 0;
	return found->second;
}

void Octree2Gfx::SetMaterialState(int key, BYTE value)
{
	BYTE previous = GetMaterialState(key);
	if ((value != 0) != (previous != 0))
	{
		if (value)
		{
			std::map<int, OctreeFadeState>::iterator found = fadeStates.find(key);
			if (found != fadeStates.end())
			{
				if (found->second.direction)
				{
					DWORD elapsed = timeGetTime() - found->second.timestamp;
					found->second.timestamp = timeGetTime() + elapsed - 300;
					found->second.direction = 0;
				}
			}
			else
			{
				OctreeFadeState state;
				state.direction = 0;
				state.timestamp = timeGetTime();
				fadeStates[key] = state;
			}
		}
		else
		{
			std::map<int, OctreeFadeState>::iterator found = fadeStates.find(key);
			if (found != fadeStates.end())
			{
				if (!found->second.direction)
				{
					DWORD elapsed = timeGetTime() - found->second.timestamp;
					found->second.timestamp = timeGetTime() + elapsed - 300;
					found->second.direction = 1;
				}
			}
			else
			{
				OctreeFadeState state;
				state.direction = 1;
				state.timestamp = timeGetTime();
				fadeStates[key] = state;
			}
		}
		materialStates[key] = value;
	}
}

void Octree2Gfx::UpdateMaterialStates()
{
	bool changed = false;
	for (std::set<int>::iterator item = valueC0.begin(); item != valueC0.end(); ++item)
	{
		const int key = *item;
		std::set<int>::iterator previous = valueCC.find(key);
		if (previous == valueCC.end()) SetMaterialState(key, 0);
		else valueCC.erase(previous);
	}
	for (std::set<int>::iterator item = valueCC.begin(); item != valueCC.end(); ++item)
	{
		SetMaterialState(*item, 1);
	}
	std::map<int, OctreeFadeState>::iterator fade = fadeStates.begin();
	while (fade != fadeStates.end())
	{
		bool used = false;
		int key = fade->first;
		DWORD elapsed = timeGetTime() - fade->second.timestamp;
		if (elapsed > 300) elapsed = 300;
		// Original keeps the forward fraction in x87; only the reverse opacity
		// amount is stored as float before its alpha conversion. The bounded
		// integer0..300 products are exact in binary64; no early float rounding.
		double fraction = elapsed * static_cast<double>(0.0033333334f);
		int colorAlpha, binaryAlpha, opacityAlpha;
		if (fade->second.direction)
		{
			float amount = static_cast<float>(1.0 - fraction);
			colorAlpha = 102 - static_cast<int>((1.0 - fraction) * -153.0f);
			binaryAlpha = fraction != 0 ? 255 : 0;
			opacityAlpha = static_cast<int>(amount * 255.0);
		}
		else
		{
			colorAlpha = 102 - static_cast<int>(fraction * -153.0f);
			binaryAlpha = fraction != 1.0 ? 255 : 0;
			opacityAlpha = static_cast<int>(fraction * 255.0f);
		}
		std::map<int, std::list<Gfx_Material*> >::iterator opacityGroup = opacityMaterials.find(key);
		if (opacityGroup != opacityMaterials.end())
		{
			std::list<Gfx_Material*>::iterator item = opacityGroup->second.begin();
			if (item != opacityGroup->second.end())
			{
				used = true;
				do
				{
				Gfx_Material* material = *item;
				if (opacityAlpha == 255)
				{
					material->flag240 = 1;
					material->ResetTextureFactor();
					material->mode28 = material->mode24;
				}
				else if (opacityAlpha)
				{
					material->flag240 = 1;
					material->SetColor(static_cast<DWORD>(opacityAlpha) << 24, 0, 1);
					material->mode28 = 1;
				}
				else material->flag240 = 0;
				}
				while (++item != opacityGroup->second.end());
			}
		}
		std::map<int, std::list<Gfx_Material*> >::iterator colorGroup = colorMaterials.find(key);
		if (colorGroup != colorMaterials.end())
		{
			std::list<Gfx_Material*>::iterator item = colorGroup->second.begin();
			if (item != colorGroup->second.end())
			{
				used = true;
				do
				{
				Gfx_Material* material = *item;
				if (colorAlpha == 255)
				{
					material->flag240 = 1;
					material->ResetTextureFactor();
					material->mode28 = material->mode24;
				}
				else if (colorAlpha)
				{
					material->flag240 = 1;
					material->SetColor(static_cast<DWORD>(colorAlpha) << 24, 0, 1);
					material->mode28 = 1;
				}
				else material->flag240 = 0;
				}
				while (++item != colorGroup->second.end());
			}
		}
		std::map<int, std::list<Gfx_Material*> >::iterator enabledGroup = enabledMaterials.find(key);
		if (enabledGroup != enabledMaterials.end())
		{
			std::list<Gfx_Material*>::iterator item = enabledGroup->second.begin();
			if (item != enabledGroup->second.end())
			{
				used = true;
				do
				{
				Gfx_Material* material = *item;
				if (binaryAlpha == 255)
				{
					material->flag240 = 1;
					material->ResetTextureFactor();
					material->mode28 = material->mode24;
				}
				else if (colorAlpha) // Original tests colorAlpha here, not binaryAlpha.
				{
					material->flag240 = 1;
					material->SetColor(static_cast<DWORD>(binaryAlpha) << 24, 0, 1);
					material->mode28 = 1;
				}
				else material->flag240 = 0;
				}
				while (++item != enabledGroup->second.end());
			}
		}
		if (elapsed >= 300 || !used) fade = fadeStates.erase(fade);
		else ++fade;
		changed |= used;
	}
	if (changed)
	{
		for (DWORD index = 0; index < meshCount; ++index) meshes[index].materialSet.RebuildGroups();
	}
	valueCC = valueC0;
}
