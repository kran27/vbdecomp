#pragma once

#include "game/GameInterface.hpp"

namespace F3
{
	class EscapeMenuInterface : public GameInterface
	{
	public:
		explicit EscapeMenuInterface(const std::string& name); //433A90.
		virtual ~EscapeMenuInterface(); //4338E0/433900.
		virtual int OnEvent(::GUI::Window* window, ::GUI::Event* event); //433EE0, thunk434110.
		void ReturnToMainMenu(); //433BE0, actual Escape receiver; policy factor remains unclosed.
		::GUI::Window* rootWindow; //Original+4; +8 is compiler vtordisp, not a user field.
	};
}
