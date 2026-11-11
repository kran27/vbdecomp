#pragma once

#include <windows.h>

UINT __fastcall Gfx_NextPowerOfTwo(UINT value); //5BE130; native ECX input, EAX result; zero remains zero.

// Actual5BE240 with the original selected-device maximum texture dimensions
// exposed as inputs. This factor does not construct a second graphics owner.
void Gfx_CalculateMaskDimensions(const float* low, const float* high,
	UINT maximumWidth, UINT maximumHeight, UINT* width, UINT* height);
