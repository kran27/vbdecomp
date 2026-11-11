#include "SkinMeshPacket.hpp"
#include "AnimationInstance.hpp"
#include "SystemInterface.hpp"
#include <string.h>

void Gfx_SkinMeshPkt::EnableAllMaterials()
{
	for (DWORD i = 0; i < mesh->materialCount; ++i)
		materialEnabled[i] = 1;
}

void Gfx_SkinMeshPkt::SetScale(float x, float y, float z)
{
	if (animation) animation->SetScale(x, y, z);
}

Gfx_SkinMeshPkt::Gfx_SkinMeshPkt(Gfx_SkinMesh* mesh)
	: mesh(mesh),
	animation(NULL), overrideMaterials(NULL), flag98(0), flag99(0),
	scale(1.0f, 1.0f, 1.0f), ownsSkinning(1), skinning(NULL), castsShadow(1),
	parent(NULL), valueBC(1), ambientOverride(0)
{
	D3DXMatrixIdentity(&transform);
	materialCount = this->mesh->materialCount; //49DF7B, live member.
	self = this;
	groups = new std::list<DWORD>[6];
	//49DFD2/49DFF7 retain the incoming mesh across list allocation and reload
	//its count for allocation and every loop bound; they do not use materialCount.
	materialEnabled = new BYTE[mesh->materialCount];
	for (DWORD material = 0; material < mesh->materialCount; ++material)
		materialEnabled[material] = 1;
	extraLight.x = 0.0f;
	extraLight.y = 0.0f;
	extraLight.z = 0.0f;
	extraLight.w = 1.0f;
	ambientColor = 0x00808080;
	ResetSkinning();
}

void Gfx_SkinMeshPkt::AttachTo(Gfx_SkinMeshPkt* value, DWORD bone)
{
	parent = value;
	self = value;
	for (Gfx_SkinMeshPkt* next = value; next; next = next->parent)
		if (next->parent) self = next->parent;
	valueBC = bone;
}

Gfx_SkinMeshPkt::~Gfx_SkinMeshPkt()
{
	ReleaseSkinning();
	if (materialEnabled)
	{
		delete[] materialEnabled;
		materialEnabled = NULL;
	}
	if (overrideMaterials)
	{
		delete[] overrideMaterials;
		overrideMaterials = NULL;
	}
	if (groups)
	{
		delete[] groups;
		groups = NULL;
	}
	//The real lights member then dies before the Gfx_NodeAttribute base.
}

void Gfx_SkinMeshPkt::ReleaseSkinning()
{
	if (ownsSkinning && skinning) { delete skinning; skinning = NULL; }
}

void Gfx_SkinMeshPkt::ResetSkinning()
{
	ReleaseSkinning();
	skinning = new Gfx_Skinning;
	skinning->Allocate(mesh->shadowFaceCount);
	ownsSkinning = 1;
}

void Gfx_SkinMeshPkt::CloneMaterials()
{
	if (overrideMaterials)
	{
		delete[] overrideMaterials;
		overrideMaterials = NULL;
	}
	overrideMaterials = new Gfx_Material[materialCount];
	for (DWORD material = 0; material < materialCount; ++material)
		overrideMaterials[material] = mesh->materials[material];
}

Gfx_Material* Gfx_SkinMeshPkt::GetMaterial(DWORD index)
{
	if (!overrideMaterials)
	{
		CloneMaterials();
		mesh->RebuildGroups(overrideMaterials, groups);
	}
	return overrideMaterials + index;
}

void Gfx_SkinMeshPkt::EnableMaterial(const std::string& materialName, BYTE enabled)
{
	const DWORD count = mesh->materialCount;
	std::string otherName;
	for (DWORD i = 0; i < count; ++i)
	{
		if (materialName == mesh->materialNames[i])
		{
			materialEnabled[i] = enabled;
			const DWORD associatedCount = mesh->materialNamesExtra[i].count;
			if (associatedCount)
			{
				for (DWORD associated = 0; associated < associatedCount; ++associated)
				{
					otherName = mesh->GetMaterialExtraName(i, associated);
					for (DWORD material = 0; material < count; ++material)
						if (otherName == mesh->materialNames[material])
							materialEnabled[material] = enabled == 0;
				}
			}
			break;
		}
	}
}

void Gfx_SkinMeshPkt::ResetMaterialValues()
{
	if (overrideMaterials)
	{
		for (DWORD i = 0; i < materialCount; ++i)
		{
			Gfx_Material& material = overrideMaterials[i];
			material.style2 = material.style1;
			material.mode28 = material.mode24;
			material.ResetTextureFactor();
		}
		mesh->RebuildGroups(overrideMaterials, groups);
	}
}

void Gfx_SkinMeshPkt::SetRenderPhase(DWORD phase)
{
	if (!overrideMaterials) CloneMaterials();
	for (DWORD i = 0; i < materialCount; ++i)
		overrideMaterials[i].mode28 = phase;
	mesh->RebuildGroups(overrideMaterials, groups);
}

void Gfx_SkinMeshPkt::SetMaterialFlagBC(BYTE value)
{
	if (!overrideMaterials) CloneMaterials();
	for (DWORD i = 0; i < materialCount; ++i)
		overrideMaterials[i].flagBC = value;
}

void Gfx_SkinMeshPkt::SetMaterialColors(BYTE useRGB, BYTE useAlpha, DWORD color, const DWORD* emissive)
{
	if (!overrideMaterials) CloneMaterials();
	DWORD emissiveColor;
	if (emissive) emissiveColor = *emissive;
	for (DWORD i = 0; i < materialCount; ++i)
	{
		Gfx_Material* material = &overrideMaterials[i];
		if (useRGB || useAlpha) material->SetColor(color, useRGB, useAlpha);
		if (emissive) material->SetEmissive(emissiveColor);
	}
	mesh->RebuildGroups(overrideMaterials, groups);
}

void Gfx_SkinMeshPkt::SetMaterialStyle(DWORD style)
{
	if (!overrideMaterials) CloneMaterials();
	for (DWORD i = 0; i < materialCount; ++i)
		overrideMaterials[i].style2 = style;
	mesh->RebuildGroups(overrideMaterials, groups);
}

void Gfx_SkinMeshPkt::Submit(Gfx_Viewport* viewport, BYTE shadow, float value, DWORD color)
{
	Gfx_System_Interface* system = g_GfxSystem;
	DWORD rawValue;
	memcpy(&rawValue, &value, sizeof(rawValue));
	system->QueueRenderable(this, viewport, shadow, rawValue, color);
}
