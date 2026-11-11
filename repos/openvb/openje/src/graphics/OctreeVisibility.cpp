#include "Octree.hpp"
#include <string.h>

namespace
{
	DWORD ReadIndexPair(const WORD* input)
	{
		DWORD result;
		memcpy(&result,input,sizeof(result));
		return result;
	}

	void WriteIndexPair(WORD* output,DWORD value)
	{
		memcpy(output,&value,sizeof(value));
	}

}

int OCTREE::NextVisibleVertexBuffer(DWORD* buffer, DWORD* vertices)
{
	while (visibleBufferCursor < vertexBufferCount && !sceneNodeCounts[visibleBufferCursor])
		++visibleBufferCursor;
	*buffer = visibleBufferCursor;
	if (visibleBufferCursor >= vertexBufferCount)
	{
		*vertices = 0;
		return 0;
	}
	*vertices = vertexBuffers[visibleBufferCursor].count;
	DWORD current = visibleBufferCursor;
	visibleNodes = visibleBuckets[current];
	visibleBufferCursor = current + 1;
	visibleNodeCursor = 0;
	value30 = 0;
	value50 = 0;
	value40 = 0;
	for (DWORD index = 0; index < materialCount; ++index) materialValues[index] = 0;
	OctreeNode* node = visibleNodes[0].node;
	DWORD reference = 0;
	while (node)
	{
		WORD* indices = node->indices;
		for (DWORD word = 0; word < node->wordCount; word += indices[word + 1] + 2)
		{
			materialValues[indices[word] & 0x7FFF] = 1;
		}
		node = visibleNodes[++reference].node;
	}
	return 1;
}

int OCTREE::ReadVisibleNodeIDs(DWORD* capacity, DWORD* output)
{
	OctreeNode* node = visibleNodes[visibleNodeCursor].node;
	DWORD written = 0;
	if (node)
	{
		while (written < *capacity)
		{
			output[written] = node->ordinal;
			++visibleNodeCursor;
			node = visibleNodes[visibleNodeCursor].node;
			++written;
			if (!node)
			{
				*capacity = written;
				return 0;
			}
		}
		if (written == *capacity) return 1;
	}
	*capacity = written;
	return 0;
}

int OCTREE::NextVisibleMaterial(WORD* material)
{
	while (value30<materialCount && !materialValues[value30]) ++value30;
	*material=WORD(value30);
	if (value30>=materialCount)
	{
		*material=0;
		return 0;
	}
	value50=0;
	for (DWORD index=0;index<lightmapCount;++index) lightmapHandles[index]=0;
	DWORD reference=0;
	OctreeNode* node=visibleNodes[0].node;
	while (node)
	{
		DWORD* cursor=&visibleNodes[reference].unwritten;
		WORD* words=node->indices+*cursor;
		if (*cursor<node->wordCount && (words[0]&0x7FFF)==value30)
		{
			*cursor+=2;
			DWORD remaining=words[1],word=2;
			while (remaining)
			{
				lightmapHandles[words[word]&0xAFFF]=1;
				DWORD count=words[word+1];
				remaining-=count+2;
				word+=count+2;
			}
		}
		node=visibleNodes[++reference].node;
	}
	return 1;
}

int OCTREE::NextVisibleLightmap(WORD* lightmap)
{
	while (value50 < lightmapCount)
	{
		if (lightmapHandles[value50])
		{
			// Publish before the live cursor/count gate, including output aliases.
			*lightmap = WORD(value50);
			break;
		}
		++value50;
	}
	if (value50>=lightmapCount)
	{
		++value30;
		return 0;
	}
	value40=0;
	for (DWORD index=0;index<firstLightmap;++index) textureHandles[index]=0;
	DWORD reference=0;
	OctreeNode* node=visibleNodes[0].node;
	while (node)
	{
		DWORD* cursor=&visibleNodes[reference].unwritten;
		WORD* words=node->indices+*cursor;
		if (*cursor<node->wordCount && (words[0]&0xAFFF)==value50)
		{
			*cursor+=2;
			DWORD remaining=words[1],word=2;
			while (remaining)
			{
				DWORD count=words[word+1];
				textureHandles[words[word]]+=count;
				DWORD step=count+(count&1)+2;
				word+=step;
				remaining-=step;
			}
		}
		node=visibleNodes[++reference].node;
	}
	if (*lightmap) *lightmap=WORD(*lightmap+firstLightmap-1);
	return 1;
}

int OCTREE::NextVisibleTexture(WORD* texture,DWORD* indices)
{
	while (value40 < firstLightmap)
	{
		if (textureHandles[value40])
		{
			*texture = WORD(value40);
			*indices = textureHandles[value40];
			return 1;
		}
		++value40;
	}
	++value50;
	return 0;
}

int OCTREE::CopyVisibleIndices(WORD* output)
{
	DWORD reference=0,written=0;
	OctreeNode* node=visibleNodes[0].node;
	while (node)
	{
		DWORD cursor=visibleNodes[reference].unwritten;
		if (cursor<node->wordCount)
		{
			WORD* words=node->indices+cursor;
			if (words[0]==value40)
			{
				DWORD count=words[1];
				//5AE790..5AE7F2 is part of this routine, with no packet-copy call.
				//Keep its alignment branches and tiny-count writes intact.
				WORD* destination=output+written;
				const WORD* input=words+2;
				if (!(reinterpret_cast<ULONG_PTR>(destination)&2))
				{
					LONG remaining=LONG(count&~1u);
					do
					{
						DWORD pair=ReadIndexPair(input);
						input+=2;
						WriteIndexPair(destination,pair);
						destination+=2;
						remaining-=2;
					}
					while (remaining>0);
					if (count&1) *destination=*input;
				}
				else
				{
					DWORD previous=ReadIndexPair(input);
					input+=2;
					*destination++=WORD(previous);
					LONG remaining=LONG((count-1)&~1u);
					do
					{
						DWORD next=ReadIndexPair(input);
						input+=2;
						WriteIndexPair(destination,(previous>>16)|(next<<16));
						destination+=2;
						previous=next;
						remaining-=2;
					}
					while (remaining>0);
					if ((count-1)&1) *destination=WORD(previous>>16);
				}
				visibleNodes[reference].unwritten+=count+(count&1)+2;
				written+=count;
			}
		}
		node=visibleNodes[++reference].node;
	}
	++value40;
	return 0;
}
