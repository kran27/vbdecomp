#include "Picture.hpp"
#include "resources/ResourceSystem.hpp"
#include "entity/MapSound.hpp"
#include "graphics/G3DModel.hpp"
#include "graphics/SystemInterface.hpp"
#include "graphics/Texture.hpp"

namespace GUI
{
	void Picture::LoadTexture(const std::string& name, BYTE resize, float x, float y,
		float sourceWidth, float sourceHeight, DWORD fit) //4AB1F0.
	{
		DWORD handle = 0;
		LONG width = 0, height = 0;
		if (!name.empty())
		{
			JE::Resources::ResourceData* resource = JE::Resources::g_pResourceSystem->Demand(name.c_str(), 1);
			if (resource)
			{
				bool haveDimensions = false;
				const LONG length = static_cast<LONG>(name.size());
				if (length >= 4 && name[length-4] == '.' &&
					JE::UpperMapSoundByte(static_cast<BYTE>(name[length-3])) == 'T' &&
					JE::UpperMapSoundByte(name[length-2]) == 'G' &&
					JE::UpperMapSoundByte(static_cast<BYTE>(name[length-1])) == 'A')
				{
					G3D_Texture image;
					image.SetWidth(-1);
					if (image.LoadImage(static_cast<const BYTE*>(resource->data), 0) || image.width > 0)
					{
						width = image.width;
						height = image.height;
						haveDimensions = true;
					}
				}
				if (!g_GfxSystem->AcquireTextureByName(&handle,name))
				{
					if (FAILED(g_GfxSystem->LoadTextureFromBuffer(&handle,name,static_cast<const BYTE*>(resource->data),
						resource->size,D3DFMT_UNKNOWN,D3DPOOL_MANAGED,1,1,5,0))) handle = 0;
				}
				JE::Resources::g_pResourceSystem->ReleaseResource(resource);
				if (!haveDimensions && handle)
				{
					Gfx_Texture* texture = g_GfxSystem->LookupTexture(handle);
					if (texture)
					{
						width = texture->frameCount > 0 ? texture->frames[0].width : 0;
						height = texture->frameCount > 0 ? texture->frames[0].height : 0;
					}
				}
			}
		}
		SetTexture(handle,resize,width,height,x,y,sourceWidth,sourceHeight,fit,1);
	}

	void Picture::SetTexture(DWORD handle, BYTE resize, LONG width, LONG height,
		float x, float y, float sourceWidth, float sourceHeight, DWORD fit, BYTE owned) //4AAB20.
	{
		if (textureHandle && ownsTexture)
		{
			g_GfxSystem->DeleteTexture(textureHandle);
			textureHandle = 0;
		}
		Fragment& center = fragments[4];
		center.SetTextureHandle(handle);
		center.SetTileMode(0);
		center.SetFrameRate(0);
		center.SetFrameCount(1);
		pictureWidth = width;
		pictureHeight = height;
		if (handle)
		{
			Gfx_Texture* texture = g_GfxSystem->LookupTexture(handle);
			if (texture)
			{
				float frameWidth = static_cast<float>(texture->frameCount > 0 ? texture->frames[0].width : 0);
				float frameHeight = static_cast<float>(texture->frameCount > 0 ? texture->frames[0].height : 0);
				pictureWidth = texture->frameCount > 0 ? texture->frames[0].width : 0;
				pictureHeight = texture->frameCount > 0 ? texture->frames[0].height : 0;
				if (width >= 0)
				{
					if (!(frameWidth < width)) frameWidth = static_cast<float>(width);
					if (pictureWidth >= width) pictureWidth = width;
				}
				if (height >= 0)
				{
					if (!(frameHeight < height)) frameHeight = static_cast<float>(height);
					if (pictureHeight >= height) pictureHeight = height;
				}
				float selectedWidth = static_cast<float>(pictureWidth), selectedHeight = static_cast<float>(pictureHeight);
				if (sourceWidth >= 0)
				{
					if (!(frameWidth < sourceWidth)) frameWidth = sourceWidth;
					if (!(selectedWidth < sourceWidth)) selectedWidth = sourceWidth;
				}
				if (sourceHeight >= 0)
				{
					if (!(frameHeight < sourceHeight)) frameHeight = sourceHeight;
					if (!(selectedHeight < sourceHeight)) selectedHeight = sourceHeight;
				}
				if (static_cast<BYTE>(fit))
				{
					LONG currentWidth, currentHeight;
					GetSize(currentWidth,currentHeight);
					if (!(frameWidth < currentWidth)) frameWidth = static_cast<float>(currentWidth);
					if (!(frameHeight < currentHeight)) frameHeight = static_cast<float>(currentHeight);
				}
				center.SetFrameSize(static_cast<DWORD>(static_cast<__int64>(frameWidth)),
					static_cast<DWORD>(static_cast<__int64>(frameHeight)));
				center.SetSourceRect(x,y,selectedWidth,selectedHeight);
				if (resize)
				{
					LONG left = 0, right = 0, top = 0, bottom = 0;
					if (frameVisible)
					{
						LONG widths[9], heights[9];
						for (int i = 0; i != 9; ++i) fragments[i].GetSize(widths[i],heights[i]);
						for (int i = 0; i != 3; ++i)
						{
							if (widths[3*i] > left) left = widths[3*i];
							if (widths[3*i+2] > right) right = widths[3*i+2];
							if (heights[i] > top) top = heights[i];
							if (heights[i+6] > bottom) bottom = heights[i+6];
						}
					}
					Resize(static_cast<LONG>(static_cast<__int64>(double(left+right)+frameWidth)),
						static_cast<LONG>(static_cast<__int64>(double(top+bottom)+frameHeight)));
				}
			}
		}
		textureHandle = handle;
		ownsTexture = owned;
	}
}
