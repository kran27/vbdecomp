#include "startup/NetworkComponent.hpp"
#include "network/EngineClient.hpp"
#include "VisualEffect.hpp"
#include "Entity.hpp"
#include "World.hpp"
#include "Map.hpp"
#include "MapFog.hpp"
#include "EntityEnvironment.hpp"
#include "model/ModelInstance.hpp"
#include "graphics/VfxVeg.hpp"
#include "graphics/VfxLight.hpp"
#include "graphics/SystemInterface.hpp"
#include <stdexcept>
#include <cstring>

namespace JE
{
	VisualEffect::VisualEffect(DWORD owner, VFX_VEGInstance* input, BYTE release, DWORD map)
	{
		value174 = 0;
		value178 = 0;
		value17C = 0;
		explicitMap = map;
		ownerID = owner;
		releaseInstance = release;
		instance = input;
		elapsedSeconds = 0;
		flag30 = 0;
		selectable = 1;
		flag180 = 0;
		flag1C4 = 1;
		flag1C5 = 0;
		flag1C6 = 0;
		flag1C7 = 0;
		if (instance) flag30 = instance->GetFlag();
		UpdateBounds();
	}
	VisualEffect::~VisualEffect()
	{
		if (instance && releaseInstance) GetVfxVegCache().Release(instance);
	}
	DWORD VisualEffect::GetMap() const
	{
		DWORD map = 0;
		if (ownerID)
		{
			for (Entity* entity = GetEntityPtr(ownerID); entity;
				entity = GetEntityPtr(entity->inventoryOwnerID))
			{
				if (map) break;
				map = entity->mapHandle;
			}
		}
		else return explicitMap;
		return map;
	}
	void VisualEffect::SetTransform(BYTE enabled,const D3DXMATRIX* matrix)
	{
		if(enabled && matrix) { flag180=1; memcpy(&transform,matrix,sizeof(transform)); }
		else flag180=0;
	}
	void VisualEffect::SetEnabled(BYTE enabled)
	{
		if((flag1C4!=0)==(enabled!=0)) return;
		flag1C4=enabled;
		if(instance) instance->SetEnabled(enabled);
	}
	BOOL VisualEffect::IsSelectionDisabled() const
	{
		return selectable == 0;
	}
	BYTE VisualEffect::IsFinished() const { if (!instance) return 1; return instance->IsFinished(); }
	void VisualEffect::ResetVisibility() //59F8D0.
	{
		flag1C6 = 0;
		flag1C7 = 0;
	}
	void VisualEffect::CollectLights(std::list<VFX_Light*>& output) //59FF20.
	{
		if (static_cast<Client*>(F3::g_pNetworkInstance->client)->field0x104 || !BYTE(IsEffectVisible())) return;
		if (!instance) return;
		const LONG count = static_cast<LONG>(instance->effects.size());
		for (LONG i = 0; i < count; ++i)
			if (VFX_Light* light = dynamic_cast<VFX_Light*>(instance->GetEffect(i))) output.push_back(light);
	}
	void VisualEffect::UpdateVisibility()
	{
		if (flag1C6) return;
		flag1C6 = 1;
		flag1C7 = 1;
		if (!g_GfxSystem->FogOfWarEnabled(NULL) || BYTE(IsSelectionDisabled())) return;
		if (ownerID)
		{
			Entity* owner = GetEntityPtr(ownerID);
			if (!owner) return;
			if (!owner->renderEnabled) { flag1C7 = 0; return; }
			const DWORD map = GetMap();
			if (!map) return;
			Map* mapOwner = GetWorldInstance()->GetMap(map);
			AreaMap* const area = mapOwner->areaMap;
			if (!area) return;
			flag1C7 = 0;
			D3DXVECTOR3 low, high;
			GetBounds(low, high);
			LONG first, last, firstRow, lastRow;
			area->WorldToCell(low.x, low.y, low.z, &first, &lastRow);
			area->WorldToCell(high.x, high.y, high.z, &last, &firstRow);
			for (LONG row = firstRow; row <= lastRow; ++row)
				for (LONG cell = first; cell <= last; ++cell)
					if (area->IsVisible(cell, row)) { flag1C7 = 1; return; }
		}
		else
		{
			const DWORD map = explicitMap;
			if (!map) return;
			Map* mapOwner = GetWorldInstance()->GetMap(map);
			if (!mapOwner) return;
			AreaMap* const area = mapOwner->areaMap;
			if (!area) return;
			flag1C7 = 0;
			D3DXVECTOR3 low, high;
			GetBounds(low, high);
			LONG first, last, firstRow, lastRow;
			area->WorldToCell(low.x, low.y, low.z, &first, &lastRow);
			area->WorldToCell(high.x, high.y, high.z, &last, &firstRow);
			for (LONG row = firstRow; row <= lastRow; ++row)
				for (LONG cell = first; cell <= last; ++cell)
					if (area->IsVisible(cell, row)) { flag1C7 = 1; return; }
		}
	}
	BOOL VisualEffect::IsEffectVisible()
	{
		if (!flag1C6) UpdateVisibility();
		if (flag1C6 && !flag1C7) return 0;
		return 1;
	}
	void VisualEffect::Draw()
	{
		if(static_cast<Client*>(F3::g_pNetworkInstance->client)->field0x104 || !BYTE(VisualEffect::IsEffectVisible())) return;
		if(instance) instance->Draw(NULL);
		DynamicObject::Draw(); //59FDA4 tailcalls the actual shared404B70 base hook.
	}
	BYTE VisualEffect::AddLight(Gfx_Light* light) //59F7F0.
	{
		VFX_VEGInstance* current = instance;
		if (current) return current->AddLight(light);
		return 0;
	}
	void VisualEffect::RemoveLight(Gfx_Light* light) //59F810.
	{
		VFX_VEGInstance* current = instance;
		if (current) current->RemoveLight(light);
	}
	void VisualEffect::ClearLights() //59F830.
	{
		VFX_VEGInstance* current = instance;
		if (current) current->ClearLights();
	}
	void VisualEffect::FinishLights() //59F840.
	{
		VFX_VEGInstance* current = instance;
		if (current) current->FinishLights();
	}
	void VisualEffect::Update(DWORD elapsed)
	{
		elapsedSeconds = elapsed * 0.001f + elapsedSeconds;
		if(static_cast<Client*>(F3::g_pNetworkInstance->client)->field0x104==2) return;
		if(!flag1C4)
		{
			if(!flag1C5) return;
			if(!instance || instance->IsFinished())
			{
				RemoveFromOctree(); flag1C5=0; return;
			}
		}
		if(instance)
		{
			std::list<Gfx_ModelInstance*> extraSources;
			Gfx_ModelInstance* source = NULL;
			const D3DXMATRIX* sourceTransform = NULL;
			Entity* owner = GetEntityPtr(ownerID);
			VFX_Rotation firstValue, secondValue; //Original two unconsumed12B zero-value lifetimes.
			if(owner)
			{
				if(owner->model) source = owner->model->GetModel(0); //563900.
				owner->CollectEffectModels(extraSources); //Actual virtual+9C.
			}
			else if(flag180) sourceTransform = &transform;
			instance->Update(elapsedSeconds,source,NULL,&extraSources,sourceTransform,NULL);
		}
		UpdateBounds();
	}
	void VisualEffect::ClientFrame(DWORD elapsed)
	{
		Update(elapsed);
	}
	void VisualEffect::UpdateBounds()
	{
		if (!instance) return;
		D3DXVECTOR3 boundMinimum, boundMaximum;
		instance->GetBounds(boundMinimum, boundMaximum);
		minimum = boundMinimum;
		maximum = boundMaximum;
		D3DXVECTOR3 center(
			(boundMaximum.x - boundMinimum.x) * 0.5f + boundMinimum.x,
			(boundMaximum.y - boundMinimum.y) * 0.5f + boundMinimum.y,
			(boundMaximum.z - boundMinimum.z) * 0.5f + boundMinimum.z);
		ChangePosition(center);
		DWORD map = GetMap();
		if (map)
		{
			IWorld* world = g_World;
			Map* owner = world->GetMap(map);
			if (owner) { AttachToOctree(owner->GetOctree()); flag1C5=1; }
		}
	}
}
