#include "BufferManager.hpp"
#include "D3DState.hpp"

void Gfx_BufferManager::InitializeStreams(BYTE omitDoNotClip)
{
	usage = omitDoNotClip != 0 ? 0 : D3DUSAGE_DONOTCLIP;
	vertexStream->Create(0x1C0000, usage | 0x208, 0, D3DPOOL_DEFAULT);
	// Create HRESULTs are ignored. Accounting occurs after each call, even when
	// a buffer already exists or the actual SDK creation failed.
	vertexBytes += 0x1C0000;
	if (vertexHighWater < vertexBytes)
	{
		vertexHighWater = vertexBytes;
	}
	// Reload usage and the index owner after the first callback.
	indexStream->Create(0x60000, usage | 0x208,
		static_cast<D3DFORMAT>(g_D3D.value0AC), D3DPOOL_DEFAULT);
	indexBytes += 0x60000;
	if (indexHighWater < indexBytes)
	{
		indexHighWater = indexBytes;
	}
}

HRESULT Gfx_StreamingVertexBuffer::Create(UINT length, DWORD usage, DWORD fvf, D3DPOOL pool)
{
	return Gfx_VertexBuffer::Create(length, usage | D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY,
		fvf, D3DPOOL_DEFAULT);
}

HRESULT Gfx_StreamingIndexBuffer::Create(UINT length, DWORD usage, D3DFORMAT format,
	D3DPOOL pool)
{
	return Gfx_IndexBuffer::Create(length, usage | D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY,
		format, D3DPOOL_DEFAULT);
}

void Gfx_StreamingVertexBuffer::Release()
{
	bytePosition = 0;
	Gfx_VertexBuffer::Release();
}

void Gfx_StreamingIndexBuffer::Release()
{
	capacity = 0;
	indexStride = 0;
	bytePosition = 0;
	indexPosition = 0;
	Gfx_IndexBuffer::Release();
}

Gfx_VertexBuffer::~Gfx_VertexBuffer()
{
	Release();
}

Gfx_IndexBuffer::~Gfx_IndexBuffer()
{
	Release();
}
