#pragma once

#include "VertexBuffer.hpp"
#include "IndexBuffer.hpp"

// Actual32-byte stored-buffer record: factory0x507660 allocates and clears all
// eight words. The inline default writes are evidenced in that factory, not a
// separate function target. Destruction owns both wrappers:0x506A10 inlines
// the policy and0x5077A0 uses the compiler deleting helper at0x506850.
class Gfx_BufferRecord
{
public:
	Gfx_BufferRecord() // Inlined in0x507660; no separately counted constructor.
		: vertices(0), indices(0), vertexCount(0), vertexStride(0), fvf(0),
		vertexData(0), indexCount(0), indexData(0) {}
	~Gfx_BufferRecord()
	{
		if (vertices)
		{
			delete vertices;
			vertices = 0;
		}
		if (indices)
		{
			delete indices;
			indices = 0;
		}
	}
	HRESULT InitializeVertices(UINT count, UINT stride, DWORD fvf,
		DWORD usage, const void* data); // 0x506550; publishes before allocation.
	HRESULT InitializeIndices(UINT count, DWORD usage, const void* data); // 0x506660.
	HRESULT Bind(UINT stream); // 0x506760; SDK results and live reload order.
	Gfx_VertexBuffer* vertices; // +00, owned wrapper.
	Gfx_IndexBuffer* indices; // +04, owned wrapper.
	UINT vertexCount, vertexStride; // +08/+0C, published by0x506550.
	DWORD fvf; // +10.
	const void* vertexData; // +14, source retained without copying here.
	UINT indexCount; // +18, published by0x506660.
	const void* indexData; // +1C.

private:
	Gfx_BufferRecord(const Gfx_BufferRecord&);
	Gfx_BufferRecord& operator=(const Gfx_BufferRecord&);
};
