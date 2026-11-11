#pragma once

#include <windows.h>
#include <stdio.h>
#include <float.h>
#include <string.h>
#include "GfxVector3.hpp"

// Proprietary .8 scene owner. See docs/world-scene.md for retained originals,
// supported file domain and the explicitly unresolved virtual APIs.
struct OctreeLight
{
	DWORD words[16];
};

struct OctreeMaterial
{
	BYTE bytes[196];
};

class OCTREE_Camera;
class OCTREE_Shape;
struct OctreeShapeConfiguration;
class OctreeInput;
namespace JE { namespace Resources { struct ResourceFileHandle; } }
struct OctreeNode;
struct OctreeRay;

struct OctreeSpatialObject
{
	float center[3], extent[3];
	DWORD flags, handle;
	OctreeSpatialObject* previous;
	OctreeSpatialObject* next;
	OctreeNode* owner;
	void* object;
	DWORD references;
};

// Descriptive name for the map vertex streams; original class name unknown.

class OctreeVertexStreams
{
public:
	OctreeVertexStreams(); // 0x4ABE60.
	~OctreeVertexStreams(); // 0x5AD8D0.
	int ReadVertexStreams(FILE* stream, WORD attributes); // 0x5BD840.
	int ReadVertexInput(OctreeInput& stream, WORD attributes); // Same parser, explicit IO callbacks.
	DWORD count;
	float* positions;
	float* normals;
	DWORD* colors;
	float* textureUV;
	float* lightmapUV;

private:
	OctreeVertexStreams(const OctreeVertexStreams&);
	OctreeVertexStreams& operator=(const OctreeVertexStreams&);
};

struct OctreeNode
{
	OctreeNode() // Inlined by 0x5B0AD0 / 0x5AF800.
		: parent(0), value08(0), wordCount(0), indices(0), triangleFlags(0), objects(0)
	{
	}
	~OctreeNode(); // 0x5AD8A0; does not own child nodes.
	int ClassifyObject(const float* center, const float* extent) const; // 0x5BD1A0, ECX node / RET8.
	const DWORD* GetChildOrder(const float* position, DWORD* count) const; //5BCF30, ECX node / RET8.
	int QueryFloorHeight(const OctreeRay& ray, const float* positions,
		float* distance, float* height, DWORD mask, DWORD mode, DWORD* flags) const; //5BD410, RET1C.
	DWORD ordinal;
	OctreeNode* parent;
	DWORD value08;
	float bounds[6];
	DWORD vertexBuffer;
	DWORD flags;
	OctreeNode* children[8];
	DWORD wordCount;
	WORD* indices;
	DWORD* triangleFlags;
	OctreeSpatialObject* objects;
};

struct OctreeNodeReference
{
	OctreeNode* node;
	DWORD unwritten; //+04: packed-word cursor, initialized0 by CullVisible.
};

struct OctreeBatchInfo
{
	DWORD vertexBuffer, material, lightmap, texture, indexCount;
};

class OCTREE;

class OCTREE_Scene
{
public:
	virtual int Release(); // 0x5BD690; self-delete, not a deleting-destructor slot.
	virtual int AdvanceNode(); // 0x5BD6E0.
	virtual int NextBatchInfo(OctreeBatchInfo* output); // 0x5BD720.
	virtual int CopyIndices(WORD* output); // 0x5BD7D0.
	virtual DWORD* CurrentTriangleFlags(); // 0x5BD820.

private:
	friend class OCTREE;
	OCTREE_Scene();
	~OCTREE_Scene();
	OCTREE_Scene(const OCTREE_Scene&);
	OCTREE_Scene& operator=(const OCTREE_Scene&);
	DWORD lightmapAdjustment;
	OctreeBatchInfo current;
	DWORD bucketIndex, nodeIndex, wordCursor;
	OctreeNodeReference** buckets;
};

struct OctreeHandleTable
{
	OctreeHandleTable();
	~OctreeHandleTable();
	int Initialize(DWORD count); // 0x5B9340, real allocation services.
	int Insert(OctreeSpatialObject* object); // 0x5B9410, native thiscall receiver.
	int Remove(OctreeSpatialObject* object); // 0x5B9440, native thiscall receiver.
	DWORD capacity, used;
	DWORD* freeIndices;
	void** objects;
};

struct OctreeLightGrid
{
	DWORD words[4];
	BYTE* data;
};

struct OctreeTraversal
{
	DWORD nextChild, childCount;
	int classification;
	const DWORD* childOrder;
};

