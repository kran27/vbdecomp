#include "Entity.hpp"
#include "PathManager.hpp"

namespace JE
{
	BYTE Entity::HasMapAssociation(Entity* entity) const
	{
		std::list<Entity*>::const_iterator entry = mapAssociations.begin();
		std::list<Entity*>::const_iterator last = mapAssociations.end();
		for (; entry != last; ++entry)
		{
			if (entity == *entry) return 1;
		}
		return 0;
	}

	void Entity::AddMapAssociation(Entity* entity)
	{
		mapAssociations.push_back(entity);
	}

	BYTE Entity::MatchesPathCell(const PathPoint& cell) const
	{
		std::list<EntityMapMessage>::const_iterator entry = mapMessages.begin();
		std::list<EntityMapMessage>::const_iterator last = mapMessages.end();
		for (; entry != last; ++entry)
		{
			const EntityMapMessage message = *entry;
			if (static_cast<DWORD>(cell.x) == message.words[0]
				&& static_cast<DWORD>(cell.y) == message.words[1]) return 1;
		}
		return 0;
	}
}
