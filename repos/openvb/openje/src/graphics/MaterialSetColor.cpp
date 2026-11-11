#include "MeshData.hpp"

void Gfx_MaterialSet::SetColors(BYTE useRGB, BYTE useAlpha, DWORD color, const DWORD* emissive)
{
	DWORD emissiveColor;
	if (emissive) emissiveColor = *emissive;
	for (DWORD i = 0; i < count; ++i)
	{
		Gfx_Material* material = &materials[i];
		if (useRGB || useAlpha) material->SetColor(color, useRGB, useAlpha);
		if (emissive) material->SetEmissive(emissiveColor);
	}
}
