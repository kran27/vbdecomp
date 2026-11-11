#pragma once

#include <windows.h>
#include <mmsystem.h>
#include <strmif.h>
#include <control.h>
#include <dmusicc.h>
#undef INTERFACE
#include <dmusici.h>
#include <list>

namespace JE
{
	class DirectShowAudio
	{
	public:
		DirectShowAudio(); //539D90.
		~DirectShowAudio(); //404B70, folded empty ordinary destructor.
		HRESULT Initialize(); //539B70.
		void Close(); //539C00.
		int Stop(); //539C90.
		LONG CalculateVolume(); //539CC0.
		int SetVolume(LONG volume, BYTE force); //539DF0.
		int RefreshVolume(BYTE force); //539E50.
		void Update(); //539E70, caller discards the unconstrained return register.

		IGraphBuilder* graph;
		IMediaControl* control;
		IMediaSeeking* seeking;
		IMediaPosition* position;
		IBasicAudio* audio;
		IUnknown* interface14;
		IUnknown* interface18;
		LONG state;
		BYTE initialized;
		LONG baseVolume, effectiveVolume, lastVolume, mixVolume;
		DWORD fadeFlags, fadeStart, fadeDuration, fadeFrom, fadeTo;
	};

	struct DirectMusicSegment
	{
		LONG CalculateVolume(); //539FB0.
		int SetVolume(LONG volume, BYTE force); //53A080.
		int RefreshVolume(BYTE force); //53A0E0.
		HRESULT Update(); //53A100.
		IDirectMusicSegment* segment;
		IDirectMusicAudioPath* path;
		void* value08;
		IDirectMusicPerformance8* performance;
		DWORD value10, value14, flags;
		LONG baseVolume, effectiveVolume, lastVolume, mixVolume;
		DWORD fadeStart, fadeDuration, fadeFrom, fadeTo;
	};

	class DirectMusicAudio
	{
	public:
		DirectMusicAudio(); //53A1E0.
		~DirectMusicAudio(); //53A180; borrows, never deletes segment elements.
		HRESULT Initialize(HWND window); //539EF0.
		void Close(); //539F60.
		void Update(); //53A150, caller discards the unconstrained return register.
		IDirectMusicLoader8* loader;
		IDirectMusicPerformance8* performance;
		std::list<DirectMusicSegment*> segments;
	};

	class CDAudio
	{
	public:
		CDAudio(); //53A510.
		~CDAudio(); //404B70, folded empty ordinary destructor.
		MCIERROR Initialize(); //53A550.
		MCIERROR Stop(); //53A640, may initialize/reopen first.
		MCIERROR Close(); //53A6D0.
		void Update(); //53A720.
		LONG Volume() const; //5C0190, shared original getter.
		int SetVolume(LONG volume, BYTE force); //53A670.
		int RefreshVolume(BYTE force); //53A6B0.
		DWORD ReadMixerVolume(LONG& left, LONG& right); //53A220.
		void SetMixerVolume(LONG left, LONG right); //53A320.
		int OpenMixer(DWORD_PTR callback); //53A3E0.
		MMRESULT CloseMixer(); //53A4F0.

		MCIDEVICEID deviceID;
		DWORD mode, trackCount, seconds, currentPosition;
		LONG baseVolume, effectiveVolume, lastVolume;
		HMIXER mixer;
		DWORD maximum, minimum, volumeControl, previousClock;
	};
}
