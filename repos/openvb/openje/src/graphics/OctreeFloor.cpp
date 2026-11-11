#include "OctreeRender.hpp"
#include "BufferManager.hpp"
#include "BufferRecord.hpp"
#include "Camera.hpp"
#include "D3DState.hpp"
#include "SystemInterface.hpp"
#include "diagnostics/DebugOutput.hpp"
#include <cstring>
#include <dxerr8.h>

void Octree2Gfx::RenderFloorCB()
{
	if (!floorEnabled) return;

	D3DXMATRIX mapProjection = g_D3D.identity;
	D3DXVECTOR3 minimum, maximum;
	g_GfxSystem->GetLevelAABB(&minimum.x, &maximum.x);
	const float width = maximum.x - minimum.x;
	const float height = maximum.z - minimum.z;
	UINT textureWidth, textureHeight;
	g_GfxSystem->GetProjectedShadowDimensions(&textureWidth, &textureHeight);
	D3DXMATRIX inverseView;
	g_GfxSystem->Camera()->GetInverseViewMatrix(inverseView);

	const double inverseWidth = 1.0 / double(width);
	mapProjection._11 = float(inverseWidth);
	mapProjection._22 = 0.0f;
	mapProjection._41 = float(0.5 / double(textureWidth) - inverseWidth * minimum.x);
	const double inverseHeight = 1.0 / double(height);
	mapProjection._32 = float(-1.0 * inverseHeight);
	mapProjection._33 = 0.0f;
	mapProjection._42 = float(inverseHeight * maximum.z + 0.5 / double(textureHeight));
	D3DXMATRIX product, textureTransform;
	D3DXMatrixMultiply(&product, &inverseView, &mapProjection);
	textureTransform = product;
	g_D3D.SetTransform(D3DTS_WORLD, &g_D3D.identity);
	g_D3D.SetTransform(D3DTS_TEXTURE0, &textureTransform);
	g_D3D.SetRenderState(D3DRS_SHADEMODE, D3DSHADE_FLAT);
	g_D3D.SetRenderState(D3DRS_LIGHTING, 0);
	g_D3D.SetRenderState(D3DRS_FOGENABLE, 0);
	g_D3D.SetRenderState(D3DRS_ALPHABLENDENABLE, 1);
	g_D3D.SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ZERO);
	g_D3D.SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCCOLOR);
	g_D3D.SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
	g_D3D.SetTexture(0, g_GfxSystem->ProjectedShadowRenderTexture());
	g_D3D.SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEPOSITION);
	g_D3D.SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);
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
	g_D3D.device->SetVertexShader(0x252);

	//Captured once before the loop, as original BufferManager+30.
	Gfx_StreamingIndexBuffer* stream = g_BufferManager->indexStream;
	for (DWORD group = 0; group < meshCount; ++group)
	{
		if (!floorGroups[group].visible) continue;
		Gfx_MeshData* mesh = &meshes[group];
		const DWORD handle = mesh->value04;
		Gfx_BufferRecord* record = g_BufferManager->FindStoredBuffer(handle);
		const DWORD vertexCount = mesh->vertexCount;
		if (!record)
		{
			JE::LogDebugString("Octree2Gfx::RenderFloorCB - Failed to get Static Mesh Data (handle: %d)\n", handle);
			continue;
		}
		record->Bind(0); //HRESULT ignored; binds existing stored VB/IB.
		for (DWORD index = 0; index < floorGroups[group].count; ++index)
		{
			OctreeFloorSegment& segment = floorGroups[group].segments[index];
			if (!segment.visible) continue;
			BYTE* destination;
			UINT base;
			HRESULT result = stream->Lock(segment.indexCount, &destination, &base, 0);
			if (FAILED(result))
			{
				JE::LogDebugString("Octree2Gfx::RenderFloorCB - Failed StreamingIB Lock (%x %s)\n",
					result, DXGetErrorDescription8A(result));
				continue;
			}
			//Always WORD source/copy, even if stream.indexStride differs.
			std::memcpy(destination, segment.indices, 2 * segment.indexCount);
			stream->buffer->Unlock(); //HRESULT ignored.
			g_D3D.device->SetIndices(stream->buffer, 0);
			const DWORD triangles = segment.triangleCount;
			++g_D3D.statistics[12];
			g_D3D.statistics[3] += triangles;
			result = g_D3D.device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, vertexCount, base, triangles);
			if (FAILED(result))
				JE::LogDebugString("Octree2Gfx::RenderFloorCB - DIP failed! (%x %s)\n",
					result, DXGetErrorDescription8A(result));
		}
	}
	g_D3D.SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0);
	g_D3D.SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, 0);
	g_D3D.SetTextureStageState(0, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP);
	g_D3D.SetTextureStageState(0, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP);
	g_D3D.SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD);
	if (g_GfxSystem->Lighting()) g_D3D.SetRenderState(D3DRS_LIGHTING, 1);
	if (g_GfxSystem->FogEnabled(NULL)) g_D3D.SetRenderState(D3DRS_FOGENABLE, 1);
	//No blanket state/transform restoration and no BeginScene/EndScene.
}