// Actual32-byte nonvirtual ray value4F4C00. A single-inheritance member
// pointer at+1C retains the observed ECX receiver and four stack arguments.
struct OctreeRay
{
	typedef int (OctreeRay::*BoundsTest)(const float*, const float*, float*, float*) const;
	int TestBounds(const float* minimum, const float* maximum,
		float* distance, float* point) const; //4ADC20, factored entry-face policy.
	int IntersectDownwardTriangle(const float* a, const float* b, const float* c,
		float* distance, float* height) const; //4AC5E0, RET14.
	void SetDirection(const float* inputDirection); //4ADC90; NULL reselects current signs.
	OctreeRay() //41D030; also inline in4615F0/58AB80. Origin remains unwritten.
	{
		direction.value[0] = 0.0f;
		direction.value[1] = 0.0f;
		direction.value[2] = 0.0f;
		maximumDistance = FLT_MAX;
		testBounds = &OctreeRay::TestBounds;
	}
	OctreeRay(const float* inputOrigin, const float* inputDirection) //4F4C00; also inline in4F4EB0.
	{
		direction.value[0] = direction.value[1] = direction.value[2] = 0.0f;
		maximumDistance = FLT_MAX;
		testBounds = &OctreeRay::TestBounds;
		memcpy(origin, inputOrigin, sizeof(origin));
		memcpy(direction.value, inputDirection, sizeof(direction.value));
	}
	float origin[3];
	Gfx_Vector3f direction; //Actual12B subobject at+C, receiver of5221C0.
	float maximumDistance;
	BoundsTest testBounds;

private:
	int TestBoundsPPP(const float* minimum, const float* maximum,
		float* distance, float* point) const; //4AC810; sign-specialized.
	int TestBoundsPPN(const float* minimum, const float* maximum,
		float* distance, float* point) const; //4ACA90; sign-specialized.
	int TestBoundsPNP(const float* minimum, const float* maximum,
		float* distance, float* point) const; //4ACD10; sign-specialized.
	int TestBoundsPNN(const float* minimum, const float* maximum,
		float* distance, float* point) const; //4ACF90; sign-specialized.
	int TestBoundsNPP(const float* minimum, const float* maximum,
		float* distance, float* point) const; //4AD210; sign-specialized.
	int TestBoundsNPN(const float* minimum, const float* maximum,
		float* distance, float* point) const; //4AD490; sign-specialized.
	int TestBoundsNNP(const float* minimum, const float* maximum,
		float* distance, float* point) const; //4AD720; sign-specialized.
	int TestBoundsNNN(const float* minimum, const float* maximum,
		float* distance, float* point) const; //4AD9A0; sign-specialized.
};

struct OctreeObjectHit
{
	float distance;
	float point[3]; // Original inside-box hit leaves these three words unwritten.
	void* object; // Borrowed actual DynamicObject payload.
	DWORD hit; // Final sentinel is0; successful object record is1.
};

struct OctreeGroundHit
{
	float distance;
	float point[3];
	DWORD unwritten; //+10 is not published by5AEB20.
	DWORD triangleFlags; //+14, different from OctreeObjectHit's sentinel.
};

struct OctreeNodeBounds
{
	DWORD depth;
	const float* bounds; //Borrowed six-float node bounds; NULL alone terminates the list.
};

