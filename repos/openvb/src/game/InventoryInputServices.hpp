#pragma once
#include <windows.h>
#include <string>
#include "gui/EngineInterfaceServices.hpp"
namespace JE { class Client; }
namespace F3
{
	class GameCursorManager;
    namespace InventoryInputServices
    {
        JE::Client* GetClient(); // Actual Network+2C, may be NULL.
		void SetContext(GameCursorManager& cursor,DWORD value);
		void BeginItemDrag(GameCursorManager& cursor,DWORD item,DWORD owner);
		BYTE CancelSpecialContext(GameCursorManager& cursor);
    }
}

