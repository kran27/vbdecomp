#pragma once

#include "GameUsableObject.hpp"

namespace F3
{
	class GameContainer : public GameUsableObject
	{
		friend JE::Entity* CreateGameEntity(DWORD type);
	public:
		virtual void CopyFrom(JE::Entity* source); //409050/409150, existing Entity slot28.
		virtual BYTE IsVisible(); //408D20 ->shared408D30, always1.
		virtual DWORD GetContextAction(JE::Entity* actor); //Shared408980,9.
		virtual void GetContextActions(JE::Entity* actor, std::list<JE::ContextMenuEntry>& entries); //409260.
		virtual BYTE ReadFromFile(JE::Common::Chunk& chunk, JE::Resources::ResourceMemoryStream* stream,
			BYTE skipDefaultOverride); //408E60/408BC0.
		BYTE ReadContainer(JE::Common::Chunk& chunk, JE::Resources::ResourceMemoryStream* stream,
			BYTE skipDefaultOverride); //408B80; third argument is passed but unused.
		void Operate(JE::Entity* actor); //408990, script-gated open/close animation.
		BYTE open, locked; //Original+BC/+BD, normalized by GCON reader.
		DWORD valueC0; //Original constructor0; version1 reads/randomizes.
		std::string valueC4, keyResource; //Original+C4/+E0, normal owning strings.
	protected:
		GameContainer(); //408C20; actual GameUsableObject + virtual Entity lifetime.
	private:
		BYTE ReadVersion0(JE::Resources::ResourceMemoryStream* stream); //408A70.
		BYTE ReadVersion1(JE::Resources::ResourceMemoryStream* stream); //408AF0.
	};
}
