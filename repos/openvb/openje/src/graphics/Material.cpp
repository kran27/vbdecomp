#include "Material.hpp"

Gfx_TextureParameters::Gfx_TextureParameters()
	: value00(0), value04(0), value08(1), value0C(0.5f), value10(1),
	value14(0.5f), value18(0.5f), value1C(0.5f), value20(0.5f), value24(15),
	value28(1), value2C(1), value30(1), value34(1), value38(1)
{
}

Gfx_Material::Gfx_Material()
	: style1(0), style2(0), mode24(0), mode28(0),
	textureFactorColor(0), textureFactorAlpha(0), flagB8(1), flagB9(1),
	flagBA(1), flagBB(1), flagBC(1), flagBD(0)
{
	flag240 = 1;
	values.Diffuse.r = 1;
	values.Diffuse.g = 1;
	values.Diffuse.b = 1;
	values.Diffuse.a = 1;
	values.Ambient.r = 1;
	values.Ambient.g = 1;
	values.Ambient.b = 1;
	values.Ambient.a = 1;
	values.Specular.r = 1;
	values.Specular.g = 1;
	values.Specular.b = 1;
	values.Specular.a = 1;
	values.Emissive.a = 1;
	values.Emissive.r = 0;
	values.Emissive.g = 0;
	values.Emissive.b = 0;
	values.Power = 0;
	cachedValues = values;
	for (int i = 0; i < 4; ++i)
	{
		textureHandles[i] = 0;
		values140[i] = 0;
	}
}

Gfx_Material::~Gfx_Material() {}
