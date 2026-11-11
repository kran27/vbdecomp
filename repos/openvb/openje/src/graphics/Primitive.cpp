#include "Primitive.hpp"

Gfx_Primitive::Gfx_Primitive()
{
	value00 = 0xFFFFFFFF;
	type = D3DPT_TRIANGLELIST;
	value24 = 0;
	value28 = 0;
	value2C = 0;
	value30 = 0;
}

Gfx_Primitive::~Gfx_Primitive() {}
