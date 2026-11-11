#include "Engine.hpp"
#include "D3DState.hpp"
#include "Viewport.hpp"
#include "RenderManager.hpp"

DWORD Gfx_Engine::AdapterCount()
{
	return g_D3D.adapterCount;
}

DWORD Gfx_Engine::DeviceCount(UINT adapter)
{
	if (adapter < g_D3D.adapterCount)
		return g_D3D.adapters[adapter].deviceCount;
	return 0;
}

DWORD Gfx_Engine::ModeCount(UINT adapter, UINT device)
{
	if (adapter >= g_D3D.adapterCount ||
		device >= g_D3D.adapters[adapter].deviceCount)
	{
		return 0;
	}
	return g_D3D.adapters[adapter].devices[device].modeCount;
}

Gfx_D3DMode* Gfx_Engine::Modes(UINT adapter, UINT device)
{
	if (adapter >= g_D3D.adapterCount ||
		device >= g_D3D.adapters[adapter].deviceCount)
	{
		return NULL;
	}
	return g_D3D.adapters[adapter].devices[device].modes;
}

BYTE Gfx_Engine::VerticalSync()
{
	return g_D3D.verticalSync;
}

BYTE Gfx_Engine::State68()
{
	return value44;
}

bool Gfx_Engine::FindMode(DWORD& output, UINT adapter, UINT device, DWORD width,
	DWORD height, BYTE color32, DWORD refresh)
{
	return g_D3D.FindMode(output, adapter, device, width, height, color32, refresh);
}

Gfx_D3DMode* Gfx_Engine::CurrentMode()
{
	return g_D3D.currentMode;
}

BYTE Gfx_Engine::Windowed()
{
	return g_D3D.requestedWindowed;
}

void Gfx_Engine::SetColor(BYTE red, BYTE green, BYTE blue)
{
	Gfx_Viewport* viewport = defaultViewport;
	const DWORD color = 0xFF000000u | (DWORD(red) << 16) | (DWORD(green) << 8) | blue;
	viewport->clearColor = color;
	viewport->vertices[3].diffuse = color;
	viewport->vertices[2].diffuse = color;
	viewport->vertices[1].diffuse = color;
	viewport->vertices[0].diffuse = color;
}

void Gfx_Engine::SetViewportFlags(DWORD flags)
{
	defaultViewport->clearFlags = flags;
}

DWORD Gfx_Engine::ModeIndex()
{
	//5C00C0 reads70839C (+A4), not currentModeIndex at708390 (+98).
	return g_D3D.value0A4;
}

HWND Gfx_Engine::Window()
{
	return window;
}

DWORD Gfx_Engine::ActiveLightLimit()
{
	return activeLightLimit;
}

DWORD Gfx_Engine::MaximumActiveLights()
{
	return g_D3D.currentDevice->caps.MaxActiveLights;
}

DWORD Gfx_Engine::SetActiveLightLimit(DWORD limit)
{
	const DWORD maximum = g_D3D.currentDevice->caps.MaxActiveLights;
	if (limit > maximum)
	{
		activeLightLimit = maximum;
		return maximum;
	}
	activeLightLimit = limit;
	return limit;
}

DWORD Gfx_Engine::MaximumVertexBlendMatrices()
{
	return g_D3D.currentDevice->caps.MaxVertexBlendMatrices;
}

void Gfx_Engine::MaximumTextureSize(DWORD& width, DWORD& height)
{
	if (g_D3D.currentDevice)
	{
		width = g_D3D.currentDevice->caps.MaxTextureWidth;
		//The original reloads the current device after publishing width.
		height = g_D3D.currentDevice->caps.MaxTextureHeight;
	}
	else
	{
		width = 0;
		height = 0;
	}
}

void Gfx_Engine::SetRendererFlag56(BYTE value)
{
	defaultViewport->renderer->projectedShadows = value;
}

BYTE Gfx_Engine::RendererFlag56()
{
	return defaultViewport->renderer->projectedShadows;
}

void Gfx_Engine::SetCamera(Gfx_Camera* camera)
{
	Gfx_Viewport* viewport = defaultViewport;
	viewport->activeCamera = camera ? camera : &viewport->camera;
}

void Gfx_Engine::SetCameraController(Gfx_CamCtrl* control)
{
	defaultViewport->SetCameraController(control);
}

Gfx_Camera* Gfx_Engine::Camera()
{
	return defaultViewport->activeCamera;
}

Gfx_CamCtrl* Gfx_Engine::CameraController()
{
	return defaultViewport->cameraController;
}

void Gfx_Engine::SetPositiveParameter84(float value)
{
	if (value > 0.0f)
	{
		parameter84 = value;
	}
}

float Gfx_Engine::Parameter84()
{
	return parameter84;
}

void Gfx_Engine::SetPositiveParameter88(float value)
{
	if (value > 0.0f)
	{
		parameter88 = value;
	}
}

float Gfx_Engine::Parameter88()
{
	return parameter88;
}

UINT Gfx_Engine::ProjectedShadowPointCapacity()
{
	return shadowPointCapacity;
}
