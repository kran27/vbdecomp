#pragma once

#include "MenuLayout.hpp"
#include "configuration/Configuration.hpp"
#include <map>


namespace JE
{
	namespace GUI
	{
		class MenuSkinServices;
		class MenuSkinCache;

		struct MenuSkinFragment
		{
			explicit MenuSkinFragment(MenuSkinServices& services)
				: flags(0), texture(NULL), color(0xFF800000u), width(8), height(8), rate(0), services(services) {}
			~MenuSkinFragment();
			DWORD flags;
			IDirect3DBaseTexture8* texture;
			DWORD color; //523450 default differs from parsed-section reset.
			DWORD width, height, rate;
			MenuSkinServices& services;
		private:
			MenuSkinFragment(const MenuSkinFragment&);
			MenuSkinFragment& operator=(const MenuSkinFragment&);
		};

		class MenuSkin
		{
		public:
			explicit MenuSkin(MenuSkinServices& services) : services(services), fallback(new MenuSkinFragment(services)) {}
			virtual ~MenuSkin();
			virtual BYTE LoadFile(const std::string& filename); //522B10.
			virtual BYTE LoadResource(const std::string& filename); //522C30.
			virtual void Clear(); //5226A0.
			BYTE Apply(const Configuration::Settings& settings); //522AC0.
			MenuSkinFragment& Find(const std::string& name); //522530.
			MenuSkinServices& services;
			std::map<std::string, MenuSkinFragment*> fragments; // Case-sensitive byte ordering.
			MenuSkinFragment* fallback; // Owned; destroyed before the fragments, as before.
		private:
			void DestroyFragments();
			MenuSkin(const MenuSkin&);
			MenuSkin& operator=(const MenuSkin&);
		};

		class MenuSkinCache
		{
		public:
			explicit MenuSkinCache(MenuSkinServices& services) : services(services) {}
			~MenuSkinCache();
			MenuSkin* Get(const std::string& filename); //4B5C30.
			MenuSkinServices& services;
			std::map<std::string, MenuSkin*> skins;
		private:
			MenuSkinCache(const MenuSkinCache&);
			MenuSkinCache& operator=(const MenuSkinCache&);
		};

		class MenuSkinServices
		{
		public:
			virtual ~MenuSkinServices() {}
			virtual MenuSkin* CreateSkin();
			virtual BYTE ReadResource(const std::string& filename, Configuration::Settings& settings);
			virtual BYTE ReadFile(const std::string& filename, Configuration::Settings& settings);
			virtual const std::string* FindValue(const Configuration::Values& values, const char* key);
			virtual LONG Integer(const char* text);
			virtual char* Token(char* text, const char* delimiters);
			virtual void AssignConfiguration(std::string& target, const std::string& filename);

			// Actual texture-manager integration; no decoder/COM implementation here.
			virtual IDirect3DBaseTexture8* AcquireTexture(const std::string& name) = 0;
			virtual const FragmentTextureInfo* TextureInfo(IDirect3DBaseTexture8* texture) = 0;
			virtual void ReleaseTexture(IDirect3DBaseTexture8* texture) = 0;
			virtual void Resize(MenuFragment& fragment, LONG width, LONG height) = 0;
			virtual void ApplyFragmentColor(MenuFragment& fragment, DWORD color) = 0;

			virtual const char* ChildName(MenuNode& child);
			virtual void SetChildHidden(MenuNode& child, BYTE value);
			virtual void SetChildStateFlag(MenuNode& child, BYTE value);
			virtual void ApplyBaseSkin(MenuNode& node, MenuSkinCache& cache,
				const std::string& filename, const std::string& prefix);
			virtual void ApplyNodeSkin(MenuNode& node, MenuSkinCache& cache,
				const std::string& filename, const std::string& prefix);
		};

		void ParseMenuSkinFragment(MenuSkinFragment& fragment,
			const Configuration::Values& values, MenuSkinServices& services); //523490.
		void ApplyMenuSkin(MenuNode& node, MenuSkinCache& cache, const std::string& filename,
			const std::string& prefix, MenuSkinServices& services); //4B06C0.
		void ApplyMenuPictureSkin(MenuNode& node, MenuSkinCache& cache, const std::string& filename,
			const std::string& prefix, MenuSkinServices& services); //4AA9B0.
		void SetMenuWindowConfiguration(MenuNode& node, MenuSkinCache& cache,
			const std::string& filename, MenuSkinServices& services); //4B0480.
		void SetMenuButtonConfiguration(MenuNode& node, MenuSkinCache& cache,
			const std::string& filename, MenuSkinServices& services); //4A70D0.
	}
}
