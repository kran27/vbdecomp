#pragma once

#include "GraphicsBaseObject.hpp"
#include <d3d8.h>

// Actual Gfx_Light identity and seven inherited virtual slots. This declaration
// recovers the B4-byte owner; the constructor uses real SDK matrix/descriptor types.
class Gfx_Light : public Gfx_BaseObject
{
public:
	explicit Gfx_Light(DWORD value = 0); // 0x4B8E50; array wrapper0x4B90B0 supplies0.
	Gfx_Light(const D3DLIGHT8& descriptor, DWORD value) //4247E0; flag48/padding unwritten.
		: value44(value)
	{
		light = descriptor;
	}
	virtual ~Gfx_Light(); // Actual deleting wrapper0x4B9050, normal base cleanup.
	virtual void SetPosition(float x, float y, float z) // +0C ->0x424840.
	{
		transform._41 = x;
		transform._42 = y;
		transform._43 = z;
		light.Position.x = x;
		light.Position.y = y;
		light.Position.z = z;
	}

	DWORD value44; // +44; nonzero lights are skipped when AutomapReady is nonzero.
	BYTE flag48; // +48; original ctor writes only this byte.
	BYTE unknown49[3]; // +49..4B remains unwritten by that constructor.
	D3DLIGHT8 light; // +4C; SDK descriptor, including Position+80 and Range+98.
};

// Observed eighteen-word cdecl descriptor API424740; ownership unresolved.
// EAX returns output, while the observed caller ignores it; nominal spelling inferred.
D3DLIGHT8* Gfx_InitLightDescriptor(D3DLIGHT8* output, D3DLIGHTTYPE type,
	float diffuseRed, float diffuseGreen, float diffuseBlue,
	float specularRed, float specularGreen, float specularBlue,
	float ambientRed, float ambientGreen, float ambientBlue,
	float range, float falloff, float attenuation0, float attenuation1,
	float attenuation2, float theta, float phi);
