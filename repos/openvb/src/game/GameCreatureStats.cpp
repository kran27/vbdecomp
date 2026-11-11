#include <winsock2.h>
#include "GameCreature.hpp"
#include "GameWeapon.hpp"
#include "startup/GameServer.hpp"
#include "startup/NetworkComponent.hpp"
#include "entity/Inventory.hpp"
#include "diagnostics/DebugOutput.hpp"
#include <stdexcept>

namespace F3
{
	LONG GameEntity::GetMaximumHealth() { return maximumHealth; }
	void GameCreature::SetPerkRank(LONG perk, LONG rank) //40A370.
	{
		if (perk > 82) return;
		traits[perk] = rank;
		RebuildInventoryAttackModes();
	}
	LONG GameCreature::IncrementPerk(LONG perk) //40A2F0.
	{
		if (perk > 82) return -1; //Original signed upper-bound-only predicate.
		traits[perk] = LONG(DWORD(traits[perk]) + 1);
		JE::Server* base = static_cast<JE::Server*>(g_pNetworkInstance->server);
		GameServer* server = dynamic_cast<GameServer*>(base);
		server->NotifyPerkRank(entityID, perk, DWORD(traits[perk]));
		RebuildInventoryAttackModes();
		return traits[perk]; //Reload after notification and weapon callbacks, as in the original.
	}
	void GameCreature::RebuildInventoryAttackModes() //40A120.
	{
		if (!inventory) return;
		JE::Inventory* owner = inventory;
		for (std::list<JE::Entity*>::iterator i = owner->items.begin(); i != owner->items.end(); ++i)
			if (GameWeapon* weapon = dynamic_cast<GameWeapon*>(*i)) weapon->RebuildAttackModes(this);
	}
	BYTE GameCreature::HasTrait(DWORD trait) const //40A0E0.
	{
		for (std::list<DWORD>::const_iterator entry=selectedTraits.begin(); entry!=selectedTraits.end(); ++entry)
			if (*entry==trait) return 1;
		return 0;
	}

	BYTE GameCreature::HasTagSkill(DWORD skill) const //409CD0.
	{
		for (std::list<DWORD>::const_iterator entry=tagSkills.begin(); entry!=tagSkills.end(); ++entry)
			if (*entry==skill) return 1;
		return 0;
	}

	LONG GameCreature::TagSkillCount() const { return LONG(tagSkills.size()); }
	LONG GameCreature::TraitCount() const { return LONG(selectedTraits.size()); }
	void GameCreature::AddTagSkill(DWORD skill)
	{
		tagSkills.push_back(skill);
		skills[skill] = LONG(DWORD(skills[skill]) + 20);
	}
	void GameCreature::RemoveTagSkill(DWORD skill)
	{
		for (std::list<DWORD>::iterator entry=tagSkills.begin(); entry!=tagSkills.end();)
			if (*entry==skill)
			{
				entry=tagSkills.erase(entry);
				skills[skill] = LONG(DWORD(skills[skill]) - 20);
			}
			else ++entry;
	}
	void GameCreature::AddTrait(DWORD trait) { selectedTraits.push_back(trait); }
	void GameCreature::RemoveTrait(DWORD trait) { selectedTraits.remove(trait); }

	LONG GameCreature::GetBaseSkill(LONG skill) { return skills[skill]; }
	LONG GameCreature::GetTotalSkill(LONG skill)
	{
		DWORD base = DWORD(GetBaseSkill(skill)); //Original reads base before the switch.
		DWORD bonus;
		switch (skill)
		{
		case 0: case 1: bonus = 50; break;
		case 2: bonus = DWORD(baseSpecial[2]) + 2 * DWORD(baseSpecial[0]); break;
		case 3: case 4: bonus = 3 * DWORD(baseSpecial[3]); break;
		case 5: bonus = DWORD(baseSpecial[4]) + 2 * DWORD(baseSpecial[3]); break;
		case 6: case 7: bonus = DWORD(baseSpecial[1]) + 2 * DWORD(baseSpecial[4]); break;
		case 8: bonus = DWORD(baseSpecial[2]) + 2 * DWORD(baseSpecial[1]); break;
		case 9: bonus = 3 * DWORD(baseSpecial[4]); break;
		case 10: case 11: bonus = DWORD(baseSpecial[5]) + 2 * DWORD(baseSpecial[1]); break;
		case 12: bonus = DWORD(baseSpecial[1]) + 2 * DWORD(baseSpecial[5]); break;
		default:
			JE::LogDebugString("GameCreature::GetTotalSkill() - UNKNOWN SKILL: %d", skill);
			return LONG(base);
		}
		return LONG(base + bonus); //Actual DWORD arithmetic wraps.
	}
}
