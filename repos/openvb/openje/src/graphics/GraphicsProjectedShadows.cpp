#include "GraphicsProjectedShadows.hpp"
#include "GraphicsLevelResources.hpp"
#include "BufferManager.hpp"
#include "D3DState.hpp"
#include "SystemInterface.hpp"
#include "Texture.hpp"
#include "Viewport.hpp"
#include "RenderManager.hpp"
#include "diagnostics/DebugOutput.hpp"
#include <cstring>
#include <dxerr8.h>

namespace
{
	void Failure(const char* operation, HRESULT result)
	{
		JE::LogDebugString("Gfx_Engine::ProjectShadows - %s: %s(%x) - %s\n", operation,
			DXGetErrorString8A(result), result, DXGetErrorDescription8A(result));
	}
	void Draw(UINT first, UINT triangles)
	{
		g_D3D.statistics[3] += triangles;
		++g_D3D.statistics[11];
		g_D3D.device->DrawPrimitive(D3DPT_TRIANGLELIST, first, triangles);
	}
	struct ShadowVertex { float x, y, z, rhw, u, v; };
}

namespace JE
{
	GraphicsProjectedShadows::GraphicsProjectedShadows()
		: capacity(50), count(0), points(new Point[50]), textureHandle(0)
	{
	}
	GraphicsProjectedShadows::~GraphicsProjectedShadows()
	{
		ReleaseTexture();
		delete[] points;
	}
	void GraphicsProjectedShadows::ReleaseTexture()
	{
		if (textureHandle) g_GfxSystem->DeleteTexture(textureHandle);
		textureHandle = 0;
	}
	void GraphicsProjectedShadows::SetCapacity(UINT size)
	{
		delete[] points;
		points = NULL;
		capacity = size;
		points = new Point[size];
		count = 0;
	}
	void GraphicsProjectedShadows::SetTexture(DWORD handle)
	{
		if (!handle)
			LogDebugString("Gfx_Engine::SetProjectedShadowTexture - Setting a NULL texture!\n");
		else if (g_GfxSystem->LookupTexture(handle)) textureHandle = handle;
		else LogDebugString("Gfx_Engine::SetProjectedShadowTexture - Unable to find texture!\n");
	}
	void GraphicsProjectedShadows::SetEnabled(BYTE& flag, Gfx_Viewport& viewport, BYTE value)
	{
		if (value && !textureHandle)
		{
			LogDebugString("Gfx_Engine::EnableProjectedShadow - ProjectedShadowTex and/or ShadowTex not available.\n");
			return;
		}
		flag = value;
		viewport.renderer->flag49 = value;
	}
	void GraphicsProjectedShadows::AddPoint(BYTE enabled, const float* point)
	{
		if (!enabled) return;
		if (count >= capacity)
			LogDebugString("Gfx_Engine::AddProjectedShadowPoint - Need larger buffer!!! (current size: %d)\n", capacity);
		else
		{
			std::memcpy(points + count, point, 12);
			++count;
		}
	}
	void GraphicsProjectedShadows::Render(GraphicsLevelResources& level)
	{
		if (!level.projectedShadowTarget || !textureHandle) { count = 0; return; }
		IDirect3DSurface8 *originalTarget = NULL, *originalDepth = NULL, *shadowSurface = NULL;
		HRESULT result = g_D3D.device->GetRenderTarget(&originalTarget);
		if (FAILED(result)) { Failure("failed to get original render target", result); return; }
		result = g_D3D.device->GetDepthStencilSurface(&originalDepth);
		if (FAILED(result))
		{
			Failure("failed to get original depth/stencil", result);
			if (originalTarget) originalTarget->Release();
			return;
		}
		result = level.projectedShadowTarget->GetSurfaceLevel(0, &shadowSurface);
		if (FAILED(result))
		{
			Failure("failed to get ProjectedShadowTex surface", result);
			if (originalTarget) originalTarget->Release();
			if (originalDepth) originalDepth->Release();
			return;
		}
		result = g_D3D.device->SetRenderTarget(shadowSurface, NULL);
		if (FAILED(result))
			LogDebugString("Gfx_Engine::ProjectShadows - SetRenderTarget failed:  %s(%x) - %s\n",
				DXGetErrorString8A(result), result, DXGetErrorDescription8A(result));
		else
		{
			++g_D3D.statistics[6];
			result = g_D3D.device->BeginScene();
			if (FAILED(result)) Failure("BeginScene failed", result);
			else
			{
				g_D3D.device->Clear(0, NULL, D3DCLEAR_TARGET, 0, 1.0f, 0);
				if (count)
				{
					g_D3D.SetRenderState(D3DRS_ZENABLE, 0);
					g_D3D.SetRenderState(D3DRS_ALPHABLENDENABLE, 1);
					g_D3D.SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
					g_D3D.SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
					g_D3D.SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
					g_D3D.SetTextureStageState(0, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
					g_D3D.SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
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
					Gfx_Texture* texture = g_GfxSystem->LookupTexture(textureHandle);
					g_D3D.SetTexture(0, texture->frameCount > 0 ? texture->frames[0].texture : NULL);
					const UINT stride = D3DXGetFVFVertexSize(0x104);
					Gfx_StreamingVertexBuffer* stream = g_BufferManager->vertexStream;
					const UINT aligned = stream->bytePosition / stride + (stream->bytePosition % stride != 0);
					UINT available = (stream->capacity - stride * aligned) / stride;
					if (available >= g_D3D.vertexLimit) available = g_D3D.vertexLimit;
					UINT maximum = stream->capacity / stride;
					if (maximum >= g_D3D.vertexLimit) maximum = g_D3D.vertexLimit;
					g_D3D.device->SetVertexShader(0x104);
					g_D3D.SetTransform(D3DTS_WORLD, &g_D3D.identity);
					++g_D3D.statistics[9];
					g_D3D.device->SetStreamSource(0, stream->buffer, stride);
					BYTE* output = NULL;
					UINT first = 0, triangles = 0, written = 0, remaining = 6 * count;
					BYTE locked = 0;
					for (UINT index = 0; index < count; ++index, remaining -= 6)
					{
						if (remaining >= available)
						{
							if (locked) stream->buffer->Unlock();
							if (triangles) Draw(first, triangles);
							result = stream->Lock(maximum, stride, &output, &first, 0);
							locked = SUCCEEDED(result) ? 1 : 0;
							if (!locked) LogDebugString("Gfx_Engine::ProjectShadows - Failed to relock streaming VB\n");
							triangles = 0;
							available = maximum;
							//Original5BF5CD..63E skips this point and retains written.
							continue;
						}
						if (!locked)
						{
							result = stream->Lock(available, stride, &output, &first, 0);
							locked = SUCCEEDED(result) ? 1 : 0;
							if (!locked)
							{
								LogDebugString("Gfx_Engine::ProjectShadows - Failed to lock streaming VB\n");
								continue;
							}
						}
						Point& point = points[index];
						point.x -= level.minimum.x;
						point.z = level.maximum.z - point.z;
						const float left = float((double(point.x) - point.radius * 1.0) / level.width * level.shadowWidth);
						const float right = float((double(point.x) + point.radius) / level.width * level.shadowWidth);
						const float top = float((double(point.z) - point.radius * 1.0) / level.height * level.shadowHeight);
						const float bottom = float((double(point.z) + point.radius) / level.height * level.shadowHeight);
						const ShadowVertex quad[6] = {
							{left,top,0,1,0,0}, {right,top,0,1,1,0}, {left,bottom,0,1,0,1},
							{left,bottom,0,1,0,1}, {right,top,0,1,1,0}, {right,bottom,0,1,1,1}};
						std::memcpy(output + stride * written, quad, sizeof(quad));
						written += 6;
						triangles += 2;
					}
					if (locked) stream->buffer->Unlock();
					if (triangles) Draw(first, triangles);
					g_D3D.SetRenderState(D3DRS_ALPHABLENDENABLE, 0);
					g_D3D.SetTextureStageState(0, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP);
					g_D3D.SetTextureStageState(0, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP);
				}
				++g_D3D.statistics[7];
				g_D3D.device->EndScene();
			}
			g_D3D.device->SetRenderTarget(originalTarget, originalDepth);
		}
		count = 0;
		if (shadowSurface) shadowSurface->Release();
		if (originalTarget) originalTarget->Release();
		if (originalDepth) originalDepth->Release();
	}
}
