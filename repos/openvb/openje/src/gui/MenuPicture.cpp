#include "MenuPicture.hpp"

#include <d3dx8tex.h>
#include <cctype>
#include <stdexcept>

namespace JE
{
	namespace GUI
	{
		MenuPictureImageInfo ReadMenuPictureImageInfo(const void* data, DWORD size)
		{
			D3DXIMAGE_INFO information;
			const HRESULT result = D3DXGetImageInfoFromFileInMemory(data, size, &information);
			if (FAILED(result))
				throw std::logic_error("D3DX image metadata failure: original partial-header dimensions are not established by this adapter");
			MenuPictureImageInfo output;
			output.accepted = SUCCEEDED(result);
			output.width = static_cast<LONG>(information.Width);
			output.height = static_cast<LONG>(information.Height);
			return output;
		}

		MenuPictureImageInfo ResourceMenuPictureServices::ReadImageInfo(Resources::ResourceData& resource)
		{
			return ReadMenuPictureImageInfo(resource.data, resource.size);
		}

		void UpdateMenuPictureDimensions(MenuNode& node, MenuPictureServices& services)
		{
			node.pictureHeight = 0;
			node.pictureWidth = 0;
			bool dimensionsFound = false;
			const std::string name = services.GetTextureName(node);
			const std::size_t size = name.size();
			if (size >= 4 && name[size - 4] == '.'
				&& std::toupper(static_cast<unsigned char>(name[size - 3])) == 'T'
				&& std::toupper(static_cast<unsigned char>(name[size - 2])) == 'G'
				&& std::toupper(static_cast<unsigned char>(name[size - 1])) == 'A')
			{
				Resources::ResourceData* resource = services.DemandImage(name.c_str());
				if (resource)
				{
					const MenuPictureImageInfo information = services.ReadImageInfo(*resource);
					if (information.accepted || information.width > 0)
					{
						node.pictureWidth = information.width;
						node.pictureHeight = information.height;
						dimensionsFound = true;
					}
					services.ReleaseImage(resource);
				}
			}
			if (!dimensionsFound)
			{
				IDirect3DBaseTexture8* texture = services.GetTexture(node);
				if (texture)
				{
					const FragmentTextureInfo* information = services.TextureInfo(texture);
					if (information)
					{
						node.pictureWidth = information->width;
						node.pictureHeight = information->height;
					}
				}
			}
			//4B15A0 stores the source rectangle and sets geometryDirty only.
			MenuFragment& center = node.fragments[4];
			center.sourceX = 0;
			center.sourceY = 0;
			center.sourceWidth = static_cast<float>(node.pictureWidth);
			center.sourceHeight = static_cast<float>(node.pictureHeight);
			center.geometryDirty = 1;
		}
	}
}
