#include "gui/DebugWindow.hpp"
#include "gui/Event.hpp"
#include "gui/Label.hpp"
#include "gui/Manager.hpp"
#include "diagnostics/DebugOutput.hpp"
#include <stddef.h>

namespace JE
{
#if defined(_MSC_VER) && _MSC_VER == 1300 && defined(_M_IX86)
	typedef char DebugWindowLineSize[(sizeof(DebugWindowLine) == 0x28) ? 1 : -1];
	typedef char DebugWindowSize[(sizeof(DebugWindow) == 0xF8) ? 1 : -1];
	typedef char DebugWindowRootOffset[(offsetof(DebugWindow, rootWindow) == 4) ? 1 : -1];
	typedef char DebugWindowRowsOffset[(offsetof(DebugWindow, rows) == 0x94) ? 1 : -1];
	typedef char DebugWindowHistoryOffset[(offsetof(DebugWindow, history) == 0xA0) ? 1 : -1];
	typedef char DebugWindowSourceOffset[(offsetof(DebugWindow, sourceLines) == 0xB0) ? 1 : -1];
#endif

	DebugWindowLine::DebugWindowLine(const std::string& value, int r, int g, int b) //557B80.
	{
		//Original default-constructs the string, then assigns under its own EH.
		text = value;
		red = r;
		blue = b;
		green = g;
	}

	DebugWindow::DebugWindow(const std::string& name) //558740.
		: EngineInterface(name)
	{
		LoadResource("Debug.INT"); //Original reader result is ignored.
		rows.clear();
		history.clear();
		leftMargin = 0;
		rightMargin = 0;
		topMargin = 0;
		bottomMargin = 0;
		rowHeight = 0;
		rowSkin = "";
		rowFontName = "";
		rowFontSize = 0;
		historyOffset = 0;
		x = 0;
		y = 0;
		width = 0;
		height = 0;
		rowCount = 0;
		dragMode = 0;
		mouseOffsetX = 0;
		mouseOffsetY = 0;
		rightOffset = 0;
		bottomOffset = 0;
		dragStartX = 0;
		dragStartY = 0;
		dragStartWidth = 0;
		dragStartHeight = 0;
		sourceMode = 0;
		rootWindow = FindWindow("DebugWin");
		if (rootWindow)
		{
			LONG rootX, rootY, rootWidth, rootHeight;
			rootWindow->GetAbsolutePosition(rootX, rootY);
			rootWindow->GetSize(rootWidth, rootHeight);
			x = rootX; y = rootY; width = rootWidth; height = rootHeight;
			::GUI::Label* lineTemplate = static_cast< ::GUI::Label*>(FindWindow("DebugLine"));
			if (lineTemplate)
			{
				LONG lineX, lineY, lineWidth, lineHeight;
				lineTemplate->GetClientPosition(lineX, lineY);
				lineTemplate->GetSize(lineWidth, lineHeight);
				leftMargin = lineX;
				rightMargin = rootWidth - lineWidth - lineX;
				topMargin = lineY;
				bottomMargin = rootHeight - lineHeight - lineY;
				rowHeight = lineHeight;
				lineTemplate->GetSkin(rowSkin);
				lineTemplate->GetFont(rowFontName, 0, 0, 0);
				rowFontSize = lineTemplate->GetFontSize();
				lineTemplate->SetHidden(1);
			}
			else
				LogDebugString("CDebugInterface::CDebugInterface() Unable to load template Lable.");
		}
		else
			LogDebugString("CDebugInterface::CDebugInterface() m_winDebug came back NULL.");
		Write("Welcome to F3.", 128, 128, 128);
	}

	DebugWindow::~DebugWindow() //557BF0.
	{
		ClearRows();
		ClearHistory();
		//Real vector/lists/strings die before the actual virtual EngineInterface.
	}

