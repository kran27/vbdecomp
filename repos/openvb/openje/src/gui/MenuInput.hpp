#pragma once

#include "MenuLayout.hpp"

namespace JE
{
	namespace GUI
	{
		// The proprietary six-word event API. These are normal native values;
		// fixed original object/list storage exists only in comparison transport.
		struct MenuEvent
		{
			int type, x, y;
			DWORD buttons;
			int wheel;
			DWORD modifiers;
		};

		struct MenuMouseState
		{
			int x, y;
			DWORD buttons;
			int wheel;
		};

		struct MenuInputState
		{
			MenuInputState() : dirty(0), blocked(0), scaleX(1), scaleY(1),
				hoveredOwner(NULL), hoveredNode(NULL)
			{
				mouse.x = mouse.y = mouse.wheel = 0;
				mouse.buttons = 0;
				for (unsigned i = 0; i < 3; ++i) pressed[i] = NULL;
			}
			BYTE dirty; // Manager+4, set by graph mutation4B2AB0.
			BYTE blocked;
			float scaleX, scaleY;
			MenuMouseState mouse;
			MenuLayout* hoveredOwner;
			MenuNode* hoveredNode;
			MenuNode* pressed[3];
			std::list<MenuLayout*> interfaces;
			std::list<MenuLayout*> modal;
			std::string tooltip; // Canonical Manager+112 string, shared owner callbacks.
			// Capture always resides in the existing g_PointerCapture.
		};

		class MenuInputServices
		{
		public:
			virtual ~MenuInputServices() {}
			virtual MenuLayout* Owner(MenuNode& node);
			virtual MenuNode* Parent(MenuNode& node);
			virtual MenuNode* HitNode(MenuNode& node, int x, int y, int overrideSelf);
			virtual MenuNode* HitLayout(MenuLayout& layout, int x, int y, int overrideSelf);
			virtual int Contains(MenuNode& node, int x, int y);
			virtual BYTE SuppressSelf(MenuNode& node);
			virtual bool ContainsDescendant(MenuNode& node, MenuNode* candidate);
			virtual void SetOwner(MenuInputState& state, MenuNode& node, MenuLayout* owner);
			virtual void SetParent(MenuInputState& state, MenuNode& node, MenuNode* parent);
			virtual void AttachRoot(MenuInputState& state, MenuLayout& owner, MenuNode& node);
			virtual void AttachChild(MenuInputState& state, MenuNode& parent, MenuNode& child);
			virtual void RemoveRoot(MenuInputState& state, MenuLayout& owner, MenuNode& node);
			virtual void RemoveChild(MenuInputState& state, MenuNode& parent, MenuNode& child);
			virtual void Position(MenuNode& node, int& x, int& y);
			virtual int OwnerEvent(MenuLayout& owner, MenuNode& node, MenuEvent& event);
			virtual int NodeEvent(MenuInputState& state, MenuNode& node, MenuEvent& event);
			virtual void RefreshButton(MenuNode& node, const std::string& configuration) = 0;
			virtual int ConvertCoordinate(double value);
		};

		MenuNode* HitMenuNode(MenuNode& node, int x, int y, int overrideSelf,
			MenuInputServices& services); //0x4AF8F0.
		MenuNode* HitMenuLayout(MenuLayout& layout, int x, int y, int overrideSelf,
			MenuInputServices& services); //0x4952B0.
		MenuNode* FindMenuTarget(MenuInputState& state, int x, int y,
			MenuInputServices& services); //0x4B2C20.
		int DispatchMenuEvent(MenuInputState& state, MenuEvent& event,
			MenuLayout* owner, MenuNode* node, MenuInputServices& services); //0x4B2F30.
		bool ProcessMenuMouse(MenuInputState& state, const MenuMouseState& mouse,
			DWORD modifiers, MenuInputServices& services); //0x4B3040.
		bool MenuButtonEnabled(const MenuNode& node); //0x4A6D70.
		int HandleMenuNodeEvent(MenuInputState& state, MenuNode& node,
			MenuEvent& event, MenuInputServices& services); //0x4A6DA0 /0x4AFFA0.
		int HandleMenuBaseEvent(const MenuNode& node, const MenuEvent& event); //0x4AFFA0.
		bool MenuContainsDescendant(MenuNode& node, MenuNode* candidate,
			MenuInputServices& services); //0x4AF8A0.
		void SetMenuOwner(MenuInputState& state, MenuNode& node, MenuLayout* owner,
			MenuInputServices& services); //0x4AF320.
		void SetMenuParent(MenuInputState& state, MenuNode& node, MenuNode* parent,
			MenuInputServices& services); //0x4AF390.
		void AttachMenuRoot(MenuInputState& state, MenuLayout& owner, MenuNode& node,
			MenuInputServices& services); //0x4959B0.
		void AttachMenuChild(MenuInputState& state, MenuNode& parent, MenuNode& child,
			MenuInputServices& services); //0x4B0CA0, not serializer4AFD30.
		void RemoveMenuRoot(MenuInputState& state, MenuLayout& owner, MenuNode& node,
			MenuInputServices& services); //0x4954A0.
		void RemoveMenuChild(MenuInputState& state, MenuNode& parent, MenuNode& child,
			MenuInputServices& services); //0x4AFF30.
	}
}
