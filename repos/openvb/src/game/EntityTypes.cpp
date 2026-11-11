#include "EntityTypes.hpp"
#include "GameCreature.hpp"
#include "GamePlayer.hpp"
#include "GameUsableObject.hpp"
#include "GameDoor.hpp"
#include "GameContainer.hpp"
#include "GameItem.hpp"
#include "GameWeapon.hpp"
#include "GameArmor.hpp"
#include "GameAmmo.hpp"
#include "GameProjectile.hpp"
#include "entity/EntityResourceTypes.hpp"
#include "diagnostics/DebugOutput.hpp"
#include <string.h>
#include <stdexcept>
#include <new>

namespace F3
{
	JE::Entity* CreateGameEntity(DWORD type)
	{
		switch (type)
		{
		case 0: return new GameEntity;
		case 1: return new GameCreature;
		case 2: return new GameUsableObject;
		case 5: return new GameItem;
		case 6: return new GameWeapon;
		case 7: return new GameArmor;
		case 9: return new GameAmmo;
		case 3: return new GameDoor;
		case 4: return new GameContainer;
		case 8: return new GameProjectile;
		case 10: return new GamePlayer;
		default:
			JE::FatalError("World::CreateEntity() - Unknown entity type %d.\n", type);
			return NULL;
		}
	}

}
