#include "Fragment.hpp"
#include "Manager.hpp"
#include "graphics/SystemInterface.hpp"
#include "graphics/Texture.hpp"

namespace GUI
{
	void Fragment::UpdateGeometry()
	{
		Base::UpdateGeometry();
		float scaleX, scaleY;
		g_Manager.GetScale(scaleX, scaleY);
		RECT bounds = rectangle;
		if (tileMode & 4)
		{
			float selectedWidth = 0;
			float selectedHeight = 0;
			DWORD handle = material->textureHandles[0];
			if (handle)
			{
				// The original dereferences this result without a null check.
				Gfx_Texture* texture = g_GfxSystem->LookupTexture(handle);
				int count = texture->frameCount;
				LONG width, height;
				if (count <= 0) width = 0;
				else width = static_cast<LONG>(texture->frames[0].width);
				if (count <= 0) height = 0;
				else height = static_cast<LONG>(texture->frames[0].height);
				if (sourceWidth >= 0) selectedWidth = sourceWidth;
				else selectedWidth = static_cast<float>(width);
				if (sourceHeight >= 0) selectedHeight = sourceHeight;
				else selectedHeight = static_cast<float>(height);
				if (0.0f > selectedWidth) selectedWidth = 0;
				if (0.0f > selectedHeight) selectedHeight = 0;
			}

			LONG width = static_cast<LONG>(static_cast<DWORD>(rectangle.right)
				- static_cast<DWORD>(rectangle.left));
			LONG height = static_cast<LONG>(static_cast<DWORD>(rectangle.bottom)
				- static_cast<DWORD>(rectangle.top));
			LONG clippedWidth = static_cast<LONG>(selectedWidth < static_cast<double>(width)
				? selectedWidth : static_cast<double>(width));
			float roundedHeight = static_cast<float>(height);
			if (!(selectedHeight < roundedHeight)) selectedHeight = roundedHeight;
			LONG clippedHeight = static_cast<LONG>(selectedHeight);
			bounds.right = static_cast<LONG>(static_cast<DWORD>(bounds.left) + clippedWidth);
			bounds.bottom = static_cast<LONG>(static_cast<DWORD>(bounds.top) + clippedHeight);
		}

		// Lookup callbacks may change tileMode; the original tests this bit again.
		if (tileMode & 4)
		{
			float left = static_cast<LONG>(static_cast<double>(bounds.left) * scaleX);
			vertices[0].x = static_cast<float>(left);
			float top = static_cast<LONG>(static_cast<double>(bounds.top) * scaleY);
			vertices[0].y = static_cast<float>(top);
			LONG width = static_cast<LONG>(static_cast<DWORD>(bounds.right)
				- static_cast<DWORD>(bounds.left));
			LONG height = static_cast<LONG>(static_cast<DWORD>(bounds.bottom)
				- static_cast<DWORD>(bounds.top));
			float right = static_cast<LONG>(width + left);
			vertices[1].y = static_cast<float>(top);
			vertices[1].x = static_cast<float>(right);
			vertices[2].x = static_cast<float>(left);
			float roundedTop = static_cast<float>(top);
			float bottom = static_cast<LONG>(height + static_cast<double>(roundedTop));
			vertices[2].y = static_cast<float>(bottom);
			vertices[3].y = static_cast<float>(bottom);
			vertices[3].x = static_cast<float>(right);
		}
		else
		{
			float halfX = scaleX * 0.5f;
			double left = static_cast<double>(bounds.left) * scaleX - halfX;
			vertices[0].x = static_cast<float>(left);
			float halfY = scaleY * 0.5f;
			double top = static_cast<double>(bounds.top) * scaleY - halfY;
			vertices[0].y = static_cast<float>(top);
			float right = (bounds.right - 1.0f) * scaleX + halfX;
			LONG bottomEdge = static_cast<LONG>(static_cast<DWORD>(bounds.bottom) - 1);
			vertices[1].x = right;
			vertices[3].x = right;
			vertices[1].y = static_cast<float>(top);
			vertices[2].x = static_cast<float>(left);
			double bottom = static_cast<double>(bottomEdge) * scaleY + halfY;
			vertices[2].y = static_cast<float>(bottom);
			vertices[3].y = static_cast<float>(bottom);
		}
	}
}
