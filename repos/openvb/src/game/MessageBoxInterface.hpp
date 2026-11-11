#pragma once

#include "game/GameInterface.hpp"
#include <stddef.h>

namespace JE { class CSound; }

namespace F3
{
	class MessageBoxInterface : public GameInterface
	{
	public:
		typedef void (__cdecl *Callback)(LONG answer);
		MessageBoxInterface(); //441420, fixed actual MessageBox name.
		virtual ~MessageBoxInterface(); //441320.
		virtual int OnEvent(::GUI::Window* window, ::GUI::Event* event); //441000/4413D0.
		void Show(DWORD reference, LONG mode, Callback callback); //441B30.
		void ShowText(const std::string& text, LONG mode, Callback callback); //441C00.
		void Close(); //440FC0.
		JE::CSound* openSound; //Original+04, owned raw allocation.
		JE::CSound* closeSound; //Original+08, owned raw allocation.
		LONG mode, answer, buttonAnswers[3]; //Original+0C,+10,+14..1C.
		Callback callback; //+20, borrowed code pointer, reloaded after Close.
	private:
		void ShowButtons(LONG mode); //441650.
	};
}

#if defined(_MSC_VER) && _MSC_VER == 1300 && defined(_M_IX86)
typedef char F3_MessageBox_Size[(sizeof(F3::MessageBoxInterface) == 0x5C) ? 1 : -1];
typedef char F3_MessageBox_OpenSoundOffset[(offsetof(F3::MessageBoxInterface, openSound) == 4) ? 1 : -1];
typedef char F3_MessageBox_CloseSoundOffset[(offsetof(F3::MessageBoxInterface, closeSound) == 8) ? 1 : -1];
typedef char F3_MessageBox_ModeOffset[(offsetof(F3::MessageBoxInterface, mode) == 0x0C) ? 1 : -1];
typedef char F3_MessageBox_CallbackOffset[(offsetof(F3::MessageBoxInterface, callback) == 0x20) ? 1 : -1];
#endif
