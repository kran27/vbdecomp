#pragma once

#include "MeshData.hpp"
#include "PacketData.hpp"

// The actual nonpolymorphic120-byte allocation constructed at0x505880 and
// destroyed at0x505CD0. No RTTI source name exists in the observed object;
// this address-derived spelling is explicit. Geometry is three wire circles
// and three diameters, shared by all instances.
class Gfx_Viewport;

class Gfx_DebugSphere505880 : public Gfx_MeshInstance
{
public:
	Gfx_DebugSphere505880(DWORD value, UINT segments); // 0x505880; value is unused.
	~Gfx_DebugSphere505880(); //505CD0; last user releases shared geometry.
	void Reset(); // 0x505750; packet scale, radius and transform reset; no lifetime claim.
	void SetPosition(float x, float y, float z); //0x5057A0; matrix translation only.
	void SetColors(BYTE useRGB, BYTE useAlpha, DWORD color); //0x505840.
	void Submit(Gfx_Viewport* viewport); //0x505820; actual mesh-instance queue.

	Gfx_BasePacketData* packet; // +28; owned and also borrowed by base.value0C.
	Gfx_MaterialSet* copiedMaterials; // +2C; independently owned.
	D3DXMATRIX transform; // +30..6F; packet borrows this matrix.
	float radius; // +70.
	DWORD unknown74; //505840 stores the packed color; constructor leaves it unwritten.

private:
	void CopyMaterials(); //0x5057C0; independent copy, borrowed by base.value04.
};
