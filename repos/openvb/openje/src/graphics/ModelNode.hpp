#pragma once

#include <d3dx8.h>
#include <vector>
#include "ModelMesh.hpp"

class Gfx_ModelData526620;
class Gfx_AnimationInstance;
class Gfx_Viewport;
class Gfx_Light;

// Nonvirtual scene owner49F150. Normal C++ containers own attributes/children.
class Gfx_ModelNode
{
public:
	Gfx_ModelNode();
	~Gfx_ModelNode(); // 0x49ECA0; attributes before children, both forward.
	std::vector<Gfx_NodeAttribute*> GetAttributes() const; //565AA0; owned vector snapshot.
	std::vector<Gfx_ModelNode*> GetChildren() const; //565AD0; owned vector snapshot.
	void SetRotation(const D3DXQUATERNION& value); // 0x49D1B0.
	void SetTransform(const D3DXMATRIX& value); // 0x49D210.
	Gfx_NodeAttribute* Clone(const Gfx_ModelNode& source); // 0x49FAE0; mesh families.
	Gfx_NodeAttribute* Initialize(DWORD kind, std::vector<DWORD>& textures, DWORD shadow); // 0x49F680.
	void SetAnimation(Gfx_AnimationInstance* animation); // 0x49D570.
	void Update(float elapsed); // 0x49DAD0; attributes, then children.
	void AddLight(Gfx_Light* light); //49D610; captured attribute/child counts.
	void RemoveLight(Gfx_Light* light); //49D7C0; remove first per packet.
	void FinishLights(); //49D6F0; static sort, skin callback, then children.
	void ClearLights(); //49E040; static and skin collections, then children.
	void SetMaterialStyle(DWORD style); //0x49D2F0, recursive per-instance overrides.
	void EnableMaterialByIndex(LONG index, BYTE enabled); // 0x49D420.
	void Submit(DWORD mask, const D3DXMATRIX* parentMatrix, Gfx_Viewport* viewport,
		BYTE shadow, float value, DWORD color); // 0x49D8A0.
	Gfx_ModelData526620* owner;
	Gfx_ModelNode* root;
	Gfx_ModelNode* parent;
	BYTE dirty;
	D3DXQUATERNION rotation;
	D3DXVECTOR3 position;
	D3DXMATRIX localTransform, worldTransform;
	std::vector<Gfx_NodeAttribute*> attributes;
	std::vector<Gfx_ModelNode*> children;
	D3DXVECTOR3 minimum, maximum, subtreeMinimum, subtreeMaximum;

private:
	Gfx_ModelNode(const Gfx_ModelNode&);
	Gfx_ModelNode& operator=(const Gfx_ModelNode&);
};
