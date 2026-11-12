#pragma once

#include "gui/Win32Input.hpp"
#include <string>
#include <list>
#include <cstddef>

namespace GUI { class Interface; class Picture; class Window; class Label; }

namespace JE
{
	class CursorManager
	{
	public:
		CursorManager(); //591700.
		virtual ~CursorManager(); //5916A0, borrows the GUI hierarchy.
		virtual void OnCleanup(); //404B70.
		virtual void Initialize(); //591780, real cursor Interface/Picture.
		virtual void ShowSystemCursor(); //5915D0.
		virtual void UpdateFrame(); //5915E0, original no-argument slot+10.
		virtual void RequestImage(const char* name, LONG x, LONG y, BYTE center); //5919D0.
		virtual void GetImage(std::string& output); //591B60.
		virtual BYTE ReleaseInput(const Input::InputEvent& event); //591690, actual event-only slot+1C.
		virtual void OnUnused(); //404B70.
		void SetVisible(BYTE visible); //591670 ->Picture::SetHidden(!visible).
		::GUI::Interface* cursorInterface; //+04, GUI hierarchy lifetime.
		::GUI::Picture* picture; //+08, GUI hierarchy lifetime.
		LONG hotspotX, hotspotY; //+0C/+10.
		std::string imageName; //+14.
	};
}

namespace F3
{
	class GameCursorManager : public JE::CursorManager
	{
	public:
		GameCursorManager(); //436330.
		virtual ~GameCursorManager(); //436270.
		virtual void Initialize(); //437BB0; base cursor, tooltip children, then default state.
		virtual void ShowSystemCursor(); //4361E0, qualified base forwarding override.
		void InitializeDefaultCursorState(); //Explicit437BC0 suffix, excludes4364C0 quickslot.
		virtual void UpdateFrame(); //437BE0, actual live Client/Window owners.
		virtual void RequestImage(const char* name, LONG x, LONG y, BYTE center); //437AB0.
		virtual void GetImage(std::string& output); //437B20.
		virtual BYTE ReleaseInput(const JE::Input::InputEvent& event); //437D80, actual event-only slot+1C.
		virtual void OnUnused(); //437B40, reset pending/saved/context/drag state.
		void SetContext(DWORD value); //436AD0, calls actual no-argument virtual+10.
		void BeginItemDrag(DWORD item, DWORD owner); //437940, actual no-snapshot route.
		void ClearDragChoices(); //436300.
		void AppendDragChoices(const std::list<DWORD>& source); //437E70.
		BYTE CancelSpecialContext(); //437B90, refreshes actual cursor only in contexts15/16.
		DWORD context, subcontext, draggedItemID, draggedOwnerID; //+30..3C, ownerID unwritten.
		std::list<DWORD> choices; //+40, real normal nodes.
		BYTE pendingImage; //+4C; padding4D..4F is constructor-unwritten.
		std::string pendingName; //+50.
		LONG pendingX, pendingY; //+6C/+70.
		BYTE pendingCenter; //+74.
		std::string savedName; //+78.
		LONG savedX, savedY; //+94/+98, unwritten by436330.
		::GUI::Window* tooltipWindow; //+9C.
		::GUI::Label* tooltipLabel; //+A0; actual quickslot initializer4364C0.
		BYTE tooltipPending; //+A4; paddingA5..A7 is constructor-unwritten.
		DWORD tooltipEntityID, tooltipTime; //+A8/+AC.
	private:
		void InitializeTooltip(); //4364C0, actual Window/Label children under cursor Picture.
		void QueueContextImage(DWORD value, LONG x, LONG y); //4361F0.
		void SetSubcontext(DWORD value); //436210.
		void UpdateTooltip(LONG entityID); //436840, incoming stack word also receives Window height.
		BYTE LeftPress(LONG x, LONG y); //436230, folded press prototype; physical RET8.
		BYTE RightPress(LONG x, LONG y); //Same folded436230, separate native press branch.
		BYTE LeftRelease(LONG x, LONG y); //436B40, captures published Client once.
		BYTE RightRelease(LONG x, LONG y); //436FC0, reloads published Client for the menu call.
	};
}

