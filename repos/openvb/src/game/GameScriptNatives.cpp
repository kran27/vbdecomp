#include "../startup/NetworkComponent.hpp"
#include "GameScriptNatives.hpp"
#include "entity/Action.hpp"
#include "entity/ActionMoveTo.hpp"
#include "entity/ActionWait.hpp"
#include "entity/Entity.hpp"
#include "entity/World.hpp"
#include "entity/Gregorian.hpp"
#include "entity/EntityEnvironment.hpp"
#include "entity/MapEffect.hpp"
#include "model/CommonException.hpp"
#include "scripting/ScriptEnvironment.hpp"
#include "scripting/ScriptDebugger.hpp"
#include "../startup/Random.hpp"
#include "../startup/GameScriptingComponent.hpp"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <map>
#include <stdexcept>

extern "C" AMX_NATIVE_INFO float_Natives[];

namespace
{
	JE::World& ServerWorld()
	{
		if(!(JE::GetEntityEnvironment().NetworkMode()&1))
			throw ::Common::Exception("Invalid attempt to retrieve world server instance.");
		return *JE::g_World;
	}
	template<DWORD Address> cell AMX_NATIVE_CALL UnresolvedNative(AMX*, cell*)
	{
		char message[96];
		sprintf(message, "Proprietary script native0x%08lX remains unresolved.", Address);
		throw std::logic_error(message);
	}

}

