#include "ModelMesh.hpp"
#include "BufferManager.hpp"
#include "D3DState.hpp"
#include "PacketData.hpp"
#include "SystemInterface.hpp"
#include <string.h>

Gfx_ModelMesh::Gfx_ModelMesh(DWORD value)
	: value24(value)
{
	flag28 = 0;
	flag29 = 0;
	ownedVertices = NULL;
	ownedIndices = NULL;
}

Gfx_ModelMesh::~Gfx_ModelMesh()
{
	g_BufferManager->ReleaseStoredHandle(mesh.value04);
	const DWORD materialCount = mesh.materialSet.count;
	for (DWORD index = 0; index < materialCount; ++index)
	{
		Gfx_Material& material = mesh.materialSet.materials[index];
		for (DWORD texture = 0; texture < 4; ++texture)
		{
			if (texture < 4 && material.textureHandles[texture])
			{
				g_GfxSystem->DeleteTexture(material.textureHandles[texture]);
			}
		}
	}
	if (ownedVertices) { ::operator delete[](ownedVertices); ownedVertices = NULL; }
	if (ownedIndices) { ::operator delete[](ownedIndices); ownedIndices = NULL; }
	// Actual member destruction supplies41F200 and41FBD0 using real C++ owners.
}

void Gfx_ModelMesh::Initialize(BYTE stored, BYTE shadow)
{
	if (flag29) return;
	flag28 = stored;
	mesh.materialSet.RebuildGroups();
	if (shadow) mesh.BuildShadowGeometry();
	if (flag28) g_BufferManager->CreateStoredBuffers(&mesh);
	else mesh.value04 = DWORD(-1);
	flag29 = 1;
}

BYTE Gfx_ModelMesh::SetVertexData(DWORD fvf, DWORD stride, DWORD count, const void* vertices)
{
	Gfx_PacketData temporary; // Original50E760 constructs/destroys this unused local.
	ownedVertices = ::operator new[](count * stride);
	if (!ownedVertices) return 0;
	memcpy(ownedVertices, vertices, count * stride);
	mesh.vertices = ownedVertices;
	mesh.SetVertexDesc(fvf, stride, count);
	return 1;
}

BYTE Gfx_ModelMesh::SetPrimitives(DWORD count, const D3DPRIMITIVETYPE* types,
	const std::string* names, const DWORD* indexCounts, const DWORD* primitiveCounts,
	const void* const* indices)
{
	BYTE result = 0;
	DWORD total = 0;
	DWORD stride = g_D3D.value0A4; // Original70839C index element byte width.
	for (DWORD index = 0; index < count; ++index) total += indexCounts[index];
	ownedIndices = ::operator new[](stride * total);
	if (!ownedIndices) return 0;
	Gfx_Primitive* primitives = mesh.SetPrimitiveCount(count);
	if (primitives)
	{
		DWORD indexOffset = 0, byteOffset = 0;
		for (DWORD index = 0; index < count; ++index)
		{
			DWORD bytes = stride * indexCounts[index];
			memcpy(static_cast<BYTE*>(ownedIndices) + byteOffset, indices[index], bytes);
			Gfx_Primitive& primitive = primitives[index];
			primitive.type = types[index];
			primitive.value24 = 1;
			primitive.value28 = 0;
			primitive.value2C = indexOffset;
			primitive.value30 = primitiveCounts[index];
			if (names) primitive.name = names[index];
			primitive.value00 = index;
			indexOffset += indexCounts[index];
			byteOffset += bytes;
		}
		mesh.value10 = reinterpret_cast<DWORD>(ownedIndices);
		mesh.SetIndexDesc(stride, total);
		result = 1;
	}
	return result;
}

void Gfx_ModelMesh::SetMaterial(DWORD index, DWORD style, const std::string& materialName,
	DWORD /*unused*/, DWORD blend, const D3DMATERIAL8& values, DWORD textures, const std::string* names,
	const DWORD* handles, BYTE flagB9, BYTE flagBA)
{
	if (index >= mesh.primitiveCount) return;
	Gfx_Material& material = mesh.materialSet.materials[index];
	material.name = materialName;
	material.style1 = style;
	material.style2 = style;
	material.SetValues(values);
	material.flagBA = flagBA;
	material.mode24 = material.mode28 = blend;
	material.flagB9 = flagB9;
	if (textures > 4) textures = 4;
	for (DWORD texture = 0; texture < textures; ++texture)
	{
		if (texture < 4)
		{
			material.textureNames[texture] = names[texture];
			material.textureHandles[texture] = handles[texture];
		}
	}
}
