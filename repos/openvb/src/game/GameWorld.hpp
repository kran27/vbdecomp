#pragma once

#include "entity/World.hpp"

namespace F3
{
	class GameAmmo;
	class GameWorld : public JE::World
	{
	public:
		GameWorld(); // 0x470340; calls real ammunition initializer470220.
		virtual void ClearAuxiliaryObjects(); //46FB10, qualified World forwarding override.
		virtual void Reset(); //46FFA0 ->46FCC0: actual World reset, then GameClient PipBoy/Actionbar suffix.
		virtual ~GameWorld(); // 0x46F190; Client cleanup then qualified World lifetime.
		virtual JE::Entity* NewEntity(DWORD type); // 0x46F530, factored constructor cases; original adjustor46FDC0.
		virtual JE::Action* NewAction(DWORD type); // 0x46F290.
		virtual JE::Effect* NewEffect(DWORD type); //46FA40, actual GameEffectDamage type1.
		using JE::World::ClassifyEntity;
		virtual DWORD ClassifyEntity(const std::string& name, BYTE alternate); //4702F0, existing IWorld+34 override.
		virtual DWORD ClassifyEntity(const char* name, BYTE alternate); // 0x46FB20, char-pointer implementation.
		virtual JE::Map* NewMap(); // 0x46F7C0/46FDD0, actual GameMap.
		virtual JE::PathManager* NewPathManager(); // 0x46F9E0.
		virtual JE::Party* NewParty(); // 0x46F820.
		virtual JE::Journal* NewJournal(); // 0x46F880.
		virtual JE::Encyclopedia* NewEncyclopedia(); // 0x46F8E0.
		virtual JE::StoreManager* NewStoreManager(); // 0x46F930.
		virtual JE::WorldMap* NewWorldMap(); // 0x46F980.
		GameAmmo* FindAmmunition(LONG caliber, LONG modifier); //46FAC0, signed32-bit high/low bounds.
		BYTE InitializeAmmunition(); // 0x470220.
		DWORD ammoIDs[140]; // Original+164, 560 bytes; original lookup stride7.
	};
#if defined(_MSC_VER) && _MSC_VER == 1300 && defined(_M_IX86)
	typedef char GameWorldOriginalExtent[(sizeof(GameWorld) == 0x39C) ? 1 : -1];
	typedef char GameWorldOriginalAmmo[(offsetof(GameWorld, ammoIDs) == 0x164) ? 1 : -1];
#endif
}
