// Implementation fragment: compiled once by PipBoyCompilation.cpp.
#include "game/PipBoyInterface.hpp"
#include <cstring>
#include "gui/Label.hpp"
#include "graphics/StringFormat.hpp"
#include "startup/EngineStringSystem.hpp"
#include "entity/WorldCollections.hpp"

namespace
{
	//6F3388: original interleaved names, indexed as row*2+column.
	static const char* TaskLabelName(LONG column, LONG row) //44A350, private compiler argument convention.
	{
		if (column < 0 || column >= 2 || row < 0 || row >= 14) return NULL;
		return TaskLabelNames[row][column];
	}
}

namespace F3
{
	static LONG TaskGroupIndex(const std::string& name, LONG top) //44A880.
	{
		LONG row = 0;
		while (_stricmp(name.c_str(), TaskLabelNames[row][0]))
			if (++row >= 14) return -1;
		if (row + top <= 0) return -1;
		return row + top - 1;
	}

	static LONG TaskEntryIndex(const std::string& name, LONG top, LONG openCount, LONG closedCount) //44A8C0.
	{
		LONG row = 0;
		while (_stricmp(name.c_str(), TaskLabelNames[row][1]))
			if (++row >= 14) return -1;
		LONG selected = row + top;
		if (openCount <= 0 || selected < 1 || selected > openCount)
		{
			if (closedCount <= 0) return -1;
			const LONG gap = openCount <= 0 ? 0 : 2;
			if (selected < gap + openCount + 1 || selected > gap + openCount + closedCount + 1)
				return -1;
			selected -= gap;
		}
		return selected - 1;
	}

