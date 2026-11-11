#include "Octree.hpp"
#include "OctreeShape.hpp"
#include "../diagnostics/DebugOutput.hpp"
#include <string.h>

namespace
{
	DWORD ReadGridDWORD(const BYTE* data)
	{
		DWORD value;
		memcpy(&value, data, sizeof(value));
		return value;
	}
	SHORT ReadGridWORD(const BYTE* data)
	{
		SHORT value;
		memcpy(&value, data, sizeof(value));
		return value;
	}
}

int OCTREE::QueryVisibleObjects(DWORD capacity, void** output, OCTREE_Shape* filter, DWORD* more)
{
	DWORD written = 0;
	*more = 0;
	if (!visibleObjects) return 3;
	const DWORD limit = capacity - 1; // Preserve native unsigned zero-capacity underflow.
	OctreeSpatialObject* object = visibleObjects[spatialCursor];
	// Original selects one traversal before entering its loop.
	if (filter)
	{
		while (object && written < limit)
		{
			if (filter->ClassifyObject(object->center, object->extent))
				output[written++] = object->object;
			object = visibleObjects[++spatialCursor];
		}
	}
	else
	{
		while (object && written < limit)
		{
			if (object->flags & 1)
				output[written++] = object->object;
			object = visibleObjects[++spatialCursor];
		}
	}
	output[written] = NULL;
	if (object) *more = 1;
	return 0;
}

int OCTREE::GetLightVisibility(const float* position, DWORD* count, WORD* output)
{
	*count = 0;
	OctreeLightGrid* grid;
	if (position[0] < root.bounds[0] || position[2] < root.bounds[2]
		|| !(grid = lightGrid)) return 2;
	if (position[0] < root.bounds[0] || root.bounds[3] <= position[0]
		|| position[2] < root.bounds[2] || root.bounds[5] <= position[2])
	{
		JE::LogDebugString("OCTREE::GetLightVisibility - invalid position ( %f, %f, %f ) in this octree( %f, %f, %f ) - ( %f, %f, %f )\n",
			position[0], position[1], position[2], root.bounds[0], root.bounds[1], root.bounds[2],
			root.bounds[3], root.bounds[4], root.bounds[5]);
		return 2;
	}
	// Original5AFD7B truncates row before5AFD88's integer multiply by width.
	LONG row = LONG((root.bounds[5] - position[2]) /
		*reinterpret_cast<const float*>(grid->words + 3) + 0.5f);
	LONG column = LONG((position[0] - root.bounds[0]) /
		*reinterpret_cast<const float*>(grid->words + 2) + 0.5f);
	DWORD cell = DWORD(row) * grid->words[0] + DWORD(column);
	const BYTE* bytes = grid->data;
	DWORD bit = cell & 31;
	DWORD directory = (cell >> 4) & ~DWORD(1);
	DWORD wordIndex = ReadGridDWORD(bytes + directory * 4) >> 1;
	DWORD bitmap = ReadGridDWORD(bytes + (directory + 1) * 4);
	if (!(bitmap & (0x80000000UL >> bit))) return 0;

	SHORT tag = ReadGridWORD(bytes + wordIndex * 2) & SHORT(0x8000);
	DWORD mask = 0x80000000UL;
	DWORD remaining = bit;
	while (remaining)
	{
		if (bitmap & mask)
		{
			const BYTE* cursor = bytes + wordIndex * 2;
			do { cursor += 2; ++wordIndex; }
			while (SHORT(ReadGridWORD(cursor) & SHORT(0x8000)) == tag);
			tag ^= SHORT(0x8000);
		}
		mask >>= 1;
		--remaining;
	}
	SHORT value = SHORT(ReadGridWORD(bytes + wordIndex * 2));
	do
	{
		output[(*count)++] = value & 0x7FFF;
		value = SHORT(ReadGridWORD(lightGrid->data + (++wordIndex) * 2));
	}
	while (SHORT(value & SHORT(0x8000)) == tag);
	return 0;
}
