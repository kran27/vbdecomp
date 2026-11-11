#include "game/InventoryInterface.hpp"
#include "game/GameInventory.hpp"
#include "game/GameEntity.hpp"
#include "game/GameCreature.hpp"
#include "game/GameArmor.hpp"
#include "game/GameWeapon.hpp"
#include "game/GameAmmo.hpp"
#include "game/GameWorld.hpp"
#include "startup/EngineStringSystem.hpp"
#include "gui/Label.hpp"
#include "gui/ScrollBar.hpp"
#include <cstdio>
#include <stdexcept>

namespace F3
{
	void InventoryInterface::Open(DWORD actor) //43F4C0, actual RET4.
	{
		actorID = actor;
		SetDescription(std::string());
		backpackScroll->SetValue(0, 1);
		RefreshGridBindings();
		ResetGrid();
		HideSort();
		if (sortLabels[0]) sortLabels[0]->SetStringReference(150);
		RefreshCharacterName();
		RefreshPrimaryAttributes();
		RefreshSkills();
		RefreshEquipmentAvailability();
		PopulateItems();
		RefreshEquipmentText();
		RefreshWeight();
		SortItems(4, 0);
	}

	void InventoryInterface::Refresh() //43F5E0.
	{
		if (!GetHidden())
		{
			PopulateItems();
			RefreshEquipmentText();
			RefreshWeight();
			SortItems(4, 0);
		}
	}

	void InventoryInterface::ItemName(JE::Entity* item, std::string& text) //43A8F0, actual RET8.
	{
		GameEntity* game = dynamic_cast<GameEntity*>(item);
		(void)dynamic_cast<GameArmor*>(item);
		if (game) GetString(game->stringRefs[0], text);
		else GetString(7, text);
	}

	namespace
	{
		GameCreature* ActorCreature(DWORD id)
		{
			JE::Entity* entity = id ? JE::GetEntityPtr(id) : NULL;
			return dynamic_cast<GameCreature*>(entity);
		}
	}

	void InventoryInterface::ArmorValue(JE::Entity* item, DWORD damageType,
		std::string& text, DWORD part) //43C570, thiscall/RET16.
	{
		GameCreature* creature = dynamic_cast<GameCreature*>(JE::GetEntityPtr(actorID));
		(void)dynamic_cast<GameArmor*>(item);
		if (!creature) { text = "-/-"; return; }
		LONG value = 0;
		if (part == 1) value = creature->baseThresholds[damageType];
		else if (part == 0) value = creature->thresholds[damageType];
		char buffer[256];
		std::sprintf(buffer, "%d-%d", value / 2, value);
		text = std::string(buffer);
	}

	void InventoryInterface::WeaponName(JE::Entity* item, std::string& text) //43C6F0.
	{
		GameEntity* game = dynamic_cast<GameEntity*>(item);
		(void)dynamic_cast<GameWeapon*>(item);
		if (game) GetString(game->stringRefs[0], text);
		else if (dynamic_cast<GameCreature*>(JE::GetEntityPtr(actorID))) GetString(0x55, text);
		else text = "---";
	}

	void InventoryInterface::WeaponAmmo(JE::Entity* item, std::string& text) //43C7B0.
	{
		(void)dynamic_cast<GameEntity*>(item);
		GameWeapon* weapon = dynamic_cast<GameWeapon*>(item);
		DWORD capacity;
		if (!weapon || (capacity = weapon->primaryAmmoCapacity) == 0) { text = "-/-"; return; }
		char buffer[256];
		std::sprintf(buffer, "%d/%d", LONG(weapon->primaryAmmoLoaded), LONG(capacity));
		text = buffer;
	}

	void InventoryInterface::WeaponRange(JE::Entity* item, std::string& text) //43C8A0.
	{
		(void)dynamic_cast<GameEntity*>(item);
		GameWeapon* weapon = dynamic_cast<GameWeapon*>(item);
		if (!weapon) { text = "-/-"; return; }
		AttackModeIterator selected = weapon->currentAttackMode;
		if (selected == AttackModeIterator()) { text = "-/-"; return; }
		char buffer[256];
		std::sprintf(buffer, "%d", LONG(selected->range));
		text = buffer;
	}

	void InventoryInterface::DamageTypeName(DWORD type, std::string& text) //43C990, actual RET8.
	{
		switch (type)
		{
		case 0: GetString(0x23F, text); break;
		case 1: GetString(0x240, text); break;
		case 2: GetString(0x241, text); break;
		case 3: GetString(0x242, text); break;
		case 4: GetString(0x243, text); break;
		case 5: GetString(0x244, text); break;
		default: text = ""; break;
		}
	}

