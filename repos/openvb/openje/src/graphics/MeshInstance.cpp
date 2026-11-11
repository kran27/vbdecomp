#include "MeshData.hpp"

Gfx_MeshInstance::~Gfx_MeshInstance()
{
	if (materialSet) { delete materialSet; materialSet = 0; }
	if (primitiveFlags) { delete[] primitiveFlags; primitiveFlags = 0; }
	if (ownsSkinning && skinning) { delete skinning; skinning = 0; }
}

void Gfx_MeshInstance::SetMesh(Gfx_MeshData* next)
{
	mesh = next;
	if (primitiveFlags) { delete[] primitiveFlags; primitiveFlags = 0; }
	DWORD count = mesh->primitiveCount;
	BYTE* flags = new BYTE[count];
	primitiveFlags = flags;
	for (DWORD i = 0; i < mesh->primitiveCount; ++i) primitiveFlags[i] = 1;
}

void Gfx_MeshInstance::AllocateSkinning()
{
	if (ownsSkinning && skinning) { delete skinning; skinning = NULL; }
	skinning = new Gfx_Skinning;
	skinning->Allocate(mesh->value40);
	ownsSkinning = 1;
}

void Gfx_MeshInstance::EnableMaterial(const std::string& materialName, BYTE enabled)
{
	Gfx_Primitive* primitives = mesh->primitives;
	const DWORD count = mesh->primitiveCount;
	for (DWORD i = 0; i < count; ++i)
	{
		if (materialName == primitives[i].name)
		{
			primitiveFlags[i] = enabled;
			return;
		}
	}
}
