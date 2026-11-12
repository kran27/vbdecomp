#pragma once

#include "DynamicObject.hpp"
#include "Chunk.hpp"
#include "amx.h"
#include <map>
#include <list>
#include <string>
#include <deque>
#include <cstddef>

class Gfx_ModelInstance;
class Gfx_Viewport;
struct Gfx_AnimationCue;

namespace JE
{
	namespace Net { class Packet; }
	class ModelInstance;
	class Inventory;
	class EffectTreeTrigger;
	typedef EffectTreeTrigger EntityLight; //Original+2F4 payload, not a Gfx_Light.
	class Entity;
	class VFX_VEGInstance;
	class VisualEffect;
	class Action;
	class Effect;
	struct Script_Small;
	class World;
	class VfxSoundInstance;
	struct PathPoint;
	struct AreaMapHeight;
	struct ContextMenuEntry //568C20/568420, shared context/skill list payload.
	{
		explicit ContextMenuEntry(DWORD value)
		{
			selector=value;
			text="";
			stringReference=0;
			value24=0;
			value28=0;
			disabled=0;
		}
		~ContextMenuEntry();
		void SetStringReference(DWORD value); //568420, ordinary member; field+20.
		DWORD selector;
		std::string text;
		DWORD stringReference, value24, value28;
		BYTE disabled;
	};
	template<class T> class RLEPrimitive;
	template<class T> class RLEGrid;
	struct EntityEffectBinding
	{
		DWORD id;
		VisualEffect* object;
		BYTE deleteObject;
		float elapsedSeconds; //Original binding+0C, incremented by client-frame delta.
		VFX_VEGInstance* instance;
		BYTE releaseInstance, enabled;
	};
	struct EntityAnimationRequest
	{
		DWORD group, file, loops;
		BYTE reverse;
		DWORD timeOffset;
		BYTE clearPending;
	};
	struct EntityMapMessage
	{
		DWORD words[2]; // +320 list payload width established by16-byte node allocation.
	};
	struct EntityMessage
	{
		EntityMessage() : cellCount(0), values(NULL) {}
		DWORD cellCount; //54EE60: includes sender and message ID.
		cell* values; //Owned copied cells: sender, message ID, optional arguments.
	};
	struct EntitySoundBinding
	{
		DWORD id;
		VfxSoundInstance* sound;
		BYTE deleteObject;
		float elapsedSeconds; // Original16-byte map payload in550AD0/54F4D0.
	};
	Entity* GetEntityPtr(DWORD id); // 0x54BE10; creation list before registered map.
	Entity* FindEntityByName(std::string name); // 0x54B370, creation list only.
	DWORD AllocateVisualEffectID(); // 0x552A20, shared70BE60 counter, skips zero after wrap.
	DWORD AllocateSoundID(); //552A40, separate70BE64 counter, skips zero after wrap.
	extern std::list<Entity*> g_createdEntities; //Original70BE08; actual shared creation-list storage.
	void VisitEntities(void (*visitor)(Entity*,void*), void* context); // Live70BE08 traversal for actual Combat.
	void ProcessEntityCreationList(void (*deleteEntity)(Entity*)); // Factored545D60 creation/retirement pass.
	void DeleteAllEntities(); //54694D..546975, repeatedly deletes the live creation-list head.
	void UpdateEntities(World& world); // Factored545D60 live update-list pass.
	// Recovered engine fields/methods and Create parent. The visible entry supplies
	// explicit world/network inputs around a partial construction seam; complete
	// lifetime, world activation and networking/script owners remain open.
	class Entity : public DynamicObject
	{
		friend class Map;
		friend class World;
	public:
		// Original Entity additions28..E4 follow DynamicObject's00..24 prefix.
		virtual void CopyFrom(Entity* source); //552420,+28.
		virtual DWORD GetUpdateInterval() const; //408080,+2C.
		virtual DWORD GetContextAction(Entity* actor); //404BB0,+30, base0.
		virtual void GetContextActions(Entity* actor, std::list<ContextMenuEntry>& entries); //408090,+34.
		// H-TYPE unresolved: only the original empty RET8 ABI shape is known.
		// Two DWORDs are opaque compatibility spelling, not a recovered prototype.
		virtual void UnresolvedSlot38(DWORD opaque0, DWORD opaque1); //408090,+38.
		virtual void SetMap(DWORD map); //54BBA0,+3C.
		virtual void SetUpdateInterval(DWORD value); //550750,+40.
		virtual void SetSelectable(BYTE value); //54B540,+44.
		virtual void ResolveAnimation(DWORD group, DWORD file, DWORD* outGroup, DWORD* outFile); //549380,+48.
		virtual BYTE SeparateTextures() const; //549020,+4C, base0.
		virtual BYTE IsDead() const; //549030,+50.
		virtual BYTE CanTalk() const; //408400,+54.
		virtual DWORD GetIdleAnimationGroup() const; //4080B0,+58, base0.
		virtual BYTE IsIdle() const { return actions.empty(); } //4084C0,+5C.
		virtual void UpdateClient(DWORD time); //549090,+60.
		virtual BYTE IsPeriodic(); //549120,+64.
		virtual BYTE UpdateDue(DWORD time); //54AB50,+68.
		virtual BYTE NeedsUpdate(); //54AB00,+6C.
	protected:
		virtual void AddFootprint(RLEGrid<Entity*>* grid); //551CC0,+70.
		virtual void RemoveFootprint(RLEGrid<Entity*>* grid); //54DE50,+74.
	public:
		virtual void OnMoveFailed(DWORD time); //54BEB0,+78.
		virtual void OnMoveArrived(DWORD time); //54BEB0,+7C.
		virtual void SubmitEffects(Gfx_Viewport* viewport); //54B4E0,+80.
		virtual BYTE IsSelectionDisabled() const; //4080A0,+84; inverted original selectable+388.
		virtual BYTE IsUnderFogOfWar(); //54A2C0,+88.
		virtual float GetViewRadius() const; //549970,+8C.
		virtual DWORD GetLOS(Entity* target); //549B00,+90.
		virtual DWORD GetLOSPoint(float x, float y, float z); //549A80,+94, native RET12.
		virtual DWORD GetLOSCell(LONG first, LONG row, const float* targetHeight); //549980,+98.
		virtual void CollectEffectModels(std::list<Gfx_ModelInstance*>& output); //54FD90,+9C.
		virtual BYTE WriteToFile(Resources::ResourceFileHandle* stream); //54F6A0,+A0.
	protected:
		virtual void OnAnimationCue(const Gfx_AnimationCue& cue); //5A9320,+A4.
	public:
		virtual std::string GetDisplayName(); //5505B0,+A8; actual empty base result.
		virtual std::list<AreaMapHeight>* GetHeightInfluence(); //4080B0,+AC.
	protected:
		virtual Inventory* CreateInventory(); //549320,+B0.
		virtual RLEPrimitive<Entity*>* CreateSpatialFootprint(); //550780,+B4.
	public:
		virtual void OnActivate(DWORD time); //550970,+B8, original OnCreate.
		virtual void OnDeath(DWORD time); //5509B0,+BC.
		virtual void OnDestroy(DWORD time); //550A50,+C0.
		virtual void OnLoad(DWORD time); //550A90,+C4.
		virtual void Update(DWORD time); //54C570,+C8.
		virtual void OnAnimationChanged(DWORD group, DWORD file); //408090,+CC.
	protected:
		virtual void OnClientFrame(DWORD elapsed); //550AD0,+D0.
	public:
		virtual void OnAnimationUpdateReceived(DWORD group, DWORD file); //408090,+D4.
	protected:
		virtual void OnRefreshAssociations(); //404B70,+D8.
		virtual void OnCreated(); //404B70,+DC.
		virtual void OnModelLoaded(); //404B70,+E0.
	public:
		virtual BYTE ReadFromFile(Common::Chunk& chunk, Resources::ResourceMemoryStream* stream,
			BYTE skipDefaultOverride); //5528D0,+E4.
		virtual void Draw(); // Original virtual+04,0x54DEA0.
		virtual BYTE AddLight(Gfx_Light* light); //549CD0, delegates to ModelInstance.
		virtual void RemoveLight(Gfx_Light* light); //549CF0.
		virtual void ClearLights(); //549D10.
		virtual void FinishLights(); //549D20.
		virtual void ClientFrame(DWORD elapsed); // Original+08,0x549240; dispatches OnClientFrame.
		virtual BYTE IsVisible(); // Original+1C,0x54A110; game override0x466570.
		void WriteUpdate(Net::Packet& packet); //54A060, position/rotation/current animation wire state.
		void PackState(Net::Packet& packet); //54A610, full start snapshot, distinct from WriteUpdate.
		void UnpackState(DWORD id, Net::Packet& packet); //54E690, invalid-entity incoming snapshot.
		void ReadUpdate(Net::Packet& packet); //54ABB0, consumes all fields before local-server mode gate.
		static void SkipUpdate(Net::Packet& packet); //549250, nine separate DWORD reads.
		VisualEffect* AttachVisualEffect(const char* name, DWORD id, BYTE bindingEnabled = 1); //5512B0.
		VisualEffect* AttachVisualEffect(VFX_VEGInstance* instance, BYTE release,
			BYTE own, DWORD id, BYTE bindingEnabled); // Actual54EF90.
		BYTE RemoveVisualEffect(VisualEffect* effect, BYTE force); // Original54F150; Client58AB80 caller.
		BYTE RemoveVisualEffectByID(DWORD id,BYTE force); //54F270, remove every matching active binding.
		DWORD GetVisualEffectID(const char* name) const; //54B450, exact active-instance name, first pointer-ordered match.
		BYTE RemoveSound(DWORD id, BYTE force); //54F590, remove all active sound bindings with this ID.
		BYTE RemoveSound(VfxSoundInstance* sound, BYTE force); //54F4D0, active pointer-key removal.
		VfxSoundInstance* AttachSound(const char* filename, BYTE looping, DWORD id); //5512F0.
		VfxSoundInstance* AttachSound(VfxSoundInstance* sound, BYTE looping, BYTE own, DWORD id); //54F3B0.
		void ClearSounds(); //54C180, active owners then inactive owners, ordinary map nodes.
		void UpdateSounds(DWORD elapsed); //550D74..550F7F sound-map portion before VFX.
		void Create(const char* resource, BYTE loadModel, BYTE skipDefaultOverride); // 0x551880.
		void Destroy(); // 0x551B60, state transition only; not immediate deletion.
		void Die(); //551CA0, server-mode-only ChangeState(6).
		void ChangeState(DWORD value); // 0x5507F0.
		void EnterCombat(); //5490B0; actual server guard, live WorldServer combat dispatch.
		void LeaveCombat(); //551C80, state5-to4 transition only.
		void ServerUpdate(DWORD time); // 0x549130, state pump.
		void RefreshAssociations(); // 0x54AA20; populated association deque unresolved.
		void ClearMapMessages(); // 0x54C530, node-only clear of+320; distinct from+378 messages.
		void ClearMapAssociations(); // 0x54BE70, node-only clear of+32C.
		BYTE HasMapAssociation(Entity* entity) const; // 0x54AEB0, borrowed-pointer list membership.
		void AddMapAssociation(Entity* entity); // 0x54EE20, append borrowed pointer, duplicates retained.
		BYTE MatchesPathCell(const PathPoint& cell) const; // 0x54AE60, compares+320 list's two coordinate words.
		BYTE ReadResource(const char* name, BYTE skipDefaultOverride); // 0x54A460.
		BYTE ReadChunk(Resources::ResourceMemoryStream* stream, BYTE skipDefaultOverride); // 0x5493A0.
		BYTE ReadEENT(Common::Chunk& chunk, Resources::ResourceMemoryStream* stream); //551CF0, obsolete engine data.
		BYTE ReadEEN2(Common::Chunk& chunk, Resources::ResourceMemoryStream* stream,
			BYTE skipDefaultOverride); // 0x552760.
		BYTE ReadEEOV(Common::Chunk& chunk, Resources::ResourceMemoryStream* stream); // 0x552000.
		void SetShadow(BYTE value); // 0x548D60.
		void SetCount(DWORD value); // 0x549D60; zero becomes1.
		void SetDisplayName(const std::string& value); //54DE00, retain spelling on case-insensitive equality.
		void SetClientOwner(DWORD value); //551BA0, ownership transition/idle action/map count/client selection.
		void AttachEffectsToMap(DWORD map); // 0x54B220.
		void DetachEffectsFromMap(); // 0x54B2D0; unregistered spatial domain.
		void SetInventoryOwner(DWORD id); // 0x54ED50; creation-list/registered-map transition.
		ModelInstance* AcquireModel(BYTE loadAnimations); // 0x54CA40.
		void SetModel(const char* name, BYTE loadAnimations); // 0x54DC50.
		const char* GetTextureOverrideName() const; //409CB0, borrowed C string; original name inferred.
		void RefreshTexture(); // 0x54B050.
		void SetRotation(const D3DXVECTOR3& value); // 0x549FE0; map-attached branch open.
		void SetPosition(const D3DXVECTOR3& value); // 0x54A8A0; unregistered/map0 path connected.
		void SnapToFloor(); // 0x54ADC0, actual downward height query then SetPosition.
		virtual void GetSpatialCenter(D3DXVECTOR3& output) const; // 549F90, half-height offset.
		BYTE UpdateVisibility(BYTE force); //549450, per-cell visible/explored updates.
		void QueueCurrentVisibility(); //549710, visible cells only, ignores movement cache.
		void RotateBounds(); // 0x549D80.
		float GetModelHeight() const; //548FF0, original no-argument x87 member.
		void ReleaseModel(); // Shared observed release sequence, not a new original routine.
		void PlayAnimation(DWORD group, DWORD file, WORD loops, BYTE reverse, DWORD timeOffset); // 0x548D80.
		LONG QueueAnimationPercent(DWORD group, DWORD file, DWORD loops, DWORD reverse,
			float percent, DWORD clearPending); // 0x5505F0.
		void QueueAnimation(DWORD group, DWORD file, DWORD loops, BYTE reverse,
			DWORD timeOffset, BYTE clearPending); // Owning deque append54EB40, library supplied.
		void UpdateAnimationQueue(); // Factored54C891..54CA35; not complete Entity::Update.
		DWORD GetAnimationTime() const; // 0x549040.
		float GetAnimationDuration() const; // 0x548FC0.
		DWORD EndAnimationNow(); // 0x548F90, writes end time; does not clear queue.
		void ClearQueuedAnimations(); //0x54BD90, consumes pending records without changing current animation.
		void FreezeAnimation(); // 0x548EE0, captures World time once.
		void UnfreezeAnimation(); // 0x548F30, shifts both animation clocks by paused duration.
		Action* GetCurrentAction(); //4677E0; borrowed front pointer or NULL, no starting filter.
		void QueueAction(Action* action); // 0x550720.
		void AddEffect(Effect* effect); //5506C0, owns after initialization/activation.
		void CompleteEffect(DWORD id); //54AA90, mark first matching queued effect; do not erase/destroy.
		void UpdateEffects(); //54C597 prefix of Update, before messages.
		void RefreshEffects(); //54AA2D, actual association refresh loop.
		void ClearEffects(); //54C390, destroys queued effects and updates scheduling.
		void ClearActions(); // 0x54C440.
		void DispatchActions(DWORD time); // Explicit dispatch/completion factor of54C570.
		Script_Small* GetScript(); // 0x54AEF0.
		const char* GetScriptName() const; //436250, borrowed current string storage.
		void SetScriptName(const char* name); //426510; assigns only the existing CString.
		DWORD GetScriptValue(LONG key) const; //54BEC0, missing numeric key returns0.
		void SetScriptValue(LONG key, DWORD value); //54EEF0, original+394 map.
		BYTE RunScript(const char* name, cell* result, int count, cell* arguments); // 0x54AFE0.
		void EnqueueMessage(DWORD from, DWORD message, const cell* arguments, DWORD count); //54EE60.
		DWORD MessageArgCount() const; //5492D0, current message only.
		cell MessageGetArg(LONG index) const; //5492F0, original signed predicate.
		void DispatchMessage(); //54C643..54C6C4, at most one front message per Update.
		void DiscardMessage(); //54B3F0, exactly one front payload, not an all-message clear.
		// Original independent members follow DynamicObject's complete15C bytes.
		// Declaration order also owns the original automatic member lifetimes.
		DWORD entityClass; //+15C, unwritten in Entity/GameEntity; concrete subclasses establish it.
		ModelInstance* model; //+160.
		BYTE modelAttempted; //+164.
		Inventory* inventory; //+168, owning; acquired by Create, initiallyNULL.
		RLEPrimitive<Entity*>* spatialFootprint; //+16C, owning server-side spatial primitive.
		BYTE inFootprintGrid, associationsDirty; //+170/+171, initially0.
		DWORD actionBudget; //+174, initially0.
		BYTE enabled, shadow, flag17A; //+178/+179/+17A.
		DWORD value17C; //+17C.
		unsigned int animationStartTime, animationEndTime; // +180/+184, direct UInt32 snapshot outputs.
		BYTE animationLooping; //+188.
		std::string resourceName; //+18C.
		std::deque<EntityAnimationRequest> animationQueue; // Original+1A8.
		BYTE pendingAnimation; //+1BC.
		std::string displayName; //+1C0.
		DWORD entityID, state; //+1DC/+1E0; state uses the native reader scratch.
		unsigned int visibilityOwnerFlag; //+1E4, direct UInt32 snapshot output.
		BYTE inUpdateQueue; //+1E8.
		unsigned int mapHandle; //+1EC, direct UInt32 snapshot output.
		D3DXVECTOR3 rotation; //+1F0.
		std::string roleName, modelName, textureOverride, inventoryTexture, activeTexture; //+1FC/+218/+234/+250/+26C.
		BYTE animationEnabled; // +288, default1.
		unsigned int currentAnimationGroup, currentAnimationIndex; // +28C/+290, direct snapshot outputs.
		BYTE animationReverse, suppressIdleAnimation, flag296, frozenAnimation; //+294..297.
		unsigned int frozenAnimationTime, value29C; // +298/+29C, direct snapshot outputs.
		std::string scriptName; //+2A0.
		Script_Small* script; //+2BC.
		BYTE scriptDisabled; //+2C0.
		std::map<std::string,cell> variables; //+2C4, shared Small preload cells; real VC7 owner.
		DWORD updateInterval, lastUpdateTime, nextUpdateTime, inventoryOwnerID; //+2D0..2DC.
		std::deque<Effect*> effects; //+2E0, owning;2F0 is its count.
		std::list<EntityLight*> lights; //Original+2F4, attached gameplay-effect triggers.
		DWORD frameTime; //+300.
		std::deque<Action*> actions; //+304, owning.
		BYTE actionAlternating, actionBlocked; //+318/+319, initially0.
		DWORD value31C; //+31C, actual551605 zero-store; purpose remains unknown.
		std::list<EntityMapMessage> mapMessages; //+320, borrowed/POD payload policy.
		std::list<Entity*> mapAssociations; //+32C, borrowed pointers;54EE20/54AEB0.
		std::map<VFX_VEGInstance*,EntityEffectBinding> activeEffects; //+338.
		std::map<DWORD,VisualEffect*> effectIDs; // +344, borrowed object lookup.
		std::map<VFX_VEGInstance*,EntityEffectBinding> inactiveEffects; //+350.
		D3DXVECTOR3 lastWaterPosition; // Original+35C, ctor zero; client water-interaction sample.
		DWORD value368, value36C; //+368/+36C, independent of state(+1E0).
		DWORD squadID, squadRank; //+370/+374, ctor5516F2/5516F8 zero.
		std::list<EntityMessage*> messages; //+378, copied owning payloads; destructor drops one only.
		EntityMessage* currentMessage; //+384, NULL until callback publication.
		BYTE selectable; //+388.
		DWORD value908, value912; //+38C/+390.
		std::map<LONG,DWORD> scriptValues; //+394, distinct from named script preload values.
		BYTE renderEnabled, dialogueEnabled, moving; //+3A0/+3A1/+3A2.
		D3DXVECTOR3 modelMinimum, modelMaximum; //+3A4/+3B0.
		DWORD count956; //+3BC.
		std::map<VfxSoundInstance*,EntitySoundBinding> activeSounds, inactiveSounds; //+3C0/+3CC, real outer52E2E0 owners.
		void RemoveAttachment(EffectTreeTrigger* trigger); //54BDF0, remove borrowed list entries.
		virtual ~Entity();
	protected:
		BYTE IsFogVisible(); // Shared source factor of54A110/466570; not an original entry.
		Entity(); //5513A0, member initialization then creation-list publication.
		// Source factor only; Create uses the actual IWorld+24 service.
		DWORD AllocateEntityID(); // Actual IWorld+24 service required.
		std::string PlayerResourceName(); // Actual Server resource string consumed by551880.
		void PublishPlayerID(DWORD id); // Actual Client player member at551A60.
		DWORD CreateNetworkMode(); // Actual SecondElement(HudBase).
		void MoveLight(EntityLight* light, const D3DXVECTOR3& value); // Actual5AA4E0 child.
		void AddToUpdateQueue(); // 0x54EF30.
		void RemoveFromUpdateQueue(); // 0x54BF00.
		void ClearVisualEffects(); // 0x54BF30.
		void AddInventoryResource(const std::string& name); // Actual552000 version2 factory/Create/Add child.
		// Source factor of54B540; no additional Entity virtual slot.
		void ApplyAssociationSelection(BYTE value); // Factored54B540 active-effect loop.
	private:
		Entity(const Entity&);
		Entity& operator=(const Entity&);
	};
#if defined(_MSC_VER) && _MSC_VER == 1300 && defined(_M_IX86)
	// Original5451A2 allocates3D8; the complete5513A0 member graph fixes every offset.
	typedef char EntityOriginalBaseExtent[(sizeof(DynamicObject) == 0x15C) ? 1 : -1];
	typedef char EntityOriginalExtent[(sizeof(Entity) == 0x3D8) ? 1 : -1];
#define JE_ENTITY_OFFSET(member, offset) typedef char EntityOriginalOffset_##member[(offsetof(Entity, member) == (offset)) ? 1 : -1]
	JE_ENTITY_OFFSET(entityClass, 0x15C);
	JE_ENTITY_OFFSET(model, 0x160);
	JE_ENTITY_OFFSET(modelAttempted, 0x164);
	JE_ENTITY_OFFSET(inventory, 0x168);
	JE_ENTITY_OFFSET(spatialFootprint, 0x16C);
	JE_ENTITY_OFFSET(inFootprintGrid, 0x170);
	JE_ENTITY_OFFSET(associationsDirty, 0x171);
	JE_ENTITY_OFFSET(actionBudget, 0x174);
	JE_ENTITY_OFFSET(enabled, 0x178);
	JE_ENTITY_OFFSET(shadow, 0x179);
	JE_ENTITY_OFFSET(flag17A, 0x17A);
	JE_ENTITY_OFFSET(value17C, 0x17C);
	JE_ENTITY_OFFSET(animationStartTime, 0x180);
	JE_ENTITY_OFFSET(animationEndTime, 0x184);
	JE_ENTITY_OFFSET(animationLooping, 0x188);
	JE_ENTITY_OFFSET(resourceName, 0x18C);
	JE_ENTITY_OFFSET(animationQueue, 0x1A8);
	JE_ENTITY_OFFSET(pendingAnimation, 0x1BC);
	JE_ENTITY_OFFSET(displayName, 0x1C0);
	JE_ENTITY_OFFSET(entityID, 0x1DC);
	JE_ENTITY_OFFSET(state, 0x1E0);
	JE_ENTITY_OFFSET(visibilityOwnerFlag, 0x1E4);
	JE_ENTITY_OFFSET(inUpdateQueue, 0x1E8);
	JE_ENTITY_OFFSET(mapHandle, 0x1EC);
	JE_ENTITY_OFFSET(rotation, 0x1F0);
	JE_ENTITY_OFFSET(roleName, 0x1FC);
	JE_ENTITY_OFFSET(modelName, 0x218);
	JE_ENTITY_OFFSET(textureOverride, 0x234);
	JE_ENTITY_OFFSET(inventoryTexture, 0x250);
	JE_ENTITY_OFFSET(activeTexture, 0x26C);
	JE_ENTITY_OFFSET(animationEnabled, 0x288);
	JE_ENTITY_OFFSET(currentAnimationGroup, 0x28C);
	JE_ENTITY_OFFSET(currentAnimationIndex, 0x290);
	JE_ENTITY_OFFSET(animationReverse, 0x294);
	JE_ENTITY_OFFSET(suppressIdleAnimation, 0x295);
	JE_ENTITY_OFFSET(flag296, 0x296);
	JE_ENTITY_OFFSET(frozenAnimation, 0x297);
	JE_ENTITY_OFFSET(frozenAnimationTime, 0x298);
	JE_ENTITY_OFFSET(value29C, 0x29C);
	JE_ENTITY_OFFSET(scriptName, 0x2A0);
	JE_ENTITY_OFFSET(script, 0x2BC);
	JE_ENTITY_OFFSET(scriptDisabled, 0x2C0);
	JE_ENTITY_OFFSET(variables, 0x2C4);
	JE_ENTITY_OFFSET(updateInterval, 0x2D0);
	JE_ENTITY_OFFSET(lastUpdateTime, 0x2D4);
	JE_ENTITY_OFFSET(nextUpdateTime, 0x2D8);
	JE_ENTITY_OFFSET(inventoryOwnerID, 0x2DC);
	JE_ENTITY_OFFSET(effects, 0x2E0);
	JE_ENTITY_OFFSET(lights, 0x2F4);
	JE_ENTITY_OFFSET(frameTime, 0x300);
	JE_ENTITY_OFFSET(actions, 0x304);
	JE_ENTITY_OFFSET(actionAlternating, 0x318);
	JE_ENTITY_OFFSET(actionBlocked, 0x319);
	JE_ENTITY_OFFSET(value31C, 0x31C);
	JE_ENTITY_OFFSET(mapMessages, 0x320);
	JE_ENTITY_OFFSET(mapAssociations, 0x32C);
	JE_ENTITY_OFFSET(activeEffects, 0x338);
	JE_ENTITY_OFFSET(effectIDs, 0x344);
	JE_ENTITY_OFFSET(inactiveEffects, 0x350);
	JE_ENTITY_OFFSET(lastWaterPosition, 0x35C);
	JE_ENTITY_OFFSET(value368, 0x368);
	JE_ENTITY_OFFSET(value36C, 0x36C);
	JE_ENTITY_OFFSET(squadID, 0x370);
	JE_ENTITY_OFFSET(squadRank, 0x374);
	JE_ENTITY_OFFSET(messages, 0x378);
	JE_ENTITY_OFFSET(currentMessage, 0x384);
	JE_ENTITY_OFFSET(selectable, 0x388);
	JE_ENTITY_OFFSET(value908, 0x38C);
	JE_ENTITY_OFFSET(value912, 0x390);
	JE_ENTITY_OFFSET(scriptValues, 0x394);
	JE_ENTITY_OFFSET(renderEnabled, 0x3A0);
	JE_ENTITY_OFFSET(dialogueEnabled, 0x3A1);
	JE_ENTITY_OFFSET(moving, 0x3A2);
	JE_ENTITY_OFFSET(modelMinimum, 0x3A4);
	JE_ENTITY_OFFSET(modelMaximum, 0x3B0);
	JE_ENTITY_OFFSET(count956, 0x3BC);
	JE_ENTITY_OFFSET(activeSounds, 0x3C0);
	JE_ENTITY_OFFSET(inactiveSounds, 0x3CC);
#undef JE_ENTITY_OFFSET
#endif
}
