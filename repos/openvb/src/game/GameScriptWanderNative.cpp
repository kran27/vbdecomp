#include "GameScriptNatives.hpp"
#include "entity/ActionWander.hpp"
#include "entity/Entity.hpp"
#include "entity/EntityEnvironment.hpp"
#include "entity/World.hpp"
#include "scripting/ScriptEnvironment.hpp"
#include <stdexcept>

namespace F3
{
	cell AMX_NATIVE_CALL NativeWander(AMX* vm, cell* parameters)
	{
		if (!VerifyScriptParameters(vm,parameters,4,"A_Wander")) return 0;
		JE::Entity* entity = NULL;
		if (!ResolveScriptEntity(vm,parameters[1],&entity,1))
		{
			LogScriptError(vm,"A_Wander() was unable to find the requested entity to move.");
			return 0;
		}
		D3DXVECTOR3 first(0,0,0), last(0,0,0);
		if (!JE::ScriptServices::ReadNativeVector(vm,parameters[2],first))
		{
			LogScriptError(vm,"A_Wander() was unable to retrieve the upper-left bounds.");
			return 0;
		}
		if (!JE::ScriptServices::ReadNativeVector(vm,parameters[3],last))
		{
			LogScriptError(vm,"A_Wander() was unable to retrieve the lower-right bounds.");
			return 0;
		}
		JE::ActionWander* action = static_cast<JE::ActionWander*>(JE::g_World->NewAction(4));
		action->SetBounds(first,last);
		action->repetitions = parameters[4];
		entity->QueueAction(action);
		return 1;
	}
}
