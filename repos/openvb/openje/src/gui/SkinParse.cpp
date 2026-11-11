#include "Skin.hpp"
#include "SkinServices.hpp"
#include "graphics/SystemInterface.hpp"
#include "graphics/Texture.hpp"
#include <stdlib.h>
#include <string.h>

namespace GUI
{
	SkinFragment::SkinFragment()
	{
		flags = 0;
		textureHandle = 0;
		rate = 0;
		color = 0xFF800000;
		width = 8;
		height = 8;
	}

	SkinFragment::~SkinFragment()
	{
		if (textureHandle)
		{
			g_GfxSystem->DeleteTexture(textureHandle);
		}
	}

	void SkinFragment::Parse(const JE::Configuration::Values& values)
	{
		flags = 0;
		JE::Configuration::Values::const_iterator value = values.find("flags");
		if (value != values.end())
		{
			flags = static_cast<DWORD>(atol(value->second.c_str()));
		}
		// Parsing an existing record overwrites its old reference without Release.
		textureHandle = 0;
		value = values.find("texture");
		if (value != values.end())
		{
			if (!g_GfxSystem->AcquireTextureByName(&textureHandle, value->second))
			{
				JE::Resources::ResourceData* resource = SkinServices::Demand(value->second.c_str());
				if (resource)
				{
					const DWORD size = SkinServices::Size(resource);
					const BYTE* data = SkinServices::Data(resource);
					if (FAILED(g_GfxSystem->LoadTextureFromBuffer(&textureHandle, value->second,
						data, size, D3DFMT_UNKNOWN, D3DPOOL_MANAGED, 1, 1, 5, 0)))
					{
						textureHandle = 0;
					}
					SkinServices::Release(resource);
				}
			}
		}
		Gfx_Texture* texture;
		if (textureHandle)
		{
			texture = g_GfxSystem->LookupTexture(textureHandle);
			if (!texture)
			{
				textureHandle = 0; // No release of the acquired handle on this path.
			}
		}
		color = 0xFF808080;
		value = values.find("color");
		if (value != values.end())
		{
			char text[64];
			strcpy(text, value->second.c_str()); // Original unchecked buffer.
			DWORD components[4];
			unsigned count = 0;
			char* token = strtok(text, ",\n");
			while (token && count < 4)
			{
				components[count++] = static_cast<DWORD>(atol(token));
				token = strtok(NULL, ",\n");
			}
			if (count == 4)
			{
				color = (components[0] << 24) | ((components[1] & 255) << 16)
					| ((components[2] & 255) << 8) | (components[3] & 255);
			}
		}
		value = values.find("width");
		if (value != values.end())
		{
			width = static_cast<DWORD>(atol(value->second.c_str()));
		}
		else if (textureHandle)
		{
			width = texture->frameCount > 0 ? texture->frames[0].width : 0;
		}
		else
		{
			width = 8;
		}
		value = values.find("height");
		if (value != values.end())
		{
			height = static_cast<DWORD>(atol(value->second.c_str()));
		}
		else if (textureHandle)
		{
			height = texture->frameCount > 0 ? texture->frames[0].height : 0;
		}
		else
		{
			height = 8;
		}
		rate = 0;
		value = values.find("rate");
		if (value != values.end())
		{
			rate = static_cast<DWORD>(atol(value->second.c_str()));
		}
	}
}