namespace F3
{
	// Original6FFAF8, all155 names in original order. Unresolved entries are
	// addressed API boundaries, not reconstructed native-body progress.
	AMX_NATIVE_INFO engineNatives[] =
	{
		{ const_cast<char*>("Sample"), F3::NativeSample },
		{ const_cast<char*>("ClearActions"), F3::NativeClearActions },
		{ const_cast<char*>("DestroyItem"), F3::NativeDestroyItem },
		{ const_cast<char*>("GiveNewItem"), F3::NativeGiveNewItem },
		{ const_cast<char*>("A_MoveToEntity"), F3::NativeMoveToEntity },
		{ const_cast<char*>("A_MoveToPosition"), F3::NativeMoveToPosition },
		{ const_cast<char*>("TransferItem"), F3::NativeTransferItem },
		{ const_cast<char*>("TransferItemFromParty"), F3::NativeTransferItemFromParty },
		{ const_cast<char*>("TransferItemToParty"), F3::NativeTransferItemToParty },
		{ const_cast<char*>("DropItem"), F3::NativeDropItem },
		{ const_cast<char*>("A_Wander"), F3::NativeWander },
		{ const_cast<char*>("SpawnEntity"), F3::NativeSpawnEntity },
		{ const_cast<char*>("DestroyEntity"), F3::NativeDestroyEntity },
		{ const_cast<char*>("A_Kill"), F3::NativeKill },
		{ const_cast<char*>("A_Wait"), F3::NativeWait },
		{ const_cast<char*>("A_PlayAnim"), F3::NativePlayAnim },
		{ const_cast<char*>("MapEffectSetActive"), F3::NativeMapEffectSetActive },
		{ const_cast<char*>("MapSoundSetActive"), F3::NativeMapSoundSetActive },
		{ const_cast<char*>("MapEffectResetElapsedTime"), F3::NativeMapEffectResetElapsedTime },
		{ const_cast<char*>("JoinParty"), F3::NativeJoinParty },
		{ const_cast<char*>("LeaveParty"), F3::NativeLeaveParty },
		{ const_cast<char*>("TeleportParty"), F3::NativeTeleportParty },
		{ const_cast<char*>("ActivateEntity"), F3::NativeActivateEntity },
		{ const_cast<char*>("DeactivateEntity"), F3::NativeDeactivateEntity },
		{ const_cast<char*>("EnableDialogue"), F3::NativeEnableDialogue },
		{ const_cast<char*>("DisableDialogue"), F3::NativeDisableDialogue },
		{ const_cast<char*>("PickupItem"), F3::NativePickupItem },
		{ const_cast<char*>("A_PickupItem"), F3::NativeActionPickupItem },
		{ const_cast<char*>("QueueFunction"), F3::NativeQueueFunction },
		{ const_cast<char*>("ClearFunctionQueue"), F3::NativeClearFunctionQueue },
		{ const_cast<char*>("StartCutscene"), F3::NativeStartCutscene },
		{ const_cast<char*>("StopCutscene"), F3::NativeStopCutscene },
		{ const_cast<char*>("IsInCutscene"), F3::NativeIsInCutscene },
		{ const_cast<char*>("Distance"), F3::NativeDistance },
		{ const_cast<char*>("FindEntity"), F3::NativeFindEntity },
		{ const_cast<char*>("GetPosition"), F3::NativeGetPosition },
		{ const_cast<char*>("SetPosition"), F3::NativeSetPosition },
		{ const_cast<char*>("GetFacing"), F3::NativeGetFacing },
		{ const_cast<char*>("SetFacing"), F3::NativeSetFacing },
		{ const_cast<char*>("SetFacingTarget"), F3::NativeSetFacingTarget },
		{ const_cast<char*>("SetFacingPoint"), F3::NativeSetFacingPoint },
		{ const_cast<char*>("GetDirectionToEntity"), F3::NativeGetDirectionToEntity },
		{ const_cast<char*>("GetDirectionToPoint"), F3::NativeGetDirectionToPoint },
		{ const_cast<char*>("IsIdle"), F3::NativeIsIdle },
		{ const_cast<char*>("IsDead"), F3::NativeIsDead },
		{ const_cast<char*>("IsInParty"), F3::NativeIsInParty },
		{ const_cast<char*>("Myself"), F3::NativeMyself },
		{ const_cast<char*>("GetGlobal"), F3::NativeGetGlobal },
		{ const_cast<char*>("SetGlobal"), F3::NativeSetGlobal },
		{ const_cast<char*>("IncGlobal"), F3::NativeIncGlobal },
		{ const_cast<char*>("DecGlobal"), F3::NativeDecGlobal },
		{ const_cast<char*>("GetValue"), F3::NativeGetValue },
		{ const_cast<char*>("SetValue"), F3::NativeSetValue },
		{ const_cast<char*>("IncValue"), F3::NativeIncValue },
		{ const_cast<char*>("DecValue"), F3::NativeDecValue },
		{ const_cast<char*>("HasItem"), F3::NativeHasItem },
		{ const_cast<char*>("PartyHasItem"), F3::NativePartyHasItem },
		{ const_cast<char*>("FindObjects"), F3::NativeFindObjects },
		{ const_cast<char*>("MatchEntity"), F3::NativeMatchEntity },
		{ const_cast<char*>("GetObject"), F3::NativeGetObject },
		{ const_cast<char*>("GetLOS"), F3::NativeGetLOS },
		{ const_cast<char*>("GetLOSPoint"), F3::NativeGetLOSPoint },
		{ const_cast<char*>("SetModel"), F3::NativeSetModel },
		{ const_cast<char*>("GetAreaRestStatus"), F3::NativeGetAreaRestStatus },
		{ const_cast<char*>("SetAreaRestStatus"), F3::NativeSetAreaRestStatus },
		{ const_cast<char*>("IsEntityInArea"), F3::NativeIsEntityInArea },
		{ const_cast<char*>("GetMapID"), F3::NativeGetMapID },
		{ const_cast<char*>("RecordDeath"), F3::NativeRecordDeath },
		{ const_cast<char*>("HasDied"), F3::NativeHasDied },
		{ const_cast<char*>("IsUnderFOW"), F3::NativeIsUnderFOW },
		{ const_cast<char*>("IsInCombat"), F3::NativeIsInCombat },
		{ const_cast<char*>("GetType"), F3::NativeGetType },
		{ const_cast<char*>("EditToken"), F3::NativeEditToken },
		{ const_cast<char*>("FindTeamMembers"), F3::NativeFindTeamMembers },
		{ const_cast<char*>("GetTeamMember"), F3::NativeGetTeamMember },
		{ const_cast<char*>("GetTeam"), F3::NativeGetTeam },
		{ const_cast<char*>("AddToTeam"), F3::NativeAddToTeam },
		{ const_cast<char*>("RemoveFromTeam"), F3::NativeRemoveFromTeam },
		{ const_cast<char*>("GetTeamRank"), F3::NativeGetTeamRank },
		{ const_cast<char*>("SetTeamRank"), F3::NativeSetTeamRank },
		{ const_cast<char*>("SetTeamAttitude"), F3::NativeSetTeamAttitude },
		{ const_cast<char*>("CreateTeam"), F3::NativeCreateTeam },
		{ const_cast<char*>("FindSquadMembers"), F3::NativeFindSquadMembers },
		{ const_cast<char*>("GetSquadMEmber"), F3::NativeGetSquadMember },
		{ const_cast<char*>("GetSquadID"), F3::NativeGetSquadID },
		{ const_cast<char*>("GetSquadRank"), F3::NativeGetSquadRank },
		{ const_cast<char*>("CreateSquad"), F3::NativeCreateSquad },
		{ const_cast<char*>("RemoveFromSquad"), F3::NativeRemoveFromSquad },
		{ const_cast<char*>("SetSquadRank"), F3::NativeSetSquadRank },
		{ const_cast<char*>("AddToSquad"), F3::NativeAddToSquad },
		{ const_cast<char*>("SetSquadAttitude"), F3::NativeSetSquadAttitude },
		{ const_cast<char*>("GetNavPointCount"), F3::NativeGetNavPointCount },
		{ const_cast<char*>("GetNavPointLinks"), F3::NativeGetNavPointLinks },
		{ const_cast<char*>("GetNavPointPosition"), F3::NativeGetNavPointPosition },
		{ const_cast<char*>("GetNavPointOrientation"), F3::NativeGetNavPointOrientation },
		{ const_cast<char*>("GetWayPointCount"), F3::NativeGetWayPointCount },
		{ const_cast<char*>("GetWayPointPosition"), F3::NativeGetWayPointPosition },
		{ const_cast<char*>("GetWayPointOrientation"), F3::NativeGetWayPointOrientation },
		{ const_cast<char*>("GetNearestPath"), F3::NativeGetNearestPath },
		{ const_cast<char*>("A_SendMessage"), F3::NativeActionSendMessage },
		{ const_cast<char*>("A_SendMessageToTeam"), F3::NativeActionSendMessageToTeam },
		{ const_cast<char*>("SendMessage"), F3::NativeSendMessage },
		{ const_cast<char*>("SendMessageToTeam"), F3::NativeSendMessageToTeam },
		{ const_cast<char*>("MessageArgCount"), F3::NativeMessageArgCount },
		{ const_cast<char*>("MessageGetArg"), F3::NativeMessageGetArg },
		{ const_cast<char*>("SendMessageToSquad"), F3::NativeSendMessageToSquad },
		{ const_cast<char*>("A_SendMessageToSquad"), F3::NativeActionSendMessageToSquad },
		{ const_cast<char*>("Year"), F3::NativeYear },
		{ const_cast<char*>("Month"), F3::NativeMonth },
		{ const_cast<char*>("Day"), F3::NativeDay },
		{ const_cast<char*>("Hour"), F3::NativeHour },
		{ const_cast<char*>("Minute"), F3::NativeMinute },
		{ const_cast<char*>("Second"), F3::NativeSecond },
		{ const_cast<char*>("GetTimeStamp"), F3::NativeGetTimeStamp },
		{ const_cast<char*>("YearsPassed"), F3::NativeYearsPassed },
		{ const_cast<char*>("MonthsPassed"), F3::NativeMonthsPassed },
		{ const_cast<char*>("DaysPassed"), F3::NativeDaysPassed },
		{ const_cast<char*>("HoursPassed"), F3::NativeHoursPassed },
		{ const_cast<char*>("MinutesPassed"), F3::NativeMinutesPassed },
		{ const_cast<char*>("SecondsPassed"), F3::NativeSecondsPassed },
		{ const_cast<char*>("DialogueCall"), F3::NativeDialogueCall },
		{ const_cast<char*>("RegisterLinkToOtherDialogue"), F3::NativeRegisterLinkToOtherDialogue },
		{ const_cast<char*>("RegisterNodeText"), F3::NativeRegisterNodeText },
		{ const_cast<char*>("RegisterReply"), F3::NativeRegisterReply },
		{ const_cast<char*>("SetNodeTextFunction"), F3::NativeSetNodeTextFunction },
		{ const_cast<char*>("SetCurrentNodeName"), F3::NativeSetCurrentNodeName },
		{ const_cast<char*>("WasLastNodeName"), F3::NativeWasLastNodeName },
		{ const_cast<char*>("IsInDialogue"), F3::NativeIsInDialogue },
		{ const_cast<char*>("PCSpeaker"), F3::NativePCSpeaker },
		{ const_cast<char*>("SetTokenStrref"), F3::NativeSetTokenStrref },
		{ const_cast<char*>("SetTokenInt"), F3::NativeSetTokenInt },
		{ const_cast<char*>("RandomNumber"), F3::NativeRandomNumber },
		{ const_cast<char*>("RandomNumberFloat"), F3::NativeRandomNumberFloat },
		{ const_cast<char*>("Debug"), F3::NativeDebug },
		{ const_cast<char*>("PointDistance"), F3::NativePointDistance },
		{ const_cast<char*>("Breakpoint"), F3::NativeBreakpoint },
		{ const_cast<char*>("AttachVEG"), F3::NativeAttachVEG },
		{ const_cast<char*>("RemoveVEG"), F3::NativeRemoveVEG },
		{ const_cast<char*>("GetCameraFog"), F3::NativeGetCameraFog },
		{ const_cast<char*>("SetCameraFog"), F3::NativeSetCameraFog },
		{ const_cast<char*>("ResetCameraFog"), F3::NativeResetCameraFog },
		{ const_cast<char*>("GetHeightFog"), F3::NativeGetHeightFog },
		{ const_cast<char*>("SetHeightFog"), F3::NativeSetHeightFog },
		{ const_cast<char*>("ResetHeightFog"), F3::NativeResetHeightFog },
		{ const_cast<char*>("GetVEGID"), F3::NativeGetVEGID },
		{ const_cast<char*>("AttachSound"), F3::NativeAttachSound },
		{ const_cast<char*>("RemoveSound"), F3::NativeRemoveSound },
		{ const_cast<char*>("FloatingSpeechText"), F3::NativeImmediateFloatingSpeechText },
		{ const_cast<char*>("A_FloatingSpeechText"), F3::NativeFloatingSpeechText },
		{ const_cast<char*>("FloatingDamageText"), F3::NativeFloatingDamageText },
		{ const_cast<char*>("SetEncyclopediaData"), F3::NativeSetEncyclopediaData },
		{ const_cast<char*>("SetWorldMapAreaVisible"), F3::NativeSetWorldMapAreaVisible },
		{ const_cast<char*>("SetWorldMapAreaExplorable"), F3::NativeSetWorldMapAreaExplorable },
		{ const_cast<char*>("SetWorldMapAreaExplored"), F3::NativeSetWorldMapAreaExplored },
		{ const_cast<char*>("StartStore"), F3::NativeStartStore },
		{ NULL, NULL }
	};

