#pragma once

#include "GameItem.hpp"
#include "AttackMode.hpp"

namespace F3
{
	class GameAmmo;
	GameAmmo* FindAmmunition(DWORD caliber, DWORD modifier); //408110, actual cdecl world lookup.
	struct AttackSubAttack;
	class GameWeapon : public GameItem
	{
		friend JE::Entity* CreateGameEntity(DWORD type);
	public:
		virtual void CopyFrom(JE::Entity* source); //415670/416A60, existing Entity slot28.
		virtual BYTE ReadFromFile(JE::Common::Chunk& chunk, JE::Resources::ResourceMemoryStream* stream,
			BYTE skipDefaultOverride); // 0x415530/415660.
		BYTE ReadWeapon(JE::Common::Chunk& chunk, JE::Resources::ResourceMemoryStream* stream,
			BYTE skipDefaultOverride); // 0x415190, RET12; third argument is unused.
		BYTE ReadAttackModeData(JE::Common::Chunk& chunk, JE::Resources::ResourceMemoryStream* stream); //4148A0.
		BYTE ReadVersion0(JE::Resources::ResourceMemoryStream* stream); // 0x412CD0.
		void RebuildAttackModes(JE::Entity* owner); // 0x414EF0; active copies and creature trait71.
		AttackModeIterator PeekNextAttackMode(); //412E60, hidden iterator output.
		AttackModeIterator PeekAttackMode(LONG step); //412ED0, hidden output then signed step.
		AttackMode* CurrentAttackMode(); //Nonvirtual nullable-node to payload API adapter.
		void CycleAttackMode(); //4134A0.
		void SelectAttackMode(LONG step); //413500.
		BYTE ResolveTargets(JE::Entity* attacker, JE::Entity* requested, AttackModeIterator attack,
			const AttackSubAttack& sub, BYTE pointTarget, const D3DXVECTOR3& point,
			std::list<JE::Entity*>& output); //415740, seven actual arguments (RET1Ch).
		LONG CalculateToHit(AttackModeIterator attack, JE::Entity* attacker, JE::Entity* target,
			DWORD calledPart); //413560, dispatch uses this weapon's selected mode kind.
		LONG CalculateMeleeToHit(AttackModeIterator attack, JE::Entity* attacker, JE::Entity* target,
			DWORD calledPart); //412FE0.
		LONG CalculateRangedToHit(AttackModeIterator attack, JE::Entity* attacker, JE::Entity* target,
			DWORD calledPart); //413150.
		LONG CalculateDamage(AttackModeIterator attack, JE::Entity* attacker, JE::Entity* target,
			DWORD attackFlags, DWORD calledPart); //413310; attackFlags is not consumed.
		LONG GetEffectiveRange(AttackModeIterator attack); //412F50, preserves the NULL-argument quirk.
		void SetPrimaryAmmo(DWORD modifier, DWORD count); //412C10, ordinary receiver/RET8.
		void SetSecondaryAmmo(DWORD modifier, DWORD count); //412C70.
		void DeductPrimaryAmmo(DWORD amount); //412B10, unsigned subtraction then reliable publication.
		void DeductSecondaryAmmo(DWORD amount); //412B90.
		BYTE ReloadPrimaryAmmo(GameAmmo* stack, JE::Entity* owner); //413A90.
		BYTE ReloadSecondaryAmmo(GameAmmo* stack, JE::Entity* owner); //413CA0; original secondary quirks retained.
		BYTE UnloadPrimaryAmmo(JE::Entity* owner); //4135C0.
		BYTE UnloadSecondaryAmmo(JE::Entity* owner); //413810.
		DWORD weaponClass, primaryCaliber, primaryModifier, primaryReloadDuration;
		DWORD secondaryCaliber, secondaryModifier, secondaryReloadDuration;
		std::string value972;
		DWORD value1000, primaryAmmoLoaded, primaryAmmoCapacity;
		DWORD secondaryAmmoLoaded, secondaryAmmoCapacity, value1020, value1024, attackModeCount;
		AttackModeIterator currentAttackMode; //Original+408 nullable node; NULL differs from end().
		std::list<AttackMode> attackModes, activeAttackModes;
		GameAmmo* primaryAmmo; // Borrowed actual GameWorld ammunition entity.
		GameAmmo* secondaryAmmo;
	protected:
		GameWeapon(); // Recovered data defaults414510; complete Entity/Create lifetime stays open.
	};
}
