#include "Manager.hpp"
#include "Interface.hpp"
#include "Skin.hpp"
#include "Font.hpp"
#include "PacketManager.hpp"
#include "PacketStorage.hpp"
#include "graphics/CameraController.hpp"
#include "graphics/Viewport.hpp"
#include <stdexcept>

namespace GUI
{
	Manager::Manager()
		: depthDirty(1), hoveredWindow(0), pressedLeft(0), pressedRight(0), pressedMiddle(0), inputBlocked(0),
		resolutionWidth(1024), resolutionHeight(768), scaleX(1.0f), scaleY(1.0f),
		view(0), camera(0), capturedWindow(0)
	{
		// hoveredOwner and mouse.wheel are deliberately not initialized here.
		// Real members finish construction before these original body actions.
		tooltip = "";
		modal.clear();
		callback172 = 0;
		storage = new cls_0x4b6910(0x80000);
		packets = new CPktManager;
		view = new Gfx_Viewport;
		camera = new Gfx_CamCtrl_Orbit;
		entries148.clear();
		ResetMouseState();
	}

	Manager::~Manager()
	{
		Cleanup();
		Gfx_Viewport* capturedView = view;
		if (capturedView)
		{
			delete capturedView;
			view = 0;
		}
		if (camera)
		{
			delete camera;
			camera = 0;
		}
		if (packets)
		{
			delete packets;
			packets = 0;
		}
		cls_0x4b6910* capturedStorage = storage;
		if (capturedStorage)
		{
			delete capturedStorage;
			storage = 0;
		}
		// Actual members then destroy modal, entries, tooltip, fonts, mask cache,
		// skins and interfaces in reverse declaration order.
	}

	void Manager::Cleanup()
	{
		depthDirty = 1;
		hoveredWindow = 0;
		capturedWindow = 0;
		std::list<Interface*>::iterator entry = interfaces.begin();
		while (entry != interfaces.end())
		{
			Interface* value = *entry;
			entry = interfaces.erase(entry);
			// Original queries HeapTracker before testing the captured pointer.
			// Calling a nonstatic method on NULL remains outside the C++ domain.
			if (value->IsHeapAllocated() && value)
			{
				delete value;
			}
		}
		interfaces.clear();
		ResetMouseState();
		textureMasks.Clear();
		for (std::map<std::string, Skin*>::iterator skin = skins.begin();
			skin != skins.end(); ++skin)
		{
			if (skin->second)
			{
				delete skin->second;
			}
		}
		skins.clear();
		for (std::list<Font*>::iterator font = fonts.begin(); font != fonts.end(); ++font)
		{
			if (*font)
			{
				delete *font;
			}
		}
		fonts.clear();
		resolutionWidth = 1024;
		resolutionHeight = 768;
		scaleX = 1.0f;
		scaleY = 1.0f;
		packets->Clear();
		storage->Reset();
		entries148.clear();
		modal.clear();
		tooltip = "";
		// hoveredOwner, pressed-window pointers, inputBlocked, wheel and callback172 survive.
	}
}