	// Original6F3750, all45 game names in original order.
	AMX_NATIVE_INFO gameNatives[] =
	{
		{ const_cast<char*>("A_Attack"), F3::NativeAttack },
		{ const_cast<char*>("A_JustAttack"), F3::NativeJustAttack },
		{ const_cast<char*>("StartDialogue"), F3::NativeStartDialogue },
		{ const_cast<char*>("DisplayStringRef"), F3::NativeDisplayStringRef },
		{ const_cast<char*>("HideInterfaces"), F3::NativeHideInterfaces },
		{ const_cast<char*>("SetMoveMode"), F3::NativeSetMoveMode },
		{ const_cast<char*>("EquipItem"), F3::NativeEquipItem },
		{ const_cast<char*>("UnequipItem"), F3::NativeUnequipItem },
		{ const_cast<char*>("LockDoor"), F3::NativeLockDoor },
		{ const_cast<char*>("UnlockDoor"), F3::NativeUnlockDoor },
		{ const_cast<char*>("AddCNPCToParty"), F3::NativeAddCNPCToParty },
		{ const_cast<char*>("A_ReloadWeapon"), F3::NativeReloadWeapon },
		{ const_cast<char*>("Protagonist"), F3::NativeProtagonist },
		{ const_cast<char*>("GetWeaponCombo"), F3::NativeGetWeaponCombo },
		{ const_cast<char*>("SetWeaponCombo"), F3::NativeSetWeaponCombo },
		{ const_cast<char*>("GetSquadAttitude"), F3::NativeGetSquadAttitude },
		{ const_cast<char*>("GetTeamAttitude"), F3::NativeGetTeamAttitude },
		{ const_cast<char*>("GetPerkRank"), F3::NativeGetPerkRank },
		{ const_cast<char*>("GetLevel"), F3::NativeGetLevel },
		{ const_cast<char*>("GetPrimaryAttribute"), F3::NativeGetPrimaryAttribute },
		{ const_cast<char*>("GetDerivedAttribute"), F3::NativeGetDerivedAttribute },
		{ const_cast<char*>("GetBaseSkill"), F3::NativeGetBaseSkill },
		{ const_cast<char*>("GetTotalSkill"), F3::NativeGetTotalSkill },
		{ const_cast<char*>("GetRace"), F3::NativeGetRace },
		{ const_cast<char*>("GetGender"), F3::NativeGetGender },
		{ const_cast<char*>("GetEthnicity"), F3::NativeGetEthnicity },
		{ const_cast<char*>("GetSubRace"), F3::NativeGetSubRace },
		{ const_cast<char*>("GetMaxHP"), F3::NativeGetMaxHP },
		{ const_cast<char*>("GetCurrentHP"), F3::NativeGetCurrentHP },
		{ const_cast<char*>("GetWeaponRange"), F3::NativeGetWeaponRange },
		{ const_cast<char*>("GetItemType"), F3::NativeGetItemType },
		{ const_cast<char*>("GetUsableObjectType"), F3::NativeGetUsableObjectType },
		{ const_cast<char*>("SetHP"), F3::NativeSetHP },
		{ const_cast<char*>("AddHP"), F3::NativeAddHP },
		{ const_cast<char*>("IsLocked"), F3::NativeIsLocked },
		{ const_cast<char*>("IsItemEquipped"), F3::NativeIsItemEquipped },
		{ const_cast<char*>("GetLockDC"), F3::NativeGetLockDC },
		{ const_cast<char*>("PlayMusic"), F3::NativePlayMusic },
		{ const_cast<char*>("OpenTask"), F3::NativeOpenTask },
		{ const_cast<char*>("CloseTask"), F3::NativeCloseTask },
		{ const_cast<char*>("AddEntry"), F3::NativeAddEntry },
		{ const_cast<char*>("OpenQuest"), F3::NativeNullQuest },
		{ const_cast<char*>("CloseQuest"), F3::NativeNullQuest },
		{ const_cast<char*>("AddQuestEntry"), F3::NativeNullQuest },
		{ const_cast<char*>("AddJournalEntry"), F3::NativeNullQuest },
		{ NULL, NULL }
	};
}

