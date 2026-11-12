// Implementation fragment: compiled once by PipBoyCompilation.cpp.
#include "game/PipBoyInterface.hpp"
#include "audio/Sound.hpp"
#include "game/PipBoyAddNoteInterface.hpp"
#include "game/CounterControl.hpp"
#include "game/AreaMapWindow.hpp"
#include "game/GameCreature.hpp"
#include "game/GameWorldCollections.hpp"
#include "gui/Button.hpp"
#include "gui/Event.hpp"
#include "gui/Label.hpp"
#include "gui/Manager.hpp"
#include "gui/Picture.hpp"
#include "graphics/SystemInterface.hpp"
#include "graphics/StringFormat.hpp"
#include "startup/EngineStringSystem.hpp"
#include "entity/EntityEnvironment.hpp"
#include "entity/Gregorian.hpp"
#include "entity/Map.hpp"
#include "entity/World.hpp"
#include "model/CommonException.hpp"
#include <climits>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <new>
#include <stdexcept>

namespace
{
	static const char* DownloadName(LONG column, LONG row) //44A390, private name lookup.
	{
		if (column < 0 || column >= 2 || row < 0 || row >= 14) return NULL;
		return DownloadNames[row][column];
	}
	DWORD exploreShortcutHeld = 0; //Original707878, zero-initialized game debug latch.
	LONG Truncate(double value) { return LONG(static_cast<LONGLONG>(value)); }
	static const char* AlarmName(LONG index) //44A280, search stored indices.
	{
		for (LONG row = 0; row < 14; ++row)
			if (index == AlarmNames[row].index) return AlarmNames[row].name;
		return NULL;
	}
	static LONG AlarmHours(LONG index) //44A2A0, complete original54B decoded from PE.
	{
		switch (index)
		{
		case 2: return 1; case 3: return 2; case 4: return 3;
		case 5: return 4; case 6: return 5; case 7: return 6;
		default: return 0;
		}
	}
	static LONG AlarmMinutes(LONG index) //44A2F0, original private switch.
	{
		switch (index)
		{
		case 0: return 10;
		case 1: return 30;
		default: return 0;
		}
	}
	static void NormalizeAlarmTime(LONG& minutes, LONG& hours) //44A310, original signed division/remainder.
	{
		hours = (hours + minutes / 60) % 24;
		minutes %= 60;
	}
	std::string FormatAlarmSelection(LONG index) //44B000; optimized ECX/ESI ABI unproved.
	{
		std::string result, format, spare;
		switch (index)
		{
		case 0: case 1: case 2: case 3: case 4: case 5: case 6: case 7:
		{
			LONG hours = AlarmHours(index), minutes = AlarmMinutes(index);
			NormalizeAlarmTime(minutes, hours);
			F3::GetString(0x197, format);
			JE::FormatString(result, format.c_str(), hours, minutes);
			break;
		}
		case 8:
			F3::GetString(0x197, format);
			JE::FormatString(result, format.c_str(), 6, 0);
			break;
		case 9:
			F3::GetString(0x197, format);
			JE::FormatString(result, format.c_str(), 12, 0);
			break;
		case 10:
			F3::GetString(0x197, format);
			JE::FormatString(result, format.c_str(), 22, 0);
			break;
		case 11:
			F3::GetString(0x197, format);
			JE::FormatString(result, format.c_str(), 0, 0);
			break;
		case 12: F3::GetString(0x1A4, result); break;
		case 13: F3::GetString(0x1A5, result); break;
		default: break;
		}
		return result;
	}
}

namespace F3
{
	static LONG AlarmNameIndex(const std::string& name) //44A840, original private argument ABI remains unproved.
	{
		for (LONG index = 0; index < 14; ++index)
			if (!_stricmp(name.c_str(), AlarmNames[index].name)) return AlarmNames[index].index;
		return -1;
	}

	static LONG DownloadHolodiscIndex(const std::string& name, LONG top) //44AA30.
	{
		LONG row = 0;
		while (_stricmp(name.c_str(), DownloadNames[row][0]))
			if (++row >= 14) return -1;
		if (row + top <= 0) return -1;
		return row + top - 1;
	}

	static LONG DownloadVideoIndex(const std::string& name, LONG top, LONG count) //44AA70.
	{
		LONG row = 0;
		while (_stricmp(name.c_str(), DownloadNames[row][1]))
			if (++row >= 14) return -1;
		const LONG selected = row + top;
		if (count <= 0 || selected < 1 || selected >= count + 1) return -1;
		return selected - 1;
	}

