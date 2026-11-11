#include <winsock2.h>
#include "FontTestInterface.hpp"
#include "TextEditInterface.hpp"
#include "gui/Label.hpp"
#include "gui/Event.hpp"
#include "gui/Manager.hpp"
#include "network/EngineClient.hpp"
#include "startup/NetworkComponent.hpp"
#include <cstring>

namespace JE
{
	void Client::CommandFontTest() //5887B0.
	{
		FontTestInterface* owner = static_cast<Client*>(F3::g_pNetworkInstance->client)->child0x98;
		if (owner)
		{
			if (owner->GetHidden())
			{
				owner->SetHidden(0);
				owner->BringToFront();
			}
			else owner->SetHidden(1);
		}
	}

	FontTestInterface::FontTestInterface(const std::string& name) : EngineInterface(name)
	{
		LoadResource("FontTest.INT");
		std::string face;
		main = FindWindow("TextWin");
		text1 = static_cast<::GUI::Label*>(FindWindow("Text1"));
		text2 = static_cast<::GUI::Label*>(FindWindow("Text2"));
		text3 = static_cast<::GUI::Label*>(FindWindow("Text3"));
		text4 = static_cast<::GUI::Label*>(FindWindow("Text4"));
		if (text1)
		{
			text1->GetFont(face, NULL, NULL, NULL);
			LONG size = text1->GetFontSize();
			text1->SetFont(face, 0, size, 1);
			text1->SetText("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz1234567890`~!@#$%^&*()-_=+[{]}\\|;:'\",<.>/?");
			text2->SetFont(face, 1, size, 1);
			text2->SetText("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz1234567890`~!@#$%^&*()-_=+[{]}\\|;:'\",<.>/?");
			text3->SetFont(face, 2, size, 1);
			text3->SetText("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz1234567890`~!@#$%^&*()-_=+[{]}\\|;:'\",<.>/?");
			text4->SetFont(face, 0, size, 1);
			text4->SetText("Custom Text");
		}
		editor = new TextEditInterface("TextEdit");
		editor->SetHidden(1);
	}

	FontTestInterface::~FontTestInterface() {}

	int FontTestInterface::OnEvent(::GUI::Window* window, ::GUI::Event* event)
	{
		if (GetHidden()) return EngineInterface::OnEvent(window, event);
		if (event->type == 4 && window->type == 1)
		{
			if (!_stricmp(window->GetName(), "FontBtn"))
			{
				editor->SetHidden(0);
				editor->BringToFront();
				editor->Clear();
				editor->SetMode(1);
				::GUI::g_Manager.PushModal(editor);
			}
			else if (!_stricmp(window->GetName(), "FontSizeBtn"))
			{
				editor->SetHidden(0);
				editor->BringToFront();
				editor->Clear();
				editor->SetMode(2);
				::GUI::g_Manager.PushModal(editor);
			}
			else if (!_stricmp(window->GetName(), "TextBtn"))
			{
				editor->SetHidden(0);
				editor->BringToFront();
				editor->Clear();
				editor->SetMode(3);
				::GUI::g_Manager.PushModal(editor);
			}
			else if (!_stricmp(window->GetName(), "AlphaBtn"))
			{
				editor->SetHidden(0);
				editor->BringToFront();
				editor->Clear();
				editor->SetMode(4);
				::GUI::g_Manager.PushModal(editor);
			}
			else if (!_stricmp(window->GetName(), "ColorBtn"))
			{
				editor->SetHidden(0);
				editor->BringToFront();
				editor->Clear();
				editor->SetMode(5);
				::GUI::g_Manager.PushModal(editor);
			}
		}
		return EngineInterface::OnEvent(window, event);
	}
}