namespace F3
{
	void LogScriptError(AMX* vm, const char* format, ...)
	{
		const char* script = "<Unknown>";
		void* ownerData;
		if (!amx_GetUserData(vm, 0x464C534D, &ownerData)) // Original FLSM.
		{
			switch (reinterpret_cast<DWORD>(ownerData) >> 29)
			{
			case 0:
				{
					JE::Entity* entity = JE::GetEntityPtr(reinterpret_cast<DWORD>(ownerData));
					if (entity) script = entity->scriptName.c_str();
					break;
				}
			case 1:
				{
					JE::Map* map=JE::GetWorldServerInstance()->GetMap(reinterpret_cast<DWORD>(ownerData));
					if (map) script=map->scriptName.c_str();
					break;
				}
			}
		}
		char filename[256] = "<unknown>", extension[256] = { 0 };
		void* fileData;
		if (!amx_GetUserData(vm, 0x4C425446, &fileData)) // Original FTBL.
		{
			typedef JE::ScriptDebug::FileMap FileMap;
			FileMap* files = static_cast<FileMap*>(fileData);
			LONG fileIndex=vm->curfile;
			FileMap::iterator found = files->find(fileIndex);
			if (found != files->end())
				_splitpath(found->second.c_str(), NULL, NULL, filename, extension);
		}
		char message[1028];
		sprintf(message, "!Run-time error in script %s, file %s%s, line %d: ",
			script, filename, extension, vm->curline);
		va_list arguments;
		va_start(arguments, format);
		vsprintf(message + strlen(message), format, arguments);
		va_end(arguments);
		// Preserve the original buffer-as-format call into the actual Scripting
		// virtual service, including its subsequent formatting policy.
		g_pScriptingInstance->DebugOutput(message);
	}

