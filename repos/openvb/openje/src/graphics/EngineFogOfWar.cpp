#include "Engine.hpp"
#include "SurfaceRelease.hpp"
#include "D3DState.hpp"
#include "Texture.hpp"
#include "diagnostics/DebugOutput.hpp"
#include <dxerr8.h>

namespace
{
	struct FogVertex
	{
		float x, y, z, rhw;
		DWORD color;
		float u, v;
	};

}

void Gfx_Engine::ProcessFogOfWar()
{
	if (!automapMaskHandle || !visibleMaskHandle)
	{
		return;
	}
	Gfx_Texture* mask = AutomapMaskTexture();
	IDirect3DTexture8* explored = mask->frameCount > 0 ? mask->frames[0].texture : NULL;
	mask = VisibleMaskTexture();
	IDirect3DTexture8* visible = mask->frameCount > 0 ? mask->frames[0].texture : NULL;
	if (!fogOfWarTarget || !explored || !visible)
	{
		JE::LogDebugString("Gfx_Engine::ProcessFogOfWar - trying to use an invalid texture!\n");
		return;
	}

	IDirect3DSurface8* originalTarget = NULL;
	IDirect3DSurface8* originalDepth = NULL;
	IDirect3DSurface8* fogSurface = NULL;
	HRESULT result = g_D3D.device->GetRenderTarget(&originalTarget);
	if (FAILED(result))
	{
		const char* description = DXGetErrorDescription8A(result);
		const char* name = DXGetErrorString8A(result);
		JE::LogDebugString("Gfx_Engine::ProcessFogOfWar - failed to get original render target: %s(%x) - %s\n",
			name, result, description);
		return;
	}
	result = g_D3D.device->GetDepthStencilSurface(&originalDepth);
	if (FAILED(result))
	{
		const char* description = DXGetErrorDescription8A(result);
		const char* name = DXGetErrorString8A(result);
		JE::LogDebugString("Gfx_Engine::ProcessFogOfWar - failed to get original depth/stencil: %s(%x) - %s\n",
			name, result, description);
		Gfx_ReleaseSurface(originalTarget);
		return;
	}
	result = fogOfWarTarget->GetSurfaceLevel(0, &fogSurface);
	if (FAILED(result))
	{
		const char* description = DXGetErrorDescription8A(result);
		const char* name = DXGetErrorString8A(result);
		JE::LogDebugString("Gfx_Engine::ProcessFogOfWar - failed to get FOW render target: %s(%x) - %s\n",
			name, result, description);
		Gfx_ReleaseSurface(originalTarget);
		Gfx_ReleaseSurface(originalDepth);
		return;
	}

	const float right = static_cast<float>(maskWidth) - 1.0f;
	const float bottom = static_cast<float>(maskHeight) - 1.0f;
	const DWORD exploredColor = exploredGray;
	const DWORD visibleColor = visibleGray;
	const FogVertex exploredQuad[4] =
	{
		{0.0f, 0.0f, 0.01f, 1.0f, exploredColor, 0.0f, 0.0f},
		{right, 0.0f, 0.01f, 1.0f, exploredColor, 1.0f, 0.0f},
		{0.0f, bottom, 0.01f, 1.0f, exploredColor, 0.0f, 1.0f},
		{right, bottom, 0.01f, 1.0f, exploredColor, 1.0f, 1.0f}
	};
	const FogVertex visibleQuad[4] =
	{
		{0.0f, 0.0f, 0.01f, 1.0f, visibleColor, 0.0f, 0.0f},
		{right, 0.0f, 0.01f, 1.0f, visibleColor, 1.0f, 0.0f},
		{0.0f, bottom, 0.01f, 1.0f, visibleColor, 0.0f, 1.0f},
		{right, bottom, 0.01f, 1.0f, visibleColor, 1.0f, 1.0f}
	};
	result = g_D3D.device->SetRenderTarget(fogSurface, NULL);
	if (SUCCEEDED(result))
	{
		++g_D3D.statistics[6];
		result = g_D3D.device->BeginScene();
		if (SUCCEEDED(result))
		{
			if (revealExplored)
			{
				g_D3D.device->SetVertexShader(0x144);
				g_D3D.SetRenderState(D3DRS_ZENABLE, 0);
				g_D3D.SetRenderState(D3DRS_ALPHABLENDENABLE, 0);
				g_D3D.SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
				g_D3D.SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
				g_D3D.SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
				g_D3D.SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
				g_D3D.SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
				g_D3D.SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
				g_D3D.SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
				g_D3D.SetTexture(0, explored);
				const UINT stride = D3DXGetFVFVertexSize(0x144);
				result = g_D3D.device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2,
					exploredQuad, stride);
				if (FAILED(result))
				{
					const char* description = DXGetErrorDescription8A(result);
					const char* name = DXGetErrorString8A(result);
					JE::LogDebugString("Gfx_Engine::ProcessFogOfWar - Draw Automap failed: %s(%x) - %s\n",
						name, result, description);
				}
				g_D3D.SetRenderState(D3DRS_ALPHABLENDENABLE, 1);
				g_D3D.SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
				g_D3D.SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
				g_D3D.SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
			}
			else
			{
				//Unlike the quads, the original clear reloads the live Engine color.
				g_D3D.device->Clear(0, NULL, D3DCLEAR_TARGET, exploredGray, 0.0f, 0);
				g_D3D.device->SetVertexShader(0x144);
				g_D3D.SetRenderState(D3DRS_ZENABLE, 0);
				g_D3D.SetRenderState(D3DRS_ALPHABLENDENABLE, 1);
				g_D3D.SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
				g_D3D.SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
				g_D3D.SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
				g_D3D.SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
				g_D3D.SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
				g_D3D.SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
				g_D3D.SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
				g_D3D.SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
				g_D3D.SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
				g_D3D.SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
			}
			g_D3D.SetTexture(0, visible);
			const UINT stride = D3DXGetFVFVertexSize(0x144);
			result = g_D3D.device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, visibleQuad, stride);
			if (FAILED(result))
			{
				const char* description = DXGetErrorDescription8A(result);
				const char* name = DXGetErrorString8A(result);
				JE::LogDebugString("Gfx_Engine::ProcessFogOfWar - Draw VisibleMask failed: %s(%x) - %s\n",
					name, result, description);
			}
			g_D3D.SetRenderState(D3DRS_ALPHABLENDENABLE, 0);
			++g_D3D.statistics[7];
			g_D3D.device->EndScene();
		}
		else
		{
			const char* description = DXGetErrorDescription8A(result);
			const char* name = DXGetErrorString8A(result);
			JE::LogDebugString("Gfx_Engine::ProcessFogOfWar - BeginScene failed: %s(%x) - %s\n",
				name, result, description);
		}
		g_D3D.device->SetRenderTarget(originalTarget, originalDepth);
	}
	else
	{
		const char* description = DXGetErrorDescription8A(result);
		const char* name = DXGetErrorString8A(result);
		JE::LogDebugString("Gfx_Engine::ProcessFogOfWar - SetRenderTarget failed:  %s(%x) - %s\n",
			name, result, description);
	}
	Gfx_ReleaseSurface(fogSurface);
	Gfx_ReleaseSurface(originalTarget);
	Gfx_ReleaseSurface(originalDepth);
}
