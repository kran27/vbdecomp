#pragma once

#include <d3dx8.h>

// Actual global RTTI identity and seven observed virtual operations. English
// method names and unconsumed void returns are source hypotheses. No unused
// slot is fabricated. Construction uses the real SDK identity operation;
// ordinary destruction is shared by observed derived destruction.
class Gfx_BaseObject
{
public:
	Gfx_BaseObject() { D3DXMatrixIdentity(&transform); } // Inlined; no separate original address.
	virtual ~Gfx_BaseObject() = 0; // Ordinary 0x4245B0; actual base table uses purecall.
	virtual void SetTransform(const D3DXMATRIX& matrix); // +04 ->0x49CB90; snapshot copy.
	virtual void SetPosition(float x, float y, float z); // +0C ->0x424570.
	virtual void SetPosition(const D3DXVECTOR3& position); // +08 ->0x424590; VC7 reverses overload declarations.
	virtual void SetEulerRotation(float yaw, float pitch, float roll); // +10 ->0x424620.
	virtual void SetQuaternionRotation(const D3DXQUATERNION& rotation); // +14 ->0x4245C0.
	virtual void OnTransformChanged(); // +18 ->0x404B70 in the actual camera/light tables.

	// The original matrix copy writes 64 bytes at +04 through +43.
	// Camera and Light share this 68-byte base.

	D3DXMATRIX transform; // +04; position is _41/_42/_43 at+34/+38/+3C.
};

inline Gfx_BaseObject::~Gfx_BaseObject() {}
