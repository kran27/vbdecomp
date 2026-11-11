#pragma once

#include "GameItem.hpp"

namespace F3
{
	class GameAmmo : public GameItem
	{
		friend JE::Entity* CreateGameEntity(DWORD type);
		friend class GameWorld;
		friend struct GameClient; //461A10 packet92 directly constructs an unactivated ammo clone.
		friend class GameWeapon; //Original unload directly constructs an unactivated data clone.
	public:
		virtual void CopyFrom(JE::Entity* source); //408200/4082C0, class+9 ammo words before GameItem.
		virtual BYTE ReadFromFile(JE::Common::Chunk& chunk, JE::Resources::ResourceMemoryStream* stream,
			BYTE skipDefaultOverride); // 0x408510/408570.
		BYTE ReadAmmo(JE::Common::Chunk& chunk, JE::Resources::ResourceMemoryStream* stream,
			BYTE skipDefaultOverride); // 0x408420; third argument is unused.
		DWORD caliber, modifier, value952, value956, value960, value964, value968, value972, value976;
	private:
		BYTE ReadBaseAmmo(JE::Resources::ResourceMemoryStream* stream); //408160, five fields.
	protected:
		GameAmmo(); // 0x407FB0 data defaults; value968 remains unwritten before version2.
	};
}
