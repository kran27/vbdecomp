#include "Octree.hpp"
#include "OctreeShape.hpp"
#include <d3dx8math.h>
#include <float.h>
#include <new>
#include <stdexcept>
#include <string.h>

D3DXVECTOR3 GfxSubtractVectors(const float* first, const float* second);

namespace
{
	OctreeSpatialObject* FindObject(OctreeHandleTable& table, DWORD handle)
	{
		return handle < table.capacity ? static_cast<OctreeSpatialObject*>(table.objects[handle]) : NULL;
	}
	bool Contains(const OctreeNode& node, const OctreeSpatialObject& object)
	{
		return node.ClassifyObject(object.center, object.extent) == 2;
	}
	OctreeNode* Descend(OctreeNode* node, const OctreeSpatialObject& object)
	{
		while (!(node->flags & 1))
		{
			int index = 0;
			for (; index < 8; ++index)
				if (node->children[index] && Contains(*node->children[index], object)) break;
			if (index == 8) break;
			node = node->children[index];
		}
		return node;
	}
	void Link(OctreeNode& node, OctreeSpatialObject& object)
	{
		if (node.objects) node.objects->previous = &object;
		object.next = node.objects;
		node.objects = &object;
		object.owner = &node;
	}
	void Unlink(OctreeSpatialObject& object)
	{
		if (object.previous) object.previous->next = object.next;
		else if (object.owner) object.owner->objects = object.next;
		if (object.next) object.next->previous = object.previous;
	}
	int Reinsert(OctreeNode& root, OctreeSpatialObject& object)
	{
		OctreeNode* node = object.owner ? object.owner : &root;
		Unlink(object);
		object.owner = NULL;
		object.next = object.previous = NULL;
		while (node && !Contains(*node, object)) node = node->parent;
		if (!node)
		{
			object.flags |= 4;
			Link(root, object);
			return 20;
		}
		Link(*Descend(node, object), object);
		return 0;
	}


}

int OctreeRay::IntersectDownwardTriangle(const float* a, const float* b, const float* c,
	float* distance, float* height) const //4AC5E0, actual receiver and five stack arguments.
{
	float bx = b[0]-a[0], bz = b[2]-a[2], cx = c[0]-a[0], cz = c[2]-a[2];
	double determinant = double(cx)*bz - double(cz)*bx;
	float storedDeterminant = static_cast<float>(determinant);
	if (!(determinant > 0.000099999997f) && !(storedDeterminant < -0.000099999997f)) return 0;
	float x = origin[0]-a[0], y = origin[1]-a[1], z = origin[2]-a[2];
	float u = static_cast<float>(double(z)*cx - double(x)*cz);
	double cross = double(z)*bx - double(x)*bz;
	float storedCross = static_cast<float>(cross);
	double v = -cross;
	if (determinant > 0.000099999997f)
	{
		if (!(u >= 0.0f && u <= storedDeterminant && v >= 0.0 && u+v <= storedDeterminant)) return 0;
	}
	else if (!(u <= 0.0f && u >= storedDeterminant && v <= 0.0 && u+v >= storedDeterminant)) return 0;
	double by = double(b[1])-a[1];
	*distance = static_cast<float>(((double(c[1])-a[1])*storedCross
		+ (by*x-double(y)*bx)*cz + (double(y)*bz-by*z)*cx) * (1.0/storedDeterminant));
	*height = origin[1]-*distance;
	return 1;
}

