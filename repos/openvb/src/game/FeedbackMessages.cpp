#include "game/FeedbackInterface.hpp"
#include "gui/Label.hpp"

namespace F3
{
	FeedbackMessage::FeedbackMessage()
	{
		text = "";
		speaker = "";
		red = 128;
		green = 128;
		blue = 128;
	}
	void FeedbackInterface::AddMessage(std::string text,DWORD red,DWORD green,DWORD blue,std::string speaker) //434B70.
	{
		if (!lineTemplate) return;
		std::string formatted;
		if (speaker.size() > 0) formatted="-"+speaker+": "+text;
		else formatted="-"+text;
		lineTemplate->SetText(formatted);
		const LONG count=LONG(lineTemplate->lines.size());
		for (LONG i=0;i<count;++i)
		{
			std::string line;
			FeedbackMessage* message=new FeedbackMessage;
			lineTemplate->GetLineText(i,line); //Actual4A8B90; wrapping stays owned by Label.
			message->text=line;
			message->speaker=speaker;
			message->red=red; message->green=green; message->blue=blue;
			history.push_front(message); //Each wrapped row prepends; retain reversed-line order.
		}
		Refresh();
	}
	void FeedbackInterface::Refresh() //434760.
	{
		std::list<::GUI::Label*>::iterator row=rows.begin();
		std::list<FeedbackMessage*>::iterator line=history.begin();
		for (;row!=rows.end() && line!=history.end();++row,++line)
		{
			::GUI::Label* label=*row;
			FeedbackMessage* message=*line;
			if (label && message)
			{
				label->SetText(message->text);
				label->SetTextColor(D3DCOLOR_XRGB(message->red,message->green,message->blue));
			}
		}
		for (;row!=rows.end();++row)
		{
			::GUI::Label* label=*row;
			if (label) label->SetText("");
		}
	}
}
