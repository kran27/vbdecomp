#pragma once

#include "gui/HeapTracker.hpp"
#include <windows.h>
#include <list>
#include <string>

namespace GUI
{
	class Window;
	struct Event;

	// Actual RTTI Interface : HeapTracker@0,52 bytes and six actual primary slots.
	// H-TYPE/H-RETURN labels distinguish compatible source spellings from proven
	// operand widths/stack cleanup. Undefined APIs do not supply successful owners.
	class Interface : public HeapTracker
	{
	public:
		Interface(const std::string& name); // 495600; real members and actual Manager registration.
		virtual ~Interface(); // +00,ordinary495500; compiler scalar-deleting support separate.
		virtual float SetDepth(float depth, float increment); // +04,4953C0; consumed ST0, float spelling H-TYPE.
		virtual int OnEvent(Window* window, Event* event); // +08,4958D0; full EAX consumed/H-TYPE.
		virtual void BeforeUpdate(); // +0C,shared404B70; name/H-RETURN hypothesis.
		virtual void BeforeDraw(); // +10,shared404B70; name/H-RETURN hypothesis.
		virtual BYTE OnCursor(Window* window, LONG clientX, LONG clientY); // +14,430790; AL/RET12.

		void SetHidden(BYTE value); // 495220; raw BYTE/H-RETURN; original capture side effects.
		BYTE GetHidden(); // 406A50; raw hidden BYTE, no normalization.
		std::list<Window*>* GetRoots(); // 495290; borrowed actual root-list object.
		Window* FindAt(LONG x, LONG y, BYTE overrideSelf); // 4952B0; forwards the low-byte flag.
		void BringToFront(); // 4952A0; ordinary Manager ordering only/H-RETURN.
		void BringWindowToFront(Window* window); //495A50; reorder existing root, mark depth dirty.
		void Update(); // 495340; reverse roots, actual Window+28; H-RETURN.
		void RefreshPositions(); //495310; forward roots, actual Offset(0,0).
		void Draw(); // 495380; reverse roots, actual Window+2C; H-RETURN.
		void Cleanup(); // 495420; selective root cleanup, modal removal and actual unregister.
		void AddWindow(Window* window); // 4959B0; actual ownership policy recovered separately/H-RETURN.
		void RemoveWindow(Window* window); // 4954A0; H-RETURN.
		void ClearWindows(); // 4955B0; list nodes only, no ownership cleanup.
		BYTE LoadResource(const std::string& filename); // 495B50; actual INT reader.
		Window* FindWindow(const std::string& name); //495740, root-first recursive named lookup.

		std::string name; // +04..1F, real VC7 string.
		BYTE hidden; // +20; padding21..23 unwritten.
		std::list<Window*> roots; // +24..2F; actual Window elements.
		BYTE pendingDestroy; // +30; source role/name hypothesis; padding31..33 unwritten.
	};
}
