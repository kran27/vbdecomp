#include "game/ActionbarInterface.hpp"
#include "game/CounterControl.hpp"
#include "gui/Window.hpp"
#include "gui/Picture.hpp"
#include "gui/Button.hpp"
#include "gui/Label.hpp"
#include <cstdio>
#include <new>

	PopUpButton::PopUpButton() //417040, actual allocation/publication order.
	{
		button = new ::GUI::Button(1);
		up = new ::GUI::Picture(2);
		down = new ::GUI::Picture(2);
		iconUp = new ::GUI::Picture(2);
		iconDown = new ::GUI::Picture(2);
		modeUp = new ::GUI::Label(3);
		modeDown = new ::GUI::Label(3);
		apUp = new ::GUI::Label(3);
		apDown = new ::GUI::Label(3);
		calledShotUp = new ::GUI::Picture(2);
		calledShotDown = new ::GUI::Picture(2);
	}
	PopUpButton::~PopUpButton() {} //417290 only restores its own vtable.

namespace F3
{
	ActionbarInterface::ActionbarInterface(const std::string& name) //41A2B0.
		: JE::EngineInterface(name), GameInterface(name)
	{
		LoadResource("Actionbar.INT"); //Original ignores return.
		cachedCurrentAP = cachedMaximumAP = cachedReservedAP = -1;
		entityID = otherEntityID = 0;
		value0C = 1;
		popupValue290 = popupValue294 = 0;
		rootWindow = FindWindow("winActionbar");
		if (rootWindow) rootWindow->SetAbsolutePosition(0, 604);
		background = static_cast< ::GUI::Picture*>(FindWindow("picBackground"));
		sneakButton = static_cast< ::GUI::Button*>(FindWindow("btnSneak"));
		inventoryButton = static_cast< ::GUI::Button*>(FindWindow("btnInventory"));
		pipBoyButton = static_cast< ::GUI::Button*>(FindWindow("btnPipBoy"));
		useSkillButton = static_cast< ::GUI::Button*>(FindWindow("btnUseSkill"));
		characterButton = static_cast< ::GUI::Button*>(FindWindow("btnCharacter"));
		optionsButton = static_cast< ::GUI::Button*>(FindWindow("btnOptions"));
		for (int i = 0; i < 6; ++i)
		{
			char name[32];
			std::sprintf(name, "picPlayer%02d", i);
			partyPictures[i] = static_cast< ::GUI::Picture*>(FindWindow(name));
			if (partyPictures[i])
			{
				partyNames[i] = static_cast< ::GUI::Label*>(partyPictures[i]->FindWindow("lblName"));
				partyHealth[i] = static_cast< ::GUI::Picture*>(partyPictures[i]->FindWindow("picHealth"));
				if (partyHealth[i]) partyHealth[i]->SetValue3B(1);
				partyFatigue[i] = static_cast< ::GUI::Picture*>(partyPictures[i]->FindWindow("picFatigue"));
				partyFatigue[i]->SetValue3B(1); //Original does not guard this required child.
				partyPictures[i]->SetHidden(1);
			}
		}
		healthDigits = new(std::nothrow) CounterControl;
		healthDigits->SetDigits(3); healthDigits->SetMinimum(0); healthDigits->SetMaximum(999);
		healthDigits->BindDigit(0, FindWindow("winHealthDigit1"));
		healthDigits->BindDigit(1, FindWindow("winHealthDigit10"));
		healthDigits->BindDigit(2, FindWindow("winHealthDigit100"));
		healthDigits->animationStep = 0.1f;
		fatigueDigits = new(std::nothrow) CounterControl;
		fatigueDigits->SetDigits(3); fatigueDigits->SetMinimum(0); fatigueDigits->SetMaximum(999);
		fatigueDigits->BindDigit(0, FindWindow("winFatigueDigit1"));
		fatigueDigits->BindDigit(1, FindWindow("winFatigueDigit10"));
		fatigueDigits->BindDigit(2, FindWindow("winFatigueDigit100"));
		fatigueDigits->animationStep = 0.1f;
		evasionDigits = new(std::nothrow) CounterControl;
		evasionDigits->SetDigits(3); evasionDigits->SetMinimum(0); evasionDigits->SetMaximum(999);
		evasionDigits->BindDigit(0, FindWindow("winEvasionDigit1"));
		evasionDigits->BindDigit(1, FindWindow("winEvasionDigit10"));
		evasionDigits->BindDigit(2, FindWindow("winEvasionDigit100"));
		evasionDigits->animationStep = 0.1f;

		weaponButtons[0] = static_cast< ::GUI::Button*>(FindWindow("btnWeapon01"));
		weaponIconsUp[0] = static_cast< ::GUI::Picture*>(FindWindow("picWeapon01Icon_up"));
		weaponIconsDown[0] = static_cast< ::GUI::Picture*>(FindWindow("picWeapon01Icon_down"));
		weaponModeUp[0] = static_cast< ::GUI::Label*>(FindWindow("lblWeapon01Mode_up"));
		weaponModeDown[0] = static_cast< ::GUI::Label*>(FindWindow("lblWeapon01Mode_down"));
		weaponAPUp[0] = static_cast< ::GUI::Label*>(FindWindow("lblWeapon01AP_up"));
		weaponAPDown[0] = static_cast< ::GUI::Label*>(FindWindow("lblWeapon01AP_down"));
		weaponMasks[0] = static_cast< ::GUI::Picture*>(FindWindow("picWeapon01Mask_up"));
		reloadIcons[0][0][0] = static_cast< ::GUI::Picture*>(FindWindow("picReloadIcon01_up"));
		reloadIcons[1][0][0] = static_cast< ::GUI::Picture*>(FindWindow("picReloadIcon01_down"));
		calledShotUp[0] = static_cast< ::GUI::Picture*>(FindWindow("picWeapon01CalledShot_up"));
		calledShotDown[0] = static_cast< ::GUI::Picture*>(FindWindow("picWeapon01CalledShot_down"));
		weaponButtons[1] = static_cast< ::GUI::Button*>(FindWindow("btnWeapon02"));
		weaponIconsUp[1] = static_cast< ::GUI::Picture*>(FindWindow("picWeapon02Icon_up"));
		weaponIconsDown[1] = static_cast< ::GUI::Picture*>(FindWindow("picWeapon02Icon_down"));
		weaponModeUp[1] = static_cast< ::GUI::Label*>(FindWindow("lblWeapon02Mode_up"));
		weaponModeDown[1] = static_cast< ::GUI::Label*>(FindWindow("lblWeapon02Mode_down"));
		weaponAPUp[1] = static_cast< ::GUI::Label*>(FindWindow("lblWeapon02AP_up"));
		weaponAPDown[1] = static_cast< ::GUI::Label*>(FindWindow("lblWeapon02AP_down"));
		weaponMasks[1] = static_cast< ::GUI::Picture*>(FindWindow("picWeapon02Mask_up"));
		reloadIcons[0][1][0] = static_cast< ::GUI::Picture*>(FindWindow("picReloadIcon02_up"));
		reloadIcons[1][1][0] = static_cast< ::GUI::Picture*>(FindWindow("picReloadIcon02_down"));
		calledShotUp[1] = static_cast< ::GUI::Picture*>(FindWindow("picWeapon02CalledShot_up"));
		calledShotDown[1] = static_cast< ::GUI::Picture*>(FindWindow("picWeapon02CalledShot_down"));
		for (int w = 0; w < 2; ++w)
		{
			if (weaponIconsUp[w]) weaponIconsUp[w]->SetValue3B(1);
			if (weaponIconsDown[w]) weaponIconsDown[w]->SetValue3B(1);
			if (reloadIcons[0][w][0]) reloadIcons[0][w][0]->SetValue3B(1);
			if (reloadIcons[1][w][0]) reloadIcons[1][w][0]->SetValue3B(1);
			if (calledShotUp[w]) calledShotUp[w]->SetValue3B(1);
			if (calledShotDown[w]) calledShotDown[w]->SetValue3B(1);
		}
		for (int w = 0; w < 2; ++w)
		{
			if (weaponMasks[w]) { weaponMasks[w]->SetValue3B(1); weaponMasks[w]->SetColor(0xFF000000); }
		}
		for (int w = 0; w < 2; ++w)
		{
			if (reloadIcons[0][w][0])
			{
				for (int state = 0; state < 2; ++state)
				{
					::GUI::Picture* source = reloadIcons[state][w][0];
					if (source)
					{
						LONG x, y, width, height;
						std::string skin;
						source->GetAbsolutePosition(x, y); source->GetSize(width, height); source->GetSkin(skin);
						::GUI::Picture* parent = state ? weaponIconsDown[w] : weaponIconsUp[w];
						for (int i = 1; i < 16; ++i)
						{
							::GUI::Picture* picture = new ::GUI::Picture(2);
							parent->AddChild(picture);
							picture->ApplySkin(skin); picture->SetFrameVisible(0);
							picture->Resize(width, height); picture->SetAbsolutePosition(x, y - height * i);
							picture->SetValue3B(1);
							reloadIcons[state][w][i] = picture;
						}
						parent->BringChildToFront(source);
					}
				}
			}
		}
		actionPointsLabel = static_cast< ::GUI::Label*>(FindWindow("lblActionPoints"));
		actionPointsLabel->SetValue3B(0);
		actionPointsValue = static_cast< ::GUI::Label*>(FindWindow("lblActionPointsValue"));
		actionPointIcons[0] = static_cast< ::GUI::Picture*>(FindWindow("picAPIcon"));
		if (actionPointIcons[0])
		{
			LONG x, y, width, height;
			std::string skin;
			actionPointIcons[0]->GetAbsolutePosition(x, y); actionPointIcons[0]->GetSize(width, height); actionPointIcons[0]->GetSkin(skin);
			for (int i = 1; i < 30; ++i)
			{
				::GUI::Picture* picture = new ::GUI::Picture(2);
				background->AddChild(picture);
				picture->ApplySkin(skin); picture->SetFrameVisible(0);
				picture->Resize(width, i % 5 == 4 ? height + 3 : height);
				picture->SetAbsolutePosition(x + i * width, i % 5 == 4 ? y - 3 : y);
				actionPointIcons[i] = picture;
			}
		}
		SetWeaponActive(0, 1);
		SetWeaponActive(1, 0);
		BuildPopupButtons();
	}

	ActionbarInterface::~ActionbarInterface() //416A70; original GUI-base adjustment is compiler-owned.
	{
		if (healthDigits) delete healthDigits;
		if (fatigueDigits) delete fatigueDigits;
		if (evasionDigits) delete evasionDigits;
		//Original does not release PopUpButton record allocations here.
	}
	void ActionbarInterface::BeforeUpdate() //416C20; physical code, merged Hex-Rays body is wrong.
	{
		if (healthDigits) healthDigits->Update();
		if (fatigueDigits) fatigueDigits->Update();
		if (evasionDigits) evasionDigits->Update();
	}
}
