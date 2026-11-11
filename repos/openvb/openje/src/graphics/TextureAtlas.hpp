#pragma once

#include <windows.h>

// Original4CA590 copies two double pairs from the game's fixed slot table.
BYTE Gfx_GetTextureRegion(LONG slot, double* minimum, double* maximum);
