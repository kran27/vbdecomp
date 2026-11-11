#pragma once

#include "ModelMesh.hpp"
#include "PacketData.hpp"
#include <vector>

#include "LineSystemValue.hpp"

class Gfx_Viewport;

// Actual embedded DynamicObject/ribbon line geometry owner.
class Gfx_LineSystem : public Gfx_NodeAttribute
{
public:
	Gfx_LineSystem(); // 0x50F910.
	Gfx_LineSystem(const Gfx_LineSystem& source); // 0x50FB40; fresh geometry ownership.
	virtual ~Gfx_LineSystem(); // 0x50EF50.
	Gfx_LineSystem& operator=(const Gfx_LineSystem& source); // 0x50F780; values only.
	LONG GetSegmentCount() const; //49D260.
	LONG* GetSegment(LONG index, float* x1, float* y1, float* z1,
		float* x2, float* y2, float* z2, LONG* r1, LONG* g1, LONG* b1, LONG* a1,
		LONG* r2, LONG* g2, LONG* b2, LONG* a2) const; //50E8C0; nullable outputs.
	void AddSegment(float x1, float y1, float z1, float x2, float y2, float z2,
		LONG r1, LONG g1, LONG b1, LONG a1, LONG r2, LONG g2, LONG b2, LONG a2); //50F870; fourteen scalar stack arguments.
	void Clear(); //50EAD0; release values and raw geometry, dirty1.
	void BuildGeometry(); //50F060; actual FVF42 line-list mesh and materials.
	void Draw(Gfx_Viewport* viewport); //50F4C0.
	Gfx_MeshData mesh;
	Gfx_MeshInstance* instances;
	Gfx_PacketData* packet;
	D3DXMATRIX transform;
	DWORD instanceCount;
	void* vertices;
	DWORD vertexCount, vertexCapacity;
	void* indices;
	DWORD indexCount, indexCapacity;
	D3DXVECTOR3 position;
	DWORD color, value104;
	std::vector<Gfx_LineSystemValue> values108; // Original allocator+108, first pointer+10C.
	BYTE enabled; //Original+118 geometryDirty; not a draw-enable gate.
};
