#include "game/ActionbarInterface.hpp"
#include "gui/Window.hpp"
#include "gui/Picture.hpp"
#include "gui/Button.hpp"
#include "gui/Label.hpp"
#include <new>

namespace F3
{
	namespace
	{
		void CopyButton(::GUI::Button* source, ::GUI::Button* target, ::GUI::Window* parent)
		{
			LONG x, y, width, height;
			std::string skin;
			source->GetAbsolutePosition(x, y); source->GetSize(width, height); source->GetSkin(skin);
			const BYTE frame = source->GetFrameVisible();
			target->SetName("PopUpButton");
			target->SetAbsolutePosition(x, y); target->Resize(width, height);
			target->ApplySkin(skin); target->SetFrameVisible(frame);
			parent->AddChild(target);
		}
		enum PictureKind { Plate, Icon, CalledShot };
		void CopyPicture(::GUI::Picture* source, ::GUI::Picture* target, const char* name,
			::GUI::Window* parent, PictureKind kind)
		{
			LONG x, y, width, height;
			std::string skin, texture;
			source->GetAbsolutePosition(x, y); source->GetSize(width, height); source->GetSkin(skin);
			if (kind != Icon) source->GetTextureName(texture);
			const BYTE frame = source->GetFrameVisible();
			if (kind == CalledShot)
			{
				//Actual419914/419A91 load and frame prefix precedes name/geometry/skin.
				target->LoadTexture(texture, 0, 0, 0, -1, -1, 1);
				target->SetFrameVisible(frame);
			}
			target->SetName(name);
			target->SetAbsolutePosition(x, y); target->Resize(width, height); target->ApplySkin(skin);
			if (kind == Plate) target->LoadTexture(texture, 0, 0, 0, -1, -1, 1);
			if (kind != CalledShot) target->SetFrameVisible(frame);
			if (kind != Plate) target->SetValue3B(1);
			parent->AddChild(target);
		}
		void CopyLabel(::GUI::Label* source, ::GUI::Label* target, ::GUI::Window* parent)
		{
			LONG x, y, width, height, fontSize;
			DWORD flags;
			std::string skin, face;
			source->GetAbsolutePosition(x, y); source->GetSize(width, height); source->GetSkin(skin);
			source->GetFont(face, &flags, &fontSize, NULL);
			const BYTE frame = source->GetFrameVisible(); //Original also queries unused textFrame.
			const DWORD horizontal = source->alignment, vertical = source->verticalAlignment;
			target->SetName("PopUpText");
			target->SetAbsolutePosition(x, y); target->Resize(width, height); target->ApplySkin(skin);
			target->SetFont(face, flags, fontSize, 1);
			target->SetTextColor(0xB46E6E3C);
			target->SetFrameVisible(frame);
			target->SetAlignment(horizontal);
			//Actual4AA3A0 setter: field write then RebuildText, no replacement layout.
			target->verticalAlignment = vertical;
			target->RebuildText();
			parent->AddChild(target);
		}
	}

	void ActionbarInterface::BuildPopupButtons() //4188A0.
	{
		for (int i = 0; i < 6; ++i)
		{
			ActionbarPopUpButton* popup = new(std::nothrow) ActionbarPopUpButton;
			popups[i] = popup;
			CopyButton(weaponButtons[0], popup->button, rootWindow);
			::GUI::Picture* up = static_cast< ::GUI::Picture*>(weaponButtons[0]->FindWindow("btnUp"));
			CopyPicture(up, popup->up, "btnUp", popup->button, Plate);
			::GUI::Picture* down = static_cast< ::GUI::Picture*>(weaponButtons[0]->FindWindow("btnDown"));
			CopyPicture(down, popup->down, "btnDown", popup->button, Plate);
			CopyPicture(weaponIconsUp[0], popup->iconUp, "PopUpIcon", popup->up, Icon);
			CopyPicture(weaponIconsDown[0], popup->iconDown, "PopUpIcon", popup->down, Icon);
			CopyLabel(weaponModeUp[0], popup->modeUp, popup->up);
			CopyLabel(weaponModeDown[0], popup->modeDown, popup->down);
			CopyLabel(weaponAPUp[0], popup->apUp, popup->up);
			CopyLabel(weaponAPDown[0], popup->apDown, popup->down);
			CopyPicture(calledShotUp[0], popup->calledShotUp, "CalledShotIcon_up", popup->up, CalledShot);
			CopyPicture(calledShotDown[0], popup->calledShotDown, "CalledShotIcon_down", popup->down, CalledShot);
			popup->button->SetHidden(1);
		}
	}
}
