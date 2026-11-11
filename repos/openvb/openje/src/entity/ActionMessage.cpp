#include "ActionMessage.hpp"
#include "Entity.hpp"
#include "World.hpp"
#include "diagnostics/DebugOutput.hpp"
#include <cstdlib>
#include <new>
#include <stdexcept>

namespace JE
{
	ActionMessage::ActionMessage() : completed(0), targetID(0), teamID(0),
		squadID(0), message(DWORD(-1)), argumentCount(0), arguments(NULL)
	{
		kind = 1;
		//Owner and minimumBudget remain unwritten, as in5A8FF0.
	}
	BYTE ActionMessage::IsComplete() const { return completed; }
	void ActionMessage::Cancel() {}
	int ActionMessage::Finish(DWORD) { return 0; }
	int ActionMessage::Update(DWORD, DWORD)
	{
		completed = 1;
		return 0;
	}
	void ActionMessage::SetArguments(LONG count, const cell* values)
	{
		arguments = new cell[count];
		argumentCount = count;
		for (LONG i = 0; i < argumentCount; ++i) arguments[i] = values[i];
	}
	int ActionMessage::Start(DWORD)
	{
		DWORD groupID = teamID;
		if (groupID)
		{
			IWorld* world = g_World;
			TeamSystem* teams = world->GetTeams();
			std::list<DWORD>* members = teams->GetTeamMembers(groupID);
			for (std::list<DWORD>::iterator i = members->begin(); i != members->end(); ++i)
			{
				Entity* entity = GetEntityPtr(*i);
				if (!entity) LogDebugString("ActionMessage was unable to retrieve a team entity.");
				else entity->EnqueueMessage(owner->entityID, message, arguments, argumentCount);
			}
		}
		else if ((groupID = squadID) != 0)
		{
			IWorld* world = g_World;
			SquadSystem* squads = world->GetSquads();
			std::list<DWORD>* members = squads->GetSquadMembers(groupID);
			for (std::list<DWORD>::iterator i = members->begin(); i != members->end(); ++i)
			{
				Entity* entity = GetEntityPtr(*i);
				if (!entity) LogDebugString("ActionMessage was unable to retrieve a squad entity.");
				else entity->EnqueueMessage(owner->entityID, message, arguments, argumentCount);
			}
		}
		else if (targetID)
		{
			Entity* entity = GetEntityPtr(targetID);
			if (!entity)
			{
				LogDebugString("ActionMessage was unable to retrieve an individual entity.");
				delete[] arguments;
				return 0;
			}
			entity->EnqueueMessage(owner->entityID, message, arguments, argumentCount);
		}
		delete[] arguments;
		return 0;
	}
}
