#include "Engine.hpp"
#include "SurfaceRelease.hpp"
#include "AutomapDimensions.hpp"
#include "D3DState.hpp"
#include "Texture.hpp"
#include "TextureManager.hpp"
#include "diagnostics/DebugOutput.hpp"
#include <d3dx8.h>
#include <dxerr8.h>
#include <cstring>

ULONG __cdecl Gfx_ReleaseSurface(IDirect3DSurface8*& surface) //5024D0; return spelling inferred.
{
	ULONG result = 0;
	if (surface)
	{
		result = surface->Release();
		surface = NULL;
	}
	return result;
}

namespace
{
	void ReportTextureFailure(const char* message, HRESULT result)
	{
		const char* description = DXGetErrorDescription8A(result);
		const char* name = DXGetErrorString8A(result);
		JE::LogDebugString(message, name, result, description);
	}


	//51F6D0 traverses frame-pool keys, not each pool entry's frame-index list.
	void PrintPool(Gfx_TextureManager* manager,
		std::map<DWORD, std::list<int> >& pool, const char* heading)
	{
		if (!pool.empty())
		{
			JE::LogDebugString(heading);
			for (std::map<DWORD, std::list<int> >::iterator item = pool.begin();
				item != pool.end(); ++item)
			{
				Gfx_Texture* texture = manager->Lookup(item->first);
				const char* mipmap = texture->frameCount > 0 && texture->frames[0].mipCount > 1
					? "MIPMAP" : "";
				std::string name = texture->GetDiagnosticName();
				DWORD size = texture->frameCount > 0 ? texture->frames[0].firstMipSize : 0;
				DWORD bits = texture->frameCount > 0 ? texture->frames[0].bitsPerPixelWord : 0;
				UINT height = texture->frameCount > 0 ? texture->frames[0].height : 0;
				UINT width = texture->frameCount > 0 ? texture->frames[0].width : 0;
				JE::LogDebugString("(%08x) %s  (%d x %d %dbpp) Size: %d %s\n",
					texture->value20, name.c_str(), width, height, bits, size, mipmap);
			}
			JE::LogDebugString("\n\n");
		}
	}
}

//Actual lower owner502950. Surface releases follow each observed exit;
//GetFrontBuffer failure intentionally leaves the first surface unreleased.
HRESULT Gfx_D3D::ScreenCapture(const char* filename)
{
	HRESULT result = D3DERR_INVALIDCALL;
	if (!device || !filename)
		return result;

	D3DDISPLAYMODE display;
	result = device->GetDisplayMode(&display);
	if (FAILED(result))
	{
		JE::LogDebugString("Gfx_D3D::ScreenCapture - GetDisplayMode failed (%x)\n", result);
		return result;
	}

	IDirect3DSurface8* image;
	result = device->CreateImageSurface(display.Width, display.Height,
		D3DFMT_A8R8G8B8, &image);
	if (FAILED(result))
	{
		JE::LogDebugString("Gfx_D3D::ScreenCapture - CreateImageSurface failed (%x)\n", result);
		return result;
	}

	if (requestedWindowed)
	{
		result = device->GetFrontBuffer(image);
		if (FAILED(result))
		{
			JE::LogDebugString("Gfx_D3D::ScreenCapture - GetFrontBuffer failed (%x)\n", result);
			return result;
		}

		IDirect3DSurface8* cropped;
		result = device->CreateImageSurface(presentation.BackBufferWidth,
			presentation.BackBufferHeight, D3DFMT_A8R8G8B8, &cropped);
		if (FAILED(result))
		{
			JE::LogDebugString("Gfx_D3D::ScreenCapture - CreateImageSurface(2) failed (%x)\n",
				result);
			Gfx_ReleaseSurface(image);
			return result;
		}

		POINT point;
		point.x = 0;
		point.y = 0;
		ClientToScreen(window, &point);
		RECT rectangle;
		GetClientRect(window, &rectangle);
		rectangle.left += point.x;
		rectangle.bottom += point.y;
		rectangle.top += point.y;
		rectangle.right += point.x;
		result = D3DXLoadSurfaceFromSurface(cropped, NULL, NULL, image, NULL,
			&rectangle, D3DX_FILTER_NONE, 0);
		if (FAILED(result))
		{
			JE::LogDebugString("Gfx_D3D::ScreenCapture - D3DXLoadSurfaceFromSurface(2) failed (%x)\n",
				result);
			Gfx_ReleaseSurface(cropped);
			Gfx_ReleaseSurface(image);
			return result;
		}

		result = D3DXSaveSurfaceToFileA(filename, D3DXIFF_BMP, cropped, NULL, NULL);
		if (FAILED(result))
			JE::LogDebugString("Gfx_D3D::ScreenCapture - D3DXSaveSurfaceToFile failed (%x)\n", result);
		Gfx_ReleaseSurface(cropped);
		Gfx_ReleaseSurface(image);
		return result;
	}

	result = device->GetFrontBuffer(image);
	if (FAILED(result))
	{
		JE::LogDebugString("Gfx_D3D::ScreenCapture - GetFrontBuffer failed (%x)\n", result);
		return result;
	}
	result = D3DXSaveSurfaceToFileA(filename, D3DXIFF_BMP, image, NULL, NULL);
	if (FAILED(result))
		JE::LogDebugString("Gfx_D3D::ScreenCapture - D3DXSaveSurfaceToFile failed (%x)\n", result);
	Gfx_ReleaseSurface(image);
	return result;
}


