#include "Octree.hpp"
#include <new>
#include <d3dx8math.h>

D3DXVECTOR3 GfxAddVectors(const float* first, const float* second);
D3DXVECTOR3 GfxSubtractVectors(const float* first, const float* second);
D3DXVECTOR3 GfxScaleVector(float scalar, const D3DXVECTOR3& input);


OctreeNode::~OctreeNode()
{
	if (indices) delete[] indices;
	if (triangleFlags) delete[] triangleFlags;
}

int OctreeNode::ClassifyObject(const float* center, const float* extent) const
{
	D3DXVECTOR3 lower, upper;
	lower = GfxSubtractVectors(center, extent);
	upper = GfxAddVectors(center, extent);
	if (!(lower.x < bounds[0]) && !(lower.y < bounds[1]) && !(lower.z < bounds[2])
		&& !(upper.x > bounds[3]) && !(upper.y > bounds[4]) && !(upper.z > bounds[5])) return 2;
	return 0;
}

OctreeHandleTable::OctreeHandleTable()
	: capacity(0), freeIndices(0), objects(0)
{
}

int OctreeHandleTable::Insert(OctreeSpatialObject* object)
{
	if (used == capacity) return 50;
	object->handle = freeIndices[used];
	++used;
	objects[object->handle] = object;
	return 0;
}

int OctreeHandleTable::Remove(OctreeSpatialObject* object)
{
	if (!object || object->handle >= capacity || !objects[object->handle]) return 2;
	freeIndices[--used] = object->handle;
	objects[object->handle] = NULL;
	return 0;
}

int OctreeHandleTable::Initialize(DWORD count)
{
	if (freeIndices || objects) return 4;
	capacity = count;
	used = 0;
	freeIndices = static_cast<DWORD*>(::operator new(count * sizeof(DWORD)));
	if (freeIndices)
	{
		for (DWORD index = 0; index < count; ++index) freeIndices[index] = index;
		objects = static_cast<void**>(::operator new(count * sizeof(void*)));
		if (objects)
		{
			for (DWORD index = 0; index < count; ++index) objects[index] = NULL;
			return 0;
		}
	}
	if (freeIndices)
	{
		::operator delete(freeIndices);
		freeIndices = NULL;
	}
	if (objects)
	{
		::operator delete(objects);
		objects = NULL;
	}
	return 1;
}

OctreeHandleTable::~OctreeHandleTable()
{
	if (freeIndices) ::operator delete(freeIndices);
	if (objects)
	{
		for (DWORD index = 0; index < capacity; ++index)
		{
			if (objects[index]) ::operator delete(objects[index]);
		}
		::operator delete(objects);
	}
}

OCTREE::OCTREE(DWORD attributes)
	: attributeMask(attributes), lightGrid(0), visibleObjects(0), currentShape(0)
{
	handles.Initialize(1024);
}

OCTREE::~OCTREE()
{
}

int OCTREE::Release()
{
	ClearLights();
	if (lightGrid)
	{
		delete[] lightGrid->data;
		delete lightGrid;
		lightGrid = NULL;
	}
	if (materials) ClearMaterials();
	delete[] materialValues;
	if (textureNames) ClearTextures();
	delete[] textureHandles;
	delete[] lightmapHandles;
	if (vertexBuffers) ClearVertexAttributes();
	delete[] vertexBuffers;
	delete[] indexedNodeCounts;
	delete[] scratch;
	for (DWORD index = 0; index < depth; ++index) traversal[index].nextChild = 0;
	OctreeNode* node = &root;
	int level = 0;
	do
	{
		while (traversal[level].nextChild < 8)
		{
			OctreeNode* child = node->children[traversal[level].nextChild++];
			if (child)
			{
				node = child;
				++level;
				goto NextNode;
			}
		}
		traversal[level].nextChild = 0;
		{
			OctreeNode* parent = node->parent;
			if (parent) delete node;
			node = parent;
		}
		--level;
	NextNode:;
	}
	while (node);
	delete[] sceneNodeCounts;
	delete[] traversal;
	this->~OCTREE();
	::operator delete(this);
	return 0;
}

int OCTREE::ClearLights()
{
	if (lights) delete[] lights;
	lights = NULL;
	lightCount = 0;
	return 0;
}

int OCTREE::ClearMaterials()
{
	if (materials) delete[] materials;
	materials = NULL;
	return 0;
}

int OCTREE::ClearTextures()
{
	if (textureNames)
	{
		for (DWORD index = textureCount; index; --index) delete[] textureNames[index - 1];
		delete[] textureNames;
		textureNames = NULL;
	}
	return 0;
}

