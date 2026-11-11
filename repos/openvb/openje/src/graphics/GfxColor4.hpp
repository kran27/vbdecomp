#pragma once

// Observed four-float color arithmetic. Original library ownership is unresolved.
struct Gfx_Color4f
{
	float values[4];

	Gfx_Color4f& Add(const Gfx_Color4f& other); //52C0B0, thiscall/RET4.
	Gfx_Color4f& Subtract(const Gfx_Color4f& other); //52C0E0, thiscall/RET4.
	Gfx_Color4f& Scale(float factor); //52C110, thiscall/RET4.
};

// Observed cdecl output-pointer transport. The original C++ return spelling is unproved.
Gfx_Color4f* Gfx_ScaleColor4f(Gfx_Color4f* output, const Gfx_Color4f& color, float factor); //52C140.
Gfx_Color4f* Gfx_AddColor4f(Gfx_Color4f* output, const Gfx_Color4f& left, const Gfx_Color4f& right); //52C1A0.
Gfx_Color4f* Gfx_SubtractColor4f(Gfx_Color4f* output, const Gfx_Color4f& left, const Gfx_Color4f& right); //52C200.

typedef char GfxColor4fExtent[(sizeof(Gfx_Color4f) == 16) ? 1 : -1];
