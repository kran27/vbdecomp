#include "GameDialogueInterface.hpp"
#include "gui/Window.hpp"
#include "gui/Label.hpp"

namespace F3
{
	GameDialogueInterface::GameDialogueInterface(std::string name)
		: JE::EngineInterface(name), GameInterface(name), JE::DialogueInterface(name)
	{
		std::string skin, font;
		LONG width, height, x, y;
		LoadResource("Dialog.INT");
		{
			::GUI::Window* window = FindWindow("SpeakerWin");
			if (window)
			{
				window->GetSize(width, height);
				window->GetSkin(skin);
				speaker.windowSkin = skin;
				speaker.windowWidth = width;
				speaker.windowHeight = height;
				speaker.windowAlpha = 180;
				window->SetHidden(1);
			}
		}
		{
			::GUI::Window* window = FindWindow("SpeakerPic");
			if (window)
			{
				window->GetSize(width, height);
				window->GetClientPosition(x, y);
				window->GetSkin(skin);
				const BYTE frame = window->GetFrameVisible();
				speaker.portraitSkin = skin;
				speaker.portraitFrameVisible = frame;
				speaker.portraitX = x;
				speaker.portraitWidth = width;
				speaker.portraitY = y;
				speaker.portraitHeight = height;
				window->SetHidden(1);
			}
		}
		{
			::GUI::Window* window = FindWindow("SpeakerNameLbl");
			if (window)
			{
				window->GetSize(width, height);
				window->GetClientPosition(x, y);
				window->GetSkin(skin);
				::GUI::Label* label = static_cast<::GUI::Label*>(window);
				label->GetFont(font, NULL, NULL, NULL);
				const LONG fontSize = label->GetFontSize();
				speaker.nameLabelSkin = skin;
				speaker.nameLabelFontName = font;
				speaker.nameLabelFontSize = fontSize;
				speaker.nameLabelX = x;
				speaker.nameLabelY = y;
				speaker.nameLabelFontFlags = 0;
				speaker.nameLabelWidth = width;
				speaker.nameLabelHeight = height;
				window->SetHidden(1);
			}
		}
		{
			::GUI::Window* window = FindWindow("SpeakerTextLbl");
			if (window)
			{
				window->GetSize(width, height);
				window->GetClientPosition(x, y);
				window->GetSkin(skin);
				::GUI::Label* label = static_cast<::GUI::Label*>(window);
				label->GetFont(font, NULL, NULL, NULL);
				const LONG fontSize = label->GetFontSize();
				speaker.textLabelSkin = skin;
				speaker.textLabelFontName = font;
				speaker.textLabelX = x;
				speaker.textLabelFontSize = fontSize;
				speaker.textLabelHeight = height;
				speaker.textLabelY = y;
				speaker.textLabelFontFlags = 0;
				speaker.textLabelWidth = width;
				speaker.textValue100 = 0;
				::GUI::Window* parent = FindWindow("SpeakerWin");
				if (parent)
				{
					LONG parentX, parentY, textX, textY;
					parent->GetAbsolutePosition(parentX, parentY);
					window->GetAbsolutePosition(textX, textY);
					speaker.textBottomMargin = parentY + speaker.windowHeight - speaker.textLabelHeight - textY;
				}
				window->SetHidden(1);
			}
		}
		{
			::GUI::Window* window = FindWindow("SpeakerModel");
			if (window)
			{
				window->GetSize(width, height);
				window->GetClientPosition(x, y);
				window->GetSkin(skin);
				const BYTE frame = window->GetFrameVisible();
				speaker.modelSkin = skin;
				speaker.modelFrameVisible = frame;
				speaker.modelX = x;
				speaker.modelWidth = width;
				speaker.modelY = y;
				speaker.modelHeight = height;
				window->SetHidden(1);
			}
		}
		{
			::GUI::Window* window = FindWindow("ListenerWin");
			if (window)
			{
				window->GetSize(width, height);
				window->GetSkin(skin);
				listener.windowSkin = skin;
				listener.windowWidth = width;
				listener.windowHeight = height;
				listener.windowAlpha = 180;
				window->SetHidden(1);
			}
		}
		{
			::GUI::Window* window = FindWindow("ListenerPic");
			if (window)
			{
				window->GetSize(width, height);
				window->GetClientPosition(x, y);
				window->GetSkin(skin);
				const BYTE frame = window->GetFrameVisible();
				listener.portraitSkin = skin;
				listener.portraitFrameVisible = frame;
				listener.portraitX = x;
				listener.portraitWidth = width;
				listener.portraitY = y;
				listener.portraitHeight = height;
				window->SetHidden(1);
			}
		}
		{
			::GUI::Window* window = FindWindow("ListenerNameLbl");
			if (window)
			{
				window->GetSize(width, height);
				window->GetClientPosition(x, y);
				window->GetSkin(skin);
				::GUI::Label* label = static_cast<::GUI::Label*>(window);
				label->GetFont(font, NULL, NULL, NULL);
				const LONG fontSize = label->GetFontSize();
				listener.nameLabelSkin = skin;
				listener.nameLabelFontName = font;
				listener.nameLabelFontSize = fontSize;
				listener.nameLabelX = x;
				listener.nameLabelY = y;
				listener.nameLabelFontFlags = 0;
				listener.nameLabelWidth = width;
				listener.nameLabelHeight = height;
				window->SetHidden(1);
			}
		}
		{
			::GUI::Window* window = FindWindow("ListenerTextLbl");
			if (window)
			{
				window->GetSize(width, height);
				window->GetClientPosition(x, y);
				window->GetSkin(skin);
				::GUI::Label* label = static_cast<::GUI::Label*>(window);
				label->GetFont(font, NULL, NULL, NULL);
				const LONG fontSize = label->GetFontSize();
				listener.textLabelSkin = skin;
				listener.textLabelFontName = font;
				listener.textLabelX = x;
				listener.textLabelFontSize = fontSize;
				listener.textLabelHeight = height;
				listener.textLabelY = y;
				listener.textLabelFontFlags = 0;
				listener.textLabelWidth = width;
				listener.textValue100 = 0;
				::GUI::Window* parent = FindWindow("ListenerWin");
				if (parent)
				{
					LONG parentX, parentY, textX, textY;
					parent->GetAbsolutePosition(parentX, parentY);
					window->GetAbsolutePosition(textX, textY);
					listener.textBottomMargin = parentY + listener.windowHeight - listener.textLabelHeight - textY;
				}
				window->SetHidden(1);
			}
		}
	}
}
