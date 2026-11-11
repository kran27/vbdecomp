// Existing source-derived reconstruction of the scanner in original 0x4A1610.
// Source donor: Microsoft WTL 7.5 atlmisc.h, CString::FormatV.
// Copyright (C) Microsoft Corporation. Distributed under CPL 1.0.
// See FormatScanner.LICENSE and FormatScanner.provenance.json.
// Shared implementation lineage does not prove an original WTL dependency.
// This adapted implementation remains nonmatching; preserve that limitation.
	enum _FormatModifiers
	{
		FORCE_ANSI =	0x10000,
		FORCE_UNICODE =	0x20000,
		FORCE_INT64 =	0x40000
	};

	va_list argListSave = argList;

	// make a guess at the maximum length of the resulting string
	int nMaxLen = 0;
	for (LPCTSTR lpsz = lpszFormat; *lpsz != _T('\0'); lpsz = _tcsinc(lpsz))
	{
		// handle '%' character, but watch out for '%%'
		if (*lpsz != _T('%') || *(lpsz = _tcsinc(lpsz)) == _T('%'))
		{
			// this is instead of _tclen()
#if !defined(_UNICODE) && defined(_MBCS)
			nMaxLen += _tclen(lpsz);
#else
			nMaxLen++;
#endif
			continue;
		}

		int nItemLen = 0;

		// handle '%' character with format
		int nWidth = 0;
		for (; *lpsz != _T('\0'); lpsz = _tcsinc(lpsz))
		{
			// check for valid flags
			if (*lpsz == _T('#'))
				nMaxLen += 2;   // for '0x'
			else if (*lpsz == _T('*'))
				nWidth = va_arg(argList, int);
			else if (*lpsz == _T('-') || *lpsz == _T('+') || *lpsz == _T('0') || *lpsz == _T(' '))
				;
			else // hit non-flag character
				break;
		}
		// get width and skip it
		if (nWidth == 0)
		{
			// width indicated by
			nWidth = _ttoi(lpsz);
			for (; *lpsz != _T('\0') && _istdigit(*lpsz); lpsz = _tcsinc(lpsz))
				;
		}
		ATLASSERT(nWidth >= 0);

		int nPrecision = 0;
		if (*lpsz == _T('.'))
		{
			// skip past '.' separator (width.precision)
			lpsz = _tcsinc(lpsz);

			// get precision and skip it
			if (*lpsz == _T('*'))
			{
				nPrecision = va_arg(argList, int);
				lpsz = _tcsinc(lpsz);
			}
			else
			{
				nPrecision = _ttoi(lpsz);
				for (; *lpsz != _T('\0') && _istdigit(*lpsz); lpsz = _tcsinc(lpsz))
					;
			}
			ATLASSERT(nPrecision >= 0);
		}

		// should be on type modifier or specifier
		int nModifier = 0;
		if(_tcsncmp(lpsz, _T("I64"), 3) == 0)
		{
			lpsz += 3;
			nModifier = FORCE_INT64;
		}
		else
		{
			switch (*lpsz)
			{
			// modifiers that affect size
			case _T('h'):
				nModifier = FORCE_ANSI;
				lpsz = _tcsinc(lpsz);
				break;
			case _T('l'):
				nModifier = FORCE_UNICODE;
				lpsz = _tcsinc(lpsz);
				break;

			// modifiers that do not affect size
			case _T('F'):
			case _T('N'):
			case _T('L'):
				lpsz = _tcsinc(lpsz);
				break;
			}
		}

		// now should be on specifier
		switch (*lpsz | nModifier)
		{
		// single characters
		case _T('c'):
		case _T('C'):
			nItemLen = 2;
			va_arg(argList, TCHAR);
			break;
		case _T('c') | FORCE_ANSI:
		case _T('C') | FORCE_ANSI:
			nItemLen = 2;
			va_arg(argList, char);
			break;
		case _T('c') | FORCE_UNICODE:
		case _T('C') | FORCE_UNICODE:
			nItemLen = 2;
			va_arg(argList, WCHAR);
			break;

		// strings
		case _T('s'):
		{
			LPCTSTR pstrNextArg = va_arg(argList, LPCTSTR);
			if (pstrNextArg == NULL)
			{
				nItemLen = 6;  // "(null)"
			}
			else
			{
				nItemLen = lstrlen(pstrNextArg);
				nItemLen = max(1, nItemLen);
			}
			break;
		}

		case _T('S'):
		{
#ifndef _UNICODE
			LPWSTR pstrNextArg = va_arg(argList, LPWSTR);
			if (pstrNextArg == NULL)
			{
				nItemLen = 6;  // "(null)"
			}
			else
			{
				nItemLen = (int)wcslen(pstrNextArg);
				nItemLen = max(1, nItemLen);
			}
#else // _UNICODE
			LPCSTR pstrNextArg = va_arg(argList, LPCSTR);
			if (pstrNextArg == NULL)
			{
				nItemLen = 6; // "(null)"
			}
			else
			{
#if defined(_WIN32_WCE) && (_ATL_VER >= 0x0800)
				nItemLen = ATL::lstrlenA(pstrNextArg);
#else
				nItemLen = lstrlenA(pstrNextArg);
#endif
				nItemLen = max(1, nItemLen);
			}
#endif // _UNICODE
			break;
		}

		case _T('s') | FORCE_ANSI:
		case _T('S') | FORCE_ANSI:
		{
			LPCSTR pstrNextArg = va_arg(argList, LPCSTR);
			if (pstrNextArg == NULL)
			{
				nItemLen = 6; // "(null)"
			}
			else
			{
#if defined(_WIN32_WCE) && (_ATL_VER >= 0x0800)
				nItemLen = ATL::lstrlenA(pstrNextArg);
#else
				nItemLen = lstrlenA(pstrNextArg);
#endif
				nItemLen = max(1, nItemLen);
			}
			break;
		}

		case _T('s') | FORCE_UNICODE:
		case _T('S') | FORCE_UNICODE:
		{
			LPWSTR pstrNextArg = va_arg(argList, LPWSTR);
			if (pstrNextArg == NULL)
			{
				nItemLen = 6; // "(null)"
			}
			else
			{
				nItemLen = (int)wcslen(pstrNextArg);
				nItemLen = max(1, nItemLen);
			}
			break;
		}
		}

		// adjust nItemLen for strings
		if (nItemLen != 0)
		{
			if (nPrecision != 0)
				nItemLen = min(nItemLen, nPrecision);
			nItemLen = max(nItemLen, nWidth);
		}
		else
		{
			switch (*lpsz)
			{
			// integers
			case _T('d'):
			case _T('i'):
			case _T('u'):
			case _T('x'):
			case _T('X'):
			case _T('o'):
				if (nModifier & FORCE_INT64)
					va_arg(argList, __int64);
				else
					va_arg(argList, int);
				nItemLen = 32;
				nItemLen = max(nItemLen, nWidth + nPrecision);
				break;

#ifndef _ATL_USE_CSTRING_FLOAT
			case _T('e'):
			case _T('f'):
			case _T('g'):
			case _T('G'):
				ATLASSERT(!"Floating point (%%e, %%E, %%f, %%g, and %%G) is not supported by the WTL::CString class.");
#ifndef _DEBUG
				::OutputDebugString(_T("Floating point (%%e, %%f, %%g, and %%G) is not supported by the WTL::CString class."));
#ifndef _WIN32_WCE
				::DebugBreak();
#else // CE specific
				DebugBreak();
#endif // _WIN32_WCE
#endif // !_DEBUG
				break;
#else // _ATL_USE_CSTRING_FLOAT
			case _T('e'):
			case _T('g'):
			case _T('G'):
				va_arg(argList, double);
				nItemLen = 128;
				nItemLen = max(nItemLen, nWidth + nPrecision);
				break;
			case _T('f'):
				{
					double f;
					LPTSTR pszTemp;

					// 312 == strlen("-1+(309 zeroes).")
					// 309 zeroes == max precision of a double
					// 6 == adjustment in case precision is not specified,
					//   which means that the precision defaults to 6
					pszTemp = (LPTSTR)_alloca(max(nWidth, 312 + nPrecision + 6) * sizeof(TCHAR));

					f = va_arg(argList, double);
					_stprintf(pszTemp, _T("%*.*f"), nWidth, nPrecision + 6, f);
					nItemLen = _tcslen(pszTemp);
				}
				break;
#endif // _ATL_USE_CSTRING_FLOAT

			case _T('p'):
				va_arg(argList, void*);
				nItemLen = 32;
				nItemLen = max(nItemLen, nWidth + nPrecision);
				break;

			// no output
			case _T('n'):
				va_arg(argList, int*);
				break;

			default:
				ATLASSERT(FALSE);  // unknown formatting option
			}
		}

		// adjust nMaxLen for output nItemLen
		nMaxLen += nItemLen;
	}

