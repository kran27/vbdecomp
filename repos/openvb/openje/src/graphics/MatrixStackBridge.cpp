#include "MatrixStackBridge.hpp"

// The real DirectX8.1 SDK library provides the original matrix-stack API.
#include <d3dx8math.h>

namespace JE
{
	namespace Gfx
	{
		namespace MatrixStackBridge
		{
			HRESULT Create(DWORD flags, Handle* output)
			{
				// Preserve the SDK's direct output write and unchanged HRESULT.
				return D3DXCreateMatrixStack(flags,
					reinterpret_cast<ID3DXMatrixStack**>(output));
			}

			ULONG Release(Handle stack)
			{
				return static_cast<ID3DXMatrixStack*>(stack)->Release();
			}
		}
	}
}
