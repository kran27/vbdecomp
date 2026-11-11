#include "gui/Label.hpp"
#include "gui/Manager.hpp"

namespace GUI
{
	LONG Label::GetTextAdvance(const std::string& value, LONG startX) //4A8900.
	{
		Font* font=g_Manager.GetFont(fontName,fontFlags,fontSize,fontCharset);
		return MeasureAdvance(value,font,startX);
	}
	LONG Label::MeasureAdvance(const std::string& value, Font* font, LONG x) //4A8200.
	{
		const LONG length = static_cast<LONG>(value.size());
		LONG advance = 0;
		if (length <= 0) return advance;
		for (LONG i = 0; i < length; ++i)
		{
			FontGlyph glyph;
			Measure(font, value[i], glyph, x);
			advance += glyph.width;
			x += glyph.width;
		}
		return advance;
	}
	BYTE Label::GetLineText(LONG index, std::string& output) //4A8B90.
	{
		if (index < 0 || index >= static_cast<LONG>(lines.size())) return 0;
		const LabelLine* line = &lines[index];
		output = "";
		for (LONG character = line->begin; character < line->end; ++character)
			output.append(1, text[character]);
		return 1;
	}
	LONG Label::GetLineCount() const //434220.
	{
		return LONG(lines.size());
	}
	LabelLine* Label::GetLine(LONG index) //4A87B0.
	{
		if (index < 0 || index >= LONG(lines.size())) return NULL;
		return &lines[index];
	}
	LONG Label::FindLine(LONG character) const //4A8800.
	{
		LONG index = 0;
		const std::vector<LabelLine>::const_iterator last = lines.end();
		for (std::vector<LabelLine>::const_iterator i = lines.begin(); i != last; ++i, ++index)
		{
			LONG end = i->end;
			if (i->hardBreak) end = LONG(DWORD(end) + 1);
			if (character >= i->begin && character <= LONG(DWORD(end) - DWORD(i->id != lines.size() - 1)))
				return index;
		}
		return -1;
	}

}

