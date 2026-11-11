#pragma once

#include "gui/Label.hpp"

namespace JE
{
	class DialogueWindow;

	class ReplyLabel : public ::GUI::Label
	{
	public:
		ReplyLabel(); //591BA0, actual Label type3.
		~ReplyLabel(); //591C10, ordinary base lifetime; no owner delete.
		void SetReplyID(DWORD value); //591C30, caller ignores original EAX=value.
		void SetDialogueWindow(DialogueWindow* value); //591C40, original EAX=value ignored.
		BOOL OnEvent(::GUI::Event* event); //592680, actual Window+11C.

		DWORD replyID; //Original+E64, FFFFFFFF sentinel.
		DialogueWindow* dialogueWindow; //Original+E68, borrowed, initiallyNULL.
	};
}
