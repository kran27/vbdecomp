#include "DebugSphere.hpp"
#include "SystemInterface.hpp"
#include <math.h>
#include <new>

namespace
{
	//Original7089B8..708A43. MeshData borrows the vertices; its real material
	//and primitive members own their allocations. Each sphere owns its packet.
	D3DXVECTOR3* sphereVertices = NULL;
	DWORD sphereUsers = 0;
	DWORD sphereVertexCount = 0;
	DWORD sphereVertexStride = 0;
	Gfx_MeshData sphereMesh;
	DWORD sphereFVF = D3DFVF_XYZ; //6FBCFC.
	DWORD sphereEmissive = 0x80802000; //6FBD00; alpha is ignored by SetEmissive.
}

Gfx_DebugSphere505880::Gfx_DebugSphere505880(DWORD, UINT segments)
	: packet(NULL), copiedMaterials(NULL), radius(1.0f)
{
	//The shared registration handle, rather than the user count, gates creation.
	if (!sphereMesh.value04)
	{
		if (sphereVertices) { delete[] sphereVertices; sphereVertices = NULL; }
		sphereVertexStride = D3DXGetFVFVertexSize(sphereFVF);
		sphereVertexCount = 2 * (3 * segments + 3);
		sphereVertices = static_cast<D3DXVECTOR3*>(::operator new[](sizeof(D3DXVECTOR3) * sphereVertexCount));
		const float step = 6.2831855f / segments;
		for (UINT segment = 0; segment < segments; ++segment)
		{
			const double angle0 = double(segment) * step;
			const double angle1 = double(segment + 1) * step;
			const float sine0 = float(sin(angle0));
			const float cosine0 = float(cos(angle0));
			const float sine1 = float(sin(angle1));
			const float cosine1 = float(cos(angle1));
			sphereVertices[2 * segment].x = 0.0f;
			sphereVertices[2 * segment].y = sine0;
			sphereVertices[2 * segment].z = cosine0;
			sphereVertices[2 * segment + 1].x = 0.0f;
			sphereVertices[2 * segment + 1].y = sine1;
			sphereVertices[2 * segment + 1].z = cosine1;
			sphereVertices[2 * segments + 2 * segment].x = cosine0;
			sphereVertices[2 * segments + 2 * segment].y = 0.0f;
			sphereVertices[2 * segments + 2 * segment].z = sine0;
			sphereVertices[2 * segments + 2 * segment + 1].x = cosine1;
			sphereVertices[2 * segments + 2 * segment + 1].y = 0.0f;
			sphereVertices[2 * segments + 2 * segment + 1].z = sine1;
			sphereVertices[4 * segments + 2 * segment].x = sine0;
			sphereVertices[4 * segments + 2 * segment].y = cosine0;
			sphereVertices[4 * segments + 2 * segment].z = 0.0f;
			sphereVertices[4 * segments + 2 * segment + 1].x = sine1;
			sphereVertices[4 * segments + 2 * segment + 1].y = cosine1;
			sphereVertices[4 * segments + 2 * segment + 1].z = 0.0f;
		}
		const UINT axis = 6 * segments;
		sphereVertices[axis].x = 0.0f;
		sphereVertices[axis].y = 0.0f;
		sphereVertices[axis].z = 1.0f;
		sphereVertices[axis + 1].x = 0.0f;
		sphereVertices[axis + 1].y = 0.0f;
		sphereVertices[axis + 1].z = -1.0f;
		sphereVertices[axis + 2].x = 1.0f;
		sphereVertices[axis + 2].y = 0.0f;
		sphereVertices[axis + 2].z = 0.0f;
		sphereVertices[axis + 3].x = -1.0f;
		sphereVertices[axis + 3].y = 0.0f;
		sphereVertices[axis + 3].z = 0.0f;
		sphereVertices[axis + 4].x = 0.0f;
		sphereVertices[axis + 4].y = 1.0f;
		sphereVertices[axis + 4].z = 0.0f;
		sphereVertices[axis + 5].x = 0.0f;
		sphereVertices[axis + 5].y = -1.0f;
		sphereVertices[axis + 5].z = 0.0f;
		sphereMesh.vertices = sphereVertices;
		sphereMesh.value00 = 0;
		sphereMesh.SetVertexDesc(sphereFVF, sphereVertexStride, sphereVertexCount);
		Gfx_Primitive* primitive = sphereMesh.SetPrimitiveCount(1);
		primitive->type = D3DPT_LINELIST;
		primitive->value28 = 0;
		primitive->value30 = 3 * segments + 3;
		Gfx_Material& material = sphereMesh.materialSet.materials[0];
		material.style1 = 0;
		material.style2 = 0;
		material.mode28 = 5;
		material.mode24 = 5;
		material.SetEmissive(sphereEmissive);
		sphereMesh.materialSet.RebuildGroups();
		g_GfxSystem->CreateGeometryData(&sphereMesh);
	}
	++sphereUsers;
	D3DXMatrixIdentity(&transform);
	packet = Gfx_CreatePacketData(0);
	SetMesh(&sphereMesh);
	value0C = reinterpret_cast<DWORD>(packet);
	packet->transform = &transform;
	packet->transformEnabled = 1;
	//unknown74 remains unwritten until SetColors.
}

Gfx_DebugSphere505880::~Gfx_DebugSphere505880()
{
	if (packet)
	{
		delete packet;
		packet = 0;
	}
	Gfx_MaterialSet* capturedMaterials = copiedMaterials;
	if (capturedMaterials)
	{
		delete capturedMaterials;
		copiedMaterials = 0;
	}
	if (!--sphereUsers)
	{
		g_GfxSystem->ReleaseGeometryData(&sphereMesh);
		sphereMesh.Free();
		sphereMesh.value04 = 0;
		if (sphereVertices) { delete[] sphereVertices; sphereVertices = NULL; }
	}
	//The real MeshInstance base then releases its primitive flags and children.
}

void Gfx_DebugSphere505880::SetPosition(float x, float y, float z)
{
	transform._41 = x;
	transform._42 = y;
	transform._43 = z;
}

void Gfx_DebugSphere505880::CopyMaterials()
{
	copiedMaterials = new Gfx_MaterialSet(sphereMesh.materialSet);
	value04 = reinterpret_cast<DWORD>(copiedMaterials);
}

void Gfx_DebugSphere505880::SetColors(BYTE useRGB, BYTE useAlpha, DWORD color)
{
	if (!copiedMaterials) CopyMaterials();
	copiedMaterials->SetColors(useRGB, useAlpha, color, NULL);
	copiedMaterials->RebuildGroups();
	unknown74 = color;
}

void Gfx_DebugSphere505880::Submit(Gfx_Viewport* viewport)
{
	Gfx_System_Interface* system = g_GfxSystem;
	system->QueueMeshInstances(this, viewport, 1);
}
