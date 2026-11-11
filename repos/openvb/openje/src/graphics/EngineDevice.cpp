#include "Engine.hpp"
#include "D3DState.hpp"
#include "BufferManager.hpp"
#include "TextureManager.hpp"
#include "SkinManager.hpp"
#include "Viewport.hpp"
#include "BuiltinVertexShaders.hpp"
#include "RenderStyleRegistry.hpp"
#include "GraphicsDeviceState.hpp"
#include "diagnostics/DebugOutput.hpp"

BYTE Gfx_Engine::CreateDevice(HWND owner, UINT adapter, UINT device, UINT mode,
	BYTE windowed, BYTE flag)
{
	if (window) return 1;
	if (FAILED(g_D3D.CreateDevice(owner, adapter, device, mode, windowed, flag))) return 0;
	g_BufferManager->InitializeStreams(g_D3D.currentDevice->guardBand == 0);
	Gfx_InitializeRenderStyles(value44);
	D3DVIEWPORT8 descriptor;
	descriptor.X = 0;
	descriptor.Y = 0;
	descriptor.Width = g_D3D.currentMode->display.Width;
	descriptor.Height = g_D3D.currentMode->display.Height;
	descriptor.MinZ = 0.0f;
	descriptor.MaxZ = 1.0f;
	defaultViewport = new Gfx_Viewport;
	if (!defaultViewport) JE::FatalError("Gfx_Engine::CreateDevice - Failed to create Default Viewport");
	defaultViewport->SetViewport(descriptor);
	defaultViewport->clearColor = 0xFF002080;
	int vertex;
	for (vertex = 3; vertex >= 0; --vertex) defaultViewport->vertices[vertex].diffuse = 0xFF002080;
	defaultViewport->clearFlags = 3;
	currentViewport = defaultViewport;
	postViewport = new Gfx_Viewport;
	if (!postViewport) JE::FatalError("Gfx_Engine::CreateDevice - Failed to create PostRenderViewport");
	postViewport->SetViewport(descriptor);
	postViewport->clearFlags = 2;
	postViewport->clearColor = 0;
	for (vertex = 3; vertex >= 0; --vertex) postViewport->vertices[vertex].diffuse = 0;
	window = owner;
	return PostDevice();
}

BYTE Gfx_Engine::PostDevice()
{
	if (g_D3D.currentMode->stencil) defaultViewport->clearFlags |= D3DCLEAR_STENCIL;
	else defaultViewport->clearFlags &= ~D3DCLEAR_STENCIL;
	Gfx_CreateBuiltinVertexShaders();
	statisticsFont.InitDeviceObjects(g_D3D.device);
	InitializeFog();
	const DWORD lights = MaximumActiveLights() >= 8 ? 8 : MaximumActiveLights();
	SetActiveLightLimit(lights);
	SetLighting(0);
	InitializeSystem();
	mapName.clear();
	return 1;
}

void Gfx_Engine::InitializeFog()
{
	SetLinearFiltering(1);
	SetMipFiltering(1);
	g_D3D.SetRenderState(D3DRS_AMBIENTMATERIALSOURCE, D3DMCS_COLOR1);
	g_D3D.SetRenderState(D3DRS_CLIPPING, g_D3D.currentDevice->guardBand ? 0 : 1);
	g_D3D.SetRenderState(D3DRS_LOCALVIEWER, 0);
	if (g_D3D.currentDevice->caps.RasterCaps & D3DPRASTERCAPS_FOGVERTEX)
	{
		g_D3D.SetRenderState(D3DRS_FOGVERTEXMODE, D3DFOG_LINEAR);
		if (g_D3D.currentDevice->caps.RasterCaps & D3DPRASTERCAPS_FOGRANGE)
			g_D3D.SetRenderState(D3DRS_RANGEFOGENABLE, 1);
		else
			JE::LogDebugString("Device does NOT support range-based fog.  Default to depth-based.");
	}
	SetAlphaTest(1);
	//5BDB16 tails into the real SDK CD3DFont RestoreDeviceObjects body.
	statisticsFont.RestoreDeviceObjects();
}

void Gfx_Engine::InitializeSystem()
{
	const D3DFORMAT colors[] = { D3DFMT_R5G6B5, D3DFMT_X1R5G5B5, D3DFMT_R8G8B8, D3DFMT_X8R8G8B8 };
	const D3DFORMAT depths[] = { D3DFMT_D16, D3DFMT_D15S1, D3DFMT_D24X8, D3DFMT_D24S8, D3DFMT_D32 };
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
	JE::FatalError("Gfx_Engine::InitializeSystem - failed to find a suitable render target format for automap!\n");
}