	void PipBoyInterface::RefreshTaskLabels() //44BE60.
	{
		value98 = 0;
		value9C = 0;
		PipBoyTaskGroups::iterator group = taskGroups.begin();
		PipBoyTaskIndex* openTasks = NULL;
		PipBoyTaskIndex* closedTasks = NULL;

		for (DWORD row = 0; row < 14; ++row)
		{
			const LONG displayed = LONG(row + taskTop);
			::GUI::Label* label = static_cast< ::GUI::Label*>(FindWindow(TaskLabelName(0, LONG(row))));
			if (label)
			{
				LONG left, right, top, bottom;
				label->GetPadding(left, right, top, bottom);
				if (displayed == 0)
				{
					label->SetStringReference(0x198);
					label->SetValue3B(1);
					label->SetHidden(0);
					label->SetPadding(alarmEmptyMetric, right, top, bottom);
				}
				else if (displayed <= LONG(taskGroups.size()))
				{
					std::string text;
					JE::FormatString(text, "Area %d", group->first);
					label->SetText(text);
					label->SetValue3B(0);
					label->SetHidden(0);
					if (value78 == displayed - 1)
					{
						openTasks = &group->second.first;
						closedTasks = &group->second.second;
						value98 = DWORD(openTasks->size());
						value9C = DWORD(closedTasks->size());
						text = g_EmptyInterfaceText + text;
						label->SetTextColor(0xFF000080);
						label->SetText(text);
						label->SetPadding(0, right, top, bottom);
					}
					else
					{
						label->SetTextColor(taskColor);
						label->SetText(text);
						label->SetPadding(alarmEmptyMetric, right, top, bottom);
					}
					//Original advances only for a present, displayed group label;
					//taskTop does not first advance this iterator.
					++group;
				}
				else label->SetHidden(1);

				LONG width, height;
				label->GetSize(width, height);
				label->GetPadding(left, right, top, bottom);
				label->Resize(left + label->GetMaximumRowWidth(), height);
			}
		}

		PipBoyTaskIndex::iterator openTask;
		PipBoyTaskIndex::iterator closedTask;
		if (openTasks) openTask = openTasks->begin();
		if (closedTasks) closedTask = closedTasks->begin();
		for (DWORD row = 0; row < 14; ++row)
		{
			const LONG displayed = LONG(row + taskTop);
			::GUI::Label* label = static_cast< ::GUI::Label*>(FindWindow(TaskLabelName(1, LONG(row))));
			if (label)
			{
				LONG left, right, top, bottom;
				label->GetPadding(left, right, top, bottom);
				LONG width, height;
				label->GetSize(width, height);
				label->Resize(1024, height);
				const LONG openCount = LONG(value98);
				if (displayed != 0)
				{
					if (openCount > 0)
					{
						if (displayed >= 1 && displayed < openCount + 1)
						{
							std::string text;
							GetString(DWORD(openTask->first), text);
							label->SetValue3B(0);
							label->SetHidden(0);
							if (value7C == displayed - 1)
							{
								text = g_EmptyInterfaceText + text;
								label->SetTextColor(0xFF000080);
								label->SetText(text);
								label->SetPadding(0, right, top, bottom);
							}
							else
							{
								label->SetTextColor(taskColor);
								label->SetText(text);
								label->SetPadding(alarmEmptyMetric, right, top, bottom);
							}
							++openTask;
							goto resizeTask;
						}
						if (LONG(value9C) <= 0 || displayed == openCount + 1) goto hideTask;
					}
				}
				else if (openCount > 0)
				{
					label->SetStringReference(0x1A6);
					label->SetValue3B(1);
					label->SetHidden(0);
					label->SetPadding(alarmEmptyMetric, right, top, bottom);
					label->SetTextColor(taskColor);
					goto resizeTask;
				}

				if (LONG(value9C) > 0)
				{
					const LONG gap = openCount <= 0 ? 0 : 2;
					if (displayed == gap + openCount)
					{
						label->SetStringReference(0x1A7);
						label->SetValue3B(1);
						label->SetHidden(0);
						label->SetPadding(alarmEmptyMetric, right, top, bottom);
						label->SetTextColor(taskColor);
						goto resizeTask;
					}
					if (displayed >= gap + openCount + 1 && displayed < gap + LONG(value9C) + openCount + 1)
					{
						const LONG index = displayed - gap - openCount - 1;
						std::string text;
						GetString(DWORD(closedTask->first), text);
						label->SetValue3B(0);
						label->SetHidden(0);
						if (value7C == index + LONG(value98))
						{
							text = g_EmptyInterfaceText + text;
							label->SetTextColor(0xFF000080);
							label->SetText(text);
							label->SetPadding(0, right, top, bottom);
						}
						else
						{
							label->SetTextColor(taskColor);
							label->SetText(text);
							label->SetPadding(alarmEmptyMetric, right, top, bottom);
						}
						++closedTask;
						goto resizeTask;
					}
				}
			hideTask:
				label->SetHidden(1);
			resizeTask:
				label->GetSize(width, height);
				label->GetPadding(left, right, top, bottom);
				label->Resize(left + label->GetMaximumRowWidth(), height);
			}
		}
	}

	void PipBoyInterface::RefreshTaskDescription() //44EB80.
	{
		std::string text;
		LONG taskIndex = value7C;
		if (taskIndex >= 0)
		{
			LONG groupIndex = value78;
			if (groupIndex >= 0 && groupIndex < LONG(taskGroups.size()))
			{
				PipBoyTaskGroups::iterator group = taskGroups.begin();
				while (groupIndex > 0) { ++group; --groupIndex; }
				const LONG openCount = LONG(group->second.first.size());
				if (taskIndex < openCount + LONG(group->second.second.size()))
				{
					PipBoyTaskIndex::iterator task;
					if (taskIndex >= openCount)
					{
						task = group->second.second.begin();
						const LONG closedIndex = taskIndex - openCount;
						for (LONG index = 0; index < closedIndex; ++index) ++task;
					}
					else
					{
						task = group->second.first.begin();
						while (taskIndex > 0) { ++task; --taskIndex; }
					}
					PipBoyTaskEntries& entries = task->second;
					for (PipBoyTaskEntries::iterator entry = entries.begin(); entry != entries.end(); ++entry)
					{
						std::string entryText;
						GetString((*entry)->value8, entryText);
						if (entry != entries.begin()) text.append("\n\n");
						text.append(entryText);
					}
				}
			}
		}
		::GUI::Label* label = static_cast< ::GUI::Label*>(FindWindow("lblTasks2"));
		label->SetText(text);
		label->SetScroll(0, taskScroll);
		ShowLilPip(text.empty(), 0);
		RefreshButtons();
	}
}
