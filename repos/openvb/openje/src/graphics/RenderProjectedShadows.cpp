#include "RenderManager.hpp"
#include "BufferManager.hpp"
#include "D3DState.hpp"
#include "PacketData.hpp"
#include "SkinMeshPacket.hpp"
#include "Skinning.hpp"
#include "SystemInterface.hpp"
#include "VertexBuffer.hpp"
#include "diagnostics/DebugOutput.hpp"

namespace
{
	void DrawShadowVolumeTriangles(UINT first, UINT triangles)
	{
		++g_D3D.statistics[11];
		g_D3D.statistics[3] += triangles;
		g_D3D.device->DrawPrimitive(D3DPT_TRIANGLELIST, first, triangles);
	}
}

void Gfx_RenderManager::RenderShadowVolumes2()
{
	Gfx_StreamingVertexBuffer* stream = g_BufferManager->vertexStream;
	const UINT capacity = stream->capacity;
	const UINT aligned = stream->bytePosition / 12 + (stream->bytePosition % 12 != 0);
	UINT available = (capacity - 12 * aligned) / 12;
	const UINT vertexLimit = g_D3D.vertexLimit;
	if (available >= vertexLimit) available = vertexLimit;
	UINT maximum = capacity / 12;
	if (maximum >= vertexLimit) maximum = vertexLimit;
	UINT triangles = 0;
	UINT first = 0;
	BYTE* data = NULL;
	BYTE locked = 0;
	UINT written = 0;
	++g_D3D.statistics[9];
	g_D3D.device->SetStreamSource(0, stream->buffer, 12);
	g_D3D.device->SetVertexShader(D3DFVF_XYZ);
	g_D3D.SetTransform(D3DTS_WORLD, &g_D3D.identity);
	std::map<DWORD, Gfx_RenderGroup>::iterator group = staticGroups[0].begin();
	while (group != staticGroups[0].end())
	{
		std::list<Gfx_MeshInstance>::iterator entry = group->second.instances.begin();
		while (entry != group->second.instances.end())
		{
			Gfx_MeshInstance& instance = *entry;
			if (instance.flag1C && instance.skinning)
			{
				const UINT count = 6 * instance.skinning->value08;
				if (count > available)
				{
					if (locked) stream->buffer->Unlock();
					if (triangles) DrawShadowVolumeTriangles(first, triangles);
					if (FAILED(stream->Lock(maximum, 12, &data, &first, 0)))
					{
						locked = 0;
						JE::LogDebugString("Gfx_RenderManager::RenderShadowVolumes2 - Failed to lock/flush streaming VB for shadow volume!\n");
					}
					else locked = 1;
					triangles = 0;
					written = 0;
					available = maximum;
					//491835..844 advances past this volume after a flush.
					++entry;
					continue;
				}
				Gfx_PacketData* packet = dynamic_cast<Gfx_PacketData*>(
					reinterpret_cast<Gfx_BasePacketData*>(instance.value0C));
				if (packet)
				{
					const DWORD transformEnabled = packet->transformEnabled;
					const D3DXMATRIX* transform = packet->transform;
					if (!locked)
					{
						if (FAILED(stream->Lock(available, 12, &data, &first, 0)))
						{
							JE::LogDebugString("Gfx_RenderManager::RenderShadowVolumes2 - Failed Streaming Lock!\n");
							++entry;
							continue;
						}
						locked = 1;
					}
					instance.skinning->WriteShadowVertices(transformEnabled ? transform : NULL,
						reinterpret_cast<D3DXVECTOR3*>(data) + written);
					written += count;
					available -= count;
					triangles += 2 * instance.skinning->value08;
				}
			}
			++entry;
		}
		++group;
	}
	std::map<DWORD, std::list<Gfx_RenderSubmission16> >::iterator skin = skinSubmissions.begin();
	while (skin != skinSubmissions.end())
	{
		//4918D0..DA performs this actual lookup even though its result is unused.
		g_SkinMeshManager->Lookup(skin->first);
		std::list<Gfx_RenderSubmission16>::iterator entry = skin->second.begin();
		while (entry != skin->second.end())
		{
			Gfx_SkinMeshPkt* packet = entry->packet;
			if (packet->castsShadow)
			{
				const UINT count = 6 * packet->skinning->value08;
				if (count > available)
				{
					if (locked) stream->buffer->Unlock();
					if (triangles) DrawShadowVolumeTriangles(first, triangles);
					if (FAILED(stream->Lock(maximum, 12, &data, &first, 0)))
					{
						locked = 0;
						JE::LogDebugString("Gfx_RenderManager::RenderShadowVolumes2 - Failed to lock/flush streaming VB for skinned shadow volume!\n");
					}
					else locked = 1;
					written = 0;
					triangles = 0;
					available = maximum;
					++entry;
					continue;
				}
				if (!locked)
				{
					if (FAILED(stream->Lock(available, 12, &data, &first, 0)))
					{
						locked = 0;
						JE::LogDebugString("Gfx_RenderManager::RenderShadowVolumes2 - Failed Streaming Lock for skinned mesh!\n");
						++entry;
						continue;
					}
					locked = 1;
				}
				packet->skinning->WriteShadowVertices(&packet->transform,
					reinterpret_cast<D3DXVECTOR3*>(data) + written);
				written += count;
				available -= count;
				triangles += 2 * packet->skinning->value08;
			}
			++entry;
		}
		++skin;
	}
	if (locked) stream->buffer->Unlock();
	if (triangles) DrawShadowVolumeTriangles(first, triangles);
}

