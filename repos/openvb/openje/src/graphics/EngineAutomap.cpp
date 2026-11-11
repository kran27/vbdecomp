#include "Engine.hpp"
#include "AutomapDimensions.hpp"
#include "Camera.hpp"
#include "D3DState.hpp"
#include "Texture.hpp"
#include "Viewport.hpp"
#include "diagnostics/DebugOutput.hpp"
#include <dxerr8.h>

namespace
{
	void AutomapFailure(const char* operation, HRESULT result)
	{
		JE::LogDebugString("Gfx_Engine::RenderAutomap - %s: %s(%x) - %s\n", operation,
			DXGetErrorString8A(result), result, DXGetErrorDescription8A(result));
	}

}

HRESULT Gfx_Viewport::SetViewport(UINT x, UINT y, UINT width, UINT height, float minimumZ, float maximumZ)
{
	if (!width) width = viewport.Width;
	if (!height) height = viewport.Height;
	D3DVIEWPORT8 descriptor;
	descriptor.X = x;
	descriptor.Y = y;
	descriptor.Width = width;
	descriptor.Height = height;
	descriptor.MinZ = minimumZ;
	descriptor.MaxZ = maximumZ;
	return SetViewport(descriptor);
}

void Gfx_Viewport::SetClearColor(BYTE red, BYTE green, BYTE blue, BYTE alpha)
{
	const D3DCOLOR color = D3DCOLOR_RGBA(red, green, blue, alpha);
	clearColor = color;
	vertices[3].diffuse = color;
	vertices[2].diffuse = color;
	vertices[1].diffuse = color;
	vertices[0].diffuse = color;
}

void Gfx_Camera::SetViewTransform(const D3DXMATRIX& matrix)
{
	const D3DXMATRIX snapshot(matrix);
	transform = snapshot;
	viewDirty = 1;
}