	cell AMX_NATIVE_CALL NativeClearActions(AMX* vm, cell* parameters)
	{
		if (!VerifyScriptParameters(vm,parameters,1,"ClearActions")) return 0;
		JE::Entity* entity;
		if (!ResolveScriptEntity(vm,parameters[1],&entity,1))
		{ LogScriptError(vm,"ClearActions() is unable to find the requested entity."); return 0; }
		entity->ClearActions();
		return 1;
	}
	cell AMX_NATIVE_CALL NativeMoveToPosition(AMX* vm, cell* parameters)
	{
		if (!VerifyScriptParameters(vm,parameters,4,"MoveToPosition")) return 0;
		JE::Entity* entity;
		if (!ResolveScriptEntity(vm,parameters[1],&entity,1))
		{ LogScriptError(vm,"MoveToPosition() was unable to find the requested entity to move."); return 0; }
		D3DXVECTOR3 destination(0,0,0);
		if (!JE::ScriptServices::ReadNativeVector(vm,parameters[2],destination))
		{ LogScriptError(vm,"MoveToPosition() was unable to retrieve the destination."); return 0; }
		JE::ActionMoveTo* action = static_cast<JE::ActionMoveTo*>(ServerWorld().NewAction(2));
		action->destination = destination;
		if (parameters[3] > 0) action->stopDistance = parameters[3];
		//Original verifies4 parameters but does not consume parameters[4].
		entity->QueueAction(action);
		return 1;
	}
	cell AMX_NATIVE_CALL NativeMoveToEntity(AMX* vm, cell* parameters)
	{
		if (!VerifyScriptParameters(vm,parameters,4,"MoveToEntity")) return 0;
		JE::Entity* entity;
		if (!ResolveScriptEntity(vm,parameters[1],&entity,1))
		{ LogScriptError(vm,"MoveToEntity() was unable to find the requested entity to move."); return 0; }
		JE::Entity* target;
		if (!ResolveScriptEntity(vm,parameters[2],&target,1))
		{ LogScriptError(vm,"MoveToEntity() was unable to find the requested target entity."); return 0; }
		JE::ActionMoveTo* action = static_cast<JE::ActionMoveTo*>(ServerWorld().NewAction(2));
		action->SetTarget(target);
		if (parameters[3] < 0) action->stopDistance = parameters[3]; //Original negative-only store.
		if (parameters[4] != -1)
		{ action->reachability = parameters[4]; action->checkReachability = 1; }
		entity->QueueAction(action);
		return 1;
	}
	cell AMX_NATIVE_CALL NativeWait(AMX* vm, cell* parameters)
	{
		if (!VerifyScriptParameters(vm,parameters,2,"Wait")) return 0;
		JE::Entity* entity;
		if (!ResolveScriptEntity(vm,parameters[1],&entity,1))
		{ LogScriptError(vm,"Wait() was unable to find the requested entity."); return 0; }
		JE::ActionWait* action = static_cast<JE::ActionWait*>(JE::GetWorldInstance()->NewAction(5));
		action->SetDuration(parameters[2]);
		entity->QueueAction(action);
		return 1;
	}

