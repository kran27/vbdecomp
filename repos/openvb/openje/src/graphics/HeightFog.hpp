#pragma once

#include "GraphicsBaseObject.hpp"

// Actual RTTI class and100-byte allocation. It inherits the seven real base
// virtual operations; original table6744DC changes only deleting destruction.
class Gfx_HeightFog : public Gfx_BaseObject
{
public:
	Gfx_HeightFog(); //0x505D90; value48 deliberately remains unwritten.
	virtual ~Gfx_HeightFog(); //Ordinary0x505E50; always deletes texture handle.
	void Evaluate(float seconds, float* baseHeight, float* totalHeight); //505DF0.
	HRESULT CreateFog(D3DCOLOR color, float baseHeight, float baseRate, float baseAmplitude,
		float totalHeight, float totalRate, float totalAmplitude, float maximumDensity); //505ED0.

	DWORD value44;
	float value48; //Unwritten by the observed constructor; base height after CreateFog.
	float value4C;
	DWORD color50;
	float value54, value58, value5C, value60;
};
