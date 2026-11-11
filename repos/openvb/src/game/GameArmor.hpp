#pragma once

#include "GameItem.hpp"

namespace F3
{
	class GameArmor : public GameItem
	{
		friend JE::Entity* CreateGameEntity(DWORD type);
	public:
		virtual void CopyFrom(JE::Entity* source); //4086E0/408770, existing Entity slot28.
		virtual BYTE ReadFromFile(JE::Common::Chunk& chunk, JE::Resources::ResourceMemoryStream* stream,
			BYTE skipDefaultOverride); // 0x408910/408970.
		BYTE ReadArmor(JE::Resources::ResourceMemoryStream* stream); // 0x408680 via4088B0.
		LONG values944[6], value968;
	protected:
		GameArmor(); // 0x408580 data defaults; complete game lifetime remains open.
	private:
		BYTE ReadArmorData(JE::Common::Chunk& chunk, JE::Resources::ResourceMemoryStream* stream,
			BYTE skipDefaultOverride); //4088B0; chunk and third argument are unused.
	};
}
