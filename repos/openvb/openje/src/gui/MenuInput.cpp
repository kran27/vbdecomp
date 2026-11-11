#include "MenuInput.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <float.h>
#include <stdexcept>

namespace JE
{
	namespace GUI
	{
		namespace
		{
			int Bits(DWORD value)
			{
				return value <= 0x7FFFFFFFu ? static_cast<int>(value)
					: -1 - static_cast<int>(~value);
			}

			MenuNode* CapturedNode()
			{
				// CaptureTarget is the shared source interface. A foreign target is
				// outside this bounded Window/Button/Picture graph, not a new owner.
				MenuNode* node = dynamic_cast<MenuNode*>(g_PointerCapture.pTarget);
				if (g_PointerCapture.pTarget && !node)
					throw std::logic_error("GUI capture target is outside the recovered menu node graph");
				return node;
			}
			int DispatchTypedEvent(MenuInputState& state, MenuEvent& event,
				MenuInputServices& services, int type, MenuLayout* owner, MenuNode* node)
			{
				event.type = type;
				return DispatchMenuEvent(state, event, owner, node, services);
			}

		}

		MenuLayout* MenuInputServices::Owner(MenuNode& node) { return node.owner; }
		MenuNode* MenuInputServices::Parent(MenuNode& node) { return node.parent; }
		BYTE MenuInputServices::SuppressSelf(MenuNode& node) { return node.field0x3B; }
		int MenuInputServices::Contains(MenuNode& node, int x, int y)
		{
			const POINT point = {x, y};
			return ::PtInRect(&node.rectangle, point);
		}
		MenuNode* MenuInputServices::HitNode(MenuNode& node, int x, int y, int overrideSelf)
		{ return HitMenuNode(node, x, y, overrideSelf, *this); }
		MenuNode* MenuInputServices::HitLayout(MenuLayout& layout, int x, int y, int overrideSelf)
		{ return HitMenuLayout(layout, x, y, overrideSelf, *this); }
		bool MenuInputServices::ContainsDescendant(MenuNode& node, MenuNode* candidate)
		{ return MenuContainsDescendant(node, candidate, *this); }
		void MenuInputServices::SetOwner(MenuInputState& state, MenuNode& node, MenuLayout* owner)
		{ SetMenuOwner(state, node, owner, *this); }
		void MenuInputServices::SetParent(MenuInputState& state, MenuNode& node, MenuNode* parent)
		{ SetMenuParent(state, node, parent, *this); }
		void MenuInputServices::AttachRoot(MenuInputState& state, MenuLayout& owner, MenuNode& node)
		{ AttachMenuRoot(state, owner, node, *this); }
		void MenuInputServices::AttachChild(MenuInputState& state, MenuNode& parent, MenuNode& child)
		{ AttachMenuChild(state, parent, child, *this); }
		void MenuInputServices::RemoveRoot(MenuInputState& state, MenuLayout& owner, MenuNode& node)
		{ RemoveMenuRoot(state, owner, node, *this); }
		void MenuInputServices::RemoveChild(MenuInputState& state, MenuNode& parent, MenuNode& child)
		{ RemoveMenuChild(state, parent, child, *this); }
		void MenuInputServices::Position(MenuNode& node, int& x, int& y)
		{ x = node.rectangle.left; y = node.rectangle.top; }
		int MenuInputServices::ConvertCoordinate(double value)
		{
			if (!::_finite(value) || (value < 0 ? std::ceil(value) : std::floor(value)) < (std::numeric_limits<int>::min)()
				|| (value < 0 ? std::ceil(value) : std::floor(value)) > (std::numeric_limits<int>::max)())
				throw std::logic_error("Unresolved GUI floating conversion outside finite signed32 domain");
			return static_cast<int>(value);
		}
		int MenuInputServices::OwnerEvent(MenuLayout& owner, MenuNode& node, MenuEvent& event)
		{
			if (!owner.eventOwner)
				throw std::logic_error("Unrecovered interface event owner callback");
			return owner.eventOwner->HandleEvent(node, event);
		}
		int MenuInputServices::NodeEvent(MenuInputState& state, MenuNode& node, MenuEvent& event)
		{ return HandleMenuNodeEvent(state, node, event, *this); }

