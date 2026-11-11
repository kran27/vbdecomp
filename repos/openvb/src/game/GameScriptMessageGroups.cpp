#include "GameScriptNatives.hpp"
#include "entity/ActionMessage.hpp"
#include "entity/Entity.hpp"
#include "entity/World.hpp"
#include <cstdlib>
#include <new>
#include <stdexcept>

namespace F3
{
	namespace
	{
		enum Destination { Individual, Team, Squad };
		DWORD SourceID(AMX* vm, const char* name, BYTE* valid)
		{
			void* value;
			*valid = amx_GetUserData(vm, 0x464C534D, &value) == 0;
			if (!*valid)
			{
				LogScriptError(vm, "%s() was unable to retrieve the 'from' id.", name);
				return 0;
			}
			return DWORD(reinterpret_cast<UINT_PTR>(value));
		}
		cell* ReadArguments(AMX* vm, cell* parameters, LONG count,
			const char* failure, BYTE* valid)
		{
			cell* values = static_cast<cell*>(::operator new(sizeof(cell) * count));
			for (LONG i = 0; i < count; ++i)
			{
				cell* source;
				if (amx_GetAddr(vm, parameters[i + 3], &source))
				{
					LogScriptError(vm, failure, i + 2);
					*valid = 0;
					return NULL; //Original temporary allocation leaks on address failure.
				}
				values[i] = *source;
			}
			*valid = 1;
			return values;
		}
		JE::World& MessageWorld()
		{
			if (!JE::g_World) throw std::logic_error("Message native: actual World required");
			return *JE::g_World;
		}
		cell QueueMessage(AMX* vm, cell* parameters, Destination destination, const char* name)
		{
			LONG provided;
			if (!VerifyMinimumScriptParameters(vm, parameters, 2, name, &provided)) return 0;
			BYTE valid;
			DWORD sourceID = SourceID(vm, name, &valid);
			if (!valid) return 0;
			JE::Entity* source = JE::GetEntityPtr(sourceID);
			if (!source)
			{
				LogScriptError(vm, destination == Squad
					? "A_SendMessageToSquad() was unable to retrieve the 'from' entity.(2)"
					: "%s() was unable to retrieve the 'from' entity.", name);
				return 0;
			}
			if (destination == Individual)
			{
				JE::Entity* target;
				if (!ResolveScriptEntity(vm, DWORD(parameters[1]), &target, 1))
				{
					LogScriptError(vm, "A_SendMessage() was unable to find the requested entity.");
					return 0;
				}
			}
			const char* failure = destination == Individual
				? "A_SendMessage() was unable to resolve address of parameter %d."
				: destination == Team
				? "A_SendMessageToTeam() was unable to resolve address of parameter %d."
				: "A_SendMessageToSquad() was unable to resolve address of parameter %d";
			cell* values = ReadArguments(vm, parameters, provided - 2, failure, &valid);
			if (!valid) return 0;
			JE::ActionMessage* action = static_cast<JE::ActionMessage*>(MessageWorld().NewAction(7));
			action->message = DWORD(parameters[2]);
			if (destination == Individual) action->targetID = DWORD(parameters[1]);
			else if (destination == Team) action->teamID = DWORD(parameters[1]);
			else action->squadID = DWORD(parameters[1]);
			action->SetArguments(provided - 2, values);
			source->QueueAction(action);
			free(values);
			return 1;
		}
		cell SendGroupMessage(AMX* vm, cell* parameters, BYTE squad, const char* name)
		{
			LONG provided;
			if (!VerifyMinimumScriptParameters(vm, parameters, 2, name, &provided)) return 0;
			BYTE valid;
			DWORD sourceID = SourceID(vm, name, &valid);
			if (!valid) return 0;
			cell* values = ReadArguments(vm, parameters, provided - 2, squad
				? "SendMessageToSquad() was unable to resolve the address of parameter %d."
				: "SendMessageToTeam() was unable to resolve address of parameter %d.", &valid);
			if (!valid) return 0;
			JE::IWorld* world = &MessageWorld();
			DWORD groupID = DWORD(parameters[1]);
			std::list<DWORD>* members = squad
				? world->GetSquads()->GetSquadMembers(groupID)
				: world->GetTeams()->GetTeamMembers(groupID);
			if (!members)
				throw std::logic_error("SendMessageToGroup5C58BD/5C5A31: original unchecked NULL member list");
			cell delivered = 0;
			for (std::list<DWORD>::iterator i = members->begin(); i != members->end(); ++i)
			{
				JE::Entity* entity = JE::GetEntityPtr(*i);
				if (entity)
				{
					entity->EnqueueMessage(sourceID, DWORD(parameters[2]), values, provided - 2);
					++delivered;
				}
				else LogScriptError(vm, squad
					? "SendMessageToSquad() was unable to retrieve an entity. Internal error."
					: "SendMessageToTeam() was unable to retrieve an entity.");
			}
			free(values);
			return squad ? delivered : 1;
		}
	}
	cell AMX_NATIVE_CALL NativeActionSendMessage(AMX* vm, cell* parameters)
	{
		return QueueMessage(vm, parameters, Individual, "A_SendMessage");
	}
	cell AMX_NATIVE_CALL NativeActionSendMessageToTeam(AMX* vm, cell* parameters)
	{
		return QueueMessage(vm, parameters, Team, "A_SendMessageToTeam");
	}
	cell AMX_NATIVE_CALL NativeActionSendMessageToSquad(AMX* vm, cell* parameters)
	{
		return QueueMessage(vm, parameters, Squad, "A_SendMessageToSquad");
	}
	cell AMX_NATIVE_CALL NativeSendMessageToTeam(AMX* vm, cell* parameters)
	{
		LONG provided;
		if (!VerifyMinimumScriptParameters(vm, parameters, 2, "SendMessageToTeam", &provided)) return 0;
		void* owner;
		if (amx_GetUserData(vm, 0x464C534D, &owner))
		{
			LogScriptError(vm, "SendMessageToTeam() was unable to retrieve the 'from' id.");
			return 0;
		}
		cell* values = new cell[provided - 2];
		for (LONG i = 0; i < provided - 2; ++i)
		{
			cell* source;
			if (amx_GetAddr(vm, parameters[i + 3], &source))
			{
				LogScriptError(vm, "SendMessageToTeam() was unable to resolve address of parameter %d.", i + 2);
				return 0; //The original leaks this allocation on argument failure.
			}
			values[i] = *source;
		}
		JE::IWorld* world = JE::g_World;
		std::list<DWORD>* members = world->GetTeams()->GetTeamMembers(DWORD(parameters[1]));
		for (std::list<DWORD>::iterator i = members->begin(); i != members->end(); ++i)
		{
			JE::Entity* entity = JE::GetEntityPtr(*i);
			if (!entity)
				LogScriptError(vm, "SendMessageToTeam() was unable to retrieve an entity.");
			else
				entity->EnqueueMessage(DWORD(reinterpret_cast<UINT_PTR>(owner)),
					DWORD(parameters[2]), values, provided - 2);
		}
		delete[] values;
		return 1;
	}
	cell AMX_NATIVE_CALL NativeSendMessageToSquad(AMX* vm, cell* parameters)
	{
		cell delivered = 0;
		LONG provided;
		if (!VerifyMinimumScriptParameters(vm, parameters, 2, "SendMessageToSquad", &provided)) return 0;
		void* owner;
		if (amx_GetUserData(vm, 0x464C534D, &owner))
		{
			LogScriptError(vm, "SendMessageToSquad() was unable to retrieve the 'from' id.");
			return 0;
		}
		cell* values = new cell[provided - 2];
		for (LONG i = 0; i < provided - 2; ++i)
		{
			cell* source;
			if (amx_GetAddr(vm, parameters[i + 3], &source))
			{
				LogScriptError(vm, "SendMessageToSquad() was unable to resolve the address of parameter %d.", i + 2);
				return 0; //The original leaks this allocation on argument failure.
			}
			values[i] = *source;
		}
		JE::IWorld* world = JE::g_World;
		std::list<DWORD>* members = world->GetSquads()->GetSquadMembers(DWORD(parameters[1]));
		for (std::list<DWORD>::iterator i = members->begin(); i != members->end(); ++i)
		{
			JE::Entity* entity = JE::GetEntityPtr(*i);
			if (!entity)
				LogScriptError(vm, "SendMessageToSquad() was unable to retrieve an entity. Internal error.");
			else
			{
				entity->EnqueueMessage(DWORD(reinterpret_cast<UINT_PTR>(owner)),
					DWORD(parameters[2]), values, provided - 2);
				++delivered;
			}
		}
		delete[] values;
		return delivered;
	}
}
