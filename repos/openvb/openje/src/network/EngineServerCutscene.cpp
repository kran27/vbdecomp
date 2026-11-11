#include "EngineServer.hpp"
#include "entity/World.hpp"

namespace JE
{
	void Server::StartCutscene(DWORD id, DWORD mapID) //556080.
	{
		IWorld* world = g_World;
		world->StartCutscene(id, mapID);
		SendCutsceneStart(id, mapID);
	}
	void Server::SendCutsceneStart(DWORD id, DWORD mapID) //553470.
	{
		Net::Packet notification;
		notification.field0x30 = 77;
		notification.WriteDword(id);
		notification.WriteDword(mapID);
		Broadcast(notification, 1);
	}

	void Server::StopCutscene() //5560C0, owns actual553550 sender/EH scope.
	{
		IWorld* world = g_World;
		world->StopCutscene();
		SendCutsceneStop();
	}
	void Server::SendCutsceneStop() //553550.
	{
		Net::Packet notification;
		notification.field0x30 = 78;
		Broadcast(notification, 1);
	}
}
