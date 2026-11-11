#pragma once

#include "GameEntity.hpp"
#include "AttackMode.hpp"
#include <list>

namespace F3
{
	struct ModelSocket { std::string model, texture; }; // Actual40A220 normal string pair.

	class GameCreature : public GameEntity
	{
		friend JE::Entity* CreateGameEntity(DWORD type);
		friend class CharacterGenerationInterface;
	public:
		virtual void CopyFrom(JE::Entity* source); //40ACC0, appearance then qualified GameEntity copy.
		virtual void OnActivate(DWORD time); //4093C0, server publication then GameEntity activation.
		virtual void UpdateClient(DWORD time); //409BE0, adjustor40D430; realtime action-point regeneration.
		LONG GetDerivedAttribute(LONG attribute); //409A60, actual current-stat policy.
		BYTE HasTrait(DWORD trait) const; //40A0E0, actual AL-only membership result.
		BYTE HasTagSkill(DWORD skill) const; //409CD0, actual AL-only membership result.
		LONG TagSkillCount() const; //409BD0, raw count; UI compares it as signed.
		LONG TraitCount() const; //409CA0, raw count; UI compares it as signed.
		void AddTagSkill(DWORD skill); //40C890.
		void RemoveTagSkill(DWORD skill); //40A270, removes every occurrence.
		void AddTrait(DWORD trait); //40C8F0.
		void RemoveTrait(DWORD trait); //40A390, removes every occurrence.
		LONG GetBaseSkill(LONG skill); //409820, trusted index into existing skills.
		LONG GetTotalSkill(LONG skill); //409830, base plus actual current-primary formulas.
		LONG CalculateUnarmedChance(AttackModeIterator mode, JE::Entity* target, DWORD calledPart,
			GameWeapon* weapon); //409E30, actual skill/range/armor calculation.
		LONG CalculateUnarmedDamage(AttackModeIterator mode, JE::Entity* target, DWORD ranged,
			DWORD calledPart, GameWeapon* weapon); //409F90.
		DWORD ChooseBodyPart(DWORD roll) const; //409960, unsigned cumulative probabilities.
		virtual BYTE ReadFromFile(JE::Common::Chunk& chunk, JE::Resources::ResourceMemoryStream* stream,
			BYTE skipDefaultOverride); // 0x40E530/40E590.
		virtual BYTE WriteToFile(JE::Resources::ResourceFileHandle* stream); //40BB80, GCRE5/GWAM5.
		BYTE ReadCreature(JE::Common::Chunk& chunk, JE::Resources::ResourceMemoryStream* stream,
			BYTE skipDefaultOverride); //40E360, RET12; third argument is unused.
		BYTE ReadAttackModeData(JE::Common::Chunk& chunk, JE::Resources::ResourceMemoryStream* stream); //40DBA0.
		BYTE ReadVersion0(JE::Resources::ResourceMemoryStream* stream); // 0x40B6A0.
		BYTE ReadVersion1(JE::Resources::ResourceMemoryStream* stream); // 0x409410.
		BYTE ReadVersion2(JE::Resources::ResourceMemoryStream* stream); // 0x40D9E0.
		BYTE ReadVersion3(JE::Resources::ResourceMemoryStream* stream); // 0x409700.
		BYTE ReadVersion4(JE::Resources::ResourceMemoryStream* stream); // 0x40B9E0.
		void RebuildAttackModes(); // 0x40E180.
		void SetPerkRank(LONG perk, LONG rank); //40A370; original signed upper bound only.
		LONG IncrementPerk(LONG perk); //40A2F0, increment then reliable rank notification and weapon refresh.
		void RebuildInventoryAttackModes(); //40A120, borrowed inventory items; GameWeapon RTTI only.
		AttackMode* CurrentAttackMode(); //Typed normal current payload getter.
		AttackModeIterator PeekNextAttackMode(); //409D10, hidden iterator output.
		void CycleAttackMode(); //409DA0.
		BYTE ResolveWeaponAndAttackMode(GameWeapon** weapon, AttackModeIterator* mode, BYTE* unarmed); //40B380.
		virtual DWORD GetContextAction(JE::Entity* actor); //4092A0/40D320.
		virtual void GetContextActions(JE::Entity* actor, std::list<JE::ContextMenuEntry>& entries); //40E340.
		void RecalculateArmor(); // 0x40A670; base profile plus equipped armor values.
		// Factored appearance suffix after the recovered inventory reapply call.
		void RestoreSavedAppearance();
		std::string GetSocketModel(DWORD slot) const; //4697D0, actual value-returning member.
		std::string GetSocketTexture(DWORD slot) const; //469810, actual value-returning member.
		// Original own payload begins+A0; GameEntity's unresolved24B interval
		// still moves every actual compiler offset24B earlier. No padding added.
		LONG special[7], baseValues[7]; //Original+A0/+BC.
		DWORD valueD8; //Original+D8, version5 subrace; skipped by version0.
		LONG valueDC; //Original+DC, eighth additional version0 word.
		LONG skills[13], traits[82]; //Original+E0/+114.
		std::list<DWORD> selectedTraits, tagSkills; //Original+25C/+268, serialized in this order.
		DWORD value274;
		std::string portrait; //Original+278.
		DWORD value294;
		std::string value298;
		DWORD value2B4;
		BYTE materialMaskMode;
		std::string materialNames[12]; //Original+2BC.
		ModelSocket sockets[13]; //Original+40C.
		BYTE socketFlags[12]; //Original+6E4; constructor leaves these unwritten.
		DWORD attackModeCount; //Original+6F0.
		AttackModeIterator currentAttackMode; //Original+6F4, nullable list node.
		std::list<AttackMode> attackModes, activeAttackModes;
		LONG uncalledChances[8], resistances[6], baseResistances[6];
		LONG baseSpecial[7]; //Original+760, refreshed by4097A0 and version0.
		DWORD runtimeValues77C[13]; //4097A0 zeros13 words; their further meaning remains unproved.
		LONG thresholds[6], baseThresholds[6]; //Original+7B0/+7C8.
		// Original+7E0..7EB has no proven member type/producer. It remains
		// unrepresented, so the following tail is another12B early.
		DWORD value7EC; //Original constructor explicitly writes0.
		std::list<JE::Entity*> pendingInventory;
		DWORD value7FC; //Original+7FC, initialized0; derived attribute4.
	protected:
		GameCreature(); //40D010; known member lifetimes/order, incomplete absolute layout.
		virtual ~GameCreature();
		virtual JE::Entity* CreateInventoryItem(const std::string& resource); // Actual game factory/Create required.
		virtual void OnRefreshAssociations(); //4097A0, adjustor40D420, actual Entity+D8.
		virtual void OnCreated(); // 0x40A430/40D450, equip pending items before unlink.
		virtual void OnModelLoaded(); // 0x40AE60/40D470, inventory reapply then saved appearance.
		virtual JE::RLEPrimitive<JE::Entity*>* CreateSpatialFootprint(); // 40E350 adjustor ->40D840.
	};
}
