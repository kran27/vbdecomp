#include "RSBase.hpp"
#include "TextureTransform.hpp"

HRESULT Gfx_RenderStyle::SetTextureTransform(DWORD stage, BYTE flags,
	const Gfx_TextureParameters* parameters, float seconds)
{
	return Gfx_SetTextureTransform(stage, flags, parameters, seconds);
}
