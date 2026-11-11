#pragma once

#include "GraphicsBaseObject.hpp"
#include "AnimationInstance.hpp"
#include "ModelNode.hpp"
#include <list>
#include <map>
#include <string>

class Gfx_ModelData526620;
class Gfx_ModelNode;
class Gfx_ModelMeshPkt;
class Gfx_SkinMeshPkt;
class Gfx_AnimationInstance;
class Gfx_Viewport;
class Gfx_Material;
class Gfx_Light;

class Gfx_ModelInstance;
struct Gfx_ModelAfterimage
{
	Gfx_ModelInstance* model;
	float remaining;
	BYTE color[4], flag; // Stored A/R/G/B bytes, followed by the original use-color BYTE.
	DWORD style; // Current render phase supplied by the afterimage owner.
};

class Gfx_ModelInstance : public Gfx_BaseObject
{
public:
	explicit Gfx_ModelInstance(Gfx_ModelData526620* data); // 0x4A0400.
	virtual ~Gfx_ModelInstance(); // 0x49F2C0.
	void SetAnimation(Gfx_AnimationInstance* animation, BYTE owns); // 0x49DD40/49D570.
	void SetPacketFlag1(BYTE value); // 0x49CED0.
	void SetPacketFlag2(BYTE value); // 0x49CEF0.
	void SetShadow(BYTE value); // 0x49CF10.
	void GetBounds(D3DXVECTOR3& minimum, D3DXVECTOR3& maximum); // 0x49CB10.
	void SetScale(float x, float y, float z); // 0x49CF30/507C30.
	void Detach(); // 0x49CF90.
	void AttachTo(Gfx_ModelInstance* parent, DWORD bone); // 0x49D290.
	void EnableMaterial(const std::string& name, BYTE enabled); // 0x49DD80.
	void EnableMaterialByIndex(LONG index, BYTE enabled); // 0x49DB40.
	Gfx_Material* GetMaterial(DWORD index); // 0x49CC40; lazy per-instance material copy.
	void ResetMaterialStates(); // 0x49CDD0; restores only existing material copies.
	void ResetMaterialValues(); // 0x49CC70; preserves instance color-merge flags.
	void SetMaterialColors(DWORD color, BYTE useRGB, BYTE useAlpha, const DWORD* emissive); //0x49CCA0.
	void SetRenderPhase(DWORD phase); //0x49CE10; current mode28 only, then regroup.
	void SetMaterialStyle(DWORD style); //0x49DB30/49D2F0; all node mesh/skin attributes.
	void SetMaterialFlagBC(BYTE value); // 0x49CE40; creates material copies as needed.
	void SetAmbientOverride(BYTE value); //49CE70, first mesh packet and skin.
	void SetAmbientColor(DWORD value); //49CEA0.
	D3DXMATRIX* GetWorldTransform(D3DXMATRIX& output, LONG semanticBone = 0) const; // 0x49CFD0.
	void UpdateAfterimages(float elapsed); // 0x4A0750; real snapshot/animation/material owners.
	void Submit(Gfx_Viewport* viewport, BYTE shadow, float value, DWORD color); // 0x4A0DD0.
	BYTE AddLight(Gfx_Light* light); //49DB50/49D610; source returns1 even forNULL.
	void RemoveLight(Gfx_Light* light); //49DB80/49D7C0.
	void FinishLights(); //49DB70/49D6F0.
	void ClearLights(); //49E130/49E040.
	Gfx_ModelData526620* data;
	Gfx_ModelNode* root;
	Gfx_AnimationInstance* animation;
	BYTE ownsAnimation; // Original constructor leaves this unwritten; binding establishes ownership.
	Gfx_ModelInstance* parent;
	DWORD parentBone, mask;
	D3DXVECTOR3 scale;
	Gfx_ModelMeshPkt* firstMesh;
	Gfx_SkinMeshPkt* firstSkin;
	BYTE afterimagesEnabled;
	float afterimageTime;
	std::list<Gfx_ModelAfterimage> afterimages;
	float afterimageLifetime, afterimagePeriod;
	DWORD value90;
	D3DXMATRIX* previousTransform;
	float previousTime;
	DWORD previousGroup, previousIndex, previousSubAnimation, valueA8;
	std::list<Gfx_ModelInstance*> borrowedModels;
	std::map<DWORD, Gfx_ModelInstance*> attachedModels;
	DWORD color;
	BYTE flagC8, flagC9;
private:
	Gfx_ModelInstance(const Gfx_ModelInstance&);
	Gfx_ModelInstance& operator=(const Gfx_ModelInstance&);
};

inline void Gfx_ModelInstance::SetAnimation(Gfx_AnimationInstance* value, BYTE owns)
{
	if (ownsAnimation) delete animation;
	animation = value;
	root->SetAnimation(value);
	ownsAnimation = owns;
}
