#include "game/CounterControl.hpp"
#include "gui/Window.hpp"
#include "gui/Picture.hpp"
#include "gui/Button.hpp"
#include "gui/Event.hpp"
#include <mmsystem.h>
#include <cmath>
#include <cstdlib>
#include <climits>
#include <stdexcept>
#include <cstddef>

namespace F3
{
#if defined(_MSC_VER) && _MSC_VER == 1300 && defined(_M_IX86)
	//Original offsets from433560 and the retained CounterControl consumers.
	struct CounterControlLayout
	{
		typedef char Extent[sizeof(CounterControl) == 0xCC ? 1 : -1];
		typedef char Increase[offsetof(CounterControl, incrementButton) == 0x04 ? 1 : -1];
		typedef char Decrease[offsetof(CounterControl, decrementButton) == 0x08 ? 1 : -1];
		typedef char Windows[offsetof(CounterControl, digitWindows) == 0x0C ? 1 : -1];
		typedef char Values[offsetof(CounterControl, digitValues) == 0x1C ? 1 : -1];
		typedef char Lows[offsetof(CounterControl, digitMinimums) == 0x2C ? 1 : -1];
		typedef char Highs[offsetof(CounterControl, digitMaximums) == 0x3C ? 1 : -1];
		typedef char Primary[offsetof(CounterControl, primaryPictures) == 0x4C ? 1 : -1];
		typedef char Secondary[offsetof(CounterControl, secondaryPictures) == 0x5C ? 1 : -1];
		typedef char Current[offsetof(CounterControl, currentValue) == 0x6C ? 1 : -1];
		typedef char Minimum[offsetof(CounterControl, minimum) == 0x70 ? 1 : -1];
		typedef char Maximum[offsetof(CounterControl, maximum) == 0x74 ? 1 : -1];
		typedef char Animating[offsetof(CounterControl, animating) == 0x78 ? 1 : -1];
		typedef char Mode[offsetof(CounterControl, transitionMode) == 0x7C ? 1 : -1];
		typedef char Target[offsetof(CounterControl, targetValue) == 0x80 ? 1 : -1];
		typedef char Transition[offsetof(CounterControl, transitionTime) == 0x84 ? 1 : -1];
		typedef char Times[offsetof(CounterControl, digitTimes) == 0x88 ? 1 : -1];
		typedef char Textures[offsetof(CounterControl, digitTextures) == 0x98 ? 1 : -1];
		typedef char Press[offsetof(CounterControl, pressTime) == 0xA8 ? 1 : -1];
		typedef char Repeat[offsetof(CounterControl, repeatTime) == 0xAC ? 1 : -1];
		typedef char Hold[offsetof(CounterControl, holdDelay) == 0xB0 ? 1 : -1];
		typedef char Delay[offsetof(CounterControl, repeatDelay) == 0xB4 ? 1 : -1];
		typedef char Disabled[offsetof(CounterControl, disabled) == 0xB8 ? 1 : -1];
		typedef char Width[offsetof(CounterControl, digitWidth) == 0xBC ? 1 : -1];
		typedef char Height[offsetof(CounterControl, digitHeight) == 0xC0 ? 1 : -1];
		typedef char Step[offsetof(CounterControl, animationStep) == 0xC4 ? 1 : -1];
		typedef char Radix[offsetof(CounterControl, radix) == 0xC8 ? 1 : -1];
	};
#endif
	namespace
	{
		const std::string digitTextureNames[10] = {"f3_digit_0.tga", "f3_digit_1.tga",
			"f3_digit_2.tga", "f3_digit_3.tga", "f3_digit_4.tga", "f3_digit_5.tga",
			"f3_digit_6.tga", "f3_digit_7.tga", "f3_digit_8.tga", "f3_digit_9.tga"};
		int Direction(LONG from, LONG to, LONG minimum, LONG maximum) //4307C0.
		{
			const LONG delta = to - from;
			if (!delta) return 0;
			const LONG threshold = (maximum - minimum + 1) / 2 + 1;
			return delta >= threshold || (delta < 0 && delta > -threshold) ? -1 : 1;
		}

	}

