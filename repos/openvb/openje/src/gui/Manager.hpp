#pragma once

#include <windows.h>
#include <list>
#include <map>
#include <string>
#include "TextureMask.hpp"

class Gfx_CamCtrl_Orbit;
class Gfx_Viewport;

namespace GUI
{
	class Window;
	class Interface;
	struct Event;
	class Skin;
	class Font;
	class CPktManager;
	class cls_0x4b6910; // Original nonvirtual 40-byte storage owner.
	typedef ::Gfx_Viewport cls_0x4c0190; // Same actual view owner.
	struct TextureMask;

	// Manager+0x2C. These are mouse values, not a RECT. Reset leaves wheel intact.
	struct MouseState
	{
		LONG x, y;
		DWORD buttons;
		LONG wheel;
	};

	class Manager
	{
	public:
		typedef BYTE (__cdecl *StringCallback)(DWORD reference, std::string& output);
		Manager(); // 0x4B5640; actual storage, packets, viewport and Orbit owners.
		virtual ~Manager(); // 0x4B5280; original table0x66F2CC has this one slot.
		void Cleanup(); //0x4B4920; owns Interface/Skin/Font cleanup and resets packets/state.
		Skin* GetSkin(const std::string& name); // 0x4B5C30; exact filename cache, borrowed result.
		Font* GetFont(const std::string& name, DWORD flags, LONG pointSize, DWORD charset); //4B58D0.
		BYTE ResolveString(DWORD reference, std::string& output); //4B4180.
		void SetStringCallback(StringCallback callback); //4B2AC0; borrowed callback only.
		void SetTooltip(const std::string& text); //4B4160; assigns the actual tooltip member.
		BYTE WriteClipboardText(const std::string& text); //4B2CB0; borrowed text, original clipboard policy.

		void GetResolution(DWORD& width, DWORD& height); // 0x4B28A0.
		void SetResolution(LONG width, LONG height); //4B4CE0, actual viewport/scaling setup.
		void RefreshPositions(); //4B2C80; forward ordinary interfaces, including hidden.
		void GetScale(float& x, float& y); // 0x4B28C0.
		void GetMouseState(MouseState& state); // 0x4B28E0.
		void ResetMouseState(); // 0x4B2900; does not write wheel.
		Window* CreateWindowByType(DWORD type); //4B2910; ordinary Manager factory, borrowed receiver.
		Window* FindCursorWindow(LONG x, LONG y); //4B2C20; scaled hit test, capture first.
		BYTE ProcessCursor(HWND window); //4B2D40; raw client coordinates to owner+14.
		TextureMask* GetTextureMask(DWORD handle); // 0x4B2AA0; borrowed lazy cache result.
		void MarkDepthDirty(); // 0x4B2AB0.
		BOOL HasModal(); // 0x4B2C10; full EAX zero/one result.
		void PushModal(Interface* value); //4B4F00; capture/hover prefix, then prepend borrowed value.
		void RemoveModal(Interface* value); // 0x4B2FE0; removes every equal borrowed value.
		Interface* FrontModal(); // 0x4B3000; empty returns NULL, otherwise the borrowed front value.
		void BringToFront(Interface* value); // 0x4B4E60; reorders an existing ordinary interface.
		const char* GetFragmentType(DWORD index); // 0x4B2A90; unchecked five-entry type-name table.
		BYTE SetCapture(Window* next); // 0x4B2AD0; previous target notified before final store.
		void ForgetWindow(Window* window); // 0x4B34B0; H-RETURN, inspected destructor ignores result.
		void RegisterInterface(Interface* value); // 0x4B4DD0; erase first equal, then always prepend.
		void UnregisterInterface(Interface* value); // 0x4B3430; capture/list/hover ordering, H-RETURN.
		void Render(); // 0x4B3690; borrowed view queue, reverse visible Interface draw.
		void AddViewport(Gfx_Viewport* viewport); //4B4D90; append borrowed view for the current render.
		void Update(); // 0x4B37F0; pending deletion, shared tree event, reverse Update, required depth.
		int DispatchEvent(Event* event, Interface* owner, Window* window); // 0x4B2F30; raw modal front, owner first.
		int DispatchTree(Event* event, Interface* owner, Window* window); // 0x4B3500; exact-one child stop, full EAX.
		void AssignDepth(); // 0x4B35F0; dirty-gated x87 depth assignment.
		DWORD CountDepthSlots(); // 0x4B3570; includes hidden ordinary Interfaces.
		DWORD CountWindowDepthSlots(Window* window); // 0x4B2F80; wrapped subtree count.
		// Actual required mouse processor0x4B3040, AL result forwarded by581230.
		BYTE ProcessMouse(MouseState* state, DWORD flags);
		BYTE ProcessKey(LONG key, BYTE down, DWORD modifiers); // 0x4B40A0; event17/18, exact-one handled result.
		BYTE ProcessCharacter(char character, DWORD modifiers); // 0x4B40F0; CRT graphical character or space.
		int DispatchInterfaceEvent(Event* event, Interface* owner); // 0x4B3780; front-forward roots, exact-one stop.
		int DispatchInputEvent(Event* event); // 0x4B3920; modal consumption, then ordinary visible interfaces.

		BYTE depthDirty; // +04; padding+05..07 remains unwritten.
		Interface* hoveredOwner; // +08; constructor does not initialize this field.
		Window* hoveredWindow; // +0C.
		Window* pressedLeft; // +10.
		Window* pressedRight; // +14.
		Window* pressedMiddle; // +18.
		std::list<Interface*> interfaces; // +1C; actual VC7 container.
		BYTE inputBlocked; // +28; padding+29..2B remains unwritten.
		MouseState mouse; // +2C..3B; fourth word survives ResetMouseState.
		std::map<std::string, Skin*> skins; // +3C; 48-byte original nodes.
		TextureMaskCache textureMasks; // +48; actual map-backed lazy mask owner.
		std::list<Font*> fonts; // +54.
		DWORD resolutionWidth, resolutionHeight; // +60/+64.
		float scaleX, scaleY; // +68/+6C.
		std::string tooltip; // +70; actual VC7 string.
		cls_0x4c0190* view; // +8C.
		Gfx_CamCtrl_Orbit* camera; // +90.
		std::list<Gfx_Viewport*> entries148; // +94; borrowed AddViewport inputs, nodes cleared by Render.
		std::list<Interface*> modal; // +A0.
		StringCallback callback172; // +AC, optional actual string callback4B4180.
		CPktManager* packets; // +B0.
		cls_0x4b6910* storage; // +B4.
		Window* capturedWindow; // +B8; canonical legacy capture member.
	};

	// Original static object at 0x707E48.


	extern Manager g_Manager;
}
