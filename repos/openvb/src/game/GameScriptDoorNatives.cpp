#include "GameScriptNatives.hpp"
#include "GameDoor.hpp"

namespace F3
{
	cell AMX_NATIVE_CALL NativeLockDoor(AMX* vm,cell* parameters) //45A1B0.
	{
		if (!VerifyScriptParameters(vm,parameters,1,"LockDoor")) return 0;
		JE::Entity* entity;
		if (!ResolveScriptEntity(vm,DWORD(parameters[1]),&entity,1))
		{
			LogScriptError(vm,"LockDoor() was unable to find the entity.");
			return 0;
		}
		GameDoor* door=dynamic_cast<GameDoor*>(entity);
		if (!door)
		{
			LogScriptError(vm,"LockDoor() - Entity is not a door");
			return 0;
		}
		door->locked=1;
		return 1;
	}

	cell AMX_NATIVE_CALL NativeUnlockDoor(AMX* vm,cell* parameters) //45A240.
	{
		if (!VerifyScriptParameters(vm,parameters,1,"UnlockDoor")) return 0;
		JE::Entity* entity;
		if (!ResolveScriptEntity(vm,DWORD(parameters[1]),&entity,1))
		{
			LogScriptError(vm,"UnlockDoor() was unable to find the entity.");
			return 0;
		}
		GameDoor* door=dynamic_cast<GameDoor*>(entity);
		if (!door)
		{
			LogScriptError(vm,"UnlockDoor() - Entity is not a door");
			return 0;
		}
		door->locked=0;
		return 1;
	}
}
