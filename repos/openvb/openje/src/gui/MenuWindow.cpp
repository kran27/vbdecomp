#include "MenuWindow.hpp"

#include <algorithm>
#include <cstring>
#include <stdexcept>

namespace JE
{
	namespace GUI
	{
		namespace
		{
			LONG Signed(DWORD value)
			{
				LONG result;
				std::memcpy(&result, &value, sizeof(result));
				return result;
			}
			LONG Add(LONG left, LONG right)
			{ return Signed(static_cast<DWORD>(left) + static_cast<DWORD>(right)); }
			LONG Subtract(LONG left, LONG right)
			{ return Signed(static_cast<DWORD>(left) - static_cast<DWORD>(right)); }

			void RequireWindowTable(const MenuNode& node)
			{
				if (node.type > 2)
					throw std::logic_error("Unrecovered Label/Edit GUI virtual layout/update/draw override");
			}

			void ReverseChildren(MenuNode& node, MenuWindowServices& services,
				void (MenuWindowServices::*callback)(MenuNode&))
			{
				if (node.children.empty()) return;
				std::list<MenuNode*>::iterator current = node.children.end();
				--current;
				for (;;)
				{
					(services.*callback)(**current);
					// Read the live predecessor after the callback. This also sees
					// an inserted first node, without revisiting appended tail nodes.
					if (current == node.children.begin()) break;
					--current;
				}
			}
		}

		void MenuWindowServices::Size(MenuFragment& fragment, LONG& width, LONG& height)
		{
			width = Subtract(fragment.rectangle.right, fragment.rectangle.left);
			height = Subtract(fragment.rectangle.bottom, fragment.rectangle.top);
		}

		void MenuWindowServices::Position(MenuFragment& fragment, LONG x, LONG y)
		{
			const LONG dx = Subtract(x, fragment.rectangle.left);
			const LONG dy = Subtract(y, fragment.rectangle.top);
			fragment.geometryDirty = 1;
			OffsetRect(&fragment.rectangle, dx, dy);
		}

		void MenuWindowServices::Resize(MenuFragment& fragment, LONG width, LONG height)
		{
			if (width >= 2 && height >= 2)
			{
				fragment.rectangle.right = Add(width, fragment.rectangle.left);
				fragment.geometryDirty = 1;
				fragment.rectangle.bottom = Add(height, fragment.rectangle.top);
			}
			fragment.uvDirty = 1; //0x4B1450 stores this even when0x4B2790 rejects dimensions.
		}

		void MenuWindowServices::LayoutNode(MenuNode& node)
		{
			RequireWindowTable(node);
			LayoutMenuWindow(node, *this);
		}

		void MenuWindowServices::UpdateChild(MenuNode& node)
		{
			RequireWindowTable(node);
			UpdateMenuWindow(node, *this);
		}

		void MenuWindowServices::DrawChild(MenuNode& node)
		{
			RequireWindowTable(node);
			DrawMenuWindow(node, *this);
		}

		void NativeMenuWindowServices::UpdateFragment(MenuFragment& fragment)
		{
			if (fragment.geometryDirty) fragment.UpdateGeometry(fragments);
		}

		void NativeMenuWindowServices::DrawFragment(MenuFragment& fragment,
			Gfx::Viewport* viewport, BYTE flags)
		{
			fragment.Draw(fragments, viewport, flags);
		}

