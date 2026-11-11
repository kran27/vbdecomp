#include "D3DState.hpp"
#include "../diagnostics/DebugOutput.hpp"

HRESULT Gfx_D3D::CreateDevice(HWND owner, UINT adapter, UINT deviceIndex, UINT modeIndex,
	BYTE windowed, BYTE multithreaded)
{
	if (!direct3D)
	{
		return D3DERR_INVALIDCALL;
	}
	Gfx_D3DAdapterInfo* selected = &adapters[adapter];
	currentAdapter = adapter;
	currentDeviceIndex = deviceIndex;
	window = owner;
	currentDevice = &selected->devices[deviceIndex];
	currentModeIndex = modeIndex;
	currentMode = &currentDevice->modes[modeIndex];
	requestedWindowed = windowed;
	if (windowed && currentDevice->windowed)
	{
		RECT rectangle;
		GetClientRect(owner, &rectangle);
		presentation.BackBufferWidth = rectangle.right;
		presentation.BackBufferHeight = rectangle.bottom;
		presentation.BackBufferFormat = selected->desktop.Format;
		presentation.Windowed = TRUE;
		presentation.FullScreen_RefreshRateInHz = 0;
		presentation.FullScreen_PresentationInterval = 0;
		presentation.AutoDepthStencilFormat = selected->windowDepthStencil;
		currentMode = &windowMode;
		currentMode->behavior = selected->windowBehavior;
		Gfx_D3DMode* mode = currentMode;
		mode->display.Width = selected->desktop.Width;
		mode->display.Height = selected->desktop.Height;
		mode->display.RefreshRate = selected->desktop.RefreshRate;
		mode->display.Format = selected->desktop.Format;
		mode->backBufferFormat = mode->display.Format;
		currentMode->display.Width = rectangle.right;
		currentMode->display.Height = rectangle.bottom;
		currentMode->display.RefreshRate = 0;
		const D3DFORMAT depth = selected->windowDepthStencil;
		currentMode->stencil = depth == D3DFMT_D24S8 || depth == D3DFMT_D24X4S4 ||
			depth == D3DFMT_D15S1;
		currentMode->depthStencilFormat = depth;
	}
	else
	{
		Gfx_D3DMode* mode = currentMode;
		presentation.BackBufferWidth = mode->display.Width;
		// The original retains this import across both metrics calls.
		int (WINAPI* const metrics)(int) = GetSystemMetrics;
		presentation.BackBufferHeight = mode->display.Height;
		presentation.BackBufferFormat = mode->backBufferFormat;
		presentation.Windowed = FALSE;
		presentation.FullScreen_RefreshRateInHz = mode->display.RefreshRate;
		presentation.FullScreen_PresentationInterval = verticalSync ?
			D3DPRESENT_INTERVAL_ONE : D3DPRESENT_INTERVAL_IMMEDIATE;
		presentation.AutoDepthStencilFormat = mode->depthStencilFormat;
		const int height = metrics(SM_CYSCREEN);
		const int width = metrics(SM_CXSCREEN);
		MoveWindow(owner, 0, 0, width, height, TRUE);
	}
	presentation.BackBufferCount = 1;
	presentation.SwapEffect = D3DSWAPEFFECT_DISCARD;
	presentation.EnableAutoDepthStencil = TRUE;
	presentation.MultiSampleType = D3DMULTISAMPLE_NONE;
	presentation.hDeviceWindow = owner;
	presentation.Flags = 0;
	const HRESULT created = direct3D->CreateDevice(currentAdapter, currentDevice->type, owner,
		currentMode->behavior | (multithreaded ? D3DCREATE_MULTITHREADED : 0),
		&presentation, &device);
	if (FAILED(created))
	{
		JE::FatalError("Failed to create D3DDevice (%x)\n", created);
	}
	JE::LogDebugString("Created D3DDevice");
	const HRESULT stackResult = D3DXCreateMatrixStack(0, &matrixStack);
	if (FAILED(stackResult))
	{
		JE::LogDebugString("Gfx_D3D::CreateDevice - Failed to create matrix stack");
	}
	GetFreeTextureMemory();
	textureStageCount = currentDevice->caps.MaxTextureBlendStages;
	textureStates = new Gfx_D3DTextureState[textureStageCount];
	defaultTextureStates = new Gfx_D3DTextureState[textureStageCount];
	InitializeDefaultRenderStates();
	InitializeDefaultTextureStates();
	// Original0x503BC8 additionally reads caps.MaxVertexIndex without using it.
	// That read/fault prefix is an unresolved source/codegen gap; see device-review.
	vertexLimit = 0xFFFF;
	value0AC = 101;
	value0A4 = 2;
	ApplyCachedStates(1);
	ready = 1;
	return stackResult;
}
