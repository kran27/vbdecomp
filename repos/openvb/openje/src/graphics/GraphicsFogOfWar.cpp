#include "GraphicsFogOfWar.hpp"
#include "GraphicsLevelResources.hpp"
#include "SystemInterface.hpp"
#include "Texture.hpp"
#include "D3DState.hpp"
#include "diagnostics/DebugOutput.hpp"
#include <d3dx8.h>
#include <dxerr8.h>

namespace
{
	void Failure(const char* operation, HRESULT result)
	{
		JE::LogDebugString("Gfx_Engine::ProcessFogOfWar - %s: %s(%x) - %s\n", operation,
			DXGetErrorString8A(result), result, DXGetErrorDescription8A(result));
	}
	struct MaskVertex
	{
		float x, y, z, rhw;
		DWORD color;
		float u, v;
	};
	void SetMaskStates()
	{
		g_D3D.SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
		g_D3D.SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
		g_D3D.SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
		g_D3D.SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
		g_D3D.SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
		g_D3D.SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
		g_D3D.SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
	}
	void SetAdditiveBlend()
	{
		g_D3D.SetRenderState(D3DRS_ALPHABLENDENABLE, 1);
		g_D3D.SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
		g_D3D.SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
		g_D3D.SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
	}
}

namespace JE
{
	BYTE SetGraphicsFogGray(DWORD& color, float intensity)
	{
		double value = intensity;
		if (value > 1.0) value = 1.0;
		else if (value < 0.0) value = 0.0;
		const BYTE channel = static_cast<BYTE>(static_cast<__int64>(value * 255.0 + 0.5));
		color = 0xFF000000u | (DWORD(channel) << 16) | (DWORD(channel) << 8) | channel;
		return channel;
	}

	void ProcessGraphicsFogOfWar(GraphicsLevelResources& level, const DWORD& exploredColor,
		const DWORD& visibleColor, const BYTE& revealExplored)
	{
		if (!level.automapMaskHandle || !level.visibleMaskHandle) return;
		Gfx_Texture* mask = g_GfxSystem->AutomapMaskTexture();
		IDirect3DTexture8* explored = mask->frameCount > 0 ? mask->frames[0].texture : NULL;
		mask = g_GfxSystem->VisibleMaskTexture();
		IDirect3DTexture8* visible = mask->frameCount > 0 ? mask->frames[0].texture : NULL;
		if (!level.fogOfWarTarget || !explored || !visible)
		{
			LogDebugString("Gfx_Engine::ProcessFogOfWar - trying to use an invalid texture!\n");
			return;
		}
		IDirect3DSurface8 *originalTarget = NULL, *originalDepth = NULL, *fogSurface = NULL;
		HRESULT result = g_D3D.device->GetRenderTarget(&originalTarget);
		if (FAILED(result)) { Failure("failed to get original render target", result); return; }
		result = g_D3D.device->GetDepthStencilSurface(&originalDepth);
		if (FAILED(result))
		{
			Failure("failed to get original depth/stencil", result);
			if (originalTarget) originalTarget->Release();
			return;
		}
		result = level.fogOfWarTarget->GetSurfaceLevel(0, &fogSurface);
		if (FAILED(result))
		{
			Failure("failed to get FOW render target", result);
			if (originalTarget) originalTarget->Release();
			if (originalDepth) originalDepth->Release();
			return;
		}
		const float right = float(double(level.maskWidth) - 1.0);
		const float bottom = float(double(level.maskHeight) - 1.0);
		const MaskVertex exploredQuad[4] = {
			{0,0,0.01f,1,exploredColor,0,0}, {right,0,0.01f,1,exploredColor,1,0},
			{0,bottom,0.01f,1,exploredColor,0,1}, {right,bottom,0.01f,1,exploredColor,1,1}};
		const MaskVertex visibleQuad[4] = {
			{0,0,0.01f,1,visibleColor,0,0}, {right,0,0.01f,1,visibleColor,1,0},
			{0,bottom,0.01f,1,visibleColor,0,1}, {right,bottom,0.01f,1,visibleColor,1,1}};
		result = g_D3D.device->SetRenderTarget(fogSurface, NULL);
		if (FAILED(result))
			LogDebugString("Gfx_Engine::ProcessFogOfWar - SetRenderTarget failed:  %s(%x) - %s\n",
				DXGetErrorString8A(result), result, DXGetErrorDescription8A(result));
		else
		{
			++g_D3D.statistics[6];
			result = g_D3D.device->BeginScene();
			if (FAILED(result)) Failure("BeginScene failed", result);
			else
			{
				if (revealExplored)
				{
					g_D3D.device->SetVertexShader(0x144);
					g_D3D.SetRenderState(D3DRS_ZENABLE, 0);
					g_D3D.SetRenderState(D3DRS_ALPHABLENDENABLE, 0);
					SetMaskStates();
					g_D3D.SetTexture(0, explored);
					result = g_D3D.device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, exploredQuad,
						D3DXGetFVFVertexSize(0x144));
					if (FAILED(result)) Failure("Draw Automap failed", result);
					SetAdditiveBlend();
				}
				else
				{
					g_D3D.device->Clear(0, NULL, D3DCLEAR_TARGET, exploredColor, 0.0f, 0);
					g_D3D.device->SetVertexShader(0x144);
					g_D3D.SetRenderState(D3DRS_ZENABLE, 0);
					SetAdditiveBlend();
					SetMaskStates();
				}
				g_D3D.SetTexture(0, visible);
				result = g_D3D.device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, visibleQuad,
					D3DXGetFVFVertexSize(0x144));
				if (FAILED(result)) Failure("Draw VisibleMask failed", result);
				g_D3D.SetRenderState(D3DRS_ALPHABLENDENABLE, 0);
				++g_D3D.statistics[7];
				g_D3D.device->EndScene();
			}
			g_D3D.device->SetRenderTarget(originalTarget, originalDepth);
		}
		if (fogSurface) fogSurface->Release();
		if (originalTarget) originalTarget->Release();
		if (originalDepth) originalDepth->Release();
	}
}
