#include "HeightFog.hpp"
#include "SystemInterface.hpp"
#include "Texture.hpp"
#include "diagnostics/DebugOutput.hpp"
#include <cmath>

Gfx_HeightFog::Gfx_HeightFog()
	: value44(0), value4C(0), color50(0xFF000000), value54(0), value58(0),
	value5C(0), value60(0)
{
}

Gfx_HeightFog::~Gfx_HeightFog()
{
	g_GfxSystem->DeleteTexture(value44);
}

void Gfx_HeightFog::Evaluate(float seconds, float* baseHeight, float* totalHeight)
{
	const float twoPi = 6.2831854820251465f;
	float phase = seconds * value54;
	phase -= static_cast<LONG>(phase);
	*baseHeight = float(std::sin(phase * twoPi)
		* value58 + value48);
	phase = seconds * value5C;
	phase -= static_cast<LONG>(phase);
	*totalHeight = float(std::sin(phase * twoPi)
		* value60 + value4C);
}

HRESULT Gfx_HeightFog::CreateFog(D3DCOLOR color, float baseHeight, float baseRate,
	float baseAmplitude, float totalHeight, float totalRate, float totalAmplitude,
	float maximumDensity)
{
	if (baseHeight <= 0.0f)
		JE::FatalError("Gfx_HeightFog::CreateFog() - fog base_height <= 0.0f!");
	if (baseHeight > totalHeight)
		JE::FatalError("Gfx_HeightFog::CreateFog() - fog base_height > total_height!");
	if (totalHeight <= 0.0f)
		JE::FatalError("Gfx_HeightFog::CreateFog() - fog total_height <= 0.0f!");
	if (maximumDensity <= 0.0f || maximumDensity > 1.0f)
		JE::FatalError("Gfx_HeightFog::CreateFog() - invalid max_fog_density");
	value48 = baseHeight;
	value4C = totalHeight;
	value54 = baseRate;
	value58 = baseAmplitude;
	color50 = color;
	value5C = totalRate;
	value60 = totalAmplitude;
	if (!value44)
	{
		if (SUCCEEDED(g_GfxSystem->CreateAndAddTexture(&value44, 32, 32, 1,
			D3DFMT_A8R8G8B8, D3DPOOL_MANAGED)))
		{
			Gfx_Texture* texture = g_GfxSystem->LookupTexture(value44);
			if (texture)
			{
				std::string fogName("[FogMap]");
				texture->name = fogName;
			}
		}
		else JE::FatalError("Gfx_HeightFog::CreateFog() - Failed to create fog map!");
	}
	Gfx_Texture* texture = g_GfxSystem->LookupTexture(value44);
	D3DLOCKED_RECT locked;
	if (SUCCEEDED(texture->LockRect(0, &locked, NULL, 0, 0)))
	{
		for (DWORD row = 0; row < 32; ++row)
		{
			float density = maximumDensity - row * 0.03703703731298447f;
			if (density < 0.0f) density = 0.0f;
			const DWORD pixel = (color & 0xFFFFFF)
				| (static_cast<DWORD>(static_cast<LONG>(density * 255.0f)) << 24);
			DWORD offset = row << 7;
			for (DWORD remaining = 32; remaining; --remaining)
			{
				*reinterpret_cast<DWORD*>(static_cast<BYTE*>(locked.pBits) + offset) = pixel;
				offset += 4;
			}
		}
		return texture->UnlockRect(0);
	}
	else JE::FatalError("Gfx_HeightFog::CreateFog() - Failed to lock fog map!");
}
