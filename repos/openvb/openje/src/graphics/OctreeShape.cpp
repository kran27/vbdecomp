#include "OctreeShape.hpp"
#include "Octree.hpp"
#include <d3dx8math.h>
#include <new>
#include <stdexcept>

float GfxDistanceSquaredXZ(const float* first, const float* second);
D3DXVECTOR3 GfxAddVectors(const float* first, const float* second);
D3DXVECTOR3 GfxSubtractVectors(const float* first, const float* second);

OctreeShapeNodeCache::OctreeShapeNodeCache(DWORD blocksCount,DWORD valuesPerBlock)
	: count(0),blockCount(blocksCount),mask(valuesPerBlock-1),blockSize(valuesPerBlock)
{
	DWORD bit;
	for (bit=1;bit<32;++bit)
		if (!(mask>>bit)) break;
	shift=bit;
	for (;bit<32;++bit)
		if (mask>>bit) break;
	if (bit!=32 || (DWORD(1)<<(shift&31))-1!=mask)
		__debugbreak(); //VC7 intrinsic; original53B206 INT3.
	blocks=new OctreeNode**[blockCount];
	for (DWORD index=0;index<blockCount;++index) blocks[index]=NULL;
}
OctreeShapeNodeCache::~OctreeShapeNodeCache()
{
	if (blocks)
	{
		for (DWORD index=0;index<blockCount && blocks[index];++index) delete[] blocks[index];
		delete[] blocks;
	}
}
DWORD OctreeShapeNodeCache::Append(OctreeNode* const& node)
{
	DWORD block=count>>shift,element=count&mask;
	if (!blocks[block]) blocks[block]=new OctreeNode*[blockSize];
	blocks[block][element]=node;
	++count;
	return count - 1;
}

OCTREE_Shape::OCTREE_Shape() : selectedObjects(NULL), cachedNodes(NULL)
{
	//Original52F6B0 leaves position and the cursor unwritten.
}
int OCTREE_Shape::Release()
{
	if (selectedObjects)
		for (DWORD index = 0; selectedObjects[index]; ++index)
		{
			--selectedObjects[index]->references;
			if (!selectedObjects[index]->references) delete selectedObjects[index];
			selectedObjects[index] = NULL;
		}
	delete cachedNodes;
	//This shared Release entry explicitly destroys the nonvirtual base owner
	//then frees storage; circle contributes only its two scalar values.
	this->~OCTREE_Shape();
	::operator delete(this);
	return 0;
}
int OCTREE_Shape::SetPosition(const float* value)
{
	for (int axis = 0; axis < 3; ++axis) position[axis] = value[axis];
	if (cachedNodes)
	{
		cachedNodes->count = 0;
		cachedNodes->blocks[DWORD(0) >> cachedNodes->shift][DWORD(0) & cachedNodes->mask] = NULL;
	}
	return 0;
}
int OCTREE_Shape::SetTransform(const float* matrix)
{
	for (int axis = 0; axis < 3; ++axis) position[axis] = matrix[12+axis];
	if (cachedNodes)
	{
		cachedNodes->count = 0;
		cachedNodes->blocks[DWORD(0) >> cachedNodes->shift][DWORD(0) & cachedNodes->mask] = NULL;
	}
	return 0;
}
OctreeShapeNodes* OCTREE_Shape::GetNodes() { return &nodes; }
int OCTREE_Shape::EnableNodeCache(DWORD capacity)
{
	if (cachedNodes) return 0;
	DWORD blockCount=(capacity>>7)+1;
	cachedNodes=new OctreeShapeNodeCache(blockCount,128);
	return cachedNodes==NULL;
}
void OCTREE_Shape::AddNode(const OctreeNode* node)
{
	if (node && (node->flags & 1)) nodes[node->vertexBuffer].push_back(node->ordinal);
}
int OCTREE_Shape::QueryObjects(DWORD capacity, void** output, DWORD* more)
{
	*more = 0;
	if (!selectedObjects) return 5;
	OctreeShapeNodeCache* cache=cachedNodes;
	if (cache && cache->Node(0) && !queryCursor)
	{
		DWORD nodeIndex=0;
		OctreeNode* node=cache->Node(0);
		while (node)
		{
			for (OctreeSpatialObject* object=node->objects;object;object=object->next)
				if (ClassifyObject(object->center,object->extent))
				{
					selectedObjects[queryCursor++]=object;
					++object->references;
				}
			node=cache->Node(++nodeIndex);
		}
		selectedObjects[queryCursor]=NULL;
		queryCursor=0;
	}
	OctreeSpatialObject* object = selectedObjects[queryCursor];
	DWORD written = 0;
	while (object && written < capacity-1)
	{
		output[written++] = object->object;
		object = selectedObjects[++queryCursor];
	}
	output[written] = NULL;
	if (object) *more = 1;
	return 0;
}

