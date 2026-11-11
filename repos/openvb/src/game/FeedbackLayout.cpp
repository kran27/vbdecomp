#include "game/FeedbackInterface.hpp"
#include "gui/Label.hpp"

namespace F3
{
	void FeedbackInterface::Resize(LONG newWidth,LONG newHeight) //434910, actualRET8, no EBP argument.
	{
		if (newWidth<10 || newHeight<10 || (newWidth==width && newHeight==height)) return;
		lineTemplate->Resize(newWidth-rightMargin-leftMargin,newHeight);
		width=newWidth; height=newHeight;
		rootWindow->Resize(newWidth,newHeight);
		const LONG count=(height-topMargin-bottomMargin)/rowHeight;
		if (count!=rowCount)
		{
			ClearRows();
			for (LONG i=0;i<count;++i)
			{
				::GUI::Label* label=new ::GUI::Label(3);
				rootWindow->AddChild(label);
				label->ApplySkin(rowSkin);
				label->SetFont(rowFontName,0,rowFontSize,1);
				label->SetTextColor(0xFF808080);
				label->SetText("");
				rows.push_front(label);
			}
			rowCount=count;
		}
		LONG index=1;
		for (std::list<::GUI::Label*>::iterator i=rows.begin();i!=rows.end();++i,++index)
		{
			::GUI::Label* label=*i;
			if (label)
			{
				label->Resize(width-rightMargin-leftMargin,rowHeight);
				label->SetAbsolutePosition(leftMargin+x,height+y-index*rowHeight-bottomMargin);
			}
		}
		Refresh();
	}
}
