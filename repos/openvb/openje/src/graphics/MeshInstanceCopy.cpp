#include "MeshData.hpp"
#include <cstring>

Gfx_MeshInstance::Gfx_MeshInstance(const Gfx_MeshInstance& source)
{
	mesh = source.mesh;
	value04 = 0;
	materialSet = 0;
	value0C = source.value0C;
	flag1C = source.flag1C;
	flag1D = source.flag1D;
	flag1E = source.flag1E;
	flag1F = source.flag1F;
	// The original copies this float's raw word without an x87 load/store.
	std::memcpy(&value20, &source.value20, sizeof(value20));
	value24 = source.value24;
	if (source.value04 != 0)
	{
		materialSet = new Gfx_MaterialSet(
			*reinterpret_cast<const Gfx_MaterialSet*>(source.value04));
		// Publication at +08 precedes RebuildGroups; +04 follows it.
		materialSet->RebuildGroups();
		value04 = reinterpret_cast<DWORD>(materialSet);
	}
	primitiveFlags = new BYTE[mesh->primitiveCount];
	// Reload the mesh count after allocation, as in the original copy.
	std::memcpy(primitiveFlags, source.primitiveFlags, mesh->primitiveCount);
	ownsSkinning = 0;
	skinning = source.skinning;
}
