#pragma once

#include <windows.h>
#include <d3dx8.h>

// Descriptive name for the original six-word scratch owner; broader producers
// and exact field meanings remain unresolved. No available library is replaced.
class Gfx_Skinning
{
public:
	Gfx_Skinning(); // 0x4ABE60.
	~Gfx_Skinning(); // 0x4AC210.
	void Free(); // 0x4ABE80; retains value04.
	void Allocate(DWORD faceCount); // 0x4AC240.
	void FinishAllocation(); // Observed shared empty boundary404B70; descriptive name.
	void ProcessProjectedShadow(const void* faces52, const void* edges12, DWORD edgeCount,
		const void* vertices, DWORD vertexStride, const D3DXVECTOR3* lightPoint,
		const DWORD* primitiveIDs, const BYTE* primitiveFlags); //4ABED0; actual face/edge extrusion, unused result.
	DWORD WriteShadowVertices(const D3DXMATRIX* transform, D3DXVECTOR3* output); //4AC190, RET8.
	DWORD value00, value04, value08;
	void* owned0C;
	void* owned10;
	void* owned14;

private:
	Gfx_Skinning(const Gfx_Skinning&);
	Gfx_Skinning& operator=(const Gfx_Skinning&);
};
