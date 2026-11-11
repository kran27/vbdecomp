#include "GameScriptNatives.hpp"
#include "GameCreature.hpp"

namespace F3
{
	namespace
	{
		template<class Owner> Owner* ResolveOwner(AMX* vm, DWORD id, const char* missing, const char* wrong)
		{
			JE::Entity* entity = NULL;
			if (!ResolveScriptEntity(vm, id, &entity, 1))
			{
				LogScriptError(vm, missing);
				return NULL;
			}
			Owner* owner = dynamic_cast<Owner*>(entity);
			if (!owner) LogScriptError(vm, wrong);
			return owner;
		}
	}
	cell AMX_NATIVE_CALL NativeGetPrimaryAttribute(AMX* vm, cell* parameters)
	{
		if (!VerifyScriptParameters(vm, parameters, 2, "GetPrimaryAttribute")) return 0;
		JE::Entity* entity;
		if (!ResolveScriptEntity(vm, DWORD(parameters[1]), &entity, 1))
		{
			LogScriptError(vm, "GetPrimaryAttribute() - Unable to resolve requested entity.");
			return 0;
		}
		GameCreature* creature = dynamic_cast<GameCreature*>(entity);
		if (!creature)
		{
			LogScriptError(vm, "GetPrimaryAttribute() - Entity ID does not belong to a game creature.");
			return 0;
		}
		return cell(creature->baseSpecial[parameters[2]]); //Original+760; no native index clamp.
	}
	cell AMX_NATIVE_CALL NativeGetDerivedAttribute(AMX* vm, cell* parameters)
	{
		if (!VerifyScriptParameters(vm, parameters, 2, "GetDerivedAttribute")) return 0;
		JE::Entity* source;
		if (!ResolveScriptEntity(vm, DWORD(parameters[1]), &source, 1))
		{
			LogScriptError(vm, "GetDerivedAttribute() - Unable to resolve requested entity.");
			return 0;
		}
		GameCreature* creature = dynamic_cast<GameCreature*>(source);
		if (!creature)
		{
			LogScriptError(vm, "GetDerivedAttribute() - EntityID does not belong to a game creature.");
			return 0;
		}
		return cell(creature->GetDerivedAttribute(parameters[2]));
	}
	cell AMX_NATIVE_CALL NativeGetBaseSkill(AMX* vm, cell* parameters)
	{
		if (!VerifyScriptParameters(vm, parameters, 2, "GetBaseSkill")) return 0;
		JE::Entity* source;
		if (!ResolveScriptEntity(vm, DWORD(parameters[1]), &source, 1))
		{
			LogScriptError(vm, "GetBaseSkill() - Unable to resolve requested entity.");
			return 0;
		}
		GameCreature* creature = dynamic_cast<GameCreature*>(source);
		if (!creature)
		{
			LogScriptError(vm, "GetBaseSkill() - EntityID does not belong to a game creature.");
			return 0;
		}
		return cell(creature->GetBaseSkill(parameters[2]));
	}
	cell AMX_NATIVE_CALL NativeGetTotalSkill(AMX* vm, cell* parameters)
	{
		if (!VerifyScriptParameters(vm, parameters, 2, "GetTotalSkill")) return 0;
		JE::Entity* source;
		if (!ResolveScriptEntity(vm, DWORD(parameters[1]), &source, 1))
		{
			LogScriptError(vm, "GetTotalSkill() - Unable to resolve requested entity.");
			return 0;
		}
		GameCreature* creature = dynamic_cast<GameCreature*>(source);
		if (!creature)
		{
			LogScriptError(vm, "GetTotalSkill() - EntityID does not belong to a game creature.");
			return 0;
		}
		return cell(creature->GetTotalSkill(parameters[2]));
	}
	cell AMX_NATIVE_CALL NativeGetRace(AMX* vm, cell* parameters)
	{
		if (!VerifyScriptParameters(vm, parameters, 1, "GetRace")) return 0;
		GameCreature* creature = ResolveOwner<GameCreature>(vm, DWORD(parameters[1]),
			"GetRace() - Unable to resolve requested entity.",
			"GetRace() - EntityID does not belong to a game creature.");
		return creature ? cell(creature->baseValues[3]) : 0; //Original+C8.
	}
	cell AMX_NATIVE_CALL NativeGetGender(AMX* vm, cell* parameters)
	{
		if (!VerifyScriptParameters(vm, parameters, 1, "GetGender")) return 0;
		GameCreature* creature = ResolveOwner<GameCreature>(vm, DWORD(parameters[1]),
			"GetGender() - Unable to resolve requested entity.",
			"GetGender() - EntityID does not belong to a game creature.");
		return creature ? cell(creature->baseValues[5]) : 0; //Original+D0.
	}
	cell AMX_NATIVE_CALL NativeGetEthnicity(AMX* vm, cell* parameters)
	{
		if (!VerifyScriptParameters(vm, parameters, 1, "GetEthnicity")) return 0;
		GameCreature* creature = ResolveOwner<GameCreature>(vm, DWORD(parameters[1]),
			"GetEthnicity() - Unable to resolve requested entity.",
			"GetEthnicity() - EntityID does not belong to a game creature.");
		return creature ? cell(creature->baseValues[6]) : 0; //Original+D4.
	}
	cell AMX_NATIVE_CALL NativeGetSubRace(AMX* vm, cell* parameters)
	{
		if (!VerifyScriptParameters(vm, parameters, 1, "GetSubRace")) return 0;
		GameCreature* creature = ResolveOwner<GameCreature>(vm, DWORD(parameters[1]),
			"GetSubRace() - Unable to resolve requested entity.",
			"GetSubRace() - Entity ID does not belong to a game creature.");
		return creature ? cell(creature->valueD8) : 0; //Distinct from baseValues[7], which represents original+DC.
	}
	cell AMX_NATIVE_CALL NativeGetLevel(AMX* vm, cell* parameters)
	{
		if (!VerifyScriptParameters(vm, parameters, 1, "GetLevel")) return 0;
		GameCreature* creature = ResolveOwner<GameCreature>(vm, DWORD(parameters[1]),
			"GetLevel() - Unable to resolve the requested entity.",
			"GetLevel() - Entity ID does not belong to a game creature.");
		return creature ? cell(creature->baseValues[0]) : 0; //Original+BC.
	}
	cell AMX_NATIVE_CALL NativeGetMaxHP(AMX* vm, cell* parameters)
	{
		if (!VerifyScriptParameters(vm, parameters, 1, "GetMaxHP")) return 0;
		JE::Entity* source = NULL;
		if (!ResolveScriptEntity(vm, DWORD(parameters[1]), &source, 1))
		{
			LogScriptError(vm, "GetMaxHP() - Unable to resolve requested entity.");
			return 0;
		}
		GameEntity* entity = dynamic_cast<GameEntity*>(source);
		if (!entity)
		{
			LogScriptError(vm, "GetMaxHP() - Entity ID does not belong to a GameEntity.");
			return 0;
		}
		return cell(entity->GetMaximumHealth());
	}
	cell AMX_NATIVE_CALL NativeGetCurrentHP(AMX* vm, cell* parameters)
	{
		if (!VerifyScriptParameters(vm, parameters, 1, "GetCurrentHP")) return 0;
		JE::Entity* source = NULL;
		if (!ResolveScriptEntity(vm, DWORD(parameters[1]), &source, 1))
		{
			LogScriptError(vm, "GetCurrentHP() - Unable to resolve requested entity.");
			return 0;
		}
		GameEntity* entity = dynamic_cast<GameEntity*>(source);
		if (!entity)
		{
			LogScriptError(vm, "GetCurrentHP() - Entity ID does not belong to a GameEntity.");
			return 0;
		}
		return cell(entity->currentHealth);
	}
	cell AMX_NATIVE_CALL NativeGetPerkRank(AMX* vm, cell* parameters)
	{
		if (!VerifyScriptParameters(vm, parameters, 2, "GetPerkRank")) return 0;
		GameCreature* creature = ResolveOwner<GameCreature>(vm, DWORD(parameters[1]),
			"GetPosition() was unable to find the requested entity.", //Actual original typo.
			"GetPerkRank() - Entity ID does not belong to a game creature.");
		return creature ? cell(creature->traits[parameters[2]]) : 0; //Original+114, unchecked index.
	}
}
