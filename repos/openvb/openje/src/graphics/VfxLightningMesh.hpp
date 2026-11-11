#pragma once

#include "MeshData.hpp"
#include "PacketData.hpp"

namespace JE
{
	struct VFX_LightningVertex
	{
		float x, y, z;
		DWORD color;
		float u, v; // The original lightning builder leaves these unwritten.
	};

	// Actual nonvirtual204-byte owner created by4EA6F0. MaterialSet is the
	// embedded MeshData member at+28; the complete MeshData itself starts+00.
	struct VFX_LightningMesh
	{
		VFX_LightningMesh(); // 0x4E9460.
		Gfx_MeshData mesh; // +00.
		Gfx_MeshInstance* instances; // +6C; owned array.
		Gfx_PacketData* packet; // +70; owned, packet.transform borrows below.
		D3DXMATRIX transform; // +74, constructor identity.
		DWORD instanceCount; // +B4.
		VFX_LightningVertex* vertices; // +B8; raw allocation, original free API.
		DWORD vertexCount; // +BC; vertex count, not an independent capacity.
		WORD* indices; // +C0; raw allocation, original free API.
		DWORD indexCount; // +C4.
		BYTE flagC8; // +C8; trailing padding remains unwritten.
	private:
		VFX_LightningMesh(const VFX_LightningMesh&);
		VFX_LightningMesh& operator=(const VFX_LightningMesh&);
	};
}
