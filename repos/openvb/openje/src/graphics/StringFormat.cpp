#include "StringFormat.hpp"
#include "VfxParameters.hpp"
#include "VfxValueOwners.hpp"
#include <windows.h>
#include <d3dx8.h>
#include <cstdarg>
#include <cstdlib>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <malloc.h>
#ifndef _MBCS
#define _MBCS
#endif
#include <tchar.h>

// The original helper selects the actual multibyte CRT rather than CharNext.
// The retained source-derived scanner is provisional; see its provenance file.
#define _ATL_USE_CSTRING_FLOAT
#define ATLASSERT(expression) ((void)0)
#ifndef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#endif
#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

namespace JE
{

	namespace
	{
		static const char* FindTokenDelimiter(const char* delimiters, int character)
		{
			return delimiters ? reinterpret_cast<const char*>(_mbschr(
				reinterpret_cast<const unsigned char*>(delimiters), character)) : NULL;
		}
	}

	unsigned char __cdecl IsStringBlank(const char* input, const char* delimiters) //4A1170.
	{
		if (!input || !*input) return 1;
		if (!delimiters) return 0;
		const size_t delimiterCount = strlen(delimiters);
		if (!delimiterCount) return 0;
		for (int index = int(strlen(input)) - 1; index >= 0; --index)
			if (!FindTokenDelimiter(delimiters, input[index])) return 0;
		return 1;
	}

	char* __cdecl TrimString(char* output, const char* input, const char* delimiters) //4A10A0.
	{
		if (!output) return NULL;
		if (!input)
		{
			*output = 0;
			return output;
		}
		int read = 0;
		int written = 0;
		while (input[read] && FindTokenDelimiter(delimiters, input[read])) ++read;
		// Empty/all-delimiter input leaves the existing output buffer untouched.
		if (!input[read]) return output;
		for (;;)
		{
			while (input[read] && !FindTokenDelimiter(delimiters, input[read]))
			{
				output[written] = input[read];
				++written;
				++read;
			}
			while (input[read] && FindTokenDelimiter(delimiters, input[read])) ++read;
			if (!input[read]) break;
			output[written] = ' ';
			++written;
		}
		output[written] = 0;
		return output;
	}

	unsigned char IsTokenLeadByte(const char* input)
	{
		return IsDBCSLeadByte(static_cast<BYTE>(*input)) != 0;
	}

	const char* FindTokenChar(const char* input, char character)
	{
		return strchr(input, character);
	}

	const char* TokenizeBytes(char* output, const char* input, int capacity,
		const char* delimiters, unsigned char copyLeading)
	{
		if (capacity <= 0) return NULL;
		if (output) *output = 0;
		if (!input) return NULL;
		int written = 0;
		bool started = false;
		while (*input)
		{
			char current = *input++;
			if (copyLeading)
			{
				if (!started)
				{
					if (!FindTokenDelimiter(delimiters, current)) started = true;
				}
				else if (FindTokenDelimiter(delimiters, current)) break;
			}
			else if (FindTokenDelimiter(delimiters, current)) break;
			if (output)
			{
				++written;
				*output = current;
				if (written >= capacity) break;
				++output;
			}
		}
		if (output) *output = 0;
		if (copyLeading)
			while (*input && FindTokenDelimiter(delimiters, *input)) ++input;
		return input;
	}

	const char* TokenizeMultibyte(char* output, const char* input, int capacity,
		const char* delimiters, unsigned char copyLeading)
	{
		if (capacity <= 1) return NULL;
		if (output) *output = 0;
		if (!input) return NULL;
		int written = 0;
		bool started = false;
		while (*input)
		{
			if (copyLeading)
			{
				if (IsTokenLeadByte(input))
				{
					if (output)
					{
						output[0] = input[0];
						output[1] = input[1];
						written += 2;
						if (written >= capacity) break;
						output += 2;
					}
					input += 2;
				}
				else
				{
					char current = *input++;
					if (!started)
					{
						if (!FindTokenDelimiter(delimiters, current)) started = true;
					}
					else if (FindTokenDelimiter(delimiters, current)) break;
					if (output)
					{
						++written;
						*output = current;
						if (written >= capacity) break;
						++output;
					}
				}
			}
			else
			{
				if (IsTokenLeadByte(input))
				{
					if (output)
					{
						output[0] = input[0];
						output[1] = input[1];
						written += 2;
						if (written >= capacity) break;
						output += 2;
					}
					input += 2;
				}
				else
				{
					char current = *input++;
					if (FindTokenDelimiter(delimiters, current)) break;
					if (output)
					{
						++written;
						*output = current;
						if (written >= capacity) break;
						++output;
					}
				}
			}
		}
		if (output) *output = 0;
		if (copyLeading)
			while (*input && !IsTokenLeadByte(input) && FindTokenChar(delimiters, *input)) ++input;
		return input;
	}

