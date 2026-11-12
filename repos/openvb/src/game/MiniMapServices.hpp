#pragma once
#include <winsock2.h>

class MapMaskManager;
namespace JE { class Client; }
namespace F3
{
	class ActionbarInterface;
	namespace MiniMapServices
	{
		//Borrowed current Client, Graphics and Actionbar owners.
		JE::Client& GetClient();
		MapMaskManager& GetMaskManager();
		ActionbarInterface& GetActionbar();
	}
}

