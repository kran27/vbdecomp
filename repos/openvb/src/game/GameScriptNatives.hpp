#pragma once

#include <windows.h>
extern "C"
{
#include "amx.h"
}

namespace JE
{
	class Entity;
}

namespace F3
{
	extern AMX_NATIVE_INFO engineNatives[]; //Original6FFAF8; actual table, not a provider adapter.
	extern AMX_NATIVE_INFO gameNatives[]; //Original6F3750; original order retained.
	AMX_NATIVE_INFO* GetScriptNativeTable(DWORD ordinal);
	cell AMX_NATIVE_CALL NativeSample(AMX* vm, cell* parameters); //5CA820, original reference-output diagnostic.
	cell AMX_NATIVE_CALL NativeEditToken(AMX* vm, cell* parameters); //5C7C60, actual Client B8 modal setup.
	cell AMX_NATIVE_CALL NativeDebug(AMX* vm, cell* parameters); //5C3710; partial derived debug-window writer.
	cell AMX_NATIVE_CALL NativeBreakpoint(AMX* vm, cell* parameters); //5C33A0; partial debugger UI owners.
	cell AMX_NATIVE_CALL NativePlayMusic(AMX* vm, cell* parameters); //45B9C0, original diagnostic/no-op.
	cell AMX_NATIVE_CALL NativeAttack(AMX* vm,cell* parameters); //459EB0, actual GameActionAttack queue.
	cell AMX_NATIVE_CALL NativeJustAttack(AMX* vm,cell* parameters); //459F70, disables turn/automatic approach.
	cell AMX_NATIVE_CALL NativeReloadWeapon(AMX* vm,cell* parameters); //45A380, actual GameActionReload queue.
	cell AMX_NATIVE_CALL NativeSetMoveMode(AMX* vm,cell* parameters); //45A140; raw mode+speed cell, server86 publication.
	cell AMX_NATIVE_CALL NativeGiveNewItem(AMX* vm,cell* parameters); //5C8A60, whole item instances.
	cell AMX_NATIVE_CALL NativeTransferItem(AMX* vm,cell* parameters); //5C9780, whole entity transfers.
	cell AMX_NATIVE_CALL NativeTransferItemFromParty(AMX* vm,cell* parameters); //5C9FF0.
	cell AMX_NATIVE_CALL NativeTransferItemToParty(AMX* vm,cell* parameters); //5CA1E0.
	cell AMX_NATIVE_CALL NativeHasItem(AMX* vm,cell* parameters); //5C84C0.
	cell AMX_NATIVE_CALL NativePartyHasItem(AMX* vm,cell* parameters); //5C88A0.
	cell AMX_NATIVE_CALL NativeLockDoor(AMX* vm,cell* parameters); //45A1B0, actual door lock BYTE.
	cell AMX_NATIVE_CALL NativeUnlockDoor(AMX* vm,cell* parameters); //45A240.
	cell AMX_NATIVE_CALL NativeDropItem(AMX* vm,cell* parameters); //5C9930, remove then actual map placement.
	cell AMX_NATIVE_CALL NativeDestroyEntity(AMX* vm,cell* parameters); //5CA5F0, deferred Entity::Destroy.
	cell AMX_NATIVE_CALL NativeSpawnEntity(AMX* vm,cell* parameters); //5CA650, current script-owner map.
	cell AMX_NATIVE_CALL NativeSetModel(AMX* vm,cell* parameters); //5C78A0, direct model-cache replacement.
	cell AMX_NATIVE_CALL NativeDestroyItem(AMX* vm,cell* parameters); //5C9650, first match then deferred destruction.
	cell AMX_NATIVE_CALL NativeStartCutscene(AMX* vm,cell* parameters); //5C9520, owner map and server77.
	cell AMX_NATIVE_CALL NativeStopCutscene(AMX* vm,cell* parameters); //5C95B0, return captured current ID.
	cell AMX_NATIVE_CALL NativeIsInCutscene(AMX* vm,cell* parameters); //5C9610, current ID, not Boolean.
	cell AMX_NATIVE_CALL NativePickupItem(AMX* vm,cell* parameters); //5C9380, detach then transfer.
	cell AMX_NATIVE_CALL NativeActionPickupItem(AMX* vm,cell* parameters); //5C9460, action3 +target +queue.
	cell AMX_NATIVE_CALL NativeKill(AMX* vm,cell* parameters); //5C8E70, actual World death-action factory.
	cell AMX_NATIVE_CALL NativeGetPrimaryAttribute(AMX* vm, cell* parameters); //45AC70.
	cell AMX_NATIVE_CALL NativeGetDerivedAttribute(AMX* vm, cell* parameters); //45AD00.
	cell AMX_NATIVE_CALL NativeGetBaseSkill(AMX* vm, cell* parameters); //45AD90.
	cell AMX_NATIVE_CALL NativeGetTotalSkill(AMX* vm, cell* parameters); //45AE20.
	cell AMX_NATIVE_CALL NativeGetRace(AMX* vm, cell* parameters); //45AEB0.
	cell AMX_NATIVE_CALL NativeGetGender(AMX* vm, cell* parameters); //45AF40.
	cell AMX_NATIVE_CALL NativeGetEthnicity(AMX* vm, cell* parameters); //45AFD0.
	cell AMX_NATIVE_CALL NativeGetSubRace(AMX* vm, cell* parameters); //45B070.
	cell AMX_NATIVE_CALL NativeGetLevel(AMX* vm, cell* parameters); //45ABE0.
	cell AMX_NATIVE_CALL NativeGetItemType(AMX* vm, cell* parameters); //45B240.
	cell AMX_NATIVE_CALL NativeGetUsableObjectType(AMX* vm, cell* parameters); //45B440.
	cell AMX_NATIVE_CALL NativeIsLocked(AMX* vm, cell* parameters); //45B4E0, doors only.
	cell AMX_NATIVE_CALL NativeGetLockDC(AMX* vm, cell* parameters); //45B570, container then door RTTI.
	cell AMX_NATIVE_CALL NativeIsItemEquipped(AMX* vm, cell* parameters); //45B750, resource-name/slot query.
	cell AMX_NATIVE_CALL NativeGetMaxHP(AMX* vm, cell* parameters); //45B110.
	cell AMX_NATIVE_CALL NativeGetCurrentHP(AMX* vm, cell* parameters); //45B1B0.
	cell AMX_NATIVE_CALL NativeSetHP(AMX* vm, cell* parameters); //45B2E0.
	cell AMX_NATIVE_CALL NativeAddHP(AMX* vm, cell* parameters); //45B390, DWORD wrapping addition.
	cell AMX_NATIVE_CALL NativeGetPerkRank(AMX* vm, cell* parameters); //45AB50.
	cell AMX_NATIVE_CALL NativeFindObjects(AMX* vm,cell* parameters); //5C7580.
	cell AMX_NATIVE_CALL NativeProtagonist(AMX* vm, cell* parameters); //45A960.
	cell AMX_NATIVE_CALL NativeJoinParty(AMX* vm, cell* parameters); //5C9030, actual Party virtual Add.
	cell AMX_NATIVE_CALL NativeAddCNPCToParty(AMX* vm, cell* parameters); //45A2D0, GameParty RTTI then raw Add BYTE.
	cell AMX_NATIVE_CALL NativeLeaveParty(AMX* vm, cell* parameters); //5C90D0, Contains then Remove.
	cell AMX_NATIVE_CALL NativeTeleportParty(AMX* vm,cell* parameters); //5CA3A0, member snapshot and named-map transfer.
	cell AMX_NATIVE_CALL NativeAttachVEG(AMX* vm,cell* parameters); //5C29B0, real attachment ID.
	cell AMX_NATIVE_CALL NativeRemoveVEG(AMX* vm,cell* parameters); //5C2A80, deferred remove and reliable19.
	cell AMX_NATIVE_CALL NativeGetVEGID(AMX* vm,cell* parameters); //5C2AF0, active binding exact-name lookup.
	cell AMX_NATIVE_CALL NativeSendMessage(AMX* vm, cell* parameters); //5C5410, variadic copied enqueue.
	cell AMX_NATIVE_CALL NativeQueueFunction(AMX* vm, cell* parameters); //5CA4F0, borrowed live FQUE.
	cell AMX_NATIVE_CALL NativeClearFunctionQueue(AMX* vm, cell* parameters); //5C9F80.
	cell AMX_NATIVE_CALL NativeStartDialogue(AMX* vm,cell* parameters); //45A040, approach/retry action.
	cell AMX_NATIVE_CALL NativeIsInDialogue(AMX* vm,cell* parameters); //5C39D0.
	cell AMX_NATIVE_CALL NativeIsInCombat(AMX* vm,cell* parameters); //5C7B30, entity state5 only.
	cell AMX_NATIVE_CALL NativeRecordDeath(AMX* vm,cell* parameters); //5C85C0.
	cell AMX_NATIVE_CALL NativeHasDied(AMX* vm,cell* parameters); //5C8750.
	cell AMX_NATIVE_CALL NativeEnableDialogue(AMX* vm,cell* parameters); //5C9280.
	cell AMX_NATIVE_CALL NativeDisableDialogue(AMX* vm,cell* parameters); //5C9300.
	cell AMX_NATIVE_CALL NativeGetAreaRestStatus(AMX* vm,cell* parameters); //5C7980.
	cell AMX_NATIVE_CALL NativeSetAreaRestStatus(AMX* vm,cell* parameters); //5C7A50.
	cell AMX_NATIVE_CALL NativeGetWeaponRange(AMX* vm,cell* parameters); //45B620.
	cell AMX_NATIVE_CALL NativeNullQuest(AMX* vm,cell* parameters); //420990, actual shared return1.
	cell AMX_NATIVE_CALL NativeIsUnderFOW(AMX* vm,cell* parameters); //5C7D10, actual graphics gate and map visibility.
	cell AMX_NATIVE_CALL NativeGetCameraFog(AMX* vm,cell* parameters); //5C2BB0.
	cell AMX_NATIVE_CALL NativeSetCameraFog(AMX* vm,cell* parameters); //5C2D20.
	cell AMX_NATIVE_CALL NativeResetCameraFog(AMX* vm,cell* parameters); //5C2EB0.
	cell AMX_NATIVE_CALL NativeGetHeightFog(AMX* vm,cell* parameters); //5C2F40.
	cell AMX_NATIVE_CALL NativeSetHeightFog(AMX* vm,cell* parameters); //5C30B0.
	cell AMX_NATIVE_CALL NativeResetHeightFog(AMX* vm,cell* parameters); //5C3260.
	cell AMX_NATIVE_CALL NativeStartStore(AMX* vm,cell* parameters); //5C21A0, original length/read failure preserved.
	cell AMX_NATIVE_CALL NativeSetWorldMapAreaVisible(AMX* vm,cell* parameters); //5C2240.
	cell AMX_NATIVE_CALL NativeSetWorldMapAreaExplorable(AMX* vm,cell* parameters); //5C22F0.
	cell AMX_NATIVE_CALL NativeSetWorldMapAreaExplored(AMX* vm,cell* parameters); //5C23A0.
	cell AMX_NATIVE_CALL NativeSetEncyclopediaData(AMX* vm,cell* parameters); //5C2450, targeted packet42.
	cell AMX_NATIVE_CALL NativeRegisterNodeText(AMX* vm,cell* parameters); //5C3980.
	cell AMX_NATIVE_CALL NativeRegisterReply(AMX* vm,cell* parameters); //5C4060.
	cell AMX_NATIVE_CALL NativeSetNodeTextFunction(AMX* vm,cell* parameters); //5C41B0.
	cell AMX_NATIVE_CALL NativeSetCurrentNodeName(AMX* vm,cell* parameters); //5C42A0.
	cell AMX_NATIVE_CALL NativeDialogueCall(AMX* vm,cell* parameters); //5C3C30.
	cell AMX_NATIVE_CALL NativeRegisterLinkToOtherDialogue(AMX* vm,cell* parameters); //5C3F20.
	cell AMX_NATIVE_CALL NativeWasLastNodeName(AMX* vm,cell* parameters); //5C4390.
	cell AMX_NATIVE_CALL NativePCSpeaker(AMX* vm,cell* parameters); //5C3A50.
	cell AMX_NATIVE_CALL NativeSetTokenStrref(AMX* vm,cell* parameters); //5C3AE0.
	cell AMX_NATIVE_CALL NativeSetTokenInt(AMX* vm,cell* parameters); //5C3B80.
	cell AMX_NATIVE_CALL NativeDisplayStringRef(AMX* vm,cell* parameters); //45A410, original absent-interface return0.
	cell AMX_NATIVE_CALL NativeActionSendMessage(AMX* vm, cell* parameters); //5C5120, action7.
	cell AMX_NATIVE_CALL NativeActionSendMessageToTeam(AMX* vm, cell* parameters); //5C52B0.
	cell AMX_NATIVE_CALL NativeActionSendMessageToSquad(AMX* vm, cell* parameters); //5C5660.
	cell AMX_NATIVE_CALL NativeSendMessageToTeam(AMX* vm, cell* parameters); //5C57C0.
	cell AMX_NATIVE_CALL NativeSendMessageToSquad(AMX* vm, cell* parameters); //5C5930.
	cell AMX_NATIVE_CALL NativeMessageArgCount(AMX* vm, cell* parameters); //5C5540.
	cell AMX_NATIVE_CALL NativeMessageGetArg(AMX* vm, cell* parameters); //5C55C0.
	cell AMX_NATIVE_CALL NativeOpenTask(AMX* vm, cell* parameters); //45B900.
	cell AMX_NATIVE_CALL NativeCloseTask(AMX* vm, cell* parameters); //45B940.
	cell AMX_NATIVE_CALL NativeAddEntry(AMX* vm, cell* parameters); //45B980.
	cell AMX_NATIVE_CALL NativeFloatingSpeechText(AMX* vm,cell* parameters); //5C2710, action8.
	cell AMX_NATIVE_CALL NativeRemoveSound(AMX* vm,cell* parameters); //5C2940, local removal then packet21.
	cell AMX_NATIVE_CALL NativeAttachSound(AMX* vm,cell* parameters); //5C2860, returns allocated request ID.
	cell AMX_NATIVE_CALL NativeImmediateFloatingSpeechText(AMX* vm,cell* parameters); //5C25E0.
	cell AMX_NATIVE_CALL NativeFloatingDamageText(AMX* vm, cell* parameters); //5C2500.
	cell AMX_NATIVE_CALL NativeEquipItem(AMX* vm,cell* parameters); //45A580.
	cell AMX_NATIVE_CALL NativeUnequipItem(AMX* vm,cell* parameters); //45A6F0.
	cell AMX_NATIVE_CALL NativeGetWeaponCombo(AMX* vm,cell* parameters); //45A9F0, actual resolver-only return0.
	cell AMX_NATIVE_CALL NativeSetWeaponCombo(AMX* vm,cell* parameters); //45AA40, actual resolver-only return0.
	cell AMX_NATIVE_CALL NativeWander(AMX* vm, cell* parameters); //5C8D40.
	cell AMX_NATIVE_CALL NativeIsIdle(AMX* vm, cell* parameters); //5C71F0.
	cell AMX_NATIVE_CALL NativeIsDead(AMX* vm, cell* parameters); //5C7250.
	cell AMX_NATIVE_CALL NativeIsInParty(AMX* vm, cell* parameters); //5C72C0.
	cell AMX_NATIVE_CALL NativeGetType(AMX* vm, cell* parameters); //5C7C00.
	cell AMX_NATIVE_CALL NativeMatchEntity(AMX* vm, cell* parameters); //5C6A50.
	cell AMX_NATIVE_CALL NativeGetObject(AMX* vm, cell* parameters); //5C7700.
	cell AMX_NATIVE_CALL NativeGetValue(AMX* vm, cell* parameters); //5C7390.
	cell AMX_NATIVE_CALL NativeSetValue(AMX* vm, cell* parameters); //5C7400.
	cell AMX_NATIVE_CALL NativeIncValue(AMX* vm, cell* parameters); //5C7480.
	cell AMX_NATIVE_CALL NativeDecValue(AMX* vm, cell* parameters); //5C7500.
	cell AMX_NATIVE_CALL NativeGetTeam(AMX* vm, cell* parameters); //5C6460.
	cell AMX_NATIVE_CALL NativeFindTeamMembers(AMX* vm, cell* parameters); //5C6390.
	cell AMX_NATIVE_CALL NativeGetTeamMember(AMX* vm, cell* parameters); //5C6420.
	cell AMX_NATIVE_CALL NativeGetTeamRank(AMX* vm, cell* parameters); //5C6550.
	cell AMX_NATIVE_CALL NativeFindSquadMembers(AMX* vm, cell* parameters); //5C6680.
	cell AMX_NATIVE_CALL NativeGetSquadMember(AMX* vm, cell* parameters); //5C6710.
	cell AMX_NATIVE_CALL NativeGetSquadID(AMX* vm, cell* parameters); //5C6750.
	cell AMX_NATIVE_CALL NativeGetSquadRank(AMX* vm, cell* parameters); //5C67C0.
	cell AMX_NATIVE_CALL NativeAddToTeam(AMX* vm, cell* parameters); //5C64D0.
	cell AMX_NATIVE_CALL NativeRemoveFromTeam(AMX* vm, cell* parameters); //5C6510.
	cell AMX_NATIVE_CALL NativeSetTeamRank(AMX* vm, cell* parameters); //5C65C0.
	cell AMX_NATIVE_CALL NativeSetTeamAttitude(AMX* vm, cell* parameters); //5C6600.
	cell AMX_NATIVE_CALL NativeGetTeamAttitude(AMX* vm, cell* parameters); //45AAF0.
	cell AMX_NATIVE_CALL NativeGetSquadAttitude(AMX* vm, cell* parameters); //45AA90.
	cell AMX_NATIVE_CALL NativeCreateTeam(AMX* vm, cell* parameters); //5C6650.
	cell AMX_NATIVE_CALL NativeCreateSquad(AMX* vm, cell* parameters); //5C6830.
	cell AMX_NATIVE_CALL NativeRemoveFromSquad(AMX* vm, cell* parameters); //5C6860.
	cell AMX_NATIVE_CALL NativeSetSquadRank(AMX* vm, cell* parameters); //5C68A0.
	cell AMX_NATIVE_CALL NativeAddToSquad(AMX* vm, cell* parameters); //5C68E0.
	cell AMX_NATIVE_CALL NativeSetSquadAttitude(AMX* vm, cell* parameters); //5C6920.
	cell AMX_NATIVE_CALL NativeGetNavPointCount(AMX* vm, cell* parameters); //5C5AA0.
	cell AMX_NATIVE_CALL NativeGetNavPointLinks(AMX* vm, cell* parameters); //5C5B50.
	cell AMX_NATIVE_CALL NativeGetNavPointPosition(AMX* vm, cell* parameters); //5C5C40.
	cell AMX_NATIVE_CALL NativeGetNavPointOrientation(AMX* vm, cell* parameters); //5C5D30.
	cell AMX_NATIVE_CALL NativeGetWayPointCount(AMX* vm, cell* parameters); //5C5FC0.
	cell AMX_NATIVE_CALL NativeGetWayPointPosition(AMX* vm, cell* parameters); //5C60E0.
	cell AMX_NATIVE_CALL NativeGetWayPointOrientation(AMX* vm, cell* parameters); //5C6240.
	cell AMX_NATIVE_CALL NativeGetNearestPath(AMX* vm, cell* parameters); //5C5E20.
	DWORD ResolveScriptMapID(DWORD ownerID); //59ACD0, actual tagged entity/map/trigger owner resolution.
	cell AMX_NATIVE_CALL NativeGetPosition(AMX* vm, cell* parameters); //5C6B30.
	cell AMX_NATIVE_CALL NativeSetPosition(AMX* vm, cell* parameters); //5C6BD0.
	cell AMX_NATIVE_CALL NativeGetFacing(AMX* vm, cell* parameters); //5C6C90.
	cell AMX_NATIVE_CALL NativeSetFacing(AMX* vm, cell* parameters); //5C6CF0.
	cell AMX_NATIVE_CALL NativeSetFacingTarget(AMX* vm, cell* parameters); //5C6D80.
	cell AMX_NATIVE_CALL NativeSetFacingPoint(AMX* vm, cell* parameters); //5C6EB0.
	cell AMX_NATIVE_CALL NativeGetDirectionToEntity(AMX* vm, cell* parameters); //5C6FF0.
	cell AMX_NATIVE_CALL NativeGetDirectionToPoint(AMX* vm, cell* parameters); //5C70E0.
	cell AMX_NATIVE_CALL NativeDistance(AMX* vm, cell* parameters); //5C6970.
	cell AMX_NATIVE_CALL NativePointDistance(AMX* vm, cell* parameters); //5C3780, numeric integer result.
	cell AMX_NATIVE_CALL NativeRandomNumberFloat(AMX* vm, cell* parameters); //5C38B0, actual48CD30/RandomNext.
	cell AMX_NATIVE_CALL NativeGetLOS(AMX* vm, cell* parameters); //5C7740.
	cell AMX_NATIVE_CALL NativeGetLOSPoint(AMX* vm, cell* parameters); //5C77E0.
	cell AMX_NATIVE_CALL NativeMapEffectResetElapsedTime(AMX* vm, cell* parameters); //5C9DF0.
	cell AMX_NATIVE_CALL NativeGetGlobal(AMX* vm, cell* parameters); //5C8000.
	cell AMX_NATIVE_CALL NativeSetGlobal(AMX* vm, cell* parameters); //5C8120.
	cell AMX_NATIVE_CALL NativeIncGlobal(AMX* vm, cell* parameters); //5C8200.
	cell AMX_NATIVE_CALL NativeDecGlobal(AMX* vm, cell* parameters); //5C8360.
	cell AMX_NATIVE_CALL NativeMyself(AMX* vm, cell* parameters); // 0x5C7340.
	cell AMX_NATIVE_CALL NativePlayAnim(AMX* vm, cell* parameters); // 0x5C8F90.
	cell AMX_NATIVE_CALL NativeMoveToPosition(AMX* vm, cell* parameters); //5C8C50.
	cell AMX_NATIVE_CALL NativeMoveToEntity(AMX* vm, cell* parameters); //5C8B70.
	cell AMX_NATIVE_CALL NativeClearActions(AMX* vm, cell* parameters); //5C8A00.
	cell AMX_NATIVE_CALL NativeActivateEntity(AMX* vm, cell* parameters); //5C9180, actual Entity+3A0 gate.
	cell AMX_NATIVE_CALL NativeDeactivateEntity(AMX* vm, cell* parameters); //5C9200.
	cell AMX_NATIVE_CALL NativeWait(AMX* vm, cell* parameters); //5C8F00.
	cell AMX_NATIVE_CALL NativeGetTimeStamp(AMX* vm, cell* parameters); //5C4910.
	cell AMX_NATIVE_CALL NativeSecondsPassed(AMX* vm, cell* parameters); //5C4F30.
	cell AMX_NATIVE_CALL NativeYear(AMX* vm, cell* parameters); //5C5040.
	cell AMX_NATIVE_CALL NativeMonth(AMX* vm, cell* parameters); //5C4490.
	cell AMX_NATIVE_CALL NativeDay(AMX* vm, cell* parameters); //5C4570.
	cell AMX_NATIVE_CALL NativeHour(AMX* vm, cell* parameters); //5C4650.
	cell AMX_NATIVE_CALL NativeMinute(AMX* vm, cell* parameters); //5C4730.
	cell AMX_NATIVE_CALL NativeSecond(AMX* vm, cell* parameters); //5C4810.
	cell AMX_NATIVE_CALL NativeYearsPassed(AMX* vm, cell* parameters); //5C49E0.
	cell AMX_NATIVE_CALL NativeMonthsPassed(AMX* vm, cell* parameters); //5C4AF0.
	cell AMX_NATIVE_CALL NativeDaysPassed(AMX* vm, cell* parameters); //5C4C00.
	cell AMX_NATIVE_CALL NativeHoursPassed(AMX* vm, cell* parameters); //5C4D10.
	cell AMX_NATIVE_CALL NativeMinutesPassed(AMX* vm, cell* parameters); //5C4E20.
	cell AMX_NATIVE_CALL NativeFindEntity(AMX* vm, cell* parameters); //5C7F10.
	cell AMX_NATIVE_CALL NativeGetMapID(AMX* vm, cell* parameters); //5C7BA0.
	cell AMX_NATIVE_CALL NativeIsEntityInArea(AMX* vm, cell* parameters); //5C7E00, logical map-name prefix.
	cell AMX_NATIVE_CALL NativeRandomNumber(AMX* vm, cell* parameters); //5C3870.
	cell AMX_NATIVE_CALL NativeMapEffectSetActive(AMX* vm, cell* parameters); //5C9AB0.
	cell AMX_NATIVE_CALL NativeMapSoundSetActive(AMX* vm, cell* parameters); //5C9C50.
	cell AMX_NATIVE_CALL NativeHideInterfaces(AMX* vm, cell* parameters); //45A100.
	BYTE VerifyScriptParameters(AMX* vm, cell* parameters, DWORD count,
		const char* name); // 0x59AF40.
	BYTE VerifyMinimumScriptParameters(AMX* vm, cell* parameters, LONG minimum,
		const char* name, LONG* provided); //59AF90, variadic minimum arity.
	BYTE ResolveScriptEntity(AMX* vm, DWORD id, JE::Entity** output,
		BYTE requireActive); // 0x59AEF0; inactive logs but still succeeds.
	void LogScriptError(AMX* vm, const char* format, ...); // 0x59AD20.
}
