#pragma once

#include <d3dx8.h>
#include <string>
#include <vector>
#include <list>

class Gfx_ModelInstance;
class Gfx_AnimationObject;
class Gfx_AnimationInstance;
class Gfx_Viewport;
class Gfx_Light;

namespace JE
{
	class ModelInfo;
	class ModelInstance
	{
	public:
		ModelInstance(std::string name, DWORD unused, BYTE loadAnimations); // 0x564CE0.
		virtual ~ModelInstance(); // 0x5644C0.
		virtual void SetSocket(DWORD slot, DWORD variation, const char* texture); // 0x565810.
		virtual void GetBounds(D3DXVECTOR3& minimum, D3DXVECTOR3& maximum); // 0x563950.
		std::string GetSocketModelName(DWORD slot, DWORD variation); //5648D0, actual info forwarding.
		std::string GetSocketTextureName(DWORD slot, DWORD index, const char* variation); //566FB0.
		BYTE FindSocketModel(DWORD slot, const std::string& name, DWORD* variation); // 0x564F20.
		void RemoveSocket(DWORD slot); // 0x563CB0.
		void SetSocketModel(DWORD slot, const char* texture); // 0x564F90; stitched creature atlas.
		void SaveSocketTextureName(DWORD slot, std::string texture); // 0x564F30.
		void Detach(ModelInstance* child); //563910, explicit child; original receiver unused.
		void DetachAll(); // 0x564460; borrowed attached models.
		void Attach(ModelInstance* child, DWORD bone); // 0x565B00; does not acquire ownership.
		void DetachChild(ModelInstance* child); // 0x564400; removes the first matching entry.
		void GetPosition(D3DXVECTOR3& position); // 0x563980.
		void SetTransform(const D3DXMATRIX& transform); // 0x563FF0.
		void SetPosition(float x, float y, float z); // 0x563E30.
		void SetShadow(BYTE value); // 0x563F90.
		void SetRotation(const D3DXVECTOR3& degrees); // 0x564050.
		void UpdateAnimation(DWORD milliseconds); // 0x563EB0; absolute clock.
		void ProcessAnimationCues(DWORD milliseconds); // 0x5643A0, current/attached owners.
		void ResetMaterialStates(); // 0x563DD0; slots, then attached owners.
		void SetMaterialFlagBC(BYTE value); // 0x564280; raw material byte, recursive.
		void SetAmbientOverride(BYTE value); //5642E0, slots then attached owners.
		void SetAmbientColor(DWORD value); //564340.
		Gfx_ModelInstance* GetModel(DWORD slot); // 0x563900; unchecked borrowed slot.
		BYTE IsAttached(); // 0x563930.
		void GetAttachedPosition(D3DXVECTOR3& position); // 0x563A40.
		void UpdateAfterimages(DWORD elapsedMilliseconds); // 0x563F10; elapsed, unlike animation clock.
		void Submit(Gfx_Viewport* viewport, BYTE shadow, float value, DWORD color); // 0x563D30.
		BYTE AddLight(Gfx_Light* light); //5640E0; all slots and attached owners.
		void RemoveLight(Gfx_Light* light); //564160.
		void FinishLights(); //5641C0.
		void ClearLights(); //564220.
		ModelInfo* info;
		Gfx_AnimationObject* animationObject;
		Gfx_AnimationInstance* animation;
		Gfx_ModelInstance** instances; // Owned slotCount-element pointer array.
		DWORD* variations; // Owned slotCount-element array.
		DWORD textureHandle;
		std::string name;
		D3DXVECTOR3 scale;
		std::list<ModelInstance*> attached;
		std::string* textureNames; // Owned slotCount-element array.
	private:
		ModelInstance(const ModelInstance&);
		ModelInstance& operator=(const ModelInstance&);
	};
}