HRESULT Gfx_Engine::ScreenCapture(const char* filename)
{
	return g_D3D.ScreenCapture(filename);
}

void Gfx_Engine::SetLevelAABB(const float* minimumXYZ, const float* maximumXYZ)
{
	g_D3D.device->ResourceManagerDiscardBytes(0);
	std::memcpy(&levelMinimum, minimumXYZ, 12);
	std::memcpy(&levelMaximum, maximumXYZ, 12);
	levelWidth = levelMaximum.x - levelMinimum.x;
	levelHeight = levelMaximum.z - levelMinimum.z;
	CalculateMaskDimensions(&levelMinimum.x, &levelMaximum.x, &maskWidth, &maskHeight);

	Gfx_D3DDeviceInfo* device = g_D3D.currentDevice;
	UINT maximumWidth = device->caps.MaxTextureWidth;
	UINT maximumHeight = device->caps.MaxTextureHeight;
	const UINT calculatedHeight = UINT(static_cast<__int64>(levelHeight * 4.0f));
	shadowHeight = calculatedHeight;
	shadowWidth = Gfx_NextPowerOfTwo(UINT(static_cast<__int64>(levelWidth * 4.0f)));
	while (shadowWidth > maximumWidth)
		shadowWidth >>= 1;
	if (calculatedHeight > maximumHeight)
	{
		do { shadowHeight >>= 1; } while (shadowHeight > maximumHeight);
	}
	UINT dimension = shadowWidth;
	if (dimension <= 1024) dimension = 1024;
	shadowWidth = dimension;
	dimension = shadowHeight;
	if (dimension <= 1024) dimension = 1024;
	shadowHeight = dimension;

	if (automapMaskHandle && DeleteTexture(automapMaskHandle))
		JE::LogDebugString("Gfx_Engine::SetLevelAABB - AutomapMask still in use!!!\n");
	HRESULT result = CreateAndAddTexture(&automapMaskHandle, maskWidth, maskHeight,
		1, colorFormat, D3DPOOL_MANAGED);
	if (SUCCEEDED(result))
	{
		Gfx_Texture* texture = LookupTexture(automapMaskHandle);
		if (texture) texture->name = std::string("[AutomapMask]");
	}
	else
	{
		JE::LogDebugString("Gfx_Engine::SetLevelAABB - failed to create AutomapMask texture: %s(%x) - %s\n",
			DXGetErrorString8A(result), result, DXGetErrorDescription8A(result));
	}

	if (visibleMaskHandle && DeleteTexture(visibleMaskHandle))
		JE::LogDebugString("Gfx_Engine::SetLevelAABB - VisibleMask still in use!!!\n");
	result = CreateAndAddTexture(&visibleMaskHandle, maskWidth, maskHeight,
		1, colorFormat, D3DPOOL_MANAGED);
	if (SUCCEEDED(result))
	{
		Gfx_Texture* texture = LookupTexture(visibleMaskHandle);
		if (texture) texture->name = std::string("[VisibleMask]");
	}
	else
	{
		JE::LogDebugString("Gfx_Engine::SetLevelAABB - failed to create VisibleMask texture: %s(%x) - %s\n",
			DXGetErrorString8A(result), result, DXGetErrorDescription8A(result));
	}
	CreateFogOfWarRenderTarget();
	CreateProjectedShadowRenderTarget();
}

void Gfx_Engine::GetLevelAABB(float* minimumXYZ, float* maximumXYZ)
{
	std::memcpy(minimumXYZ, &levelMinimum, 12);
	std::memcpy(maximumXYZ, &levelMaximum, 12);
}

