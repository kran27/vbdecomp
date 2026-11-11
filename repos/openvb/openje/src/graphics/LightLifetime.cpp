#include "Light.hpp"

Gfx_Light::Gfx_Light(DWORD value)
	: flag48(0)
{
	value44 = value;
	light.Type = D3DLIGHT_DIRECTIONAL;
	light.Diffuse.r = 1;
	light.Diffuse.g = 1;
	light.Diffuse.b = 1;
	light.Diffuse.a = 1;
	light.Specular.r = 0;
	light.Specular.g = 0;
	light.Specular.b = 0;
	light.Specular.a = 1;
	light.Ambient.r = 0;
	light.Ambient.g = 0;
	light.Ambient.b = 0;
	light.Ambient.a = 1;
	light.Position.x = 0;
	light.Position.y = 0;
	light.Position.z = 0;
	light.Direction.x = 0;
	light.Direction.y = 0;
	light.Direction.z = 1;
	light.Range = 0;
	light.Falloff = 1;
	light.Attenuation0 = 1;
	light.Attenuation1 = 0;
	light.Attenuation2 = 0;
	light.Theta = 0;
	light.Phi = 0;
}

Gfx_Light::~Gfx_Light()
{
}

D3DLIGHT8* Gfx_InitLightDescriptor(D3DLIGHT8* output, D3DLIGHTTYPE type,
	float diffuseRed, float diffuseGreen, float diffuseBlue,
	float specularRed, float specularGreen, float specularBlue,
	float ambientRed, float ambientGreen, float ambientBlue,
	float range, float falloff, float attenuation0, float attenuation1,
	float attenuation2, float theta, float phi)
{
	output->Type = type;
	output->Diffuse.r = diffuseRed; output->Diffuse.g = diffuseGreen; output->Diffuse.b = diffuseBlue;
	output->Diffuse.a = 1.0f;
	output->Specular.r = specularRed; output->Specular.g = specularGreen; output->Specular.b = specularBlue;
	output->Specular.a = 1.0f;
	output->Ambient.r = ambientRed; output->Ambient.g = ambientGreen; output->Ambient.b = ambientBlue;
	output->Ambient.a = 1.0f;
	output->Direction.z = 1.0f;
	output->Position.x = output->Position.y = output->Position.z = 0.0f;
	output->Direction.x = output->Direction.y = 0.0f;
	output->Range = range; output->Falloff = falloff;
	output->Attenuation0 = attenuation0; output->Attenuation1 = attenuation1; output->Attenuation2 = attenuation2;
	output->Theta = theta; output->Phi = phi;
	return output;
}