	void DebugWindow::ClearRows() //434250.
	{
		for (std::list< ::GUI::Label*>::iterator row = rows.begin(); row != rows.end(); ++row)
			if (*row) delete *row;
		rows.clear();
	}

	void DebugWindow::ClearHistory() //557A50.
	{
		for (std::list<DebugWindowLine*>::iterator line = history.begin(); line != history.end(); ++line)
			if (*line) delete *line;
		history.clear();
	}

	void DebugWindow::Write(const std::string& text, int red, int green, int blue) //5583E0.
	{
		sourceMode = 0;
		sourceLines.clear(); //VC7 clear destroys and frees the entire allocation.
		DebugWindowLine* line = new DebugWindowLine(text, red, green, blue);
		history.push_front(line);
		if (history.size() > 1000)
			history.pop_back(); //Original erases only the node; the evicted record leaks.
		Refresh();
	}

	void DebugWindow::Refresh() //557FF0.
	{
		if (sourceMode)
		{
			SetLines(sourceLines); //Preserve the actual vector self-assignment path.
			return;
		}
		std::list< ::GUI::Label*>::iterator row = rows.begin();
		std::list<DebugWindowLine*>::iterator line = history.begin();
		for (LONG skip = 0; skip < historyOffset && line != history.end(); ++skip) ++line;
		for (; row != rows.end() && line != history.end(); ++row, ++line)
		{
			::GUI::Label* label = *row;
			DebugWindowLine* entry = *line;
			if (label && entry)
			{
				label->SetText(entry->text);
				label->SetTextColor(D3DCOLOR_XRGB(entry->red, entry->green, entry->blue));
			}
		}
		for (; row != rows.end(); ++row)
		{
			::GUI::Label* label = *row;
			if (label) label->SetText("");
		}
	}

	void DebugWindow::SetLines(const std::vector<std::string>& lines) //557D90.
	{
		ClearHistory();
		sourceMode = 1;
		sourceLines = lines;
		std::list< ::GUI::Label*>::iterator row = rows.end();
		--row; //Original starts at the tail, including its unwritten empty-list domain.
		LONG index = 0;
		while (index < historyOffset && index < LONG(lines.size())) ++index;
		for (; row != rows.begin() && index < LONG(lines.size()); ++index, --row)
			if (*row) (*row)->SetText(lines[index]);
		if (index < LONG(lines.size()))
		{
			if (*row) (*row)->SetText(lines[index]);
		}
		else
		{
			for (; row != rows.begin(); --row)
			{
				::GUI::Label* label = *row;
				if (label) label->SetText("");
			}
			::GUI::Label* label = *row;
			if (label) label->SetText("");
		}
	}

	void DebugWindow::SetPosition(LONG newX, LONG newY) //434120.
	{
		if (newX < 0) newX = 0;
		if (newY < 0) newY = 0;
		if (width + newX > 1024) newX = 1024 - width;
		if (height + newY > 768) newY = 768 - height;
		if (newX != x || newY != y)
		{
			x = newX;
			y = newY;
			rootWindow->SetAbsolutePosition(newX, newY);
		}
	}

	void DebugWindow::Resize(LONG newWidth, LONG newHeight) //558190.
	{
		if (newWidth < 10 || newHeight < 10 || (newWidth == width && newHeight == height)) return;
		width = newWidth;
		height = newHeight;
		rootWindow->Resize(newWidth, newHeight);
		const LONG count = (height - topMargin - bottomMargin) / rowHeight;
		if (count != rowCount)
		{
			ClearRows();
			for (LONG index = 0; index < count; ++index)
			{
				::GUI::Label* row = new ::GUI::Label(3);
				rootWindow->AddChild(row);
				row->ApplySkin(rowSkin);
				row->SetFont(rowFontName, 0, rowFontSize, 1);
				row->SetTextColor(0xFF808080);
				row->SetText("");
				rows.push_front(row);
			}
			rowCount = count;
		}
		LONG index = 1;
		for (std::list< ::GUI::Label*>::iterator row = rows.begin(); row != rows.end(); ++row, ++index)
		{
			::GUI::Label* label = *row;
			if (label)
			{
				label->Resize(width - rightMargin - leftMargin, rowHeight);
				label->SetAbsolutePosition(x + leftMargin,
					y + height - index * rowHeight - bottomMargin);
			}
		}
		Refresh();
	}

