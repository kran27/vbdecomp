#include "game/FeedbackInterface.hpp"
#include "game/ActionbarInterface.hpp"
#include "game/FeedbackServices.hpp"
#include "startup/GameClient.hpp"
#include "startup/NetworkComponent.hpp"
#include "gui/Event.hpp"
#include "gui/Label.hpp"

namespace F3
{
	FeedbackInterface::FeedbackInterface(const std::string& name) : JE::EngineInterface(name), GameInterface(name) //434F70.
	{
		LoadResource("Feedback.INT");
		rows.clear();
		history.clear();
		leftMargin=rightMargin=topMargin=bottomMargin=rowHeight=0;
		rowSkin="";
		rowFontName="";
		rowFontSize=0;
		x=y=width=height=rowCount=0;
		value20=value24=value28=value2C=value30=0;
		value34=value38=value3C=value40=0;
		rootWindow=FindWindow("FeedbackWin");
		lineTemplate=static_cast<::GUI::Label*>(FindWindow("FeedbackLine"));
		if (rootWindow)
		{
			LONG rootX,rootY,rootWidth,rootHeight;
			rootWindow->GetAbsolutePosition(rootX,rootY);
			rootWindow->GetSize(rootWidth,rootHeight);
			x=rootX; y=rootY; width=rootWidth; height=rootHeight;
			rootWindow->SetAlpha(0);
			if (lineTemplate)
			{
				LONG lineX,lineY,lineWidth,lineHeight;
				lineTemplate->GetClientPosition(lineX,lineY);
				lineTemplate->GetSize(lineWidth,lineHeight);
				leftMargin=lineX;
				rightMargin=rootWidth-lineWidth-lineX;
				topMargin=lineY;
				bottomMargin=rootHeight-lineHeight-lineY;
				rowHeight=lineHeight;
				lineTemplate->GetSkin(rowSkin);
				lineTemplate->GetFont(rowFontName,0,0,0);
				rowFontSize=lineTemplate->GetFontSize();
				lineTemplate->SetHidden(1);
				lineTemplate->SetWrap(1);
			}
		}
		AddMessage("Welcome to Fallout.",128,128,128,"");
	}

	FeedbackInterface::~FeedbackInterface() //434340.
	{
		ClearRows();
		ClearHistory();
		//Real lists/strings and qualified GameInterface member/base lifetime follow.
	}

	void FeedbackInterface::ClearRows() //434250.
	{
		for (std::list<::GUI::Label*>::iterator i=rows.begin();i!=rows.end();++i)
			if (*i) delete *i;
		rows.clear();
	}

	void FeedbackInterface::ClearHistory() //4342C0.
	{
		for (std::list<FeedbackMessage*>::iterator i=history.begin();i!=history.end();++i)
			if (*i) delete *i;
		history.clear();
	}

	void FeedbackInterface::SetAbsolutePosition(LONG newX,LONG newY) //434120.
	{
		if (newX<0) newX=0;
		if (newY<0) newY=0;
		if (width+newX>1024) newX=1024-width;
		if (height+newY>768) newY=768-height;
		if (newX!=x || newY!=y)
		{
			x=newX;
			y=newY;
			rootWindow->SetAbsolutePosition(newX,newY);
		}
	}

	int FeedbackInterface::OnEvent(::GUI::Window* window,::GUI::Event* event) //434180.
	{
		if (event->type==6 && window==rootWindow)
			dynamic_cast<GameClient*>(static_cast<JE::Client*>(g_pNetworkInstance->client))->Interfaces()->actionbar->SetCompanionMode(2); //4172D0, then RefreshCompanions416B80.
		return JE::EngineInterface::OnEvent(window,event); //5803E0 after actual prefix.
	}
}

