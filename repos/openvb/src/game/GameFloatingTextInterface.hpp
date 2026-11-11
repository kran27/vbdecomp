#pragma once

#include "graphics/FloatingTextInterface.hpp"
#include "game/GameInterface.hpp"

namespace F3
{
	class GameFloatingTextInterface : public GameInterface, public JE::FloatingTextInterface
	{
	public:
		explicit GameFloatingTextInterface(const std::string& name); //4390D0.
		virtual ~GameFloatingTextInterface();
		virtual void AddSpeech(DWORD, const std::string&, DWORD, DWORD, DWORD, DWORD); //439320.
		virtual void AfterPosition(GUI::FloatingLabel*, LONG, LONG, LONG, LONG); //438F80.
	};
}
