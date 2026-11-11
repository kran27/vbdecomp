#include "World.hpp"

namespace JE
{
	void __cdecl LogDebugString(const char* format, ...);
	//56AD57/569DF7 retain cdecl stack cleanup after48CC40.
	void __cdecl FatalError(const char* format, ...);
	void World::SetEntityIDCounter(DWORD value) { value80=value&0x1FFFFFFF; } //46FE40.
	void World::SetMapIDCounter(DWORD value) { value84=value&0x1FFFFFFF; } //46FEE0.
	BYTE TeamSystem::AddEntityTeam(DWORD id,DWORD team) //56A330.
	{
		Entity* entity=GetEntityPtr(id);
		if (!entity) { LogDebugString("AddToTeam() - Invalid entity ID %d.\n",id); return 0; }
		DWORD oldTeam=entity->value368;
		if (oldTeam==team) return 0;
		Groups::iterator found=values.find(team);
		if (found==values.end())
		{
			LogDebugString("AddToTeam() - Invalid team ID %d.\n",team);
			return 0;
		}
		std::list<DWORD>& members=found->second.order;
		if (oldTeam) RemoveEntityTeam(id);
		members.push_back(id);
		entity->value368=team;
		return 1;
	}
	BYTE SquadSystem::AddEntitySquad(DWORD id,DWORD squad) //5693D0.
	{
		Entity* entity=GetEntityPtr(id);
		if (!entity) { LogDebugString("AddToTeam() - Invalid entity ID %d.\n",id); return 0; }
		DWORD oldSquad=entity->squadID;
		if (oldSquad==squad) return 0;
		Groups::iterator found=values.find(squad);
		if (found==values.end())
		{
			LogDebugString("AddToSquad() - Invalid squad ID %d.\n",squad);
			return 0;
		}
		std::list<DWORD>& members=found->second.order;
		if (oldSquad) RemoveEntitySquad(id);
		members.push_back(id);
		entity->squadID=squad;
		return 1;
	}
	BYTE SquadSystem::RemoveEntitySquad(DWORD id) //569240.
	{
		Entity* entity=GetEntityPtr(id);
		if (!entity) { LogDebugString("RemoveFromSquad() - Invalid entity ID %d.\n",id); return 0; }
		DWORD group=entity->squadID;
		if (!group) return 0;
		Groups::iterator found=values.find(group);
		if (found==values.end()) { LogDebugString("RemoveFromSquad() - Invalid squad ID %d.\n",group); return 0; }
		found->second.order.remove(id);
		entity->squadID=0;
		return 1;
	}
	BYTE TeamSystem::SetTeamAttitude(DWORD source,DWORD target,DWORD attitude) //56A3E0.
	{
		Groups::iterator found=values.find(source);
		const Groups::iterator end=values.end();
		if (found==end)
		{
			LogDebugString("SetTeamAttitude() - Invalid source team ID %d.\n",source);
			return 0;
		}
		Groups::iterator targetGroup=values.find(target);
		if (targetGroup==end)
		{
			LogDebugString("SetTeamAttitude() - Invalid target team ID %d.\n",target);
			return 0;
		}
		std::map<DWORD,DWORD>& relations=found->second.values;
		std::map<DWORD,DWORD>::iterator relation=relations.find(target);
		if (relation==found->second.values.end())
		{
			if (attitude) relations[target]=attitude;
		}
		else
		{
			if (attitude) relations[target]=attitude;
			else relations.erase(relation);
		}
		return 1;
	}
	BYTE SquadSystem::SetSquadAttitude(DWORD source,DWORD target,DWORD attitude) //569480.
	{
		Groups::iterator found=values.find(source);
		const Groups::iterator end=values.end();
		if (found==end)
		{
			LogDebugString("SetSquadAttitude() - Invalid source squad ID %d.\n",source);
			return 0;
		}
		Groups::iterator targetGroup=values.find(target);
		if (targetGroup==end)
		{
			LogDebugString("SetSquadAttitude() - Invalid target squad ID %d.\n",target);
			return 0;
		}
		std::map<DWORD,DWORD>& relations=found->second.values;
		std::map<DWORD,DWORD>::iterator relation=relations.find(target);
		if (relation==found->second.values.end())
		{
			if (attitude) relations[target]=attitude;
		}
		else
		{
			if (attitude) relations[target]=attitude;
			else relations.erase(relation);
		}
		return 1;
	}
	DWORD TeamSystem::CreateGroup() //56AD40.
	{
		if (next==DWORD(-1)) FatalError("CreateTeam() - %d teams already exist. Cannot create more without overwriting old teams.",DWORD(-1));
		DWORD id=next++;
		values[id];
		return id;
	}
	DWORD SquadSystem::CreateGroup() //569DE0; original retains the team wording.
	{
		if (next==DWORD(-1)) FatalError("CreateTeam() - %d teams already exist. Cannot create more without overwriting old teams.",DWORD(-1));
		DWORD id=next++;
		values[id];
		return id;
	}
}
