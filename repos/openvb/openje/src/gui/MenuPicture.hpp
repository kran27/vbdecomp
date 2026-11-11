#pragma once

#include "MenuLayout.hpp"

namespace JE
{
	namespace GUI
	{
		struct MenuPictureImageInfo
		{
			bool accepted;
			LONG width, height;
		};
		MenuPictureImageInfo ReadMenuPictureImageInfo(const void* data, DWORD size); //Real SDK adapter.

		class MenuPictureServices
		{
		public:
			virtual ~MenuPictureServices() {}
			virtual std::string GetTextureName(MenuNode& node) = 0; //Vslot312.
			virtual IDirect3DBaseTexture8* GetTexture(MenuNode& node) = 0; //Vslot308.
			// Return NULL for missing texture metadata; zero dimensions for no levels.
			virtual const FragmentTextureInfo* TextureInfo(IDirect3DBaseTexture8* texture) = 0;
			virtual Resources::ResourceData* DemandImage(const char* name) = 0;
			virtual MenuPictureImageInfo ReadImageInfo(Resources::ResourceData& resource) = 0;
			virtual void ReleaseImage(Resources::ResourceData* resource) = 0;
		};

		class ResourceMenuPictureServices : public MenuPictureServices
		{
		public:
			Resources::ResourceData* DemandImage(const char* name)
			{
				return Resources::g_pResourceSystem->Demand(name, 1);
			}
			MenuPictureImageInfo ReadImageInfo(Resources::ResourceData& resource);
			void ReleaseImage(Resources::ResourceData* resource)
			{
				Resources::g_pResourceSystem->ReleaseResource(resource);
			}
		};

		void UpdateMenuPictureDimensions(MenuNode& node, MenuPictureServices& services); //4AAF10.
	}
}
