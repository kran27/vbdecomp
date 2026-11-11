#include "GameEntity.hpp"
#include "GameInventory.hpp"
#include "entity/World.hpp"
#include "entity/WorldCollections.hpp"

namespace F3
{
	LONG GameEntity::QueueOperate()
	{
		LONG result = 0;
		switch (value2C)
		{
		case 4: result = QueueAnimationPercent(359, 0, 1, 0, 0, 0); break;
		case 5: result = QueueAnimationPercent(431, 0, 1, 0, 0, 0); break;
		case 6:
			if (static_cast<LONG>(weaponStance) > 0 && static_cast<LONG>(weaponStance) <= 6)
				result = QueueAnimationPercent(442, 0, 1, 0, 0, 0);
			else result = QueueAnimationPercent(449, 0, 1, 0, 0, 0);
			break;
		case 10: break;
		default:
			result = QueueToDefault();
			value2C = 5;
			result += QueueOperate();
			break;
		}
		return result;
	}

	BYTE GameInventory::ContainsResource(const std::string& name)
	{
		JE::IWorld* world=JE::g_World;
		JE::Party* party=world->GetParty();
		if (party->Contains(JE::GetEntityPtr(ownerEntityID))==1)
		{
			JE::IWorld* registryWorld=JE::g_World;
			if (registryWorld->GetItemRegistry()->Contains(name)==1) return 1;
		}
		return JE::Inventory::ContainsResource(name);
	}
}