	PipBoyInterface::PipBoyInterface(const std::string& name) : JE::EngineInterface(name), GameInterface(name) //455E40.
	{
		lilTextureWidth = lilTextureHeight = 0;
		lilPip1.x = lilPip1.y = lilPip1.width = lilPip1.height = 0;
		lilPip2.x = lilPip2.y = lilPip2.width = lilPip2.height = 0;
		value78 = value7C = -1;
		taskTop = taskScroll = 0;
		value98 = value9C = 0;
		mapMode = 0;
		valueA4 = valueA8 = -1;
		mapTop = valueB4 = valueB8 = valueBC = 0;
		primaryMap = secondaryMap = NULL;
		mapX = mapY = 0;
		minimumX = minimumY = LONG_MAX;
		maximumX = maximumY = LONG_MIN;
		noteIndex = -1;
		noteIconIndex = 1;
		mapZoomIndex = 2;
		cachedTexture = cachedMapID = 0;
		value1A4 = value1A8 = -1;
		downloadsTop = value1B4 = 0;
		value1B8 = value1BC = value1C0 = 0;
		holdDelay = 400;
		repeatDelay = 35;
		value4 = 0;
		//currentPage is genuinely unwritten here. Reset455A70 initializes it.
		//taskGroups has its real automatic map lifetime; labels remain a separate owner.

		LoadResource("PipBoy.INT"); //Original intentionally ignores the reader result.
		addNote = new PipBoyAddNoteInterface("AreaMapNote");
		addNote->SetHidden(1);

		::GUI::Picture* picture = static_cast< ::GUI::Picture*>(FindWindow("picLilPip1"));
		picture->GetAbsolutePosition(lilPip1.x, lilPip1.y);
		picture->GetSize(lilPip1.width, lilPip1.height);
		picture->SetTileMode(4);
		picture = static_cast< ::GUI::Picture*>(FindWindow("picLilPip2"));
		picture->GetAbsolutePosition(lilPip2.x, lilPip2.y);
		picture->GetSize(lilPip2.width, lilPip2.height);
		picture->SetTileMode(4);
		lilTextureWidth = picture->pictureWidth;
		lilTextureHeight = picture->pictureHeight;
		picture->GetTextureName(lilTextureName);

		::GUI::Label* label = static_cast< ::GUI::Label*>(FindWindow("lblAlarmClock"));
		DWORD reference = label->stringReference; //Actual4A7740 out-parameter getter.
		label->SetText(g_EmptyInterfaceText); //6F34FC, actual shared empty-string owner.
		alarmEmptyMetric = label->maximumRowWidth; //4A7790 returns original+E30.
		label->SetStringReference(reference);
		alarmColor = label->textColor; //4A7810 returns original+D64.
		labelReferences = static_cast<DWORD*>(::operator new(14 * sizeof(DWORD)));
		for (LONG index = 0; index < 14; ++index)
		{
			labelReferences[index] = DWORD(-1);
			label = static_cast< ::GUI::Label*>(FindWindow(AlarmName(index)));
			if (label) labelReferences[index] = label->stringReference;
		}
		taskColor = static_cast< ::GUI::Label*>(FindWindow("lblTasks1"))->textColor;
		mapsColor = static_cast< ::GUI::Label*>(FindWindow("lblMaps1"))->textColor;
		downloadsColor = static_cast< ::GUI::Label*>(FindWindow("lblDownloads1"))->textColor;
		for (LONG index = 0; index < 16; ++index)
		{
			const char* control = index < 14 ? AlarmName(index) :
				(index == 14 ? "lblRest" : "lblMapMode");
			::GUI::Window* window = FindWindow(control);
			if (window) window->SetValue3B(0);
		}

		dayCounter = new CounterControl;
		dayCounter->SetDigits(2);
		dayCounter->SetDigitRange(1, 0, 3);
		dayCounter->BindDigit(0, FindWindow("winDate1"));
		dayCounter->BindDigit(1, FindWindow("winDate10"));
		dayCounter->animationStep = 0.1f;

		monthCounter = new CounterControl;
		monthCounter->radix = 12; //Original direct+C8 write before SetDigits.
		monthCounter->SetDigits(1);
		monthCounter->SetDigitRange(0, 0, 11);
		monthCounter->digitTextures.resize(12); //Real STL resize433430, not a recovered library.
		monthCounter->SetMaximum(11);
		for (LONG index = 0; index < 12; ++index)
		{
			char filename[32];
			std::sprintf(filename, "f3_month%02d.tga", index + 1);
			monthCounter->digitTextures[index] = filename;
		}
		monthCounter->BindDigit(0, FindWindow("winMonth"));
		monthCounter->animationStep = 0.1f;

		yearCounter = new CounterControl;
		yearCounter->SetDigits(4);
		static const char* yearNames[4] =
			{"winYear1", "winYear10", "winYear100", "winYear1000"};
		for (LONG index = 0; index < 4; ++index)
			yearCounter->BindDigit(index, FindWindow(yearNames[index]));
		yearCounter->animationStep = 0.1f;

		timeCounter = new CounterControl;
		timeCounter->SetDigits(4);
		timeCounter->SetDigitRange(1, 0, 5);
		timeCounter->SetDigitRange(3, 0, 2);
		static const char* timeNames[4] =
			{"winMinute1", "winMinute10", "winHour1", "winHour10"};
		for (LONG index = 0; index < 4; ++index)
			timeCounter->BindDigit(index, FindWindow(timeNames[index]));
		timeCounter->animationStep = 0.1f;
	}

