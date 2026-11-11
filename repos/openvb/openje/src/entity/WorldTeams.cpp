#include "World.hpp"
#include "diagnostics/DebugOutput.hpp"

namespace JE
{
	WorldTeam::WorldTeam() {}
	WorldTeam::WorldTeam(const WorldTeam& other) : order(other.order), values(other.values) {}
	WorldTeam::~WorldTeam() {}
	WorldSquad::WorldSquad() {}
	WorldSquad::WorldSquad(const WorldSquad& other) : order(other.order), values(other.values) {}
	WorldSquad::~WorldSquad() {}
	TeamSystem::TeamSystem()
	{
		for (DWORD i = 0; i < 2000; ++i) values[i];
		next = 2000;
	}
	TeamSystem::~TeamSystem() {}
	SquadSystem::SquadSystem()
	{
		for (DWORD i = 0; i < 2000; ++i) values[i];
		next = 2000;
	}
	SquadSystem::~SquadSystem() {}
	DWORD TeamSystem::GetAttitude(DWORD source,DWORD target)
	{
		if (!source || !target || source==target) return 0;
		Groups::iterator sourceGroup=values.find(source);
		const Groups::iterator end=values.end();
		if (sourceGroup==end)
		{
			LogDebugString("GetTeamAttitude() - Invalid source team ID %d.\n",source);
			return 0;
		}
		Groups::iterator targetGroup=values.find(target);
		if (targetGroup==end)
		{
			LogDebugString("GetTeamAttitude() - Invalid target team ID %d.\n",target);
			return 0;
		}
		std::map<DWORD,DWORD>::iterator relation=sourceGroup->second.values.find(target);
		return relation==sourceGroup->second.values.end() ? 0 : relation->second;
	}

	DWORD SquadSystem::GetAttitude(DWORD source,DWORD target)
	{
		if (!source || !target) return 0;
		Groups::iterator sourceGroup=values.find(source);
		const Groups::iterator end=values.end();
		if (sourceGroup==end)
		{
			LogDebugString("GetSquadAttitude() - Invalid source squad ID %d.\n",source);
			return 0;
		}
		Groups::iterator targetGroup=values.find(target);
		if (targetGroup==end)
		{
			LogDebugString("GetSquadAttitude() - Invalid target squad ID %n",target);
			return 0;
		}
		std::map<DWORD,DWORD>::iterator relation=sourceGroup->second.values.find(target);
		return relation==sourceGroup->second.values.end() ? 0 : relation->second;
	}
	BYTE TeamSystem::RemoveEntityTeam(DWORD id) //56A1A0.
	{
		Entity* entity=GetEntityPtr(id);
		if (!entity)
		{
			LogDebugString("RemoveFromTeam() - Invalid entity ID %d.\n",id);
			return 0;
		}
		DWORD team=entity->value368;
		if (!team) return 0;
		Groups::iterator found=values.find(team);
		if (found==values.end())
		{
			LogDebugString("RemoveFromTeam() - Invalid team ID %d.\n",team);
			return 0;
		}
		found->second.order.remove(id); //40E780 removes all matching members.
		entity->value368=0;
		return 1;
	}
}
