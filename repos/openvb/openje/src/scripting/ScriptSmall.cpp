#include <winsock2.h>
#include "ScriptSmall.hpp"
#include "ScriptDebugger.hpp"
#include "resources/ResourceSystem.hpp"
#include "diagnostics/DebugOutput.hpp"
#include "network/EngineServer.hpp"
#include "network/EngineClient.hpp"
#include "gui/DebugWindow.hpp"
#include "game/GameScriptNatives.hpp"
#include "startup/NetworkComponent.hpp"
#include <stdlib.h>
#include <string.h>
#include <stdexcept>
#include <stddef.h>
#include <stdio.h>
#include <stack>

extern "C" AMX_NATIVE_INFO float_Natives[];

namespace
{
	const long fileTableTag = 0x4C425446;
	const long ownerTag = 0x464C534D;
	const long stateTag = 0x54415453;
	const long queueTag = 0x45555146;
	DWORD scriptDebugState; // Original70C724.

}

namespace JE
{
	IScript::~IScript()
	{
	}

	Script_Small::Script_Small(DWORD owner)
		: IScript(owner), debugging(0), fileDebug(0), program(NULL)
	{
#if defined(_MSC_VER) && _MSC_VER == 1300
		typedef char OriginalSize[(sizeof(Script_Small) == 0xE0) ? 1 : -1];
		typedef char OriginalOwner[(offsetof(Script_Small, ownerID) == 4) ? 1 : -1];
		typedef char OriginalFilename[(offsetof(Script_Small, filename) == 8) ? 1 : -1];
		typedef char OriginalDebugging[(offsetof(Script_Small, debugging) == 0x24) ? 1 : -1];
		typedef char OriginalFileDebug[(offsetof(Script_Small, fileDebug) == 0x25) ? 1 : -1];
		typedef char OriginalVMSize[(sizeof(AMX) == 0x98) ? 1 : -1];
		typedef char OriginalNativeInfoSize[(sizeof(AMX_NATIVE_INFO) == 8) ? 1 : -1];
		typedef char OriginalNativeInfoName[(offsetof(AMX_NATIVE_INFO, name) == 0) ? 1 : -1];
		typedef char OriginalNativeInfoFunction[(offsetof(AMX_NATIVE_INFO, func) == 4) ? 1 : -1];
		typedef char OriginalVM[(offsetof(Script_Small, vm) == 0x26) ? 1 : -1];
		typedef char OriginalQueue[(offsetof(Script_Small, functions) == 0xC0) ? 1 : -1];
		typedef char OriginalProgram[(offsetof(Script_Small, program) == 0xD0) ? 1 : -1];
		typedef char OriginalFiles[(offsetof(Script_Small, files) == 0xD4) ? 1 : -1];
#endif
		// Original leaves the VM bytes unwritten until Load/Unload initializes it.
	}

	Script_Small::~Script_Small()
	{
		Unload();
		// Actual members then destroy files, queued function strings and filename.
	}

	void Script_Small::Unload()
	{
		if (program != NULL)
		{
			if (debugging || fileDebug)
			{
				ScriptDebug::FileMap* files;
				amx_GetUserData(&vm, fileTableTag, reinterpret_cast<void**>(&files));
			}
			if (debugging)
			{
				// Original5A7349/5A7379 delete only on lookup failure.
				// Failed lookups leave output storage unwritten. VC7 coalesces
				// the distinct FTBL/VSTK scoped outputs into the original frame
				// slot, retaining its mismatched stack interpretation on failure.
				ScriptDebug::VariableMap* globals;
				std::stack<std::string>* stack;
				if (amx_GetUserData(&vm, 0x50414D56, reinterpret_cast<void**>(&globals)))
					delete globals;
				if (amx_GetUserData(&vm, 0x4B545356, reinterpret_cast<void**>(&stack)))
					delete stack;
			}
			free(program);
			program = NULL;
		}
		memset(&vm, 0, sizeof(vm));
		// Original retains filename, flags, function queue and file-index map.
	}

	int AMXAPI Script_Small::FileHook(AMX* amx)
	{
		AMX* vm = amx;
		switch (vm->dbgcode)
		{
		case DBG_FILE:
		if (amx_GetUserData(vm, fileTableTag, reinterpret_cast<void**>(&amx)) == 0)
		{
			// Original minimal hook inserts pointer-valued nodes even though
			// Load publishes its string-valued D4 member. Preserve this observed
			// incompatible specialization; full Debugger owns a separate FileMap.
			std::map<LONG,const char*>& files = *static_cast<std::map<LONG,const char*>*>(static_cast<void*>(amx));
			std::map<LONG,const char*>::iterator entry = files.insert(std::make_pair(static_cast<LONG>(vm->curfile), static_cast<const char*>(NULL))).first;
			entry->second = vm->dbgname;
		}
		break;
		}
		return 0;
	}

