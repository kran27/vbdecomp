#include "game/StoreInventoryScreen.hpp"
#include "gui/Window.hpp"
#include "gui/ScrollBar.hpp"
#include "gui/Picture.hpp"
#include "gui/Label.hpp"
#include <cstdio>
#include <cstdlib>

namespace F3
{
	void StoreInventoryScreen::PlaceForStore(BYTE active)
	{
		if (active)
		{
			rootWindow->SetAlpha(0);
			rootWindow->SetAbsolutePosition(344, 388);
		}
	}

	void StoreInventoryScreen::ClearFirstEntries()
	{
		for (std::list<StoreScreenEntry*>::iterator i = firstEntries.begin();
			i != firstEntries.end(); ++i)
		{
			StoreScreenEntry* entry = *i;
			if (entry)
			{
				entry->~StoreScreenEntry();
				::operator delete(entry);
			}
		}
		firstEntries.clear();
	}

	void StoreInventoryScreen::ClearSecondEntries()
	{
		for (std::list<StoreScreenEntry*>::iterator i = secondEntries.begin();
			i != secondEntries.end(); ++i)
		{
			StoreScreenEntry* entry = *i;
			if (entry)
			{
				entry->~StoreScreenEntry();
				::operator delete(entry);
			}
		}
		secondEntries.clear();
	}

	void StoreInventoryScreen::UpdateFirstPictures()
	{
		const LONG offset = firstScrollbar->GetValue();
		std::list<StoreScreenEntry*>::iterator i = firstEntries.begin();
		for (LONG skipped = 0; i != firstEntries.end() && skipped != offset; ++skipped)
			++i;
		LONG index = 0;
		while (i != firstEntries.end() && index < 8)
		{
			StoreScreenEntry* entry = *i;
			if (entry)
				firstPictures[index]->LoadTexture(entry->texture, 0, 0.0f, 0.0f, -1.0f, -1.0f, 1);
			else
			{
				const std::string texture("icon_none.bmp");
				firstPictures[index]->LoadTexture(texture, 0, 0.0f, 0.0f, -1.0f, -1.0f, 1);
			}
			++i;
			++index;
		}
		for (; index < 8; ++index)
		{
			const std::string texture("icon_none.bmp");
			firstPictures[index]->LoadTexture(texture, 0, 0.0f, 0.0f, -1.0f, -1.0f, 1);
		}
	}

	void StoreInventoryScreen::UpdateSecondPictures()
	{
		const LONG offset = secondScrollbar->GetValue();
		std::list<StoreScreenEntry*>::iterator i = secondEntries.begin();
		for (LONG skipped = 0; i != secondEntries.end() && skipped != offset; ++skipped)
			++i;
		LONG index = 0;
		while (i != secondEntries.end() && index < 8)
		{
			StoreScreenEntry* entry = *i;
			if (entry)
				secondPictures[index]->LoadTexture(entry->texture, 0, 0.0f, 0.0f, -1.0f, -1.0f, 1);
			else
			{
				const std::string texture("icon_none.bmp");
				secondPictures[index]->LoadTexture(texture, 0, 0.0f, 0.0f, -1.0f, -1.0f, 1);
			}
			++i;
			++index;
		}
		for (; index < 8; ++index)
		{
			const std::string texture("icon_none.bmp");
			secondPictures[index]->LoadTexture(texture, 0, 0.0f, 0.0f, -1.0f, -1.0f, 1);
		}
	}

	void StoreInventoryScreen::UpdateFirstTotal()
	{
		DWORD total = 0;
		for (std::list<StoreScreenEntry*>::iterator i = firstEntries.begin();
			i != firstEntries.end(); ++i)
			if (*i) total += (*i)->value0;
		if (firstTotal)
		{
			char buffer[32];
			std::sprintf(buffer, "%d", LONG(total));
			const std::string text(buffer);
			firstTotal->SetText(text);
		}
	}

	void StoreInventoryScreen::Refresh()
	{
		ClearFirstEntries();
		ClearSecondEntries();
		firstScrollbar->SetValue(0, 1);
		secondScrollbar->SetValue(0, 1);
		UpdateFirstPictures();
		UpdateFirstTotal();
		UpdateSecondPictures();
		UpdateSecondTotal();
	}

	void StoreInventoryScreen::UpdateSecondTotal()
	{
		DWORD total = 0;
		for (std::list<StoreScreenEntry*>::iterator i = secondEntries.begin();
			i != secondEntries.end(); ++i)
			if (*i) total += (*i)->value0;
		if (secondTotal)
		{
			char buffer[32];
			std::sprintf(buffer, "%d", LONG(total));
			const std::string text(buffer);
			secondTotal->SetText(text);
		}
	}
}
