#include "game/StoreCatalogueScreen.hpp"
#include "gui/Window.hpp"
#include "gui/ScrollBar.hpp"
#include "gui/Picture.hpp"
#include "gui/Label.hpp"
#include <cstddef>
#include <cstdio>
#include <cstdlib>

#if defined(_MSC_VER) && _MSC_VER == 1300 && defined(_M_IX86)
//Only the complete entry and six-pointer row are established, not the screen.
typedef char StoreCatalogueEntrySize[(sizeof(F3::StoreCatalogueEntry) == 0x2C) ? 1 : -1];
typedef char StoreCatalogueTextureOffset[
	(offsetof(F3::StoreCatalogueEntry, texture) == 0x10) ? 1 : -1];
typedef char StoreCatalogueRowSize[(sizeof(F3::StoreCatalogueRow) == 0x18) ? 1 : -1];
typedef char StoreCatalogueRowAliasOffset[
	(offsetof(F3::StoreCatalogueRow, entry) == 0x14) ? 1 : -1];
#endif

namespace F3
{
	StoreCatalogueEntry::StoreCatalogueEntry()
	{
		//The real string is constructed before these original body stores.
		value0 = 0;
		value4 = 0;
		value8 = 0;
		valueC = 0;
		texture = "icon_unknown.bmp";
	}

	void StoreCatalogueScreen::ClearEntries()
	{
		for (std::list<StoreCatalogueEntry*>::iterator i = entries.begin();
			i != entries.end(); ++i)
		{
			StoreCatalogueEntry* entry = *i;
			if (entry)
			{
				entry->~StoreCatalogueEntry();
				::operator delete(entry);
			}
		}
		entries.clear();
	}

	void StoreCatalogueScreen::UpdateRows()
	{
		const LONG offset = scrollbar->GetValue();
		std::list<StoreCatalogueEntry*>::iterator i = entries.begin();
		for (LONG skipped = 0; i != entries.end() && skipped != offset; ++skipped)
			++i;
		LONG index = 0;
		while (i != entries.end() && index < 8)
		{
			StoreCatalogueEntry* entry = *i;
			if (entry)
			{
				rows[index].firstReference->SetStringReference(entry->value0);
				rows[index].secondReference->SetStringReference(entry->value4);
				{
					char buffer[32];
					std::sprintf(buffer, "%d", LONG(entry->value8));
					const std::string text(buffer);
					rows[index].number->SetText(text);
				}
				rows[index].picture->LoadTexture(entry->texture,
					0, 0.0f, 0.0f, -1.0f, -1.0f, 1);
				::GUI::Window* container = rows[index].container;
				rows[index].entry = entry;
				container->SetHidden(0);
			}
			else
				rows[index].container->SetHidden(1);
			++i;
			++index;
		}
		for (; index < 8; ++index)
			rows[index].container->SetHidden(1);
	}

	void StoreCatalogueScreen::BuildExamples()
	{
		ClearEntries();
		StoreCatalogueEntry* entry = new StoreCatalogueEntry;
		entry->value0 = 0xD08;
		entry->value4 = 0x104F;
		entry->value8 = 100;
		entry->texture = "icon_sword.bmp";
		entries.push_back(entry);
		entry = new StoreCatalogueEntry;
		entry->value0 = 0xD0A;
		entry->value4 = 0xD09;
		entry->value8 = 100;
		entry->texture = "icon_gem.bmp";
		entries.push_back(entry);
		entry = new StoreCatalogueEntry;
		entry->value0 = 0x1052;
		entry->value4 = 0x1051;
		entry->value8 = 100;
		entry->texture = "icon_bow.bmp";
		entries.push_back(entry);
	}

	void StoreCatalogueScreen::Configure(const char*)
	{
		BuildExamples();
		scrollbar->SetValue(0, 1);
		UpdateRows();
	}
}