	PipBoyInterface::~PipBoyInterface() //455B90; original ordinary phase.
	{
		if (labelReferences) std::free(labelReferences); //Actual new/free pairing.
		if (primaryMap) delete primaryMap;
		if (secondaryMap) delete secondaryMap;
		if (dayCounter) delete dayCounter;
		if (monthCounter) delete monthCounter;
		if (yearCounter) delete yearCounter;
		if (timeCounter) delete timeCounter;
		if (cachedTexture) g_GfxSystem->DeleteTexture(cachedTexture);
		//Original omits deletion of addNote, a separately Manager-registered owner.
		//Normal members destroy cachedMapName, vectors, note, the nested task
		//maps/vectors and strings. TaskEntry pointers belong to the journal.
	}

	void PipBoyInterface::Open(DWORD entity) //455DC0.
	{
		value4 = entity;
		valueB8 = 0;
		valueB4 = 0;
		//Real VC7 vector::clear frees its allocation and zeroes all three pointers.
		areaIDs.clear();
		areaMaps.clear();
		currentPage = -1;
		SelectPage(0);
		RefreshClock(1);
	}

	void PipBoyInterface::Close() //44A3B0.
	{
		SetHidden(1);
		addNote->SetHidden(1);
		CounterControl* counters[4] =
			{dayCounter, monthCounter, yearCounter, timeCounter};
		for (LONG index = 0; index < 4; ++index) counters[index]->Finish();
	}

	void PipBoyInterface::RebuildTasks() //454C30.
	{
		taskGroups.clear();
		JE::IWorld* world = JE::g_World;
		GameJournal* journal = dynamic_cast<GameJournal*>(world->GetJournal());
		//The original snapshots the list of pointers and does not guard the
		//required Journal cast or delete the entries when that snapshot ends.
		std::list<JE::JournalEntry*> entries(journal->entries);
		for (std::list<JE::JournalEntry*>::iterator i = entries.begin(); i != entries.end(); ++i)
		{
			JE::TaskEntry* entry = dynamic_cast<JE::TaskEntry*>(*i);
			if (entry)
			{
				const LONG id = static_cast<LONG>(entry->value4);
				PipBoyTaskGroup& group = taskGroups[journal->GetTaskGroup(id)];
				if (journal->IsTaskOpen(id)) group.first[id].push_back(entry);
				else if (journal->IsTaskClosed(entry->value4)) group.second[id].push_back(entry);
			}
		}
	}

	void PipBoyInterface::Reset() //455A70.
	{
		value78 = value7C = -1;
		taskTop = taskScroll = 0;
		mapMode = 0;
		valueA4 = valueA8 = -1;
		mapTop = valueB4 = valueB8 = 0;
		value98 = value9C = 0;
		value1A4 = value1A8 = -1;
		downloadsTop = value1B4 = 0;
		value1B8 = value1BC = value1C0 = 0;
		holdDelay = 400;
		repeatDelay = 35;
		mapX = mapY = valueBC = 0;
		noteIndex = -1;
		noteIconIndex = 1;
		mapZoomIndex = 2;
		cachedMapID = 0;
		minimumX = minimumY = LONG_MAX;
		maximumX = maximumY = LONG_MIN;
		currentPage = -1;
		SelectPage(0); //Original resets scalars, not the embedded note or separate interfaces.
	}

