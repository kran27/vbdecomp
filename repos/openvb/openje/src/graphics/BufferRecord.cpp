#include "BufferRecord.hpp"
#include "BufferManager.hpp"
#include "D3DState.hpp"

HRESULT Gfx_BufferRecord::Bind(UINT stream)
{
	HRESULT result = D3DERR_INVALIDCALL;
	if (vertices)
	{
		const UINT stride = vertices->vertexStride;
		IDirect3DVertexBuffer8* buffer = vertices->buffer;
		++g_D3D.statistics[9];
		result = g_D3D.device->SetStreamSource(stream, buffer, stride);
	}
	if (indices)
	{
		return g_D3D.device->SetIndices(indices->buffer, 0);
	}
	return result;
}

HRESULT Gfx_BufferManager::BindVertexData(DWORD handle)
{
	HRESULT result = D3DERR_INVALIDCALL;
	if (handle != 0xFFFFFFFF)
	{
		std::map<DWORD, Gfx_BufferRecord*>::iterator found = records.find(handle);
		if (found != records.end()) result = found->second->Bind(0);
	}
	return result;
}

Gfx_BufferRecord* Gfx_BufferManager::FindStoredBuffer(DWORD handle) //506B30.
{
	Gfx_BufferRecord* result = NULL;
	if (handle != 0xFFFFFFFF)
	{
		std::map<DWORD, Gfx_BufferRecord*>::iterator found = records.find(handle);
		if (found != records.end()) result = found->second;
	}
	return result;
}
