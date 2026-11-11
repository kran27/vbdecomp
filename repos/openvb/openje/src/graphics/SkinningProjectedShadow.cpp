#include "Skinning.hpp"
#include "SkinRender.hpp"
#include <cstring>

void Gfx_Skinning::ProcessProjectedShadow(const void* faces52, const void* edges12,
	DWORD edgeCount, const void* vertices, DWORD vertexStride,
	const D3DXVECTOR3* lightPoint, const DWORD* primitiveIDs, const BYTE* primitiveFlags)
{
	//4ABED0: the face bytes and selected-edge count are rebuilt each call. The
	//other eight bytes of each12-byte scratch edge retain their unwritten state.
	value08 = 0;
	std::memset(owned10, 0, value00);
	const Gfx_SkinFace* faces = static_cast<const Gfx_SkinFace*>(faces52);
	DWORD face;
	for (face = 0; face < value00; ++face)
	{
		const Gfx_SkinFace& current = faces[face];
		if (current.flagBA && (!current.flagB9 ||
			current.normal.z * lightPoint->z + current.normal.y * lightPoint->y +
			current.normal.x * lightPoint->x > 0.0f)) static_cast<BYTE*>(owned10)[face] = 1;
	}

	const Gfx_SkinEdge* edges = static_cast<const Gfx_SkinEdge*>(edges12);
	DWORD edge;
	for (edge = 0; edge < edgeCount; ++edge)
	{
		const Gfx_SkinEdge& current = edges[edge];
		if (primitiveIDs && primitiveFlags &&
			!primitiveFlags[primitiveIDs[current.ownerFace]]) continue;
		if (static_cast<BYTE*>(owned10)[current.ownerFace])
		{
			if (current.adjacentFace != -1 && static_cast<BYTE*>(owned10)[current.adjacentFace]) continue;
			static_cast<Gfx_SkinEdge*>(owned0C)[value08].vertices[0] = current.vertices[0];
			static_cast<Gfx_SkinEdge*>(owned0C)[value08].vertices[1] = current.vertices[1];
		}
		else
		{
			if (current.adjacentFace == -1 || !static_cast<BYTE*>(owned10)[current.adjacentFace]) continue;
			static_cast<Gfx_SkinEdge*>(owned0C)[value08].vertices[0] = current.vertices[1];
			static_cast<Gfx_SkinEdge*>(owned0C)[value08].vertices[1] = current.vertices[0];
		}
		++value08;
	}

	const BYTE* vertexData = static_cast<const BYTE*>(vertices);
	if (!value08) return;
	D3DXVECTOR3* output = static_cast<D3DXVECTOR3*>(owned14);
	for (edge = 0; edge < value08; ++edge)
	{
		const Gfx_SkinEdge* selected = static_cast<const Gfx_SkinEdge*>(owned0C);
		const D3DXVECTOR3 first = *reinterpret_cast<const D3DXVECTOR3*>(
			vertexData + vertexStride * selected[edge].vertices[0]);
		const D3DXVECTOR3 second = *reinterpret_cast<const D3DXVECTOR3*>(
			vertexData + vertexStride * selected[edge].vertices[1]);
		//Literal66DBC4 is20.0f. The original explicitly stores scaledX and
		//scaledZ as floats, but retains Y and the first Z subtraction in x87.
		const float scaledX = 20.0f * lightPoint->x;
		const double scaledY = 20.0f * lightPoint->y;
		const double scaledZ = 20.0f * lightPoint->z;
		const float roundedZ = static_cast<float>(scaledZ);
		D3DXVECTOR3 firstFar;
		firstFar.x = first.x - scaledX;
		firstFar.y = static_cast<float>(first.y - scaledY);
		firstFar.z = static_cast<float>(first.z - scaledZ);
		D3DXVECTOR3 secondFar;
		secondFar.x = second.x - scaledX;
		secondFar.y = static_cast<float>(second.y - scaledY);
		secondFar.z = second.z - roundedZ;
		output[0] = second;
		output[1] = first;
		output[2] = firstFar;
		output[3] = second;
		output[4] = firstFar;
		output[5] = secondFar;
		output += 6;
	}
}

DWORD Gfx_Skinning::WriteShadowVertices(const D3DXMATRIX* transform, D3DXVECTOR3* output)
{
	//4AC190: retain the actual count result and live source/count reads around
	//each real SDK call. The callers consume the generated vertices, not EAX.
	DWORD result = value08;
	if (!result) return result;
	if (!transform)
	{
		std::memcpy(output, owned14, 72u * result);
		return result;
	}
	DWORD count = 6u * result;
	if (!count) return count;
	DWORD vertex = 0, offset = 0;
	do
	{
		D3DXVec3TransformCoord(reinterpret_cast<D3DXVECTOR3*>(
			reinterpret_cast<BYTE*>(output) + offset), reinterpret_cast<const D3DXVECTOR3*>(
			static_cast<const BYTE*>(owned14) + offset), transform);
		result = value08;
		++vertex;
		offset += 12;
	}
	while (vertex < 6u * result);
	return result;
}