void Gfx_Engine::RenderAutomap()
{
	const float width = float(double(levelMaximum.x) - levelMinimum.x);
	const float height = float(double(levelMaximum.z) - levelMinimum.z);
	const float cameraY = float(double(levelMaximum.y) + 10.0);
	IDirect3DSurface8* originalTarget = NULL;
	IDirect3DSurface8* originalDepth = NULL;
	if (!automapRenderTarget || !automapDepthSurface)
	{
		JE::LogDebugString("Gfx_Engine::RenderAutomap - invalid Automap RenderTarget and DepthStencil\n");
		return;
	}
	HRESULT result = g_D3D.device->GetRenderTarget(&originalTarget);
	if (FAILED(result))
	{
		AutomapFailure("failed to get original render target", result);
		if (automapRenderTarget)
		{
			automapRenderTarget->Release();
			automapRenderTarget = NULL;
		}
		return;
	}
	result = g_D3D.device->GetDepthStencilSurface(&originalDepth);
	if (FAILED(result))
	{
		AutomapFailure("failed to get original depth/stencil", result);
		if (originalTarget) originalTarget->Release();
		if (automapRenderTarget)
		{
			automapRenderTarget->Release();
			automapRenderTarget = NULL;
		}
		return;
	}
	result = g_D3D.device->SetRenderTarget(automapRenderTarget, automapDepthSurface);
	if (FAILED(result))
	{
		AutomapFailure("SetRenderTarget failed", result);
	}
	else
	{
		Gfx_Viewport* viewport = DefaultViewport();
		Gfx_Camera* camera = viewport->activeCamera;
		const DWORD clearFlags = viewport->clearFlags;
		viewport->clearFlags = depthFormat == D3DFMT_D15S1 || depthFormat == D3DFMT_D24S8
			? D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL
			: D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER;
		const D3DVIEWPORT8 originalViewport = viewport->viewport;
		viewport->SetViewport(0, 0, automapWidth, automapHeight, 0.0f, 1.0f);
		const D3DCOLOR clearColor = viewport->clearColor;
		viewport->SetClearColor(0, 0, 0, 255);
		const float nearPlane = camera->nearPlane;
		const float farPlane = camera->farPlane;
		D3DXMATRIX originalProjection;
		camera->GetProjectionMatrix(originalProjection);
		const D3DXMATRIX originalTransform = camera->transform;
		D3DXMATRIX projection;
		D3DXMatrixOrthoLH(&projection, width, height, nearPlane, farPlane);
		camera->projectionMatrix = projection;
		D3DXMATRIX rotation, translation;
		D3DXMatrixRotationX(&rotation, 1.57079637050628662109375f);
		const float centerZ = float(double(height) * 0.5 + levelMinimum.z);
		const float centerX = float(double(width) * 0.5 + levelMinimum.x);
		D3DXMatrixTranslation(&translation, centerX, cameraY, centerZ);
		D3DXMatrixMultiply(&rotation, &rotation, &translation);
		camera->SetViewTransform(rotation);
		const BYTE originalStatistics = statistics;
		statistics = 0;
		const BYTE originalFogOfWar = FogOfWarEnabled(NULL);
		SetFogOfWarEnabled(0, NULL);
		RenderFrame(0, 0);
		statistics = originalStatistics;
		SetFogOfWarEnabled(originalFogOfWar, NULL);
		g_D3D.device->SetRenderTarget(originalTarget, originalDepth);
		viewport->SetViewport(originalViewport);
		viewport->SetClearColor(BYTE(clearColor >> 16), BYTE(clearColor >> 8), BYTE(clearColor), BYTE(clearColor >> 24));
		viewport->clearFlags = clearFlags;
		camera->projectionMatrix = originalProjection;
		camera->SetViewTransform(originalTransform);

		UINT textureWidth = automapWidth;
		UINT textureHeight = automapHeight;
		if (automapReady == 1)
		{
			textureWidth = Gfx_NextPowerOfTwo(textureWidth);
			textureHeight = Gfx_NextPowerOfTwo(textureHeight);
		}
		automapTexture = 0;
		result = CreateAndAddTexture(&automapTexture, textureWidth, textureHeight,
			1, colorFormat, D3DPOOL_MANAGED);
		if (FAILED(result))
		{
			AutomapFailure("failed to create blank texture", result);
		}
		else
		{
			Gfx_Texture* texture = LookupTexture(automapTexture);
			texture->name = "[AutomapTexture]";
			IDirect3DTexture8* deviceTexture = texture->frameCount > 0
				? texture->frames[0].texture : NULL;
			IDirect3DSurface8* outputSurface = NULL;
			//The original does not test this HRESULT before CopyRects.
			deviceTexture->GetSurfaceLevel(0, &outputSurface);
			RECT sourceRect;
			sourceRect.left = 0;
			sourceRect.top = 0;
			sourceRect.right = LONG(automapWidth);
			sourceRect.bottom = LONG(automapHeight);
			POINT destination;
			destination.x = LONG((textureWidth >> 1)
				- UINT(static_cast<__int64>(double(automapWidth) * 0.5)));
			destination.y = LONG((textureHeight >> 1)
				- UINT(static_cast<__int64>(double(automapHeight) * 0.5)));
			result = g_D3D.device->CopyRects(automapRenderTarget, &sourceRect, 1,
				outputSurface, &destination);
			if (FAILED(result))
			{
				AutomapFailure("failed to copy surface", result);
			}
			else if (!automapTextureName.empty())
			{
				result = D3DXSaveSurfaceToFileA(automapTextureName.c_str(), D3DXIFF_BMP,
					outputSurface, NULL, NULL);
				if (FAILED(result)) AutomapFailure("failed to save surface to file", result);
			}
			if (outputSurface) outputSurface->Release();
		}
	}
	if (originalTarget) originalTarget->Release();
	if (originalDepth) originalDepth->Release();
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
}
