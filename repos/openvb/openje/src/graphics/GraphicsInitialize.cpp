#include "Graphics.hpp"
#include "GraphicsServices.hpp"
#include "SystemInterface.hpp"
#include "D3DState.hpp"
#include "OctreeRender.hpp"
#include "gui/Manager.hpp"
#include "resources/ResourceMemoryStream.hpp"
#include "diagnostics/DebugOutput.hpp"
#include <dxerr8.h>
#include <string>

void Graphics::SetShadowMode(DWORD requested)
{
	//580F80 uses the member backend, reloading it after each callback. The
	//renderer is also read only after those callbacks; no captured projection.
	switch (requested)
	{
	case 0:
		backend->SetRendererFlag56(0);
		backend->EnableProjectedShadow(0);
		if (renderer) renderer->floorEnabled = 0;
		break;
	case 2:
		backend->SetRendererFlag56(1);
		backend->EnableProjectedShadow(0);
		if (renderer) renderer->floorEnabled = 0;
		break;
	default:
		backend->SetRendererFlag56(0);
		backend->EnableProjectedShadow(1);
		if (renderer) renderer->floorEnabled = 1;
		break;
	}
	shadowMode = requested;
}

void Graphics::SetFogEnabled(BYTE enabled)
{
	backend->SetFogEnabled(enabled, NULL); //581050; actual default viewport selector.
}

void Graphics::SetFogColor(DWORD color)
{
	//581070 passes the address of the full stack argument, not a grayscale BYTE.
	backend->SetFogColor(&color, NULL);
}

void Graphics::SetFogRange(float start, float end)
{
	backend->SetFogRange(start, end, NULL); //581090; raw float arguments, RET8.
}

void Graphics::Initialize(DWORD)
{
	//581810's one stack argument is unused. The actual window is published70C0AC.
	if (initialized) Cleanup();
	const BYTE requestedFullscreen = fullscreen;
	const DWORD requestedWidth = width;
	const DWORD requestedHeight = height;
	const DWORD requestedRefresh = refresh;
	const BYTE requestedColor32 = color32;
	initialized = 1;
	const DWORD adapters = g_GfxSystem->AdapterCount();
	if (adapter >= adapters) adapter = 0;
	const DWORD devices = g_GfxSystem->DeviceCount(adapter);
	if (device >= devices) device = 0;
	g_GfxSystem->ModeCount(adapter, device); //Both original unused queries remain.
	g_GfxSystem->Modes(adapter, device);
	if (!g_GfxSystem->FindMode(displayMode, adapter, device, requestedWidth,
		requestedHeight, requestedColor32, requestedRefresh))
		JE::FatalError("Cannot find requested mode.\n");
	Gfx_D3DMode* modes = g_GfxSystem->Modes(adapter, device);
	const Gfx_D3DMode& selected = modes[displayMode];
	if (!selected.stencil)
	{
		JE::LogDebugString(
			"@@@ Selected Displaymode does NOT support stencil buffers: %d x %d x %dbpp DisplayFormat = %d DepthStencil = %d\n",
			selected.display.Width, selected.display.Height,
			selected.backBufferFormat == D3DFMT_X8R8G8B8 ||
			selected.backBufferFormat == D3DFMT_A8R8G8B8 ? 32 : 16,
			selected.backBufferFormat, selected.depthStencilFormat);
	}
	if (!g_GfxSystem->CreateDevice(JE::GraphicsServices::Window(), adapter, device, displayMode,
		requestedFullscreen == 0, 0)) JE::FatalError("CreateDevice failed.\n");
	g_GfxSystem->SetColor(clearRed, clearGreen, clearBlue);
	g_GfxSystem->SetFlag2188(statistics);
	fullscreen = requestedFullscreen;
	width = requestedWidth;
	height = requestedHeight;
	refresh = requestedRefresh;
	const D3DFORMAT format = modes[displayMode].backBufferFormat;
	color32 = format == D3DFMT_X8R8G8B8 || format == D3DFMT_A8R8G8B8;
	g_GfxSystem->SetLighting(lighting);
	g_GfxSystem->SetLinearFiltering(linearFiltering);
	SetFogEnabled(0);
	SetFogColor(0xFF808080u);
	SetFogRange(250.0f, 500.0f);
	g_GfxSystem->SetWireframe(wireframe);
	g_GfxSystem->SetAlphaTest(transparency);
	::GUI::g_Manager.SetResolution(width, height);
	::GUI::g_Manager.SetStringCallback(JE::GraphicsServices::ResolveString);
	g_GfxSystem->SetFogOfWarEnabled(1, NULL);

	JE::Resources::ResourceMemoryStream* stream =
		JE::Resources::OpenResourceMemoryStream("ProjectedShadow.tga", 0);
	if (stream)
	{
		DWORD handle; //No added initialization; actual texture policy writes the output.
		const HRESULT result = g_GfxSystem->LoadTextureFromBuffer(&handle,
			std::string("ProjectedShadow.tga"), stream->begin, UINT(stream->end - stream->begin),
			D3DFMT_UNKNOWN, D3DPOOL_MANAGED, D3DX_DEFAULT, D3DX_FILTER_NONE, D3DX_FILTER_BOX, 0);
		if (FAILED(result))
			JE::LogDebugString(
				"Graphics::Initialize - Failed to load \"ProjectedShadow.tga\" from buffer (%x %s)\n",
				result, DXGetErrorDescription8A(result));
		JE::Resources::CloseResourceMemoryStream(stream);
		g_GfxSystem->SetProjectedShadowTexture(handle);
	}
	else
		JE::LogDebugString(
			"Graphics::Initialize - Cannot load \"ProjectedShadow.tga\" for projected shadow\n");
	SetShadowMode(shadowMode);
	stream = JE::Resources::OpenResourceMemoryStream("SilhouetteEdge.tga", 0);
	if (stream)
	{
		DWORD handle;
		const HRESULT result = g_GfxSystem->LoadTextureFromBuffer(&handle,
			std::string("SilhouetteEdge.tga"), stream->begin, UINT(stream->end - stream->begin),
			D3DFMT_UNKNOWN, D3DPOOL_MANAGED, D3DX_DEFAULT, D3DX_FILTER_NONE, D3DX_FILTER_BOX, 0);
		if (FAILED(result))
			JE::LogDebugString(
				"Graphics::Initialize - Failed to load \"SilhouetteEdge.tga\" from buffer (%x %s)\n",
				result, DXGetErrorDescription8A(result));
		JE::Resources::CloseResourceMemoryStream(stream);
		g_GfxSystem->SetSilhouetteEdgeTexture(handle);
	}
	else
		JE::LogDebugString(
			"Graphics::Initialize - Cannot load \"SilhouetteEdge.tga\" for Silhouette rendering\n");
}
