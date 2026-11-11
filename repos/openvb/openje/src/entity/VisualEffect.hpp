#pragma once

#include "DynamicObject.hpp"

namespace JE
{
	class VFX_VEGInstance;
	class VFX_Light;
	class VisualEffect : public DynamicObject
	{
	public:
		VisualEffect(DWORD owner, VFX_VEGInstance* input, BYTE release, DWORD map); // 0x59FCB0.
		virtual ~VisualEffect(); // 0x59F6F0.
		DWORD GetMap() const; // 0x59F8E0.
		void UpdateBounds(); // 0x59F950.
		void SetTransform(BYTE enabled, const D3DXMATRIX* matrix); // 0x59F850.
		void SetEnabled(BYTE enabled); // 0x59F890.
		void Update(DWORD elapsed); //59FDB0, milliseconds accumulated as float seconds.
		virtual void ClientFrame(DWORD elapsed); //59FDB0, actual dynamic-object frame callback.
		virtual void Draw(); //59FD70, actual default-viewport draw path.
		virtual BYTE AddLight(Gfx_Light* light); //59F7F0.
		virtual void RemoveLight(Gfx_Light* light); //59F810.
		virtual void ClearLights(); //59F830.
		virtual void FinishLights(); //59F840.
		BYTE IsFinished() const; //59F7D0.
		virtual BOOL IsSelectionDisabled() const; //59F760, slot28; selectable BYTE equals0.
		BOOL IsEffectVisible(); //59FC70, cached fog visibility; distinct from virtual octree visibility.
		void ResetVisibility(); //59F8D0, clears both cached result bytes.
		void CollectLights(std::list<VFX_Light*>& output); //59FF20, actual child RTTI.
		DWORD ownerID, explicitMap;
		VFX_VEGInstance* instance;
		BYTE releaseInstance;
		float elapsedSeconds; //Original+16C.
		BYTE selectable;
		DWORD value174, value178, value17C;
		BYTE flag180;
		D3DXMATRIX transform; // +184, unwritten until SetTransform succeeds.
		BYTE flag1C4, flag1C5, flag1C6, flag1C7;
	private:
		void UpdateVisibility(); //59FA40, updates the cached fog result.
	};
}
