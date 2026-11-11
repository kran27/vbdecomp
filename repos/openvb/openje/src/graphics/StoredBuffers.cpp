#include "BufferRecord.hpp"
#include "BufferManager.hpp"
#include "D3DState.hpp"
#include "MeshData.hpp"
#include <cstring>

HRESULT Gfx_BufferRecord::InitializeVertices(UINT count, UINT stride,
	DWORD format, DWORD usage, const void* data)
{
	const UINT bytes = count * stride;
	vertexCount = count;
	vertexStride = stride;
	fvf = format;
	vertexData = data;
	vertices = new Gfx_VertexBuffer;
	HRESULT result = vertices->Create(bytes, usage | D3DUSAGE_WRITEONLY,
		format, D3DPOOL_MANAGED);
	if (SUCCEEDED(result))
	{
		Gfx_VertexBuffer* wrapper = vertices;
		BYTE* destination = 0;
		result = wrapper->buffer->Lock(0, 0, &destination, 0);
		if (SUCCEEDED(result))
		{
			std::memcpy(destination, data, bytes);
			vertices->buffer->Unlock();
			vertices->vertexStride = stride;
		}
		else if (vertices)
		{
			delete vertices;
			vertices = 0;
		}
	}
	else if (vertices)
	{
		delete vertices;
		vertices = 0;
	}
	return result;
}

HRESULT Gfx_BufferRecord::InitializeIndices(UINT count, DWORD usage,
	const void* data)
{
	const UINT bytes = count * g_D3D.value0A4;
	indexCount = count;
	indexData = data;
	indices = new Gfx_IndexBuffer;
	HRESULT result = indices->Create(bytes, usage | D3DUSAGE_WRITEONLY,
		static_cast<D3DFORMAT>(g_D3D.value0AC), D3DPOOL_MANAGED);
	if (SUCCEEDED(result))
	{
		Gfx_IndexBuffer* wrapper = indices;
		BYTE* destination = 0;
		result = wrapper->buffer->Lock(0, 0, &destination, 0);
		if (SUCCEEDED(result))
		{
			std::memcpy(destination, data, bytes);
			indices->buffer->Unlock();
		}
		else if (indices)
		{
			delete indices;
			indices = 0;
		}
	}
	else if (indices)
	{
		delete indices;
		indices = 0;
	}
	return result;
}

bool Gfx_BufferManager::CreateStoredBuffers(Gfx_MeshData* mesh)
{
	HRESULT result = D3DERR_INVALIDCALL;
	const DWORD handle = AllocateHandle();
	if (!handle) return false;
	Gfx_BufferRecord* record = new Gfx_BufferRecord;
	if (!record) return false;
	const void* vertexData = mesh->vertices;
	const void* indexData = reinterpret_cast<const void*>(mesh->value10);
	if (vertexData)
	{
		const UINT count = mesh->vertexCount;
		const UINT stride = mesh->vertexStride;
		result = record->InitializeVertices(count, stride, mesh->vertexFVF,
			usage, vertexData);
		if (SUCCEEDED(result))
		{
			vertexBytes += count * stride;
			if (vertexHighWater < vertexBytes) vertexHighWater = vertexBytes;
		}
	}
	if (indexData)
	{
		const UINT count = mesh->value24;
		result = record->InitializeIndices(count, usage, indexData);
		if (SUCCEEDED(result))
		{
			indexBytes += count * g_D3D.value0A4;
			if (indexHighWater < indexBytes) indexHighWater = indexBytes;
		}
	}
	records.insert(std::make_pair(handle, static_cast<Gfx_BufferRecord*>(0)))
		.first->second = record;
	mesh->value04 = handle;
	return SUCCEEDED(result);
}
