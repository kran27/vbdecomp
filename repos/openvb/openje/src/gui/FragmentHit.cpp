#include "Fragment.hpp"
#include "Manager.hpp"
#include "TextureMask.hpp"
#include "graphics/SystemInterface.hpp"
#include "graphics/Texture.hpp"

namespace GUI
{
	bool Fragment::Contains(LONG x, LONG y)
	{
		if (!Base::Contains(x, y)) return false;
		DWORD handle = material->textureHandles[0];
		if (!handle) return true;
		TextureMask* mask = g_Manager.GetTextureMask(handle);
		if (!mask) return false;
		Gfx_Texture* texture = g_GfxSystem->LookupTexture(handle);
		if (!texture) return false;

		// Both callbacks precede these live rectangle/texture reads. The handle
		// stays cached even if either callback changes the material or its handle.
		DWORD offsetX = static_cast<DWORD>(x) - static_cast<DWORD>(rectangle.left);
		int count = texture->frameCount;
		DWORD width = count > 0 ? texture->frames[0].width : 0;
		DWORD localX = offsetX % width;
		DWORD offsetY = static_cast<DWORD>(y) - static_cast<DWORD>(rectangle.top);
		DWORD height = count > 0 ? texture->frames[0].height : 0;
		DWORD localY = offsetY % height;
		return mask->Test(static_cast<LONG>(localX), static_cast<LONG>(localY));
	}
}
