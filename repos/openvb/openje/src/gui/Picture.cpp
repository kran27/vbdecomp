#include "Picture.hpp"
#include "resources/ResourceSystem.hpp"
#include "entity/MapSound.hpp"
#include "graphics/G3DModel.hpp"
#include "graphics/SystemInterface.hpp"
#include "graphics/Texture.hpp"

namespace GUI
{
	Picture::Picture(DWORD type) : Window(type), textureHandle(0), ownsTexture(0)
	{
		pictureHeight = 0;
		pictureWidth = 0;
	}

	Picture::~Picture()
	{
		const DWORD handle = textureHandle;
		if (handle && ownsTexture) g_GfxSystem->DeleteTexture(handle);
	}

	BYTE Picture::ReadResource(JE::Resources::ResourceFileHandle* file, int version)
	{
		const BYTE result = Window::ReadResource(file, version);
		UpdateDimensions();
		return result;
	}

	BYTE Picture::ReadFile(FILE* file, int version)
	{
		const BYTE result = Window::ReadFile(file, version);
		UpdateDimensions();
		return result;
	}

	void Picture::ApplyProfile(const std::string& skinName, const std::string& profile)
	{
		Fragment& center = fragments[4];
		const DWORD handle = center.GetTextureHandle();
		const DWORD tile = center.GetTileMode();
		LONG width, height;
		center.GetSize(width, height);
		DWORD frameWidth, frameHeight;
		center.GetFrameSize(frameWidth, frameHeight);
		const DWORD rate = center.GetFrameRate();
		const DWORD count = center.GetFrameCount();
		const DWORD color = center.GetColor();
		Window::ApplyProfile(skinName, profile);
		center.SetTextureHandle(handle);
		center.SetTileMode(tile);
		center.Resize(width, height);
		// Actual4AB6C0 writes only these two fields, without dirtying UVs.
		center.SetFrameSize(frameWidth, frameHeight);
		center.SetFrameRate(rate);
		center.SetFrameCount(count);
		center.SetColor(color);
	}

	void Picture::ApplySkin(const std::string& skinName)
	{
		ApplyProfile(skinName, "picture");
	}

	void Picture::GetTextureHandle(DWORD& handle)
	{
		handle = fragments[4].GetTextureHandle();
	}

	void Picture::GetTextureName(std::string& output)
	{
		output.erase(output.begin(), output.end());
		const DWORD handle = fragments[4].GetTextureHandle();
		if (handle)
		{
			Gfx_Texture* texture = g_GfxSystem->LookupTexture(handle);
			if (texture)
			{
				output = texture->GetDiagnosticName();
			}
		}
	}

	void Picture::SetTileMode(DWORD mode)
	{
		fragments[4].SetTileMode(mode);
	}

	void Picture::SetSourceRect(float x, float y, float width, float height)
	{
		float selectedWidth = static_cast<float>(pictureWidth);
		float selectedHeight = static_cast<float>(pictureHeight);
		if (width >= 0.0f && !(selectedWidth < width)) selectedWidth = width;
		if (height >= 0.0f && !(selectedHeight < height)) selectedHeight = height;
		fragments[4].SetSourceRect(x, y, selectedWidth, selectedHeight);
	}

	void Picture::UpdateDimensions()
	{
		pictureHeight = 0;
		pictureWidth = 0;
		bool found = false;
		std::string name;
		GetTextureName(name);
		if (!name.empty())
		{
			const LONG length = static_cast<LONG>(name.size());
			if (length >= 4 && name.c_str()[length - 4] == '.'
				&& JE::UpperMapSoundByte(static_cast<BYTE>(name.c_str()[length - 3])) == 'T'
				&& JE::UpperMapSoundByte(name.c_str()[length - 2]) == 'G'
				&& JE::UpperMapSoundByte(static_cast<BYTE>(name[length - 1])) == 'A')
			{
				JE::Resources::ResourceData* resource = JE::Resources::g_pResourceSystem->Demand(name.c_str(), 1);
				if (resource)
				{
					G3D_Texture image;
					image.SetWidth(-1);
					if (image.LoadImage(static_cast<const BYTE*>(resource->data), 0) || image.width > 0)
					{
						const LONG imageWidth = image.width;
						const LONG imageHeight = image.height;
						pictureWidth = imageWidth;
						pictureHeight = imageHeight;
						found = true;
					}
					JE::Resources::g_pResourceSystem->ReleaseResource(resource);
				}
			}
		}
		if (!found)
		{
			DWORD handle;
			GetTextureHandle(handle);
			if (handle)
			{
				Gfx_Texture* texture = g_GfxSystem->LookupTexture(handle);
				if (texture)
				{
					LONG width;
					if (texture->frameCount <= 0) width = 0;
					else width = texture->frames[0].width;
					pictureWidth = width;
					LONG height;
					if (texture->frameCount <= 0) height = 0;
					else height = texture->frames[0].height;
					pictureHeight = height;
				}
			}
		}
		fragments[4].SetSourceRect(0.0f, 0.0f,
			static_cast<float>(pictureWidth), static_cast<float>(pictureHeight));
	}
}
