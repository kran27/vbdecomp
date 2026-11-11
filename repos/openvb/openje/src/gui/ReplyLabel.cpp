#include "gui/ReplyLabel.hpp"
#include "gui/DialogueWindow.hpp"
#include "gui/Event.hpp"

namespace JE
{
	ReplyLabel::ReplyLabel() : ::GUI::Label(3)
	{
		dialogueWindow = NULL;
		replyID = 0xFFFFFFFFu;
		SetValue3B(0); //591BEA. Do not substitute a GUI::Button/audio path.
	}

	ReplyLabel::~ReplyLabel() {} //Ordinary591C10; actual Label base owns its members.

	void ReplyLabel::SetReplyID(DWORD value) { replyID = value; }
	void ReplyLabel::SetDialogueWindow(DialogueWindow* value) { dialogueWindow = value; }

	BOOL ReplyLabel::OnEvent(::GUI::Event* event)
	{
		if (event->type == 1)
			SetTextColor(0xFF808000u);
		else if (event->type == 3)
			SetTextColor(0xFF808080u);
		else if (event->type == 5 && replyID != 0xFFFFFFFFu)
			dialogueWindow->SelectReply(replyID);
		// Original direct4AFFA0 call, after the hover/send side effect. No early
		// successful return, capture/pressed check, revision change or debounce.
		return ::GUI::Window::OnEvent(event);
	}
}
