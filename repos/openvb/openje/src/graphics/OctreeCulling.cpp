#include "OctreeCamera.hpp"
#include <new>

namespace
{


	void ReleaseReferences(OctreeSpatialObject** objects)
	{
		for (DWORD index = 0; objects[index]; ++index)
		{
			OctreeSpatialObject* object = objects[index];
			if (--object->references) object->flags &= ~2u;
			else ::operator delete(object);
			objects[index] = NULL;
		}
	}
}

int OCTREE::GetLevelBounds(float* output)
{
	output[3] = root.bounds[3]; output[4] = root.bounds[4]; output[5] = root.bounds[5];
	output[0] = root.bounds[0]; output[1] = root.bounds[1]; output[2] = root.bounds[2];
	return 0;
}

int OCTREE::CullVisible(OCTREE_Camera* camera, DWORD limit)
{
	if (!limit) limit = 0xFFFFFFFF;
	visibleTreeNodes = camera->nodes;
	DWORD mask = camera->mask;
	visibleBuckets = camera->buckets;
	visibleObjects = camera->objects;
	spatialCursor = 0;
	ReleaseReferences(visibleObjects);
	spatialCursor = 0;
	visibleTreeNodes[0] = NULL;
	for (DWORD index = 0; index < vertexBufferCount; ++index)
	{
		visibleBuckets[index][0].node = NULL;
		sceneNodeCounts[index] = 0;
	}
	visibleTreeNodeCount = 0;
	visibleNodeCount = 0;
	OctreeNode* node = &root;
	int level = 0, classification = 0;
	do
	{
		OctreeTraversal& frame = traversal[level];
		DWORD next = frame.nextChild;
		if (visibleNodeCount < limit)
		{
			if (!next)
			{
				if (!(node->flags & 1)) frame.childOrder = node->GetChildOrder(&camera->position.x, &frame.childCount);
				if (classification != 2)
				{
					classification = camera->ClassifyBox(node->bounds);
					if (!classification) next = 8;
				}
				frame.classification = classification;
				if (classification)
				{
					if (mask & node->flags)
					{
						DWORD buffer = node->vertexBuffer;
						OctreeNodeReference* reference = visibleBuckets[buffer] + sceneNodeCounts[buffer];
						reference->node = node;
						reference->unwritten = 0;
						(reference + 1)->node = NULL;
						++sceneNodeCounts[buffer];
						++visibleNodeCount;
					}
					visibleTreeNodes[visibleTreeNodeCount++] = node;
					visibleTreeNodes[visibleTreeNodeCount] = NULL;
					for (OctreeSpatialObject* object = node->objects; object; object = object->next)
					{
						if (classification == 2 || camera->ClassifySpatialBox(
							reinterpret_cast<D3DXVECTOR3*>(object->center),
							reinterpret_cast<D3DXVECTOR3*>(object->extent)))
						{
							object->flags |= 2;
							++object->references;
							visibleObjects[spatialCursor++] = object;
						}
					}
				}
			}
		}
		else next = 8;
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
		// Original5AE044 reads the unused prior-frame word even at root (-8
		// before its allocation). Omit only that terminal unused load in native
		// source; no sentinel row or initialized original byte is invented.
		if (level) classification = traversal[level - 1].classification;
		node = node->parent;
		--level;
	NextNode:;
	}
	while (node);
	visibleObjects[spatialCursor] = NULL;
	spatialCursor = 0;
	visibleBufferCursor = 0;
	return visibleNodeCount;
}

int OCTREE::RestoreVisibility(OCTREE_Camera* camera)
{
	visibleTreeNodes = camera->nodes;
	visibleBuckets = camera->buckets;
	visibleObjects = camera->objects;
	if (visibleBufferCursor || spatialCursor)
	{
		for (DWORD index = 0; visibleObjects[index]; ++index)
		{
			--visibleObjects[index]->references;
			OctreeSpatialObject* object = visibleObjects[index];
			if (!object->references) ::operator delete(object);
			else object->flags &= ~2u;
			visibleObjects[index] = NULL;
		}
		spatialCursor = 0;
		for (DWORD buffer = 0; buffer < vertexBufferCount; ++buffer)
		{
			for (OctreeNodeReference* reference = visibleBuckets[buffer]; reference->node;)
				(reference++)->unwritten = 0;
		}
		for (DWORD index = 0; visibleTreeNodes[index]; ++index)
		{
			for (OctreeSpatialObject* object = visibleTreeNodes[index]->objects; object; object = object->next)
			{
				if (camera->ClassifySpatialBox(reinterpret_cast<D3DXVECTOR3*>(object->center),
					reinterpret_cast<D3DXVECTOR3*>(object->extent)))
				{
					object->flags |= 2;
					++object->references;
					visibleObjects[spatialCursor++] = object;
				}
			}
		}
		visibleObjects[spatialCursor] = NULL;
		spatialCursor = 0;
		visibleBufferCursor = 0;
	}
	return visibleNodeCount;
}

int OCTREE::QueryNodeBounds(OCTREE_Camera* camera, DWORD minimumDepth,
	DWORD* maximumDepth, OctreeNodeBounds* output)
{
	DWORD level = 0, outputCount = 0, classification = 0;
	if (*maximumDepth >= depth) *maximumDepth = depth - 1;
	if (minimumDepth > *maximumDepth) minimumDepth = *maximumDepth;
	DWORD mask;
	if (camera) mask = camera->mask;
	OctreeNode* node = &root;
	do
	{
		DWORD next = traversal[level].nextChild;
		if (!next)
		{
			// Even a leaf initializes the original ordering scratch. Without a
			// camera (or on a leaf), its reference position is the root minimum.
			const D3DXVECTOR3& position = (node->flags & 1) || !camera
				? *reinterpret_cast<const D3DXVECTOR3*>(root.bounds) : camera->position;
			const DWORD* order = node->GetChildOrder(&position.x, &traversal[level].childCount);
			traversal[level].childOrder = order;
			if (camera)
			{
				if (level == *maximumDepth || (level >= minimumDepth && (mask & node->flags)))
				{
					if (classification < 2)
						classification = static_cast<DWORD>(camera->ClassifyBox(node->bounds));
					if (classification)
					{
						output[outputCount].bounds = node->bounds;
						output[outputCount++].depth = level;
					}
				}
				if (level == *maximumDepth) next = 8;
			}
			else if (level >= minimumDepth && level <= *maximumDepth)
			{
				output[outputCount].bounds = node->bounds;
				output[outputCount++].depth = level;
				next = 8;
			}
		}
		if (!(node->flags & 1))
		{
			// Reload the map's traversal owner after the virtual classifier.
			OctreeTraversal& frame = traversal[level];
			while (next < frame.childCount)
			{
				OctreeNode* child = node->children[frame.childOrder[next++]];
				if (child)
				{
					node = child;
					frame.nextChild = next;
					++level;
					if (classification > 1) ++classification;
					goto NextBoundsNode;
				}
			}
		}
		traversal[level].nextChild = 0;
		node = node->parent;
		--level;
		if (classification) --classification;
	NextBoundsNode:;
	}
	while (node);
	output[outputCount].bounds = NULL;
	return 0;
}