void Gfx_RenderManager::DrawShadowOverlay()
{
	Gfx_VertexBuffer* buffer = quadBuffer;
	const UINT stride = buffer->vertexStride;
	IDirect3DVertexBuffer8* deviceBuffer = buffer->buffer;
	++g_D3D.statistics[9];
	g_D3D.device->SetStreamSource(0, deviceBuffer, stride);
	//49017D reloads the wrapper after SetStreamSource.
	g_D3D.device->SetVertexShader(quadBuffer->fvf);
	++g_D3D.statistics[11];
	g_D3D.statistics[3] += 2;
	g_D3D.device->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
}

void Gfx_RenderManager::RenderProjectedShadows()
{
	g_D3D.SetRenderState(D3DRS_SHADEMODE, D3DSHADE_FLAT);
	g_D3D.SetRenderState(D3DRS_LIGHTING, 0);
	g_D3D.SetRenderState(D3DRS_FOGENABLE, 0);
	g_D3D.SetRenderState(D3DRS_ZWRITEENABLE, 0);
	g_D3D.SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
	g_D3D.SetRenderState(D3DRS_STENCILENABLE, 1);
	g_D3D.SetRenderState(D3DRS_STENCILFUNC, D3DCMP_ALWAYS);
	g_D3D.SetRenderState(D3DRS_STENCILZFAIL, D3DSTENCILOP_KEEP);
	g_D3D.SetRenderState(D3DRS_STENCILFAIL, D3DSTENCILOP_KEEP);
	g_D3D.SetRenderState(D3DRS_STENCILREF, 1);
	g_D3D.SetRenderState(D3DRS_STENCILMASK, 0xFFFFFFFF);
	g_D3D.SetRenderState(D3DRS_STENCILWRITEMASK, 0xFFFFFFFF);
	g_D3D.SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_INCR);
	g_D3D.SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	g_D3D.SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TFACTOR);
	g_D3D.SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG2);
	g_D3D.SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	g_D3D.SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_TFACTOR);
	g_D3D.SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG2);
	g_D3D.SetRenderState(D3DRS_TEXTUREFACTOR, shadowColor);
	if (!flag74) g_D3D.SetRenderState(D3DRS_COLORWRITEENABLE, 0);
	RenderShadowVolumes2();
	g_D3D.SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
	g_D3D.SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_DECR);
	RenderShadowVolumes2();
	g_D3D.SetRenderState(D3DRS_COLORWRITEENABLE, 0xF);
	g_D3D.SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
	g_D3D.SetRenderState(D3DRS_ALPHABLENDENABLE, 1);
	g_D3D.SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	g_D3D.SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	g_D3D.SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
	g_D3D.SetRenderState(D3DRS_STENCILFUNC, D3DCMP_LESSEQUAL);
	g_D3D.SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_KEEP);
	g_D3D.SetRenderState(D3DRS_ZFUNC, D3DCMP_ALWAYS);
	DrawShadowOverlay();
	g_D3D.SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
	g_D3D.SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD);
	g_D3D.SetRenderState(D3DRS_ZWRITEENABLE, 1);
	g_D3D.SetRenderState(D3DRS_STENCILENABLE, 0);
	if (g_GfxSystem->Lighting()) g_D3D.SetRenderState(D3DRS_LIGHTING, 1);
	if (g_GfxSystem->FogEnabled(NULL)) g_D3D.SetRenderState(D3DRS_FOGENABLE, 1);
}