#if defined(_MSC_VER) && _MSC_VER == 1300
namespace JE
{
	typedef char CursorOriginalSize[(sizeof(CursorManager) == 0x30) ? 1 : -1];
	typedef char CursorOriginalInterface[(offsetof(CursorManager, cursorInterface) == 0x04) ? 1 : -1];
	typedef char CursorOriginalPicture[(offsetof(CursorManager, picture) == 0x08) ? 1 : -1];
	typedef char CursorOriginalHotspotX[(offsetof(CursorManager, hotspotX) == 0x0C) ? 1 : -1];
	typedef char CursorOriginalHotspotY[(offsetof(CursorManager, hotspotY) == 0x10) ? 1 : -1];
	typedef char CursorOriginalImageName[(offsetof(CursorManager, imageName) == 0x14) ? 1 : -1];
}
namespace F3
{
	typedef char GameCursorOriginalSize[(sizeof(GameCursorManager) == 0xB0) ? 1 : -1];
	typedef char GameCursorOriginalContext[(offsetof(GameCursorManager, context) == 0x30) ? 1 : -1];
	typedef char GameCursorOriginalSubcontext[(offsetof(GameCursorManager, subcontext) == 0x34) ? 1 : -1];
	typedef char GameCursorOriginalDraggedItem[(offsetof(GameCursorManager, draggedItemID) == 0x38) ? 1 : -1];
	typedef char GameCursorOriginalDraggedOwner[(offsetof(GameCursorManager, draggedOwnerID) == 0x3C) ? 1 : -1];
	typedef char GameCursorOriginalChoices[(offsetof(GameCursorManager, choices) == 0x40) ? 1 : -1];
	typedef char GameCursorOriginalPending[(offsetof(GameCursorManager, pendingImage) == 0x4C) ? 1 : -1];
	typedef char GameCursorOriginalPendingName[(offsetof(GameCursorManager, pendingName) == 0x50) ? 1 : -1];
	typedef char GameCursorOriginalPendingX[(offsetof(GameCursorManager, pendingX) == 0x6C) ? 1 : -1];
	typedef char GameCursorOriginalPendingY[(offsetof(GameCursorManager, pendingY) == 0x70) ? 1 : -1];
	typedef char GameCursorOriginalPendingCenter[(offsetof(GameCursorManager, pendingCenter) == 0x74) ? 1 : -1];
	typedef char GameCursorOriginalSavedName[(offsetof(GameCursorManager, savedName) == 0x78) ? 1 : -1];
	typedef char GameCursorOriginalSavedX[(offsetof(GameCursorManager, savedX) == 0x94) ? 1 : -1];
	typedef char GameCursorOriginalSavedY[(offsetof(GameCursorManager, savedY) == 0x98) ? 1 : -1];
	typedef char GameCursorOriginalTooltipWindow[(offsetof(GameCursorManager, tooltipWindow) == 0x9C) ? 1 : -1];
	typedef char GameCursorOriginalTooltipLabel[(offsetof(GameCursorManager, tooltipLabel) == 0xA0) ? 1 : -1];
	typedef char GameCursorOriginalTooltipPending[(offsetof(GameCursorManager, tooltipPending) == 0xA4) ? 1 : -1];
	typedef char GameCursorOriginalTooltipEntity[(offsetof(GameCursorManager, tooltipEntityID) == 0xA8) ? 1 : -1];
	typedef char GameCursorOriginalTooltipTime[(offsetof(GameCursorManager, tooltipTime) == 0xAC) ? 1 : -1];
	typedef char GameCursorOriginalPendingByte[(sizeof(((GameCursorManager*)0)->pendingImage) == 1) ? 1 : -1];
	typedef char GameCursorOriginalCenterByte[(sizeof(((GameCursorManager*)0)->pendingCenter) == 1) ? 1 : -1];
	typedef char GameCursorOriginalTooltipByte[(sizeof(((GameCursorManager*)0)->tooltipPending) == 1) ? 1 : -1];
}
#endif
