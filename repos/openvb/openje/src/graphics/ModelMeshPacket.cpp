#include "ModelMeshPacket.hpp"
#include "SystemInterface.hpp"

Gfx_ModelMeshPkt::Gfx_ModelMeshPkt(Gfx_ModelMesh* mesh)
{
	Gfx_MeshData* geometry = &mesh->mesh;
	source = mesh;
	packet = Gfx_CreatePacketData(geometry->value00);
	instance.SetMesh(geometry);
	instance.value0C = reinterpret_cast<DWORD>(packet);
	BYTE flags = static_cast<BYTE>(source->value24 & 6);
	if (!flags) instance.AllocateSkinning();
	overrideMaterials = NULL;
}

Gfx_ModelMeshPkt::~Gfx_ModelMeshPkt()
{
	if (packet) { delete packet; packet = NULL; }
	if (overrideMaterials) { delete overrideMaterials; overrideMaterials = NULL; }
}

void Gfx_ModelMeshPkt::Submit(Gfx_Viewport* viewport, BYTE shadow, float value, DWORD color)
{
	Gfx_MeshInstance* submission = &instance;
	submission->flag1F = shadow;
	submission->value20 = value;
	submission->value24 = color;
	g_GfxSystem->QueueMeshInstances(submission, viewport, 1);
}

void Gfx_ModelMeshPkt::CloneMaterials()
{
	Gfx_MeshData* model = instance.mesh;
	overrideMaterials = new Gfx_MaterialSet(model->materialSet);
	overrideMaterials->RebuildGroups();
	instance.value04 = reinterpret_cast<DWORD>(overrideMaterials);
}

Gfx_Material* Gfx_ModelMeshPkt::GetMaterial(DWORD index)
{
	if (!overrideMaterials) CloneMaterials();
	return overrideMaterials->materials + index;
}

void Gfx_ModelMeshPkt::SetRenderPhase(DWORD phase)
{
	if (!overrideMaterials) CloneMaterials();
	Gfx_MaterialSet* materials = overrideMaterials;
	for (DWORD i = 0; i < materials->count; ++i)
	{
		Gfx_Material& material = materials->materials[i];
		material.mode28 = phase;
	}
	overrideMaterials->RebuildGroups();
}

void Gfx_ModelMeshPkt::SetMaterialFlagBC(BYTE value)
{
	if (!overrideMaterials) CloneMaterials();
	Gfx_MaterialSet* materials = overrideMaterials;
	for (DWORD i = 0; i < materials->count; ++i)
	{
		Gfx_Material& material = materials->materials[i];
		material.flagBC = value;
	}
}
