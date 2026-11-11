#pragma once

#include "GameUsableObject.hpp"
#include "entity/MapFog.hpp"

namespace F3
{
	// Explicit workspace override for door feedback. The native default uses
	// actual Client+A0 / GameInterfaces.feedback and captured434B70 receivers.
	class GameDoorFeedbackServices
	{
	public:
		virtual ~GameDoorFeedbackServices() {}
		virtual BYTE HasFeedback() = 0;
		virtual void Display(std::string text, DWORD red, DWORD green, DWORD blue,
			std::string speaker) = 0;
	};
	GameDoorFeedbackServices* SetGameDoorFeedbackServices(GameDoorFeedbackServices* services);

	class GameDoor : public GameUsableObject
	{
		friend JE::Entity* CreateGameEntity(DWORD type);
	public:
		virtual void CopyFrom(JE::Entity* source); //40F000/40F2F0, existing Entity slot28.
		virtual BYTE IsVisible(); //40E8F0 ->408D30, always1.
		virtual DWORD GetContextAction(JE::Entity* actor); //40E900->shared408980,9.
		virtual void GetContextActions(JE::Entity* actor, std::list<JE::ContextMenuEntry>& entries); //40FFD0.
		virtual BYTE ReadFromFile(JE::Common::Chunk& chunk, JE::Resources::ResourceMemoryStream* stream,
			BYTE skipDefaultOverride); //40E720/40EA40.
		BYTE ReadDoorData(JE::Resources::ResourceMemoryStream* stream); //40E5E0.
		BYTE ReadDoorVersion1(JE::Resources::ResourceMemoryStream* stream); //40E660.
		BYTE ReadDoor(JE::Common::Chunk& chunk, JE::Resources::ResourceMemoryStream* stream,
			BYTE skipDefaultOverride); //40E6E0; third argument is passed but unused.
		BYTE CanOperate(JE::Entity* actor); //40E5A0, exact-one inventory permission.
		void Operate(JE::Entity* actor); //40EC90, scripts before state/animation/grid changes.
		virtual void OnActivate(DWORD time); //40FD60/40FFE0.
		void BuildFogHeights(); //40F3C0, actual animated collision raster.
		virtual std::list<JE::AreaMapHeight>* GetHeightInfluence(); //40EA20, NULL when list empty.
		BYTE open, locked; //Original+BC/+BD, normalized by reader.
		DWORD valueC0; //Original reader40E660; ctor leaves unwritten.
		std::string keyResource; //+C4, owned.
		std::list<JE::AreaMapHeight> doorHeights; //+E0, owned cell/height values.
	protected:
		GameDoor(); //40E7F0.
		virtual void AddFootprint(JE::RLEGrid<JE::Entity*>* grid); //40FFE8/4100A0.
		virtual void RemoveFootprint(JE::RLEGrid<JE::Entity*>* grid); //40F300/40FC20.
	};
}
