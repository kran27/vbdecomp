#include "GameScriptNatives.hpp"
#include "GameActionReload.hpp"
#include "entity/Entity.hpp"
#include <new>

namespace F3
{
	cell AMX_NATIVE_CALL NativeReloadWeapon(AMX* vm,cell* parameters) //45A380.
	{
		if (!VerifyScriptParameters(vm,parameters,1,"ReloadWeapon")) return 0;
		JE::Entity* owner;
		if (!ResolveScriptEntity(vm,DWORD(parameters[1]),&owner,1))
		{
			LogScriptError(vm,"ReloadWeapon() was unable to find the  entity.");
			return 0;
		}
		GameActionReload* action=new(std::nothrow) GameActionReload;
		owner->QueueAction(action); //Original also passes NULL after allocation failure.
		return 1;
	}
}
