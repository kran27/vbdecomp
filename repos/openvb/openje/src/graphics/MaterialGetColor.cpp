#include "Material.hpp"

void Gfx_Material::GetColor(BYTE& useRGB, BYTE& useAlpha, DWORD& color)
{
	useRGB = textureFactorColor;
	useAlpha = textureFactorAlpha;
	// Output aliases can change these inputs. Read channels only after both
	// flag writes; retain the original R/A/G/B conversion and packing order.
	DWORD packed = static_cast<int>(values.Diffuse.r * 255.0f) & 0xFF;
	packed |= static_cast<DWORD>(static_cast<int>(values.Diffuse.a * 255.0f)) << 8;
	packed <<= 8;
	packed |= static_cast<int>(values.Diffuse.g * 255.0f) & 0xFF;
	packed <<= 8;
	color = packed | (static_cast<int>(values.Diffuse.b * 255.0f) & 0xFF);
}
