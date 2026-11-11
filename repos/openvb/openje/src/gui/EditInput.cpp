#include "gui/Edit.hpp"
#include "gui/Event.hpp"
#include "gui/Manager.hpp"
#include <algorithm>
#include <mmsystem.h>
#include <stdexcept>
#include <cstring>

namespace
{
	std::string ReadClipboardText() //Manager4B41D0; no Manager field is read.
	{
		std::string result;
		if (OpenClipboard(NULL))
		{
			HANDLE data = GetClipboardData(CF_TEXT);
			if (!data) { GetLastError(); return result; } //Original omits CloseClipboard here.
			const char* text = static_cast<const char*>(GlobalLock(data));
			result.assign(text, std::strlen(text));
			GlobalUnlock(data); CloseClipboard();
		}
		return result;
	}

}

namespace GUI
{
	void Edit::SetText(const std::string& value)
	{
		Label::SetText(value); editGeometryDirty = 1; ClampSelection(1);
	}
	void Edit::SetStringReference(DWORD value)
	{
		Label::SetStringReference(value); editGeometryDirty = 1; ClampSelection(1);
	}
	void Edit::AppendText(const std::string& value)
	{
		Label::AppendText(value); editGeometryDirty = 1; ClampSelection(1);
	}
	void Edit::UnusedTextOperation(DWORD value)
	{
		Label::UnusedTextOperation(value); editGeometryDirty = 1; ClampSelection(1);
	}
	void Edit::AppendCharacter(char value)
	{
		Label::AppendCharacter(value); editGeometryDirty = 1; ClampSelection(1);
	}
	void Edit::SetSelection(LONG anchor, LONG caret)
	{
		selectionAnchor = anchor; caretPosition = caret;
		ClampSelection(1); editGeometryDirty = 1;
	}
	void Edit::SetScroll(LONG horizontal, LONG vertical) //4C04B0.
	{ Label::SetScroll(horizontal, vertical); editGeometryDirty = 1; }
	void Edit::FollowCaret() //4C0EF0.
	{
		if (GetWrap())
		{
			LONG index = FindLine(caretPosition);
			if (index == firstSelectionLine + 1) --index;
			LONG horizontal, vertical;
			GetScroll(horizontal, vertical);
			const LONG available = rectangle.bottom - paddingBottom - paddingTop - rectangle.top;
			LabelLine* line = GetLine(index);
			if (!line) return;
			const LONG height = line->height;
			LONG y = 0;
			for (LONG i = 0; i < index; ++i) y += GetLine(i)->height;
			if (y < vertical) SetScroll(horizontal, y);
			else if (y + height >= available + vertical)
				SetScroll(horizontal, y + height - available);
		}
		else
		{
			LONG index = FindLine(caretPosition);
			if (index == firstSelectionLine + 1) --index;
			LONG horizontal, vertical;
			GetScroll(horizontal, vertical);
			const LONG left = rectangle.left + paddingLeft;
			const LONG available = rectangle.right - paddingRight - left;
			LabelLine* line = GetLine(index);
			if (!line) return;
			LONG x = line->x;
			switch (alignment)
			{
			case 1: x += available - line->width; break;
			case 2: x += (available - line->width) / 2; break;
			}
			std::string value;
			std::string prefix;
			GetText(value);
			prefix = value.substr(0, caretPosition);
			Font* font = g_Manager.GetFont(fontName, fontFlags, fontSize, fontCharset);
			FontGlyph glyph;
			x -= left;
			Measure(font, value.c_str()[caretPosition], glyph, x);
			LONG width = (selectionAnchor != caretPosition || insertMode) ? 2 : LONG(glyph.width);
			LONG advance = MeasureAdvance(prefix, font, x);
			if (advance < horizontal) SetScroll(advance, vertical);
			else if (advance + width >= available + horizontal)
				SetScroll(advance + width - available, vertical);
		}
	}
	void Edit::ScrollLines(LONG amount) //4C05C0.
	{
		if (!amount) return;
		LabelLine* line=GetLine(0);
		if (!line) return;
		LONG horizontal, vertical;
		GetScroll(horizontal,vertical);
		const LONG available=rectangle.bottom-paddingBottom-paddingTop-rectangle.top;
		const LONG height=line->height;
		const LONG maximum=GetTextHeight();
		LONG next=vertical-height*amount;
		if (amount>0)
		{
			if (next<0) next=0;
		}
		else
		{
			const LONG limit=maximum-available;
			if (next>limit) next=limit;
		}
		if (vertical!=next) { SetScroll(horizontal,next); ResetBlink(); }
	}
	void Edit::ScrollColumns(LONG amount) //4C0660.
	{
		if (!amount) return;
		LONG index=FindLine(caretPosition);
		if (index==firstSelectionLine+1) --index;
		if (!GetLine(index)) return;
		LONG horizontal, vertical;
		GetScroll(horizontal,vertical);
		const LONG available=rectangle.right-paddingRight-paddingLeft-rectangle.left;
		const LONG maximum=GetMaximumRowWidth();
		LONG next=horizontal-8*amount;
		if (amount>0)
		{
			if (next<0) next=0;
		}
		else
		{
			const LONG limit=maximum-available;
			if (next>limit) next=limit;
		}
		if (horizontal!=next) { SetScroll(next,vertical); ResetBlink(); }
	}
	LONG Edit::HitTestCaret(LONG x, LONG y) //4C09A0.
	{
		lastSelectionLine = -2;
		LONG index = FindLine(firstVisibleCharacter);
		if (index == firstSelectionLine + 1) --index;
		if (text.empty()) return -1;
		const LONG left = rectangle.left + paddingLeft, right = rectangle.right - paddingRight;
		while (index >= 0 && index < LONG(lines.size()))
		{
			const LabelLine& row = lines[index];
			if (y < row.y) { if (--index < 0) index = 0; break; }
			if (y < row.y + row.height || row.hidden) break;
			++index;
		}
		if (index >= LONG(lines.size())) --index;
		if (index < 0) return -1;
		const LabelLine& row = lines[index];
		std::string value;
		if (!GetLineText(index, value)) return -1;
		LONG last = row.end;
		if (row.hardBreak) { value += ' '; ++last; }
		const LONG origin = LineOrigin(row);
		if (x < origin) return firstVisibleCharacter > 0 ? firstVisibleCharacter - 1 : firstVisibleCharacter;
		if (x >= origin + row.width) { lastSelectionLine = row.id; return last; }
		Font* font = g_Manager.GetFont(fontName, fontFlags, fontSize, fontCharset);
		LONG px = row.x, scroll = row.horizontalScroll, offset = 0;
		BYTE visible = 0;
		for (; offset < LONG(value.size()); ++offset)
		{
			FontGlyph glyph; Measure(font, value[offset], glyph, origin - left);
			LONG py, width, height;
			ClipGlyph(glyph, row.x, row.y, scroll, row.verticalScroll, px, py, width, height, NULL);
			if (width <= 0) { if (visible) break; }
			else if (!visible) visible = 1;
			if (x < px) break;
			if (x < px + width)
				return row.begin + offset + (insertMode && x >= px + width / 2);
			if (px > x) break;
			px += width; scroll -= glyph.width;
		}
		return row.begin + offset;
	}
	void Edit::HandleText(Event* event) //4C14D0, byte-based insertion/replacement.
	{
		const char character = *reinterpret_cast<const char*>(&event->x);
		ClampSelection(0);
		LONG begin = (std::min)(selectionAnchor, caretPosition);
		LONG end = (std::max)(selectionAnchor, caretPosition);
		std::string value(text);
		if (begin == end)
		{
			if (!insertMode && begin < LONG(value.size())) value.erase(begin, 1);
			if (begin == LONG(text.size()))
			{
				//4C1652 uses qualified Label append-character4AA2B0.
				text.append(1, character); RebuildText();
				selectionAnchor = caretPosition = LONG(text.size());
				ResetBlink(); editGeometryDirty = 1; ClampSelection(1);
				return;
			}
		}
		else value.erase(begin, end - begin);
		value.insert(std::size_t(begin), 1, character);
		SetText(value);
		selectionAnchor = caretPosition = begin + 1;
		editGeometryDirty = 1; ClampSelection(1); RebuildText(); ResetBlink();
	}
	void Edit::HandleKey(Event* event) //4C3AF0, byte-based selection and wrapped navigation.
	{
		const LONG key = event->x;
		const DWORD modifiers = event->modifiers;
		if (key == 1)
		{
			text.clear(); RebuildText(); caretPosition = selectionAnchor = 0;
			editGeometryDirty = 1; ResetBlink(); return;
		}
		if ((key == 45 || key == 88 || key == 120) && (modifiers & 1))
		{ CutSelection(); ResetBlink(); return; }
		if ((key == 46 || key == 67 || key == 99) && (modifiers & 1))
		{ CopySelection(); ResetBlink(); return; }
		if (((key == 47 || key == 86 || key == 118) && (modifiers & 1))
			|| ((key == 210 || key == 338) && (modifiers & 2) && !(modifiers & 1)))
		{ Paste(); ResetBlink(); return; }
		if (key == 14 || key == 211 || key == 339)
		{
			if (key != 14 && (modifiers & 2))
			{ CopySelection(1); ResetBlink(); return; }
			ClampSelection(0);
			LONG begin = (std::min)(selectionAnchor, caretPosition);
			LONG end = (std::max)(selectionAnchor, caretPosition);
			std::string value(text);
			if (begin != end) value.erase(begin, end - begin);
			else if (key == 14 && begin > 0) { value.erase(--begin, 1); }
			else if (key != 14 && begin < LONG(value.size())) value.erase(begin, 1);
			else return;
			selectionAnchor = caretPosition = begin;
			SetText(value); editGeometryDirty = 1; ClampSelection(1); ResetBlink(); return;
		}
		if (key == 210 || key == 338)
		{
			if ((modifiers & 3) == 3) { ResetBlink(); return; }
			if (modifiers & 1)
			{ CopySelection(0); ResetBlink(); return; }
			insertMode = insertMode == 0;
		}
		else if (key == 199 || key == 327)
		{
			if (caretPosition <= 0 && selectionAnchor == caretPosition) return;
			if (modifiers & 1) caretPosition = 0;
			else
			{
				LONG index = CaretLine();
				if (index < 0 || index >= LONG(lines.size())) return;
				caretPosition = lines[index].begin;
			}
			if (!(modifiers & 2)) selectionAnchor = caretPosition;
		}
		else if (key == 207 || key == 335)
		{
			if (caretPosition >= LONG(text.size()) && selectionAnchor == caretPosition) return;
			LONG previousLine = -1;
			if ((modifiers & 1) || !wrap) caretPosition = LONG(text.size());
			else
			{
				LONG index = CaretLine();
				if (index < 0 || index >= LONG(lines.size())) return;
				caretPosition = lines[index].end + (lines[index].hardBreak != 0);
				previousLine = lines[index].id;
			}
			if (!(modifiers & 2)) selectionAnchor = caretPosition;
			ClampSelection(1); editGeometryDirty = 1;
			if (previousLine >= 0) firstSelectionLine = previousLine;
			ResetBlink(); return;
		}
		else if (key == 203 || key == 331)
		{
			if (caretPosition <= 0 && selectionAnchor == caretPosition) return;
			--caretPosition; if (!(modifiers & 2)) selectionAnchor = caretPosition;
		}
		else if (key == 205 || key == 333)
		{
			if (caretPosition >= LONG(text.size()) && selectionAnchor == caretPosition) return;
			++caretPosition; if (!(modifiers & 2)) selectionAnchor = caretPosition;
		}
		else if (key == 200 || key == 328 || key == 208 || key == 336
			|| key == 201 || key == 329 || key == 209 || key == 337)
		{
			if (!GetWrap()) return;
			if ((key == 200 || key == 328 || key == 208 || key == 336) && (modifiers & 1))
			{ ScrollLines(key == 200 || key == 328 ? 1 : -1); return; }
			if (key == 200 || key == 328)
			{
				if (caretPosition <= 0 && selectionAnchor == caretPosition) return;
				const LONG index = CaretLine();
				if (index <= 0)
				{
					if (!(modifiers & 2) && selectionAnchor != caretPosition)
					{
						selectionAnchor = caretPosition; editGeometryDirty = 1; ClampSelection(1);
					}
					return;
				}
				LabelLine* source = index < LONG(lines.size()) ? &lines[index] : NULL;
				const LONG targetIndex = index - 1;
				LabelLine* target = targetIndex < LONG(lines.size()) ? &lines[targetIndex] : NULL;
				if (!source || !target) return;
				const LONG targetEnd = target->end + (target->hardBreak != 0);
				Font* font = g_Manager.GetFont(fontName, fontFlags, fontSize, fontCharset);
				{
					std::string value, prefix;
					GetText(value);
					prefix = value.substr(source->begin, caretPosition - source->begin);
					const LONG origin = LineOrigin(*source) - rectangle.left - paddingLeft;
					const LONG advance = MeasureAdvance(prefix, font, origin);
					LONG position = target->begin, measured = 0;
					for (; position < targetEnd; ++position)
					{
						FontGlyph glyph; Measure(font, value.c_str()[position], glyph, origin);
						const LONG width = LONG(glyph.width);
						if (measured + width / 2 > advance) break;
						measured += width; //The original target scan keeps the tab origin fixed.
					}
					caretPosition = position;
					if (!(modifiers & 2)) selectionAnchor = position;
					ClampSelection(1); editGeometryDirty = 1;
					if (position == targetEnd) firstSelectionLine = targetIndex;
				}
				ResetBlink(); return;
			}
			if (key == 208 || key == 336)
			{
				{
					std::string value;
					GetText(value); //The downward end test uses the virtual text snapshot.
					if (caretPosition >= LONG(value.size()) && selectionAnchor == caretPosition) return;
					const LONG index = CaretLine();
					if (index >= GetLineCount() - 1)
					{
						if (!(modifiers & 2) && selectionAnchor != caretPosition)
						{
							selectionAnchor = caretPosition; editGeometryDirty = 1; ClampSelection(1);
						}
						return;
					}
					LabelLine* source = index >= 0 ? &lines[index] : NULL;
					const LONG targetIndex = index + 1;
					LabelLine* target = targetIndex >= 0 && targetIndex < LONG(lines.size()) ? &lines[targetIndex] : NULL;
					if (!source || !target) return;
					const LONG targetEnd = target->end + (target->hardBreak != 0);
					Font* font = g_Manager.GetFont(fontName, fontFlags, fontSize, fontCharset);
					std::string prefix;
					prefix = value.substr(source->begin, caretPosition - source->begin);
					const LONG origin = LineOrigin(*source) - rectangle.left - paddingLeft;
					const LONG advance = MeasureAdvance(prefix, font, origin);
					LONG position = target->begin, measured = 0;
					for (; position < targetEnd; ++position)
					{
						FontGlyph glyph; Measure(font, value.c_str()[position], glyph, origin);
						const LONG width = LONG(glyph.width);
						if (measured + width / 2 > advance) break;
						measured += width;
					}
					caretPosition = position;
					if (!(modifiers & 2)) selectionAnchor = position;
					ClampSelection(1); editGeometryDirty = 1;
					if (position == targetEnd) firstSelectionLine = targetIndex;
				}
				ResetBlink(); return;
			}
			if (key == 201 || key == 329)
			{
				if (caretPosition <= 0 && selectionAnchor == caretPosition) return;
				const LONG index = CaretLine();
				LabelLine* current = index >= 0 && index < LONG(lines.size()) ? &lines[index] : NULL;
				if (!current) return;
				const LONG right = rectangle.right - paddingRight, left = rectangle.left + paddingLeft;
				const LONG pageHeight = rectangle.bottom - paddingBottom - (rectangle.top + paddingTop);
				const LONG pageRows = pageHeight / current->height; //Original unchecked signed division.
				if (index <= 0)
				{
					if (!(modifiers & 2) && selectionAnchor != caretPosition)
					{
						selectionAnchor = caretPosition; editGeometryDirty = 1; ClampSelection(1);
					}
					return;
				}
				LONG targetIndex = index - pageRows;
				if (targetIndex < 0) targetIndex = 0;
				LabelLine* source = index < LONG(lines.size()) ? &lines[index] : NULL;
				LabelLine* target = targetIndex >= 0 && targetIndex < LONG(lines.size()) ? &lines[targetIndex] : NULL;
				if (!source || !target) return;
				const LONG targetEnd = target->end + (target->hardBreak != 0);
				Font* font = g_Manager.GetFont(fontName, fontFlags, fontSize, fontCharset);
				{
					std::string value, prefix;
					GetText(value);
					prefix = value.substr(source->begin, caretPosition - source->begin);
					LONG origin = target->x; //Page navigation uses the target row's alignment origin.
					if (alignment == 1) origin += right - left - target->width;
					else if (alignment == 2) origin += (right - left - target->width) / 2;
					origin -= left;
					const LONG advance = MeasureAdvance(prefix, font, origin);
					LONG position = target->begin, measured = 0;
					for (; position < targetEnd; ++position)
					{
						FontGlyph glyph; Measure(font, value.c_str()[position], glyph, origin);
						const LONG width = LONG(glyph.width);
						if (measured + width / 2 > advance) break;
						measured += width;
					}
					caretPosition = position;
					if (!(modifiers & 2)) selectionAnchor = position;
					LONG horizontal, vertical; GetScroll(horizontal, vertical);
					vertical -= pageHeight;
					if (vertical < 0) vertical = 0;
					SetScroll(horizontal, vertical);
					ClampSelection(1); editGeometryDirty = 1;
					if (position == targetEnd) firstSelectionLine = targetIndex;
				}
				ResetBlink(); return;
			}
			//PageDown retains its separate early text snapshot and later live text read.
			{
				std::string initialText;
				GetText(initialText);
				if (caretPosition >= LONG(initialText.size()) && selectionAnchor == caretPosition) return;
				const LONG index = CaretLine();
				LabelLine* current = index >= 0 && index < LONG(lines.size()) ? &lines[index] : NULL;
				if (!current) return;
				const LONG right = rectangle.right - paddingRight, left = rectangle.left + paddingLeft;
				const LONG pageHeight = rectangle.bottom - paddingBottom - (rectangle.top + paddingTop);
				const LONG pageRows = pageHeight / current->height;
				if (index >= GetLineCount() - 1)
				{
					if (!(modifiers & 2) && selectionAnchor != caretPosition)
					{
						selectionAnchor = caretPosition; editGeometryDirty = 1; ClampSelection(1);
					}
					return;
				}
				LONG targetIndex = index + pageRows;
				if (targetIndex > GetLineCount() - 1) targetIndex = GetLineCount() - 1;
				LabelLine* source = index >= 0 && index < LONG(lines.size()) ? &lines[index] : NULL;
				LabelLine* target = targetIndex >= 0 && targetIndex < LONG(lines.size()) ? &lines[targetIndex] : NULL;
				if (!source || !target) return;
				const LONG targetEnd = target->end + (target->hardBreak != 0);
				Font* font = g_Manager.GetFont(fontName, fontFlags, fontSize, fontCharset);
				std::string value, prefix;
				GetText(value);
				prefix = value.substr(source->begin, caretPosition - source->begin);
				LONG origin = target->x;
				if (alignment == 1) origin += right - left - target->width;
				else if (alignment == 2) origin += (right - left - target->width) / 2;
				origin -= left;
				const LONG advance = MeasureAdvance(prefix, font, origin);
				LONG position = target->begin, measured = 0;
				for (; position < targetEnd; ++position)
				{
					FontGlyph glyph; Measure(font, value.c_str()[position], glyph, origin);
					const LONG width = LONG(glyph.width);
					if (measured + width / 2 > advance) break;
					measured += width;
				}
				caretPosition = position;
				if (!(modifiers & 2)) selectionAnchor = position;
				LONG horizontal, vertical; GetScroll(horizontal, vertical);
				vertical += pageHeight;
				const LONG maximum = GetTextHeight() - pageHeight;
				if (vertical > maximum) vertical = maximum; //No lower clamp in the original downward branch.
				SetScroll(horizontal, vertical);
				ClampSelection(1); editGeometryDirty = 1;
				if (position == targetEnd) firstSelectionLine = targetIndex;
			}
			ResetBlink(); return;
		}
		else return;
		ClampSelection(1); editGeometryDirty = 1; ResetBlink();
	}
	void Edit::CutSelection() //4C3AA0.
	{
		CopySelection(1);
	}
	void Edit::CopySelection() //4C3AB0.
	{
		CopySelection(0);
	}
	void Edit::CopySelection(BYTE cut) //4C1FA0.
	{
		ClampSelection(0);
		const LONG begin = (std::min)(selectionAnchor, caretPosition);
		const LONG end = (std::max)(selectionAnchor, caretPosition);
		if (end - begin <= 0) return;
		std::string value, prefix, selected;
		GetText(value);
		prefix = value.substr(0, begin);
		selected = value.substr(begin, end - begin);
		if (end < LONG(value.size()))
			value = prefix + value.substr(end, value.size() - end);
		else
			value = prefix;
		if (cut)
		{
			SetText(value); selectionAnchor = caretPosition = begin;
			editGeometryDirty = 1; ClampSelection(1); ResetBlink();
		}
		g_Manager.WriteClipboardText(selected); //Copy/cut retains ignored result and mutation-before-copy order.
	}
	void Edit::Paste() //4C1960.
	{
		std::string clipboard = ReadClipboardText();
		if (!GetWrap())
		{
			for (LONG i = 0; i < LONG(clipboard.size()); ++i)
				if (clipboard[i] == '\n' || clipboard[i] == '\r')
				{
					clipboard = clipboard.substr(0, i);
					break;
				}
		}
		std::string unnormalized(clipboard);
		clipboard.assign("");
		for (LONG i = 0; i < LONG(unnormalized.size()); ++i)
			if (unnormalized[i] == '\t') clipboard.append(1, ' ');
			else if (unnormalized[i] != '\r') clipboard.append(1, unnormalized[i]);
		ClampSelection(0);
		LONG begin = (std::min)(selectionAnchor, caretPosition);
		LONG end = (std::max)(selectionAnchor, caretPosition);
		std::string value;
		std::string originalText;
		GetText(originalText);
		const LONG originalAnchor = selectionAnchor;
		const LONG originalCaret = caretPosition;
		if (end - begin > 0)
		{
			std::string prefix;
			std::string removed; //The original retains this substring through SetText/ResetBlink.
			GetText(value);
			prefix = value.substr(0, begin);
			removed = value.substr(begin, end - begin);
			if (end < LONG(value.size()))
				value = prefix + value.substr(end, value.size() - end);
			else value = prefix;
			SetText(value); selectionAnchor = caretPosition = begin;
			editGeometryDirty = 1; ResetBlink();
		}
		ClampSelection(0);
		begin = (std::min)(selectionAnchor, caretPosition);
		end = (std::max)(selectionAnchor, caretPosition);
		if (begin != end)
		{
			SetText(originalText);
			selectionAnchor = originalAnchor;
			caretPosition = originalCaret;
			editGeometryDirty = 1; ClampSelection(1); ResetBlink();
			return;
		}
		GetText(value);
		value.insert(begin, clipboard, 0, std::string::npos);
		SetText(value); selectionAnchor = caretPosition = begin + LONG(clipboard.size());
		editGeometryDirty = 1; ClampSelection(1); ResetBlink();
	}
	BOOL Edit::OnEvent(Event* event) //4C4EE0.
	{
		if (readOnly) return Window::OnEvent(event);
		LONG position;
		switch (event->type)
		{
		case 0:
		case 2:
			if (!dragging) return Window::OnEvent(event);
			if (!event->type)
			{
				MouseState mouse; g_Manager.GetMouseState(mouse);
				if (wrap ? (mouse.y >= rectangle.top + paddingTop && mouse.y < rectangle.bottom - paddingBottom)
					: (mouse.x >= rectangle.left + paddingLeft && mouse.x < rectangle.right - paddingRight))
					return Window::OnEvent(event);
				position = HitTestCaret(mouse.x, mouse.y);
			}
			else position = HitTestCaret(event->x, event->y);
			if (position >= 0 && caretPosition != position)
			{
				caretPosition = position; ClampSelection(1); editGeometryDirty = 1; ResetBlink();
				if (lastSelectionLine >= 0) firstSelectionLine = lastSelectionLine;
			}
			break;
		case 4:
			position = HitTestCaret(event->x, event->y);
			if (position < 0) return Window::OnEvent(event);
			caretPosition = position;
			if (!(event->modifiers & 2)) selectionAnchor = position;
			ClampSelection(1); editGeometryDirty = 1; ResetBlink();
			if (lastSelectionLine >= 0) firstSelectionLine = lastSelectionLine;
			dragging = 1; g_Manager.SetCapture(this); break;
		case 5:
			if (!dragging) return Window::OnEvent(event);
			g_Manager.SetCapture(NULL); dragging = 0; break;
		case 16:
			if (event->wheel) { if (wrap) ScrollLines(event->wheel); else ScrollColumns(event->wheel); }
			break;
		case 17:
			if (dragging && event->x != 42 && event->x != 29)
			{ g_Manager.SetCapture(NULL); dragging = 0; }
			HandleKey(event); break;
		case 19:
			if (dragging) { g_Manager.SetCapture(NULL); dragging = 0; }
			HandleText(event); return 1;
		}
		return Window::OnEvent(event);
	}
}
