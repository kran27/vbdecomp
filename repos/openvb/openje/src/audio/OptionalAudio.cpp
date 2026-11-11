#include "OptionalAudio.hpp"
#include "SoundSystem.hpp"
#include <uuids.h>
#include <cstddef>

namespace JE
{
	typedef char ShowSize[(sizeof(DirectShowAudio) == 0x48) ? 1 : -1];
	typedef char ShowFlag[(offsetof(DirectShowAudio, initialized) == 0x20) ? 1 : -1];
	typedef char ShowVolume[(offsetof(DirectShowAudio, baseVolume) == 0x24) ? 1 : -1];
#if defined(_MSC_VER) && _MSC_VER == 1300 && defined(_M_IX86)
	typedef char MusicSize[(sizeof(DirectMusicAudio) == 0x14) ? 1 : -1];
#endif
	typedef char MusicList[(offsetof(DirectMusicAudio, segments) == 8) ? 1 : -1];
	typedef char SegmentSize[(sizeof(DirectMusicSegment) == 0x3C) ? 1 : -1];
	typedef char SegmentFlags[(offsetof(DirectMusicSegment, flags) == 0x18) ? 1 : -1];
	typedef char CdSize[(sizeof(CDAudio) == 0x34) ? 1 : -1];
	typedef char CdMixer[(offsetof(CDAudio, mixer) == 0x20) ? 1 : -1];
	typedef char CdClock[(offsetof(CDAudio, previousClock) == 0x30) ? 1 : -1];
	typedef char MciOpenSize[(sizeof(MCI_OPEN_PARMSA) == 20) ? 1 : -1];
	typedef char MciSetSize[(sizeof(MCI_SET_PARMS) == 12) ? 1 : -1];
	typedef char MciStatusSize[(sizeof(MCI_STATUS_PARMS) == 16) ? 1 : -1];
	typedef char MixerLineSize[(sizeof(MIXERLINEA) == 168) ? 1 : -1];
	typedef char MixerControlSize[(sizeof(MIXERCONTROLA) == 148) ? 1 : -1];
	typedef char MixerLineControlsSize[(sizeof(MIXERLINECONTROLSA) == 24) ? 1 : -1];
	typedef char MixerDetailsSize[(sizeof(MIXERCONTROLDETAILS) == 24) ? 1 : -1];
	typedef char MixerValueSize[(sizeof(MIXERCONTROLDETAILS_UNSIGNED) == 4) ? 1 : -1];

	DirectShowAudio::DirectShowAudio() //539D90.
	{
		graph = NULL; control = NULL; seeking = NULL; position = NULL;
		audio = NULL; interface14 = NULL; interface18 = NULL;
		state = 0; initialized = 0; baseVolume = 100;
		effectiveVolume = CalculateVolume(); //Original reads the still-unwritten fade members.
		fadeFlags = 0;
		fadeStart = 0;
		fadeDuration = 0;
		fadeFrom = 0;
		fadeTo = 0;
		lastVolume = 100; mixVolume = 100;
	}
	DirectShowAudio::~DirectShowAudio() {} //404B70; Close is explicit.
	CDAudio::~CDAudio() {} //404B70; Close is explicit.
	HRESULT DirectShowAudio::Initialize() //539B70.
	{
		const HRESULT initializedCom = CoInitialize(NULL);
		if (initializedCom < 0) return initializedCom;
		const HRESULT created = CoCreateInstance(CLSID_FilterGraph, NULL, 3,
			IID_IGraphBuilder, reinterpret_cast<void**>(&graph));
		if (created < 0
			|| graph->QueryInterface(IID_IMediaControl, reinterpret_cast<void**>(&control)) < 0
			|| graph->QueryInterface(IID_IMediaSeeking, reinterpret_cast<void**>(&seeking)) < 0
			|| graph->QueryInterface(IID_IMediaPosition, reinterpret_cast<void**>(&position)) < 0
			|| graph->QueryInterface(IID_IBasicAudio, reinterpret_cast<void**>(&audio)) < 0)
			return created; //A QueryInterface failure retains the creation HRESULT.
		initialized = 1;
		return S_OK;
	}
	void DirectShowAudio::Close() //539C00.
	{
		if (control) control->Stop();
		if (interface18) { interface18->Release(); interface18 = NULL; }
		if (interface14) { interface14->Release(); interface14 = NULL; }
		if (audio) { audio->Release(); audio = NULL; }
		if (position) { position->Release(); position = NULL; }
		if (seeking) { seeking->Release(); seeking = NULL; }
		if (control) { control->Release(); control = NULL; }
		if (graph) { graph->Release(); graph = NULL; }
		CoUninitialize();
		initialized = 0;
	}
	int DirectShowAudio::Stop() //539C90.
	{
		if (!initialized) return 9;
		return control && control->Stop() ? 9 : 0;
	}
	LONG DirectShowAudio::CalculateVolume() //539CC0.
	{
		const DWORD clock=AudioBackend::g_System->now;
		const DWORD previous=fadeFlags;
		if (previous & 1)
		{
			if (clock >= fadeStart)
			{
				if (clock >= fadeDuration + fadeStart)
				{
					mixVolume=LONG(fadeTo);
					fadeFlags=previous & ~DWORD(1);
				}
				else mixVolume=LONG(fadeFrom+(clock-fadeStart)*(fadeTo-fadeFrom)/fadeDuration);
			}
		}
		else if (previous & 2)
		{
			if (clock >= fadeStart)
			{
				if (clock >= fadeDuration + fadeStart)
				{
					mixVolume=LONG(fadeTo);
					fadeFlags=previous & ~DWORD(2);
				}
				else mixVolume=LONG(fadeFrom-(clock-fadeStart)*(fadeFrom-fadeTo)/fadeDuration);
			}
		}
		if (mixVolume < 0) mixVolume=0;
		if (mixVolume > 100) mixVolume=100;
		return LONG(DWORD(AudioBackend::g_System->masterVolume) * DWORD(baseVolume)
			* DWORD(mixVolume)) / 10000;
	}
	int DirectShowAudio::SetVolume(LONG volume, BYTE force) //539DF0.
	{
		if (!initialized || !audio) return 9;
		baseVolume = volume;
		const LONG calculated = CalculateVolume();
		const bool unchanged = calculated == lastVolume;
		effectiveVolume = calculated;
		if (!unchanged || force == 1)
		{
			const LONG attenuation = AudioBackend::g_System->LookupVolume(calculated);
			const HRESULT result = audio->put_Volume(attenuation);
			lastVolume = effectiveVolume;
			if (result) return 9;
		}
		return 0;
	}