void Gfx_Engine::CalculateMaskDimensions(const float* minimumXYZ, const float* maximumXYZ,
	UINT* width, UINT* height)
{
	//The original writes both outputs before loading the selected-device caps.
	//The explicit-cap helper would load those arguments before these writes.
	float heightValue = maximumXYZ[2] - minimumXYZ[2];
	*width = UINT(static_cast<__int64>(maximumXYZ[0] - minimumXYZ[0] + 0.5f));
	*height = UINT(static_cast<__int64>(heightValue + 0.5f));
	Gfx_D3DDeviceInfo* device = g_D3D.currentDevice;
	UINT maximumWidth = device->caps.MaxTextureWidth < 1024
		? device->caps.MaxTextureWidth : 1024;
	UINT maximumHeight = device->caps.MaxTextureHeight < 1024
		? device->caps.MaxTextureHeight : 1024;
	*width = Gfx_NextPowerOfTwo(*width);
	while (*width > maximumWidth)
		*width >>= 1;
	*height = Gfx_NextPowerOfTwo(*height);
	while (*height > maximumHeight)
		*height >>= 1;
	UINT dimension = *width;
	if (dimension <= 256) dimension = 256;
	*width = dimension;
	dimension = *height;
	if (dimension <= 256) dimension = 256;
	*height = dimension;
}

void Gfx_Engine::ConfigureAutomap(UINT width, UINT height, RECT* outputRect,
	const char* name, DWORD exactSize)
{
	automapWidth = width;
	automapHeight = height;
	if (name)
		automapTextureName.assign(name, std::strlen(name));
	else
		automapTextureName.erase(automapTextureName.begin(), automapTextureName.end());

	Gfx_D3DDeviceInfo* device = g_D3D.currentDevice;
	UINT maximumWidth = device->caps.MaxTextureWidth;
	UINT maximumHeight;
	if (BYTE(exactSize))
		maximumHeight = device->caps.MaxTextureHeight;
	else
	{
		if (maximumWidth >= 1024)
			maximumWidth = 1024;
		if (device->caps.MaxTextureHeight >= 1024)
			maximumHeight = 1024;
		else
			maximumHeight = device->caps.MaxTextureHeight;
	}
	if (automapWidth >= automapHeight)
	{
		if (automapWidth > maximumWidth)
		{
			UINT previousWidth = automapWidth;
			automapWidth = maximumWidth;
			automapHeight = UINT(static_cast<__int64>(float(automapHeight)
				* (float(maximumWidth) / float(previousWidth)) + 0.5f));
		}
	}
	else if (automapHeight > maximumHeight)
	{
		automapWidth = UINT(static_cast<__int64>((float(maximumHeight) / float(automapHeight))
			* float(automapWidth) + 0.5f));
		automapHeight = maximumHeight;
	}

	const UINT originalWidth = automapWidth;
	UINT paddedWidth = originalWidth;
	UINT paddedHeight = automapHeight;
	if (!BYTE(exactSize))
	{
		paddedWidth = Gfx_NextPowerOfTwo(paddedWidth);
		paddedHeight = Gfx_NextPowerOfTwo(paddedHeight);
	}
	else
	{
		automapReady = 0;
	}
	outputRect->left = LONG((paddedWidth >> 1)
		- UINT(static_cast<__int64>(float(originalWidth) * 0.5f)));
	outputRect->top = LONG((paddedHeight >> 1)
		- UINT(static_cast<__int64>(float(automapHeight) * 0.5f)));
	outputRect->right = LONG(DWORD(outputRect->left) + automapWidth);
	outputRect->bottom = LONG(DWORD(outputRect->top) + automapHeight);
	CreateAutomapRenderTarget();
}

void Gfx_Engine::CreateAutomapRenderTarget()
{
	if (automapRenderTarget)
	{
		automapRenderTarget->Release();
		automapRenderTarget = NULL;
	}
	if (automapDepthSurface)
	{
		automapDepthSurface->Release();
		automapDepthSurface = NULL;
	}
	if (automapWidth && automapHeight)
	{
		HRESULT result = g_D3D.device->CreateRenderTarget(automapWidth, automapHeight,
			colorFormat, D3DMULTISAMPLE_NONE, FALSE, &automapRenderTarget);
		if (SUCCEEDED(result))
		{
			result = g_D3D.device->CreateDepthStencilSurface(automapWidth, automapHeight,
				depthFormat, D3DMULTISAMPLE_NONE, &automapDepthSurface);
			if (SUCCEEDED(result))
			{
				automapRequested = 1;
				return;
			}
			JE::LogDebugString("Gfx_Engine::CreateAutomapRenderTarget - failed to create depth/stencil: %s(%x) - %s\n",
				DXGetErrorString8A(result), result, DXGetErrorDescription8A(result));
		}
		else
		{
			JE::LogDebugString("Gfx_Engine::CreateAutomapRenderTarget - failed to create render target: %s(%x) - %s\n",
				DXGetErrorString8A(result), result, DXGetErrorDescription8A(result));
		}
	}
}

