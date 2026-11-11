#include "MenuLayout.hpp"

#include <cstring>
#include <new>
#include <stdexcept>

namespace JE
{
	namespace GUI
	{
		void* MenuLayoutServices::AllocateString(DWORD size)
		{
			return ::operator new(size);
		}

		void MenuLayoutServices::AssignString(std::string& output, const char* bytes,
			std::size_t count)
		{
			output.assign(bytes, count);
		}

		void MenuLayoutServices::FreeString(void* bytes)
		{
			::operator delete(bytes);
		}

		BYTE MenuLayoutServices::ReadNode(MenuNode& node, File* file, int version)
		{
			switch (node.type)
			{
			case 0: return ReadMenuWindow(node, file, version, *this);
			case 1: return ReadMenuButton(node, file, version, *this);
			case 2: return ReadMenuPicture(node, file, version, *this);
			case 3: return ReadMenuLabel(node, file, version, *this);
			default:
				throw std::logic_error("Unrecovered GUI reader for this object type");
			}
		}

		BYTE ReadMenuString(std::string& output, MenuLayoutServices::File* file,
			MenuLayoutServices& services)
		{
			DWORD size = static_cast<DWORD>(output.size());
			if (!services.Read(&size, 4, file)) return 0;
			char* bytes = static_cast<char*>(services.AllocateString(size + 1));
			if (!services.Read(bytes, size, file)) return 0; // Original allocation leak.
			bytes[size] = 0;
			services.AssignString(output, bytes, std::strlen(bytes));
			services.FreeString(bytes);
			return 1;
		}

		BYTE ReadMenuRect(RECT& output, BYTE& dirty, MenuLayoutServices::File* file,
			int, MenuLayoutServices& services)
		{
			if (!services.Read(&output, 16, file)) return 0;
			dirty = 1;
			return 1;
		}

		BYTE ReadMenuFragment(MenuFragment& fragment, MenuLayoutServices::File* file,
			int version, MenuLayoutServices& services)
		{
			if (!services.Read(&fragment.tileMode, 4, file)
				|| !services.Read(&fragment.frameWidth, 4, file)
				|| !services.Read(&fragment.frameHeight, 4, file)
				|| !services.Read(&fragment.frameRate, 4, file)
				|| !services.Read(&fragment.currentFrame, 4, file)
				|| !services.Read(&fragment.frameCount, 4, file)) return 0;
			std::string texture;
			if (!ReadMenuString(texture, file, services)) return 0;
			fragment.mesh.material.texture = services.AcquireTexture(texture);
			if (!services.Read(&fragment.mesh.vertices[0].z, 4, file)) return 0;
			for (unsigned i = 1; i < 4; ++i)
				std::memcpy(&fragment.mesh.vertices[i].z, &fragment.mesh.vertices[0].z, 4);
			if (!services.Read(&fragment.color, 4, file)) return 0;
			services.ApplyFragmentColor(fragment, fragment.color);
			fragment.uvDirty = 1;
			services.RebuildFragmentMaterial(fragment);
			return ReadMenuRect(fragment.rectangle, fragment.geometryDirty, file, version, services);
		}

		BYTE ReadMenuWindow(MenuNode& node, MenuLayoutServices::File* file,
			int version, MenuLayoutServices& services)
		{
			services.ResetWindow(node);
			if (!ReadMenuRect(node.rectangle, node.geometryDirty, file, version, services)
				|| !ReadMenuString(node.name, file, services)
				|| !services.Read(&node.flags[0], 1, file)
				|| !services.Read(&node.flags[1], 1, file)
				|| !services.Read(&node.flags[2], 1, file)
				|| !ReadMenuString(node.configuration, file, services)
				|| !services.Read(&node.color, 4, file)) return 0;
			if (version < 2 && static_cast<BYTE>(node.color >> 24) != 255)
				node.field0x64 = 1;
			if (!services.Read(&node.rectangle2, 16, file)) return 0;
			if (version >= 2)
			{
				if (!services.Read(&node.field0x60, 1, file)
					|| !services.Read(&node.field0x64, 4, file)) return 0;
				services.SetWindowValue(node, node.field0x64);
			}
			if (version >= 3)
			{
				if (!ReadMenuString(node.field0xCB8, file, services)
					|| !services.Read(&node.field0xCD4, 4, file)
					|| !ReadMenuString(node.field0xCD8, file, services)
					|| !services.Read(&node.field0xCF4, 4, file)
					|| !ReadMenuString(node.field0xCF8, file, services)) return 0;
			}
			for (unsigned fragment = 0; fragment < 9; ++fragment)
				if (!ReadMenuFragment(node.fragments[fragment], file, version, services)) return 0;
			node.strings.clear();
			DWORD index;
			if (!services.Read(&index, 4, file)) return 0;
			while (index != 0xFFFFFFFFu)
			{
				if (index >= node.strings.size()) node.strings.resize(index + 5);
				if (!ReadMenuString(node.strings[index], file, services)
					|| !services.Read(&index, 4, file)) return 0;
			}
			node.owner = NULL;
			node.parent = NULL;
			DWORD count;
			if (!services.Read(&count, 4, file)) return 0;
			for (DWORD i = 0; i < count; ++i)
			{
				DWORD type;
				if (!services.Read(&type, 4, file)) return 0;
				MenuNode* child = services.CreateNode(type);
				if (!child) throw std::logic_error("Original GUI reader dereferences a NULL factory result");
				if (!services.ReadNode(*child, file, version)) return 0; // Unattached child leaks.
				services.AttachChild(node, *child);
			}
			node.children.reverse(); // Real std::list; original template body is not ported.
			return 1;
		}