	void PipBoyInterface::RefreshDownloads() //44C790.
	{
		value1B8 = 5;
		value1B4 = 0;
		if (value1A4 >= 0) value1B4 = value1A4 + 13;
		for (LONG index = 0; index < 14; ++index)
		{
			const LONG row = index + downloadsTop;
			const char* name = DownloadName(0, index);
			::GUI::Label* label = static_cast< ::GUI::Label*>(FindWindow(name));
			if (label)
			{
				LONG left, right, top, bottom;
				label->GetPadding(left, right, top, bottom);
				if (!row)
				{
					label->SetStringReference(417);
					label->SetValue3B(1);
					label->SetHidden(0);
					label->SetPadding(alarmEmptyMetric, right, top, bottom);
				}
				else if (row <= value1B8) //The original also accepts negative rows here.
				{
					const LONG selected = row - 1;
					std::string text;
					JE::FormatString(text, "Holodisc %d", row);
					label->SetText(text);
					label->SetValue3B(0);
					label->SetHidden(0);
					if (value1A4 == selected)
					{
						text = g_EmptyInterfaceText + text;
						label->SetTextColor(0xFF000080);
						label->SetText(text);
						label->SetPadding(0, right, top, bottom);
					}
					else
					{
						label->SetTextColor(downloadsColor);
						label->SetText(text);
						label->SetPadding(alarmEmptyMetric, right, top, bottom);
					}
				}
				else label->SetHidden(1);
				LONG width, height;
				label->GetSize(width, height);
				label->GetPadding(left, right, top, bottom);
				label->Resize(left + label->GetMaximumRowWidth(), height);
			}
		}
		for (LONG index = 0; index < 14; ++index)
		{
			const LONG row = index + downloadsTop;
			const char* name = DownloadName(1, index);
			::GUI::Label* label = static_cast< ::GUI::Label*>(FindWindow(name));
			if (label)
			{
				LONG left, right, top, bottom, width, height;
				label->GetPadding(left, right, top, bottom);
				label->GetSize(width, height);
				label->Resize(1024, height);
				const LONG videos = value1B4;
				if (videos > 0)
				{
					if (!row)
					{
						label->SetStringReference(418);
						label->SetValue3B(1);
						label->SetHidden(0);
						label->SetPadding(alarmEmptyMetric, right, top, bottom);
						label->SetTextColor(mapsColor); //The original header uses the map color.
					}
					else if (row >= 1 && row < videos + 1)
					{
						const LONG selected = row - 1;
						std::string text;
						JE::FormatString(text, "Video #%d", selected + 1);
						label->SetText(text);
						label->SetValue3B(0);
						label->SetHidden(0);
						if (value1A8 == selected)
						{
							text = g_EmptyInterfaceText + text;
							label->SetTextColor(0xFF000080);
							label->SetText(text);
							label->SetPadding(0, right, top, bottom);
						}
						else
						{
							label->SetTextColor(downloadsColor);
							label->SetText(text);
							label->SetPadding(alarmEmptyMetric, right, top, bottom);
						}
					}
					else if (value1A8 >= 0 &&
						row != videos + 1 && row == videos + 2)
					{
						label->SetStringReference(419);
						label->SetValue3B(0);
						label->SetHidden(0);
						label->SetPadding(alarmEmptyMetric, right, top, bottom);
						label->SetTextColor(downloadsColor);
					}
					else label->SetHidden(1);
				}
				else label->SetHidden(1);
				label->GetSize(width, height);
				label->GetPadding(left, right, top, bottom);
				label->Resize(height + label->GetMaximumRowWidth(), height);
			}
		}
	}

	void PipBoyInterface::SelectPage(DWORD page) //454DC0.
	{
		if (DWORD(currentPage) == page) return;
		currentPage = LONG(page); //Publication precedes every lookup/callback.
		static const char* names[8] = {"winAlarm", "winTasks1", "winTasks2", "winMaps1a",
			"winMaps1b", "winMaps2", "winDownloads1", "winDownloads2"};
		::GUI::Window* windows[8];
		for (LONG index = 0; index < 8; ++index) windows[index] = FindWindow(names[index]);
		for (LONG index = 0; index < 8; ++index)
		{
			BYTE hidden = 1;
			if (page == 1) hidden = index != 0;
			else if (page == 2) hidden = index != 1 && index != 2;
			else if (page == 3)
			{
				if (index == 3) hidden = mapMode;
				else if (index == 4) hidden = mapMode == 0;
				else if (index == 5) hidden = 0;
			}
			else if (page == 4) hidden = index != 6 && index != 7;
			windows[index]->SetHidden(hidden);
		}
		switch (page)
		{
		case 1: RefreshAlarm(); break;
		case 2:
			RebuildTasks();
			RefreshTaskLabels();
			RefreshTaskDescription();
			break;
		case 3: RefreshMaps(); SetupMapWindows(); break;
		case 4: RefreshDownloads(); break;
		default: ShowLilPip(1, 1); break;
		}
		RefreshButtons();
	}

