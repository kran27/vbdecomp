#include "MenuSkin.hpp"
#include "configuration/ResourceConfigurationRead.hpp"

#include <cstdlib>
#include <cstring>
#include <stdexcept>
#include <memory>

namespace JE
{
	namespace GUI
	{
		namespace
		{
			const char* const FragmentSuffixes[] = {".upperleft", ".uppermiddle", ".upperright",
				".left", ".middle", ".right", ".lowerleft", ".lowermiddle", ".lowerright"};
			const char* const ButtonPrefixes[] = {"button.up", "button.selected", "button.down",
				"button.disabled", "button.checked.up", "button.checked.selected",
				"button.checked.down", "button.checked.disabled"};
			LONG Signed(DWORD value)
			{ LONG result; std::memcpy(&result, &value, sizeof(result)); return result; }
			DWORD Width(const RECT& rectangle) { return DWORD(rectangle.right) - DWORD(rectangle.left); }
			DWORD Height(const RECT& rectangle) { return DWORD(rectangle.bottom) - DWORD(rectangle.top); }
		}

		MenuSkinFragment::~MenuSkinFragment()
		{
			if (texture) services.ReleaseTexture(texture);
		}

		MenuSkin* MenuSkinServices::CreateSkin() { return new MenuSkin(*this); }
		BYTE MenuSkinServices::ReadResource(const std::string& filename, Configuration::Settings& settings)
		{ return Configuration::ReadResourceConfigurationFile(filename, settings); }
		BYTE MenuSkinServices::ReadFile(const std::string& filename, Configuration::Settings& settings)
		{ return Configuration::ReadFile(filename, settings); }
		const std::string* MenuSkinServices::FindValue(const Configuration::Values& values, const char* key)
		{
			Configuration::Values::const_iterator found = values.find(key);
			return found == values.end() ? NULL : &found->second;
		}
		LONG MenuSkinServices::Integer(const char* text) { return std::atol(text); }
		char* MenuSkinServices::Token(char* text, const char* delimiters) { return std::strtok(text, delimiters); }
		void MenuSkinServices::AssignConfiguration(std::string& target, const std::string& filename) { target = filename; }
		const char* MenuSkinServices::ChildName(MenuNode& child) { return child.name.c_str(); }
		void MenuSkinServices::SetChildHidden(MenuNode& child, BYTE value) { child.flags[0] = value; }
		void MenuSkinServices::SetChildStateFlag(MenuNode& child, BYTE value) { child.field0x3B = value; }
		void MenuSkinServices::ApplyBaseSkin(MenuNode& node, MenuSkinCache& cache,
			const std::string& filename, const std::string& prefix)
		{ ApplyMenuSkin(node, cache, filename, prefix, *this); }

		void MenuSkinServices::ApplyNodeSkin(MenuNode& node, MenuSkinCache& cache,
			const std::string& filename, const std::string& prefix)
		{
			if (node.type == 2) ApplyMenuPictureSkin(node, cache, filename, prefix, *this);
			else if (node.type <= 1) ApplyMenuSkin(node, cache, filename, prefix, *this);
			else throw std::logic_error("Unrecovered Label/Edit skin override");
		}

		void MenuSkin::DestroyFragments()
		{
			while (!fragments.empty())
			{
				std::map<std::string, MenuSkinFragment*>::iterator current = fragments.begin();
				delete current->second;
				fragments.erase(current);
			}
		}
		MenuSkin::~MenuSkin()
		{
			//The fallback member was destroyed before the map. Keep that order,
			//including map cleanup if its texture release departs.
			try { delete fallback; }
			catch (...) { DestroyFragments(); throw; }
			DestroyFragments();
		}
		MenuSkinCache::~MenuSkinCache()
		{
			while (!skins.empty())
			{
				std::map<std::string, MenuSkin*>::iterator current = skins.begin();
				delete current->second;
				skins.erase(current);
			}
		}
		void MenuSkin::Clear() { DestroyFragments(); }

		BYTE MenuSkin::LoadFile(const std::string& filename)
		{
			Clear();
			Configuration::Settings settings;
			if (!services.ReadFile(filename, settings)) return 0;
			return Apply(settings);
		}

		BYTE MenuSkin::LoadResource(const std::string& filename)
		{
			Clear();
			Configuration::Settings settings;
			if (!services.ReadResource(filename, settings)) return 0;
			return Apply(settings);
		}

		BYTE MenuSkin::Apply(const Configuration::Settings& settings)
		{
			for (Configuration::Settings::const_iterator section = settings.begin(); section != settings.end(); ++section)
			{
				std::map<std::string, MenuSkinFragment*>::iterator found = fragments.find(section->first);
				if (found == fragments.end())
				{
					std::auto_ptr<MenuSkinFragment> created(new MenuSkinFragment(services));
					found = fragments.insert(std::make_pair(section->first, created.get())).first;
					created.release();
				}
				ParseMenuSkinFragment(*found->second, section->second, services);
			}
			return 1;
		}

