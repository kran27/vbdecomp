#include "entity/Entity.hpp"
#include "entity/World.hpp"
#include "GameCreature.hpp"
#include "GameDoor.hpp"
#include "GameContainer.hpp"
#include "GameUsableObject.hpp"
#include "GameItem.hpp"
#include <stdexcept>

namespace F3
{
	DWORD GameCreature::GetContextAction(JE::Entity* actor)
	{
		DWORD action=2;
		if (!visibilityOwnerFlag && actor)
		{
			JE::IWorld* world = JE::g_World;
			DWORD targetTeam = actor->value368;
			DWORD sourceTeam = value368;
			JE::TeamSystem* teams = world->GetTeams();
			DWORD team = teams->GetAttitude(sourceTeam, targetTeam);
			world = JE::g_World;
			DWORD targetSquad = actor->squadID;
			DWORD sourceSquad = squadID;
			JE::SquadSystem* squads = world->GetSquads();
			DWORD squad = squads->GetAttitude(sourceSquad, targetSquad);
			if (team==3 || squad==3) action=5;
			else if (CanTalk()) action=4;
		}
		return IsDead()==1 ? 9 : action;
	}
	DWORD GameDoor::GetContextAction(JE::Entity*) { return 9; } //408980.
	DWORD GameContainer::GetContextAction(JE::Entity*) { return 9; }
	DWORD GameUsableObject::GetContextAction(JE::Entity*) { return 3; } //412530.
	DWORD GameItem::GetContextAction(JE::Entity*) { return 13; } //4100B0; Weapon inherits.

	void GameUsableObject::GetContextActions(JE::Entity*, std::list<JE::ContextMenuEntry>& entries)
	{
		JE::ContextMenuEntry entry(1);
		entries.push_back(entry);
		entry.selector = 3;
		entries.push_back(entry);
	}

	void GameDoor::GetContextActions(JE::Entity*, std::list<JE::ContextMenuEntry>& entries)
	{
		JE::ContextMenuEntry entry(1);
		entries.push_back(entry);
		entry.selector = 5;
		entries.push_back(entry);
		entry.selector = 6;
		entries.push_back(entry);
		entry.selector = 3;
		entries.push_back(entry);
	}

	void GameContainer::GetContextActions(JE::Entity*, std::list<JE::ContextMenuEntry>& entries)
	{
		JE::ContextMenuEntry entry(1);
		entries.push_back(entry);
		entry.selector = 5;
		entries.push_back(entry);
		entry.selector = 3;
		entries.push_back(entry);
	}

	void GameItem::GetContextActions(JE::Entity*, std::list<JE::ContextMenuEntry>& entries)
	{
		JE::ContextMenuEntry entry(1);
		entries.push_back(entry);
		entry.selector = 2;
		entries.push_back(entry);
		entry.selector = 3;
		entries.push_back(entry);
	}

	void GameCreature::GetContextActions(JE::Entity*, std::list<JE::ContextMenuEntry>& entries)
	{
		JE::ContextMenuEntry entry(0);
		if (!roleName.empty()) entries.push_back(entry); //Original+210, no dialogueEnabled gate.
		if (IsDead() == 1)
		{
			entry.selector = 5;
			entries.push_back(entry);
		}
		entry.selector = 1;
		entries.push_back(entry);
		entry.selector = 2;
		entries.push_back(entry);
		entry.selector = 3;
		entries.push_back(entry);
	}
}
