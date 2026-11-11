#pragma once
#include <winsock2.h>

class MapMaskManager;
namespace JE { class Client; }
namespace F3
{
	class ActionbarInterface;
	namespace MiniMapServices
	{
		//Borrowed actual owners from original Network.Client -> Graphics+74 /
		//GameInterfaces+A0. A standalone host may project its already real owners;
		//these getters never construct or replace either missing parent.
		JE::Client& GetClient();
		MapMaskManager& GetMaskManager();
		ActionbarInterface& GetActionbar();
	}
}

