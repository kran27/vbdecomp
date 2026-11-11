#pragma once

#include <winsock2.h>
#include "game/GameInterface.hpp"
#include <string>

namespace GUI { class Window; class Picture; class Label; class Button; }
namespace JE { class Entity; }

// Actual PopUpButton417040 graph. The Actionbar destructor does not delete
// these record objects; its GUI children follow the original parent graph.
class PopUpButton
{
public:
	PopUpButton(); //417040.
	virtual ~PopUpButton(); //417290 no-op; compiler deleting entry417310.
	::GUI::Button* button; //+04.
	::GUI::Picture* up; //+08.
	::GUI::Picture* down; //+0C.
	::GUI::Picture* iconUp; //+10.
	::GUI::Picture* iconDown; //+14.
	::GUI::Label* modeUp; //+18.
	::GUI::Label* modeDown; //+1C.
	::GUI::Label* apUp; //+20.
	::GUI::Label* apDown; //+24.
	::GUI::Picture* calledShotUp; //+28.
	::GUI::Picture* calledShotDown; //+2C.
};

namespace F3
{
	class GameItem;
	class GameInventory;
	class CounterControl;

	typedef ::PopUpButton ActionbarPopUpButton;

	class ActionbarInterface : public GameInterface
	{
	public:
		explicit ActionbarInterface(const std::string& name); //41A2B0.
		virtual ~ActionbarInterface(); //416A70; three owned numeric helpers only.
		virtual int OnEvent(::GUI::Window*, ::GUI::Event*); //41CA00; root.
		virtual void BeforeUpdate(); //416C20, actual entry despite merged IDA function.
		//BeforeDraw inherits actual shared404B70 no-op.41BFA0 is an event0 tick.
		void Tick(); //41BFA0; root, called only from actual OnEvent branch.
		void BuildPopupButtons(); //4188A0.
		void SelectEntity(DWORD id); //41C9D0; root.
		void RefreshEntity(DWORD id); //41C910; root.
		GameInventory* GetInventory(); //416DF0; root.
		BYTE IsWeaponActive(DWORD index); //416E80; weapons sibling, AL/boolean result.
		void GetWindowRectangle(LONG& x, LONG& y, LONG& width, LONG& height); //416F80; root.
		void RefreshCompanions(); //416B80; root.
		void SetCompanionMode(DWORD mode); //4172D0.
		void CycleCompanionMode(); //4172E0.
		void HideCompanions(); //416B20; root.
		void PrimaryWeaponClick(DWORD index); //41C790; root.
		void ShowAttackModes(DWORD index); //419C70; root.
		void SetEvasionAP(DWORD index); //4186F0; root.
		void SetPartyHealth(LONG index, LONG percent); //416C60; stats sibling.
		void RefreshHealthDigits(BYTE force); //416CD0.
		void RefreshHealthBar(); //416D80.
		void SetPartyMember(LONG index, DWORD entity); //417370.
		void SetSelectedPartyMember(LONG index); //417A50.
		void SetActionPoints(LONG current, LONG maximum, LONG reserved, BYTE force); //418360.
		void RefreshActionPoints(BYTE force); //418820.
		void ClearStatusButtons(); //416FB0.
		void RefreshInventory(); //41C4C0; weapons sibling.
		void SetAttackMode(DWORD index, LONG step); //41C040.
		void RefreshAttackMode(DWORD index); //41C2E0.
		void WeaponIcon(std::string& output, GameItem* item); //417BC0; ordinary member, RET8.
		void SetWeaponIcon(DWORD index, DWORD item, DWORD owner); //417CF0.
		void SetWeaponText(DWORD index, const std::string& text); //416E30.
		void SetWeaponTextReference(DWORD index, DWORD reference); //417450.
		void SetWeaponActive(DWORD index, BYTE active); //416EB0.
		void SetWeaponAP(DWORD index, LONG value); //417F50.
		void SetAmmoPercent(LONG index, LONG percent); //418040.
		void SetAmmoAmount(LONG index, LONG amount, LONG maximum); //41C000.
		void SetCalledShot(DWORD index, BYTE value); //416FF0.

		::GUI::Window* rootWindow; //+04.
		::GUI::Picture* background; //+08.
		DWORD value0C; //+0C, ctor1.
		::GUI::Button* sneakButton; //+10.
		::GUI::Button* inventoryButton; //+14.
		::GUI::Button* pipBoyButton; //+18.
		::GUI::Button* useSkillButton; //+1C.
		::GUI::Button* characterButton; //+20.
		::GUI::Button* optionsButton; //+24.
		::GUI::Picture* partyPictures[6]; //+28.
		::GUI::Label* partyNames[6]; //+40.
		::GUI::Picture* partyHealth[6]; //+58.
		::GUI::Picture* partyFatigue[6]; //+70.
		DWORD entityID, otherEntityID; //+88/+8C, ctor0.
		CounterControl* healthDigits; //+90, owned.
		CounterControl* fatigueDigits; //+94, owned.
		CounterControl* evasionDigits; //+98, owned.
		::GUI::Button* weaponButtons[2]; //+9C.
		::GUI::Picture* weaponIconsUp[2]; //+A4.
		::GUI::Picture* weaponIconsDown[2]; //+AC.
		::GUI::Label* weaponModeUp[2]; //+B4.
		::GUI::Label* weaponModeDown[2]; //+BC.
		::GUI::Label* weaponAPUp[2]; //+C4.
		::GUI::Label* weaponAPDown[2]; //+CC.
		::GUI::Picture* weaponMasks[2]; //+D4.
		::GUI::Picture* calledShotUp[2]; //+DC.
		::GUI::Picture* calledShotDown[2]; //+E4.
		::GUI::Picture* reloadIcons[2][2][16]; //+EC..1E8: state,weapon,count.
		::GUI::Label* actionPointsLabel; //+1EC.
		::GUI::Label* actionPointsValue; //+1F0.
		::GUI::Picture* actionPointIcons[30]; //+1F4..268.
		LONG cachedCurrentAP, cachedMaximumAP, cachedReservedAP; //+26C/270/274, ctor-1.
		ActionbarPopUpButton* popups[6]; //+278..28C; ctor allocates, original dtor omits release.
		DWORD popupValue290, popupValue294; //+290/294, ctor0.
	private:
		ActionbarInterface(const ActionbarInterface&);
		ActionbarInterface& operator=(const ActionbarInterface&);
	};
}