		MenuSkinFragment& MenuSkin::Find(const std::string& name)
		{
			std::map<std::string, MenuSkinFragment*>::iterator found = fragments.find(name);
			return found == fragments.end() ? *fallback : *found->second;
		}

		MenuSkin* MenuSkinCache::Get(const std::string& filename)
		{
			std::map<std::string, MenuSkin*>::iterator found = skins.find(filename);
			if (found != skins.end()) return found->second;
			MenuSkin* skin = services.CreateSkin();
			if (!skin) throw std::logic_error("Original SkinCache4B5C30 dereferences a NULL allocation");
			// Original only protects construction through ordinary compiler EH.
			// A departure from LoadResource has no added delete/rollback.
			if (skin->LoadResource(filename))
			{
				skins[filename] = skin;
				return skin;
			}
			delete skin;
			return NULL;
		}

		void ParseMenuSkinFragment(MenuSkinFragment& fragment,
			const Configuration::Values& values, MenuSkinServices& services)
		{
			fragment.flags = 0;
			const std::string* value = services.FindValue(values, "flags");
			if (value) fragment.flags = services.Integer(value->c_str());
			fragment.texture = NULL; // Deliberately no release of an overwritten prior entry here.
			value = services.FindValue(values, "texture");
			if (value) fragment.texture = services.AcquireTexture(*value);
			const FragmentTextureInfo* info = NULL;
			if (fragment.texture)
			{
				info = services.TextureInfo(fragment.texture);
				if (!info) fragment.texture = NULL;
			}
			fragment.color = 0xFF808080u;
			value = services.FindValue(values, "color");
			if (value)
			{
				char text[64]; // Original unbounded strcpy; caller domain fits this buffer.
				std::strcpy(text, value->c_str());
				DWORD components[4];
				UINT count = 0;
				char* token = services.Token(text, ",\n");
				while (token && count < 4)
				{
					components[count++] = services.Integer(token);
					token = services.Token(NULL, ",\n");
				}
				if (count == 4)
					fragment.color = (components[0] << 24) | ((components[1] & 255) << 16) |
						((components[2] & 255) << 8) | (components[3] & 255);
			}
			value = services.FindValue(values, "width");
			fragment.width = value ? DWORD(services.Integer(value->c_str())) :
				fragment.texture ? DWORD(info->width) : 8;
			value = services.FindValue(values, "height");
			fragment.height = value ? DWORD(services.Integer(value->c_str())) :
				fragment.texture ? DWORD(info->height) : 8;
			fragment.rate = 0;
			value = services.FindValue(values, "rate");
			if (value) fragment.rate = services.Integer(value->c_str());
		}

		void ApplyMenuSkin(MenuNode& node, MenuSkinCache& cache, const std::string& filename,
			const std::string& prefix, MenuSkinServices& services)
		{
			services.AssignConfiguration(node.configuration, filename);
			MenuSkin* skin = cache.Get(filename);
			if (!skin) throw std::logic_error("Original Window4B06C0 uses a NULL skin after failed loading");
			const DWORD alpha = node.color & 0xFF000000u;
			for (UINT index = 0; index < 9; ++index)
			{
				MenuSkinFragment& record = skin->Find(prefix + FragmentSuffixes[index]);
				MenuFragment& fragment = node.fragments[index];
				fragment.mesh.material.texture = record.texture;
				fragment.uvDirty = 1;
				if (record.flags <= 7) { fragment.tileMode = record.flags; fragment.uvDirty = 1; }
				const DWORD width = record.width, height = record.height;
				services.Resize(fragment, Signed(width), Signed(height));
				fragment.frameWidth = width;
				fragment.frameHeight = height;
				fragment.frameRate = record.rate;
				if (record.texture)
				{
					const FragmentTextureInfo* info = services.TextureInfo(record.texture);
					if (info)
					{
						const DWORD divisor = (record.flags & 1) ? height : width;
						if (!divisor) throw std::logic_error("Original Window4B06C0 divides frame count by zero");
						fragment.frameCount = ((record.flags & 1) ? DWORD(info->height) : DWORD(info->width)) / divisor;
					}
				}
				services.ApplyFragmentColor(fragment, alpha | (record.color & 0xFFFFFFu));
			}
			node.geometryDirty = 1;
		}

