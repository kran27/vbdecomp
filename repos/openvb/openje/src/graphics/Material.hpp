#pragma once

#include <d3d8.h>
#include <string>

// Descriptive source names; the complete original owned layouts are evidenced.
// Real SDK values and real VC7 strings supply their implementation and ABI.
class Gfx_TextureParameters
{
public:
	Gfx_TextureParameters(); // 0x4B5D00.
	void SetParam(DWORD index, float value); // 0x4B5D50; original parameter policy.
	DWORD value00, value04;
	float value08, value0C, value10, value14, value18, value1C, value20;
	float value24, value28, value2C, value30, value34, value38;
};

class Gfx_Material
{
public:
	Gfx_Material(); // 0x4B60B0.
	~Gfx_Material(); // 0x401720; normal owned-string destruction.
	void SetColor(DWORD color, BYTE useRGB, BYTE useAlpha); // 0x4B5EB0.
	void SetEmissive(DWORD color); // 0x4B5F90; packed alpha is ignored.
	void GetColor(BYTE& useRGB, BYTE& useAlpha, DWORD& color); // 0x4B6030; raw flags, unclamped ARGB.
	void SetValues(const D3DMATERIAL8& source); // 0x4B5E60; ambient follows diffuse.
	void ResetTextureFactor(); // 0x4B6010; restores the cached material.

	std::string name; // +00,28 bytes in the matching VC7 lane.
	DWORD style1, style2, mode24, mode28; // +1C..28.
	D3DMATERIAL8 values; // +2C..6F.
	BYTE textureFactorColor, textureFactorAlpha; // +70/+71; padding untouched.
	D3DMATERIAL8 cachedValues; // +74..B7.
	BYTE flagB8, flagB9, flagBA, flagBB, flagBC, flagBD; // +B8..BD; roles unresolved.
	std::string textureNames[4]; // +C0..12F.
	DWORD textureHandles[4]; // +130..13F.
	DWORD values140[4]; // +140..14F.
	Gfx_TextureParameters textureParameters[4]; // +150..23F.
	BYTE flag240; // +240; trailing padding untouched.
};