int OctreeNode::QueryFloorHeight(const OctreeRay& ray, const float* positions,
	float* distance, float* height, DWORD mask, DWORD mode, DWORD* outputFlags) const //5BD410.
{
	if (!indices) return 0;
	*distance = ray.maximumDistance;
	DWORD cursor = 0, triangle = 0;
	while (cursor < wordCount)
	{
		WORD marker = indices[cursor];
		if (marker & 0xC000) { cursor += (marker & 0x8000) ? 4 : 2; continue; }
		DWORD count = indices[cursor+1];
		cursor += 2;
		DWORD end = cursor + count;
		while (cursor < end)
		{
			const float* a = positions + 3*indices[cursor];
			const float* b = positions + 3*indices[cursor+1];
			const float* c = positions + 3*indices[cursor+2];
			DWORD selectedFlags = triangleFlags[triangle++];
			cursor += 3;
			switch (mode)
			{
			case 0: if (mask && !(selectedFlags & mask)) continue; break;
			case 1: if (!(selectedFlags & mask)) continue; break;
			case 2: if ((selectedFlags & mask) != mask) continue; break;
			case 3: if (selectedFlags & mask) continue; break;
			case 4: if ((selectedFlags & mask) == mask) continue; break;
			default: break;
			}
			if ((ray.origin[1] <= a[1] && ray.origin[1] <= b[1] && ray.origin[1] <= c[1])
				|| (ray.origin[0] < a[0] && ray.origin[0] < b[0] && ray.origin[0] < c[0])
				|| (ray.origin[0] > a[0] && ray.origin[0] > b[0] && ray.origin[0] > c[0])
				|| (ray.origin[2] < a[2] && ray.origin[2] < b[2] && ray.origin[2] < c[2])
				|| (ray.origin[2] > a[2] && ray.origin[2] > b[2] && ray.origin[2] > c[2])) continue;
			float candidateDistance, candidateHeight;
			if (ray.IntersectDownwardTriangle(a,b,c,&candidateDistance,&candidateHeight) && candidateDistance < *distance)
			{
				*distance = candidateDistance;
				*height = candidateHeight;
				if (outputFlags) *outputFlags = this->triangleFlags[triangle - 1];
			}
		}
		cursor += cursor & 1;
	}
	return *distance != ray.maximumDistance;
}

int OCTREE::AddObject(const float* center, const float* bounds, void* input, DWORD* handle)
{
	OctreeSpatialObject* object = new OctreeSpatialObject;
	if (!object) return static_cast<int>(reinterpret_cast<INT_PTR>(center)); // Original5B0703.
	object->extent[0] = 0;
	object->extent[1] = 0;
	object->extent[2] = 0;
	memcpy(object->center, center, sizeof(object->center));
	D3DXVECTOR3 difference = GfxSubtractVectors(bounds + 3, bounds);
	Gfx_Vector3f scaleInput;
	memcpy(scaleInput.value, &difference, sizeof(scaleInput));
	Gfx_Vector3f extent = scaleInput.Scale(0.5f);
	memcpy(object->extent, extent.value, sizeof(object->extent));
	object->object = input;
	object->flags = 0;
	object->handle = DWORD(-1);
	object->previous = NULL;
	object->next = NULL;
	object->references = 1;
	int status = handles.Insert(object);
	if (status)
	{
		handles.Remove(object); // Original failure prefix calls5B9440 even with handle-1.
		delete object;
		return status;
	}
	*handle = object->handle;
	return 0;
}

int OCTREE::InsertObject(DWORD handle)
{
	OctreeSpatialObject* object = FindObject(handles,handle);
	if (!object) return 2;
	if (!(object->flags & 1))
	{
		object->flags |= 1;
		OctreeNode* node = &root;
		if (node)
		{
			for (;;)
			{
				if (node->flags & 1) break;
				int index = 0;
				OctreeNode** child = node->children;
				while (!*child || (*child)->ClassifyObject(object->center,object->extent) != 2)
				{
					++index;
					++child;
					if (index >= 8) goto LinkInsertedObject;
				}
				OctreeNode* descendant = node->children[index];
				if (!descendant) break;
				node = descendant;
			}
LinkInsertedObject:
			if (node->objects) node->objects->previous = object;
			object->next = node->objects;
			node->objects = object;
		}
		object->owner = node;
	}
	return 0;
}

