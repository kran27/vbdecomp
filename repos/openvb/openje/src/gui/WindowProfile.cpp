#include "Window.hpp"
#include "Manager.hpp"
#include "Skin.hpp"
#include "graphics/SystemInterface.hpp"
#include "graphics/Texture.hpp"

namespace GUI
{
	void Window::ApplyProfile(const std::string& filename, const std::string& prefix)
	{
		skin = filename;
		Skin* source = g_Manager.GetSkin(filename);
		const DWORD alpha = color & 0xFF000000;
		static const char* const suffixes[9] =
		{
			".upperleft", ".uppermiddle", ".upperright", ".left", ".middle",
			".right", ".lowerleft", ".lowermiddle", ".lowerright"
		};
		for (int index = 0; index < 9; ++index)
		{
			SkinFragment* record = source->Find(prefix + suffixes[index]);
			if (record)
			{
				Fragment& fragment = fragments[index];
				fragment.SetTextureHandle(record->textureHandle);
				fragment.SetTileMode(record->flags);
				const DWORD height = record->height;
				const DWORD width = record->width;
				fragment.Resize(width, height);
				fragment.SetFrameSize(width, height);
				fragment.SetFrameRate(record->rate);
				if (record->textureHandle)
				{
					Gfx_Texture* texture = g_GfxSystem->LookupTexture(record->textureHandle);
					if (texture)
					{
						DWORD frames;
						if (record->flags & 1)
						{
							DWORD dimension = texture->frameCount > 0 ? texture->frames[0].height : 0;
							frames = dimension / height;
						}
						else
						{
							DWORD dimension = texture->frameCount > 0 ? texture->frames[0].width : 0;
							frames = dimension / width;
						}
						fragment.SetFrameCount(frames);
					}
				}
				fragment.SetColor(alpha | (record->color & 0x00FFFFFF));
			}
		}
		geometryDirty = 1;
	}

	void Window::ApplySkin(const std::string& filename)
	{
		const std::string prefix("window");
		ApplyProfile(filename, prefix);
	}

	void Window::GetSkin(std::string& value)
	{
		value = skin;
	}
}
