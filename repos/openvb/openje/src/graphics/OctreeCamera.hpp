#pragma once

#include "Octree.hpp"
#include <d3dx8.h>

// Actual map camera (436 bytes on x86), independent of Gfx_Camera.
class OCTREE_Camera
{
public:
	OCTREE_Camera(); // 0x5B0020; deliberately leaves the observed fields unwritten.
	virtual int Release(); // +00 ->0x5BBCB0; self-delete, not a destructor slot.
	virtual void SetFrustum(float degrees, float aspect, float nearPlane, float farPlane); // 0x5BBD70.
	virtual void SetScreen(float width, float height, const D3DXMATRIX* projection); // 0x5BBFD0.
	virtual void SetMode(DWORD mode); // 0x5BC040; incidental EAX is unused.
	virtual DWORD BuildTransform(DWORD xBits, DWORD yBits, DWORD zBits,
		float pitchDegrees, float yawDegrees, D3DXMATRIX* output); //+10,5BC080; coordinates copied as raw words.
	virtual int SetTransform(const D3DXMATRIX* source); // 0x5BC1F0.
	virtual int ClassifyBox(const float* bounds); // 0x5BC530; six min/max floats.
	virtual int MakeScreenRay(LONG x, LONG y, OctreeRay* output); //+1C,5BCD20; signed pixels.
	virtual int QueryObjects(const OctreeRay& ray, DWORD capacity, OctreeObjectHit* output,
		DWORD* more); //+20,5BCDD0; camera-visible candidates, sentinel and continuation.

	int ClassifySpatialBox(const D3DXVECTOR3* center, const D3DXVECTOR3* extent); // 0x5BC8E0.

	D3DXVECTOR3 position; // +04; unwritten until SetTransform.
	D3DXMATRIX transform; // +10; translation cleared by SetTransform.
	float projectionScaleX, projectionScaleY, projectionOffsetX, projectionOffsetY; // +50.
	float halfWidth, halfHeight; // +60; unwritten until SetScreen.
	D3DXVECTOR3 transformedRays[6]; // +68; includes near/far plane offsets.
	D3DXVECTOR3 planes[6]; // +B0; world-space normals.
	DWORD planeCorners[6]; // +F8; unwritten until SetTransform.
	D3DXVECTOR3 localRays[6]; // +110.
	D3DXVECTOR3 localPlanes[6]; // +158.
	DWORD objectCursor; // +1A0.
	OctreeNode** nodes; // +1A4; camera-owned, map borrows.
	DWORD mask; // +1A8.
	OctreeNodeReference** buckets; // +1AC; camera owns lists and outer array.
	OctreeSpatialObject** objects; // +1B0; owns references, not each live object.

private:
	~OCTREE_Camera() {}
	OCTREE_Camera(const OCTREE_Camera&);
	OCTREE_Camera& operator=(const OCTREE_Camera&);
};

typedef OCTREE_Camera OCTREE_Camera_API;
