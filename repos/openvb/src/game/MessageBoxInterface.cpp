#include "game/MessageBoxInterface.hpp"
#include "gui/Event.hpp"
#include "gui/Label.hpp"
#include "gui/Manager.hpp"
#include "gui/Window.hpp"
#include "audio/Sound.hpp"
#include <cstdio>
#include <cstring>

namespace F3
{
	MessageBoxInterface::MessageBoxInterface() //441420.
		: JE::EngineInterface("MessageBox"), GameInterface("MessageBox"), mode(0), answer(0)
	{
		buttonAnswers[0] = 0;
		buttonAnswers[1] = 0;
		buttonAnswers[2] = 0;
		callback = NULL;
		LoadResource("MessageBox.INT");
		openSound = new JE::CSound;
		openSound->Load("f3_messagebox_open.wav", 0);
		closeSound = new JE::CSound;
		closeSound->Load("f3_messagebox_close.wav", 0);
		SetHidden(1);
	}
	MessageBoxInterface::~MessageBoxInterface()
	{
		if (openSound) delete openSound;
		if (closeSound) delete closeSound; //Reload only after the open sound has been deleted.
	}
	void MessageBoxInterface::Show(DWORD reference, LONG inputMode, Callback inputCallback)
	{
		callback = inputCallback;
		::GUI::Label* label = static_cast<::GUI::Label*>(FindWindow("lblMessage"));
		label->SetStringReference(reference);
		ShowButtons(inputMode);
	}
	void MessageBoxInterface::ShowText(const std::string& text, LONG inputMode,
		Callback inputCallback)
	{
		callback = inputCallback;
		::GUI::Label* label = static_cast<::GUI::Label*>(FindWindow("lblMessage"));
		label->SetText(std::string(text));
		ShowButtons(inputMode);
	}
	void MessageBoxInterface::ShowButtons(LONG inputMode) //441650.
	{
		LONG first = -1, second = -1, third = -1;
		mode = inputMode;
		switch (mode)
		{
		case 0: second = 9; buttonAnswers[1] = 1; break;
		case 1: second = 31; buttonAnswers[1] = 2; break;
		case 2:
			third = 12; buttonAnswers[2] = 4;
			first = 11; buttonAnswers[0] = 3;
			break;
		case 3:
			second = 12; buttonAnswers[1] = 4;
			third = 10; buttonAnswers[2] = 5;
			first = 11; buttonAnswers[0] = 3;
			break;
		}
		{
			::GUI::Window* container = FindWindow("winButton1");
			if (container)
			{
				if (first >= 0)
				{
					container->SetHidden(0);
					::GUI::Label* label = static_cast<::GUI::Label*>(FindWindow("lbl1"));
					if (label) label->SetStringReference(first);
				}
				else
					container->SetHidden(1);
			}
		}
		{
			::GUI::Window* container = FindWindow("winButton2");
			if (container)
			{
				if (second >= 0)
				{
					container->SetHidden(0);
					::GUI::Label* label = static_cast<::GUI::Label*>(FindWindow("lbl2"));
					if (label) label->SetStringReference(second);
				}
				else
					container->SetHidden(1);
			}
		}
		{
			::GUI::Window* container = FindWindow("winButton3");
			if (container)
			{
				if (third >= 0)
				{
					container->SetHidden(0);
					::GUI::Label* label = static_cast<::GUI::Label*>(FindWindow("lbl3"));
					if (label) label->SetStringReference(third);
				}
				else
					container->SetHidden(1);
			}
		}
		::GUI::Window* root = FindWindow("winMessageBox");
		root->BringToFront();
		root->Center();
		SetHidden(0);
		::GUI::g_Manager.PushModal(this);
		if (openSound)
		{
			openSound->Prepare();
			openSound->Play(0, 0, 0); //Reload the member after Prepare, as at441AFB.
		}
	}
	void MessageBoxInterface::Close() //440FC0.
	{
		if (closeSound)
		{
			closeSound->Prepare();
			closeSound->Play(0, 0, 0); //440FCF reloads the close-sound member.
		}
		SetHidden(1);
		::GUI::g_Manager.RemoveModal(this);
	}
	int MessageBoxInterface::OnEvent(::GUI::Window* window, ::GUI::Event* event)
	{
		if (GetHidden()) return 1;
		if (event->type == 6)
		{
			if (window->type != 1) goto Done;
			if (!_stricmp(window->GetName(), "btn1")) answer = buttonAnswers[0];
			else if (!_stricmp(window->GetName(), "btn2")) answer = buttonAnswers[1];
			else if (!_stricmp(window->GetName(), "btn3")) answer = buttonAnswers[2];
			else goto Done;
			goto AnswerSelected;
		}
		if (event->type != 17 && event->type != 19) goto Done;
		switch (event->x)
		{
		case 1:
			switch (mode)
			{
			case 0: case 1: answer = buttonAnswers[1]; break;
			case 2: case 3: answer = buttonAnswers[2]; break;
			default: goto Done;
			}
			break;
		case 28: case 156: case 284:
			switch (mode)
			{
			case 0: case 1: answer = buttonAnswers[1]; break;
			case 2: case 3: answer = buttonAnswers[0]; break;
			default: goto Done;
			}
			break;
		case 57:
			if (mode < 0 || mode > 1) goto Done;
			answer = buttonAnswers[1];
			break;
		case 21: case 89: case 121:
			if (mode < 2 || mode > 3) goto Done;
			answer = buttonAnswers[0];
			break;
		case 49: case 78: case 110:
			if (mode == 2) answer = buttonAnswers[2];
			else if (mode == 3) answer = buttonAnswers[1];
			else goto Done;
			break;
		case 46: case 67: case 99:
			if (mode != 3) goto Done;
			answer = buttonAnswers[2];
			break;
		case 24: case 79: case 111:
			if (mode != 0) goto Done;
			answer = buttonAnswers[1];
			break;
		case 32: case 68: case 100:
			if (mode != 1) goto Done;
			answer = buttonAnswers[1];
			break;
		default: goto Done;
		}
	AnswerSelected:
		Close();
		if (callback) callback(answer);
	Done:
		return JE::EngineInterface::OnEvent(window, event);
	}
}