	cell AMX_NATIVE_CALL NativeGetTimeStamp(AMX* vm,cell*)
	{
		// Original5C4910 has no parameter-count check, and reads calendar first.
		if (!(g_pNetworkInstance->GetMode() & 1))
			throw ::Common::Exception("Invalid attempt to retrieve world server instance.");
		JE::IWorldServer* world = JE::g_World;
		DWORD time = world->GetCalendar()->GetTimeStamp();
		void* data;
		if(amx_GetUserData(vm,0x464C534D,&data))
		{ LogScriptError(vm,"GetTimeStamp() was unable to retrieve 'myself' id."); return 0; }
		DWORD id=reinterpret_cast<DWORD>(data);
		if(!(id&0xE0000000))
		{
			JE::Entity* entity=JE::GetEntityPtr(id);
			if(entity) { time += entity->value29C; return cell(time); }
			LogScriptError(vm,"GetTimeStamp() was unable to retrieve 'myself' entity.");
		}
		return cell(time);
	}

	cell AMX_NATIVE_CALL NativeFindEntity(AMX* vm,cell* parameters)
	{
		if(!VerifyScriptParameters(vm,parameters,1,"FindEntity")) return 0;
		char name[260];
		if(!JE::ScriptServices::ReadNativeString(vm,parameters[1],name,sizeof(name)))
		{ LogScriptError(vm,"FindEntity() is unable to retrieve the instance name."); return 0; }
		JE::Entity* entity=JE::FindEntityByName(std::string(name));
		if(!entity)
		{ LogScriptError(vm,"FindEntity() was unable to find the requested entity."); return 0; }
		return cell(entity->entityID);
	}
	cell AMX_NATIVE_CALL NativeGetMapID(AMX* vm,cell* parameters)
	{
		if(!VerifyScriptParameters(vm,parameters,1,"GetMapID")) return 0;
		JE::Entity* entity;
		if(!ResolveScriptEntity(vm,DWORD(parameters[1]),&entity,1))
		{ LogScriptError(vm,"GetMapID() is unable to find the source entity."); return 0; }
		return cell(entity->mapHandle);
	}

