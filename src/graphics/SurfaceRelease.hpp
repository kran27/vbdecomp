#pragma once

#include "d3d9.h"

// 5024D0: releases the pointed-to surface, then clears its slot.
ULONG __cdecl Gfx_ReleaseSurface(IDirect3DSurface9*& surface);
