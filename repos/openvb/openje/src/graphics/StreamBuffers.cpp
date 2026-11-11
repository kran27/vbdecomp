#include "VertexBuffer.hpp"
#include "IndexBuffer.hpp"
#include "D3DState.hpp"

Gfx_VertexBuffer::Gfx_VertexBuffer()
{
	buffer = 0;
	capacity = 0;
	fvf = 0;
	vertexStride = 0;
}

HRESULT Gfx_VertexBuffer::Create(UINT length, DWORD usage, DWORD fvf, D3DPOOL pool)
{
	HRESULT result = D3DERR_INVALIDCALL;
	if (!buffer)
	{
		if (usage & D3DUSAGE_DYNAMIC) pool = D3DPOOL_DEFAULT;
		capacity = length;
		this->fvf = fvf;
		result = g_D3D.device->CreateVertexBuffer(length, usage, fvf, pool, &buffer);
	}
	return result;
}

void Gfx_VertexBuffer::Release()
{
	if (buffer)
	{
		buffer->Release();
		buffer = 0;
	}
	capacity = 0;
	fvf = 0;
	vertexStride = 0;
}

Gfx_StreamingVertexBuffer::Gfx_StreamingVertexBuffer()
{
	bytePosition = 0;
}

Gfx_StreamingVertexBuffer::~Gfx_StreamingVertexBuffer()
{
	bytePosition = 0;
}

bool Gfx_StreamingVertexBuffer::CanFit(UINT count, UINT stride)
{
	bool fits = false;
	const UINT first = bytePosition / stride + (bytePosition % stride != 0);
	if (static_cast<LONG>(stride * count) <= static_cast<LONG>(capacity - stride * first)) fits = true;
	return fits;
}

HRESULT Gfx_StreamingVertexBuffer::Lock(UINT count, UINT stride, BYTE** output,
	UINT* base, BYTE discard)
{
	const UINT bytes = stride * count;
	UINT first = bytePosition / stride + (bytePosition % stride != 0);
	UINT offset = stride * first;
	HRESULT result;
	if (discard || count + first >= g_D3D.vertexLimit || bytes + offset > capacity)
	{
		first = 0;
		offset = 0;
		result = buffer->Lock(0, bytes, output, D3DLOCK_NOSYSLOCK | D3DLOCK_DISCARD);
	}
	else
	{
		result = buffer->Lock(offset, bytes, output, D3DLOCK_NOSYSLOCK | D3DLOCK_NOOVERWRITE);
		if (FAILED(result)) return result;
	}
	if (SUCCEEDED(result))
	{
		*base = first;
		bytePosition = offset + bytes;
	}
	return result;
}

Gfx_IndexBuffer::Gfx_IndexBuffer()
{
	buffer = 0;
	capacity = 0;
	indexStride = 0;
}

HRESULT Gfx_IndexBuffer::Create(UINT length, DWORD usage, D3DFORMAT format, D3DPOOL pool)
{
	HRESULT result = D3DERR_INVALIDCALL;
	if (!buffer)
	{
		if (usage & D3DUSAGE_DYNAMIC) pool = D3DPOOL_DEFAULT;
		capacity = length;
		indexStride = g_D3D.value0A4;
		result = g_D3D.device->CreateIndexBuffer(length, usage, format, pool, &buffer);
	}
	return result;
}

void Gfx_IndexBuffer::Release()
{
	capacity = 0;
	indexStride = 0;
	if (buffer)
	{
		buffer->Release();
		buffer = 0;
	}
}

Gfx_StreamingIndexBuffer::Gfx_StreamingIndexBuffer()
{
	bytePosition = 0;
	indexPosition = 0;
}

Gfx_StreamingIndexBuffer::~Gfx_StreamingIndexBuffer()
{
	bytePosition = 0;
	indexPosition = 0;
}

HRESULT Gfx_StreamingIndexBuffer::Lock(UINT count, BYTE** output, UINT* base, BYTE discard)
{
	UINT offset = bytePosition;
	UINT first = indexPosition;
	const UINT bytes = count * indexStride;
	HRESULT result;
	if (discard || bytes + offset >= capacity)
	{
		offset = 0;
		result = buffer->Lock(first = 0, bytes, output, D3DLOCK_NOSYSLOCK | D3DLOCK_DISCARD);
	}
	else
	{
		result = buffer->Lock(offset, bytes, output, D3DLOCK_NOSYSLOCK | D3DLOCK_NOOVERWRITE);
		if (FAILED(result)) return result;
	}
	if (SUCCEEDED(result))
	{
		*base = first;
		indexPosition = first + count;
		bytePosition = offset + bytes;
		g_D3D.statistics[5] += count;
	}
	return result;
}

HRESULT Gfx_VertexBuffer::SetStreamSource(UINT stream, UINT stride) //4900F0.
{
	IDirect3DVertexBuffer8* vertexBuffer = buffer;
	++g_D3D.statistics[9];
	return g_D3D.device->SetStreamSource(stream, vertexBuffer, stride);
}