		MenuNode* HitMenuNode(MenuNode& node, int x, int y, int overrideSelf,
			MenuInputServices& services)
		{
			if (node.flags[0]) return NULL;
			for (std::list<MenuNode*>::iterator child = node.children.begin(); child != node.children.end(); ++child)
			{
				MenuNode* hit = services.HitNode(**child, x, y, overrideSelf);
				if (hit) return hit;
			}
			if (services.SuppressSelf(node) && !static_cast<BYTE>(overrideSelf)) return NULL;
			return static_cast<BYTE>(services.Contains(node, x, y)) ? &node : NULL;
		}

		MenuNode* HitMenuLayout(MenuLayout& layout, int x, int y, int overrideSelf,
			MenuInputServices& services)
		{
			if (layout.field0x20) return NULL;
			for (std::list<MenuNode*>::iterator node = layout.roots.begin(); node != layout.roots.end(); ++node)
			{
				MenuNode* hit = services.HitNode(**node, x, y, overrideSelf);
				if (hit) return hit;
			}
			return NULL;
		}

		MenuNode* FindMenuTarget(MenuInputState& state, int x, int y, MenuInputServices& services)
		{
			const int scaledX = services.ConvertCoordinate(static_cast<double>(x) / state.scaleX);
			const int scaledY = services.ConvertCoordinate(static_cast<double>(y) / state.scaleY);
			if (g_PointerCapture.pTarget) return CapturedNode();
			for (std::list<MenuLayout*>::iterator owner = state.interfaces.begin(); owner != state.interfaces.end(); ++owner)
			{
				MenuNode* hit = services.HitLayout(**owner, scaledX, scaledY, 0);
				if (hit) return hit;
			}
			return NULL;
		}

		int DispatchMenuEvent(MenuInputState& state, MenuEvent& event, MenuLayout* owner,
			MenuNode* node, MenuInputServices& services)
		{
			if (!owner) return 0;
			if (!state.modal.empty() && owner != state.modal.front()) return 0;
			if (!node) throw std::logic_error("Original GUI dispatcher dereferences a NULL node");
			const int result = services.OwnerEvent(*owner, *node, event);
			return result ? result : services.NodeEvent(state, *node, event);
		}

		bool MenuButtonEnabled(const MenuNode& node)
		{ return node.button0xD1C != 3 && node.button0xD1C != 7; }

		int HandleMenuBaseEvent(const MenuNode& node, const MenuEvent& event)
		{
			if (event.type < 0)
				throw std::logic_error("Original GUI event string index is negative");
			return static_cast<std::size_t>(event.type) < node.strings.size()
				&& !node.strings[static_cast<std::size_t>(event.type)].empty();
		}

		int HandleMenuNodeEvent(MenuInputState& state, MenuNode& node, MenuEvent& event,
			MenuInputServices& services)
		{
			if (node.type != 1) return HandleMenuBaseEvent(node, event);
			if (!MenuButtonEnabled(node)) return 0;
			bool refresh = true;
			switch (event.type)
			{
			case 1:
				if (!(event.buttons & 1)) node.button0xD1C = (node.button0xD20 != 0) * 4 + 1;
				else if (state.pressed[0] == &node) node.button0xD1C = (node.button0xD20 != 0) * 4 + 2;
				break;
			case 3: node.button0xD1C = (node.button0xD20 != 0) * 4; break;
			case 4: case 7: node.button0xD1C = (node.button0xD20 != 0) * 4 + 2; break;
			case 5: node.button0xD1C = (node.button0xD20 != 0) * 4 + 1; break;
			case 12: case 15:
				if (node.button0xD21) node.button0xD1C = (node.button0xD20 != 0) * 4 + 2;
				else refresh = false;
				break;
			case 13:
				if (node.button0xD21) node.button0xD1C = (node.button0xD20 != 0) * 4 + 1;
				else refresh = false;
				break;
			default: refresh = false;
			}
			if (refresh) services.RefreshButton(node, node.configuration);
			return HandleMenuBaseEvent(node, event);
		}

