#include "Material.hpp"
#include "../diagnostics/DebugOutput.hpp"

void Gfx_Material::SetValues(const D3DMATERIAL8& source)
{
	values = source;
	values.Ambient = values.Diffuse;
	cachedValues = source;
}

void Gfx_Material::ResetTextureFactor()
{
	textureFactorColor = 0;
	textureFactorAlpha = 0;
	values = cachedValues;
}

void Gfx_TextureParameters::SetParam(DWORD index, float value)
{
	switch (index)
	{
	case 10:
		if (value > 1.0f)
		{
			reinterpret_cast<float*>(this)[index] = static_cast<float>(static_cast<int>(value));
			value34 = 1.0f / value28;
		}
		else
		{
			JE::LogDebugString("Gfx_TCFX_Param::SetParam - Anim Frame Width < 1.0f!\n");
		}
		break;
	case 11:
		if (value > 1.0f)
		{
			reinterpret_cast<float*>(this)[index] = static_cast<float>(static_cast<int>(value));
			value38 = 1.0f / value2C;
		}
		else
		{
			JE::LogDebugString("Gfx_TCFX_Param::SetParam - Anim Frame Height < 1.0f!\n");
		}
		break;
	case 12:
		if (value > 1.0f)
		{
			reinterpret_cast<float*>(this)[index] = static_cast<float>(static_cast<int>(value));
		}
		else
		{
			JE::LogDebugString("Gfx_TCFX_Param::SetParam - Anim Total Frame Count < 1.0f!\n");
		}
		break;
	case 13:
	case 14:
		return;
	default:
		// The original uses an x87 store, including the first two parameter words.
		reinterpret_cast<float*>(this)[index] = value;
		break;
	}
}
