#include <winsock2.h>
#include "GameScriptNatives.hpp"
#include "entity/Entity.hpp"
#include "entity/Map.hpp"
#include "entity/World.hpp"
#include "network/EngineServer.hpp"
#include "scripting/ScriptEnvironment.hpp"
#include "scripting/ScriptSmall.hpp"
#include "../startup/NetworkComponent.hpp"
#include <string>

namespace F3
{
	namespace
	{
		DWORD dialogueCallDepth = 0; //Original70CE00, initial BSS0; global rather than per manager.

		JE::DialogueManager& Dialogue()
		{
			//Each original operation reloads70BD90 -> Network.server -> Server+5064.
			JE::DialogueManager* manager = static_cast<JE::DialogueManager*>(
				static_cast<JE::Server*>(g_pNetworkInstance->server)->dialogue);
			return *manager;
		}
	}

	cell AMX_NATIVE_CALL NativeDialogueCall(AMX* vm, cell* parameters) //5C3C30.
	{
		if (!VerifyScriptParameters(vm, parameters, 2, "DialogueCall")) return 0;
		if (++dialogueCallDepth > 1) //Unsigned5C3C74/JBE, including original DWORD wrap.
		{
			LogScriptError(vm, "DialogCall() exceeded recursion limit.");
			--dialogueCallDepth;
			return 0;
		}
		char dialogueName[260];
		if (!JE::ScriptServices::ReadNativeString(vm, parameters[1], dialogueName, sizeof(dialogueName)))
		{
			LogScriptError(vm, "DialogueCall() was unable to retrieve the dialog name.");
			--dialogueCallDepth;
			return 0;
		}
		const DWORD previousSpeaker = Dialogue().GetSpeakerID();
		JE::Entity* speaker = JE::GetEntityPtr(previousSpeaker);
		std::string filename(dialogueName);
		JE::World* capturedWorld = JE::g_World;
		DWORD mapHandle = speaker->mapHandle;
		JE::IWorld* world = capturedWorld;
		JE::Map* map = world->GetMap(mapHandle);
		JE::Entity* target = map->FindNearest(speaker, filename);
		if (!target)
		{
			LogScriptError(vm, "DialogueCall() was unable to find the character with target dialogue name attached.");
			--dialogueCallDepth;
			return 0;
		}
		Dialogue().SetNextSpeakerID(target->entityID);
		Dialogue().SetSpeakerID(target->entityID);
		filename += ".dlg";
		char function[64];
		if (!JE::ScriptServices::ReadNativeString(vm, parameters[2], function, sizeof(function)))
		{
			LogScriptError(vm, "DialogueCall() was unable to retrieve the function name.");
			--dialogueCallDepth; //Original does not restore the speaker on this failure.
			return 0;
		}
		JE::Script_Small script(target->entityID);
		if (!script.Load(filename.c_str(), 1))
		{
			LogScriptError(vm, "DialogueCall() was unable to load the script for the target dialogue.");
			Dialogue().SetSpeakerID(previousSpeaker);
			--dialogueCallDepth;
			return 0;
		}
		cell result;
		if (!script.Run(function, &result))
		{
			LogScriptError(vm, "DialogueCall() was unable to execute the script function.");
			Dialogue().SetSpeakerID(previousSpeaker);
			--dialogueCallDepth;
			return 0;
		}
		Dialogue().SetSpeakerID(previousSpeaker);
		--dialogueCallDepth; //Before actual script destruction; NextSpeaker remains published.
		return result;
	}

	cell AMX_NATIVE_CALL NativeRegisterLinkToOtherDialogue(AMX* vm, cell* parameters) //5C3F20.
	{
		if (!VerifyScriptParameters(vm, parameters, 2, "RegisterLinkToOtherDialogue")) return 0;
		char scriptName[260], functionName[260];
		if (!JE::ScriptServices::ReadNativeString(vm, parameters[1], scriptName, sizeof(scriptName)))
		{
			LogScriptError(vm, "RegisterLinkToOtherDialogue() was unable to retrieve the script name.");
			return 0;
		}
		if (!JE::ScriptServices::ReadNativeString(vm, parameters[2], functionName, sizeof(functionName)))
		{
			LogScriptError(vm, "RegisterLinkToOtherDialogue() was unable to retrieve the function name.");
			return 0;
		}
		std::string function(functionName); //Original constructs the function temporary first.
		std::string script(scriptName);
		Dialogue().RegisterLinkToOtherDialogue(script, function);
		return 1; //5C403D, correcting the database void annotation.
	}
}