	const char* __cdecl Tokenize(char* output, const char* input, int capacity,
		const char* delimiters, unsigned char multibyte, unsigned char copyLeading)
	{
		if (multibyte) return TokenizeMultibyte(output, input, capacity, delimiters, copyLeading);
		return TokenizeBytes(output, input, capacity, delimiters, copyLeading);
	}

	DWORD ReadVfxBuffer(void* output, DWORD size, DWORD count, const BYTE* buffer, DWORD& offset)
	{
		if (buffer) memcpy(output, buffer + offset, size * count);
		offset += size * count; //4B8117 reloads the live offset after copying.
		return size * count;
	}

	namespace
	{
		// Reused FormatV scanner with the game's std::string ownership tail.
		// The mixed dependency body receives no proprietary matching credit.
		static void FormatStringV(std::string& output, const char* lpszFormat, va_list argList)
		{
#include "FormatScanner.inl"
			if (nMaxLen > 0)
			{
				char* text = new char[nMaxLen + 1];
				std::vsprintf(text, lpszFormat, argListSave);
				output.assign(text, std::strlen(text));
				delete[] text;
			}
		}
	}

	void __cdecl FormatString(std::string& output, const char* format, ...) //4A1A30.
	{
		va_list arguments;
		va_start(arguments, format);
		FormatStringV(output, format, arguments);
		va_end(arguments);
	}

	std::string __cdecl FormatFloat(float input, int precision) //4A1A40.
	{
		std::string result;
		if (precision < 0)
		{
			double integerPart;
			if (std::modf(static_cast<double>(input), &integerPart) != 0.0)
			{
				std::string format;
				//Original uses the real CRT/compiler absolute-value operation.
				const int magnitude = std::abs(precision);
				FormatString(format, "%df", magnitude);
				format = "%.0" + format;
				FormatString(result, format.c_str(), static_cast<double>(input));
			}
			else
			{
				//Original FTOL returns64 bits; only EAX is passed to signed%d.
				const int value = static_cast<int>(static_cast<__int64>(input));
				FormatString(result, "%d", value);
			}
		}
		else
		{
			std::string format;
			FormatString(format, "%df", precision);
			format = "%.0" + format;
			FormatString(result, format.c_str(), static_cast<double>(input));
		}
		return result;
	}

	std::string __cdecl FormatVector2(D3DXVECTOR2 input, int precision) //52F7A0.
	{
		std::string result;
		{
			FormatString(result, "%s %s", FormatFloat(input.x, precision).c_str(),
				FormatFloat(input.y, precision).c_str());
		}
		return std::string(result);
	}

	std::string __cdecl FormatVector3(D3DXVECTOR3 input, int precision) //523B50.
	{
		std::string result;
		{
			FormatString(result, "%s %s %s", FormatFloat(input.x, precision).c_str(),
				FormatFloat(input.y, precision).c_str(), FormatFloat(input.z, precision).c_str());
		}
		return std::string(result);
	}

	std::string __cdecl FormatUVs(D3DXVECTOR4 input, int precision) //52EAC0.
	{
		std::string result;
		{
			FormatString(result, "%s %s %s %s",
				FormatFloat(input.x, precision).c_str(), FormatFloat(input.y, precision).c_str(),
				FormatFloat(input.z, precision).c_str(), FormatFloat(input.w, precision).c_str());
		}
		return std::string(result);
	}

	VFX_Target __cdecl ParseTargetValue(const char* input) //502170.
	{
		VFX_Target result;
		result = 0;
		const std::string text(input ? input : "");
		char token[1024];
		const char* cursor = Tokenize(token, text.c_str(), 1024, " \t\n\r,;", 0, 1);
		const LONG label = FindVFXTargetLabel(std::string(token));
		if (label == 3)
		{
			Tokenize(token, cursor, 1024, " \t\n\r,;", 0, 1);
			result = static_cast<DWORD>(std::atol(token)) + static_cast<DWORD>(label);
		}
		else
			result = static_cast<DWORD>(label);
		return result;
	}

	VFX_ColorBytes __cdecl ParseColor(const char* input) //52E920.
	{
		VFX_ColorBytes result;
		const std::string text(input ? input : "");
		char token[1024];
		const char* cursor = text.c_str();
		cursor = Tokenize(token, cursor, 1024, " \t\n\r,;", 0, 1);
		result.values[0] = static_cast<BYTE>(std::atol(token));
		cursor = Tokenize(token, cursor, 1024, " \t\n\r,;", 0, 1);
		result.values[1] = static_cast<BYTE>(std::atol(token));
		Tokenize(token, cursor, 1024, " \t\n\r,;", 0, 1);
		result.values[2] = static_cast<BYTE>(std::atol(token));
		return result;
	}