	void PipBoyInterface::RefreshAlarm() //44E7D0.
	{
		LONG first = 0, second = 0;
		if (value4)
		{
			JE::Entity* entity = JE::GetEntityPtr(value4);
			GameCreature* creature = entity ? dynamic_cast<GameCreature*>(entity) : NULL;
			if (creature)
			{
				//Both original calls dispatch primary slot0; do not substitute currentHealth.
				first = creature->GetMaximumHealth();
				second = creature->GetMaximumHealth();
			}
		}
		std::string text, format;
		GetString(0x187, format);
		JE::FormatString(text, format.c_str(), first, second);
		::GUI::Label* health = static_cast< ::GUI::Label*>(FindWindow("lblHealth"));
		health->SetText(text.c_str()); //Original constructs another string from the C string.
		::GUI::Label* rest = static_cast< ::GUI::Label*>(FindWindow("lblRest"));
		LONG left, right, top, bottom;
		rest->GetPadding(left, right, top, bottom);
		rest->SetPadding(alarmEmptyMetric, right, top, bottom);
		::GUI::Label* until = static_cast< ::GUI::Label*>(FindWindow("lblUntil"));
		until->GetPadding(left, right, top, bottom);
		until->SetPadding(alarmEmptyMetric, right, top, bottom);
		RefreshAlarmChoices();
		ShowLilPip(1, 0);
	}

	void PipBoyInterface::RefreshAlarmChoices() //44B960.
	{
		for (LONG row = 0; row < 14; ++row)
		{
			const char* name = AlarmName(row); //44A280/6F3318 real name/index table.
			if (!name) continue;
			::GUI::Label* label = static_cast< ::GUI::Label*>(FindWindow(name));
			if (!label) continue;
			const LONG index = AlarmNameIndex(std::string(name));
			LONG left, right, top, bottom;
			label->GetPadding(left, right, top, bottom);
			if (!alarmText.empty() && !_stricmp(name, alarmText.c_str()))
			{
				std::string text;
				GetString(labelReferences[index], text);
				text = g_EmptyInterfaceText + text; //6F34FC, actual shared object.
				label->SetTextColor(0xFF000080);
				label->SetText(text);
				label->SetPadding(0, right, top, bottom);
			}
			else
			{
				label->SetTextColor(alarmColor);
				label->SetStringReference(labelReferences[index]);
				label->SetPadding(alarmEmptyMetric, right, top, bottom);
			}
			LONG width, height;
			label->GetSize(width, height);
			label->GetPadding(left, right, top, bottom);
			label->Resize(left + label->GetMaximumRowWidth(), height);
		}
		::GUI::Label* rest = static_cast< ::GUI::Label*>(FindWindow("lblRest"));
		::GUI::Label* until = static_cast< ::GUI::Label*>(FindWindow("lblUntil"));
		const LONG index = AlarmNameIndex(alarmText);
		if (index == -1)
		{
			rest->SetHidden(1);
			until->SetText(std::string(""));
		}
		else
		{
			rest->SetHidden(0);
			until->SetText(FormatAlarmSelection(index));
		}
	}

