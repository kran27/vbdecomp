#pragma once

#include "ModelMesh.hpp"
#include "PacketData.hpp"

class Gfx_Viewport;

class Gfx_ModelMeshPkt : public Gfx_NodeAttribute
{
public:
	explicit Gfx_ModelMeshPkt(Gfx_ModelMesh* source); // 0x49E1D0.
	virtual ~Gfx_ModelMeshPkt(); // 0x49EC00.
	void Submit(Gfx_Viewport* viewport, BYTE shadow, float value, DWORD color); // 0x50E390.
	void EnableMaterial(const std::string& name, BYTE enabled)
	{
		if (!overrideMaterials) CloneMaterials();
		instance.EnableMaterial(name, enabled);
	}
	void SetMaterialColors(BYTE useRGB, BYTE useAlpha, DWORD color, const DWORD* emissive)
	{
		if (!overrideMaterials) CloneMaterials();
		overrideMaterials->SetColors(useRGB, useAlpha, color, emissive);
		overrideMaterials->RebuildGroups();
	}
	void CloneMaterials(); //50E290; allocate/copy, rebuild groups, publish instance owner.
	void SetRenderPhase(DWORD phase); //49CA60; lazy clone, update modes, rebuild groups.
	void SetMaterialFlagBC(BYTE value); //49CAB0; raw per-material flag, lazy clone.
	Gfx_Material* GetMaterial(DWORD index); // 0x50E300.
	Gfx_ModelMesh* source;
	Gfx_MeshInstance instance;
	Gfx_PacketData* packet;
	Gfx_MaterialSet* overrideMaterials;
};
