#include "network/EngineClient.hpp"
#include "ClientSelectionBinding.hpp"
#include "ClientInput.hpp"

namespace JE
{
	void CaptureClientSelectionState(const Client& client, ClientInputState& state)
	{
		state.clientID = client.clientID;
		state.hoveredEntityID = client.field0xDC;
		state.selectedEntityID = client.field0xE0;
		state.valueE4 = client.field0xE4;
	}

	void PublishClientSelectionState(Client& client, const ClientInputState& state)
	{
		client.field0xDC = state.hoveredEntityID;
		client.field0xE0 = state.selectedEntityID;
		//Client identity and Player EntityID are produced by their actual owners,
		//not written back from an input snapshot.
	}

	void PublishClientSelectedEntity(Client& client, DWORD id)
	{
		client.field0xE0 = id;
		//Original4616AC next requests Actionbar41C9D0 with this numeric ID.
		//The real HUD path remains addressed; this factor does not consume it.
	}
}
