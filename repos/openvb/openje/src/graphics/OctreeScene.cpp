#include "Octree.hpp"
#include <new>
#include <string.h>

OCTREE_Scene::OCTREE_Scene()
	: bucketIndex(0), nodeIndex(0), wordCursor(0), buckets(0)
{
	// The inherited header is deliberately unwritten until the stream supplies it.
}

OCTREE_Scene::~OCTREE_Scene()
{
	if (buckets)
	{
		OctreeNodeReference** bucket = buckets;
		while (*bucket)
		{
			delete[] *bucket++;
		}
		delete[] buckets;
	}
}

int OCTREE_Scene::Release()
{
	if (buckets)
	{
		for (DWORD index = 0; buckets[index]; ++index) delete[] buckets[index];
		delete[] buckets;
	}
	::operator delete(this);
	return 0;
}

int OCTREE_Scene::AdvanceNode()
{
	wordCursor = 0;
	++nodeIndex;
	if (buckets[bucketIndex][nodeIndex].node) return 1;
	++bucketIndex;
	if (buckets[bucketIndex])
	{
		nodeIndex = 0;
		return 1;
	}
	nodeIndex = 0;
	bucketIndex = 0;
	return 0;
}

int OCTREE_Scene::NextBatchInfo(OctreeBatchInfo* output)
{
	DWORD skip = 0;
	OctreeNode* node = buckets[bucketIndex][nodeIndex].node;
	if (wordCursor == node->wordCount)
	{
		return 0;
	}
	WORD* words = node->indices + wordCursor;
	current.vertexBuffer = node->vertexBuffer;
	output->vertexBuffer = current.vertexBuffer;
	if (*words & 0x8000)
	{
		current.material = *words & 0x7FFF;
		skip = 2;
	}
	output->material = current.material;
	if (words[skip] & 0x4000)
	{
		current.lightmap = words[skip] & 0x3FFF;
		if (current.lightmap)
		{
			current.lightmap += lightmapAdjustment;
		}
		skip += 2;
	}
	output->lightmap = current.lightmap;
	current.texture = words[skip];
	current.indexCount = words[skip + 1];
	output->texture = current.texture;
	output->indexCount = current.indexCount;
	wordCursor += skip + 2;
	return 1;
}

int OCTREE_Scene::CopyIndices(WORD* output)
{
	memcpy(output, buckets[bucketIndex][nodeIndex].node->indices + wordCursor,
		2 * current.indexCount);
	wordCursor += current.indexCount + (current.indexCount & 1);
	return 0;
}

DWORD* OCTREE_Scene::CurrentTriangleFlags()
{
	OctreeNode* node = buckets[bucketIndex][nodeIndex].node;
	if (node && node->triangleFlags) return node->triangleFlags;
	return NULL;
}

int OCTREE::CreateScene(OCTREE_Scene** output, DWORD mode)
{
	int result = 1;
	OctreeNodeReference** buckets;
	OctreeNode* node;
	int level;
	OCTREE_Scene* scene = new OCTREE_Scene;
	if (!scene) goto failed;
	buckets = new OctreeNodeReference*[vertexBufferCount + 1];
	if (!buckets) goto failed;
	memset(buckets, 0, 4 * (vertexBufferCount + 1));
	for (DWORD index = 0; index < vertexBufferCount; ++index)
	{
		buckets[index] = new OctreeNodeReference[indexedNodeCounts[index] + 1];
		if (!buckets[index]) goto failed;
		buckets[index][0].node = NULL;
		sceneNodeCounts[index] = 0;
	}
	result = 2;
	switch (mode)
	{
	case 0:
		mode = 1;
		break;
	case 1:
		if (!(flags & 2)) goto failed;
		mode = 65;
		break;
	case 2:
		if (!(flags & 2)) goto failed;
		mode = 67;
		break;
	}
	node = &root;
	level = 0;
	do
	{
		DWORD nextChild = traversal[level].nextChild;
		if (!nextChild && (node->flags & mode))
		{
			DWORD buffer = node->vertexBuffer;
			node->ordinal = sceneNodeCounts[buffer];
			buckets[buffer][sceneNodeCounts[buffer]++].node = node;
			buckets[buffer][sceneNodeCounts[buffer]].node = NULL;
		}
		if (!(node->flags & 1))
		{
			while (nextChild < 8)
			{
				OctreeNode* child = node->children[nextChild++];
				if (child)
				{
					traversal[level].nextChild = nextChild;
					node = child;
					++level;
					goto NextNode;
				}
			}
		}
		traversal[level].nextChild = 0;
		node = node->parent;
		--level;
	NextNode:;
	}
	while (node);
	scene->buckets = buckets;
	scene->lightmapAdjustment = firstLightmap - 1;
	*output = scene;
	return 0;
failed:
	// Native first-allocation failure reads buckets before its producer.
	if (buckets)
	{
		for (DWORD item = 0; item < vertexBufferCount; ++item)
			if (buckets[item]) delete[] buckets[item];
		delete[] buckets;
	}
	if (scene) scene->Release();
	return result;
}
