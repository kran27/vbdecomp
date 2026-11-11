#pragma once

#include "GameCreature.hpp"
#include <map>

namespace JE { class AreaMap; }

namespace F3
{
	// Allocated at GamePlayer+800. The actual cache owns AreaMap payloads and
	// uses ordinary, case-sensitive std::string ordering.
	class PlayerAreaMaps
	{
	public:
		PlayerAreaMaps(); //59F500, normal map construction.
		~PlayerAreaMaps(); //470C90, payload clear then normal map destruction.
		JE::AreaMap* Find(const std::string& name); //4703F0.
		void Set(const std::string& name, JE::AreaMap* value); //470D90.
		void Clear(); //4708C0.
		std::map<std::string, JE::AreaMap*> maps;
	private:
		PlayerAreaMaps(const PlayerAreaMaps&);
		PlayerAreaMaps& operator=(const PlayerAreaMaps&);
	};

	// Factory46F530 type10 and RTTI6F2598 identify GamePlayer. Ordinary NPC
	// resources use GameCreature, not a separate GameNPC constructor411820.
	class GamePlayer : public GameCreature
	{
		friend JE::Entity* CreateGameEntity(DWORD type);
	public:
		virtual void CopyFrom(JE::Entity* source); //411C50, name then qualified GameCreature copy.
		virtual std::string GetDisplayName(); //411BF0/shared411BA0, characterName or base token lookup.
		virtual BYTE ReadFromFile(JE::Common::Chunk& chunk, JE::Resources::ResourceMemoryStream* stream,
			BYTE skipDefaultOverride); //411A90/4117C0, GCHR then inherited readers.
		virtual BYTE WriteToFile(JE::Resources::ResourceFileHandle* stream); //411690/4119E0, GCHR0.
		BYTE ReadCharacter(JE::Resources::ResourceMemoryStream* stream); //411670.
		BYTE ReadCharacterData(JE::Common::Chunk& chunk, JE::Resources::ResourceMemoryStream* stream,
			BYTE skipDefaultOverride); //4117A0; only the stream argument is used.
		PlayerAreaMaps* areaMaps; //Original+800, owning allocated cache.
		std::string characterName; //Original+804, empty until GCHR or copy.
	protected:
		GamePlayer(); //411820, GameCreature then string/cache owners.
		virtual ~GamePlayer(); //411AA0.
	private:
		GamePlayer(const GamePlayer&);
		GamePlayer& operator=(const GamePlayer&);
	};
}