	void PipBoyInterface::ShowLilPip(BYTE shown, BYTE split) //44DEC0, actual RET8.
	{
		::GUI::Picture* first = static_cast< ::GUI::Picture*>(FindWindow("picLilPip1"));
		::GUI::Picture* second = static_cast< ::GUI::Picture*>(FindWindow("picLilPip2"));
		if (!shown) { first->SetHidden(1); second->SetHidden(1); return; }
		float scaleX, scaleY;
		::GUI::g_Manager.GetScale(scaleX, scaleY);
		LONG width = Truncate(double(lilTextureWidth) / scaleX);
		const LONG scaledHeight = Truncate(double(lilTextureHeight) / scaleY);
		if (!split)
		{
			LONG displayedHeight = scaledHeight, sourceWidth = lilTextureWidth;
			LONG sourceHeight = lilTextureHeight, sourceX = 0, sourceY = 0;
			if (width > lilPip2.width)
			{ sourceX = (width - lilPip2.width) / 2; width = lilPip2.width; }
			if (displayedHeight > lilPip2.height)
			{ sourceY = (displayedHeight - lilPip2.height) / 2; displayedHeight = lilPip2.height; }
			const double maximumWidth = lilPip2.width * double(scaleX);
			if (sourceWidth > maximumWidth) sourceWidth = Truncate(maximumWidth);
			const double maximumHeight = lilPip2.height * double(scaleY);
			if (sourceHeight > maximumHeight) sourceHeight = Truncate(maximumHeight);
			second->LoadTexture(lilTextureName, 0, float(sourceX), float(sourceY),
				float(sourceWidth), float(sourceHeight), 1);
			second->Resize(width, sourceHeight); //Original uses cropped source height here.
			second->SetAbsolutePosition(lilPip2.x + (lilPip2.width-width)/2,
				lilPip2.y + (lilPip2.height-sourceHeight)/2);
			first->SetHidden(1); second->SetHidden(0); return;
		}
		const LONG lower = lilTextureHeight / 2;
		const LONG upper = lilTextureHeight - lower;
		first->LoadTexture(lilTextureName, 0, 0, float(lower),
			float(lilTextureWidth), float(upper), 1);
		first->Resize(width, upper);
		first->SetAbsolutePosition(lilPip1.x + (lilPip1.width - width) / 2, lilPip1.y);
		second->LoadTexture(lilTextureName, 0, 0, 0, float(lilTextureWidth), float(lower), 1);
		second->Resize(width, lower);
		second->SetAbsolutePosition(lilPip2.x + (lilPip2.width - width) / 2,
			lilPip2.y + lilPip2.height - lower);
		first->SetHidden(0);
		second->SetHidden(0);
		//Known Picture+130 RET28 keeps ESP balanced; stale IDA labels invented EDI.
	}

	void PipBoyInterface::RefreshDownloadDescription() //44EE00.
	{
		std::string text;
		if (value1A4 >= 0) JE::FormatString(text, "Holodisc #%d", value1A4 + 1);
		::GUI::Label* label = static_cast< ::GUI::Label*>(FindWindow("lblDownloads2"));
		label->SetText(text);
		ShowLilPip(text.empty(), 0);
	}

	LONG PipBoyInterface::GetTaskRowCount() const //44A6C0.
	{
		const LONG openRows = LONG(value98);
		LONG separator;
		if (openRows > 0 && LONG(value9C) > 0) separator = 1;
		else separator = 0;
		const LONG closedRows = LONG(value9C);
		const LONG groupRows = LONG(taskGroups.size()) + 1;
		const LONG rows = closedRows + (closedRows > 0) + openRows + (openRows > 0);
		if (groupRows > rows + separator) return groupRows;
		return LONG(openRows > 0 && closedRows > 0) + rows;
	}

	LONG PipBoyInterface::GetMapRowCount() const //44A420.
	{
		LONG rows = valueB4 + 7;
		const LONG areaRows = valueB8 + 1;
		if (areaRows > rows) rows = areaRows;
		return rows;
	}

	LONG PipBoyInterface::GetDownloadRowCount() const //44A440.
	{
		const LONG videos = LONG(value1B4);
		LONG videoRows;
		if (videos > 0) videoRows = videos + 3;
		else videoRows = 0;
		const LONG holodiscRows = LONG(value1B8) + 1;
		if (holodiscRows > videoRows) return holodiscRows;
		if (videos > 0) return videos + 3;
		return 0;
	}

