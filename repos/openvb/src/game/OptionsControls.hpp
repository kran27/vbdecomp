#pragma once

#include <windows.h>

namespace GUI
{
	class Window;
	class Picture;
	class ScrollBar;
	struct Event;
}

// Actual global RTTI classes, ordinary proprietary owners; no legacy overlays.
class OptionDial
{
public:
	OptionDial(); // 4431B0.
	virtual ~OptionDial(); // 4431D0; deleting entry4434F0.
	virtual void SetValue(LONG value); // 443BE0, publishes then changes Picture.
	virtual void OnChanged(); // 443220, actual transient Toggle.wav request.
	void Next(); // 443200, calls virtual SetValue then virtual OnChanged.
	void BindPicture(::GUI::Picture* value); // 4431E0.
	void SetCount(LONG value); // 4431F0, no clamping.
	::GUI::Picture* GetPicture() const; // Shared5C0160.

	LONG value; // +04, initial0.
	LONG count; // +08, initial1.
	::GUI::Picture* picture; // +0C, borrowed, initialNULL.
};

class OptionSwitch : public OptionDial
{
public:
	OptionSwitch(); // 4432A0, count2.
	virtual ~OptionSwitch(); // 4432C0.
	virtual void SetValue(LONG value); // 443D70.
};

class OptionCheckBox : public OptionDial
{
public:
	OptionCheckBox(); // 4432D0, count2.
	virtual ~OptionCheckBox(); // 4432F0.
	virtual void SetValue(LONG value); // 443E90.
};

class OptionScrollBar
{
public:
	OptionScrollBar(); // 443300.
	virtual ~OptionScrollBar(); // 4433A0; deleting entry443550.
	void SetMaximum(LONG value); // 4433C0.
	void BindThumb(::GUI::Window* value); // 443FB0; actual asset typeButton.
	void BindTrack(::GUI::Window* value); // 4440A0; actual asset typeButton.

	::GUI::ScrollBar* scrollbar; // +04, owned actual4B8720.
};

