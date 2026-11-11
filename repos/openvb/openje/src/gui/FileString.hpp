#pragma once

#include <windows.h>
#include <cstdio>
#include <string>

namespace GUI
{
	//Actual cdecl FILE-string operations; names are descriptive. These use
	//DWORD lengths and differ from the saved-map WORD-length helpers.
	BYTE ReadFileString(std::string& value, FILE* file); //50B7F0.
	BYTE WriteFileString(const std::string& value, FILE* file); //50B790.
}
