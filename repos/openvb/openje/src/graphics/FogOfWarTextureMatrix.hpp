#pragma once
#include <d3dx8math.h>

// Common source factor of original53E823..53E905/53EFxx/54258F..542673.
// Uses the actual level AABB, active camera inverse view and real D3DX math.
void Gfx_BuildFogOfWarTextureMatrix(D3DXMATRIX& output);

