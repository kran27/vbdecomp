#include "Octree.hpp"
#include <string.h>

int OCTREE::PackVertexStreams(DWORD attributes, const OctreeVertexStreams* source, DWORD* destination)
{
	DWORD* output = destination;
	const DWORD position = attributes & 0x100;
	if (position && !source->positions) return 1;
	const DWORD normal = attributes & 0x200;
	if (normal && !source->normals) return 1;
	const DWORD color = attributes & 0x400;
	if (color && !source->colors) return 1;
	const DWORD texture = attributes & 0x800;
	if (texture && !source->textureUV) return 1;
	const DWORD lightmap = attributes & 0x1000;
	if (lightmap && !source->lightmapUV) return 1;
	for (DWORD index = 0; index < source->count; ++index)
	{
		if (position)
		{
			memcpy(output, source->positions + 3 * index, 12);
			output += 3;
		}
		if (normal)
		{
			memcpy(output, source->normals + 3 * index, 12);
			output += 3;
		}
		if (color) *output++ = source->colors[index];
		if (texture)
		{
			memcpy(output, source->textureUV + 2 * index, 8);
			output += 2;
		}
		if (lightmap)
		{
			memcpy(output, source->lightmapUV + 2 * index, 8);
			output += 2;
		}
	}
	return 0;
}
