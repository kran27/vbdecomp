#pragma once

#include <windows.h>
#include "graphics/MeshData.hpp"
#include "graphics/UIVertex.hpp"

// Gfx_UIPacketBuffer and method names are
// source hypotheses; the nonvirtual 0xA4-byte layout and argument ABI are observed.
class Gfx_UIPacketBuffer
{
public:
	Gfx_UIPacketBuffer(); // 0x4B65C0; normal mesh/instance member construction.
	~Gfx_UIPacketBuffer(); // 0x4B6670, including its separated compiler EH chunks.
	void ConfigureQuads(DWORD quadCount, LONG vertexCount, DWORD textureHandle); // 0x4B6D40.
	void ConfigureTriangles(DWORD triangleCount, DWORD vertexCount,
		DWORD textureHandle, DWORD color); // 0x4B6EB0.

	Gfx_MeshData mesh; // +00..6B; owns its actual primitive/material allocations.
	Gfx_Primitive* primitive; // +6C; borrowed from mesh.primitives.
	JE::Gfx::UIVertex* vertices; // +70; borrowed storage arena memory.
	Gfx_Material* material; // +74; borrowed from mesh.materialSet.materials.
	Gfx_MeshInstance instance; // +78..9F; its actual independent owned flag array.
	WORD* indices; // +A0; borrowed storage arena memory.

private:
	Gfx_UIPacketBuffer(const Gfx_UIPacketBuffer&);
	Gfx_UIPacketBuffer& operator=(const Gfx_UIPacketBuffer&);
};
