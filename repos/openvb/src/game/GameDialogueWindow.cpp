#include "network/Types.hpp"
#include "game/GameDialogueWindow.hpp"
#include "game/GameCreature.hpp"
#include "gui/Picture.hpp"
#include "startup/EngineStringSystem.hpp"

namespace F3
{
	GameDialogueWindow::GameDialogueWindow(BYTE id, BYTE revision,
		JE::DialogueInterface* source) : JE::DialogueWindow(id, revision, source)
	{
		//437ED0 only forwards construction and publishes derived tables.
	}

	void GameDialogueWindow::SetParticipants(DWORD listenerID, DWORD speakerID)
	{
		// Preserve resolution/cast order even though the public args are listener first.
		GameCreature* speaker = dynamic_cast<GameCreature*>(JE::GetEntityPtr(speakerID));
		GameCreature* listener = dynamic_cast<GameCreature*>(JE::GetEntityPtr(listenerID));
		if (speaker)
			speakerPicture->LoadTexture(speaker->portrait, 1, 0.0f, 0.0f, -1.0f, -1.0f, 1);
		if (listener)
			listenerPicture->LoadTexture(listener->portrait, 1, 0.0f, 0.0f, -1.0f, -1.0f, 1);
		//437FA8 retains the original unconditional listener dereference.
		//56B940 ->actualTokenTable5AD460. The raw string ref is not decremented or
		//converted to rendered text; existing token resolver semantics remain intact.
		g_EngineStringSystem.SetTokenValue("CHARNAME", static_cast<JE::Net::Int32>(listener->stringRefs[0]));
		JE::DialogueWindow::SetParticipants(listenerID, speakerID);
	}
}