	std::string __cdecl FormatColor(VFX_ColorBytes input) //52E860.
	{
		std::string result;
		FormatString(result, "%d %d %d", static_cast<int>(input.values[0]),
			static_cast<int>(input.values[1]), static_cast<int>(input.values[2]));
		return std::string(result);
	}

	VFX_Vector __cdecl ParseVector3Value(const char* input) //523D00.
	{
		VFX_Vector result;
		const std::string text(input ? input : "");
		char token[1024];
		const char* cursor = text.c_str();
		cursor = Tokenize(token, cursor, 1024, " \t\n\r,;", 0, 1);
		result.xy.values[0] = static_cast<float>(std::atof(token));
		cursor = Tokenize(token, cursor, 1024, " \t\n\r,;", 0, 1);
		result.xy.values[1] = static_cast<float>(std::atof(token));
		Tokenize(token, cursor, 1024, " \t\n\r,;", 0, 1);
		result.z = static_cast<float>(std::atof(token));
		return result;
	}

	VFX_Vector2 __cdecl ParseVector2Value(const char* input) //52F8F0.
	{
		VFX_Vector2 result;
		const std::string text(input ? input : "");
		char token[1024];
		const char* cursor = text.c_str();
		cursor = Tokenize(token, cursor, 1024, " \t\n\r,;", 0, 1);
		result.values[0] = static_cast<float>(std::atof(token));
		Tokenize(token, cursor, 1024, " \t\n\r,;", 0, 1);
		result.values[1] = static_cast<float>(std::atof(token));
		return result;
	}

	VFX_TextureCoordinates __cdecl ParseTextureUVs(const char* input) //52ECE0.
	{
		VFX_TextureCoordinates result;
		const std::string text(input ? input : "");
		char token[1024];
		const char* cursor = text.c_str();
		cursor = Tokenize(token, cursor, 1024, " \t\n\r,;", 0, 1);
		result.values[0] = static_cast<float>(std::atof(token));
		cursor = Tokenize(token, cursor, 1024, " \t\n\r,;", 0, 1);
		result.values[1] = static_cast<float>(std::atof(token));
		cursor = Tokenize(token, cursor, 1024, " \t\n\r,;", 0, 1);
		result.values[2] = static_cast<float>(std::atof(token));
		Tokenize(token, cursor, 1024, " \t\n\r,;", 0, 1);
		result.values[3] = static_cast<float>(std::atof(token));
		return result;
	}

	VFX_Rotation __cdecl ParseRotation(const char* input) //52C290.
	{
		const VFX_Vector parsed = ParseVector3Value(input);
		VFX_Rotation result;
		std::memcpy(result.values, &parsed, sizeof(parsed));
		return result;
	}

	std::string __cdecl FormatRotation(D3DXVECTOR3 input, int precision) //52C2F0.
	{
		VFX_Vector value(input.x, input.y, input.z);
		D3DXVECTOR3 components;
		std::memcpy(&components, &value, sizeof(components));
		return FormatVector3(components, precision);
	}

	std::string __cdecl FormatVector2(float x, float y, int precision)
	{
		return FormatVector2(D3DXVECTOR2(x, y), precision);
	}

	std::string __cdecl FormatVector3(float x, float y, float z, int precision)
	{
		return FormatVector3(D3DXVECTOR3(x, y, z), precision);
	}

	std::string __cdecl FormatUVs(float x, float y, float z, float w, int precision)
	{
		return FormatUVs(D3DXVECTOR4(x, y, z, w), precision);
	}

	std::string __cdecl FormatRotation(float x, float y, float z, int precision)
	{
		return FormatRotation(D3DXVECTOR3(x, y, z), precision);
	}
}

namespace JE
{
	void ReadVfxString(std::string& output, const BYTE* buffer, DWORD& offset)
	{
		output = "";
		WORD length; //NULL input leaves this original output unwritten.
		ReadVfxBuffer(&length, 2, 1, buffer, offset);
		if (length > 0)
		{
			if (buffer)
			{
				const DWORD count = length;
				char* text = static_cast<char*>(::operator new[](count + 1));
				if (text)
				{
					memcpy(text, buffer + offset, count);
					text[length] = 0;
					output.assign(text, strlen(text));
					::operator delete[](text); //No exception cleanup if assignment throws.
				}
			}
		}
		offset += length; //Native also reloads/stores the live cursor for length0.
	}
}

namespace JE
{
	//0x4B8340: both the prefix and the copied extent use the low WORD.
	void WriteVfxString(const std::string& input, BYTE* buffer, DWORD& offset)
	{
		DWORD length = static_cast<WORD>(input.size());
		WriteVfxBuffer(&length, 2, 1, buffer, offset);
		const DWORD count = length;
		if (static_cast<WORD>(count) > 0)
		{
			if (buffer) memcpy(buffer + offset, input.data(), static_cast<WORD>(count));
		}
		offset += static_cast<WORD>(count);
	}
}
