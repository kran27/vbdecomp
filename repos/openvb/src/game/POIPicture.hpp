#pragma once

#include "gui/Picture.hpp"
#include <cstddef>

namespace GUI { class Label; }
namespace JE { struct WorldMapLocation; }

//The original RTTI identifies a global POIPicture, not a GUI or JE class.
class POIPicture : public GUI::Picture
{
public:
	POIPicture(); //458780.
	virtual ~POIPicture(); //4587C0.

	JE::WorldMapLocation* location; //D2C, borrowed.
	GUI::Label* label; //D30, alias of an attached child; no separate delete here.
	LONG coordinateX, coordinateY; //D34/D38, signed map coordinates.
};

#if defined(_MSC_VER) && _MSC_VER == 1300
typedef char POIPictureOriginalSize[(sizeof(POIPicture) == 0xD3C) ? 1 : -1];
typedef char POIPictureOriginalLocation[(offsetof(POIPicture, location) == 0xD2C) ? 1 : -1];
typedef char POIPictureOriginalLabel[(offsetof(POIPicture, label) == 0xD30) ? 1 : -1];
typedef char POIPictureOriginalX[(offsetof(POIPicture, coordinateX) == 0xD34) ? 1 : -1];
typedef char POIPictureOriginalY[(offsetof(POIPicture, coordinateY) == 0xD38) ? 1 : -1];
#endif