	cell AMX_NATIVE_CALL NativeMapEffectSetActive(AMX* vm,cell* parameters)
	{
		if(!VerifyScriptParameters(vm,parameters,3,"MapEffectSetActive")) return 0;
		const DWORD mapID=DWORD(parameters[1]);
		char name[260];
		if(!JE::ScriptServices::ReadNativeString(vm,parameters[2],name,sizeof(name)))
		{ LogScriptError(vm,"MapEffectSetActive() is unable to retrieve effect instance name."); return 0; }
		JE::IWorld* world=JE::g_World;
		JE::Map* map=world->GetMap(mapID);
		if(!map)
		{ LogScriptError(vm,"MapEffectSetActive() is unable to find the specified map."); return 0; }
		std::list<JE::MapEffect*> effects;
		map->FindEffects(name,effects);
		if(effects.empty())
		{ LogScriptError(vm,"MapEffectSetActive() is unable to find the specified effect."); return 0; }
		for(std::list<JE::MapEffect*>::iterator i=effects.begin();i!=effects.end();++i)
			(*i)->SetEnabled(parameters[3]!=0);
		return 1; //5C9C22, missed in the decompiler's inferred return expression.
	}
	cell AMX_NATIVE_CALL NativeMapEffectResetElapsedTime(AMX* vm,cell* parameters)
	{
		if(!VerifyScriptParameters(vm,parameters,2,"MapEffectResetElapsedTime")) return 0;
		const DWORD mapID=DWORD(parameters[1]);
		char name[260];
		if(!JE::ScriptServices::ReadNativeString(vm,parameters[2],name,sizeof(name)))
		{ LogScriptError(vm,"MapEffectResetElapsedTime() is unable to retrieve effect instance name."); return 0; }
		JE::IWorld* world=JE::g_World;
		JE::Map* map=world->GetMap(mapID);
		if(!map)
		{ LogScriptError(vm,"MapEffectResetElapsedTime() is unable to find the specified map."); return 0; }
		std::list<JE::MapEffect*> effects;
		map->FindEffects(name,effects);
		if(effects.empty())
		{ LogScriptError(vm,"MapEffectResetElapsedTime() is unable to find the specified effect."); return 0; }
		for(std::list<JE::MapEffect*>::iterator i=effects.begin();i!=effects.end();++i)
			(*i)->ResetElapsedTime();
		return 1;
	}
	cell AMX_NATIVE_CALL NativeMyself(AMX* vm, cell* parameters)
	{
		if (!VerifyScriptParameters(vm, parameters, 0, "Myself")) return 0;
		void* owner;
		if (amx_GetUserData(vm, 0x464C534D, &owner))
		{
			LogScriptError(vm, "Myself() was unable to retrieve id.");
			return 0;
		}
		return reinterpret_cast<cell>(owner);
	}

	cell AMX_NATIVE_CALL NativePlayAnim(AMX* vm, cell* parameters)
	{
		if (!VerifyScriptParameters(vm, parameters, 5, "PlayAnim")) return 0;
		JE::Entity* entity;
		if (!ResolveScriptEntity(vm, parameters[1], &entity, 1))
		{
			LogScriptError(vm, "PlayAnim() was unable to find the requested entity.");
			return 0;
		}
		JE::ActionAnim* action = static_cast<JE::ActionAnim*>(JE::g_World->NewAction(6));
		action->group = parameters[2];
		action->file = parameters[3];
		action->loops = static_cast<WORD>(parameters[4]);
		action->reverse = parameters[5] != 0;
		entity->QueueAction(action);
		return 1;
	}
}

namespace F3
{
	AMX_NATIVE_INFO* GetScriptNativeTable(DWORD ordinal)
	{
		switch (ordinal)
		{
		case 0: return float_Natives; // Official supplied Small src/float.c.
		case 1: return engineNatives;
		case 2: return gameNatives;
		default:
			throw std::logic_error("Script native table ordinal outside actual three-table registration.");
		}
	}
}
