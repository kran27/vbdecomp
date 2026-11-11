#pragma once

#include "SkinRender.hpp"
#include <cstddef>

class Gfx_Light;
class Gfx_AnimationInstance;

class Gfx_SkinMeshPkt : public Gfx_NodeAttribute
{
public:
	explicit Gfx_SkinMeshPkt(Gfx_SkinMesh* mesh); // 0x49DEA0.
	virtual ~Gfx_SkinMeshPkt(); // 0x49E2C0.
	void ResetSkinning(); // 0x507B30.
	BYTE AddLight(Gfx_Light* light); //509650; capped borrowed-light list.
	void RemoveLight(Gfx_Light* light); //5088C0; first matching list entry.
	void FinishLights(); //404B70; shared empty callback.
	void ReleaseSkinning(); // 0x49E3D0.
	void AttachTo(Gfx_SkinMeshPkt* parent, DWORD bone); // 0x507BE0.
	void SetScale(float x, float y, float z); //507C30; forwards to the borrowed animation.
	void Submit(Gfx_Viewport* viewport, BYTE shadow, float value, DWORD color); //507C60, system queue.
	void EnableAllMaterials(); //507B00; live mesh count, not cached materialCount.
	void ResetMaterialValues(); //5095A0; existing overrides only, shared by reset callers.
	void CloneMaterials(); //508F80; replace owned overrides, copy live source materials.
	void SetRenderPhase(DWORD phase); //509600; lazy clone, update modes, rebuild groups.
	void SetMaterialStyle(DWORD style); //5094B0; lazy clone, style2, rebuild groups.
	void SetMaterialFlagBC(BYTE value); //509060; raw per-material flag, lazy clone.
	void SetMaterialColors(BYTE useRGB, BYTE useAlpha, DWORD color, const DWORD* emissive); //509500.
	Gfx_Material* GetMaterial(DWORD index); // 0x509470, lazy clone508F80.
	void EnableMaterial(const std::string& name, BYTE enabled); // 0x5090A0; associated complements.
	Gfx_SkinMesh* mesh;
	D3DXMATRIX transform;
	DWORD materialCount;
	Gfx_AnimationInstance* animation;
	Gfx_Material* overrideMaterials;
	BYTE* materialEnabled; //+74, owned raw material-count byte allocation.
	std::list<DWORD>* groups; //+78, owned array of six real VC7 lists.
	D3DXVECTOR4 extraLight;
	std::list<Gfx_Light*> lights;
	BYTE flag98, flag99;
	D3DXVECTOR3 scale;
	BYTE ownsSkinning;
	Gfx_Skinning* skinning;
	BYTE castsShadow;
	Gfx_SkinMeshPkt* parent; // +B4; borrowed, self below caches the chain's root.
	Gfx_SkinMeshPkt* self;
	DWORD valueBC;
	BYTE ambientOverride;
	D3DCOLOR ambientColor;

private:
	Gfx_SkinMeshPkt(const Gfx_SkinMeshPkt&);
	Gfx_SkinMeshPkt& operator=(const Gfx_SkinMeshPkt&);
};

#if defined(_MSC_VER) && _MSC_VER == 1300 && defined(_M_IX86)
//49DEA0/49E2C0 establish these actual members, without invented prefix storage.
typedef char GfxSkinPacketBaseSize[(sizeof(Gfx_NodeAttribute) == 0x24) ? 1 : -1];
typedef char GfxSkinPacketSize[(sizeof(Gfx_SkinMeshPkt) == 0xC8) ? 1 : -1];
#define GFX_SKIN_PACKET_OFFSET(member, offset) \
	typedef char GfxSkinPacketOffset_##member[(offsetof(Gfx_SkinMeshPkt, member) == offset) ? 1 : -1]
GFX_SKIN_PACKET_OFFSET(mesh, 0x24);
GFX_SKIN_PACKET_OFFSET(transform, 0x28);
GFX_SKIN_PACKET_OFFSET(materialCount, 0x68);
GFX_SKIN_PACKET_OFFSET(animation, 0x6C);
GFX_SKIN_PACKET_OFFSET(overrideMaterials, 0x70);
GFX_SKIN_PACKET_OFFSET(materialEnabled, 0x74);
GFX_SKIN_PACKET_OFFSET(groups, 0x78);
GFX_SKIN_PACKET_OFFSET(extraLight, 0x7C);
GFX_SKIN_PACKET_OFFSET(lights, 0x8C);
GFX_SKIN_PACKET_OFFSET(flag98, 0x98);
GFX_SKIN_PACKET_OFFSET(flag99, 0x99);
GFX_SKIN_PACKET_OFFSET(scale, 0x9C);
GFX_SKIN_PACKET_OFFSET(ownsSkinning, 0xA8);
GFX_SKIN_PACKET_OFFSET(skinning, 0xAC);
GFX_SKIN_PACKET_OFFSET(castsShadow, 0xB0);
GFX_SKIN_PACKET_OFFSET(parent, 0xB4);
GFX_SKIN_PACKET_OFFSET(self, 0xB8);
GFX_SKIN_PACKET_OFFSET(valueBC, 0xBC);
GFX_SKIN_PACKET_OFFSET(ambientOverride, 0xC0);
GFX_SKIN_PACKET_OFFSET(ambientColor, 0xC4);
#undef GFX_SKIN_PACKET_OFFSET
#endif
