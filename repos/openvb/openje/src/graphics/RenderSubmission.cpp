#include "RenderManager.hpp"
#include "SkinMeshPacket.hpp"

void Gfx_RenderManager::QueueMeshInstances(Gfx_MeshInstance* instances, UINT count)
{
	while (count != 0)
	{
		Gfx_MeshData* mesh = instances->mesh;
		DWORD key = mesh != 0 ? mesh->value04 : 0;
		DWORD category = mesh != 0 ? mesh->value00 : 1;
		if (key != 0xFFFFFFFF)
		{
			Gfx_RenderGroup& group = staticGroups[category][key];
			group.mesh = instances->mesh;
			group.instances.push_back(*instances);
			Gfx_MaterialSet* materials = instances->value04 != 0
				? reinterpret_cast<Gfx_MaterialSet*>(instances->value04)
				: &instances->mesh->materialSet;
			// The original reloads the selected set's groupCounts for each sum.
			group.groupCounts[0] += materials->groupCounts[0];
			group.groupCounts[1] += materials->groupCounts[1];
			group.groupCounts[2] += materials->groupCounts[2];
			group.groupCounts[3] += materials->groupCounts[3];
			group.groupCounts[4] += materials->groupCounts[4];
			group.groupCounts[5] += materials->groupCounts[5];
		}
		else
		{
			// Capture the destination before constructing/copying the local group.
			std::list<Gfx_RenderGroup>* destination = &dynamicGroups[category];
			Gfx_RenderGroup group;
			group.mesh = instances->mesh;
			group.instances.push_back(*instances);
			Gfx_MaterialSet* materials = instances->value04 != 0
				? reinterpret_cast<Gfx_MaterialSet*>(instances->value04)
				: &instances->mesh->materialSet;
			DWORD* counts = materials->groupCounts;
			group.groupCounts[0] += counts[0];
			group.groupCounts[1] += counts[1];
			group.groupCounts[2] += counts[2];
			group.groupCounts[3] += counts[3];
			group.groupCounts[4] += counts[4];
			group.groupCounts[5] += counts[5];
			// Real deep list copying makes another owned instance copy, then the
			// local group's normal destruction releases its temporary instance.
			destination->push_back(group);
		}
		++instances;
		--count;
	}
}

void Gfx_RenderManager::QueueSkinMesh(Gfx_SkinMeshPkt* packet, BYTE flag,
	DWORD value08, DWORD value0C)
{
	if (!packet) return;
	Gfx_SkinMesh* mesh = packet->mesh;
	DWORD handle = mesh->handle;
	if (!handle) return;
	skinMeshes[handle] = mesh;
	std::list<Gfx_RenderSubmission16>& destination = skinSubmissions[handle];
	Gfx_RenderSubmission16 submission;
	submission.packet = packet;
	submission.flag = flag;
	submission.value08 = value08;
	submission.value0C = value0C;
	destination.push_back(submission);
}