int OCTREE::ClearVertexAttributes()
{
	for (DWORD index = 0; index < vertexBufferCount; ++index)
	{
		if (vertexBuffers[index].normals) delete[] vertexBuffers[index].normals;
		vertexBuffers[index].normals = NULL;
		if (vertexBuffers[index].colors) delete[] vertexBuffers[index].colors;
		vertexBuffers[index].colors = NULL;
		if (vertexBuffers[index].textureUV) delete[] vertexBuffers[index].textureUV;
		vertexBuffers[index].textureUV = NULL;
		if (vertexBuffers[index].lightmapUV) delete[] vertexBuffers[index].lightmapUV;
		vertexBuffers[index].lightmapUV = NULL;
	}
	return 0;
}

int OCTREE::GetLights(DWORD* count, OctreeLight** output)
{
	if (lights)
	{
		*count = lightCount;
		*output = lights;
		return 0;
	}
	*count = 0;
	*output = NULL;
	return 6;
}

int OCTREE::GetMaterials(DWORD* count, OctreeMaterial** output)
{
	*count = materialCount;
	*output = materials;
	return 0;
}

int OCTREE::GetTextureNames(DWORD* count, char*** output)
{
	*count = textureCount;
	*output = textureNames;
	return 0;
}

int OCTREE::GetVertexBuffers(DWORD* count, DWORD* mask, OctreeVertexStreams** output)
{
	if (!vertexBuffers) return 1;
	*mask = 0;
	if (vertexBuffers[0].positions) *mask = 0x100;
	if (vertexBuffers[0].normals) *mask |= 0x200;
	if (vertexBuffers[0].colors) *mask |= 0x400;
	if (vertexBuffers[0].textureUV) *mask |= 0x800;
	if (vertexBuffers[0].lightmapUV) *mask |= 0x1000;
	*count = vertexBufferCount;
	*output = vertexBuffers;
	return 0;
}

namespace
{
	//Native initialized writable tables700048/700058/700098; returned rows are borrowed.
	DWORD nodeBinaryOrder[2][2] = {{0,1},{1,0}};
	DWORD nodeQuadOrder[4][4] = {
		{0,1,3,2},{1,2,0,3},{2,3,1,0},{3,0,2,1}};
	DWORD nodeOctantOrder[8][8] = {
		{0,1,3,2,4,5,7,6},{4,5,7,6,0,1,3,2},
		{1,2,0,3,5,6,4,7},{5,6,4,7,1,2,0,3},
		{2,3,1,0,6,7,5,4},{6,7,5,4,2,3,1,0},
		{3,0,2,1,7,4,6,5},{7,4,6,5,3,0,2,1}};
}

const DWORD* OctreeNode::GetChildOrder(const float* position, DWORD* count) const //5BCF30.
{
	D3DXVECTOR3 sum, midpoint;
	sum = GfxAddVectors(bounds + 3, bounds);
	midpoint = GfxScaleVector(0.5f, sum);
	switch (flags & 0xE)
	{
	case 2:
		*count = 2;
		return position[0] < midpoint.x ? nodeBinaryOrder[0] : nodeBinaryOrder[1];
	case 4:
		*count = 2;
		return position[1] < midpoint.y ? nodeBinaryOrder[0] : nodeBinaryOrder[1];
	case 8:
		*count = 2;
		return position[2] < midpoint.z ? nodeBinaryOrder[0] : nodeBinaryOrder[1];
	case 6:
		*count = 4;
		if (position[0] < midpoint.x)
			return position[1] < midpoint.y ? nodeQuadOrder[0] : nodeQuadOrder[1];
		return position[1] < midpoint.y ? nodeQuadOrder[3] : nodeQuadOrder[2];
	case 10:
		*count = 4;
		if (position[0] < midpoint.x)
			return position[2] < midpoint.z ? nodeQuadOrder[0] : nodeQuadOrder[1];
		return position[2] < midpoint.z ? nodeQuadOrder[3] : nodeQuadOrder[2];
	case 12:
		*count = 4;
		if (position[1] < midpoint.y)
			return position[2] < midpoint.z ? nodeQuadOrder[0] : nodeQuadOrder[3];
		return position[2] < midpoint.z ? nodeQuadOrder[1] : nodeQuadOrder[2];
	default:
		*count = 8;
		if (position[1] < midpoint.y)
		{
			if (position[0] < midpoint.x)
				return position[2] < midpoint.z ? nodeOctantOrder[0] : nodeOctantOrder[2];
			return position[2] < midpoint.z ? nodeOctantOrder[6] : nodeOctantOrder[4];
		}
		if (position[0] < midpoint.x)
			return position[2] < midpoint.z ? nodeOctantOrder[1] : nodeOctantOrder[3];
		return position[2] < midpoint.z ? nodeOctantOrder[7] : nodeOctantOrder[5];
	}
}
