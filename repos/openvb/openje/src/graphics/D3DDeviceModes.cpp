#include "D3DState.hpp"

HRESULT Gfx_D3D::SetMode(DWORD mode)
{
	const BYTE sync = verticalSync;
	requestedWindowed = 0;
	Gfx_D3DMode* selected = &currentDevice->modes[mode];
	currentMode = selected;
	D3DPRESENT_PARAMETERS parameters;
	parameters.BackBufferWidth = selected->display.Width;
	parameters.BackBufferHeight = selected->display.Height;
	parameters.BackBufferFormat = selected->backBufferFormat;
	parameters.BackBufferCount = 1;
	parameters.MultiSampleType = D3DMULTISAMPLE_NONE;
	parameters.SwapEffect = D3DSWAPEFFECT_DISCARD;
	parameters.hDeviceWindow = presentation.hDeviceWindow;
	parameters.Windowed = FALSE;
	parameters.EnableAutoDepthStencil = TRUE;
	parameters.AutoDepthStencilFormat = selected->depthStencilFormat;
	parameters.Flags = 0;
	parameters.FullScreen_RefreshRateInHz = selected->display.RefreshRate;
	parameters.FullScreen_PresentationInterval = sync ? D3DPRESENT_INTERVAL_ONE : D3DPRESENT_INTERVAL_IMMEDIATE;
	//503D90 does not publish currentModeIndex, including after successful reset.
	return ResetDevice(&parameters);
}

HRESULT Gfx_D3D::SetWindowMode(DWORD width, DWORD height)
{
	Gfx_D3DAdapterInfo* adapter = &adapters[currentAdapter];
	requestedWindowed = 1;
	D3DPRESENT_PARAMETERS parameters;
	parameters.Windowed = TRUE;
	parameters.SwapEffect = D3DSWAPEFFECT_DISCARD;
	parameters.BackBufferFormat = adapter->desktop.Format;
	parameters.AutoDepthStencilFormat = adapter->windowDepthStencil;
	parameters.FullScreen_RefreshRateInHz = 0;
	parameters.FullScreen_PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;
	currentMode = &windowMode;
	currentMode->behavior = adapter->windowBehavior;
	Gfx_D3DMode* mode = currentMode;
	mode->display = adapter->desktop;
	mode->backBufferFormat = mode->display.Format;
	currentMode->display.Width = width;
	currentMode->display.Height = height;
	currentMode->display.RefreshRate = 0;
	const D3DFORMAT depth = adapters[currentAdapter].windowDepthStencil;
	if (depth == D3DFMT_D24S8 || depth == D3DFMT_D24X4S4 || depth == D3DFMT_D15S1)
		currentMode->stencil = 1;
	else
		currentMode->stencil = 0;
	currentMode->depthStencilFormat = depth;
	parameters.BackBufferWidth = width;
	parameters.BackBufferHeight = height;
	parameters.BackBufferCount = 1;
	parameters.EnableAutoDepthStencil = TRUE;
	parameters.MultiSampleType = D3DMULTISAMPLE_NONE;
	parameters.hDeviceWindow = presentation.hDeviceWindow;
	parameters.Flags = 0;
	return ResetDevice(&parameters);
}
