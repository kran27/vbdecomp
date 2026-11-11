#include "GraphicsDeviceState.hpp"
#include "SystemInterface.hpp"
#include "D3DState.hpp"
#include "diagnostics/DebugOutput.hpp"
#include "Viewport.hpp"

namespace JE
{
	void SetGraphicsClearColor(Gfx_Viewport& viewport, BYTE red, BYTE green, BYTE blue)
	{
		const DWORD color = 0xFF000000 | (DWORD(red) << 16) | (DWORD(green) << 8) | blue;
		viewport.clearColor = color;
		for (int i = 3; i >= 0; --i) viewport.vertices[i].diffuse = color;
	}
	HRESULT SetGraphicsWireframe(BYTE enabled)
	{
		return g_D3D.SetRenderState(D3DRS_FILLMODE, enabled ? D3DFILL_WIREFRAME : D3DFILL_SOLID);
	}

	HRESULT SetGraphicsAlphaTest(BYTE& flag, BYTE enabled)
	{
		g_D3D.SetRenderState(D3DRS_ALPHATESTENABLE, enabled ? 1 : 0);
		HRESULT result = g_D3D.SetRenderState(D3DRS_ALPHAFUNC, enabled ? D3DCMP_GREATER : D3DCMP_ALWAYS);
		flag = enabled;
		return result;
	}
}
