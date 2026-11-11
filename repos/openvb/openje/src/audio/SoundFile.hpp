#pragma once

#include <windows.h>
#include <string>

namespace JE
{
	// Actual CSoundFile. Read and Parse are the two original virtual slots;
	// its string-only destructor is not virtual.
	class SoundFile
	{
	public:
		SoundFile(); // 4C84C0.
		~SoundFile(); // 4C7E80.
		virtual int Read(const char* name); // 4C8540.
		virtual void Parse(const BYTE* input, DWORD size) = 0;
		void ReadToken(char* output, int capacity); // 4C7180.

		LONG cursor;
		LONG dataSize;
		const BYTE* data;
		std::string resourceName;
	};
}
