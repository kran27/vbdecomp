#include "GfxColor4.hpp"

Gfx_Color4f& Gfx_Color4f::Add(const Gfx_Color4f& other)
{
	values[0] += other.values[0];
	values[1] += other.values[1];
	values[2] += other.values[2];
	values[3] += other.values[3];
	return *this;
}

Gfx_Color4f& Gfx_Color4f::Subtract(const Gfx_Color4f& other)
{
	values[0] -= other.values[0];
	values[1] -= other.values[1];
	values[2] -= other.values[2];
	values[3] -= other.values[3];
	return *this;
}

Gfx_Color4f& Gfx_Color4f::Scale(float factor)
{
	values[0] *= factor;
	values[1] *= factor;
	values[2] *= factor;
	values[3] *= factor;
	return *this;
}

Gfx_Color4f* Gfx_ScaleColor4f(Gfx_Color4f* output, const Gfx_Color4f& color, float factor)
{
	Gfx_Color4f result(color);
	result.Scale(factor);
	*output = result;
	return output;
}

Gfx_Color4f* Gfx_AddColor4f(Gfx_Color4f* output, const Gfx_Color4f& left, const Gfx_Color4f& right)
{
	Gfx_Color4f result(left);
	result.Add(right);
	*output = result;
	return output;
}

Gfx_Color4f* Gfx_SubtractColor4f(Gfx_Color4f* output, const Gfx_Color4f& left, const Gfx_Color4f& right)
{
	Gfx_Color4f result(left);
	result.Subtract(right);
	*output = result;
	return output;
}
