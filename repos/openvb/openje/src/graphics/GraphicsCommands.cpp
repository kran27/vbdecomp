#include <winsock2.h>
#include "Graphics.hpp"
#include "SystemInterface.hpp"
#include "D3DState.hpp"
#include "OctreeRender.hpp"
#include "MapMaskManager.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>

//Published original70C0AC; the sole definition remains the startup window owner.
namespace F3 { extern HWND g_window; }

DWORD Graphics::CycleShadowMode()
{
	shadowMode = (shadowMode + 1u) % 3u;
	SetShadowMode(shadowMode);
	return shadowMode;
}

void Graphics::SetLinearFiltering(BYTE value) //581620.
{
	linearFiltering = value;
	g_GfxSystem->SetLinearFiltering(value);
}

void Graphics::SetWireframe(BYTE value) //581660.
{
	wireframe = value;
	g_GfxSystem->SetWireframe(value);
}

void Graphics::SetTransparency(BYTE value) //581680.
{
	transparency = value;
	g_GfxSystem->SetAlphaTest(value);
}

void Graphics::SetMipFiltering(BYTE value) //581020.
{
	flag57 = value;
	backend->SetMipFiltering(value);
}

void Graphics::ToggleStatistics() //581640.
{
	statistics = !statistics;
	g_GfxSystem->SetFlag2188(statistics);
}

void Graphics::ToggleFogOfWar() //581110.
{
	backend->SetFogOfWarEnabled(!backend->FogOfWarEnabled(NULL), NULL);
}

BYTE Graphics::GetFogOfWarEnabled() //581140.
{
	return backend->FogOfWarEnabled(NULL);
}

BYTE Graphics::GetFogEnabled() //581040.
{
	return backend->FogEnabled(NULL);
}

void Graphics::SetLighting(BYTE value)
{
	lighting = value;
	g_GfxSystem->SetLighting(value);
}

void Graphics::SetStartupMovies(BYTE value) //581010.
{
	startupMovies = value;
}

void Graphics::SetFogOfWarEnabled(BYTE enabled) //5810F0.
{
	backend->SetFogOfWarEnabled(enabled, NULL);
}

void Graphics::SetBuildingVisible(LONG building, BYTE visible) //581460.
{
	if (renderer) renderer->SetMaterialState(building, visible);
}

BYTE Graphics::GetBuildingVisible(LONG building) //581470.
{
	if (renderer) return renderer->GetMaterialState(building);
	return 1;
}

void Graphics::SetDebugLights(BYTE value)
{
	flag5A = value;
	backend->SetDebugLightsEnabled(value);
}

void Graphics::SetDebugOctreeLights(BYTE value)
{
	flag5B = value;
	renderer->value80 = value;
}

void Graphics::PrintTexturePools() //581390.
{
	if (backend) backend->PrintTexturePools();
}

void Graphics::ToggleHeightFog()
{
	if (backend->HeightFogEnabled(NULL)) backend->EnableHeightFog(0, NULL);
	else backend->EnableHeightFog(1, NULL);
}

void Graphics::QueueExploredChange(DWORD mapID, LONG column, LONG row, BYTE value)
{
	if (maskManager) maskManager->QueueExploredChange(mapID, column, row, value);
}

void Graphics::QueueVisibleChange(DWORD entityID, DWORD mapID, LONG column, LONG row, BYTE value)
{
	if (maskManager) maskManager->QueueVisibleChange(entityID, mapID, column, row, value);
}

void Graphics::ClearMapMasks()
{
	if (maskManager) maskManager->Clear();
}

Gfx_Light* Graphics::GetStaticLights(DWORD* count)
{
	if (renderer)
	{
		*count = renderer->lightCount;
		return renderer->lights;
	}
	return NULL;
}

void Graphics::GetVRAMStatistics(DWORD* output)
{
	if (backend)
	{
		DWORD statistics[17];
		backend->GetVRAMStatistics(statistics);
		std::memcpy(output, statistics, sizeof(statistics));
	}
	else std::memset(output, 0, 68);
}

