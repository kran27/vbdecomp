#pragma once

#include <windows.h>

// Actual56-byte segment: two float3 endpoints followed by eight LONG channels
// (RGBA for the first and second endpoint). Normal value storage, not an overlay.
struct Gfx_LineSystemValue
{
	DWORD values[14];
};
