#include "startup/NetworkComponent.hpp"
#include "GameEntity.hpp"
#include "GameInventory.hpp"
#include "GameWeapon.hpp"
#include "GameActionAttack.hpp"
#include "GameActionPickUp.hpp"
#include "GameEquipmentActions.hpp"
#include "entity/Action.hpp"
#include "entity/EntityEnvironment.hpp"
#include "entity/Map.hpp"
#include "entity/VisualEffect.hpp"
#include "entity/World.hpp"
#include "graphics/AnimationInstance.hpp"
#include "graphics/ModelInstance.hpp"
#include "graphics/SystemInterface.hpp"
#include "graphics/VfxEffects.hpp"
#include "model/ModelInstance.hpp"
#include <string.h>
#include <stdexcept>

namespace F3
{
	BYTE GameInventory::IsEquipped(JE::Entity* entity) //469670.
	{
		if (!dynamic_cast<GameEntity*>(JE::GetEntityPtr(ownerEntityID))) return 0;
		std::list<DWORD> slots;
		if (!GetEquippedSlots(entity, slots)) return 0;
		GameWeapon* weapon = dynamic_cast<GameWeapon*>(entity);
		if (!weapon) return 1;
		return weapon->model && weapon->model->IsAttached();
	}

	void GameEntity::CollectEffectModels(std::list<Gfx_ModelInstance*>& output) //468640/468720.
	{
		JE::Entity::CollectEffectModels(output);
		JE::Inventory* source = inventory;
		if (!source) return;
		GameInventory* items = dynamic_cast<GameInventory*>(source);
		if (!items) return;
		for (std::list<JE::Entity*>::iterator i = items->items.begin(); i != items->items.end(); ++i)
		{
			JE::Entity* child = *i;
			if (items->IsEquipped(child) && child->model && child->model->GetModel(0))
				output.push_back(child->model->GetModel(0));
		}
	}

	BYTE GameEntity::IsVisible() //466570, adjustor467A80.
	{
		if (!JE::DynamicObject::IsVisible()) return 0;
		if (g_GfxSystem->FogOfWarEnabled(NULL) && (entityClass == 1 || entityClass == 5
			|| entityClass == 6 || entityClass == 7 || entityClass == 9))
			return IsFogVisible();
		return 1;
	}

	void GameEntity::OnClientFrame(DWORD elapsed) //466F40, adjustor467AC0.
	{
		JE::Entity::OnClientFrame(elapsed);
		if (JE::GetEntityEnvironment().RenderMode()) return;
		GameInventory* items = dynamic_cast<GameInventory*>(inventory);
		if (!items) return;
		for (std::list<JE::Entity*>::iterator i = items->items.begin(); i != items->items.end(); ++i)
			if (items->IsEquipped(*i)) (*i)->ClientFrame(elapsed);
	}

	void GameEntity::OnAnimationCue(const Gfx_AnimationCue& cue) //467E90, adjustor468300.
	{
		const DWORD mode = g_pNetworkInstance->GetMode();
		const char* name = cue.name.c_str();
		if ((mode & 1) && !_stricmp(name, "DM"))
		{
			GameActionAttack* attack=dynamic_cast<GameActionAttack*>(GetCurrentAction());
			if (attack) attack->receivedDamageCue=1; //467F26, raw flag; no damage execution here.
			return;
		}
		if (!_stricmp(name, "RL"))
		{
			GameActionAttack* attack=dynamic_cast<GameActionAttack*>(GetCurrentAction());
			if (attack) attack->OnReleaseCue(); //467F90, actual attack-mode VEG owner.
			return;
		}
		if (!_stricmp(name, "PU"))
		{
			GameActionPickUp* pickup=dynamic_cast<GameActionPickUp*>(GetCurrentAction());
			if (pickup) pickup->OnPickupCue(); //467FF7, no synthetic transfer on deadline.
			return;
		}
		if (!_stricmp(name, "EP"))
		{
			JE::Action* action=GetCurrentAction();
			if (GameActionEquip* equip=dynamic_cast<GameActionEquip*>(action)) equip->OnEquipCue();
			else if (GameActionSetActiveWeapon* weapon=dynamic_cast<GameActionSetActiveWeapon*>(action)) weapon->OnEquipCue();
			return;
		}
		if (!_stricmp(name, "UP"))
		{
			JE::Action* action=GetCurrentAction();
			if (GameActionUnequip* unequip=dynamic_cast<GameActionUnequip*>(action)) unequip->OnUnequipCue();
			else if (GameActionSetActiveWeapon* weapon=dynamic_cast<GameActionSetActiveWeapon*>(action)) weapon->OnUnequipCue();
			return;
		}
		const BYTE left = !_stricmp(name, "FL");
		if (!left && _stricmp(name, "FR")) return;
		JE::IWorld* world = JE::GetWorldInstance();
		JE::Map* owner = world->GetMap(mapHandle);
		if (!owner) return;
		JE::VisualEffect* effect = owner->AttachVisualEffect(left
			? "FX_FootL_Hum_Sand_Base_01.veg" : "FX_FootR_Hum_Sand_Base_01.veg", 0, 1);
		if (!effect || !model) return;
		Gfx_ModelInstance* selected = model->GetModel(0);
		if (!selected) return;
		D3DXMATRIX matrix;
		{
			const std::string snapBone;
			JE::GetVfxModelTransform(selected, left ? 12 : 7,
				snapBone, matrix, NULL, 1, 10.0f); //Original local+parameter owners;6FF0D8.
		} //Original destroys the local before publishing the effect transform.
		effect->SetTransform(1, &matrix);
	}
}
