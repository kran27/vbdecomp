#pragma once

#include "graphics/ClientInput.hpp"

namespace F3
{
	//Actual GameHotKeys owner; normal input/map/console objects, no layout overlay.
	class GameHotKeys : public JE::ClientHotKeys
	{
	public:
		GameHotKeys(JE::Input::Win32Input& input, JE::ConsoleCommandProcessor* console)
			: JE::ClientHotKeys(input, console)
		{
			//45E880 constructs the base and installs this type inside the factory.
			//Initialize/default loading is the separate469090 call.
		}
		virtual ~GameHotKeys(); //45F2D0 ->599CD0: Save, then map destruction.
		virtual void Initialize(); //469090: game defaults, then actual base load overlay.
		void LoadGameDefaults(); //468AE0, actual game defaults; input virtual lookup then base Bind.
	private:
		GameHotKeys(const GameHotKeys&);
		GameHotKeys& operator=(const GameHotKeys&);
	};
}