void Gfx_Engine::ReleaseVideoResources()
{
	for (UINT stage = 0; stage < g_D3D.currentDevice->caps.MaxTextureBlendStages; ++stage)
		g_D3D.SetTexture(stage, NULL);
	g_TextureManager->ReleaseVideoResources();
	g_BufferManager->ReleaseAll(0);
	skinManager->Visit();
	statisticsFont.InvalidateDeviceObjects();
	if (lostVideoCallback) lostVideoCallback();
	Gfx_NotifyRenderStylesLost();
	if (automapRenderTarget) { automapRenderTarget->Release(); automapRenderTarget = NULL; }
	if (automapDepthSurface) { automapDepthSurface->Release(); automapDepthSurface = NULL; }
	if (fogOfWarTarget) { fogOfWarTarget->Release(); fogOfWarTarget = NULL; }
	if (projectedShadowTarget) { projectedShadowTarget->Release(); projectedShadowTarget = NULL; }
	if (resource894) { resource894->Release(); resource894 = NULL; }
	if (resource898) { resource898->Release(); resource898 = NULL; }
	if (resource89C) { resource89C->Release(); resource89C = NULL; }
	if (resource8A0) { resource8A0->Release(); resource8A0 = NULL; }
}

void Gfx_Engine::RestoreVideoResources()
{
	CreateAutomapRenderTarget();
	CreateFogOfWarRenderTarget();
	CreateProjectedShadowRenderTarget();
	g_TextureManager->RestoreVideoResources();
	g_BufferManager->Restore();
	skinManager->Visit();
	InitializeFog();
	if (restoredVideoCallback) restoredVideoCallback();
	Gfx_NotifyRenderStylesReset();
	InitializeSystem();
}

void Gfx_Engine::SetVerticalSync(BYTE value)
{
	if (g_D3D.verticalSync == value) return;
	const BYTE windowed = g_D3D.requestedWindowed;
	g_D3D.verticalSync = value;
	if (!windowed)
	{
		ReleaseVideoResources();
		D3DPRESENT_PARAMETERS parameters = g_D3D.presentation;
		parameters.FullScreen_PresentationInterval = value ? D3DPRESENT_INTERVAL_ONE : D3DPRESENT_INTERVAL_IMMEDIATE;
		g_D3D.ResetDevice(&parameters);
		RestoreVideoResources();
	}
}

void Gfx_Engine::SetMode(DWORD mode)
{
	ReleaseVideoResources();
	g_D3D.SetMode(mode);
	RestoreVideoResources();
	const DWORD width = g_D3D.currentMode->display.Width;
	const DWORD height = g_D3D.currentMode->display.Height;
	{
		Gfx_Viewport* viewport = defaultViewport;
		const DWORD actualWidth = width ? width : viewport->viewport.Width;
		const DWORD actualHeight = height ? height : viewport->viewport.Height;
		D3DVIEWPORT8 descriptor = { 0, 0, actualWidth, actualHeight, 0.0f, 1.0f };
		viewport->SetViewport(descriptor);
	}
	if (g_D3D.currentMode->stencil) defaultViewport->clearFlags |= D3DCLEAR_STENCIL;
	else defaultViewport->clearFlags &= ~D3DCLEAR_STENCIL;
	{
		Gfx_Viewport* viewport = postViewport;
		const DWORD actualWidth = width ? width : viewport->viewport.Width;
		const DWORD actualHeight = height ? height : viewport->viewport.Height;
		D3DVIEWPORT8 descriptor = { 0, 0, actualWidth, actualHeight, 0.0f, 1.0f };
		viewport->SetViewport(descriptor);
	}
}

void Gfx_Engine::SetWindowMode(DWORD width, DWORD height, BYTE windowed, BYTE color32, DWORD refresh)
{
	DWORD mode = 0;
	if (!windowed && !g_D3D.FindMode(mode, g_D3D.currentAdapter,
		g_D3D.currentDeviceIndex, width, height, color32, refresh)) return;
	ReleaseVideoResources();
	if (windowed) g_D3D.SetWindowMode(width, height);
	else g_D3D.SetMode(mode);
	RestoreVideoResources();
	{
		Gfx_Viewport* viewport = defaultViewport;
		const DWORD actualWidth = width ? width : viewport->viewport.Width;
		const DWORD actualHeight = height ? height : viewport->viewport.Height;
		D3DVIEWPORT8 descriptor = { 0, 0, actualWidth, actualHeight, 0.0f, 1.0f };
		viewport->SetViewport(descriptor);
	}
	if (g_D3D.currentMode->stencil) defaultViewport->clearFlags |= D3DCLEAR_STENCIL;
	else defaultViewport->clearFlags &= ~D3DCLEAR_STENCIL;
	{
		Gfx_Viewport* viewport = postViewport;
		const DWORD actualWidth = width ? width : viewport->viewport.Width;
		const DWORD actualHeight = height ? height : viewport->viewport.Height;
		D3DVIEWPORT8 descriptor = { 0, 0, actualWidth, actualHeight, 0.0f, 1.0f };
		viewport->SetViewport(descriptor);
	}
}
