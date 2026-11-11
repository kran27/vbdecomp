#pragma once

#include <windows.h>

// Only the ABI prefix actually read by Video is declared. The supplied Bink1.6d
// DLL owns the remainder and implements the decoder; this is not sizeof(BINK).
struct BinkMovie
{
	DWORD Width, Height, Frames, FrameNum;
};

extern "C"
{
	__declspec(dllimport) BinkMovie* __stdcall BinkOpen(const char* filename, DWORD flags);
	__declspec(dllimport) void __stdcall BinkClose(BinkMovie* movie);
	__declspec(dllimport) LONG __stdcall BinkWait(BinkMovie* movie);
	__declspec(dllimport) LONG __stdcall BinkDoFrame(BinkMovie* movie);
	__declspec(dllimport) void __stdcall BinkNextFrame(BinkMovie* movie);
	__declspec(dllimport) LONG __stdcall BinkCopyToBuffer(BinkMovie* movie, void* destination,
		LONG pitch, DWORD height, DWORD x, DWORD y, DWORD flags);
	__declspec(dllimport) const char* __stdcall BinkGetError();
	typedef void* (__stdcall* BinkSoundOpen)(DWORD);
	__declspec(dllimport) void* __stdcall BinkOpenDirectSound(DWORD parameter);
	__declspec(dllimport) LONG __stdcall BinkSetSoundSystem(BinkSoundOpen open, DWORD parameter);
}
