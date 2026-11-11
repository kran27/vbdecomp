#pragma once

#include "GameEntity.hpp"

namespace F3
{
	class GameUsableObject : public GameEntity
	{
		friend JE::Entity* CreateGameEntity(DWORD type);
	public:
		virtual void CopyFrom(JE::Entity* source); //4125A0/412630, existing Entity slot28.
		virtual DWORD GetContextAction(JE::Entity* actor); //412640->412530,3.
		virtual void GetContextActions(JE::Entity* actor, std::list<JE::ContextMenuEntry>& entries); //412A80.
		virtual BYTE ReadFromFile(JE::Common::Chunk& chunk, JE::Resources::ResourceMemoryStream* stream,
			BYTE skipDefaultOverride); // 0x412760/4127C0, recovered from instructions.
		BYTE ReadUsable(JE::Common::Chunk& chunk, JE::Resources::ResourceMemoryStream* stream,
			BYTE skipDefaultOverride); //412540; chunk and third argument are unused.
		DWORD value160, values164[6];
	protected:
		GameUsableObject(); // 0x412420 data defaults; full runtime construction remains separate.
		virtual JE::RLEPrimitive<JE::Entity*>* CreateSpatialFootprint(); //4128E0, actual model-bound quad.
	};
}
