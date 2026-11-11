#include "GameScriptNatives.hpp"
#include "GameWorldCollections.hpp"
#include "entity/World.hpp"
#include "entity/WorldCollections.hpp"
#include <stdexcept>

namespace F3
{
	cell AMX_NATIVE_CALL NativeAddCNPCToParty(AMX* vm, cell* parameters)
	{
		if (!VerifyScriptParameters(vm, parameters, 1, "AddCNPCToParty")) return 0;
		JE::Entity* entity;
		if (!ResolveScriptEntity(vm, DWORD(parameters[1]), &entity, 1))
		{
			LogScriptError(vm, "AddCNPCToParty() was unable to find the entity.");
			return 0;
		}
		GameParty* party = dynamic_cast<GameParty*>(JE::GetWorldInstance()->GetParty());
		if (!party)
		{
			LogScriptError(vm, "AddCNPCToParty() was unable to find the party.");
			return 0;
		}
		return party->Add(entity); //45A373 zero-extends AL, without JoinParty's failure log.
	}

	cell AMX_NATIVE_CALL NativeJoinParty(AMX* vm, cell* parameters)
	{
		if (!VerifyScriptParameters(vm, parameters, 1, "JoinParty")) return 0;
		JE::Entity* entity;
		if (!ResolveScriptEntity(vm, DWORD(parameters[1]), &entity, 1))
		{
			LogScriptError(vm, "JoinParty() was unable to find the requested entity.");
			return 0;
		}
		JE::Party* party = JE::GetWorldInstance()->GetParty();
		if (!party->Add(entity))
		{
			LogScriptError(vm, "JoinParty() was unable to add the entity to the party.");
			return 0;
		}
		return 1;
	}

	cell AMX_NATIVE_CALL NativeLeaveParty(AMX* vm, cell* parameters)
	{
		if (!VerifyScriptParameters(vm, parameters, 1, "LeaveParty")) return 0;
		JE::Entity* entity;
		if (!ResolveScriptEntity(vm, DWORD(parameters[1]), &entity, 1))
		{
			LogScriptError(vm, "LeaveParty() was unable to find the requested entity.");
			return 0;
		}
		JE::Party* party = JE::GetWorldInstance()->GetParty();
		if (party->Contains(entity) && !party->Remove(entity))
		{
			LogScriptError(vm, "LeaveParty() was unable to remove the entity from the party.");
			return 0;
		}
		return 1;
	}
}