		bool ProcessMenuMouse(MenuInputState& state, const MenuMouseState& mouse,
			DWORD modifiers, MenuInputServices& services)
		{
			if (state.blocked) return false;
			const MenuMouseState previous = state.mouse;
			state.mouse = mouse;
			const int scaledX = services.ConvertCoordinate(static_cast<double>(state.mouse.x) / state.scaleX);
			const double yValue = static_cast<double>(state.mouse.y) / state.scaleY;
			state.mouse.x = scaledX;
			const int scaledY = services.ConvertCoordinate(yValue);
			state.mouse.y = scaledY;
			const bool doubleClick = (state.mouse.buttons & 0xFFFF0000u) != 0;
			if (doubleClick)
			{
				const DWORD raw = state.mouse.buttons;
				state.mouse.buttons = (raw >> 16) | ((raw & 0x80000000u) ? 0xFFFF0000u : 0);
			}
			MenuEvent event = {0, scaledX, scaledY, state.mouse.buttons, state.mouse.wheel, modifiers};
			MenuLayout* owner = NULL;
			MenuNode* target = NULL;
			if (g_PointerCapture.pTarget)
			{
				MenuNode* ancestor = CapturedNode();
				while (ancestor)
				{
					owner = services.Owner(*ancestor);
					if (owner) break;
					ancestor = services.Parent(*ancestor);
				}
				if (owner) target = CapturedNode(); // Reload after virtual ancestry callbacks.
			}
			else if (!state.modal.empty())
			{
				owner = state.modal.front();
				target = services.HitLayout(*owner, scaledX, scaledY, 0);
				if (!target)
				{
					if (owner->roots.empty())
						throw std::logic_error("Original modal mouse fallback dereferences an empty root list");
					target = owner->roots.front();
				}
			}
			else
			{
				for (std::list<MenuLayout*>::iterator entry = state.interfaces.begin(); entry != state.interfaces.end(); ++entry)
				{
					owner = *entry;
					target = services.HitLayout(*owner, state.mouse.x, state.mouse.y, 0);
					if (target) break;
				}
			}
			bool entered = false;

			if (g_PointerCapture.pTarget)
			{
				if (target && owner && services.HitLayout(*owner, state.mouse.x, state.mouse.y, 0) == target)
				{
					if (!state.hoveredNode) { DispatchTypedEvent(state, event, services, 1, owner, target); entered = true; }
					state.hoveredNode = target;
					state.hoveredOwner = owner;
				}
				else
				{
					if (state.hoveredNode) DispatchTypedEvent(state, event, services, 3, state.hoveredOwner, state.hoveredNode);
					state.hoveredNode = NULL;
					state.hoveredOwner = NULL;
				}
			}
			else
			{
				if (state.hoveredNode && target != state.hoveredNode)
					DispatchTypedEvent(state, event, services, 3, state.hoveredOwner, state.hoveredNode);
				if (target && target != state.hoveredNode) { DispatchTypedEvent(state, event, services, 1, owner, target); entered = true; }
				state.hoveredNode = target;
				state.hoveredOwner = owner;
			}
			const DWORD capturedButtons = state.mouse.buttons;
			const DWORD left = capturedButtons & 1;
			for (unsigned pressedButton = 0; pressedButton < 3; ++pressedButton)
			{
				const DWORD mask = 1u << pressedButton;
				if ((capturedButtons & mask) != (previous.buttons & mask) && (capturedButtons & mask))
					state.pressed[pressedButton] = target;
			}
			if (!target) return entered;
			for (unsigned button = 0; button < 3; ++button)
			{
				const DWORD mask = 1u << button;
				const DWORD current = button ? state.mouse.buttons & mask : left;
				if (current == (previous.buttons & mask)) continue;
				const int down = 4 + static_cast<int>(button) * 4;
				if (current) DispatchTypedEvent(state, event, services, down + (doubleClick ? 3 : 0), owner, target);
				else
				{
					if (state.pressed[button] == target)
					{
						DispatchTypedEvent(state, event, services, down + 1, owner, target);
						DispatchTypedEvent(state, event, services, down + 2, owner, target);
					}
					else DispatchTypedEvent(state, event, services, down + 1, owner, target);
				}
			}
			if (state.mouse.x != previous.x || state.mouse.y != previous.y) DispatchTypedEvent(state, event, services, 2, owner, target);
			if (state.mouse.wheel) DispatchTypedEvent(state, event, services, 16, owner, target);
			return true;
		}

