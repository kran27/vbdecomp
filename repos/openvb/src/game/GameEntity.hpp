#pragma once

#include "entity/Entity.hpp"

namespace JE { class Server; }

namespace F3
{
	JE::Entity* CreateGameEntity(DWORD type);
	class GameWeapon;
	// Original virtual Entity base. Actual game factories construct this class;
	// Broader map/gameplay lifetime remains separately reconstructed.
	class GameEntity : public virtual JE::Entity
	{
		friend JE::Entity* CreateGameEntity(DWORD type);
	public:
		virtual void CopyFrom(JE::Entity* source); //468350; health/template effects, then actual Entity copy.
		BYTE ShowSkills(); //467240, actual nonvirtual script-result query.
		void GetUsableSkills(JE::Entity* actor, std::list<JE::ContextMenuEntry>& entries); //4687C0,actual nonvirtual member.
		virtual std::string GetDisplayName(); //467CD0, token-aware stringRefs[0].
		virtual LONG GetMaximumHealth(); //4080C0, actual GameEntity primary slot0.
		void SetHealth(LONG health); //4629C0, signed upper clamp only, no death dispatch.
		virtual BYTE IsVisible(); // 0x466570, adjustor467A80.
		virtual void SubmitEffects(Gfx_Viewport* viewport); //4671A0, qualified base then equipped items.
		virtual void CollectEffectModels(std::list<Gfx_ModelInstance*>& output); // 0x468640, adjustor468720.
		virtual BYTE ReadFromFile(JE::Common::Chunk& chunk, JE::Resources::ResourceMemoryStream* stream,
			BYTE skipDefaultOverride); // 0x468A20.
		virtual BYTE WriteToFile(JE::Resources::ResourceFileHandle* stream); //463970, GENT3.
		BYTE ReadGameEntity(JE::Common::Chunk& chunk, JE::Resources::ResourceMemoryStream* stream,
			BYTE skipDefaultOverride); //466510; third argument is passed but unused.
		BYTE ReadEffectTree(JE::Common::Chunk& chunk, JE::Resources::ResourceMemoryStream* stream); // 0x468930.
		virtual BYTE SeparateTextures() const; // 0x466920.
		virtual void SetMap(DWORD map); // 0x467140 and derived thunks; forwards to inventory children.
		virtual void SetSelectable(BYTE value); //467BE0; base then actual GameInventory children.
		DWORD GetActiveWeaponSlot() const; // 0x463B60.
		void SetActiveWeaponSlot(DWORD slot); //463B70, ordinary thiscall/RET4.
		void SetWeaponStance(GameWeapon* weapon); // 0x463B80; NULL selects active equipment.
		virtual void ResolveAnimation(DWORD group, DWORD file, DWORD* outputGroup,
			DWORD* outputFile); // 0x467830, Creature adjustor40D350.
		virtual DWORD GetIdleAnimationGroup() const; // 0x4629E0, Creature adjustor40D380.
		virtual BYTE IsDead() const; // 0x462970, Creature adjustor40D370.
		LONG QueueIdle(float percent); // 0x462C50, actual5505F0 producer.
		LONG QueueIdleVariant2(float percent); //463000; event2, group254 or QueueIdle.
		LONG QueueIdleVariant3(float percent); //463040; event3, group255 or QueueIdle.
		LONG QueueIdleVariant4(float percent); //463080; event4, group256 or QueueIdle.
		LONG QueueIdleVariant5(float percent); //4630C0; event5, group257 or QueueIdle.
		LONG QueueIdleVariant6(float percent); //463100; event6, group258 or QueueIdle.
		LONG QueueWalk(); //0x465FC0, transition followed by repeating stance walk.
		LONG QueueLimp(); //0x4669F0, transition followed by repeating stance limp.
		LONG QueueToCombat(); //0x463140, root-state transition to2.
		virtual void OnActivate(DWORD time); // 0x467640, qualified Entity::OnActivate first.
		std::list<JE::Effect*>* GetEffects(DWORD event); //463960, unchecked template-list address.
		void ApplyOnCreateEffects(); // 0x466FE0; clone/target templates from effects[3].
		void InitializeActivationAnimation(); // Explicit suffix467672..467773.
		LONG QueueAnimationEvent(DWORD event, float percent); //0x4673D0, connected idle/default/combat/walk/limp.
		LONG QueueAttackAnimation(DWORD event); //463C80, actual root-state/weapon-stance attack groups.
		LONG QueueDamageReaction(DWORD event); //464D70, actual root-state/stance reaction groups.
		LONG QueueDeathAnimation(DWORD event); //464940; queues one final animation, then root10.
		LONG QueueProneBack(BYTE knock); //464B80;0 laydown, nonzero knock, root7.
		LONG QueueProneFront(BYTE knock); //464C70;0 laydown, nonzero knock, root8.
		LONG QueueToDefault(); //0x465C20, root-state transition to5.
		LONG QueueToCrouch(); //465EE0; event11, transition/unequip then group424.
		LONG QueueOperate(); //4667A0, animation event121.
		void UseSkill(DWORD skill, JE::Entity* actor); //467340, actual target-script UseSkill callback.
		LONG QueueReload(); //4659A0, actual animation event120.
		LONG QueueEquip(); //465AE0, actual animation event21.
		LONG QueueUnequip(); //4636B0, actual animation event22.
		LONG QueuePickup(); //466880, event141; root2/group302 or root5/group435.
		// Animation-only extraction of467672..467689 plus event1 gate4673D0.
		// Excludes activation's scripts/effects, random clock phase and later action.
		void InitializeIdleAnimationState();
		DWORD stringRefs[6];
		LONG maximumHealth, currentHealth;
		DWORD creationServerTime, value2C, weaponStance, value34; //Original+28/+2C/+30/+34.
		// value2C is the original animation state consumed by4629E0/462C50.
		std::list<JE::Effect*> effects[5]; //Original+38..+73, owned templates destroyed through55A570.
		// Original+74..+8B is uninitialized storage whose member types remain unknown.
		// Do not fill it with guessed objects/padding: this partial layout remains18h short.
		DWORD value8C, value90;
		float value94; //Original+94 movement speed;405720 truncates toLONG, positive or fallback10.
		DWORD activeWeaponSlot; //Original+98, default2; tail offsets remain18h short.
		BYTE flag9C;
	protected:
		virtual void OnClientFrame(DWORD elapsed); // 0x466F40, adjustor467AC0.
		virtual void OnAnimationCue(const Gfx_AnimationCue& cue); // 0x467E90, adjustor468300.
		GameEntity(); // 4678A0; interval5 and guarded World creation-time capture.
		virtual ~GameEntity(); // 0x467AF0, fresh empty-effect domain.
		virtual JE::Inventory* CreateInventory(); // 0x4637F0/463800 and game thunks.
		BYTE ReadEffectRecord(JE::Common::Chunk& chunk, JE::Resources::ResourceMemoryStream* stream,
			JE::Effect* parent, DWORD event); // 0x468490, recursive EFDT/EFCD owner.
	private:
		BYTE ReadVersion0(JE::Resources::ResourceMemoryStream* stream); //463870, six independent string refs.
		BYTE ReadVersion1(JE::Resources::ResourceMemoryStream* stream); //463910, signed upper health clamp.
		BYTE ReadVersion2(JE::Resources::ResourceMemoryStream* stream); //409BB0, original no-op.
		BYTE ReadVersion3(JE::Resources::ResourceMemoryStream* stream); //409BB0, original shared no-op.
		BYTE ReadEffectEvent(JE::Common::Chunk& chunk,
			JE::Resources::ResourceMemoryStream* stream); //468730, event key/count and effect-record loop.
	};
#if defined(_MSC_VER) && _MSC_VER == 1300 && defined(_M_IX86)
	// Proven absolute payload members; the unresolved24B prevents a full extent claim.
#define F3_GAMEENTITY_OFFSET(member, offset) typedef char GameEntityOriginalOffset_##member[(offsetof(GameEntity, member) == (offset)) ? 1 : -1]
	F3_GAMEENTITY_OFFSET(stringRefs, 0x08);
	F3_GAMEENTITY_OFFSET(maximumHealth, 0x20);
	F3_GAMEENTITY_OFFSET(currentHealth, 0x24);
	F3_GAMEENTITY_OFFSET(creationServerTime, 0x28);
	F3_GAMEENTITY_OFFSET(value2C, 0x2C);
	F3_GAMEENTITY_OFFSET(weaponStance, 0x30);
	F3_GAMEENTITY_OFFSET(value34, 0x34);
	F3_GAMEENTITY_OFFSET(effects, 0x38);
#undef F3_GAMEENTITY_OFFSET
	typedef char GameEntityEffectsOriginalExtent[(sizeof(((GameEntity*)0)->effects) == 0x3C) ? 1 : -1];
	typedef char GameEntityValue90Relative[(offsetof(GameEntity, value90) - offsetof(GameEntity, value8C) == 4) ? 1 : -1];
	typedef char GameEntityValue94Relative[(offsetof(GameEntity, value94) - offsetof(GameEntity, value8C) == 8) ? 1 : -1];
	typedef char GameEntityActiveWeaponSlotRelative[(offsetof(GameEntity, activeWeaponSlot) - offsetof(GameEntity, value8C) == 12) ? 1 : -1];
	typedef char GameEntityFlag9CRelative[(offsetof(GameEntity, flag9C) - offsetof(GameEntity, value8C) == 16) ? 1 : -1];
#endif
}