class OCTREE
{
public:
	virtual int Release(); // 0x5B1330.
	virtual int ClearLights(); // 0x5AD9E0.
	virtual int ClearMaterials(); // 0x5ADA10.
	virtual int ClearTextures(); // 0x5ADA30.
	virtual int ClearVertexAttributes(); // 0x5ADA70; positions survive this call.
	virtual int GetLevelBounds(float* output); // 0x5ADB10; six min/max floats.
	virtual int CreateCamera(float degrees, float aspect, float nearPlane, float farPlane,
		OCTREE_Camera** output); // 0x5B0230.
	virtual int CreateShape(const OctreeShapeConfiguration* configuration, OCTREE_Shape** output); //5ADB40.
	virtual int CreateScene(OCTREE_Scene** output, DWORD mode); // 0x5B0400.
	virtual int GetLights(DWORD* count, OctreeLight** output); // 0x5AE1F0.
	virtual int GetMaterials(DWORD* count, OctreeMaterial** output); // 0x5AE230.
	virtual int GetTextureNames(DWORD* count, char*** output); // 0x5AE250.
	virtual int GetVertexBuffers(DWORD* count, DWORD* mask, OctreeVertexStreams** output); // 0x5AE270.
	virtual int PackVertexStreams(DWORD attributes, const OctreeVertexStreams* source, DWORD* output); // 0x5ADC10.
	virtual int CullVisible(OCTREE_Camera* camera, DWORD limit); // 0x5ADD60; zero means unlimited.
	virtual int RestoreVisibility(OCTREE_Camera* camera); // 0x5AE090.
	virtual int NextVisibleVertexBuffer(DWORD* buffer, DWORD* vertices); // 0x5AE300.
	virtual int NextVisibleMaterial(WORD* material); //5AE490, shared packed-stream cursors.
	virtual int NextVisibleLightmap(WORD* lightmap); //5AE5A0.
	virtual int NextVisibleTexture(WORD* texture,DWORD* indices); //5AE6D0.
	virtual int CopyVisibleIndices(WORD* output); //5AE720, advances current texture.
	virtual int ReadVisibleNodeIDs(DWORD* capacity, DWORD* output); // 0x5AE420.
	virtual int QueryGroundRay(const OctreeRay& ray, OctreeGroundHit* output,
		DWORD mask, DWORD mode); //+58,5AEB20; nearest matching map triangle.
	virtual int QueryFloorHeight(const float* position, float* height, DWORD mask, DWORD mode, DWORD* flags); // 5AECD0.
	virtual int QueryVisibleObjects(DWORD capacity, void** output, OCTREE_Shape* filter, DWORD* more); //5AF630.
	virtual int QueryObjects(const OctreeRay& ray, DWORD capacity, OctreeObjectHit* output); // 0x5AEF00.
	virtual int GetLightGridCell(DWORD column,DWORD row,DWORD* count,WORD* output); //5AFED0.
	virtual int GetLightVisibility(const float* position, DWORD* count, WORD* output); //5AFCE0.
	virtual int AddObject(const float* center, const float* bounds, void* object, DWORD* handle); // 5B0640.
	virtual int InsertObject(DWORD handle); // 5AF160.
	virtual int DetachObject(DWORD handle); //5AF200, keeps handle/reference ownership.
	virtual int RemoveObject(DWORD handle); // 5AF270.
	virtual int MoveObject(DWORD handle, const float* center); // 5AF370.
	virtual int ResizeObject(DWORD handle, const float* bounds); // 5AF4C0.
	virtual int ResetQueryCursor(); // 0x5B0220; only resets spatialCursor.
	virtual int IsObjectVisible(DWORD handle); // 0x5AF710; invalid handle returns2.
	virtual int GetObjectLocalBounds(DWORD handle,float* bounds); //5AF740, sixfloats -extent/+extent.
	virtual int GetObjectPosition(DWORD handle,float* position); //5AF7A0, threecenterfloats.
	virtual int CacheShape(OCTREE_Shape* shape); //5B0730, borrows accepted tree nodes.
	virtual int UpdateShape(OCTREE_Shape* shape); //5B0880, shared selection and leaf map.
	virtual int InvalidateShape(OCTREE_Shape* shape); //5AF7E0, clears matching cached owner only.
	virtual int CompareTopologyIndices(const OCTREE* other); //5AE850;0same,1different.
	virtual int QueryNodeBounds(OCTREE_Camera* camera,DWORD minimumDepth,
		DWORD* maximumDepth,OctreeNodeBounds* output); //5AE950, borrowed bounds/sentinel.

	static int Read8File(FILE* stream, DWORD attributes, OCTREE** output); // 0x5B0AD0.
	static int Read8Input(OctreeInput& stream, DWORD attributes, OCTREE** output);
	static int Read8Resource(JE::Resources::ResourceFileHandle* stream, DWORD attributes, OCTREE** output);
	int ReadTree(FILE* stream); // 0x5AF800.
	int ReadTreeInput(OctreeInput& stream); // Same parser, explicit IO callbacks.

	// Actual scalar/array members; no library object overlays original storage.
	DWORD version, flags, depth, value10, attributeMask;
	DWORD lightCount;
	OctreeLight* lights;
	OctreeLightGrid* lightGrid;
	DWORD materialCount;
	OctreeMaterial* materials;
	DWORD* materialValues; //+2C, visible-material presence flags.
	DWORD value30, textureCount; //+30 material cursor,+34 texture-name count.
	char** textureNames;
	DWORD* textureHandles; //+3C, per-texture index counts, not renderer GPU handles.
	DWORD value40, lightmapCount, firstLightmap; //+40 texture cursor,+44/+48 file counts.
	DWORD* lightmapHandles; //+4C, visible-lightmap presence flags.
	DWORD value50, vertexBufferCount; //+50 lightmap cursor,+54 vertex-buffer count.
	DWORD* indexedNodeCounts;
	OctreeVertexStreams* vertexBuffers;
	OctreeNode root;
	OctreeNode** visibleTreeNodes; // +BC; borrowed camera node list.
	DWORD visibleTreeNodeCount; // +C0.
	DWORD* sceneNodeCounts;
	OctreeNodeReference** visibleBuckets; // +C8; borrowed camera-produced lists.
	DWORD visibleBufferCursor; // +CC.
	OctreeNodeReference* visibleNodes; // +D0; borrowed current list.
	DWORD visibleNodeCursor, visibleNodeCount; // +D4/+D8.
	OctreeHandleTable handles;
	DWORD spatialCursor; // +EC.
	OctreeSpatialObject** visibleObjects; // +F0; borrowed camera reference list.
	OCTREE_Shape* currentShape; //+F4, borrowed active query owner.
	BYTE* scratch;
	OctreeTraversal* traversal;

private:
	explicit OCTREE(DWORD attributes);
	~OCTREE();
	OCTREE(const OCTREE&);
	OCTREE& operator=(const OCTREE&);
};