		bool MenuContainsDescendant(MenuNode& node, MenuNode* candidate, MenuInputServices& services)
		{
			for (std::list<MenuNode*>::iterator child = node.children.begin(); child != node.children.end(); ++child)
				if (*child == candidate || services.ContainsDescendant(**child, candidate)) return true;
			return false;
		}

		void RemoveMenuRoot(MenuInputState& state, MenuLayout& owner, MenuNode& node,
			MenuInputServices& services)
		{
			std::list<MenuNode*>::iterator entry = std::find(owner.roots.begin(), owner.roots.end(), &node);
			if (entry == owner.roots.end()) return;
			owner.roots.erase(entry);
			services.SetOwner(state, node, NULL);
		}

		void RemoveMenuChild(MenuInputState& state, MenuNode& parent, MenuNode& child,
			MenuInputServices& services)
		{
			std::list<MenuNode*>::iterator entry = std::find(parent.children.begin(), parent.children.end(), &child);
			if (entry == parent.children.end()) return;
			parent.children.erase(entry);
			services.SetParent(state, child, NULL);
		}

		void AttachMenuRoot(MenuInputState& state, MenuLayout& owner, MenuNode& node,
			MenuInputServices& services)
		{
			if (services.Owner(node) != &owner) services.SetOwner(state, node, &owner);
			std::list<MenuNode*>::iterator entry = std::find(owner.roots.begin(), owner.roots.end(), &node);
			if (entry != owner.roots.end()) owner.roots.erase(entry);
			owner.roots.push_front(&node);
		}

		void AttachMenuChild(MenuInputState& state, MenuNode& parent, MenuNode& child,
			MenuInputServices& services)
		{
			if (services.Parent(child) != &parent) services.SetParent(state, child, &parent);
			std::list<MenuNode*>::iterator entry = std::find(parent.children.begin(), parent.children.end(), &child);
			if (entry != parent.children.end()) parent.children.erase(entry);
			parent.children.push_front(&child);
		}

		void SetMenuOwner(MenuInputState& state, MenuNode& node, MenuLayout* owner,
			MenuInputServices& services)
		{
			if (node.owner) services.RemoveRoot(state, *node.owner, node);
			if (node.parent) services.SetParent(state, node, NULL);
			node.owner = owner;
			if (owner) services.AttachRoot(state, *owner, node);
			node.relativeRectangle = node.rectangle;
			state.dirty = 1;
		}

		void SetMenuParent(MenuInputState& state, MenuNode& node, MenuNode* parent,
			MenuInputServices& services)
		{
			if (services.ContainsDescendant(node, parent))
			{
				if (!parent) throw std::logic_error("Original parent cycle adjustment dereferences NULL");
				if (node.parent) services.SetParent(state, *parent, node.parent);
				else if (node.owner) services.SetOwner(state, *parent, node.owner);
				else services.SetParent(state, *parent, NULL);
			}
			if (node.parent) services.RemoveChild(state, *node.parent, node);
			if (node.owner) services.RemoveRoot(state, *node.owner, node);
			node.parent = parent;
			if (parent)
			{
				services.AttachChild(state, *parent, node);
				if (!node.parent) throw std::logic_error("Original parent position reload dereferences NULL");
				int x, y;
				services.Position(*node.parent, x, y);
				const int relativeX = Bits(static_cast<DWORD>(node.rectangle.left) - static_cast<DWORD>(x));
				const int relativeY = Bits(static_cast<DWORD>(node.rectangle.top) - static_cast<DWORD>(y));
				node.relativeRectangle.right = Bits(static_cast<DWORD>(node.relativeRectangle.right) +
					static_cast<DWORD>(relativeX) - static_cast<DWORD>(node.relativeRectangle.left));
				node.relativeRectangle.bottom = Bits(static_cast<DWORD>(node.relativeRectangle.bottom) +
					static_cast<DWORD>(relativeY) - static_cast<DWORD>(node.relativeRectangle.top));
				node.relativeRectangle.left = relativeX;
				node.relativeRectangle.top = relativeY;
			}
			state.dirty = 1;
		}
	}
}
