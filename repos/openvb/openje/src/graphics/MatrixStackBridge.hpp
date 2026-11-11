#pragma once

#include <windows.h>

namespace JE
{
	namespace Gfx
	{
		namespace MatrixStackBridge
		{
			// Actual D3DX COM object, kept opaque to D3D8 translation units.
			typedef void* Handle;

			HRESULT Create(DWORD flags, Handle* output);
			ULONG Release(Handle stack);
		}
	}
}
