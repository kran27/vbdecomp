#include "startup/NetworkComponent.hpp"
#include "network/EngineClient.hpp"
#include "game/InventoryInputServices.hpp"
#include "graphics/ClientCursor.hpp"
#include <stdexcept>
namespace F3 { namespace InventoryInputServices
{
    JE::Client* GetClient()
    {
        //No NULL-network substitute: originaldereferencesNetwork owner then testsclient.
        return static_cast<JE::Client*>(g_pNetworkInstance->client);
    }
	void SetContext(GameCursorManager& cursor,DWORD value)
	{
		cursor.SetContext(value);
	}
	void BeginItemDrag(GameCursorManager& cursor,DWORD item,DWORD owner)
	{
		cursor.BeginItemDrag(item,owner);
	}
	BYTE CancelSpecialContext(GameCursorManager& cursor)
	{
		return cursor.CancelSpecialContext();
	}
} }

