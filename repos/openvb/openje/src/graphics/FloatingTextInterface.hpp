#pragma once

#include "gui/EngineInterface.hpp"
#include "gui/FloatingLabel.hpp"
#include <list>

namespace JE
{
	struct FloatingTextConfiguration
	{
		FloatingTextConfiguration(); //590220.
		std::string profile, font;
		LONG pointSize;
		DWORD fontFlags, red, green, blue, duration, speed, fadeOut, fadeIn, width, height;
		BYTE wrap;
		DWORD alignment;
		std::string backgroundProfile;
		BYTE backgroundAlpha;
		LONG paddingLeft, paddingRight, paddingTop, paddingBottom;
	};

	class FloatingTextInterface : public virtual EngineInterface
	{
	public:
		explicit FloatingTextInterface(const std::string& name); //590360.
		virtual ~FloatingTextInterface(); //58FFE0.
		virtual void AddSpeech(DWORD entityID, const std::string& text, DWORD duration,
			DWORD red, DWORD green, DWORD blue); //590750.
		virtual void AddDamage(DWORD entityID, const std::string& text,
			DWORD red, DWORD green, DWORD blue); //5907E0.
		virtual void AfterPosition(::GUI::FloatingLabel*, LONG, LONG, LONG, LONG); //58F9B0.
		virtual int OnEvent(::GUI::Window*, ::GUI::Event*); //5900D0/590120.
		void UpdatePositions(); //58FCC0.
		void Clear(); //58FEE0; actual erase/hide/duration policy, including next-node skip.
		FloatingTextConfiguration speech, damage;
		std::list<::GUI::FloatingLabel*> activeLabels;
		::GUI::FloatingLabel labels[32];
	private:
		::GUI::FloatingLabel* FindFree(); //58F9C0.
		DWORD GetReadingTime(const std::string& text); //58FC70, low conversion word/RET4.
		void Add(DWORD, const std::string&, DWORD, DWORD, DWORD,
			const std::string&, const std::string&, LONG, DWORD, DWORD, DWORD,
			DWORD, DWORD, LONG, LONG, BYTE, DWORD, DWORD); //590500, RET48.
		FloatingTextInterface(const FloatingTextInterface&);
		FloatingTextInterface& operator=(const FloatingTextInterface&);
	};
}