	void DebugWindow::DragTo(LONG mouseX, LONG mouseY) //558510.
	{
		LONG nextX, nextY, oldWidth, oldHeight;
		switch (dragMode)
		{
		case 1: SetPosition(mouseX - mouseOffsetX, mouseY - mouseOffsetY); break;
		case 6:
			nextX = mouseX - mouseOffsetX; nextY = mouseY - mouseOffsetY;
			oldWidth = width; oldHeight = height;
			Resize(x + oldWidth - nextX, y + oldHeight - nextY);
			if (width != oldWidth || height != oldHeight) SetPosition(nextX, nextY);
			break;
		case 7:
			nextX = mouseX - mouseOffsetX; oldWidth = width;
			Resize(x + oldWidth - nextX, mouseY + bottomOffset - y);
			if (width != oldWidth) SetPosition(nextX, y);
			break;
		case 8:
			nextY = mouseY - mouseOffsetY; oldHeight = height;
			Resize(mouseX + rightOffset - x, y + oldHeight - nextY);
			if (height != oldHeight) SetPosition(x, nextY);
			break;
		case 9: Resize(mouseX + rightOffset - x, mouseY + bottomOffset - y); break;
		case 2:
			nextX = mouseX - mouseOffsetX; oldWidth = width;
			Resize(x + oldWidth - nextX, height);
			if (width != oldWidth) SetPosition(nextX, y);
			break;
		case 3: Resize(mouseX + rightOffset - x, height); break;
		case 4:
			nextY = mouseY - mouseOffsetY; oldHeight = height;
			Resize(width, y + oldHeight - nextY);
			if (oldHeight != height) SetPosition(x, nextY);
			break;
		case 5: Resize(width, mouseY + bottomOffset - y); break;
		}
	}

	int DebugWindow::OnEvent(::GUI::Window* window, ::GUI::Event* event) //558DC0, ordinary558B70.
	{
		if (event->type == 4)
		{
			mouseOffsetX = event->x - x;
			mouseOffsetY = event->y - y;
			rightOffset = width - mouseOffsetX;
			bottomOffset = height - mouseOffsetY;
			dragStartX = x; dragStartY = y;
			dragStartWidth = width; dragStartHeight = height;
			if (mouseOffsetX < 8 && mouseOffsetY < 8) dragMode = 6;
			else if (mouseOffsetX < 8 && bottomOffset < 8) dragMode = 7;
			else if (rightOffset < 8 && mouseOffsetY < 8) dragMode = 8;
			else if (rightOffset < 8 && bottomOffset < 8) dragMode = 9;
			else if (mouseOffsetX < 8) dragMode = 2;
			else if (rightOffset < 8) dragMode = 3;
			else if (mouseOffsetY < 8) dragMode = 4;
			else dragMode = bottomOffset < 8 ? 5 : 1;
			BringToFront();
			::GUI::g_Manager.PushModal(this);
		}
		else if (event->type == 5)
		{
			::GUI::g_Manager.RemoveModal(this);
			dragMode = 0;
		}
		else if (event->type == 2)
		{
			DragTo(event->x, event->y);
		}
		else if (event->type == 16)
		{
			if (event->wheel > 0) ++historyOffset;
			else if (event->wheel < 0) --historyOffset;
			if (historyOffset < 0) historyOffset = 0;
			Refresh();
			return 1; //558D85 explicitly writes EAX=1.
		}
		else if (event->type == 12)
		{
			historyOffset = 0;
			Refresh();
			return 1;
		}
		return EngineInterface::OnEvent(window, event);
	}
}
