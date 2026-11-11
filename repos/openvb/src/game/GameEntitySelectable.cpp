#include "GameEntity.hpp"
#include "GameInventory.hpp"

namespace F3
{
	void GameEntity::SetSelectable(BYTE value) //467BE0.
	{
		JE::Entity::SetSelectable(value);
		if (!inventory) return;
		GameInventory* game = dynamic_cast<GameInventory*>(inventory);
		if (game)
		{
			//The original owns an otherwise unused empty temporary list here.
			std::list<JE::Entity*> temporary;
			for (std::list<JE::Entity*>::iterator i = game->items.begin(); i != game->items.end(); ++i)
				(*i)->SetSelectable(value);
		}
	}
}
