#include "GameInventory.hpp"
#include "GameItem.hpp"
#include "GameCreature.hpp"
#include "entity/World.hpp"
#include "entity/WorldCollections.hpp"
#include "model/ModelInstance.hpp"
#include <string.h>
#include <stdlib.h>
#include <stdexcept>

namespace F3
{
	void GameInventory::FindItemsByResource(const std::string& name,
		std::list<JE::Entity*>& equippedItems, std::list<JE::Entity*>& unequippedItems)
	{
		//46A6C0 uses ordinary slots, not469670's weapon-model attachment test.
		equippedItems.clear();
		unequippedItems.clear();
		for (std::list<JE::Entity*>::iterator i=items.begin(); i!=items.end(); ++i)
		{
			JE::Entity* item=*i;
			if (!::_stricmp(item->resourceName.c_str(), name.c_str()))
			{
				std::list<DWORD> slots;
				if (GetEquippedSlots(item, slots)) equippedItems.push_back(item);
				else unequippedItems.push_back(item);
			}
		}
	}

	GameInventory::GameInventory()
	{
		memset(equipped, 0, sizeof(equipped));
		value44 = 0;
		value48 = -1;
	}

	BYTE GameInventory::ContainsID(DWORD id) //4692D0.
	{
		JE::Entity* entity=JE::GetEntityPtr(id);
		if (entity)
		{
			JE::IWorld* world=JE::g_World;
			JE::Party* party=world->GetParty();
			if (party->Contains(JE::GetEntityPtr(ownerEntityID))==1)
			{
				JE::IWorld* registryWorld=JE::g_World;
				if (registryWorld->GetItemRegistry()->Contains(entity->displayName)==1) return 1;
			}
		}
		return JE::Inventory::ContainsID(id);
	}

	void GameInventory::PackState(JE::Net::Packet& packet) //469370, original+20.
	{
		JE::Inventory::PackState(packet);
	}

	void GameInventory::UnpackState(JE::Net::Packet& packet) //469370, original+24.
	{
		JE::Inventory::UnpackState(packet);
	}

	void GameInventory::OnAdd(JE::Entity* entity) //469400.
	{
		JE::Inventory::OnAdd(entity);
	}

	GameItem* GameInventory::GetItem(DWORD slot)
	{
		GameItem* item=NULL;
		JE::Entity* entity=equipped[slot];
		if (entity) item=dynamic_cast<GameItem*>(entity);
		return item;
	}

	void __stdcall GameInventory::BuildItemIndices(DWORD* indices, GameItem* item,
		JE::ModelInstance* model)
	{
		if (!indices) return;
		memset(indices, 0xFF, 13 * sizeof(DWORD));
		if (!item) return;
		for (LONG slot = 1; slot < 13; ++slot)
		{
			char name[260];
			strcpy(name, item->GetSocketModel(slot).c_str());
			_strupr(name);
			if (strlen(name))
			{
				DWORD variation;
				BYTE found;
				{
					std::string normalized(name);
					found = model->FindSocketModel(slot, normalized, &variation);
				}
				if (found) indices[slot] = variation;
			}
		}
	}

	void __stdcall GameInventory::BuildCreatureIndices(DWORD* indices, GameCreature* creature,
		JE::ModelInstance* model)
	{
		if (!indices) return;
		memset(indices, 0xFF, 13 * sizeof(DWORD));
		if (!creature) return;
		for (LONG slot = 1; slot < 13; ++slot)
		{
			char name[260];
			strcpy(name, creature->GetSocketModel(slot).c_str());
			_strupr(name);
			if (strlen(name))
			{
				DWORD variation;
				BYTE found;
				{
					std::string normalized(name);
					found = model->FindSocketModel(slot, normalized, &variation);
				}
				if (found) indices[slot] = variation;
			}
		}
	}

