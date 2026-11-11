#include "RenderManager.hpp"
#include "BufferManager.hpp"
#include "D3DState.hpp"
#include "Light.hpp"
#include "SystemInterface.hpp"
#include <cstring>

namespace
{
	struct DebugLightVertex
	{
		float x, y, z;
		DWORD color;
	};

	DWORD DebugLightColor(const D3DCOLORVALUE& value)
	{
		//491D56..DDF converts in red/alpha/green/blue order with the real
		//CRT ftol policy. Only RGB are masked; alpha's upper bits shift out.
		float channel = (value.r + 1.0f) * 0.5f;
		const DWORD red = DWORD(static_cast<__int64>(channel * 255.0f)) & 0xFF;
		const DWORD alpha = DWORD(static_cast<__int64>(value.a * 255.0f));
		channel = (value.g + 1.0f) * 0.5f;
		const DWORD green = DWORD(static_cast<__int64>(channel * 255.0f)) & 0xFF;
		channel = (value.b + 1.0f) * 0.5f;
		const DWORD blue = DWORD(static_cast<__int64>(channel * 255.0f)) & 0xFF;
		return (alpha << 24) | (red << 16) | (green << 8) | blue;
	}
}

void Gfx_RenderManager::RenderDebugLights()
{
	DWORD remaining = DWORD(debugLights.size());
	if (!remaining) return;
	Gfx_StreamingVertexBuffer* stream = g_BufferManager->vertexStream;
	BYTE* data = NULL;
	std::map<Gfx_Light*, DWORD>::iterator entry = debugLights.begin();
	if (g_GfxSystem->Lighting()) g_D3D.SetRenderState(D3DRS_LIGHTING, 0);
	g_D3D.SetRenderState(D3DRS_ZENABLE, 0);
	g_D3D.SetRenderState(D3DRS_ALPHABLENDENABLE, 1);
	g_D3D.SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	g_D3D.SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
	g_D3D.SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
	g_D3D.SetRenderState(D3DRS_POINTSPRITEENABLE, 1);
	g_D3D.SetRenderState(D3DRS_POINTSCALEENABLE, 1);
	g_D3D.SetRenderState(D3DRS_POINTSIZE, 0x41200000);
	g_D3D.SetRenderState(D3DRS_POINTSIZE_MIN, 0);
	g_D3D.SetRenderState(D3DRS_POINTSCALE_A, 0);
	g_D3D.SetRenderState(D3DRS_POINTSCALE_B, 0);
	g_D3D.SetRenderState(D3DRS_POINTSCALE_C, 0x3F800000);
	++g_D3D.statistics[9];
	g_D3D.device->SetStreamSource(0, stream->buffer, 16);
	g_D3D.device->SetVertexShader(D3DFVF_XYZ | D3DFVF_DIFFUSE);
	g_D3D.SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_DISABLE);
	g_D3D.SetTransform(D3DTS_WORLD, &g_D3D.identity);
	do
	{
		const UINT aligned = stream->bytePosition / 16 + (stream->bytePosition % 16 != 0);
		const UINT available = (stream->capacity - 16 * aligned) >> 4;
		const UINT count = remaining < available ? remaining : available;
		const BYTE discard = remaining > available && available < 5 ? 1 : 0;
		UINT first;
		if (SUCCEEDED(stream->Lock(count, 16, &data, &first, discard)))
		{
			DebugLightVertex* vertex = reinterpret_cast<DebugLightVertex*>(data);
			for (UINT index = 0; index < count; ++index)
			{
				Gfx_Light* light = entry->first;
				const D3DXVECTOR3 position(light->transform._41,
					light->transform._42, light->transform._43);
				const D3DCOLORVALUE diffuse = light->light.Diffuse;
				vertex->x = position.x;
				vertex->y = position.y;
				std::memcpy(&vertex->z, &position.z, sizeof(vertex->z));
				vertex->color = DebugLightColor(diffuse);
				++entry;
				++vertex;
			}
			stream->buffer->Unlock();
			++g_D3D.statistics[11];
			g_D3D.statistics[3] += count;
			g_D3D.device->DrawPrimitive(D3DPT_POINTLIST, first, count);
			remaining -= count;
		}
		//491E41 retries with unchanged remaining after a failed Lock.
	} while (remaining);
	if (g_GfxSystem->Lighting()) g_D3D.SetRenderState(D3DRS_LIGHTING, 1);
	g_D3D.SetRenderState(D3DRS_POINTSPRITEENABLE, 0);
	g_D3D.SetRenderState(D3DRS_POINTSCALEENABLE, 0);
	g_D3D.SetRenderState(D3DRS_ZENABLE, 1);
	g_D3D.SetRenderState(D3DRS_ALPHABLENDENABLE, 0);
}