	CounterControl::CounterControl() //433560; real ten-string owner initialized at63D650.
	{
		incrementButton = NULL;
		decrementButton = NULL;
		digitWindows.resize(0, NULL);
		digitValues.resize(0, 0);
		digitMinimums.resize(0, 0);
		digitMaximums.resize(0, 0);
		primaryPictures.resize(0, NULL);
		secondaryPictures.resize(0, NULL);
		digitTimes.resize(0, 0);
		digitTextures.resize(10);
		for (int i = 0; i < 10; ++i) digitTextures[i] = digitTextureNames[i];
		currentValue = LONG(0x80000000u);
		minimum = LONG(0x80000000u);
		animating = 0;
		targetValue = 0;
		transitionTime = 0;
		pressTime = 0;
		repeatTime = 0;
		disabled = 0;
		digitWidth = 0;
		digitHeight = 0;
		maximum = LONG(0x7FFFFFFFu);
		transitionMode = 1;
		holdDelay = 400;
		repeatDelay = 35;
		animationStep = 0.1f;
		radix = 10;
	}
	CounterControl::~CounterControl() //432D50.
	{
		ClearPrimary();
		ClearSecondary();
		//Real member vectors release their ordinary storage, never digit windows.
	}
	void CounterControl::ClearPrimary() //4321C0.
	{
		for (std::vector< ::GUI::Picture*>::iterator it = primaryPictures.begin(); it != primaryPictures.end(); ++it)
			if (*it) delete *it;
		primaryPictures.clear();
	}
	void CounterControl::ClearSecondary() //432460.
	{
		for (std::vector< ::GUI::Picture*>::iterator it = secondaryPictures.begin(); it != secondaryPictures.end(); ++it)
			if (*it) delete *it;
		secondaryPictures.clear();
	}
	LONG CounterControl::GetDigitCount() const { return LONG(digitWindows.size()); } //430830.
	void CounterControl::BindButtons(::GUI::Window* increase, ::GUI::Window* decrease)
	{
		//Actual shared4B8790 stores the two borrowed prefix fields, no callbacks.
		incrementButton = increase;
		decrementButton = decrease;
	}
	LONG CounterControl::GetValue() const { if (!animating) return currentValue; return targetValue; } //4307A0.
	void CounterControl::SetMinimum(LONG value) //430850.
	{
		const LONG count = GetDigitCount();
		DWORD capacity = 1;
		for (LONG remaining = count; remaining > 0; --remaining) capacity *= DWORD(radix);
		--capacity;
		minimum = value;
		if (minimum < 0) minimum = 0;
		if (minimum > LONG(capacity)) minimum = LONG(capacity);
	}
	void CounterControl::SetMaximum(LONG value) //430890.
	{
		const LONG count = GetDigitCount();
		DWORD capacity = 1;
		for (LONG remaining = count; remaining > 0; --remaining) capacity *= DWORD(radix);
		--capacity;
		maximum = value;
		if (maximum < 0) maximum = 0;
		if (maximum > LONG(capacity)) maximum = LONG(capacity);
	}
	void CounterControl::SetDigitRange(LONG index, LONG low, LONG high) //430A30.
	{
		digitMinimums[index] = low;
		digitMaximums[index] = high;
	}
	void CounterControl::CreatePrimaryDigitPictures(LONG count) //432EE0, ordinary RET4.
	{
		primaryPictures.resize(size_t(count), NULL);
		for (LONG i = 0; i < count; ++i) CreatePrimaryDigitPicture(i);
	}
	void CounterControl::CreateSecondaryDigitPictures(LONG count) //432F20, ordinary RET4.
	{
		secondaryPictures.resize(size_t(count), NULL);
		for (LONG i = 0; i < count; ++i) CreateSecondaryDigitPicture(i);
	}
	void CounterControl::SetDigits(LONG count) //4332E0.
	{
		ClearPrimary();
		ClearSecondary();
		if (count > 0)
		{
			digitWindows.resize(size_t(count), NULL);
			digitValues.resize(size_t(count), 0);
			digitMinimums.resize(size_t(count), 0);
			digitMaximums.resize(size_t(count), 0);
			primaryPictures.resize(size_t(count), NULL);
			CreatePrimaryDigitPictures(count);
			secondaryPictures.resize(size_t(count), NULL);
			CreateSecondaryDigitPictures(count);
			digitTimes.resize(size_t(count), 0);
			for (LONG i = 0; i < count; ++i)
			{
				digitTimes[i] = 0;
				digitMinimums[i] = 0;
				digitMaximums[i] = 9;
			}
		}
		else
		{
			digitWindows.resize(0, NULL);
			digitValues.resize(0, 0);
			digitMinimums.resize(0, 0);
			digitMaximums.resize(0, 0);
			primaryPictures.resize(0, NULL);
			secondaryPictures.resize(0, NULL);
			digitTimes.resize(0, 0);
		}
		SetMinimum(0);
		DWORD capacity = 1;
		for (LONG i = 0; i < GetDigitCount(); ++i) capacity *= 10;
		SetMaximum(LONG(capacity - 1));
	}
	void CounterControl::CreatePrimaryDigitPicture(LONG index) //431F80.
	{
		::GUI::Window* window = digitWindows[index];
		if (window)
		{
			::GUI::Picture* picture = new ::GUI::Picture(2);
			std::string retainedText;
			picture->SetFrameVisible(window->GetFrameVisible());
			LONG x, y, width, height;
			window->GetAbsolutePosition(x, y);
			window->GetSize(width, height);
			picture->SetAbsolutePosition(x, y);
			picture->Resize(width, height);
			picture->SetHidden(0);
			picture->SetName(window->GetName());
			picture->LoadTexture(digitTextures[0], 0, 0, 0, -1, -1, 1);
			picture->SetValueCD4(window->valueCD4);
			picture->SetValueCF4(window->valueCF4);
			picture->SetStringCF8(window->GetStringCF8());
			primaryPictures[index] = picture;
			digitWindows[index]->GetParent()->AddChild(picture);
			digitWidth = width;
			digitHeight = height;
		}
		else primaryPictures[index] = NULL;
	}
	void CounterControl::CreateSecondaryDigitPicture(LONG index) //432210.
	{
		::GUI::Window* window = digitWindows[index];
		if (window)
		{
			::GUI::Picture* picture = new ::GUI::Picture(2);
			std::string retainedText;
			picture->SetFrameVisible(window->GetFrameVisible());
			LONG x, y, width, height;
			window->GetAbsolutePosition(x, y);
			window->GetSize(width, height);
			picture->SetAbsolutePosition(x, y + height);
			picture->Resize(width, height);
			picture->SetHidden(1);
			picture->SetName(window->GetName());
			picture->LoadTexture(digitTextures[0], 0, 0, 0, -1, -1, 1);
			picture->SetValueCD4(window->valueCD4);
			picture->SetValueCF4(window->valueCF4);
			picture->SetStringCF8(window->GetStringCF8());
			secondaryPictures[index] = picture;
			digitWindows[index]->GetParent()->AddChild(picture);
			digitWidth = width;
			digitHeight = height;
		}
		else secondaryPictures[index] = NULL;
	}
	void CounterControl::BindDigit(LONG index, ::GUI::Window* window) //432B60.
	{
		digitWindows[index] = window;
		window->SetHidden(1);
		if (primaryPictures[index]) { delete primaryPictures[index]; primaryPictures[index] = NULL; }
		if (secondaryPictures[index]) { delete secondaryPictures[index]; secondaryPictures[index] = NULL; }
		CreatePrimaryDigitPicture(index);
		CreateSecondaryDigitPicture(index);
	}
	void CounterControl::SetDigit(LONG index, LONG value) //4309B0.
	{
		digitValues[index] = BYTE(value);
		if (primaryPictures[index])
		{
			primaryPictures[index]->Resize(digitWidth, digitHeight);
			primaryPictures[index]->LoadTexture(digitTextures[value], 0, 0, 0, -1, -1, 1);
			secondaryPictures[index]->SetHidden(0); //Actual430A16, then hidden again below.
		}
		if (secondaryPictures[index]) secondaryPictures[index]->SetHidden(1);
	}
	void CounterControl::SetValue(LONG value, DWORD mode) //4313B0.
	{
		if (value < minimum) value = minimum;
		if (value > maximum) value = maximum;
		const LONG before = GetValue();
		if (value == before && mode != 2) return;
		if (mode == 1) { AddValue(LONG(DWORD(value) - DWORD(before))); return; }
		if (!mode) { SetTarget(value); return; }
		currentValue = 0;
		const LONG count = GetDigitCount();
		for (LONG i = 0; i < count; ++i)
		{
			const LONG range = digitMaximums[i] - digitMinimums[i] + 1;
			LONG remainingValue = value;
			DWORD power = 1;
			if (i > 0)
			{
				const LONG base = radix;
				for (LONG remaining = i; remaining; --remaining)
				{
					remainingValue /= base;
					power *= DWORD(base);
				}
			}
			const LONG digit = remainingValue % radix % range;
			SetDigit(i, digit);
			currentValue = LONG(DWORD(currentValue) + DWORD(digit) * power);
		}
		//Original does not clear animating in this immediate branch.
	}
	void CounterControl::SetTarget(LONG value) //430C40; preserves live digit phases.
	{
		if (value > maximum) value = maximum;
		if (value < minimum) value = minimum;
		if (!animating)
		{
			animating = 1;
			transitionMode = 0;
			targetValue = value;
			const LONG count = GetDigitCount();
			for (LONG i = 0; i < count; ++i)
			{
				DWORD* times = &digitTimes[0];
				times[i] = timeGetTime();
			}
			return;
		}
		const LONG count = GetDigitCount();
		DWORD result = 0, target = 0;
		for (LONG i = 0; i < count; ++i)
		{
			LONG current = currentValue;
			LONG next;
			if (transitionMode == 1)
			{
				const LONG previousTarget = targetValue;
				const LONG step = previousTarget > current ? 1 : previousTarget < current ? -1 : 0;
				next = AddDigits(current, step);
			}
			else
			{
				next = targetValue;
			}
			LONG requested = value;
			const LONG high = digitMaximums[i];
			const LONG low = digitMinimums[i];
			const LONG range = high - low + 1;
			DWORD power = 1;
			if (i > 0)
			{
				const LONG base = radix;
				for (LONG remaining = i; remaining; --remaining)
				{
					current /= base;
					next /= base;
					requested /= base;
					power *= DWORD(base);
				}
			}
			const LONG base = radix;
			LONG digit = current % base % range;
			const LONG oldTarget = next % base % range;
			const LONG newTarget = requested % base % range;
			LONG direction;
			if (transitionMode == 1 && !i)
			{
				const LONG from = currentValue, to = targetValue;
				direction = from > to ? -1 : from < to ? 1 : 0;
			}
			else
			{
				direction = Direction(digit, oldTarget, low, high);
			}
			if (direction != Direction(digit, newTarget, low, high))
			{
				if (digit == oldTarget)
				{
					result += DWORD(digit) * power;
					DWORD* times = &digitTimes[0];
					times[i] = timeGetTime();
				}
				else
				{
					const DWORD start = transitionMode == 1 ? transitionTime : digitTimes[i];
					//Original stores the period as float, but not this elapsed difference.
					const volatile float period = float(double(animationStep) * 1000.0f);
					const double elapsed = double(timeGetTime()) - double(start);
					const DWORD phase = DWORD(static_cast<LONGLONG>(
						std::fmod(double(elapsed), double(period))));
					DWORD* times = &digitTimes[0];
					const DWORD now = timeGetTime();
					//Truncate the adjustment before adding the unsigned clock word.
					const DWORD adjustment = DWORD(static_cast<LONGLONG>(
						double(phase) - double(animationStep) * 1000.0f));
					times[i] = now + adjustment;
					if (phase)
					{
						digit = LONG(DWORD(digit) + DWORD(direction));
						if (digit > high) digit = low;
						else if (digit < low) digit = high;
					}
					result += DWORD(digit) * power;
				}
			}
			else
			{
				result += DWORD(digit) * power;
				if (transitionMode == 1)
				{
					const volatile float period = float(double(animationStep) * 1000.0f);
					const DWORD firstTime = timeGetTime();
					const DWORD start = transitionTime;
					DWORD* times = &digitTimes[0];
					const volatile float elapsed = float(double(firstTime) - double(start));
					const DWORD now = timeGetTime();
					const DWORD phase = DWORD(static_cast<LONGLONG>(
						std::fmod(double(elapsed), double(period))));
					times[i] = now - phase;
				}
			}
			target += DWORD(newTarget) * power;
		}
		currentValue = LONG(result);
		targetValue = LONG(target);
		transitionMode = 0;
	}
	void CounterControl::Update() //4314C0, both native animation modes in one owner.
	{
		if (!animating) return;
		if (!transitionMode)
		{
			DWORD result = 0;
			const LONG count = GetDigitCount();
			for (LONG index = 0; index < count; ++index)
			{
				LONG digit = currentValue, target = targetValue;
				const LONG high = digitMaximums[index], low = digitMinimums[index];
				const LONG range = high - low + 1;
				DWORD power = 1;
				for (LONG remaining = index; remaining > 0; --remaining)
				{
					digit /= radix;
					power *= DWORD(radix);
					target /= radix;
				}
				digit = digit % radix % range;
				target = target % radix % range;
				if (digit != target)
				{
					const LONG step = Direction(digit, target, low, high);
					DWORD* digitClock = &digitTimes[index];
					float elapsed = float(timeGetTime() - *digitClock) * 0.001f;
					while (elapsed > animationStep)
					{
						elapsed -= animationStep;
						digit = LONG(DWORD(digit) + DWORD(step));
						if (digit > digitMaximums[index]) digit = digitMinimums[index];
						else if (digit < digitMinimums[index]) digit = digitMaximums[index];
						if (digit == target) elapsed = animationStep;
						else *digitClock += DWORD(LONG(static_cast<LONGLONG>(animationStep * 1000.0f)));
					}
					const LONG direction = Direction(digit, target, low, high);
					LONG next = LONG(DWORD(digit) + DWORD(direction));
					const float fraction = elapsed / animationStep;
					if (next > digitMaximums[index]) next = digitMinimums[index];
					else if (next < digitMinimums[index]) next = digitMaximums[index];
					::GUI::Picture* primaryPicture = primaryPictures[index];
					::GUI::Picture* secondaryPicture = secondaryPictures[index];

					if (!direction)
					{
						primaryPicture->LoadTexture(digitTextures[next], 0, 0, 0, -1, -1, 1);
						primaryPicture->Resize(digitWidth, digitHeight);
						secondaryPicture->SetHidden(1);
					}
					else
					{
						LONG x, y;
						primaryPicture->GetAbsolutePosition(x, y);
						const LONG textureHeight = primaryPicture->pictureHeight;
						const LONG textureWidth = primaryPicture->pictureWidth;
						LONG primaryHeight;
						if (direction > 0)
						{
							const float scrolled = float(double(digitHeight) * fraction);
							const float textureBase = float(textureHeight);
							const float scrolledTexture = float(double(textureHeight) * fraction);
							primaryHeight = LONG(static_cast<LONGLONG>(double(digitHeight) - scrolled));
							primaryPicture->Resize(digitWidth, primaryHeight);
							const float width = float(textureWidth);
							const float remainingTexture = textureBase - scrolledTexture;
							primaryPicture->LoadTexture(digitTextures[digit], 0, 0, scrolledTexture, width, remainingTexture, 1);
							secondaryPicture->Resize(digitWidth, LONG(static_cast<LONGLONG>(scrolled)));
							secondaryPicture->LoadTexture(digitTextures[next], 0, 0, 0, width, scrolledTexture, 1);
						}
						else
						{
							const double scrolled = double(digitHeight) * fraction;
							const float remaining = float(double(digitHeight) - scrolled);
							const float scrolledTexture = float(double(textureHeight) * fraction);
							const float remainingTexture = float(double(textureHeight) - scrolledTexture);
							primaryHeight = LONG(static_cast<LONGLONG>(scrolled));
							primaryPicture->Resize(digitWidth, primaryHeight);
							const float width = float(textureWidth);
							primaryPicture->LoadTexture(digitTextures[next], 0, 0, remainingTexture, width, scrolledTexture, 1);
							secondaryPicture->Resize(digitWidth, LONG(static_cast<LONGLONG>(remaining)));
							secondaryPicture->LoadTexture(digitTextures[digit], 0, 0, 0, width, remainingTexture, 1);
						}
						secondaryPicture->SetAbsolutePosition(x, LONG(DWORD(y) + DWORD(primaryHeight)));
						secondaryPicture->SetHidden(0);
					}
				}
				result += DWORD(digit) * power;
			}
			currentValue = LONG(result);
			if (currentValue == targetValue) animating = 0;
			return;
		}
		float elapsed = float(timeGetTime() - transitionTime) * 0.001f;
		while (elapsed > animationStep)
		{
			elapsed -= animationStep;
			SetValue(AddDigits(currentValue, targetValue > currentValue ? 1 : -1), 2);
			if (currentValue == targetValue) animating = 0;
			if (animating)
				transitionTime += DWORD(LONG(static_cast<LONGLONG>(animationStep * 1000.0f)));
			else elapsed = animationStep;
		}
		const float fraction = elapsed / animationStep;
		const LONG count = GetDigitCount();
		for (LONG index = 0; index < count; ++index)
		{
			::GUI::Picture* primaryPicture = primaryPictures[index];
			::GUI::Picture* secondaryPicture = secondaryPictures[index];
			LONG digit = currentValue, next = digit;
			if (animating) next = AddDigits(digit, targetValue > digit ? 1 : -1);
			const LONG low = digitMinimums[index], high = digitMaximums[index];
			const LONG range = high - low + 1;
			if (index > 0)
			{
				const LONG base = radix;
				for (LONG remaining = index; remaining; --remaining)
				{
					digit /= base;
					next /= base;
				}
			}
			digit = digit % radix % range;
			next = next % radix % range;
			LONG direction = 0;
			if (digit == high && next == low) direction = 1;
			else if (digit == low && next == high) direction = -1;
			else if (digit < next) direction = 1;
			else if (digit > next) direction = -1;

			if (!direction)
			{
				primaryPicture->LoadTexture(digitTextures[next], 0, 0, 0, -1, -1, 1);
				primaryPicture->Resize(digitWidth, digitHeight);
				secondaryPicture->SetHidden(1);
			}
			else
			{
				LONG x, y;
				primaryPicture->GetAbsolutePosition(x, y);
				const LONG textureHeight = primaryPicture->pictureHeight;
				const LONG textureWidth = primaryPicture->pictureWidth;
				LONG primaryHeight;
				if (direction > 0)
				{
					const float scrolled = float(double(digitHeight) * fraction);
					const float textureBase = float(textureHeight);
					const float scrolledTexture = float(double(textureHeight) * fraction);
					primaryHeight = LONG(static_cast<LONGLONG>(double(digitHeight) - scrolled));
					primaryPicture->Resize(digitWidth, primaryHeight);
					const float width = float(textureWidth);
					const float remainingTexture = textureBase - scrolledTexture;
					primaryPicture->LoadTexture(digitTextures[digit], 0, 0, scrolledTexture, width, remainingTexture, 1);
					secondaryPicture->Resize(digitWidth, LONG(static_cast<LONGLONG>(scrolled)));
					secondaryPicture->LoadTexture(digitTextures[next], 0, 0, 0, width, scrolledTexture, 1);
				}
				else
				{
					const double scrolled = double(digitHeight) * fraction;
					const float remaining = float(double(digitHeight) - scrolled);
					const float scrolledTexture = float(double(textureHeight) * fraction);
					const float remainingTexture = float(double(textureHeight) - scrolledTexture);
					primaryHeight = LONG(static_cast<LONGLONG>(scrolled));
					primaryPicture->Resize(digitWidth, primaryHeight);
					const float width = float(textureWidth);
					primaryPicture->LoadTexture(digitTextures[next], 0, 0, remainingTexture, width, scrolledTexture, 1);
					secondaryPicture->Resize(digitWidth, LONG(static_cast<LONGLONG>(remaining)));
					secondaryPicture->LoadTexture(digitTextures[digit], 0, 0, 0, width, remainingTexture, 1);
				}
				secondaryPicture->SetAbsolutePosition(x, LONG(DWORD(y) + DWORD(primaryHeight)));
				secondaryPicture->SetHidden(0);
			}
		}
	}
	LONG CounterControl::AddDigits(LONG value, LONG delta) //430A50.
	{
		if (!delta) return value;
		LONG result = 0;
		const LONG count = GetDigitCount();
		LONG carry = 0;
		if (count <= 0) return result;

		const LONG valueMagnitude = std::abs(static_cast<int>(value));
		const LONG deltaMagnitude = std::abs(static_cast<int>(delta));
		const LONG base = radix;
		std::vector<LONG>::const_iterator minimum = digitMinimums.begin();
		std::vector<LONG>::const_iterator maximum = digitMaximums.begin();
		LONG index = 0;
		do
		{
			const LONG low = *minimum;
			const LONG range = *maximum - low + 1;
			LONG remaining = valueMagnitude;
			LONG amount = deltaMagnitude;
			const LONG valueSign = value > 0 ? 1 : value < 0 ? -1 : 0;
			const LONG deltaSign = delta > 0 ? 1 : delta < 0 ? -1 : 0;
			LONG power = 1;
			for (LONG place = index; place > 0; --place)
			{
				remaining /= base;
				amount /= base;
				power = LONG(DWORD(power) * DWORD(base));
			}
			const LONG deltaDigit = deltaSign * (amount % base % range);
			LONG digit = valueSign * (remaining % base % range) + (carry + deltaDigit);
			carry = 0;
			if (digit > *maximum) { carry = digit / range; digit %= range; }
			else if (digit < low) { carry = digit / range - 1; digit = digit % range + range; }
			result = LONG(DWORD(result) + DWORD(power) * DWORD(digit));
			++minimum;
			++maximum;
		} while (++index < count);
		return result;
	}
	void CounterControl::AddValue(LONG delta) //4310E0.
	{
		if (!delta) return;
		const LONG value = LONG(DWORD(GetValue()) + DWORD(delta));
		if (value < minimum || value > maximum) return;
		if (!animating)
		{
			const LONG current = currentValue;
			animating = 1;
			transitionMode = 1;
			targetValue = AddDigits(current, delta);
			transitionTime = timeGetTime();
			return;
		}

		const DWORD mode = transitionMode;
		if (mode == 1)
		{
			const LONG previousTarget = targetValue;
			const LONG previousCurrent = currentValue;
			const LONG difference = LONG(DWORD(previousTarget) - DWORD(previousCurrent));
			if (!difference) { animating = 0; return; }
			if ((delta == 1 && difference < 0) || (delta == -1 && difference > 0))
			{
				if (std::abs(static_cast<int>(difference)) == 1)
				{
					currentValue = previousTarget;
					targetValue = previousCurrent;
					const DWORD elapsed = timeGetTime() - transitionTime;
					const DWORD now = timeGetTime();
					transitionTime = DWORD(LONG(static_cast<LONGLONG>(double(elapsed) - animationStep * 1000.0f))) + now;
					return;
				}
				const LONG target = LONG(DWORD(previousTarget) + DWORD(delta));
				targetValue = target;
				if (target == previousCurrent) animating = 0;
				return;
			}
			targetValue = LONG(DWORD(previousTarget) + DWORD(delta));
		}
		else if (!mode) SetTarget(LONG(DWORD(targetValue) + DWORD(delta)));
	}
	BYTE CounterControl::Process(::GUI::Window* window, ::GUI::Event* event) //431280.
	{
		if (disabled) return 0;
		DWORD now;
		if (event->type == 4 || event->type == 7)
		{
			if (window->type != 1) return 0;
			if (window == incrementButton)
			{
				now = timeGetTime();
				pressTime = now;
				repeatTime = now;
				AddValue(1);
				return 1;
			}
			if (window != decrementButton) return 0;
			now = timeGetTime();
			pressTime = now;
		}
		else if (!event->type)
		{
			if (window == incrementButton)
			{
				if (static_cast<::GUI::Button*>(window)->GetState() == 2)
				{
					now = timeGetTime();
					if (pressTime + holdDelay < now && repeatTime + repeatDelay < now)
					{
						repeatTime = now;
						AddValue(1);
						return 1;
					}
				}
				return 0;
			}
			if (window != decrementButton || static_cast<::GUI::Button*>(window)->GetState() != 2)
				return 0;
			now = timeGetTime();
			if (pressTime + holdDelay >= now || repeatTime + repeatDelay >= now) return 0;
		}
		else return 0;
		repeatTime = now;
		AddValue(-1);
		return 1;
	}
	void CounterControl::Finish() //431D70.
	{
		if (animating) { SetValue(targetValue, 2); animating = 0; }
	}
}