		void ApplyMenuPictureSkin(MenuNode& node, MenuSkinCache& cache, const std::string& filename,
			const std::string& prefix, MenuSkinServices& services)
		{
			MenuFragment& fragment = node.fragments[4];
			IDirect3DBaseTexture8* texture = fragment.mesh.material.texture;
			const DWORD flags = fragment.tileMode, width = Width(fragment.rectangle), height = Height(fragment.rectangle);
			const DWORD frameWidth = fragment.frameWidth, frameHeight = fragment.frameHeight;
			const DWORD rate = fragment.frameRate, count = fragment.frameCount, color = fragment.color;
			ApplyMenuSkin(node, cache, filename, prefix, services);
			fragment.mesh.material.texture = texture; fragment.uvDirty = 1;
			if (flags <= 7) { fragment.tileMode = flags; fragment.uvDirty = 1; }
			services.Resize(fragment, Signed(width), Signed(height));
			fragment.frameWidth = frameWidth; fragment.frameHeight = frameHeight;
			fragment.frameRate = rate; fragment.frameCount = count;
			services.ApplyFragmentColor(fragment, color);
		}

		void SetMenuWindowConfiguration(MenuNode& node, MenuSkinCache& cache,
			const std::string& filename, MenuSkinServices& services)
		{
			services.ApplyNodeSkin(node, cache, filename, "window");
		}

		void SetMenuButtonConfiguration(MenuNode& node, MenuSkinCache& cache,
			const std::string& filename, MenuSkinServices& services)
		{
			if (node.button0xD1C >= 8)
				throw std::logic_error("Original Button4A70D0 indexes an unavailable skin-state prefix");
			services.ApplyBaseSkin(node, cache, filename, ButtonPrefixes[node.button0xD1C]);
			MenuNode* down = NULL;
			MenuNode* up = NULL;
			std::list<MenuNode*> selected;
			for (std::list<MenuNode*>::iterator current = node.children.begin(); current != node.children.end(); ++current)
			{
				MenuNode* child = *current;
				bool recognized = true, match = false;
				if (!::_strnicmp(services.ChildName(*child), "BtnUp", 5))
				{
					if (node.button0xD1C < 4) up = child;
					services.SetChildStateFlag(*child, 1); services.SetChildHidden(*child, 1);
					match = node.button0xD1C == 0;
				}
				else if (!::_strnicmp(services.ChildName(*child), "BtnSelected", 11))
				{
					services.SetChildStateFlag(*child, 1); services.SetChildHidden(*child, 1);
					match = node.button0xD1C == 1;
				}
				else if (!::_strnicmp(services.ChildName(*child), "BtnDown", 7))
				{
					if (node.button0xD1C >= 4 && node.button0xD1C <= 7) down = child;
					services.SetChildStateFlag(*child, 1); services.SetChildHidden(*child, 1);
					match = node.button0xD1C == 2;
				}
				else if (!::_strnicmp(services.ChildName(*child), "BtnDisabled", 11))
				{
					services.SetChildStateFlag(*child, 1); services.SetChildHidden(*child, 1);
					match = node.button0xD1C == 3;
				}
				else if (!::_strnicmp(services.ChildName(*child), "BtnCheckedUp", 5))
				{
					services.SetChildStateFlag(*child, 1); services.SetChildHidden(*child, 1);
					match = node.button0xD1C == 4;
				}
				else if (!::_strnicmp(services.ChildName(*child), "BtnCheckedSelected", 11))
				{
					services.SetChildStateFlag(*child, 1); services.SetChildHidden(*child, 1);
					match = node.button0xD1C == 5;
				}
				else if (!::_strnicmp(services.ChildName(*child), "BtnCheckedDown", 7))
				{
					services.SetChildStateFlag(*child, 1); services.SetChildHidden(*child, 1);
					match = node.button0xD1C == 6;
				}
				else if (!::_strnicmp(services.ChildName(*child), "BtnCheckedDisabled", 11))
				{
					services.SetChildStateFlag(*child, 1); services.SetChildHidden(*child, 1);
					match = node.button0xD1C == 7;
				}
				else recognized = false;
				if (recognized && match) selected.push_back(child);
				if (!down) down = child;
				if (!up) up = child;
			}
			if (!selected.empty())
			{
				for (std::list<MenuNode*>::iterator selectedChild = selected.begin(); selectedChild != selected.end(); ++selectedChild)
					services.SetChildHidden(**selectedChild, 0);
			}
			else if (node.button0xD1C < 4)
			{
				if (up) services.SetChildHidden(*up, 0);
				else if (down) services.SetChildHidden(*down, 0);
			}
			else if (node.button0xD1C <= 7)
			{
				if (down) services.SetChildHidden(*down, 0);
				else if (up) services.SetChildHidden(*up, 0);
			}
		}
	}
}
