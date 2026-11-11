#include "GameScriptNatives.hpp"
#include "GameActionAttack.hpp"
#include "entity/Entity.hpp"
#include <new>

namespace F3
{
	cell AMX_NATIVE_CALL NativeJustAttack(AMX* vm,cell* parameters) //459F70.
	{
		if (!VerifyScriptParameters(vm,parameters,2,"Attack")) return 0;
		JE::Entity* attacker;
		if (!ResolveScriptEntity(vm,DWORD(parameters[1]),&attacker,1))
		{
			LogScriptError(vm,"JustAttack() was unable to find the attacking entity.");
			return 0;
		}
		JE::Entity* target;
		if (!ResolveScriptEntity(vm,DWORD(parameters[2]),&target,1))
		{
			LogScriptError(vm,"JustAttack() was unable to find the target entity.");
			return 0;
		}
		GameActionAttack* action=new GameActionAttack;
		action->rotateToTarget=0;
		action->autoMove=0;
		action->SetTarget(target);
		attacker->QueueAction(action);
		return 1;
	}

	cell AMX_NATIVE_CALL NativeAttack(AMX* vm, cell* parameters) //459EB0.
	{
		if (!VerifyScriptParameters(vm,parameters,2,"Attack")) return 0;
		JE::Entity* attacker;
		if (!ResolveScriptEntity(vm,static_cast<DWORD>(parameters[1]),&attacker,1))
		{
			LogScriptError(vm,"Attack() was unable to find the attacking entity.");
			return 0;
		}
		JE::Entity* target;
		if (!ResolveScriptEntity(vm,static_cast<DWORD>(parameters[2]),&target,1))
		{
			LogScriptError(vm,"Attack() was unable to find the target entity.");
			return 0;
		}
		GameActionAttack* action=new GameActionAttack;
		action->SetTarget(target);
		attacker->QueueAction(action);
		return 1;
	}
}
