#pragma once

#include <d3d8.h>

class Gfx_IndexBuffer
{
public:
	Gfx_IndexBuffer(); // 0x5054A0.
	virtual ~Gfx_IndexBuffer(); // 0x505640; compiler owns deleting destruction.
	virtual HRESULT Create(UINT length, DWORD usage, D3DFORMAT format, D3DPOOL pool); // 0x5054C0.
	virtual void Release(); // 0x505510; void source hypothesis, no observed return consumer.

	IDirect3DIndexBuffer8* buffer; // +04; owned COM reference.
	UINT capacity; // +08; requested byte capacity, published before device creation.
	UINT indexStride; // +0C; copied from g_D3D.value0A4, not derived from format.
};

class Gfx_StreamingIndexBuffer : public Gfx_IndexBuffer
{
public:
	Gfx_StreamingIndexBuffer(); // 0x505650.
	virtual ~Gfx_StreamingIndexBuffer(); // 0x5056B0.
	virtual HRESULT Create(UINT length, DWORD usage, D3DFORMAT format, D3DPOOL pool); // 0x505670.
	virtual void Release(); // 0x505690; clears both cursors and descriptors first.
	HRESULT Lock(UINT count, BYTE** output, UINT* base, BYTE discard); // 0x505540.

	UINT bytePosition; // +10.
	UINT indexPosition; // +14.
};
