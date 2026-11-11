#pragma once

#include <windows.h>

namespace JE
{
	class Client;
	struct ClientInputState;

	//Typed input snapshot binding, not an original Client layout or a HUD owner.
	void CaptureClientSelectionState(const Client& client, ClientInputState& state);
	void PublishClientSelectionState(Client& client, const ClientInputState& state);
	void PublishClientSelectedEntity(Client& client, DWORD id); //46169C state-only factor.
}
