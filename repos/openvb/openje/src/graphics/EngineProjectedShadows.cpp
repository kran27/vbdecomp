#include "Engine.hpp"
#include "BufferManager.hpp"
#include "D3DState.hpp"
#include "Texture.hpp"
#include "diagnostics/DebugOutput.hpp"
#include <cstring>
#include <dxerr8.h>

namespace
{
	void ShadowFailure(const char* operation, HRESULT result)
	{
		JE::LogDebugString("Gfx_Engine::ProjectShadows - %s: %s(%x) - %s\n", operation,
			DXGetErrorString8A(result), result, DXGetErrorDescription8A(result));
	}

	struct ShadowVertex
	{
		float x, y, z, rhw, u, v;
	};
}

void Gfx_Engine::ProjectShadows()
{
	if (!projectedShadowTarget || !projectedShadowTexture)
	{
		shadowPointCount = 0;
		return;
	}
	IDirect3DSurface8* originalTarget = NULL;
	IDirect3DSurface8* originalDepth = NULL;
	IDirect3DSurface8* shadowSurface = NULL;
	HRESULT result = g_D3D.device->GetRenderTarget(&originalTarget);
	if (FAILED(result))
	{
		ShadowFailure("failed to get original render target", result);
		return;
	}
	result = g_D3D.device->GetDepthStencilSurface(&originalDepth);
	if (FAILED(result))
	{
		ShadowFailure("failed to get original depth/stencil", result);
		if (originalTarget) originalTarget->Release();
		return;
	}
	result = projectedShadowTarget->GetSurfaceLevel(0, &shadowSurface);
	if (FAILED(result))
	{
		ShadowFailure("failed to get ProjectedShadowTex surface", result);
		if (originalTarget) originalTarget->Release();
		if (originalDepth) originalDepth->Release();
		return;
	}
	result = g_D3D.device->SetRenderTarget(shadowSurface, NULL);
	if (FAILED(result))
	{
		JE::LogDebugString("Gfx_Engine::ProjectShadows - SetRenderTarget failed:  %s(%x) - %s\n",
			DXGetErrorString8A(result), result, DXGetErrorDescription8A(result));
	}
	else
	{
		++g_D3D.statistics[6];
		result = g_D3D.device->BeginScene();
		if (FAILED(result))
		{
			ShadowFailure("BeginScene failed", result);
		}
		else
		{
			g_D3D.device->Clear(0, NULL, D3DCLEAR_TARGET, 0, 1.0f, 0);
			if (shadowPointCount)
			{
				g_D3D.SetRenderState(D3DRS_ZENABLE, 0);
				g_D3D.SetRenderState(D3DRS_ALPHABLENDENABLE, 1);
				g_D3D.SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
				g_D3D.SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
				g_D3D.SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
				g_D3D.SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
				g_D3D.SetTextureStageState(0, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
				g_D3D.SetTextureStageState(0, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);
				g_D3D.SetTextureStageState(0, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
				g_D3D.SetTextureStageState(0, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
				g_D3D.SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
				g_D3D.SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
				g_D3D.SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
				g_D3D.SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
				g_D3D.SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
				g_D3D.SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
				g_D3D.SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
				g_D3D.SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
				Gfx_Texture* texture = LookupTexture(projectedShadowTexture);
				g_D3D.SetTexture(0, texture->frameCount > 0 ? texture->frames[0].texture : NULL);
				const UINT stride = D3DXGetFVFVertexSize(0x104);
				Gfx_StreamingVertexBuffer* stream = g_BufferManager->vertexStream;
				const UINT aligned = stream->bytePosition / stride
					+ (stream->bytePosition % stride != 0);
				UINT available = (stream->capacity - stride * aligned) / stride;
				const UINT vertexLimit = g_D3D.vertexLimit;
				if (available >= vertexLimit) available = vertexLimit;
				UINT maximum = stream->capacity / stride;
				if (maximum >= vertexLimit) maximum = vertexLimit;
				g_D3D.device->SetVertexShader(0x104);
				g_D3D.SetTransform(D3DTS_WORLD, &g_D3D.identity);
				stream->SetStreamSource(0, stride);
				BYTE* output = NULL;
				UINT first = 0, triangles = 0, written = 0;
				UINT remaining = 6 * shadowPointCount;
				BYTE locked = 0;
				for (UINT index = 0; index < shadowPointCount; ++index, remaining -= 6)
				{
					if (remaining >= available)
					{
						if (locked) stream->buffer->Unlock();
						if (triangles) g_D3D.DrawPrimitive(D3DPT_TRIANGLELIST, first, triangles);
						result = stream->Lock(maximum, stride, &output, &first, 0);
						locked = SUCCEEDED(result) ? 1 : 0;
						if (!locked)
							JE::LogDebugString("Gfx_Engine::ProjectShadows - Failed to relock streaming VB\n");
						triangles = 0;
						available = maximum;
						//5BF5CD..63E skips this point and retains written.
						continue;
					}
					if (!locked)
					{
						result = stream->Lock(available, stride, &output, &first, 0);
						locked = SUCCEEDED(result) ? 1 : 0;
						if (!locked)
						{
							JE::LogDebugString("Gfx_Engine::ProjectShadows - Failed to lock streaming VB\n");
							continue;
						}
					}
					Gfx_ShadowPoint& point = shadowPoints[index];
					point.x -= levelMinimum.x;
					point.z = levelMaximum.z - point.z;
					const float left = float((double(point.x) - point.radius * 1.0)
						/ levelWidth * shadowWidth);
					const float right = float((double(point.x) + point.radius)
						/ levelWidth * shadowWidth);
					const float top = float((double(point.z) - point.radius * 1.0)
						/ levelHeight * shadowHeight);
					const float bottom = float((double(point.z) + point.radius)
						/ levelHeight * shadowHeight);
					const ShadowVertex quad[6] = {
						{left, top, 0, 1, 0, 0}, {right, top, 0, 1, 1, 0},
						{left, bottom, 0, 1, 0, 1}, {left, bottom, 0, 1, 0, 1},
						{right, top, 0, 1, 1, 0}, {right, bottom, 0, 1, 1, 1}};
					std::memcpy(output + stride * written, quad, sizeof(quad));
					written += 6;
					triangles += 2;
				}
				if (locked) stream->buffer->Unlock();
				if (triangles) g_D3D.DrawPrimitive(D3DPT_TRIANGLELIST, first, triangles);
				g_D3D.SetRenderState(D3DRS_ALPHABLENDENABLE, 0);
				g_D3D.SetTextureStageState(0, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP);
				g_D3D.SetTextureStageState(0, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP);
			}
			++g_D3D.statistics[7];
			g_D3D.device->EndScene();
		}
		g_D3D.device->SetRenderTarget(originalTarget, originalDepth);
	}
	shadowPointCount = 0;
	if (shadowSurface) shadowSurface->Release();
	if (originalTarget) originalTarget->Release();
	if (originalDepth) originalDepth->Release();
}
