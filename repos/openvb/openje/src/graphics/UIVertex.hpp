#pragma once

#include <windows.h>

namespace JE
{
namespace Gfx
{
	// Actual Direct3D8 FVF0x144 stream boundary, shared by both compiler lanes.
	struct UIVertex
	{
		float x, y, z, rhw;
		DWORD diffuse;
		float u, v;
	};
	typedef char UIVertexSize[(sizeof(UIVertex) == 28) ? 1 : -1];
}
}