	BYTE Script_Small::Load(const char* name, BYTE debug)
	{
		int lastError;
		Client* client = static_cast<Client*>(F3::g_pNetworkInstance->client);
		DebugWindow* debugWindow = client->child0x8C;
		Unload();
		if (name == NULL || *name == 0)
		{
			LogDebugString("Warning: attempt to load NULL script.");
			return 0;
		}
		Resources::ResourceFileHandle* stream = Resources::g_pResourceSystem->OpenStream(name, 1);
		if (stream == NULL)
		{
			LogDebugString("Warning: attempt to load unknown script %s.", name);
			return 0;
		}
		AMX_HEADER header;
		if (!Resources::g_pResourceSystem->Read(&header, sizeof(header), stream))
		{
			LogDebugString("Warning: error reading header from script %s.  Script will not be loaded.\n", name);
			Resources::g_pResourceSystem->Release(stream, 0);
			return 0;
		}
		program = malloc(header.stp);
		if (!Resources::g_pResourceSystem->Seek(stream, 0, 0))
		{
			LogDebugString("Warning: seek error reading script %s.  Script will not be loaded.\n", name);
			free(program);
			program = NULL;
			Resources::g_pResourceSystem->Release(stream, 0);
			return 0;
		}
		if (!Resources::g_pResourceSystem->Read(program, header.size, stream))
		{
			LogDebugString("Warning: error reading script %s.  Script will not be loaded.\n", name);
			Resources::g_pResourceSystem->Release(stream, 0);
			return 0; // Original retains the allocated program until later Unload.
		}
		Resources::g_pResourceSystem->Release(stream, 0);
		memset(&vm, 0, sizeof(vm));
		if (debug == 1)
		{
			if (header.flags & 0x10)
			{
				if (debugWindow != NULL)
				{
					char buffer[256];
					sprintf(buffer, "Using Debugging Hook on file: %s", name);
					debugWindow->Write(std::string(buffer), 128, 128, 128);
				}
				debugging = 1;
				amx_SetDebugHook(&vm, Debugger);
			}
			else if (header.flags & 2)
			{
				fileDebug = 1;
				amx_SetDebugHook(&vm, FileHook);
			}
		}
		lastError = amx_Init(&vm, program);
		if (lastError != 0)
		{
			LogDebugString("Script_Small::Load() was unable to initialize the AMX.\n");
			free(program);
			program = NULL;
			return 0;
		}
		if (fileDebug == 1) amx_SetUserData(&vm, fileTableTag, &files);
		amx_Register(&vm, float_Natives, -1);
		amx_Register(&vm, F3::engineNatives, -1);
		lastError = amx_Register(&vm, F3::gameNatives, -1);
		if (lastError != 0)
		{
			LogDebugString("Script_Small::Load() Unable to locate a matching function for a Native in: %s\n", name);
			free(program);
			program = NULL;
			return 0;
		}
		amx_SetUserData(&vm, ownerTag, reinterpret_cast<void*>(static_cast<UINT_PTR>(ownerID)));
		scriptDebugState = debugging != 0;
		amx_SetUserData(&vm, stateTag,
			reinterpret_cast<void*>(static_cast<UINT_PTR>(scriptDebugState)));
		filename = name;
		amx_SetUserData(&vm, queueTag, &functions);
		return 1;
	}

	BYTE Script_Small::HasPublic(const char* name)
	{
		if (program == NULL) return 0;
		int index;
		return amx_FindPublic(&vm, const_cast<char*>(name), &index) == 0;
	}

	BYTE Script_Small::Run(const char* name, cell* result)
	{
		int lastError;
		if (program == NULL) return 0;
		int state = 0;
		if (debugging)
		{
			//5A5908..5A5912: only this overload rejects a missing STAT tag.
			if (amx_GetUserData(&vm, stateTag, reinterpret_cast<void**>(&state)) != 0) return 0;
			if (state == 2) return 0;
		}
		//Original STAT3 and enabled STAT4 share continuation and result handling.
		if ((debugging && state == 4) || state == 3)
		{
			if (state == 4)
			{
				state = 1;
				amx_SetUserData(&vm, stateTag, reinterpret_cast<void*>(state));
			}
			lastError = amx_Exec(&vm, result, AMX_EXEC_CONT, 0);
		}
		else
		{
			Server* server = static_cast<Server*>(F3::g_pNetworkInstance->server);
			if (server->ScriptSuppressed(ownerID)) return 0;
			char function[260];
			if (!strcmp("OnUpdate", name))
			{
				std::vector<std::string>* queue = NULL;
				amx_GetUserData(&vm, queueTag, reinterpret_cast<void**>(&queue)); //5A5983 reloads live FQUE.
				if (queue && queue->size())
				{
					strcpy(function, queue->front().c_str());
					queue->erase(queue->begin()); //5A59FB consumes before FindPublic.
				}
				else strcpy(function, name);
			}
			else strcpy(function, name);
			int index;
			lastError = amx_FindPublic(&vm, function, &index);
			if (lastError != 0) return 0;
			lastError = amx_Exec(&vm, result, index, 0); //5A5A57 calls Exec directly.
		}
		return lastError == 0 || lastError == AMX_ERR_SLEEP;
	}

