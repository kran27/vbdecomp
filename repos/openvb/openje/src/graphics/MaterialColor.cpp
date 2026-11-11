#include "Material.hpp"

void Gfx_Material::SetColor(DWORD color, BYTE useRGB, BYTE useAlpha)
{
	if (useRGB)
	{
		double alpha = values.Diffuse.a;
		values.Diffuse.r = ((color >> 16) & 0xFF) * (1.0f / 255.0f);
		values.Diffuse.g = ((color >> 8) & 0xFF) * (1.0f / 255.0f);
		values.Diffuse.b = (color & 0xFF) * (1.0f / 255.0f);
		values.Diffuse.a = alpha;
	}
	if (useAlpha) values.Diffuse.a = (color >> 24) * (1.0f / 255.0f);
	values.Ambient = values.Diffuse;
	textureFactorColor = useRGB;
	textureFactorAlpha = useAlpha;
}

void Gfx_Material::SetEmissive(DWORD color)
{
	values.Emissive.r = ((color >> 16) & 0xFF) * (1.0f / 255.0f);
	values.Emissive.g = ((color >> 8) & 0xFF) * (1.0f / 255.0f);
	values.Emissive.b = (color & 0xFF) * (1.0f / 255.0f);
	values.Emissive.a = 1;
}
