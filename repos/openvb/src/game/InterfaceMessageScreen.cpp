#include "game/InterfaceMessageScreen.hpp"
#include "gui/Label.hpp"
#include "gui/Picture.hpp"

namespace F3
{
	void InterfaceMessageRow::SetStringReference(DWORD reference)
	{
		if (label) label->SetStringReference(reference);
	}

	void InterfaceMessageRow::LoadTexture(const std::string& filename)
	{
		if (picture)
			picture->LoadTexture(filename, 0, 0.0f, 0.0f, -1.0f, -1.0f, 1);
	}

	void InterfaceMessageScreen::SetMessage(DWORD reference, const char* filename)
	{
		if (row)
		{
			row->SetStringReference(reference);
			const std::string image(filename);
			//4338A9 reloads the row after the Label callback and string construction.
			row->LoadTexture(image);
		}
	}
}
