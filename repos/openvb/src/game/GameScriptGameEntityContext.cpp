#include "GameScriptGameEntityContext.hpp"
#include "GameEntity.hpp"

namespace F3
{
	BYTE ResolveGameEntity(AMX* vm, DWORD id, GameEntity** output)
	{
		JE::Entity* entity;
		if (!ResolveScriptEntity(vm, id, &entity, 1)) return 0;
		*output = dynamic_cast<GameEntity*>(entity);
		return *output != NULL;
	}
}
