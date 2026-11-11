#pragma once

#include "gui/Interface.hpp"
#include <stddef.h>

namespace GUI { class Label; }
namespace JE { class CSound; }

namespace F3
{
	class LoadMapInterface : public ::GUI::Interface
	{
	public:
		explicit LoadMapInterface(const std::string& name); //440050, direct Interface base.
		virtual ~LoadMapInterface(); //43FF40.
		virtual int OnEvent(::GUI::Window* window, ::GUI::Event* event); //4404A0.

		::GUI::Label* mapNameEdit; //Original+34, borrowed serialized MapNameEdit.
		BYTE clearOnFirstCharacter; //Original+38, raw BYTE;39..3B unwritten.
		JE::CSound* clickSound; //Original+3C, owned raw allocation.
	private:
		LoadMapInterface(const LoadMapInterface&);
		LoadMapInterface& operator=(const LoadMapInterface&);
	};
}

#if defined(_MSC_VER) && _MSC_VER == 1300 && defined(_M_IX86)
typedef char F3_LoadMap_Size[(sizeof(F3::LoadMapInterface) == 0x40) ? 1 : -1];
typedef char F3_LoadMap_SoundOffset[(offsetof(F3::LoadMapInterface, clickSound) == 0x3C) ? 1 : -1];
#endif
