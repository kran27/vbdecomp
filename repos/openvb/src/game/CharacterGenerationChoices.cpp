#include "game/CharacterGenerationChoices.hpp"
#include "gui/Button.hpp"
#include "gui/Event.hpp"
#include "gui/Label.hpp"
#include "gui/Window.hpp"
#include <mmsystem.h>

namespace F3
{
	CharacterGenerationChoices::CharacterGenerationChoices() //42A6A0.
		: previousButton(NULL), nextButton(NULL), selected(-1), firstVisible(0), dirty(1),
		pressTime(0), repeatTime(0), holdDelay(400), repeatDelay(35)
	{
	}
	CharacterGenerationChoices::~CharacterGenerationChoices() //42A710.
	{
	}
	void CharacterGenerationChoices::Clear() //42A770/429B70.
	{
		entries.clear();
		dirty = 1;
	}
	void CharacterGenerationChoices::SetRowCount(LONG count) //42A740.
	{
		if (count < 0) count = 0;
		CharacterGenerationChoiceRow empty = {NULL, NULL};
		rows.resize(count, empty);
		dirty = 1;
	}
	void CharacterGenerationChoices::BindRow(LONG index, ::GUI::Window* button,
		::GUI::Label* label) //425200.
	{
		if (index >= 0 && index < LONG(rows.size()))
		{
			CharacterGenerationChoiceRow& row = rows[index];
			row.button = button;
			row.label = label;
			dirty = 1;
		}
	}
	void CharacterGenerationChoices::Add(DWORD reference, LONG value, DWORD value8,
		DWORD valueC, const std::string& text) //42CCF0.
	{
		CharacterGenerationChoice choice;
		choice.reference = reference;
		choice.value = value;
		choice.value8 = value8;
		choice.valueC = valueC;
		choice.text = text;
		entries.push_back(choice); //42CC60 is the actual standard vector value append.
		dirty = 1;
	}
	BYTE CharacterGenerationChoices::SetSelection(LONG index) //424DF0.
	{
		if (index < 0) index = -1;
		else
		{
			LONG last = LONG(entries.size()) - 1;
			if (last < 0) last = 0;
			if (index > last) index = -1;
		}
		if (selected != index)
		{
			selected = index;
			dirty = 1;
			return 1;
		}
		return 0;
	}
	BYTE CharacterGenerationChoices::SetFirstVisible(LONG index) //424E50.
	{
		if (index < 0) index = 0;
		else
		{
			LONG last = LONG(entries.size()) - LONG(rows.size());
			if (last < 0) last = 0;
			if (index > last) index = last;
		}
		if (firstVisible != index)
		{
			firstVisible = index;
			dirty = 1;
			return 1;
		}
		return 0;
	}
	LONG CharacterGenerationChoices::GetFirstVisible() const { return firstVisible; }
	void CharacterGenerationChoices::EnsureSelectionVisible() //425000..425120 common tail.
	{
		if (selected < firstVisible) SetFirstVisible(selected);
		else if (selected >= LONG(rows.size()) + firstVisible)
			SetFirstVisible(selected - LONG(rows.size()) + 1);
	}
	void CharacterGenerationChoices::BindArrows(::GUI::Window* previous, ::GUI::Window* next)
	{
		previousButton = previous;
		nextButton = next;
	}
	BYTE CharacterGenerationChoices::Process(::GUI::Window* window, ::GUI::Event* event)
	{
		if (event->type == 4 || event->type == 7)
		{
			if (window->type == 1)
			{
				if (window == previousButton)
				{
					pressTime = repeatTime = timeGetTime();
					return SetFirstVisible(firstVisible - 1);
				}
				if (window == nextButton)
				{
					pressTime = repeatTime = timeGetTime();
					return SetFirstVisible(firstVisible + 1);
				}
			}
		}
		else if (!event->type)
		{
			if (window == previousButton)
			{
				if (static_cast<::GUI::Button*>(window)->GetState() == 2)
				{
					const DWORD now = timeGetTime();
					if (pressTime + holdDelay < now && repeatTime + repeatDelay < now)
						return SetFirstVisible(firstVisible - 1);
				}
				return 0;
			}
			if (window == nextButton && static_cast<::GUI::Button*>(window)->GetState() == 2)
			{
				const DWORD now = timeGetTime();
				if (pressTime + holdDelay < now && repeatTime + repeatDelay < now)
				{
					repeatTime = now;
					return SetFirstVisible(firstVisible + 1);
				}
			}
		}
		return 0;
	}
	void CharacterGenerationChoices::Update() //425940.
	{
		if (dirty)
		{
			RebuildRows();
			dirty = 0;
		}
	}
	void CharacterGenerationChoices::RebuildRows() //425240.
	{
		const LONG rowCount = LONG(rows.size());
		for (LONG index = 0; index < rowCount; ++index)
		{
			const LONG item = firstVisible + index;
			CharacterGenerationChoiceRow& row = rows[index];
			const BYTE valid = item >= 0 && item < LONG(entries.size());
			if (row.button)
			{
				row.button->SetHidden(!valid);
				if (valid)
				{
					CharacterGenerationChoice& choice = entries[item];
					row.button->SetValueCF4(choice.value8);
					row.button->SetValueCD4(choice.valueC);
					row.button->SetStringCF8(choice.text);
				}
			}
			if (row.label)
			{
				row.label->SetHidden(!valid);
				if (valid)
				{
					CharacterGenerationChoice& choice = entries[item];
					row.label->SetStringReference(choice.reference);
					row.label->SetValueCF4(choice.value8);
					row.label->SetValueCD4(choice.valueC);
					row.label->SetStringCF8(choice.text);
					if (item == selected)
						row.label->SetTextColor(DWORD(-11575196));
					else
						row.label->SetTextColor(DWORD(-16744448));
				}
			}
		}
		if (previousButton)
			static_cast<::GUI::Button*>(previousButton)->SetEnabled(
				firstVisible > 0 && LONG(entries.size()) != 0);
		if (nextButton)
		{
			const LONG entryCount = LONG(entries.size());
			const LONG rowCount = LONG(rows.size());
			static_cast<::GUI::Button*>(nextButton)->SetEnabled(
				entryCount > rowCount && firstVisible < entryCount - rowCount);
		}
	}
}