OCTREE_Crcl2D::OCTREE_Crcl2D()
{
}
void OCTREE_Crcl2D::SetRadius(float value)
{
	radius = value;
	radiusSquared = value*value;
}
int OCTREE_Crcl2D::ContainsPoint(const float* point) const
{
	return GfxDistanceSquaredXZ(position, point) <= radiusSquared;
}
int OCTREE_Crcl2D::ClassifyObject(const float* center, const float* extent) const
{
	int candidate = 1;
	float nearest = 0;
	float farthest = 0;
	D3DXVECTOR3 maximum;
	maximum = GfxAddVectors(center,extent);
	D3DXVECTOR3 minimum;
	minimum = GfxSubtractVectors(center,extent);
	if (position[0] < minimum.x)
	{
		float nearDistance = minimum.x-position[0];
		nearest = nearDistance*nearDistance;
		float farDistance = position[0]-maximum.x;
		farthest = float(farDistance*farDistance);
	}
	else if (position[0] > maximum.x)
	{
		float nearDistance = position[0]-maximum.x;
		nearest = nearDistance*nearDistance;
		float farDistance = minimum.x-position[0];
		farthest = float(farDistance*farDistance);
	}
	else if (position[0]-minimum.x > radius
		|| maximum.x-position[0] > radius) candidate = 0;
	if (position[2] < minimum.z)
	{
		float nearDistance = minimum.z-position[2];
		nearest += nearDistance*nearDistance;
		float farDistance = position[2]-maximum.z;
		farthest = float(farDistance*farDistance+farthest);
	}
	else if (position[2] > maximum.z)
	{
		float nearDistance = position[2]-maximum.z;
		nearest += nearDistance*nearDistance;
		float farDistance = minimum.z-position[2];
		farthest = float(farDistance*farDistance+farthest);
	}
	else if (position[2]-minimum.z > radius
		|| maximum.z-position[2] > radius) candidate = 0;
	if (nearest > radiusSquared) return 0;
	if (!candidate) return 1;
	return farthest <= radiusSquared ? 2 : 1;
}
int OCTREE_Crcl2D::ClassifyBox(const float* bounds) const
{
	int candidate = 1;
	float nearest = 0;
	float farthest = 0;
	if (position[0] < bounds[0])
	{
		float nearDistance = bounds[0]-position[0];
		nearest = nearDistance*nearDistance;
		float farDistance = position[0]-bounds[3];
		farthest = float(farDistance*farDistance);
	}
	else if (position[0] > bounds[3])
	{
		float nearDistance = position[0]-bounds[3];
		nearest = nearDistance*nearDistance;
		float farDistance = bounds[0]-position[0];
		farthest = float(farDistance*farDistance);
	}
	else if (position[0]-bounds[0] > radius
		|| bounds[3]-position[0] > radius) candidate = 0;
	if (position[2] < bounds[2])
	{
		float nearDistance = bounds[2]-position[2];
		nearest += nearDistance*nearDistance;
		float farDistance = position[2]-bounds[5];
		farthest = float(farDistance*farDistance+farthest);
	}
	else if (position[2] > bounds[5])
	{
		float nearDistance = position[2]-bounds[5];
		nearest += nearDistance*nearDistance;
		float farDistance = bounds[2]-position[2];
		farthest = float(farDistance*farDistance+farthest);
	}
	else if (position[2]-bounds[2] > radius
		|| bounds[5]-position[2] > radius) candidate = 0;
	if (nearest > radiusSquared) return 0;
	if (!candidate) return 1;
	return farthest <= radiusSquared ? 2 : 1;
}

int CreateOctreeCircle2D(float radius, OCTREE_Shape** output) //52F700.
{
	OCTREE_Crcl2D* circle = new OCTREE_Crcl2D;
	if (!circle) return 1;
	circle->SetRadius(radius);
	*output = circle;
	return 0;
}

int CreateOctreeRing2D(float outerRadius, float innerRadius, OCTREE_Shape** output) //5BA4C0.
{
	OCTREE_Ring2D* ring = new OCTREE_Ring2D;
	if (!ring) return 1;
	ring->Configure(outerRadius, innerRadius);
	*output = ring;
	return 0;
}

