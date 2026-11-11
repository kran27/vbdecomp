#include "Fragment.hpp"
#include "Manager.hpp"
#include "graphics/SystemInterface.hpp"
#include "graphics/Texture.hpp"

// Original nonzero modes without bit4, other than1/2/3, consume an unwritten
// local float. Keep that uncertainty visible; no deterministic fallback exists.
// Only pinned emitted-code inspection can establish its actual stack mapping.
#pragma warning(push)
#pragma warning(disable: 4700 4701)
namespace GUI
{
	void Fragment::UpdateUV()
	{
		Gfx_Material* selectedMaterial = material;
		uvDirty = 0;
		DWORD handle = selectedMaterial->textureHandles[0];
		Gfx_Texture* texture = handle ? g_GfxSystem->LookupTexture(handle) : NULL;
		if (!texture)
		{
			vertices[0].u = 0;
			vertices[0].v = 0;
			vertices[1].v = 0;
			vertices[2].u = 0;
			vertices[1].u = 1;
			vertices[2].v = 1;
			vertices[3].u = 1;
			vertices[3].v = 1;
			return;
		}

		int count = texture->frameCount;
		LONG textureWidth, textureHeight;
		if (count <= 0) textureWidth = 0;
		else textureWidth = static_cast<LONG>(texture->frames[0].width);
		if (count <= 0) textureHeight = 0;
		else textureHeight = static_cast<LONG>(texture->frames[0].height);
		float selectedWidth, selectedHeight;
		if (sourceWidth >= 0) selectedWidth = sourceWidth;
		else selectedWidth = static_cast<float>(textureWidth);
		if (sourceHeight >= 0) selectedHeight = sourceHeight;
		else selectedHeight = static_cast<float>(textureHeight);
		if (0.0f > selectedWidth) selectedWidth = 0;
		if (0.0f > selectedHeight) selectedHeight = 0;

		LONG width = static_cast<LONG>(static_cast<DWORD>(rectangle.right)
			- static_cast<DWORD>(rectangle.left));
		LONG height = static_cast<LONG>(static_cast<DWORD>(rectangle.bottom)
			- static_cast<DWORD>(rectangle.top));
		DWORD mode = tileMode;
		float lowerU, lowerV;
		union UpperWord { LONG integer; float value; } upperU;
		upperU.integer = width;
		UpperWord upperV;
		upperV.value = selectedHeight;
		float scaleX, scaleY;
		if (!mode)
		{
			float origin = static_cast<float>(static_cast<double>(static_cast<LONG>(
				frameWidth * currentFrame)) + sourceX);
			float textureW = static_cast<float>(textureWidth);
			float textureH = static_cast<float>(textureHeight);
			lowerU = static_cast<float>(static_cast<double>(origin) / textureW);
			lowerV = static_cast<float>(static_cast<double>(sourceY) / textureH);
			upperU.value = static_cast<float>((static_cast<double>(origin)
				+ selectedWidth) / textureW);
			upperV.value = static_cast<float>((static_cast<double>(selectedHeight)
				+ sourceY) / textureH);
		}
		else if (mode & 4)
		{
			g_Manager.GetScale(scaleX, scaleY); // Original call; outputs unused here.
			if (!(selectedWidth < static_cast<double>(width)))
				selectedWidth = static_cast<float>(width);
			float clippedHeight = selectedHeight;
			if (!(clippedHeight < static_cast<double>(height)))
				clippedHeight = static_cast<float>(height);
			float origin = static_cast<float>(static_cast<double>(static_cast<LONG>(
				frameWidth * currentFrame)) + sourceX);
			float textureW = static_cast<float>(textureWidth);
			float textureH = static_cast<float>(textureHeight);
			lowerU = static_cast<float>(static_cast<double>(origin) / textureW);
			lowerV = static_cast<float>(static_cast<double>(sourceY) / textureH);
			upperU.value = static_cast<float>((static_cast<double>(origin)
				+ selectedWidth) / textureW);
			upperV.value = static_cast<float>((static_cast<double>(clippedHeight)
				+ sourceY) / textureH);
		}
		else if (mode == 1)
		{
			lowerU = static_cast<float>(0);
			float rectangleH = static_cast<float>(height);
			float textureH = static_cast<float>(textureHeight);
			upperV.integer = static_cast<LONG>(currentFrame);
			lowerV = static_cast<float>(static_cast<double>(upperV.integer)
				* rectangleH / textureH);
			upperU.value = static_cast<float>(static_cast<double>(width) / textureWidth);
			upperV.integer = static_cast<LONG>(currentFrame + 1);
			upperV.value = static_cast<float>(static_cast<double>(upperV.integer)
				* rectangleH / textureH);
		}
		else if (mode == 2)
		{
			float rectangleW = static_cast<float>(width);
			float textureW = static_cast<float>(textureWidth);
			upperV.integer = static_cast<LONG>(currentFrame);
			lowerU = static_cast<float>(static_cast<double>(upperV.integer)
				* rectangleW / textureW);
			lowerV = static_cast<float>(0);
			upperV.integer = static_cast<LONG>(currentFrame + 1);
			upperU.value = static_cast<float>(static_cast<double>(upperV.integer)
				* rectangleW / textureW);
			upperV.value = static_cast<float>(static_cast<double>(height) / textureHeight);
		}
		else if (mode == 3)
		{
			lowerU = static_cast<float>(0);
			lowerV = static_cast<float>(0);
			upperU.value = static_cast<float>(static_cast<double>(width) / textureWidth);
			upperV.value = static_cast<float>(static_cast<double>(height) / textureHeight);
		}
		else
		{
			// The original reads one unwritten local twice. scaleX is written only
			// in the mutually exclusive bit4 branch. Its value/stack placement is
			// unproven outside a complete matching compiler result.
			lowerU = static_cast<float>(scaleX);
			lowerV = static_cast<float>(scaleX);
			// Original var10 still contains the wrapped rectangle-width bits.
			// It is loaded as a floating value before the final vertex stores,
			// including the hardware's masked signaling-NaN conversion.
			union WidthWord { LONG integer; float value; } widthWord;
			widthWord.integer = width;
			upperU.value = widthWord.value;
			upperV.value = selectedHeight;
		}

		double finalU = upperU.value;
		float firstU = static_cast<float>(lowerU);
		vertices[0].u = firstU;
		vertices[2].u = firstU;
		vertices[0].v = static_cast<float>(lowerV);
		vertices[1].u = static_cast<float>(finalU);
		vertices[1].v = static_cast<float>(lowerV);
		vertices[2].v = upperV.value;
		vertices[3].v = upperV.value;
		vertices[3].u = static_cast<float>(finalU);
	}
}
#pragma warning(pop)
