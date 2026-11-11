#pragma once

#include <windows.h>
#include "GfxVector3.hpp"
#include <list>
#include <map>

struct OctreeNode;
struct OctreeSpatialObject;
class OCTREE;

//Actual24B owner reached by53B440. Buckets own pointer storage only; all
//OctreeNode values are borrowed. Historical utility/template spelling unknown.
class OctreeShapeNodeCache
{
public:
	OctreeShapeNodeCache(DWORD blockCount,DWORD blockSize); //53B180.
	~OctreeShapeNodeCache(); //53B250.
	DWORD Append(OctreeNode* const& node); //5AFFD0, by-reference pointer value.
	OctreeNode* Node(DWORD index) const { return blocks[index>>shift][index&mask]; }
	void Invalidate() { count=0; blocks[0][0]=NULL; }
	DWORD count,blockCount,mask,shift,blockSize;
	OctreeNode*** blocks;
private:
	OctreeShapeNodeCache(const OctreeShapeNodeCache&);
	OctreeShapeNodeCache& operator=(const OctreeShapeNodeCache&);
};

struct OctreeShapeConfiguration
{
	union { DWORD value0; float first; }; //Circle ignores it; Tri/Pie angle / Rect width.
	float radius;
	DWORD type;
};

typedef std::map<DWORD,std::list<DWORD> > OctreeShapeNodes;

class OCTREE_Shape
{
public:
	virtual int Release(); //53B840, self-delete.
	virtual int SetPosition(const float* position); //53B400.
	virtual int SetTransform(const float* matrix); //53B3C0, translation only.
	virtual int ClassifyObject(const float* center, const float* extent) const = 0;
	virtual int ClassifyBox(const float* bounds) const = 0;
	virtual int ContainsPoint(const float* point) const = 0;
	virtual int QueryObjects(DWORD capacity, void** output, DWORD* more); //53B290.
	virtual OctreeShapeNodes* GetNodes(); //5449D0.
	int EnableNodeCache(DWORD capacity); //53B440;0existing/created,1allocation failure.
	float position[3]; //+04, constructor leaves unwritten.
	DWORD queryCursor; //+10, UpdateShape establishes0.
	OctreeSpatialObject** selectedObjects; //+14, borrowed OCTREE scratch.
	OctreeShapeNodeCache* cachedNodes; //+18, owns buckets, borrows cached tree nodes.
	OctreeShapeNodes nodes; //+1C, buffer index -> leaf node ordinals.
protected:
	OCTREE_Shape();
private:
	friend class OCTREE;
	void AddNode(const OctreeNode* node); //53B9A0.
	OCTREE_Shape(const OCTREE_Shape&);
	OCTREE_Shape& operator=(const OCTREE_Shape&);
};

class OCTREE_Crcl2D : public OCTREE_Shape
{
public:
	OCTREE_Crcl2D(); //52F6B0.
	void SetRadius(float radius); //52EE80.
	virtual int ClassifyObject(const float* center, const float* extent) const; //52F010.
	virtual int ClassifyBox(const float* bounds) const; //52EED0.
	virtual int ContainsPoint(const float* point) const; //52EEA0, XZ only.
	float radius, radiusSquared; //+28/+2C.
};

class OCTREE_Tri2D : public OCTREE_Shape
{
public:
	OCTREE_Tri2D(); //5BBB40, zeroes vectors, leaves normalSigns unwritten.
	void Configure(float angle,float length); //5BB140, degrees.
	virtual int SetTransform(const float* matrix); //5BB9F0, Y position becomes0.
	virtual int ClassifyObject(const float* center,const float* extent) const; //5BB630.
	virtual int ClassifyBox(const float* bounds) const; //5BB2E0.
	virtual int ContainsPoint(const float* point) const; //5BB1D0.
	Gfx_Vector3f localEnd,rotatedEnd; //+28/+34, far-plane offset.
	Gfx_Vector3f localNormals[4],worldNormals[4]; //+40/+70.
	DWORD normalSigns[4]; //+A0, SetTransform establishes these.
};

class OCTREE_Ring2D : public OCTREE_Shape
{
public:
	OCTREE_Ring2D(); //5BA470, scalar fields remain unwritten.
	void Configure(float outerRadius, float innerRadius); //5BA0E0.
	virtual int ClassifyObject(const float* center, const float* extent) const; //5BA2B0.
	virtual int ClassifyBox(const float* bounds) const; //5BA150.
	virtual int ContainsPoint(const float* point) const; //5BA110, XZ only.
	float outerRadius, outerRadiusSquared; //+28/+2C.
	float innerRadius, innerRadiusSquared; //+30/+34.
};

class OCTREE_Pie2D : public OCTREE_Shape
{
public:
	OCTREE_Pie2D(); //5BB010.
	void Configure(float angle, float radius); //5BA540, degrees.
	virtual int SetTransform(const float* matrix); //5BAEE0, Y position becomes0.
	virtual int ClassifyObject(const float* center,const float* extent) const; //5BAA70.
	virtual int ClassifyBox(const float* bounds) const; //5BA690.
	virtual int ContainsPoint(const float* point) const; //5BA5E0.
	float radius, radiusSquared; //+28/+2C; configure supplies both.
	Gfx_Vector3f localNormals[3], worldNormals[3]; //+30/+54, ctor zeros.
	DWORD normalSigns[3]; //+78, established by SetTransform.
};

class OCTREE_Rect2D : public OCTREE_Shape
{
public:
	OCTREE_Rect2D(); //5B9F50.
	void Configure(float width,float length); //5B9490.
	virtual int SetTransform(const float* matrix); //5B9DF0.
	virtual int ClassifyObject(const float* center,const float* extent) const; //5B99E0.
	virtual int ClassifyBox(const float* bounds) const; //5B9660.
	virtual int ContainsPoint(const float* point) const; //5B9510.
	Gfx_Vector3f localStart, rotatedStart, localEnd, rotatedEnd; //+28..57.
	Gfx_Vector3f localNormals[4], worldNormals[4]; //+58/+88, ctor zeros.
	DWORD normalSigns[4]; //+B8, established by SetTransform.
};

//52F700: native status/output factory; failed creation leaves output unwritten.
int CreateOctreeCircle2D(float radius, OCTREE_Shape** output);

//5BA4C0: failed creation leaves output unwritten; radii are configured before publication.
int CreateOctreeRing2D(float outerRadius, float innerRadius, OCTREE_Shape** output);

//5BA030: native status/output factory; failed creation leaves output unwritten.
int CreateOctreeRect2D(float width, float length, OCTREE_Shape** output);
