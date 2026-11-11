#pragma once

#include <d3d8.h>

// Global class identities are present in the original RTTI. The three real
// virtual slots are deleting destruction, Create and Release; Lock is not virtual.
class Gfx_VertexBuffer
{
public:
	Gfx_VertexBuffer(); // 0x505310.
	virtual ~Gfx_VertexBuffer(); // 0x5055D0; compiler owns deleting destruction.
	virtual HRESULT Create(UINT length, DWORD usage, DWORD fvf, D3DPOOL pool); // 0x505330.
	virtual void Release(); // 0x505380; void source hypothesis, no observed return consumer.
	HRESULT SetStreamSource(UINT stream, UINT stride); //4900F0; captures buffer before global counter publication.

	IDirect3DVertexBuffer8* buffer; // +04; owned COM reference.
	UINT capacity; // +08; requested byte capacity, published before device creation.
	DWORD fvf; // +0C.
	UINT vertexStride; // +10; separately assigned by the stored-buffer upload API.
};

class Gfx_StreamingVertexBuffer : public Gfx_VertexBuffer
{
public:
	Gfx_StreamingVertexBuffer(); // 0x5055E0.
	virtual ~Gfx_StreamingVertexBuffer(); // 0x505620.
	virtual HRESULT Create(UINT length, DWORD usage, DWORD fvf, D3DPOOL pool); // 0x505600.
	virtual void Release(); // 0x5053B0; clears the streaming cursor first.
	bool CanFit(UINT count, UINT stride); // 0x5053C0; signed comparison of wrapped results.
	HRESULT Lock(UINT count, UINT stride, BYTE** output, UINT* base, BYTE discard); // 0x505400.

	UINT bytePosition; // +14; ceil-aligned by Lock to its incoming stride.
};