		BYTE ReadMenuButton(MenuNode& node, MenuLayoutServices::File* file,
			int version, MenuLayoutServices& services)
		{
			if (!services.Read(&node.button0xD1C, 4, file)) return 0;
			if (version >= 4)
			{
				BYTE enabled;
				if (!services.Read(&enabled, 1, file)) return 0;
				node.button0xD21 = enabled != 0;
				for (unsigned sound = 0; sound < 3; ++sound)
					if (!ReadMenuString(node.buttonSounds[sound], file, services)) return 0;
			}
			return ReadMenuWindow(node, file, version, services);
		}

		BYTE ReadMenuPicture(MenuNode& node, MenuLayoutServices::File* file,
			int version, MenuLayoutServices& services)
		{
			const BYTE result = ReadMenuWindow(node, file, version, services);
			services.UpdatePictureDimensions(node); // Also called after failed base reads.
			return result;
		}

		BYTE ReadMenuLabel(MenuNode& node, MenuLayoutServices::File* file,
			int version, MenuLayoutServices& services)
		{
			services.ResetLabel(node);
			if (!ReadMenuWindow(node, file, version, services)
				|| !services.Read(&node.stringRef, 4, file)) return 0;
			std::string text;
			if (services.ResolveLabelText(node.stringRef, text)) node.text = text;
			if (!ReadMenuString(node.font, file, services)
				|| !services.Read(&node.label0xD58, 4, file)
				|| !services.Read(&node.fontSize, 4, file)
				|| !services.Read(&node.label0xD60, 4, file)
				|| !services.Read(&node.textColor, 4, file)
				|| !services.Read(&node.label0xD68, 1, file)
				|| !services.Read(&node.label0xD69, 1, file)
				|| !services.Read(&node.justification, 4, file)
				|| !services.Read(&node.label0xE1C[0], 4, file)
				|| !services.Read(&node.label0xE1C[1], 4, file)
				|| !services.Read(&node.label0xE1C[2], 4, file)
				|| !services.Read(&node.label0xE1C[3], 4, file)
				|| (version >= 1 && !services.Read(&node.verticalAlignment, 4, file))) return 0;
			services.LayoutLabel(node);
			return 1;
		}

		BYTE LoadMenuLayout(MenuLayout& layout, const std::string& name,
			MenuLayoutServices& services)
		{
			services.ResetLayout(layout);
			MenuLayoutServices::File* file = services.Open(name.c_str());
			if (!file) return 0;
			int version = 0;
			DWORD head; // Original ignores header-read status; bytes are not preseeded.
			services.Read(&head, 4, file);
			if (head == 0x20495547u)
			{
				DWORD text;
				services.Read(&text, 4, file);
				switch (text)
				{
				case 0x302E3156u: version = 1; break;
				case 0x312E3156u: version = 2; break;
				case 0x322E3156u: version = 3; break;
				case 0x332E3156u: version = 4; break;
				}
			}
			else services.Seek(file, 0, 0);
			DWORD count;
			if (!ReadMenuString(layout.name, file, services)
				|| !services.Read(&layout.field0x20, 1, file)
				|| !services.Read(&count, 4, file))
			{
				services.Release(file);
				return 0;
			}
			for (DWORD i = 0; i < count; ++i)
			{
				DWORD type;
				if (!services.Read(&type, 4, file))
				{
					services.Release(file);
					return 0;
				}
				MenuNode* node = services.CreateNode(type);
				if (!node) throw std::logic_error("Original INT reader dereferences a NULL factory result");
				if (!services.ReadNode(*node, file, version))
				{
					services.Release(file);
					return 0; // No deletion of the failed unpublished object.
				}
				services.AttachRoot(layout, *node);
			}
			layout.roots.reverse();
			services.Release(file);
			std::list<std::list<MenuNode*>*> pending;
			pending.push_back(&layout.roots);
			while (!pending.empty())
			{
				std::list<MenuNode*>* children = pending.front();
				pending.pop_front();
				for (std::list<MenuNode*>::iterator child = children->begin(); child != children->end(); ++child)
				{
					MenuNode* node = *child;
					pending.push_back(&node->children);
					if (node->type == 1)
					{
						const std::string configuration = services.GetConfiguration(*node);
						services.SetConfiguration(*node, configuration);
					}
				}
			}
			return 1;
		}
	}
}