int OCTREE::RemoveObject(DWORD handle)
{
	OctreeSpatialObject* object = FindObject(handles,handle);
	if (!object) return 2;
	handles.Remove(object);
	--object->references;
	if (object->flags & 1) { object->flags &= ~1u; Unlink(*object); }
	DWORD references = object->references;
	if (references)
	{
		OctreeSpatialObject** selected = reinterpret_cast<OctreeSpatialObject**>(scratch);
		DWORD index = 0;
		OctreeSpatialObject** cursor = selected;
		while (*cursor && *cursor != object) { ++cursor; ++index; }
		if (selected[index])
		{
			object->references = references - 1;
			do
			{
				reinterpret_cast<OctreeSpatialObject**>(scratch)[index] = reinterpret_cast<OctreeSpatialObject**>(scratch)[index+1];
			}
			while (reinterpret_cast<OctreeSpatialObject**>(scratch)[index++]);
			if (currentShape) --currentShape->queryCursor; //5AF354, including original DWORD wrap.
		}
	}
	if (!object->references) delete object;
	return 0;
}

int OCTREE::DetachObject(DWORD handle)
{
	OctreeSpatialObject* object=FindObject(handles,handle);
	if (!object) return 2;
	if (object->flags&1)
	{
		object->flags&=~3u;
		Unlink(*object);
		object->previous=NULL;
		object->next=NULL;
		object->owner=NULL;
	}
	return 0;
}

int OCTREE::MoveObject(DWORD handle, const float* center)
{
	OctreeSpatialObject* object = FindObject(handles,handle);
	if (!object) return 2;
	for (int axis = 0; axis < 3; ++axis) object->center[axis] = center[axis];
	if (!(object->flags & 1)) return 0;
	OctreeNode* node = object->owner;
	if (!node) node = &root;
	if (object->previous) object->previous->next = object->next;
	else if (object->owner) node->objects = object->next;
	if (object->next) object->next->previous = object->previous;
	object->owner = NULL;
	object->next = NULL;
	object->previous = NULL;
	for (;;)
	{
		if (node->ClassifyObject(object->center,object->extent) == 2) break;
		node = node->parent;
		if (!node) break;
	}
	if (!node)
	{
		object->flags |= 4;
		OctreeNode* rootNode = &root;
		if (rootNode->objects) rootNode->objects->previous = object;
		object->next = rootNode->objects;
		rootNode->objects = object;
		object->owner = rootNode;
		return 20;
	}
	for (;;)
	{
		if (node->flags & 1) break;
		int index = 0;
		OctreeNode** child = node->children;
		while (!*child || (*child)->ClassifyObject(object->center,object->extent) != 2)
		{
			++index;
			++child;
			if (index >= 8) goto LinkMovedObject;
		}
		OctreeNode* descendant = node->children[index];
		if (!descendant) break;
		node = descendant;
	}
LinkMovedObject:
	if (node->objects) node->objects->previous = object;
	object->next = node->objects;
	node->objects = object;
	object->owner = node;
	return 0;
}

int OCTREE::ResizeObject(DWORD handle, const float* bounds)
{
	OctreeSpatialObject* object = FindObject(handles,handle);
	if (!object) return 2;
	for (int axis = 0; axis < 3; ++axis)
	{
		float difference = bounds[axis+3]-bounds[axis];
		object->extent[axis] = difference*0.5f;
	}
	return (object->flags & 1) ? Reinsert(root,*object) : 0;
}

int OCTREE::GetObjectLocalBounds(DWORD handle,float* bounds)
{
	OctreeSpatialObject* object=FindObject(handles,handle);
	if (!object) return 2;
	bounds[0]=-object->extent[0];
	bounds[1]=-object->extent[1];
	bounds[2]=-object->extent[2];
	bounds[3]=object->extent[0];
	bounds[4]=object->extent[1];
	bounds[5]=object->extent[2];
	return 0;
}

int OCTREE::GetObjectPosition(DWORD handle,float* position)
{
	OctreeSpatialObject* object=FindObject(handles,handle);
	if (!object) return 2;
	position[0]=object->center[0];
	position[1]=object->center[1];
	position[2]=object->center[2];
	return 0;
}

