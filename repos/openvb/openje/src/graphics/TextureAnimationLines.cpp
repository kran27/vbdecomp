#include "TextureAnimationLines.hpp"
#include <cstring>
#include <mbstring.h>

BYTE Gfx_ReadTextureAnimationLine(const char** next, int remaining,
	const char* source, int capacity, char* output)
{
	return Gfx_TextureAnimationDetail::ReadLine(next, remaining, source, capacity, output);
}

BYTE Gfx_IsTextureAnimationWhitespace(const char* text, const char* acceptedBytes)
{
	if (!text || !*text) return 1;
	if (!acceptedBytes || !std::strlen(acceptedBytes)) return 0;
	int index = static_cast<int>(std::strlen(text) - 1U);
	for (; index >= 0; --index)
	{
		const unsigned int character = static_cast<unsigned int>(
			static_cast<signed char>(text[index]));
		if (!_mbschr(reinterpret_cast<const unsigned char*>(acceptedBytes), character)) return 0;
	}
	return 1;
}
