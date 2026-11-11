#include "MeshData.hpp"
#include "SkinRender.hpp"
#include <string.h>

void Gfx_MeshData::BuildShadowGeometry() // 0x4AB740; WORD indices as in the original.
{
	value40 = value24 / 3;
	owned44 = ::operator new[](52 * value40);
	owned48 = ::operator new[](4 * value40);
	DWORD material = 0, start = 0, count = primitives[0].value30;
	for (DWORD i = 0; i < value40; ++i)
	{
		if (i - start >= count) { ++material; count = primitives[material].value30; start = i; }
		static_cast<DWORD*>(owned48)[i] = material;
	}
	for (DWORD i = 0; i < value40; ++i)
	{
		Gfx_SkinFace& face = static_cast<Gfx_SkinFace*>(owned44)[i];
		face.flagB9 = materialSet.materials[static_cast<DWORD*>(owned48)[i]].flagB9;
		face.flagBA = materialSet.materials[static_cast<DWORD*>(owned48)[i]].flagBA;
		face.edges[0].vertices[0] = reinterpret_cast<const WORD*>(value10)[3 * i];
		face.edges[0].vertices[1] = reinterpret_cast<const WORD*>(value10)[3 * i + 1];
		face.edges[0].adjacentFace = -1;
		face.edges[0].ownerFace = DWORD(-1);
		face.edges[1].vertices[0] = face.edges[0].vertices[1];
		face.edges[1].vertices[1] = reinterpret_cast<const WORD*>(value10)[3 * i + 2];
		face.edges[2].vertices[0] = face.edges[1].vertices[1];
		face.edges[1].adjacentFace = -1;
		face.edges[1].ownerFace = DWORD(-1);
		face.edges[2].adjacentFace = -1;
		face.edges[2].ownerFace = DWORD(-1);
		face.edges[2].vertices[1] = face.edges[0].vertices[0];
		const BYTE* data = static_cast<const BYTE*>(vertices);
		const D3DXVECTOR3& a = *reinterpret_cast<const D3DXVECTOR3*>(data + vertexStride * face.edges[0].vertices[0]);
		const D3DXVECTOR3& b = *reinterpret_cast<const D3DXVECTOR3*>(data + vertexStride * face.edges[1].vertices[0]);
		const D3DXVECTOR3& c = *reinterpret_cast<const D3DXVECTOR3*>(data + vertexStride * face.edges[2].vertices[0]);
		D3DXVECTOR3 ab = b - a, ac = c - a;
		D3DXVec3Cross(&face.normal, &ab, &ac);
		D3DXVec3Normalize(&face.normal, &face.normal);
	}
	Gfx_SkinFace* faces = static_cast<Gfx_SkinFace*>(owned44);
	for (DWORD i = 0; i < value40; ++i)
	{
		for (DWORD edge = 0; edge < 3; ++edge)
		{
			Gfx_SkinFace::Edge& current = faces[i].edges[edge];
			if (current.adjacentFace != -1) continue;
			current.ownerFace = i;
			if (!faces[i].flagBA) continue;
			bool found = false;
			for (DWORD other = 0; other < value40 && !found; ++other)
			{
				if (other == i) continue;
				for (DWORD candidate = 0; candidate < 3; ++candidate)
				{
					Gfx_SkinFace::Edge& test = faces[other].edges[candidate];
					if ((current.vertices[0] == test.vertices[0] && current.vertices[1] == test.vertices[1]) ||
						(current.vertices[0] == test.vertices[1] && current.vertices[1] == test.vertices[0]))
					{
						current.adjacentFace = other; test.adjacentFace = i; test.ownerFace = other;
						found = true;
						break;
					}
				}
			}
		}
	}
	Gfx_SkinEdge* edges = static_cast<Gfx_SkinEdge*>(::operator new[](36 * value40));
	value4C = 0;
	for (DWORD i = 0; i < value40; ++i)
	{
		for (DWORD edge = 0; edge < 3; ++edge)
		{
			const Gfx_SkinFace::Edge& current = faces[i].edges[edge];
			bool found = false;
			if (current.adjacentFace != -1)
			{
				for (DWORD prior = 0; prior < value4C; ++prior)
				{
					const Gfx_SkinEdge& test = edges[prior];
					if ((current.vertices[0] == test.vertices[0] && current.vertices[1] == test.vertices[1]) ||
						(current.vertices[0] == test.vertices[1] && current.vertices[1] == test.vertices[0]))
					{ found = true; break; }
				}
			}
			if (!found)
			{
				memcpy(&edges[value4C], &current, sizeof(Gfx_SkinEdge));
				++value4C;
			}
		}
	}
	owned50 = ::operator new[](12 * value4C);
	for (DWORD i = 0; i < value4C; ++i)
		static_cast<Gfx_SkinEdge*>(owned50)[i] = edges[i];
	if (edges) ::operator delete[](edges);
}
