#include "startup/NetworkComponent.hpp"
#include "network/EngineClient.hpp"
#include "startup/MessageLoop.hpp"
#include "Entity.hpp"
#include "EntityEnvironment.hpp"
#include "Map.hpp"
#include "MapFog.hpp"
#include "World.hpp"
#include "graphics/Octree.hpp"
#include "graphics/SystemInterface.hpp"
#include "graphics/VfxVeg.hpp"
#include "model/ModelCache.hpp"
#include "model/ModelInstance.hpp"
#include <stdexcept>

namespace JE
{
	void Entity::SubmitEffects(Gfx_Viewport* viewport) //54B4E0, actual virtual80.
	{
		if (static_cast<Client*>(F3::g_pNetworkInstance->client)->field0x104) return;
		for (std::map<VFX_VEGInstance*,EntityEffectBinding>::iterator i=activeEffects.begin();
			i!=activeEffects.end(); ++i)
			if (i->second.instance) i->second.instance->Draw(viewport);
	}

	void Entity::CollectEffectModels(std::list<Gfx_ModelInstance*>& output) //54FD90.
	{
		ModelInstance* owner = model;
		if (!owner) return;
		const LONG slots = g_pModelCache->slotCount;
		for (LONG slot = 1; slot < slots; ++slot)
		{
			Gfx_ModelInstance* source = owner->GetModel(slot);
			if (source) output.push_back(source);
		}
	}

	BYTE Entity::IsFogVisible()
	{
		Map* owner = g_World->GetMap(mapHandle);
		AreaMap* fog = owner->areaMap;
		if (!fog) return 0;
		D3DXVECTOR3 offset = position;
		if (model && model->IsAttached()) model->GetAttachedPosition(offset);
		const D3DXVECTOR3 low = minimum + offset, high = maximum + offset;
		LONG first, row, last, lastRow;
		fog->WorldToCell(low, &first, &row);
		fog->WorldToCell(high, &last, &lastRow);
		for (LONG y = lastRow; y <= row; ++y)
			for (LONG x = first; x <= last; ++x)
				if (fog->IsVisible(x, y)) return 1;
		return 0;
	}

	BYTE Entity::IsVisible() //54A110.
	{
		if (!DynamicObject::IsVisible()) return 0;
		if (g_GfxSystem->FogOfWarEnabled(NULL) && !IsSelectionDisabled())
		{
			IWorld* world = g_World;
			Map* owner = world->GetMap(mapHandle);
			AreaMap* fog = owner->areaMap;
			if (!fog) return 0;
			D3DXVECTOR3 low, high;
			D3DXVECTOR3 offset;
			if (model && model->IsAttached())
			{
				model->GetAttachedPosition(high);
				offset.x = high.x; offset.y = high.y; offset.z = high.z;
			}
			else { offset.x = position.x; offset.y = position.y; offset.z = position.z; }
			GetBounds(low, high);
			low.x += offset.x; low.y += offset.y; low.z += offset.z;
			high.x += offset.x; high.y += offset.y; high.z += offset.z;
			LONG first, row, last, lastRow;
			fog->WorldToCell(low.x, low.y, low.z, &first, &row);
			fog->WorldToCell(high.x, high.y, high.z, &last, &lastRow);
			for (LONG y = lastRow; y <= row; ++y)
				for (LONG x = first; x <= last; ++x)
					if (fog->IsVisible(x, y)) return 1;
			return 0;
		}
		return 1;
	}

	BYTE Entity::IsUnderFogOfWar() //54A2C0, actual virtual+88.
	{
		if (g_GfxSystem->FogOfWarEnabled(NULL))
		{
			Map* owner = GetWorldInstance()->GetMap(mapHandle);
			AreaMap* fog = owner->areaMap;
			if (fog)
			{
				float x, y, z;
				if (model && model->IsAttached())
				{
					D3DXVECTOR3 attached;
					model->GetAttachedPosition(attached);
					x = attached.x; y = attached.y; z = attached.z;
				}
				else { x = position.x; y = position.y; z = position.z; }
				D3DXVECTOR3 low = minimum;
				low.x += x; low.y += y; low.z += z;
				D3DXVECTOR3 high = maximum;
				high.x += x; high.y += y; high.z += z;
				LONG first, row, last, lastRow;
				fog->WorldToCell(low.x, low.y, low.z, &first, &row);
				fog->WorldToCell(high.x, high.y, high.z, &last, &lastRow);
				for (LONG currentRow = lastRow; currentRow <= row; ++currentRow)
					for (LONG currentFirst = first; currentFirst <= last; ++currentFirst)
						if (fog->IsVisible(currentFirst, currentRow)) return 0;
			}
			return 1;
		}
		return 0;
	}

	void Entity::Draw() //54DEA0; same entry in GameEntity's Entity table.
	{
		if (!renderEnabled || !IsVisible()) return;
		if (!model && !modelAttempted)
		{
			SetModel(modelName.c_str(), 0);
			modelAttempted = 1;
			if (model)
			{
				Map* owner = NULL;
				DWORD mapID = mapHandle;
				if (mapID) owner = GetWorldInstance()->GetMap(mapID);
				RotateBounds();
				if (owner) AttachToOctree(owner->GetOctree());
				OnModelLoaded();
			}
			else RemoveFromOctree();
		}
		if (model)
		{
			F3::GetElapsedTimeMS(); //56AF50, original captured clock read; result unused.
			if (state == 5 && value368 != 1)
				model->Submit(NULL, 1, 0.055f, 0xFFFF0000);
			else model->Submit(NULL, 0, 0.05f, 0xFF00FFFF);
		}
		DynamicObject::Draw(); //54DF9F calls the genuine shared404B70 base method.
	}
}