int OCTREE::CreateShape(const OctreeShapeConfiguration* configuration, OCTREE_Shape** output)
{
	OCTREE_Shape* shape = NULL;
	switch (configuration->type)
	{
	case 1:
		{
			const int result = CreateOctreeCircle2D(configuration->radius, &shape);
			if (result) return result;
		}
		break;
	case 0:
		{
			if (configuration->first>180.0f) return 2;
			OCTREE_Tri2D* triangle=new(std::nothrow) OCTREE_Tri2D;
			if (!triangle) return 1;
			triangle->Configure(configuration->first,configuration->radius); //5BBC00.
			shape=triangle;
		}
		break;
	case 2:
		{
			if (configuration->first>180.0f) return 2;
			OCTREE_Pie2D* pie=new(std::nothrow) OCTREE_Pie2D;
			if (!pie) return 1;
			pie->Configure(configuration->first,configuration->radius);
			shape=pie;
		}
		break;
	case 3:
		{
			//5ADBBD reverses the two configuration fields for5BA4C0.
			const int result = CreateOctreeRing2D(configuration->radius, configuration->first, &shape);
			if (result) return result;
		}
		break;
	case 4:
		{
			const int result = CreateOctreeRect2D(configuration->first, configuration->radius, &shape);
			if (result) return result;
		}
		break;
	default: break; //The original invalid-type result publishesNULL with return0.
	}
	*output = shape;
	return 0;
}
int OCTREE::InvalidateShape(OCTREE_Shape* shape)
{
	if (currentShape == shape) currentShape = NULL;
	return 0;
}
int OCTREE::CacheShape(OCTREE_Shape* shape) //5B0730.
{
	OctreeNode* node=&root;
	int level=0,classification=0;
	if (!shape) return 2;
	if (shape->EnableNodeCache(value10)) return 1;
	while (node)
	{
		if (level<0) break;
		LONG child=traversal[level].nextChild;
		if (!child)
		{
			if (classification!=2)
			{
				classification=shape->ClassifyBox(node->bounds);
				if (!classification) child=8;
			}
			traversal[level].classification=classification;
			if (classification)
			{
				OctreeNode* accepted=node;
				if (shape->cachedNodes) shape->cachedNodes->Append(accepted);
			}
		}
		OctreeNode* descendant=NULL;
		if (!(node->flags&1))
			while (child<8)
			{
				descendant=node->children[child++];
				if (descendant) break;
			}
		if (descendant)
		{
			traversal[level].nextChild=child;
			node=descendant;
			++level;
		}
		else
		{
			traversal[level].nextChild=0;
			node=node->parent;
			--level;
			classification=traversal[level].classification;
		}
	}
	OctreeNode* end=NULL;
	if (shape->cachedNodes) shape->cachedNodes->Append(end);
	return 0;
}
int OCTREE::UpdateShape(OCTREE_Shape* shape)
{
	if (!shape) return 2;
	if (currentShape) currentShape->selectedObjects = NULL;
	currentShape = shape;
	OctreeSpatialObject** selected = reinterpret_cast<OctreeSpatialObject**>(scratch);
	shape->selectedObjects = selected;
	shape->queryCursor = 0;
	//5B08EF keeps the existing selection/leaf map when a cached prefix exists;
	//QueryObjects lazily rebuilds object references. Do not add eager cleanup.
	if (shape->cachedNodes && shape->cachedNodes->Node(0)) return 0;
	shape->nodes.clear();
	for (DWORD index = 0; selected[index]; ++index)
	{
		OctreeSpatialObject* object = selected[index];
		if (!--object->references) delete object;
		selected[index] = NULL;
	}
	DWORD written = 0;
	OctreeNode* node = &root;
	int level = 0, classification = 0;
	do
	{
		OctreeTraversal& frame = traversal[level];
		DWORD child = frame.nextChild;
		if (!child)
		{
			if (classification != 2)
			{
				classification = shape->ClassifyBox(node->bounds);
				if (!classification) child = 8;
			}
			frame.classification = classification;
			if (classification)
			{
				if (node->flags & 1) shape->AddNode(node);
				for (OctreeSpatialObject* object = node->objects; object; object = object->next)
					if (classification == 2 || shape->ClassifyObject(object->center,object->extent))
					{
						selected[written++] = object;
						++object->references;
					}
			}
		}
		if (!(node->flags & 1))
			while (child < 8)
			{
				OctreeNode* descendant = node->children[child++];
				if (descendant)
				{
					frame.nextChild = child;
					node = descendant;
					++level;
					goto NextShapeNode;
				}
			}
		frame.nextChild = 0;
		node = node->parent;
		--level;
		if (level >= 0) classification = traversal[level].classification;
		//Original has one unused traversal[-8] load at the terminal root exit.
NextShapeNode:;
	}
	while (node && level >= 0);
	selected[written] = NULL;
	return 0;
}