	void InventoryInterface::WeaponDamage(JE::Entity* item, std::string& text) //43CA70.
	{
		GameEntity* game = dynamic_cast<GameEntity*>(item);
		GameWeapon* weapon = dynamic_cast<GameWeapon*>(item);
		char buffer[256];
		if (weapon)
		{
			std::string type;
			AttackModeIterator mode = weapon->currentAttackMode;
			if (mode == AttackModeIterator()) text = "-/-";
			else
			{
				DWORD minimum = mode->minimumDamage;
				DWORD maximum = mode->maximumDamage;
				if (mode->shots > 0)
				{
					GameAmmo* ammo = FindAmmunition(weapon->primaryCaliber, weapon->primaryModifier);
					if (ammo)
					{
						maximum += ammo->value956;
						const DWORD candidate = minimum + ammo->value952;
						if (candidate < maximum) minimum = candidate; //Unsigned original compare.
					}
				}
				DamageTypeName(mode->damageType, type);
				std::sprintf(buffer, "%d-%d %s", LONG(minimum), LONG(maximum), type.c_str());
				text = buffer;
			}
		}
		else if (game) text = "-/-";
		else
		{
			GameCreature* creature = dynamic_cast<GameCreature*>(JE::GetEntityPtr(actorID));
			if (!creature) { text = "-/-"; return; }
			DWORD minimum = 0, maximum = 0;
			//The original tests only the nullable selected node.
			AttackModeIterator current = creature->currentAttackMode;
			if (current != AttackModeIterator())
			{
				minimum = current->minimumDamage;
				maximum = current->maximumDamage;
			}
			else text = "-/-"; //Original is immediately overwritten by0-0 below.
			std::sprintf(buffer, "%d-%d", LONG(minimum), LONG(maximum));
			text = buffer;
		}
	}

	void InventoryInterface::RefreshWeight() //43B810.
	{
		GameInventory* inventory = GetInventory();
		GameCreature* creature = ActorCreature(actorID);
		const LONG weight = inventory ? LONG(inventory->value44) : 0;
		const LONG capacity = creature ? creature->GetDerivedAttribute(1) : 0;
		char buffer[32];
		std::sprintf(buffer, "%d / %d", weight, capacity);
		weightLabel->SetText(buffer);
	}

	void InventoryInterface::RefreshCharacterName() //43B920.
	{
		::GUI::Label* label = static_cast< ::GUI::Label*>(FindWindow("lblCharName"));
		if (label)
		{
			GameEntity* entity = actorID ? dynamic_cast<GameEntity*>(JE::GetEntityPtr(actorID)) : NULL;
			label->SetText(entity ? entity->GetDisplayName() : std::string());
		}
	}

	void InventoryInterface::RefreshPrimaryAttributes() //43BA90; no EBP argument.
	{
		GameCreature* creature = ActorCreature(actorID);
		if (!creature) return;
		for (int i = 0; i < 7; ++i)
		{
			char buffer[32];
			std::sprintf(buffer, "%d", creature->baseSpecial[i]); //Original+760, not current special.
			statLabels[i]->SetText(buffer);
		}
	}

	void InventoryInterface::RefreshSkills() //43BE70; no EBP argument.
	{
		GameCreature* creature = ActorCreature(actorID);
		if (!creature) return;
		static const DWORD order[13] = {3, 5, 0, 6, 7, 1, 8, 4, 9, 10, 11, 12, 2};
		for (int i = 0; i < 13; ++i)
		{
			const LONG value = creature->GetTotalSkill(order[i]);
			char buffer[32];
			std::sprintf(buffer, "%d", value);
			skillLabels[i]->SetText(buffer);
		}
	}

	void InventoryInterface::RefreshEquipmentText() //43EAE0.
	{
		std::string text;
		JE::Entity* body = FindEquipped(0);
		ItemName(body, text);
		bodyLabels[0]->SetText(text);
		ArmorValue(body, 0, text, 0);
		bodyLabels[1]->SetText(text);
		ArmorValue(body, 1, text, 0);
		bodyLabels[2]->SetText(text);
		ArmorValue(body, 2, text, 0);
		bodyLabels[3]->SetText(text);
		ArmorValue(body, 3, text, 0);
		bodyLabels[4]->SetText(text);
		ArmorValue(body, 5, text, 0);
		bodyLabels[5]->SetText(text);
		ArmorValue(body, 4, text, 0);
		bodyLabels[6]->SetText(text);
		JE::Entity* head = FindEquipped(1);
		ItemName(head, text);
		headLabels[0]->SetText(text);
		ArmorValue(head, 0, text, 1);
		headLabels[1]->SetText(text);
		ArmorValue(head, 1, text, 1);
		headLabels[2]->SetText(text);
		ArmorValue(head, 2, text, 1);
		headLabels[3]->SetText(text);
		ArmorValue(head, 3, text, 1);
		headLabels[4]->SetText(text);
		ArmorValue(head, 5, text, 1);
		headLabels[5]->SetText(text);
		ArmorValue(head, 4, text, 1);
		headLabels[6]->SetText(text);
		JE::Entity* left = FindEquipped(2);
		WeaponName(left, text);
		handLabels[0][0]->SetText(text);
		WeaponDamage(left, text);
		handLabels[0][1]->SetText(text);
		WeaponAmmo(left, text);
		handLabels[0][2]->SetText(text);
		WeaponRange(left, text);
		handLabels[0][3]->SetText(text);
		JE::Entity* right = FindEquipped(3);
		WeaponName(right, text);
		handLabels[1][0]->SetText(text);
		WeaponDamage(right, text);
		handLabels[1][1]->SetText(text);
		WeaponAmmo(right, text);
		handLabels[1][2]->SetText(text);
		WeaponRange(right, text);
		handLabels[1][3]->SetText(text);
	}
}