BYTE Graphics::GetVerticalSync()
{
	if (backend) vsync = backend->VerticalSync();
	return vsync;
}

void Graphics::SetVerticalSync(BYTE value)
{
	vsync = value;
	if (backend) backend->SetVerticalSync(value);
}

DWORD Graphics::GetModeCount()
{
	DWORD count = 0;
	if (backend) count = backend->ModeCount(adapter, device);
	return count;
}

Gfx_D3DMode* Graphics::GetModes()
{
	if (backend) return backend->Modes(adapter, device);
	return NULL;
}

Gfx_D3DMode* Graphics::GetCurrentMode()
{
	if (backend) return backend->CurrentMode();
	return NULL;
}

const char* Graphics::GetAdapterDescription()
{
	if (backend) return g_D3D.adapters[adapter].identifier.Description;
	return NULL;
}

BOOL Graphics::ToggleFullscreen()
{
	BYTE wasFullscreen = fullscreen;
	DWORD selectedWidth = width;
	DWORD selectedHeight = height;
	BOOL result;
	if (wasFullscreen)
	{
		RECT rectangle = {0, 0, selectedWidth, selectedHeight};
		g_GfxSystem->SetWindowMode(selectedWidth, selectedHeight, 1, 0, 0);
		AdjustWindowRect(&rectangle, 0x10C80000u, FALSE);
		LONG outerHeight = rectangle.bottom - rectangle.top;
		LONG outerWidth = rectangle.right - rectangle.left;
		LONG y = (rectangle.top - rectangle.bottom + GetSystemMetrics(SM_CYSCREEN)) / 2;
		LONG x = (rectangle.left - rectangle.right + GetSystemMetrics(SM_CXSCREEN)) / 2;
		result = SetWindowPos(F3::g_window, NULL, x, y, outerWidth, outerHeight, SWP_SHOWWINDOW);
	}
	else
	{
		g_GfxSystem->SetWindowMode(selectedWidth, selectedHeight, 0, color32, 0);
		result = SetWindowPos(F3::g_window, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
	}
	fullscreen = !wasFullscreen;
	return result;
}

void Graphics::GetScreenDimensions(LONG& width, LONG& height)
{
	if (g_GfxSystem->Windowed())
	{
		RECT rectangle;
		GetClientRect(F3::g_window, &rectangle);
		width = rectangle.right - rectangle.left;
		height = rectangle.bottom - rectangle.top;
	}
	else
	{
		Gfx_D3DMode* mode = g_GfxSystem->CurrentMode();
		width = mode->display.Width;
		height = mode->display.Height;
	}
}

BYTE Graphics::SelectDisplayMode(DWORD index)
{
	BYTE changed = 0;
	if (backend && index < GetModeCount())
	{
		displayMode = index;
		backend->SetMode(index);
		changed = 1;
		Gfx_D3DMode* selected = backend->CurrentMode();
		width = selected->display.Width;
		height = selected->display.Height;
		refresh = selected->display.RefreshRate;
		D3DFORMAT format = selected->backBufferFormat;
		if (format != D3DFMT_X8R8G8B8 && format != D3DFMT_A8R8G8B8)
		{
			color32 = 0;
			fullscreen = 1;
			return 1;
		}
		color32 = 1;
		fullscreen = 1;
	}
	return changed;
}

void Graphics::CaptureScreen()
{
	char filename[16];
	char number[8];
	WIN32_FIND_DATAA found;
	std::strcpy(filename, "Scrn????.bmp");
	int ordinal = 0;
	HANDLE handle = FindFirstFileA(filename, &found);
	if (handle != INVALID_HANDLE_VALUE)
	{
		while (FindNextFileA(handle, &found)) {}
		char* digits = std::strrchr(found.cFileName, 'n') + 1;
		int length = 0;
		while (*digits != '.') number[length++] = *digits++;
		number[length] = 0;
		ordinal = std::atol(number) + 1;
		FindClose(handle);
	}
	std::sprintf(filename + 4, "%04d.bmp", ordinal);
	g_GfxSystem->ScreenCapture(filename);
}