int OCTREE::QueryFloorHeight(const float* position, float* height, DWORD mask, DWORD mode, DWORD* flags)
{
	const float zeroDirection[3] = {0.0f, 0.0f, 0.0f};
	OctreeRay ray(position, zeroDirection);
	OctreeNode* node = &root;
	int level = 0;
	float best = FLT_MAX, bestHeight;
	DWORD bestFlags;
	do
	{
		OctreeTraversal& frame = traversal[level];
		DWORD next = frame.nextChild;
		if (!next)
		{
			if (position[1] <= node->bounds[1] || position[0] < node->bounds[0]
				|| position[2] < node->bounds[2] || position[0] > node->bounds[3] || position[2] > node->bounds[5]) next = 8;
			else if ((node->flags & 1) && double(position[1])-node->bounds[4] < best)
			{
				float distance, candidateHeight;
				DWORD candidateFlags;
				if (node->QueryFloorHeight(ray,vertexBuffers[node->vertexBuffer].positions,
					&distance,&candidateHeight,mask,mode,&candidateFlags) && distance < best)
				{
					best = distance;
					bestHeight = candidateHeight;
					bestFlags = candidateFlags;
				}
			}
			if (!(node->flags & 1)) frame.childOrder = node->GetChildOrder(position,&frame.childCount);
		}
		if (!(node->flags & 1))
		{
			while (next < frame.childCount)
			{
				OctreeNode* child = node->children[frame.childOrder[next++]];
				if (child)
				{
					frame.nextChild = next;
					node = child;
					++level;
					goto NextNode;
				}
			}
		}
		frame.nextChild = 0;
		node = node->parent;
		--level;
	NextNode:;
	} while (node);
	if (best == FLT_MAX) return 0;
	*height = bestHeight;
	if (flags) *flags = bestFlags;
	return 1;
}

int OCTREE::GetLightGridCell(DWORD column, DWORD row, DWORD* count, WORD* output) // 5AFED0.
{
	*count = 0;
	DWORD cell = column + row * lightGrid->words[0];
	const BYTE* data = lightGrid->data;
	const BYTE* directory = data + (cell >> 5) * 8;
	DWORD wordIndex, bitmap;
	memcpy(&wordIndex, directory, sizeof(wordIndex));
	wordIndex >>= 1;
	memcpy(&bitmap, directory + 4, sizeof(bitmap));
	DWORD bit = cell & 31;
	if (!(bitmap & (0x80000000UL >> bit))) return 0;

	WORD value;
	memcpy(&value, data + wordIndex * 2, sizeof(value));
	WORD tag = value & 0x8000;
	DWORD mask = 0x80000000UL;
	for (DWORD preceding = 0; preceding < bit; ++preceding)
	{
		if (bitmap & mask)
		{
			do
			{
				++wordIndex;
				memcpy(&value, data + wordIndex * 2, sizeof(value));
			}
			while ((value & 0x8000) == tag);
			tag ^= 0x8000;
		}
		mask >>= 1;
	}
	memcpy(&value, data + wordIndex * 2, sizeof(value));
	do
	{
		output[*count] = value & 0x7FFF;
		++*count;
		++wordIndex;
		// Original reloads the grid owner/data after publishing each ID and count.
		memcpy(&value, lightGrid->data + wordIndex * 2, sizeof(value));
	}
	while ((value & 0x8000) == tag);
	return 0;
}

int OCTREE::CompareTopologyIndices(const OCTREE* other) // 5AE850.
{
	OctreeNode* node = &root;
	const OctreeNode* otherNode = &other->root;
	int level = 0;
	do
	{
		OctreeTraversal& frame = traversal[level];
		DWORD next = frame.nextChild;
		OctreeNode* child = NULL;
		const OctreeNode* otherChild = NULL;
		while (next < 8)
		{
			child = node->children[next];
			otherChild = otherNode->children[next];
			if (child ? !otherChild : otherChild != NULL) return 1;
			++next;
			if (child) break;
		}
		if (child)
		{
			frame.nextChild = next;
			node = child;
			otherNode = otherChild;
			++level;
		}
		else
		{
			DWORD wordCount = node->wordCount;
			if (wordCount != otherNode->wordCount) return 1;
			if (wordCount)
			{
				const WORD* indices = node->indices;
				const WORD* otherIndices = otherNode->indices;
				for (DWORD index = 0; index < wordCount; ++index)
					if (indices[index] != otherIndices[index]) return 1;
			}
			frame.nextChild = 0;
			node = node->parent;
			otherNode = otherNode->parent;
			--level;
		}
	}
	while (node);
	return 0;
}