	void PipBoyInterface::RefreshButtons() //44CEA0.
	{
		::GUI::Button* left = static_cast< ::GUI::Button*>(FindWindow("btnLeft"));
		::GUI::Button* right = static_cast< ::GUI::Button*>(FindWindow("btnRight"));
		::GUI::Button* up1 = static_cast< ::GUI::Button*>(FindWindow("btnUp1"));
		::GUI::Button* down1 = static_cast< ::GUI::Button*>(FindWindow("btnDown1"));
		::GUI::Button* up2 = static_cast< ::GUI::Button*>(FindWindow("btnUp2"));
		::GUI::Button* down2 = static_cast< ::GUI::Button*>(FindWindow("btnDown2"));
		if (currentPage == 2)
		{
			left->SetEnabled(0);
			right->SetEnabled(0);
			const LONG rows = GetTaskRowCount();
			up1->SetEnabled(taskTop > 0);
			down1->SetEnabled(rows > 14 && taskTop < rows - 14);
			::GUI::Label* label = static_cast< ::GUI::Label*>(FindWindow("lblTasks2"));
			LONG width, height, paddingLeft, paddingRight, paddingTop, paddingBottom;
			label->GetSize(width, height);
			label->GetPadding(paddingLeft, paddingRight, paddingTop, paddingBottom);
			width -= paddingRight + paddingLeft;
			height -= paddingBottom + paddingTop;
			if (width < 0) width = 0;
			if (height < 0) height = 0;
			const LONG textHeight = label->GetTextHeight();
			if (textHeight > height)
			{
				up2->SetEnabled(taskScroll > 0);
				down2->SetEnabled(taskScroll < textHeight - height - 1);
			}
			else
			{
				up2->SetEnabled(0);
				down2->SetEnabled(0);
			}
			return;
		}
		if (currentPage == 3)
		{
			left->SetEnabled(mapX > minimumX);
			right->SetEnabled(mapX < maximumX);
			up2->SetEnabled(mapY > minimumY);
			down2->SetEnabled(mapY < maximumY);
			if (mapMode)
			{
				up1->SetEnabled(mapY > minimumY);
				down1->SetEnabled(mapY < maximumY);
			}
			else
			{
				const LONG rows = GetMapRowCount();
				up1->SetEnabled(mapTop > 0);
				down1->SetEnabled(rows > 14 && mapTop < rows - 14);
			}
			return;
		}
		if (currentPage == 4)
		{
			left->SetEnabled(0);
			right->SetEnabled(0);
			up2->SetEnabled(0);
			down2->SetEnabled(0);
			const LONG rows = GetDownloadRowCount();
			up1->SetEnabled(downloadsTop > 0);
			down1->SetEnabled(rows > 14 && downloadsTop < rows - 14);
			return;
		}
		left->SetEnabled(0);
		right->SetEnabled(0);
		up1->SetEnabled(0);
		down1->SetEnabled(0);
		up2->SetEnabled(0);
		down2->SetEnabled(0);
	}

	void PipBoyInterface::RefreshClock(BYTE force) //44A590.
	{
		if (!(g_pNetworkInstance->GetMode() & 1))
			throw ::Common::Exception("Invalid attempt to retrieve world server instance.");
		JE::IWorldServer* server = JE::g_World;
		JE::Gregorian* calendar = server->GetCalendar();
		const DWORD year = calendar->GetYear();
		const DWORD month = calendar->GetMonth();
		const DWORD day = calendar->GetDay();
		const DWORD hour = calendar->GetHour();
		const DWORD minute = calendar->GetMinute();
		const DWORD mode = force ? 2 : 0;
		dayCounter->SetValue(LONG(day), mode);
		monthCounter->SetValue(LONG(month - 1), mode);
		yearCounter->SetValue(LONG(year), mode);
		timeCounter->SetValue(LONG(minute + 100 * hour), mode);
		if (hour <= 20 && hour > 0) timeCounter->SetDigitRange(2, 0, 9);
		else timeCounter->SetDigitRange(2, 0, 3);
	}

	void PipBoyInterface::BeforeUpdate() //455DA0 ->4518A0.
	{
		if (::GUI::g_Manager.HasModal() && ::GUI::g_Manager.FrontModal() != this)
			value1BC = value1C0 = 0;
		if ((GetAsyncKeyState(VK_DELETE) & 0x8000) &&
			(GetAsyncKeyState(VK_MENU) & 0x8000) && !exploreShortcutHeld)
		{
			exploreShortcutHeld = 1;
			JE::Map* map = JE::g_World->GetMap(JE::g_World->valueE0);
			if (map->areaMap)
			{
				const LONG width = map->areaMap->width, height = map->areaMap->height;
				for (LONG row = 0; row < height; ++row)
					for (LONG column = 0; column < width; ++column)
						map->SetExploredCell(column, row, GetAsyncKeyState(VK_SHIFT) == 0);
			}
		}
		if (!(GetAsyncKeyState(VK_DELETE) & 0x8000) && !(GetAsyncKeyState(VK_MENU) & 0x8000))
			exploreShortcutHeld = 0;
		RefreshClock(0);
		CounterControl* counters[4] = {dayCounter, monthCounter, yearCounter, timeCounter};
		for (LONG index = 0; index < 4; ++index) counters[index]->Update();
		if (currentPage == 3)
		{
			//451A1E captures the borrowed selection before RefreshMaps callbacks.
			JE::AreaMap* selected = SelectedAreaMap();
			const DWORD mapID = JE::g_World->GetCurrentMapID();
			JE::Map* activeMap = JE::g_World->GetMap(mapID);
			if (activeMap && (cachedMapID != mapID ||
				cachedMapName.compare(0, cachedMapName.size(), activeMap->octreeName.c_str(),
					std::strlen(activeMap->octreeName.c_str())) != 0))
				RefreshMaps();
			if (!selected)
			{
				if (secondaryMap && secondaryMap->areaMap)
				{
					RefreshMaps();
					SetupMapWindows();
				}
			}
			else if (activeMap && (cachedMapID != mapID ||
				cachedMapName.compare(0, cachedMapName.size(), activeMap->octreeName.c_str(),
					std::strlen(activeMap->octreeName.c_str())) != 0))
				SetupMapWindows();
			else if (selected->changes & 1)
			{
				if (activeMap && activeMap->areaMap != selected) RefreshMapMask();
				QueryMapBounds();
				ClampMapOrigin(0, 0, 0);
			}
		}
		RefreshClock(0); //Actual ordinary tail repeats the calendar getter/writes.
	}