	BYTE Script_Small::Run(const char* name, cell* result, int count, cell* arguments)
	{
		int lastError;
		if (program == NULL) return 0;
		int state = 0;
		if (debugging)
		{
			amx_GetUserData(&vm, stateTag, reinterpret_cast<void**>(&state));
			if (state == 2) return 0;
		}
		//Original STAT3 and enabled STAT4 share continuation and result handling.
		if ((debugging && state == 4) || state == 3)
		{
			if (state == 4)
			{
				state = 1;
				amx_SetUserData(&vm, stateTag, reinterpret_cast<void*>(state));
			}
			lastError = amx_Exec(&vm, result, AMX_EXEC_CONT, 0);
		}
		else
		{
			Server* server = static_cast<Server*>(F3::g_pNetworkInstance->server);
			if (server->ScriptSuppressed(ownerID)) return 0;
			char function[260];
			if (!strcmp("OnUpdate", name))
			{
				std::vector<std::string>* queue = NULL;
				amx_GetUserData(&vm, queueTag, reinterpret_cast<void**>(&queue)); //Original5A5B4B reloads live FQUE.
				if (queue && queue->size())
				{
					strcpy(function, queue->front().c_str());
					queue->erase(queue->begin()); //580770, consume before FindPublic.
				}
				else strcpy(function, name);
			}
			else strcpy(function, name);
			int index;
			lastError = amx_FindPublic(&vm, function, &index);
			if (lastError != 0) return 0;
			lastError = amx_Execv(&vm, result, index, count, arguments);
		}
		return lastError == 0 || lastError == AMX_ERR_SLEEP;
	}

	BYTE Script_Small::RunArray(const char* name, cell* result, cell* values, int count)
	{
		cell address;
		cell* storage;
		if (amx_Allot(&vm,count,&address,&storage) != 0) return 0;
		memset(storage,0,count*sizeof(cell));
		if (!Run(name,result,1,&address)) return 0;
		memcpy(values,storage,count*sizeof(cell));
		//5A43C0 does not release this VM allocation on either return path.
		return 1;
	}

	BYTE Script_Small::StepOver()
	{
		if (!debugging) return 0;
		void* state = NULL;
		amx_GetUserData(&vm, stateTag, &state);
		if (state != reinterpret_cast<void*>(2)) return 0;
		state = reinterpret_cast<void*>(4);
		amx_SetUserData(&vm, stateTag, state);
		return 1;
	}

	BYTE Script_Small::StepInto()
	{
		if (!debugging) return 0;
		void* state = NULL;
		amx_GetUserData(&vm, stateTag, &state);
		if (state != reinterpret_cast<void*>(2)) return 0;
		state = reinterpret_cast<void*>(3);
		amx_SetUserData(&vm, stateTag, state);
		return 1;
	}

	BYTE Script_Small::ContinueDebugging()
	{
		if (!debugging) return 0;
		void* state = NULL;
		amx_GetUserData(&vm, stateTag, &state);
		if (state != reinterpret_cast<void*>(2)) return 0;
		state = reinterpret_cast<void*>(1);
		amx_SetUserData(&vm, stateTag, state);
		void* stack;
		amx_GetUserData(&vm, 0x4B545356, &stack); // Original leaves output unwritten on failure.
		if (stack != NULL)
			static_cast<ScriptDebug::VariableMap*>(stack)->clear();
		return 1;
	}

	void Script_Small::ShowDebugState()
	{
		void* state = NULL;
		BYTE paused = 0;
		if (amx_GetUserData(&vm, stateTag, &state) == 0)
		{
			if (state == reinterpret_cast<void*>(2)) paused = 1;
			ScriptDebug::DisplayState(&vm, paused, 0);
		}
	}

	void Script_Small::PreLoad(const std::map<std::string,cell>& variables)
	{
		if (program == NULL)
		{
			LogDebugString("Warning: Attempt to preload script variables into an unloaded script.");
			return;
		}
		for (std::map<std::string,cell>::const_iterator i = variables.begin(); i != variables.end(); ++i)
		{
			cell address;
			cell* value;
			if (amx_FindPubVar(&vm, const_cast<char*>(i->first.c_str()), &address) == 0)
			{
				if (amx_GetAddr(&vm, address, &value) == 0) *value = i->second;
				else LogDebugString("Warning: Unable to resolve address of variable %s while preloading script %s.",
					i->first.c_str(), filename.c_str());
			}
			else LogDebugString("Warning: Variable %s not found while preloading script %s.",
				i->first.c_str(), filename.c_str());
		}
	}
}
