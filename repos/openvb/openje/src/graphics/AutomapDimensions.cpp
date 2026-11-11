#include "AutomapDimensions.hpp"

UINT __fastcall Gfx_NextPowerOfTwo(UINT value) //0x5BE130, retains the original zero case.
{
	if ((value - 1) & value)
	{
		value |= value >> 1;
		value |= value >> 2;
		value |= value >> 4;
		value |= value >> 8;
		return (value | (value >> 16)) + 1;
	}
	return value;
}

void Gfx_CalculateMaskDimensions(const float* low, const float* high,
	UINT maximumWidth, UINT maximumHeight, UINT* width, UINT* height)
{
	*width = static_cast<UINT>(static_cast<__int64>(double(high[0]) - low[0] + 0.5));
	*height = static_cast<UINT>(static_cast<__int64>(double(high[2]) - low[2] + 0.5));
	if (maximumWidth >= 1024) maximumWidth = 1024;
	if (maximumHeight >= 1024) maximumHeight = 1024;
	*width = Gfx_NextPowerOfTwo(*width);
	while (*width > maximumWidth) *width >>= 1;
	*height = Gfx_NextPowerOfTwo(*height);
	while (*height > maximumHeight) *height >>= 1;
	if (*width <= 256) *width = 256;
	if (*height <= 256) *height = 256;
}
