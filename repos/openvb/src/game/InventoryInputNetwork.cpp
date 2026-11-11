#include "startup/NetworkComponent.hpp"
#include "network/EngineClient.hpp"
#include "game/InventoryInputServices.hpp"
#include "graphics/ClientCursor.hpp"
#include <stdexcept>
namespace F3 { namespace InventoryInputServices
{
    namespace { Binding* binding=NULL; }
    Binding* SetBinding(Binding* value)
    { Binding* previous=binding; binding=value; return previous; }
    JE::ClientInputState& InputState()
    {
        if (!binding || !binding->state)
            throw std::logic_error("InventoryInterface: actual borrowed Client input state required");
        return *binding->state;
    }
    HWND Window()
    {
        if (!binding || !binding->window)
            throw std::logic_error("InventoryInterface: actual input HWND required");
        return binding->window;
    }
    JE::Client* GetClient()
    {
        //No NULL-network substitute: originaldereferencesNetwork owner then testsclient.
        return static_cast<JE::Client*>(g_pNetworkInstance->client);
    }
	void SetContext(GameCursorManager& cursor,DWORD value)
	{
		if (binding)
		{
			cursor.SetContext(value,InputState(),Window());
		}
		else
		{
			cursor.SetContext(value);
		}
	}
	void BeginItemDrag(GameCursorManager& cursor,DWORD item,DWORD owner)
	{
		if (binding)
		{
			cursor.BeginItemDrag(item,owner,InputState(),Window());
		}
		else
		{
			cursor.BeginItemDrag(item,owner);
		}
	}
	BYTE CancelSpecialContext(GameCursorManager& cursor)
	{
		if (binding)
		{
			return cursor.CancelSpecialContext(InputState(),Window());
		}
		return cursor.CancelSpecialContext();
	}
    //The getters remain borrowed-only. Partial explicit bindings still throw;
    //native dispatch owns no snapshot and retains SetContext's immediate refresh.
} }