void Gfx_Engine::CreateFogOfWarRenderTarget()
{
	if (fogOfWarTarget)
	{
		fogOfWarTarget->Release();
		fogOfWarTarget = NULL;
	}
	if (maskWidth && maskHeight)
	{
		HRESULT result = g_D3D.device->CreateTexture(maskWidth, maskHeight, 1,
			D3DUSAGE_RENDERTARGET, colorFormat, D3DPOOL_DEFAULT, &fogOfWarTarget);
		if (FAILED(result))
		{
			fogOfWar = 0;
			JE::LogDebugString("Gfx_Engine::CreateFogOfWarRenderTarget - CreateRenderTarget failed: %s(%x) - %s\n",
				DXGetErrorString8A(result), result, DXGetErrorDescription8A(result));
		}
	}
}

void Gfx_Engine::CreateProjectedShadowRenderTarget()
{
	if (projectedShadowTarget)
	{
		projectedShadowTarget->Release();
		projectedShadowTarget = NULL;
	}
	if (shadowWidth && shadowHeight)
	{
		HRESULT result = g_D3D.device->CreateTexture(shadowWidth, shadowHeight, 1,
			D3DUSAGE_RENDERTARGET, colorFormat, D3DPOOL_DEFAULT, &projectedShadowTarget);
		if (FAILED(result))
		{
			JE::LogDebugString("Gfx_Engine::CreateProjectedShadowRenderTarget - CreateRenderTarget failed: %s(%x) - %s\n",
				DXGetErrorString8A(result), result, DXGetErrorDescription8A(result));
		}
	}
}

DWORD Gfx_Engine::Value164()
{
	return automapTexture;
}

BYTE Gfx_Engine::AutomapReady()
{
	return automapRequested;
}

Gfx_Texture* Gfx_Engine::AutomapMaskTexture()
{
	return LookupTexture(automapMaskHandle);
}

Gfx_Texture* Gfx_Engine::VisibleMaskTexture()
{
	return LookupTexture(visibleMaskHandle);
}

IDirect3DTexture8* Gfx_Engine::FogOfWarRenderTexture()
{
	return fogOfWarTarget;
}

void Gfx_Engine::SetGrayColor204(float intensity)
{
	double value = intensity;
	if (value > 1.0f) value = 1.0f;
	else if (value < 0.0f) value = 0.0f;
	const BYTE channel = static_cast<BYTE>(static_cast<__int64>(value * 255.0f + 0.5f));
	exploredGray = (((DWORD(channel) | 0xFFFFFF00u) << 8) | channel) << 8 | channel;
}

void Gfx_Engine::SetGrayColor208(float intensity)
{
	double value = intensity;
	if (value > 1.0f) value = 1.0f;
	else if (value < 0.0f) value = 0.0f;
	const BYTE channel = static_cast<BYTE>(static_cast<__int64>(value * 255.0f + 0.5f));
	visibleGray = (((DWORD(channel) | 0xFFFFFF00u) << 8) | channel) << 8 | channel;
}

IDirect3DTexture8* Gfx_Engine::ProjectedShadowRenderTexture()
{
	return projectedShadowTarget;
}

void Gfx_Engine::GetProjectedShadowDimensions(UINT* width, UINT* height)
{
	*width = shadowWidth;
	*height = shadowHeight;
}

void Gfx_Engine::SetString248(const char* value)
{
	mapName.assign(value, std::strlen(value));
}

void Gfx_Engine::PrintTexturePools()
{
	Gfx_TextureManager* manager = textureManager;
	if (manager)
	{
		JE::LogDebugString("\nTexture Mgr\n=============\n");
		PrintPool(manager, manager->defaultFrames, "Default Pool\n------------\n");
		PrintPool(manager, manager->managedFrames, "Managed Pool\n------------\n");
		PrintPool(manager, manager->systemFrames, "System Pool\n-----------\n");
	}
}
