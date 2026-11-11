#pragma once

#include "graphics/LineSystem.hpp"
#include <set>

class OCTREE;
class Gfx_Light;

namespace JE
{
	class DynamicObject
	{
	public:
		DynamicObject(); // 0x568350.
		virtual ~DynamicObject(); // 0x5682C0; misnamed ios_base in the database.
		virtual void Draw(); // Original+04, shared404B70 no-op.
		virtual void ClientFrame(DWORD elapsed) = 0; // Original+08, purecall.
		virtual BYTE AddLight(Gfx_Light* light); // Original+0C,591690 returns0.
		virtual void RemoveLight(Gfx_Light* light); // Original+10, shared5A9320.
		virtual void ClearLights(); // Original+14, shared404B70.
		virtual void FinishLights(); // Original+18, shared404B70.
		virtual BYTE IsVisible(); // Original+1C,568250; actual octree handle visibility.
		virtual void CopyFrom(DynamicObject* source); //568280, original+20; position/flag30 only.
		void ChangePosition(const D3DXVECTOR3& value); // 0x5680D0.
		void GetBounds(D3DXVECTOR3& low, D3DXVECTOR3& high) const; //4056B0, common local bounds, native RET8.
		int AttachToOctree(OCTREE* value); // 0x568140; preserves existing-handle reuse.
		void RemoveFromOctree(); // 0x568220.
		virtual void GetSpatialCenter(D3DXVECTOR3& output) const; // Base5682A0, zero.
		D3DXVECTOR3 position, minimum, maximum;
		OCTREE* octree;
		DWORD octreeHandle;
		BYTE flag30;
		std::set<DWORD> spatialLinks; // Original key-only node, nil byte+11h.
		Gfx_LineSystem debugLines;
	};
}
