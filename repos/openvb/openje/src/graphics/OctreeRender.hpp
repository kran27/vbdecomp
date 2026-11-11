#pragma once

#include "Octree.hpp"
#include "OctreeCamera.hpp"
#include "MeshData.hpp"
#include "PacketData.hpp"
#include "Light.hpp"
#include <list>
#include <map>
#include <set>

class Gfx_Viewport;

struct OctreeFloorSegment
{
	OctreeFloorSegment(); // 0x4B8F20.
	~OctreeFloorSegment(); // 0x4B8F40.
	BYTE visible;
	BYTE unknown01[3]; // Unwritten; only the low visibility byte is initialized.
	DWORD nodeOrdinal, triangleCount, indexCount;
	WORD* indices;
};

struct OctreeFloorGroup
{
	OctreeFloorGroup(); // 0x4B8F60.
	~OctreeFloorGroup(); // 0x4B9390.
	BYTE visible;
	BYTE unknown01[3]; // Unwritten; the original FloorCB also tests only the BYTE.
	DWORD count;
	OctreeFloorSegment* segments;
};

struct OctreeFadeState
{
	DWORD timestamp;
	BYTE direction;
	// Three ordinary compiler padding bytes remain unwritten by the producer.
};

class Octree2Gfx
{
public:
	Octree2Gfx(); // 0x4BDAE0; actual owner, normal containers.
	~Octree2Gfx(); // 0x4BD470.
	OCTREE* Cleanup(); // 0x4BB7A0; returns/borrows map, owns render data/camera.
	void ConstructRenderData(); // 0x4BDEA0; actual mesh/material producer.
	BYTE Initialize(OCTREE* map, DWORD capacity, DWORD color, BYTE bounds, BYTE textures); // 0x4BFB60.
	void SetAmbient(DWORD color); // 0x4B8F70; complement is supplied by Initialize.
	void SetInstanceFlag(BYTE value); // 0x4B9110.
	int UpdateCamera(Gfx_Viewport* viewport); // 0x4B93D0; NULL selects the default viewport.
	int RestoreVisibility(); // 0x4B90C0.
	void SetHeightFogFlag(BYTE value); // 0x4B90E0; raw instance flag1D.
	void ClearHiddenMaterials(); // 0x4B9DB0.
	void SetHiddenMaterials(const std::set<int>& hidden); // 0x4BB5B0.
	BYTE GetMaterialState(int key); // 0x4B9DF0; raw BYTE, absent returns0.
	void SetMaterialState(int key, BYTE value); // 0x4BBAA0; incidental AL unused.
	void UpdateMaterialStates(); // 0x4BC050; incidental return unused by caller.
	void SubmitVisible(BYTE debugBounds, Gfx_Viewport* viewport); // 0x4BC6F0.
	void ResetLights(); //4B9140, real packet owners.
	void AddLight(Gfx_Light* light, const std::map<DWORD,std::list<DWORD> >* nodes); //4B9B90.
	void RenderFloorCB(); //0x4B94D0; projected-shadow texture on visible floor segments.

	BYTE floorEnabled, resourceTextures;
	DWORD sceneMode;
	D3DXVECTOR4 ambient;
	OCTREE* octree; // +18 original; borrowed, never released by Cleanup.
	OCTREE_Camera* camera; // +1C; actual owned map camera.
	D3DXVECTOR3 minimum, maximum;
	DWORD nodeIDCapacity;
	DWORD* nodeIDs;
	DWORD materialCount, textureCount;
	DWORD* textureHandles;
	DWORD value4C;
	DWORD lightCount;
	Gfx_Light* lights;
	DWORD meshCount, nodeCount;
	BYTE** vertices;
	BYTE** indices;
	OctreeFloorGroup* floorGroups;
	Gfx_MeshData* meshes;
	Gfx_MeshInstance* instances;
	Gfx_PacketData** packets;
	D3DXVECTOR3** nodeMinimum;
	D3DXVECTOR3** nodeMaximum;
	BYTE value80, showBounds;
	BYTE* boundsVertices;
	DWORD boundsVertexCount;
	WORD* boundsIndices;
	Gfx_MeshData* boundsMesh;
	Gfx_Primitive* boundsPrimitive;
	Gfx_MeshInstance* boundsInstance;
	// Original find/insert branches use signed comparisons in all seven keys.
	std::map<int, std::list<Gfx_Material*> > opacityMaterials;
	std::map<int, std::list<Gfx_Material*> > colorMaterials;
	std::map<int, std::list<Gfx_Material*> > enabledMaterials;
	std::set<int> valueC0;
	std::set<int> valueCC;
	std::map<int, BYTE> materialStates;
	std::map<int, OctreeFadeState> fadeStates;

private:
	Octree2Gfx(const Octree2Gfx&);
	Octree2Gfx& operator=(const Octree2Gfx&);
};
