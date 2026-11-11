#include "game/LoadingBarInterface.hpp"

namespace F3
{
	LoadingBarInterface::LoadingBarInterface(const std::string& name) : JE::EngineInterface(name), GameInterface(name)
	{
		LoadResource("Loading.int"); //Original ignores the result.
		loadingWindow = FindWindow("LoadingWin");
	}
	LoadingBarInterface::~LoadingBarInterface() {}
	int LoadingBarInterface::OnEvent(::GUI::Window* window, ::GUI::Event* event)
	{
		return JE::EngineInterface::OnEvent(window, event);
	}
}
