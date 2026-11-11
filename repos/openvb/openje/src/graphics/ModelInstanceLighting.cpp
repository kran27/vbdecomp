#include "ModelInstance.hpp"
#include "ModelNode.hpp"
#include "ModelMeshPacket.hpp"
#include "SkinMeshPacket.hpp"

void Gfx_ModelNode::AddLight(Gfx_Light* light)
{
	if (!light) return;
	const DWORD count = static_cast<DWORD>(attributes.size());
	for (DWORD i = 0; i < count; ++i)
	{
		Gfx_NodeAttribute* attribute = attributes[i];
		Gfx_ModelMeshPkt* mesh = dynamic_cast<Gfx_ModelMeshPkt*>(attribute);
		if (mesh)
		{
			const BYTE mode = static_cast<BYTE>(mesh->source->value24 & 6);
			if (!mode) mesh->packet->AddLight(light);
		}
		else if (Gfx_SkinMeshPkt* skin = dynamic_cast<Gfx_SkinMeshPkt*>(attribute))
			skin->AddLight(light);
	}
	const DWORD childCount = static_cast<DWORD>(children.size());
	for (DWORD i = 0; i < childCount; ++i) children[i]->AddLight(light);
}

void Gfx_ModelNode::RemoveLight(Gfx_Light* light)
{
	if (!light) return;
	const DWORD count = static_cast<DWORD>(attributes.size());
	for (DWORD i = 0; i < count; ++i)
	{
		Gfx_NodeAttribute* attribute = attributes[i];
		Gfx_ModelMeshPkt* mesh = dynamic_cast<Gfx_ModelMeshPkt*>(attribute);
		if (mesh)
		{
			const BYTE mode = static_cast<BYTE>(mesh->source->value24 & 6);
			if (!mode) mesh->packet->RemoveLight(light);
		}
		else if (Gfx_SkinMeshPkt* skin = dynamic_cast<Gfx_SkinMeshPkt*>(attribute))
			skin->RemoveLight(light);
	}
	const DWORD childCount = static_cast<DWORD>(children.size());
	for (DWORD i = 0; i < childCount; ++i) children[i]->RemoveLight(light);
}

void Gfx_ModelNode::FinishLights()
{
	const DWORD count = static_cast<DWORD>(attributes.size());
	for (DWORD i = 0; i < count; ++i)
	{
		Gfx_NodeAttribute* attribute = attributes[i];
		Gfx_ModelMeshPkt* mesh = dynamic_cast<Gfx_ModelMeshPkt*>(attribute);
		if (mesh)
		{
			const BYTE mode = static_cast<BYTE>(mesh->source->value24 & 6);
			if (!mode) mesh->packet->FinishLights();
		}
		else if (Gfx_SkinMeshPkt* skin = dynamic_cast<Gfx_SkinMeshPkt*>(attribute))
			skin->FinishLights();
	}
	const DWORD childCount = static_cast<DWORD>(children.size());
	for (DWORD i = 0; i < childCount; ++i) children[i]->FinishLights();
}

void Gfx_ModelNode::ClearLights()
{
	const DWORD count = static_cast<DWORD>(attributes.size());
	for (DWORD i = 0; i < count; ++i)
	{
		Gfx_NodeAttribute* attribute = attributes[i];
		Gfx_ModelMeshPkt* mesh = dynamic_cast<Gfx_ModelMeshPkt*>(attribute);
		if (mesh)
		{
			const BYTE mode = static_cast<BYTE>(mesh->source->value24 & 6);
			if (!mode) mesh->packet->ClearLights();
		}
		else if (Gfx_SkinMeshPkt* skin = dynamic_cast<Gfx_SkinMeshPkt*>(attribute)) skin->lights.clear();
	}
	const DWORD childCount = static_cast<DWORD>(children.size());
	for (DWORD i = 0; i < childCount; ++i) children[i]->ClearLights();
}

BYTE Gfx_ModelInstance::AddLight(Gfx_Light* light) { root->AddLight(light); return 1; }
void Gfx_ModelInstance::RemoveLight(Gfx_Light* light) { root->RemoveLight(light); }
void Gfx_ModelInstance::FinishLights() { root->FinishLights(); }
void Gfx_ModelInstance::ClearLights() { root->ClearLights(); }
