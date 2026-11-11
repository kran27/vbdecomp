#pragma once

#include <windows.h>
#include <vector>
#include <string>

namespace GUI { class Window; class Picture; struct Event; }
namespace F3
{
	//Actual Picture-based digit owner433560, not a Label substitute.
	class CounterControl
	{
		friend class PipBoyInterface;
		friend class CharacterGenerationInterface;
		friend class NameAgeSexInterface;
		friend struct CounterControlLayout;
	public:
		CounterControl(); //433560.
		virtual ~CounterControl(); //432D50; deletes generated pictures, borrows digit windows.
		void SetDigits(LONG count); //4332E0.
		void BindDigit(LONG index, ::GUI::Window* window); //432B60.
		void BindButtons(::GUI::Window* increase, ::GUI::Window* decrease); //Shared4B8790.
		LONG GetDigitCount() const; //430830.
		LONG GetValue() const; //4307A0.
		void SetMinimum(LONG value); //430850.
		void SetMaximum(LONG value); //430890.
		void SetDigitRange(LONG index, LONG minimum, LONG maximum); //430A30, trusted digit index.
		BYTE Process(::GUI::Window* window, ::GUI::Event* event); //431280.
		void Finish(); //431D70, complete a live transition immediately.
		void SetValue(LONG value, DWORD mode); //4313B0;0 independent,1 relative,2 immediate.
		void Update(); //4314C0, actual independent/relative rolling digit animation.
		//Borrowed inspection adapters for explicit host cleanup; not original APIs.
		const std::vector< ::GUI::Picture*>& PrimaryPictures() const { return primaryPictures; }
		const std::vector< ::GUI::Picture*>& SecondaryPictures() const { return secondaryPictures; }
	private:
		::GUI::Window* incrementButton; //+04, borrowed; no producer used by Actionbar.
		::GUI::Window* decrementButton; //+08.
		std::vector< ::GUI::Window*> digitWindows; //+0C, borrowed.
		std::vector<BYTE> digitValues; //+1C.
		std::vector<LONG> digitMinimums, digitMaximums; //+2C/+3C.
		std::vector< ::GUI::Picture*> primaryPictures, secondaryPictures; //+4C/+5C, owned.
		LONG currentValue, minimum, maximum; //+6C/+70/+74.
		BYTE animating; //+78.
		DWORD transitionMode; //+7C.
		LONG targetValue; //+80.
		DWORD transitionTime; //+84.
		std::vector<DWORD> digitTimes; //+88.
		std::vector<std::string> digitTextures; //+98, actual global ten-name copy.
		DWORD pressTime, repeatTime, holdDelay, repeatDelay; //+A8/AC/B0/B4.
		BYTE disabled; //+B8.
		LONG digitWidth, digitHeight; //+BC/C0.
	public:
		float animationStep; //Original+C4, default0.1 seconds.
	private:
		LONG radix; //+C8, default10.
		void ClearPrimary(); //4321C0.
		void ClearSecondary(); //432460.
		void CreatePrimaryDigitPictures(LONG count); //432EE0; ordinary member.
		void CreateSecondaryDigitPictures(LONG count); //432F20; ordinary member.
		void CreatePrimaryDigitPicture(LONG index); //431F80.
		void CreateSecondaryDigitPicture(LONG index); //432210.
		void SetDigit(LONG index, LONG value); //4309B0, stores the low byte; full value indexes textures.
		void SetTarget(LONG value); //430C40 mode0.
		LONG AddDigits(LONG value, LONG delta); //430A50, actual per-digit carry rules.
		void AddValue(LONG delta); //4310E0, relative mode1.
		CounterControl(const CounterControl&);
		CounterControl& operator=(const CounterControl&);
	};
}
