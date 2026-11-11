#pragma once

#include "ClientInput.hpp"

class OCTREE;
class OCTREE_Camera;

namespace JE
{
	class Client;
	// Explicit outer Graphics/World/clock getters. The picker operates on the
	// canonical Client input fields and real camera, entities and effect owners.
	class ClientPickingServices
	{
	public:
		virtual ~ClientPickingServices() {}
		virtual DWORD Clock() = 0; //56AF50.
		virtual OCTREE_Camera* CurrentCamera() = 0; //Graphics581190.
		virtual OCTREE* CurrentOctree() = 0; //World interface+0C ->544CF0.
	};
	void PickClientMouse(ClientInputState& state, BYTE force,
		ClientPickingServices& services); //58AB80; incidental AL is not a success result.
	void PickClientMouse(Client& client, BYTE force,
		ClientPickingServices& services); //Canonical live Client fields; no owned snapshot.
}
