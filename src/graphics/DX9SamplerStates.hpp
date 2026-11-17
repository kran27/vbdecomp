#pragma once

#include <d3d9.h>

// Texture-addressing and filtering settings moved from texture stages in DX8
// to sampler states in DX9.
inline constexpr D3DSAMPLERSTATETYPE Gfx_DX9SamplerStates[] = {
    D3DSAMP_ADDRESSU,  D3DSAMP_ADDRESSV,  D3DSAMP_ADDRESSW,      D3DSAMP_BORDERCOLOR, D3DSAMP_MAGFILTER,
    D3DSAMP_MINFILTER, D3DSAMP_MIPFILTER, D3DSAMP_MIPMAPLODBIAS, D3DSAMP_MAXMIPLEVEL, D3DSAMP_MAXANISOTROPY
};
