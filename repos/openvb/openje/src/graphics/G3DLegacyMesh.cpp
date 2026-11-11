#include "G3DMeshExport.hpp"
#include "SystemInterface.hpp"
#include <string.h>
#include <stdexcept>
#include <new>

void Gfx_G3D_Mesh::PackVertices(DWORD fvf)
{
	vertices = new BYTE[D3DXGetFVFVertexSize(fvf) * vertexCount];
	if (!vertices) return;
	DWORD cursor = 0;
	for (DWORD index = 0; index < vertexCount; ++index)
	{
		switch (fvf & 0xE)
		{
		case D3DFVF_XYZ:
			memcpy(vertices + cursor, &positions[index], 12);
			cursor += 12;
			break;
		case D3DFVF_XYZB1:
			memcpy(vertices + cursor, &positions[index], 12);
			memcpy(vertices + cursor + 12, &boneWeights[0][index].weight, 4);
			cursor += 16;
			break;
		case D3DFVF_XYZB2:
			memcpy(vertices + cursor, &positions[index], 12);
			memcpy(vertices + cursor + 12, &boneWeights[0][index].weight, 4);
			memcpy(vertices + cursor + 16, &boneWeights[1][index].weight, 4);
			cursor += 16; // Actual523F51 also advances16 for XYZB2, not20.
			break;
		}
		if (fvf & D3DFVF_NORMAL)
		{
			memcpy(vertices + cursor, &normals[index], 12);
			cursor += 12;
		}
		if (fvf & D3DFVF_DIFFUSE)
		{
			const D3DXVECTOR4& color = colors[index];
			DWORD value = D3DCOLOR_ARGB(static_cast<LONG>(color.w * 255.0f),
				static_cast<LONG>(color.z * 255.0f), static_cast<LONG>(color.y * 255.0f),
				static_cast<LONG>(color.x * 255.0f));
			memcpy(vertices + cursor, &value, 4);
			cursor += 4;
		}
		DWORD textures = (fvf >> 8) & 15;
		if (textures >= 1 && !(fvf & 0x30000))
		{
			memcpy(vertices + cursor, &textureCoordinates[index], 8);
			cursor += 8;
		}
		if (textures >= 2 && !(fvf & 0xC0000))
		{
			memcpy(vertices + cursor, &lightmapCoordinates[index], 8);
			cursor += 8;
		}
	}
	//523E90 never writes vertexStride; a reused converter retains its old value.
}

void Gfx_G3D_Mesh::BuildIndices()
{
	typedef void* IndexBuffer;
	primitiveCounts = new DWORD[groupCount];
	indexCounts = new DWORD[groupCount];
	indices = new IndexBuffer[groupCount];
	const DWORD stride = g_GfxSystem->ModeIndex();
	for (DWORD group = 0; group < groupCount; ++group)
	{
		indexCounts[group] = groups[group].sourceIndexCount;
		indices[group] = new BYTE[stride * groups[group].sourceIndexCount];
		if (stride == 2)
		{
			WORD* output = static_cast<WORD*>(indices[group]);
			for (DWORD index = 0; index < groups[group].sourceIndexCount; ++index)
				output[index] = static_cast<WORD>(groups[group].sourceIndices[index]);
		}
		else if (stride == 4)
		{
			DWORD* output = static_cast<DWORD*>(indices[group]);
			for (DWORD index = 0; index < groups[group].sourceIndexCount; ++index)
				output[index] = groups[group].sourceIndices[index];
		}
		if (types[group] == D3DPT_TRIANGLELIST)
			primitiveCounts[group] = indexCounts[group] / 3;
	}
}
