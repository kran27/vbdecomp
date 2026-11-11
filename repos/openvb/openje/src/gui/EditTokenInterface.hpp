#pragma once

#include "gui/EngineInterface.hpp"
#include <cstddef>

namespace JE
{
	// Actual Client+B8 owner, with the authored Main/Up/Down window children.
	class EditTokenInterface : public EngineInterface
	{
	public:
		explicit EditTokenInterface(const std::string& name); //5B3E60.
		virtual int OnEvent(::GUI::Window* window, ::GUI::Event* event); //5B42A0.
		void EditToken(DWORD stringReference, const char* name); //5B4020, actual modal setup.
		void Confirm(); //5B4130, publish the actual Edit text and remove the modal.

		std::string tokenName; //+34, original VC7 string; capacity at+4C.
		::GUI::Window* main; //+50.
		::GUI::Window* up; //+54.
		::GUI::Window* down; //+58.
	};
#if defined(_MSC_VER) && _MSC_VER == 1300 && defined(_M_IX86)
	typedef char EditTokenOriginalSize[(sizeof(EditTokenInterface) == 0x5C) ? 1 : -1];
	typedef char EditTokenOriginalName[(offsetof(EditTokenInterface, tokenName) == 0x34) ? 1 : -1];
	typedef char EditTokenOriginalMain[(offsetof(EditTokenInterface, main) == 0x50) ? 1 : -1];
	typedef char EditTokenOriginalUp[(offsetof(EditTokenInterface, up) == 0x54) ? 1 : -1];
	typedef char EditTokenOriginalDown[(offsetof(EditTokenInterface, down) == 0x58) ? 1 : -1];
#endif
}
