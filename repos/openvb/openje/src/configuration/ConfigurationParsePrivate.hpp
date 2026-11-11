#pragma once

#include "Configuration.hpp"
#include <cctype>
#include <cstring>

namespace JE
{
	namespace Configuration
	{
		namespace Detail
		{
			namespace
			{
				int IsSpace(const char* pText)
				{
					// The proprietary caller sign-extends char into the CRT isspace API.
					// Defined comparison coverage is the ordinary C-locale ASCII domain.
					return std::isspace(static_cast<signed char>(*pText));
				}
			}

			// 0x4E0540.
			static unsigned char ParseBuffer(char* pBuffer, Settings& settings,
				Net::UInt32 nLength)
			{
				char* pCursor = pBuffer;
				char* const pEnd = pBuffer + nLength;
				// Original argument storage holds the length bits until a section is
				// encountered. Do not fabricate a default section for a malformed file.
				Values* pSection = reinterpret_cast<Values*>(static_cast<::uintptr_t>(nLength));
				while (pCursor < pEnd)
				{
					while (IsSpace(pCursor))
					{
						if (++pCursor >= pEnd)
						{
							return 1;
						}
					}
					if (pCursor >= pEnd)
					{
						return 1;
					}
					if (*pCursor == ';')
					{
						do
						{
							if (*pCursor == '\n')
							{
								break;
							}
							++pCursor;
						}
						while (pCursor < pEnd);
						++pCursor;
						continue;
					}
					if (*pCursor == '[')
					{
						do
						{
							if (++pCursor >= pEnd)
							{
								return 1;
							}
						}
						while (IsSpace(pCursor));
						char* const pStart = pCursor;
						while (*pCursor != ']')
						{
							if (++pCursor >= pEnd)
							{
								return 1;
							}
						}
						if (pCursor >= pEnd)
						{
							return 1;
						}
						char* pCut = pCursor;
						char* pPrevious = pCursor - 1;
						while (IsSpace(pPrevious))
						{
							pCut = pPrevious--;
						}
						*pCut = '\0';
						{
							std::string sSection(pStart, std::strlen(pStart));
							pSection = &settings[sSection];
						}
						++pCursor;
						continue;
					}

					char* const pKeyStart = pCursor;
					while (*pCursor != '=')
					{
						if (++pCursor >= pEnd)
						{
							return 1;
						}
					}
					if (pCursor >= pEnd)
					{
						return 1;
					}
					char* pKeyLast = pCursor - 1;
					while (IsSpace(pKeyLast))
					{
						--pKeyLast;
					}
					++pCursor;
					pKeyLast[1] = '\0';
					char* pValueStart = pCursor;
					while (pCursor < pEnd && *pCursor != '\n')
					{
						++pCursor;
					}
					char* pValueLast = pCursor;
					if (pCursor > pValueStart)
					{
						bool bReachedStart = false;
						while (IsSpace(pValueLast))
						{
							if (--pValueLast <= pValueStart)
							{
								bReachedStart = true;
								break;
							}
						}
						if (!bReachedStart)
						{
							while (pValueStart < pValueLast && IsSpace(pValueStart))
							{
								++pValueStart;
							}
						}
					}
					// Do not normalize an empty or all-whitespace value: this can retain
					// its first byte and can overwrite the next line's first byte.
					pValueLast[1] = '\0';
					if (pSection)
					{
						const std::string sValue(pValueStart, std::strlen(pValueStart));
						const std::string sKey(pKeyStart, std::strlen(pKeyStart));
						(*pSection)[sKey] = sValue;
					}
					++pCursor;
				}
				return 1;
			}
		}
	}
}
