#pragma once

#include "gui/EngineInterface.hpp"
#include <list>
#include <vector>

namespace GUI { class Label; }

namespace JE
{
	struct DebugWindowLine
	{
		DebugWindowLine(const std::string& value, int r, int g, int b); //557B80.
		std::string text;
		int red, green, blue; //+1C/+20/+24; ordinary40-byte owning record.
	};

	// Original CDebugInterface. The virtual EngineInterface base is at+C4,
	// after the compiler's vtordisp word; normal VC7 owns all base adjustment.
	class DebugWindow : public virtual EngineInterface
	{
	public:
		explicit DebugWindow(const std::string& name); //558740.
		virtual ~DebugWindow(); //557BF0, excluding compiler deleting557D40.
		virtual int OnEvent(::GUI::Window* window, ::GUI::Event* event); //558DC0 entry,558B70 ordinary body.
		void Write(const std::string& text, int red, int green, int blue); //5583E0.
		void Refresh(); //557FF0.
		void SetLines(const std::vector<std::string>& lines); //557D90.
		void Resize(LONG width, LONG height); //558190.
		void SetPosition(LONG x, LONG y); //434120, shared original code with Feedback.
		void ClearRows(); //434250, shared original code with Feedback.
		void ClearHistory(); //557A50.
		void DragTo(LONG x, LONG y); //558510.

		::GUI::Window* rootWindow; //+04; borrowed authored DebugWin.
		LONG historyOffset; //+08.
		LONG x, y, width, height; //+0C..18.
		LONG rowCount; //+1C.
		DWORD dragMode; //+20.
		LONG mouseOffsetX, mouseOffsetY, rightOffset, bottomOffset; //+24..30.
		LONG dragStartX, dragStartY, dragStartWidth, dragStartHeight; //+34..40.
		LONG leftMargin, rightMargin, topMargin, bottomMargin; //+44..50.
		LONG rowHeight; //+54.
		std::string rowSkin, rowFontName; //+58/+74.
		LONG rowFontSize; //+90.
		std::list< ::GUI::Label*> rows; //+94; owns dynamically generated Labels.
		std::list<DebugWindowLine*> history; //+A0; owns retained records, newest first.
		BYTE sourceMode; //+AC; paddingAD..AF is unwritten.
		std::vector<std::string> sourceLines; //+B0; real VC7 vector, allocator included.
	};
}
