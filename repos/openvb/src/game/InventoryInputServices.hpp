#pragma once
#include <windows.h>
#include <string>
#include "gui/EngineInterfaceServices.hpp"
namespace JE { class Client; struct ClientInputState; }
namespace F3
{
	class GameCursorManager;
    // Explicit bindings borrow existing input/window; absent bindings use native cursor APIs.
    // No additional Client or synthetic input snapshot is constructed.
    namespace InventoryInputServices
    {
        struct Binding { JE::ClientInputState* state; HWND window; };
        Binding* SetBinding(Binding* value); //Explicit borrowed host input, no original owner claim.
        JE::Client* GetClient(); // Actual Network+2C, may be NULL.
        JE::ClientInputState& InputState();
        HWND Window();
		void SetContext(GameCursorManager& cursor,DWORD value);
		void BeginItemDrag(GameCursorManager& cursor,DWORD item,DWORD owner);
		BYTE CancelSpecialContext(GameCursorManager& cursor);
    }
}

