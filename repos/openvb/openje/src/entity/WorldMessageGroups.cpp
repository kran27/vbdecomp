#include "World.hpp"
#include "diagnostics/DebugOutput.hpp"

namespace JE
{
	std::list<DWORD>* TeamSystem::GetTeamMembers(DWORD id) //56A2F0; borrowed.
	{
		Groups::iterator found=values.find(id);
		if (found==values.end())
		{
			LogDebugString("GetTeamMemberList() - Invalid team ID %d.\n",id);
			return NULL;
		}
		return &found->second.order;
	}
	std::list<DWORD>* SquadSystem::GetSquadMembers(DWORD id) //569390; borrowed.
	{
		Groups::iterator found=values.find(id);
		if (found==values.end())
		{
			LogDebugString("GetSquadMemberList() - Invalid squad ID %d.\n",id);
			return NULL;
		}
		return &found->second.order;
	}
}
