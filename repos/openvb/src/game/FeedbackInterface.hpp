#pragma once

#include <winsock2.h>
#include "game/GameInterface.hpp"
#include <list>
#include <string>

namespace GUI { class Window; class Label; }

namespace F3
{
	//4346B0. History owns ordinary records; both strings are real library owners.
	struct FeedbackMessage
	{
		FeedbackMessage();
		std::string text; //+00.
		std::string speaker; //+1C.
		DWORD red, green, blue; //+38/+3C/+40, ctor128.
	};

	class FeedbackInterface : public GameInterface
	{
	public:
		explicit FeedbackInterface(const std::string& name); //434F70.
		virtual ~FeedbackInterface(); //434340, deleting entry434470.
		virtual int OnEvent(::GUI::Window*, ::GUI::Event*); //434180; adjustment434480.
		//BeforeUpdate/BeforeDraw inherit actual shared404B70 no-ops.
		//OnCursor inherits GameInterface439610; original adjustment434490.
		void SetAbsolutePosition(LONG x, LONG y); //434120, original1024x768 clamp.
		void Resize(LONG width, LONG height); //434910; parent layout lane.
		void ClearRows(); //434250.
		void ClearHistory(); //4342C0.
		void AddMessage(std::string text, DWORD red, DWORD green, DWORD blue,
			std::string speaker); //434B70; parent message lane.
		void Refresh(); //434760; parent message lane.

		::GUI::Window* rootWindow; //+04, borrowed serialized root.
		::GUI::Label* lineTemplate; //+08, borrowed serialized prototype.
		LONG x, y, width, height; //+0C/+10/+14/+18, cached geometry.
		LONG rowCount; //+1C.
		DWORD value20, value24, value28, value2C, value30; //Ctor0, no recovered use.
		DWORD value34, value38, value3C, value40; //Ctor0, no recovered use.
		LONG leftMargin, rightMargin, topMargin, bottomMargin; //+44..50.
		LONG rowHeight; //+54, prototype serialized height.
		std::string rowSkin; //+58, actual Label.GetSkin.
		std::string rowFontName; //+74, actual Label.GetFont.
		LONG rowFontSize; //+90.
		std::list<::GUI::Label*> rows; //+94, owns generated labels.
		std::list<FeedbackMessage*> history; //+A0, owns records.
	};
}