		void LayoutMenuWindow(MenuNode& node, MenuWindowServices& services)
		{
			node.geometryDirty = 0;
			LONG widths[9] = {0}, heights[9] = {0};
			for (UINT index1 = 0; index1 < 9; ++index1)
				if (node.flags[1] || index1 == 4)
					services.Size(node.fragments[index1], widths[index1], heights[index1]);

			LONG left = 0, right = 0, top = 0, bottom = 0;
			for (UINT index2 = 0; index2 < 3; ++index2)
			{
				left = (std::max)(left, widths[index2 * 3]);
				top = (std::max)(top, heights[index2]);
				right = (std::max)(right, widths[index2 * 3 + 2]);
				bottom = (std::max)(bottom, heights[index2 + 6]);
			}
			const LONG localLeft = node.rectangle.left;
			if (Subtract(node.rectangle.right, localLeft) < Add(right, left))
			{
				const LONG nextRight = Add(Add(localLeft, right), Add(left, 1));
				const LONG offset = Subtract(node.relativeRectangle.left, localLeft);
				node.rectangle.right = nextRight;
				node.relativeRectangle.right = Add(nextRight, offset);
			}
			const LONG localTop = node.rectangle.top;
			if (Subtract(node.rectangle.bottom, localTop) < Add(top, bottom))
			{
				const LONG nextBottom = Add(Add(localTop, bottom), Add(top, 1));
				const LONG offset = Subtract(node.relativeRectangle.top, localTop);
				node.rectangle.bottom = nextBottom;
				node.relativeRectangle.bottom = Add(nextBottom, offset);
			}
			const LONG innerTop = Add(localTop, top);
			const LONG innerRight = Subtract(node.rectangle.right, right);
			const LONG innerLeft = Add(localLeft, left);
			const LONG innerBottom = Subtract(node.rectangle.bottom, bottom);
			const LONG innerWidth = Subtract(innerRight, innerLeft);
			const LONG innerHeight = Subtract(innerBottom, innerTop);
			services.Position(node.fragments[0], Subtract(innerLeft, widths[0]), Subtract(innerTop, heights[0]));
			services.Position(node.fragments[1], innerLeft, Subtract(innerTop, heights[1]));
			services.Resize(node.fragments[1], innerWidth, heights[1]);
			services.Position(node.fragments[2], innerRight, Subtract(innerTop, heights[2]));
			services.Position(node.fragments[3], Subtract(innerLeft, widths[3]), innerTop);
			services.Resize(node.fragments[3], widths[3], innerHeight);
			services.Position(node.fragments[4], innerLeft, innerTop);
			services.Resize(node.fragments[4], innerWidth, innerHeight);
			services.Position(node.fragments[5], innerRight, innerTop);
			services.Resize(node.fragments[5], widths[5], innerHeight);
			services.Position(node.fragments[6], Subtract(innerLeft, widths[6]), innerBottom);
			services.Position(node.fragments[7], innerLeft, innerBottom);
			services.Resize(node.fragments[7], innerWidth, heights[7]);
			services.Position(node.fragments[8], innerRight, innerBottom);
		}

		void UpdateMenuWindow(MenuNode& node, MenuWindowServices& services)
		{
			if (node.geometryDirty) services.LayoutNode(node);
			if (node.flags[0]) return;
			if (node.flags[1])
				for (UINT index3 = 0; index3 < 4; ++index3) services.UpdateFragment(node.fragments[index3]);
			if (node.flags[2]) services.UpdateFragment(node.fragments[4]);
			if (node.flags[1])
				for (UINT index3 = 5; index3 < 9; ++index3) services.UpdateFragment(node.fragments[index3]);
			ReverseChildren(node, services, &MenuWindowServices::UpdateChild);
		}

		void DrawMenuWindow(MenuNode& node, MenuWindowServices& services)
		{
			if (node.flags[0]) return;
			const BYTE flags = (node.field0x60 != 0 ? 1 : 0) | (static_cast<BYTE>(node.field0x68) != 0 ? 2 : 0);
			if (node.flags[1])
				for (UINT index4 = 0; index4 < 4; ++index4)
					services.DrawFragment(node.fragments[index4], node.targetViewport, flags);
			if (node.flags[2]) services.DrawFragment(node.fragments[4], node.targetViewport, flags);
			if (node.flags[1])
				for (UINT index5 = 5; index5 < 9; ++index5)
					services.DrawFragment(node.fragments[index5], node.targetViewport, flags);
			ReverseChildren(node, services, &MenuWindowServices::DrawChild);
		}
	}
}
