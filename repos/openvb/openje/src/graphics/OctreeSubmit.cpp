#include "OctreeRender.hpp"
#include "SystemInterface.hpp"
#include <string.h>

namespace
{
	// Fixed SDK FVF42h vertex boundary, not an engine-owner layout substitute.
	struct BoundsVertex
	{
		float x, y, z;
		DWORD color;
	};
}

void Octree2Gfx::SubmitVisible(BYTE debugBounds, Gfx_Viewport* viewport)
{
	DWORD vertexCount = 0, indexCount = 0;
	if (!octree) return;
	UpdateMaterialStates();
	if (floorEnabled)
	{
		for (DWORD index = 0; index < meshCount; ++index) floorGroups[index].visible = 0;
	}
	DWORD buffer, ignoredVertices;
	while (octree->NextVisibleVertexBuffer(&buffer, &ignoredVertices))
	{
		Gfx_MeshInstance* instance = instances + buffer;
		Gfx_Primitive* primitive = instance->mesh->primitives;
		DWORD count = instance->mesh->primitiveCount;
		while (count)
		{
			// This flag is indexed by the primitive's NODE ordinal, not its
			// position in the material/primitive array.
			instance->primitiveFlags[primitive->value00] = 0;
			++primitive;
			--count;
		}
		if (floorEnabled)
		{
			OctreeFloorGroup* floor = floorGroups + buffer;
			floor->visible = 1;
			for (DWORD index = 0; index < floorGroups[buffer].count; ++index)
				floor->segments[index].visible = 0;
		}
		int more;
		do
		{
			DWORD capacity = nodeIDCapacity;
			more = octree->ReadVisibleNodeIDs(&capacity, nodeIDs);
			for (DWORD index = 0; index < capacity; ++index)
			{
				DWORD node = nodeIDs[index];
				instances[buffer].primitiveFlags[node] = 1;
				if (floorEnabled)
				{
					OctreeFloorGroup* floor = floorGroups + buffer;
					for (DWORD segment = 0; segment < floor->count; ++segment)
					{
						if (floor->segments[segment].nodeOrdinal == node)
						{
							floor->segments[segment].visible = 1;
							break;
						}
					}
				}
				if (showBounds && debugBounds)
				{
					const WORD edges[24] =
					{
						0, 1, 1, 2, 2, 3, 3, 0, 4, 5, 5, 6,
						6, 7, 7, 4, 0, 4, 1, 5, 2, 6, 3, 7
					};
					for (DWORD edge = 0; edge < 24; ++edge)
						boundsIndices[indexCount + edge] = static_cast<WORD>(vertexCount + edges[edge]);
					indexCount += 24;
					const D3DXVECTOR3& low = nodeMinimum[buffer][node];
					const D3DXVECTOR3& high = nodeMaximum[buffer][node];
					BoundsVertex* output = reinterpret_cast<BoundsVertex*>(boundsVertices) + vertexCount;
					output[0].x = low.x; output[0].y = high.y; output[0].z = high.z;
					output[0].color = 0x8080FFFF;
					output[1].x = high.x; output[1].y = high.y; output[1].z = high.z;
					output[1].color = 0x80FFFFFF;
					output[2].x = high.x; output[2].y = high.y; output[2].z = low.z;
					output[2].color = 0x80FFFF80;
					output[3].x = low.x; output[3].y = high.y; output[3].z = low.z;
					output[3].color = 0x8080FF80;
					output[4].x = low.x; output[4].y = low.y; output[4].z = high.z;
					output[4].color = 0x808080FF;
					output[5].x = high.x; output[5].y = low.y; output[5].z = high.z;
					output[5].color = 0x80FF80FF;
					output[6].x = high.x; output[6].y = low.y; output[6].z = low.z;
					output[6].color = 0x80FF8080;
					output[7].x = low.x; output[7].y = low.y; output[7].z = low.z;
					output[7].color = 0x80808080;
					vertexCount += 8;
				}
			}
		}
		while (more);
		g_GfxSystem->QueueMeshInstances(instances + buffer, viewport, 1);
	}
	if (showBounds && debugBounds && vertexCount && indexCount)
	{
		boundsMesh->SetVertexDesc(0x42, D3DXGetFVFVertexSize(0x42), vertexCount);
		boundsMesh->SetIndexDesc(2, indexCount);
		boundsPrimitive->value30 = indexCount >> 1;
		boundsMesh->materialSet.RebuildGroups();
		g_GfxSystem->QueueMeshInstances(boundsInstance, viewport, 1);
	}
	if (value80 && g_GfxSystem->DebugLightsEnabled())
	{
		for (DWORD index = 0; index < lightCount; ++index)
			g_GfxSystem->SubmitLight(lights + index, NULL);
	}
}
