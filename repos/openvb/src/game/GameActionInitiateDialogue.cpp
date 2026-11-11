#include <winsock2.h>
#include "GameActionInitiateDialogue.hpp"
#include "GameActionMoveTo.hpp"
#include "entity/Entity.hpp"
#include "entity/EntityEnvironment.hpp"
#include "entity/World.hpp"
#include "entity/WorldCollections.hpp"
#include "network/EngineServer.hpp"
#include "diagnostics/DebugOutput.hpp"
#include <math.h>

namespace F3
{
	GameActionInitiateDialogue::GameActionInitiateDialogue()
		: completed(0), useTargetDialogue(1), target(NULL), targetID(0)
	{
		kind = 1;
	}
	BYTE GameActionInitiateDialogue::IsComplete() const { return completed; }
	void GameActionInitiateDialogue::Cancel() {}
	int GameActionInitiateDialogue::Finish(DWORD) { return 0; }
	int GameActionInitiateDialogue::Start(DWORD)
	{
		if (!target) JE::FatalError("GameActionInitiateDialogue::OnCreate() - No target specified.");
		targetID = target->entityID;
		return 0;
	}
	int GameActionInitiateDialogue::Update(DWORD, DWORD)
	{
		target = JE::GetEntityPtr(targetID);
		if (!target || target->mapHandle != owner->mapHandle)
		{
			completed = 1;
			return 0;
		}
		float dz = owner->position.z - target->position.z;
		float dy = owner->position.y - target->position.y;
		double dx = double(owner->position.x) - target->position.x;
		if (sqrt(dx*dx + double(dy)*dy + double(dz)*dz) >= 5.0)
		{
			GameActionMoveTo* move = new GameActionMoveTo;
			move->SetTarget(target);
			move->stopDistance = 5;
			owner->QueueAction(move);
			GameActionInitiateDialogue* retry = new GameActionInitiateDialogue;
			retry->target = target;
			retry->useTargetDialogue = useTargetDialogue;
			owner->QueueAction(retry);
			completed = 1;
			return 0;
		}

		std::string name;
		DWORD speakerID, listenerID;
		if (useTargetDialogue)
		{
			name = target->roleName; //Actual Entity+1FC dialogue filename stem.
			speakerID = target->entityID;
			JE::IWorld* world = JE::g_World;
			JE::Party* party = world->GetParty();
			if (!party->Contains(owner))
				JE::FatalError("GameActionInitiateDialogue::OnUpdate() - Initiator must be PC if target's DLG is being used.");
			listenerID = owner->entityID;
		}
		else
		{
			name = owner->roleName;
			speakerID = owner->entityID;
			JE::IWorld* world = JE::g_World;
			JE::Party* party = world->GetParty();
			if (!party->Contains(target))
				JE::FatalError("GameActionInitiateDialogue::OnUpdate() - Target must be PC if initiator's DLG is being used.");
			listenerID = target->entityID;
		}
		JE::DialogueManager* dialogue = static_cast<JE::DialogueManager*>(JE::GetEntityEnvironment().GetServer()->dialogue);
		dialogue->BeginDialogue(name, speakerID, listenerID);
		completed = 1;
		return 0;
	}
}
