#include "network/EngineClient.hpp"
#include "game/InventoryInterface.hpp"
#include "game/GameInventory.hpp"
#include "game/GameEntity.hpp"
#include "game/GameItem.hpp"
#include "graphics/ClientCursor.hpp"
#include "startup/NetworkComponent.hpp"
#include "gui/Label.hpp"
#include "gui/Picture.hpp"
#include "gui/ScrollBar.hpp"
#include <cstdio>
#include <list>
#include <stdexcept>

namespace F3
{
	namespace
	{
		GameCursorManager& InventoryCursor()
		{
			JE::Client* client = g_pNetworkInstance ? dynamic_cast<JE::Client*>(g_pNetworkInstance->client) : NULL;
			GameCursorManager* cursor = client ? dynamic_cast<GameCursorManager*>(client->child0xA4) : NULL;
			return *cursor;
		}

	}

	LONG InventoryInterface::MaxBackpackIndex() //43AC40.
	{
		GameInventory* inventory = GetInventory();
		if (!inventory) return -1;
		LONG maximum = -1;
		for (std::list<JE::Entity*>::iterator it = inventory->items.begin(); it != inventory->items.end(); ++it)
		{
			JE::Entity* entity = *it;
			GameEntity* game = dynamic_cast<GameEntity*>(entity);
			if (entity && game)
			{
				std::list<DWORD> slots;
				if (!inventory->GetEquippedSlots(entity, slots) && LONG(game->value8C) > maximum)
					maximum = LONG(game->value8C);
			}
		}
		return maximum;
	}

	void InventoryInterface::RefreshGridBindings() //43B2C0.
	{
		const LONG firstRow = backpackScroll->GetValue();
		for (LONG row = 0; row < 33; ++row)
		{
			for (LONG column = 0; column < 3; ++column)
			{
				InventorySlot* slot = backpackSlots[row][column];
				if (row < firstRow || row >= firstRow + 6)
				{
					slot->picture = NULL;
					slot->highlight = NULL;
				}
				else
				{
					char name[256];
					std::sprintf(name, "picSlot%02d%02d", row - firstRow, column);
					slot->picture = static_cast< ::GUI::Picture*>(FindWindow(name));
					std::sprintf(name, "picSlot%02d%02dHighlight", row - firstRow, column);
					slot->highlight = static_cast< ::GUI::Picture*>(FindWindow(name));
				}
			}
		}
	}

	void InventoryInterface::PopulateItems() //43EE60.
	{
		ClearSlotItems();
		GameInventory* inventory = GetInventory();
		if (!inventory) return;
		GameCursorManager& cursor = InventoryCursor();
		const DWORD draggedItem = cursor.context == 14 ? cursor.draggedItemID : 0;
		for (std::list<JE::Entity*>::iterator it = inventory->items.begin(); it != inventory->items.end(); ++it)
		{
			JE::Entity* entity = *it;
			if (entity)
			{
				std::list<DWORD> slots;
				if (inventory->GetEquippedSlots(entity, slots) && !slots.empty())
				{
					while (!slots.empty())
					{
						const DWORD index = slots.front();
						if (equipmentSlots[index]) equipmentSlots[index]->Bind(entity->entityID, entity->inventoryOwnerID);
						slots.pop_front();
					}
				}
			}
		}
		for (std::list<JE::Entity*>::iterator it = inventory->items.begin(); it != inventory->items.end(); ++it)
		{
			JE::Entity* entity = *it;
			GameEntity* game = dynamic_cast<GameEntity*>(entity);
			if (entity && game)
			{
				std::list<DWORD> slots;
				if (!inventory->GetEquippedSlots(entity, slots) && game->value8C != DWORD(-1))
				{
					const LONG index = LONG(game->value8C);
					InventorySlot* slot = backpackSlots[index / 3][index % 3];
					slot->Bind(entity->entityID, entity->inventoryOwnerID);
					if (entity->entityID == draggedItem && slot->picture) slot->SetState(1);
				}
			}
		}
		for (std::list<JE::Entity*>::iterator it = inventory->items.begin(); it != inventory->items.end(); ++it)
		{
			JE::Entity* entity = *it;
			GameEntity* game = dynamic_cast<GameEntity*>(entity);
			if (entity && game)
			{
				std::list<DWORD> slots;
				if (!inventory->GetEquippedSlots(entity, slots) && game->value8C == DWORD(-1))
				{
					game->value8C = DWORD(MaxBackpackIndex()) + 1;
					SortItems(4, 0); //Original recursive refresh, not a separate assignment pass.
				}
			}
		}
	}

	void InventoryInterface::SortItems(DWORD kind, LONG firstOrder) //43E760.
	{
		if (kind >= 1 && kind <= 3)
			sortLabels[0]->SetStringReference(sortLabels[kind]->stringReference);
		GameInventory* inventory = GetInventory();
		if (!inventory) return;
		std::list<GameEntity*> sorted;
		for (std::list<JE::Entity*>::iterator it = inventory->items.begin(); it != inventory->items.end(); ++it)
		{
			JE::Entity* entity = *it;
			GameEntity* game = dynamic_cast<GameEntity*>(entity);
			if (entity && game)
			{
				std::list<DWORD> slots;
				if (!inventory->GetEquippedSlots(entity, slots))
				{
					std::list<GameEntity*>::iterator where = sorted.begin();
					switch (kind)
					{
					case 1: //Insert before first equal class; no numeric class ordering.
						while (where != sorted.end() && game->entityClass != (*where)->entityClass) ++where;
						sorted.insert(where, game);
						break;
					case 2: //Unsigned original word at GameEntity+1C, not a string comparison.
						while (where != sorted.end() && game->stringRefs[5] < (*where)->stringRefs[5]) ++where;
						sorted.insert(where, game);
						break;
					case 3:
						while (where != sorted.end() && game->count956 < (*where)->count956) ++where;
						sorted.insert(where, game);
						break;
					case 4:
						if (game->value8C == DWORD(-1)) sorted.push_back(game);
						else
						{
							while (where != sorted.end() && LONG(game->value8C) >= LONG((*where)->value8C)) ++where;
							sorted.insert(where, game);
						}
						break;
					}
				}
			}
		}
		DWORD index = DWORD(firstOrder);
		for (std::list<GameEntity*>::iterator it = sorted.begin(); it != sorted.end(); ++it)
			(*it)->value8C = index++;
		if (kind == 4) backpackScroll->SetMaximum((LONG(sorted.size()) - 1) / 3 + 1);
		RefreshGridBindings();
		PopulateItems();
	}

	void InventoryInterface::RefreshEquipmentAvailability() //43F280.
	{
		GameInventory* inventory = GetInventory();
		GameCursorManager& cursor = InventoryCursor();
		std::list<DWORD> available;
		if (cursor.draggedItemID)
		{
			JE::Entity* entity = inventory->FindByID(cursor.draggedItemID);
			GameEntity* game = dynamic_cast<GameEntity*>(entity);
			GameItem* item = dynamic_cast<GameItem*>(game);
			const std::list<DWORD> choices(item->values168);
			for (std::list<DWORD>::const_iterator it = choices.begin(); it != choices.end(); ++it)
			{
				switch (*it)
				{
				case 0: available.push_back(0); break;
				case 1: available.push_back(1); break;
				case 2: available.push_back(2); available.push_back(3); break;
				}
			}
		}
		for (int i = 0; i < 6; ++i) equipmentSlots[i]->SetVisible(0);
		for (std::list<DWORD>::iterator it = available.begin(); it != available.end(); ++it)
			equipmentSlots[*it]->SetVisible(1);
	}
}
