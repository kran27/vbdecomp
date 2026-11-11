#include "PacketBuffer.hpp"
#include "PacketStorage.hpp"
#include "gui/Manager.hpp"
#include <d3dx8mesh.h>

// Recovered proprietary batch configuration; source spellings remain hypotheses.
// The real SDK provides D3DXGetFVFVertexSize. The actual mesh/material/instance
// classes provide lifetime and configuration; no library/runtime is reconstructed.

void Gfx_UIPacketBuffer::ConfigureQuads(DWORD quadCount, LONG vertexCount,
	DWORD textureHandle)
{
	DWORD output = 0;
	DWORD indexCount = quadCount * 6;
	vertices = 0;
	indices = 0;
	if (vertexCount != 0)
	{
		vertices = reinterpret_cast<JE::Gfx::UIVertex*>(
			GUI::g_Manager.storage->Allocate(static_cast<DWORD>(vertexCount) * 28));
		// Reload the actual Manager storage owner after the first allocation call.
		indices = reinterpret_cast<WORD*>(GUI::g_Manager.storage->Allocate(indexCount * 2));
		if (vertexCount > 0)
		{
			DWORD center = 1;
			do
			{
				// Wrap the base once per quad, not separately for each of its six stores.
				DWORD base = static_cast<WORD>(output);
				indices[base] = static_cast<WORD>(center - 1);
				indices[base + 1] = static_cast<WORD>(center);
				indices[base + 2] = static_cast<WORD>(center + 1);
				indices[base + 3] = static_cast<WORD>(center + 1);
				indices[base + 4] = static_cast<WORD>(center);
				indices[base + 5] = static_cast<WORD>(center + 2);
				center += 4;
				output += 6;
			}
			while (static_cast<LONG>(static_cast<WORD>(center - 1)) < vertexCount);
		}
	}
	mesh.value00 = 0;
	mesh.value04 = 0xFFFFFFFF;
	mesh.vertices = vertices;
	mesh.value10 = reinterpret_cast<DWORD>(indices); // Actual borrowed index pointer at +10.
	mesh.SetVertexDesc(0x144, D3DXGetFVFVertexSize(0x144), static_cast<DWORD>(vertexCount));
	mesh.SetIndexDesc(2, indexCount);
	primitive = mesh.SetPrimitiveCount(1);
	primitive->type = D3DPT_TRIANGLELIST;
	primitive->value24 = 1;
	primitive->value28 = 0;
	primitive->value2C = 0;
	primitive->value30 = quadCount * 2;
	Gfx_Material* newMaterial = mesh.materialSet.materials;
	material = newMaterial;
	newMaterial->style1 = 9;
	newMaterial->style2 = 9;
	material->textureHandles[0] = textureHandle;
	instance.SetMesh(&mesh);
	mesh.materialSet.RebuildGroups();
}

void Gfx_UIPacketBuffer::ConfigureTriangles(DWORD triangleCount, DWORD vertexCount,
	DWORD textureHandle, DWORD color)
{
	vertices = 0;
	if (vertexCount != 0)
	{
		vertices = reinterpret_cast<JE::Gfx::UIVertex*>(
			GUI::g_Manager.storage->Allocate(vertexCount * 28));
	}
	mesh.value00 = 0;
	mesh.value04 = 0xFFFFFFFF;
	mesh.vertices = vertices;
	// Actual triangle path leaves mesh+10/+20/+24 and batch.indices unchanged.
	mesh.SetVertexDesc(0x144, D3DXGetFVFVertexSize(0x144), vertexCount);
	primitive = mesh.SetPrimitiveCount(1);
	primitive->type = D3DPT_TRIANGLELIST;
	// No explicit write to primitive+24 on this path.
	primitive->value28 = 0;
	primitive->value2C = 0;
	primitive->value30 = triangleCount;
	Gfx_Material* newMaterial = mesh.materialSet.materials;
	material = newMaterial;
	newMaterial->style1 = 20;
	newMaterial->style2 = 20;
	Gfx_Material* modesMaterial = material;
	modesMaterial->mode28 = 1;
	modesMaterial->mode24 = 1;
	material->textureHandles[0] = textureHandle;
	instance.SetMesh(&mesh);
	if (color != 0xFF808080)
	{
		mesh.materialSet.SetColors(1, 1, color, 0);
	}
	mesh.materialSet.RebuildGroups();
}
