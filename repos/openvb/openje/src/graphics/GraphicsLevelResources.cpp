#include "GraphicsLevelResources.hpp"
#include "AutomapDimensions.hpp"
#include "D3DState.hpp"
#include "SystemInterface.hpp"
#include "Texture.hpp"
#include "diagnostics/DebugOutput.hpp"
#include <dxerr8.h>
#include <stdexcept>

namespace
{
	void TextureFailure(const char* operation, HRESULT result)
	{
		JE::LogDebugString("Gfx_Engine::%s: %s(%x) - %s\n", operation,
			DXGetErrorString8A(result), result, DXGetErrorDescription8A(result));
	}
}

namespace JE
{
	GraphicsLevelResources::GraphicsLevelResources(BYTE& enabled) : minimum(0,0,0), maximum(0,0,0), width(0), height(0),
		maskWidth(0), maskHeight(0), colorFormat(D3DFMT_UNKNOWN), depthFormat(D3DFMT_UNKNOWN),
		automapMaskHandle(0), visibleMaskHandle(0), fogOfWarTarget(NULL), shadowWidth(0), shadowHeight(0),
		projectedShadowTarget(NULL), fogOfWarEnabled(enabled)
	{
	}
	GraphicsLevelResources::~GraphicsLevelResources()
	{
		Release();
	}
	void GraphicsLevelResources::InitializeFormats()
	{
		if (!g_D3D.direct3D || !g_D3D.currentDevice)
			throw std::logic_error("GraphicsLevelResources5BDCB0: actual selected D3D device required");
		const D3DFORMAT colors[] = {D3DFMT_R5G6B5, D3DFMT_X1R5G5B5, D3DFMT_R8G8B8, D3DFMT_X8R8G8B8};
		const D3DFORMAT depths[] = {D3DFMT_D16, D3DFMT_D15S1, D3DFMT_D24X8, D3DFMT_D24S8, D3DFMT_D32};
		for (UINT color = 0; color < sizeof(colors) / sizeof(colors[0]); ++color)
		{
			if (FAILED(g_D3D.direct3D->CheckDeviceFormat(g_D3D.currentAdapter, g_D3D.currentDevice->type,
				g_D3D.presentation.BackBufferFormat, D3DUSAGE_RENDERTARGET, D3DRTYPE_TEXTURE, colors[color]))) continue;
			for (UINT depth = 0; depth < sizeof(depths) / sizeof(depths[0]); ++depth)
				if (SUCCEEDED(g_D3D.direct3D->CheckDepthStencilMatch(g_D3D.currentAdapter,
					g_D3D.currentDevice->type, g_D3D.presentation.BackBufferFormat, colors[color], depths[depth])))
				{
					colorFormat = colors[color];
					depthFormat = depths[depth];
					return;
				}
		}
		FatalError("Gfx_Engine::InitializeSystem - failed to find a suitable render target format for automap!\n");
	}
	void GraphicsLevelResources::SetLevelAABB(const float* low, const float* high)
	{
		if (!g_D3D.device || !g_D3D.currentDevice || !g_GfxSystem)
			throw std::logic_error("GraphicsLevelResources5C1600: actual D3D/graphics owners required");
		g_D3D.device->ResourceManagerDiscardBytes(0);
		minimum = D3DXVECTOR3(low[0], low[1], low[2]);
		maximum = D3DXVECTOR3(high[0], high[1], high[2]);
		width = float(double(maximum.x) - minimum.x);
		height = float(double(maximum.z) - minimum.z);
		g_GfxSystem->CalculateMaskDimensions(&minimum.x, &maximum.x, &maskWidth, &maskHeight);
		UINT maximumWidth = g_D3D.currentDevice->caps.MaxTextureWidth;
		UINT maximumHeight = g_D3D.currentDevice->caps.MaxTextureHeight;
		shadowHeight = UINT(static_cast<__int64>(double(height) * 4.0));
		shadowWidth = Gfx_NextPowerOfTwo(UINT(static_cast<__int64>(double(width) * 4.0)));
		while (shadowWidth > maximumWidth) shadowWidth >>= 1;
		while (shadowHeight > maximumHeight) shadowHeight >>= 1;
		if (shadowWidth <= 1024) shadowWidth = 1024;
		if (shadowHeight <= 1024) shadowHeight = 1024;
		if (automapMaskHandle && g_GfxSystem->DeleteTexture(automapMaskHandle))
			LogDebugString("Gfx_Engine::SetLevelAABB - AutomapMask still in use!!!\n");
		HRESULT result = g_GfxSystem->CreateAndAddTexture(&automapMaskHandle,
			maskWidth, maskHeight, 1, colorFormat, D3DPOOL_MANAGED);
		if (FAILED(result)) TextureFailure("SetLevelAABB - failed to create AutomapMask texture", result);
		else
		{
			Gfx_Texture* texture = g_GfxSystem->LookupTexture(automapMaskHandle);
			if (texture) texture->name = "[AutomapMask]";
		}
		if (visibleMaskHandle && g_GfxSystem->DeleteTexture(visibleMaskHandle))
			LogDebugString("Gfx_Engine::SetLevelAABB - VisibleMask still in use!!!\n");
		result = g_GfxSystem->CreateAndAddTexture(&visibleMaskHandle,
			maskWidth, maskHeight, 1, colorFormat, D3DPOOL_MANAGED);
		if (FAILED(result)) TextureFailure("SetLevelAABB - failed to create VisibleMask texture", result);
		else
		{
			Gfx_Texture* texture = g_GfxSystem->LookupTexture(visibleMaskHandle);
			if (texture) texture->name = "[VisibleMask]";
		}
		CreateFogOfWarRenderTarget();
		CreateProjectedShadowRenderTarget();
	}
	void GraphicsLevelResources::CreateFogOfWarRenderTarget()
	{
		if (fogOfWarTarget) { fogOfWarTarget->Release(); fogOfWarTarget = NULL; }
		if (maskWidth && maskHeight)
		{
			if (!g_D3D.device) throw std::logic_error("GraphicsLevelResources5BE34D: actual D3D device required");
			HRESULT result = g_D3D.device->CreateTexture(maskWidth, maskHeight, 1,
				D3DUSAGE_RENDERTARGET, colorFormat, D3DPOOL_DEFAULT, &fogOfWarTarget);
			if (FAILED(result))
			{
				fogOfWarEnabled = 0;
				TextureFailure("CreateFogOfWarRenderTarget - CreateRenderTarget failed", result);
			}
		}
	}
	void GraphicsLevelResources::CreateProjectedShadowRenderTarget()
	{
		if (projectedShadowTarget) { projectedShadowTarget->Release(); projectedShadowTarget = NULL; }
		if (shadowWidth && shadowHeight)
		{
			if (!g_D3D.device) throw std::logic_error("GraphicsLevelResources5BEB0D: actual D3D device required");
			HRESULT result = g_D3D.device->CreateTexture(shadowWidth, shadowHeight, 1,
				D3DUSAGE_RENDERTARGET, colorFormat, D3DPOOL_DEFAULT, &projectedShadowTarget);
			if (FAILED(result)) TextureFailure("CreateProjectedShadowRenderTarget - CreateRenderTarget failed", result);
		}
	}
	Gfx_Texture* GraphicsLevelResources::AutomapMaskTexture() const
	{
		return g_GfxSystem->LookupTexture(automapMaskHandle);
	}
	Gfx_Texture* GraphicsLevelResources::VisibleMaskTexture() const
	{
		return g_GfxSystem->LookupTexture(visibleMaskHandle);
	}
	void GraphicsLevelResources::Release()
	{
		if (fogOfWarTarget) { fogOfWarTarget->Release(); fogOfWarTarget = NULL; }
		if (projectedShadowTarget) { projectedShadowTarget->Release(); projectedShadowTarget = NULL; }
		if (automapMaskHandle) { g_GfxSystem->DeleteTexture(automapMaskHandle); automapMaskHandle = 0; }
		if (visibleMaskHandle) { g_GfxSystem->DeleteTexture(visibleMaskHandle); visibleMaskHandle = 0; }
	}
}
