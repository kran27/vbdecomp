#pragma once

#include "entity/Chunk.hpp"
#include <list>

namespace F3
{
	struct AttackMode
	{
		AttackMode() { flag66=0; calledShot=0; kind=3; } //40DBA0 inline local defaults; no zero fill.
		enum Owner { Weapon };
		explicit AttackMode(Owner) { flag66=0; calledShot=0; } //4148A0 inline local defaults; kind remains unwritten.
		//40A980 uses implicit memberwise copy construction, including the real string.
		AttackMode& operator=(const AttackMode& source);
		BYTE Read(JE::Common::Chunk& chunk, JE::Resources::ResourceMemoryStream* stream); // Source-only factored helper; native40DBA0 belongs to GameCreature.
		BYTE ReadWeapon(JE::Common::Chunk& chunk, JE::Resources::ResourceMemoryStream* stream,
			DWORD weaponClass); // Source-only factored helper; native4148A0 belongs to GameWeapon.
		void MakeFailsafe(); // Local record built by40D9E0.
		DWORD kind, animation, damageType, value0C;
		BYTE unwritten10[4];
		DWORD shots, values18[3], range, values28[2], minimumDamage, maximumDamage;
		BYTE flag38, calledShot, consumed, thrown;
		DWORD actionPoints, pose, nameRef;
		std::string effectName;
		BYTE flag64, flag65, flag66;
	private:
		BYTE ReadData(JE::Common::Chunk& chunk, JE::Resources::ResourceMemoryStream* stream,
			BYTE weapon, DWORD weaponClass); // Factored identical proprietary read sequence.
	};
	typedef std::list<AttackMode>::iterator AttackModeIterator;
}
