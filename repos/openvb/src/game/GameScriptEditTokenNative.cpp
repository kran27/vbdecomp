#include <winsock2.h>
#include "GameScriptNatives.hpp"
#include "scripting/ScriptEnvironment.hpp"
#include "network/EngineClient.hpp"
#include "gui/EditTokenInterface.hpp"
#include "gui/Label.hpp"
#include "gui/Manager.hpp"
#include "startup/NetworkComponent.hpp"

namespace JE
{
	void EditTokenInterface::EditToken(DWORD stringReference,
		const char* name) //5B4020.
	{
		tokenName.assign(name);
		if (!tokenName.empty())
		{
			::GUI::g_Manager.PushModal(this);
			up->SetHidden(0);
			down->SetHidden(1);
			::GUI::Window* label = main->FindWindow("Label");
			if (label)
				static_cast< ::GUI::Label*>(label)->SetStringReference(stringReference);
			main->SetHidden(0);
		}
	}

}

namespace F3
{
	cell AMX_NATIVE_CALL NativeEditToken(AMX* vm, cell* parameters) //5C7C60.
	{
		char name[256];
		if (!VerifyScriptParameters(vm, parameters, 2, "EditToken")) return 0;
		const DWORD stringReference = DWORD(parameters[1]);
		if (!JE::ScriptServices::ReadNativeString(vm, parameters[2], name, sizeof(name)))
			LogScriptError(vm, "EditToken() is unable to get the token string.");
		else
		{
			JE::Client* client = static_cast<JE::Client*>(g_pNetworkInstance->client);
			if (client) client->EditToken(stringReference, name);
		}
		return 0; //5C7CEE: every original path returns zero, including UI setup.
	}
}
