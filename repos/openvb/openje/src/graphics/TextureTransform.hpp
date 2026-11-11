#pragma once

#include "Material.hpp"

// Original5434E0: four stack arguments, RET16, no consumed this input.
// Scroll, sine oscillation, center rotation and signed frame-atlas translation.
HRESULT __stdcall Gfx_SetTextureTransform(DWORD stage, BYTE flags,
	const Gfx_TextureParameters* parameters, float seconds);
