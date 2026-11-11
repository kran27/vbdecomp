#pragma once

#include <windows.h>
#include <stdio.h>
#include <string>

// The shared Name methods receive the address of an ordinary VC7 string.
// G3D_Name is a descriptive recovered name; the original spelling and whether
// its string was a base or first member are unknown. Reuse the real string
// object, with no additional data, fixed-storage overlay or custom runtime.
class G3D_Name : public std::string
{
public:
	G3D_Name& operator=(const std::string& value)
	{
		std::string::operator=(value);
		return *this;
	}
	G3D_Name& operator=(const char* value)
	{
		std::string::operator=(value);
		return *this;
	}
	void ReadText(); //4D05E0; parenthesized name and real temporary string.
	void Read(FILE* stream); //4D0670; actual member-to-cdecl forwarding API.
	void Read(const BYTE* data, DWORD& offset); //4D0690.
	void Write(FILE* stream) const; //4CE940; real string-by-value writer boundary.
	std::string Value() const; //4CE890; actual by-value string boundary, historical method spelling unknown.
	void WriteText(FILE* stream, BYTE newline) const; //4CA8C0.
};

#if defined(_MSC_VER) && _MSC_VER == 1300 && defined(_M_IX86)
typedef char G3DName_RealStringExtent[sizeof(G3D_Name) == sizeof(std::string) ? 1 : -1];
#endif