	void GameInventory::ApplyItemAppearance(GameItem* item, DWORD bone, DWORD slot)
	{
		JE::Entity* owner = JE::GetEntityPtr(ownerEntityID);
		JE::ModelInstance* model = owner->model; // Original requires a valid owner ID.
		if (!model) return;
		GameCreature* creature = dynamic_cast<GameCreature*>(owner);
		GameItem* firstItem = NULL;
		GameItem* secondItem = NULL;
		if (!slot)
		{
			firstItem = GetItem(1);
			secondItem = GetItem(2);
		}
		DWORD selected[13], other1[13], other2[13], defaults[13];
		BuildItemIndices(selected, item, model);
		BuildItemIndices(other1, firstItem, model);
		BuildItemIndices(other2, secondItem, model);
		BuildCreatureIndices(defaults, creature, model);
		if (item->hideHair) model->RemoveSocket(2);
		if (item->hideBeard) model->RemoveSocket(5);
		if (item->hideMoustache) model->RemoveSocket(4);
		if (item->hideEyes) model->RemoveSocket(6);
		if (item->hidePonytail) model->RemoveSocket(3);
		if (item->hideVanity) model->RemoveSocket(12);
		for (LONG i = 0; i < 13; ++i)
			if (selected[i] != DWORD(-1) && other1[i] == DWORD(-1) && other2[i] == DWORD(-1))
				model->SetSocket(i, selected[i], NULL);
		std::string held = item->heldModel;
		if (!held.empty() && bone)
		{
			item->SetModel(held.c_str(), 0);
			item->SetMap(owner->mapHandle);
			if (item->model) model->Attach(item->model, bone);
		}
		item->AttachEffectsToMap(owner->mapHandle);
	}

	void GameInventory::ReapplyEquipment()
	{
		for (DWORD slot = 0; slot < 6; ++slot)
		{
			GameItem* item = GetItem(slot);
			if (item) ApplyItemAppearance(item, 0, slot);
		}
	}

	void GameInventory::RemoveItemAppearance(GameItem* item, DWORD group)
	{
		item->DetachEffectsFromMap();
		item->SetMap(0);
		std::string unused;
		JE::Entity* owner = JE::GetEntityPtr(ownerEntityID);
		if (!owner || !owner->model) return;
		JE::ModelInstance* model = owner->model;
		GameCreature* creature = dynamic_cast<GameCreature*>(owner);
		BYTE restore[6] = {item->hideHair, item->hideBeard, item->hideMoustache,
			item->hideEyes, item->hidePonytail, item->hideVanity};
		GameItem* other1 = NULL;
		GameItem* other2 = NULL;
		if (!group) { other1 = GetItem(1); other2 = GetItem(2); }
		else if (group <= 3) other1 = GetItem(0);
		DWORD selected[13], first[13], second[13], defaults[13];
		BuildItemIndices(selected, item, model);
		BuildItemIndices(first, other1, model);
		BuildItemIndices(second, other2, model);
		BuildCreatureIndices(defaults, creature, model);
		GameItem* others[2] = {other1, other2};
		for (DWORD i = 0; i < 2; ++i) if (others[i])
		{
			if (restore[0]) restore[0] = others[i]->hideHair == 0;
			if (restore[1]) restore[1] = others[i]->hideBeard == 0;
			if (restore[2]) restore[2] = others[i]->hideMoustache == 0;
			if (restore[3]) restore[3] = others[i]->hideEyes == 0;
			if (restore[4]) restore[4] = others[i]->hidePonytail == 0;
			if (restore[5]) restore[5] = others[i]->hideVanity == 0;
		}
		for (DWORD i = 0; i < 13; ++i) if (selected[i] != DWORD(-1))
		{
			DWORD variation = first[i] != DWORD(-1) ? first[i]
				: second[i] != DWORD(-1) ? second[i] : defaults[i];
			if (variation == DWORD(-1)) model->RemoveSocket(i);
			else model->SetSocket(i, variation, NULL);
		}
		std::string texture;
		if (creature)
		{
			if (restore[0])
			{
				const DWORD variation = defaults[2];
				if (variation != DWORD(-1))
				{
					texture = creature->GetSocketTexture(2);
					model->SetSocket(2, variation, texture.c_str());
				}
			}
			if (restore[1])
			{
				const DWORD variation = defaults[5];
				if (variation != DWORD(-1))
				{
					texture = creature->GetSocketTexture(5);
					model->SetSocket(5, variation, texture.c_str());
				}
			}
			if (restore[2])
			{
				const DWORD variation = defaults[4];
				if (variation != DWORD(-1))
				{
					texture = creature->GetSocketTexture(4);
					model->SetSocket(4, variation, texture.c_str());
				}
			}
			if (restore[4])
			{
				const DWORD variation = defaults[3];
				if (variation != DWORD(-1))
				{
					texture = creature->GetSocketTexture(3);
					model->SetSocket(3, variation, texture.c_str());
				}
			}
			if (restore[3] && defaults[6] != DWORD(-1))
				model->SetSocket(6, defaults[6], NULL);
			if (restore[5] && defaults[12] != DWORD(-1))
				model->SetSocket(12, defaults[12], NULL);
		}
		std::string held = item->heldModel;
		if (!held.empty() && owner->model && item->model) owner->model->DetachChild(item->model);
	}
}