	int DirectShowAudio::RefreshVolume(BYTE force) { return SetVolume(baseVolume, force); } //539E50.
	void DirectShowAudio::Update() //539E70.
	{
		if (!initialized) return;
		RefreshVolume(0);
		double duration, current;
		position->get_Duration(&duration);
		position->get_CurrentPosition(&current);
		if (current >= duration) Stop();
		if (control)
		{
			OAFilterState currentState;
			if (!control->GetState(0, &currentState)) state = currentState;
		}
		else state = 0;
	}

	DirectMusicAudio::DirectMusicAudio() { loader = NULL; performance = NULL; segments.clear(); } //53A1E0.
	DirectMusicAudio::~DirectMusicAudio() { segments.clear(); } //53A180, then real list destruction.
	HRESULT DirectMusicAudio::Initialize(HWND window) //539EF0.
	{
		HRESULT result = CoInitialize(NULL);
		if (result < 0) return result;
		result = CoCreateInstance(CLSID_DirectMusicLoader, NULL, 3,
			IID_IDirectMusicLoader8, reinterpret_cast<void**>(&loader));
		if (result < 0) return result;
		result = CoCreateInstance(CLSID_DirectMusicPerformance, NULL, 3,
			IID_IDirectMusicPerformance8, reinterpret_cast<void**>(&performance));
		if (result < 0) return result;
		performance->InitAudio(NULL, NULL, window, 1, 128, 63, NULL);
		return S_OK; //The original ignores InitAudio failure.
	}
	void DirectMusicAudio::Close() //539F60.
	{
		performance->Stop(NULL, NULL, 0, 0);
		performance->CloseDown();
		if (loader) { loader->Release(); loader = NULL; }
		if (performance) { performance->Release(); performance = NULL; }
		CoUninitialize();
	}
	void DirectMusicAudio::Update() //53A150.
	{
		for (std::list<DirectMusicSegment*>::iterator i = segments.begin(); i != segments.end(); ++i)
			(*i)->Update();
	}
	LONG DirectMusicSegment::CalculateVolume() //539FB0.
	{
		const DWORD clock=AudioBackend::g_System->now;
		const DWORD previous=flags;
		if (previous & 0x80)
		{
			if (clock >= fadeStart)
			{
				if (clock >= fadeDuration + fadeStart)
				{
					mixVolume=LONG(fadeTo);
					flags=previous & ~DWORD(0x80);
				}
				else mixVolume=LONG(fadeFrom+(clock-fadeStart)*(fadeTo-fadeFrom)/fadeDuration);
			}
		}
		else if (previous & 0x100)
		{
			if (clock >= fadeStart)
			{
				if (clock >= fadeDuration + fadeStart)
				{
					mixVolume=LONG(fadeTo);
					flags=previous & ~DWORD(0x100);
				}
				else mixVolume=LONG(fadeFrom-(clock-fadeStart)*(fadeFrom-fadeTo)/fadeDuration);
			}
		}
		if (mixVolume < 0) mixVolume=0;
		if (mixVolume > 100) mixVolume=100;
		return LONG(DWORD(AudioBackend::g_System->masterVolume) * DWORD(baseVolume)
			* DWORD(mixVolume)) / 10000;
	}
	int DirectMusicSegment::SetVolume(LONG volume, BYTE force) //53A080.
	{
		if (!AudioBackend::g_System->initialized) return 1;
		baseVolume = volume;
		const LONG calculated = CalculateVolume();
		const bool unchanged = calculated == lastVolume;
		effectiveVolume = calculated;
		if (!unchanged || force == 1)
		{
			const LONG attenuation = AudioBackend::g_System->LookupVolume(calculated);
			path->SetVolume(attenuation, 0);
			lastVolume = effectiveVolume;
		}
		return 0;
	}
	int DirectMusicSegment::RefreshVolume(BYTE force) { return SetVolume(baseVolume, force); } //53A0E0.
	HRESULT DirectMusicSegment::Update() //53A100.
	{
		HRESULT result = RefreshVolume(0);
		if (flags & 4)
		{
			result = performance->IsPlaying(segment, NULL);
			const DWORD currentFlags = flags;
			if (currentFlags & 0x20)
			{
				if (!result) flags = currentFlags & ~DWORD(0x20);
			}
			else if (result) flags = (currentFlags & 0xFFFFFFB3) | 0x48;
		}
		return result;
	}
}