	int PipBoyInterface::OnEvent(::GUI::Window* window, ::GUI::Event* event) //455DB0, shared4554B0 body.
	{
		if (event->type == 4 || event->type == 7)
		{
			AreaMapWindow* map = dynamic_cast<AreaMapWindow*>(window);
			if (map) HandleMapInput(map, event->x, event->y);
			else if (window->type == 1) HandleButtonInput(window, event->type);
			else if (window->type == 3)
			{
				JE::CSound* sound = new JE::CSound;
				if (sound)
				{
					sound->Load("f3_button_down_01.wav", 0);
					sound->Play(2, 0, 0);
				}
				HandleLabelInput(window, event->type);
			}
			return JE::EngineInterface::OnEvent(window, event);
		}
		switch (event->type)
		{
		case 6:
			if (window->type == 1)
			{
				if (!_stricmp(window->GetName(), "btnAlarm")) SelectPage(1);
				else if (!_stricmp(window->GetName(), "btnTasks")) SelectPage(2);
				else if (!_stricmp(window->GetName(), "btnMaps")) SelectPage(3);
				else if (!_stricmp(window->GetName(), "btnDownloads")) SelectPage(4);
				else if (!_stricmp(window->GetName(), "btnSwitch") || !_stricmp(window->GetName(), "btnSwitch")) Close();
			}
			return JE::EngineInterface::OnEvent(window, event);
		case 0:
			if (window->type == 1 && static_cast< ::GUI::Button*>(window)->GetState() == 2)
				HandleButtonInput(window, event->type);
			break;
		case 2:
			if (currentPage == 3)
			{
				UpdateDraggedNote(event->x, event->y);
				if (secondaryMap && !secondaryMap->Contains(event->x, event->y))
				{
					secondaryMap->UpdatePartyHover(event->x, event->y);
					secondaryMap->UpdateNoteHover(event->x, event->y);
				}
				if (mapMode && primaryMap && !primaryMap->Contains(event->x, event->y))
				{
					primaryMap->UpdatePartyHover(event->x, event->y);
					primaryMap->UpdateNoteHover(event->x, event->y);
				}
				UpdateNotePopup(event->x, event->y);
			}
			break;
		case 17:
			if (!GetHidden())
			{
				::GUI::Window* root = FindWindow("picPipBoy");
				if (window == root)
				{
					if (event->x == 1) { Close(); return 1; }
					if (currentPage == 3 && (event->x == 71 || event->x == 199 || event->x == 327))
					{ CenterMap(); return 1; }
				}
			}
			break;
		}
		if (event->type == 16 && currentPage == 3)
		{
			AreaMapWindow* map;
			if (window == secondaryMap && secondaryMap->Contains(event->x, event->y)) map = secondaryMap;
			else if (window == primaryMap && mapMode && primaryMap->Contains(event->x, event->y)) map = primaryMap;
			else return JE::EngineInterface::OnEvent(window, event);
			float scaleX, scaleY;
			::GUI::g_Manager.GetScale(scaleX, scaleY);
			const LONG originX = map->originX, originY = map->originY;
			LONG left, top;
			map->GetAbsolutePosition(left, top);
			const float zoom = g_PipBoyMapZooms[mapZoomIndex];
			const LONG anchorX = LONG(static_cast<LONGLONG>(((event->x - left) * scaleX + originX) * zoom));
			const LONG anchorY = LONG(static_cast<LONGLONG>(((event->y - top) * scaleY + originY) * zoom));
			const LONG direction = event->wheel > 0 ? -1 : event->wheel < 0;
			SetMapZoom(DWORD(mapZoomIndex + direction), 1, anchorX, anchorY);
		}
		return JE::EngineInterface::OnEvent(window, event);
	}

}
