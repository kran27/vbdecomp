#include "Viewport.hpp"
#include "RenderManager.hpp"
#include "D3DState.hpp"
#include <cstring>

void Gfx_Viewport::Render(BYTE clear)
{
	activeCamera->Prepare();
	g_D3D.SetRenderState(D3DRS_FOGENABLE, fogEnabled);
	// Read the live raw flag again after the SDK call, then transport each
	// floating field as a DWORD without an FP load or NaN conversion.
	if (fogEnabled)
	{
		DWORD bits;
		std::memcpy(&bits, &fogStart, sizeof(bits));
		g_D3D.SetRenderState(D3DRS_FOGSTART, bits);
		std::memcpy(&bits, &fogEnd, sizeof(bits));
		g_D3D.SetRenderState(D3DRS_FOGEND, bits);
		std::memcpy(&bits, &fogDensity, sizeof(bits));
		g_D3D.SetRenderState(D3DRS_FOGDENSITY, bits);
	}
	DWORD flags = clearFlags;
	if (flags)
	{
		if (flag1C)
		{
			flags &= ~D3DCLEAR_TARGET;
			flag1C = 0;
		}
		// Keep the call even when suppressing TARGET leaves a zero mask.
		g_D3D.device->Clear(0, 0, flags, clearColor, clearDepth, clearStencil);
	}
	renderer->Render(clear);
}
