#pragma once

#include "MenuFragment.hpp"
#include "PointerCapture.hpp"
#include "resources/ResourceSystem.hpp"

#include <list>
#include <string>
#include <vector>

namespace JE
{
	namespace GUI
	{
		struct MenuNode;
		struct MenuLayout;
		struct MenuEvent;

		// Native callback adapter for the existing interface/layout owner. This
		// does not introduce another root list or a second GUI ownership graph.
		class MenuEventOwner
		{
		public:
			virtual ~MenuEventOwner() {}
			virtual int HandleEvent(MenuNode& node, MenuEvent& event) = 0;
		};

		struct MenuNode : CaptureTarget
		{
			explicit MenuNode(DWORD type) : type(type), owner(NULL), parent(NULL),
				field0x68(0), targetViewport(NULL), button0xD20(0) {}
			// Window/Button/Picture +0x118 is the evidenced empty5A9320.
			virtual void OnCaptureChanged(CaptureTarget*) {}

			// Data consumed by the actual file readers; scalar fields are not seeded
			// as a substitute for the unrecovered complete GUI constructor/reset.
			DWORD type;
			RECT rectangle;
			BYTE geometryDirty;
			std::string name, configuration;
			BYTE flags[3];
			BYTE field0x3B; // Independent raw child-state BYTE+59; ctor/reset scope separate.
			DWORD color;
			RECT rectangle2;
			BYTE field0x60;
			DWORD field0x64;
			MenuLayout* owner; // Original+0x6C, not an owning pointer.
			MenuNode* parent; // Original+0x70, same borrowed graph.
			RECT relativeRectangle; // Original+0x74, distinct serialized rectangle2(+0x4C).
			// Host storage word for low-byte Draw checks, not original class layout.
			// Original constructor writes BYTE+0x68; +0x69..6B padding is unwritten.
			DWORD field0x68;
			MenuFragment fragments[9];
			std::vector<std::string> strings;
			std::string field0xCB8, field0xCD8, field0xCF8;
			DWORD field0xCD4, field0xCF4;
			Gfx::Viewport* targetViewport; // Known constructor pointer+3348.
			std::list<MenuNode*> children; // Published by the actual attach API.

			// Button4A6B60 prefix. BYTE+3361 is normalized from the input BYTE.
			DWORD button0xD1C;
			BYTE button0xD20; // Checked BYTE: actual Button constructor writes0.
			BYTE button0xD21;
			std::string buttonSounds[3];
			LONG pictureWidth, pictureHeight; //4AAF10 resets these before querying metadata.

			// Label4AA640 extension, after all base-window children.
			DWORD stringRef;
			std::string text, font;
			DWORD label0xD58, fontSize, label0xD60, textColor;
			BYTE label0xD68, label0xD69;
			DWORD justification;
			DWORD label0xE1C[4];
			DWORD verticalAlignment;
		private:
			MenuNode(const MenuNode&);
			MenuNode& operator=(const MenuNode&);
		};

		struct MenuLayout
		{
			MenuLayout() : eventOwner(NULL) {}
			std::string name;
			BYTE field0x20;
			std::list<MenuNode*> roots; // Complete GUI lifetime stays with its owner.
			MenuEventOwner* eventOwner; // Native event dispatch binding only.
		};

		class MenuLayoutServices
		{
		public:
			typedef Resources::ResourceFileHandle File;
			virtual ~MenuLayoutServices() {}
			virtual File* Open(const char* name) = 0;
			virtual BYTE Read(void* output, DWORD size, File* file) = 0;
			virtual BYTE Seek(File* file, LONG offset, int origin) = 0;
			virtual void Release(File* file) = 0;

			virtual void ResetLayout(MenuLayout& layout) = 0; //0x4955B0.
			virtual void ResetWindow(MenuNode& node) = 0; //Original vslot+176.
			virtual void ResetLabel(MenuNode& node) = 0; //0x4A8940.
			virtual MenuNode* CreateNode(DWORD type) = 0; //0x4B2910 factory API.
			virtual BYTE ReadNode(MenuNode& node, File* file, int version);
			virtual void AttachRoot(MenuLayout& layout, MenuNode& node) = 0; //0x4959B0.
			virtual void AttachChild(MenuNode& parent, MenuNode& child) = 0; //Vslot+164.
			virtual void SetWindowValue(MenuNode& node, DWORD value) = 0; //Vslot+116.

			// This boundary owns actual find/Demand/create/Release policy; the
			// returned D3D texture remains borrowed from that canonical owner.
			virtual IDirect3DBaseTexture8* AcquireTexture(const std::string& name) = 0;
			virtual void ApplyFragmentColor(MenuFragment& fragment, DWORD color) = 0;
			virtual void RebuildFragmentMaterial(MenuFragment& fragment) = 0;
			virtual void UpdatePictureDimensions(MenuNode& node) = 0; //0x4AAF10.
			virtual bool ResolveLabelText(DWORD reference, std::string& output) = 0;
			virtual void LayoutLabel(MenuNode& node) = 0; //0x4A93A0.
			virtual std::string GetConfiguration(MenuNode& node) = 0; //Vslot+208.
			virtual void SetConfiguration(MenuNode& node, const std::string& name) = 0;

			// Real MSVC library APIs; focused fixtures can observe departures here.
			virtual void* AllocateString(DWORD size);
			virtual void AssignString(std::string& output, const char* bytes, std::size_t count);
			virtual void FreeString(void* bytes);
		};

		// Production I/O always reloads the actual canonical ResourceSystem.
		// Remaining owner callbacks are deliberately still abstract.
		class ResourceMenuLayoutServices : public MenuLayoutServices
		{
		public:
			virtual File* Open(const char* name)
			{
				return Resources::g_pResourceSystem->OpenStream(name, 1);
			}
			virtual BYTE Read(void* output, DWORD size, File* file)
			{
				return Resources::g_pResourceSystem->Read(output, size, file);
			}
			virtual BYTE Seek(File* file, LONG offset, int origin)
			{
				return Resources::g_pResourceSystem->Seek(file, offset, origin);
			}
			virtual void Release(File* file)
			{
				Resources::g_pResourceSystem->Release(file, 0);
			}
		};

		BYTE ReadMenuString(std::string& output, MenuLayoutServices::File* file,
			MenuLayoutServices& services); //0x50B880.
		BYTE ReadMenuRect(RECT& output, BYTE& dirty, MenuLayoutServices::File* file,
			int version, MenuLayoutServices& services); //0x4B2850.
		BYTE ReadMenuFragment(MenuFragment& fragment, MenuLayoutServices::File* file,
			int version, MenuLayoutServices& services); //0x4B20A0.
		BYTE ReadMenuWindow(MenuNode& node, MenuLayoutServices::File* file,
			int version, MenuLayoutServices& services); //0x4B10F0.
		BYTE ReadMenuButton(MenuNode& node, MenuLayoutServices::File* file,
			int version, MenuLayoutServices& services); //0x4A6B60.
		BYTE ReadMenuPicture(MenuNode& node, MenuLayoutServices::File* file,
			int version, MenuLayoutServices& services); //0x4AB190.
		BYTE ReadMenuLabel(MenuNode& node, MenuLayoutServices::File* file,
			int version, MenuLayoutServices& services); //0x4AA640.
		BYTE LoadMenuLayout(MenuLayout& layout, const std::string& name,
			MenuLayoutServices& services); //0x495B50.
	}
}
