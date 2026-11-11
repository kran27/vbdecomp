#pragma once
#include <windows.h>
#include <d3d8.h>

// SDK sample comparator, with the observed F3 refresh tie-break. Library reuse,
// not a separate proprietary reconstruction or extra function-progress target.
int __cdecl CompareD3DDisplayModes(const void* left, const void* right);
