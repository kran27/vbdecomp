#include "SoundSystem.hpp"
#include "Sound.hpp"
#include <cstring>

namespace JE
{
	void SoundSystem::CleanupDevice()
	{
		//The actual device-cleanup prefix52A860 repeats these two cleanups.
		CleanupCD();
		CleanupDirectShow();
		for (;;)
		{
			std::list<CSound*>::iterator i=sounds.begin();
			CSound* sound;
			while (i!=sounds.end())
			{
				sound=*i;
				if (sound->playRequestFlags&2) break;
				++i;
			}
			if (i==sounds.end()) break;
			sounds.erase(i);
			delete sound;
		}
		if (scratchProperties) { scratchProperties->Release(); scratchProperties=NULL; }
		if (scratch) { scratch->Release(); scratch=NULL; }
		if (listener) { listener->Release(); listener=NULL; }
		if (primary) { primary->Release(); primary=NULL; }
		if (device) { device->Release(); device=NULL; }
		initialized=0;
		resources=NULL;
	}
	int SoundSystem::InitializeDevice(HWND window, DWORD cooperativeLevel,
		DWORD channels, DWORD sampleRate, DWORD bitsPerSample)
	{
		IDirectSoundBuffer* created = NULL;
		if (device)
		{
			device->Release();
			device = NULL;
		}
		if (SoundDeviceServices::Create(NULL, &device, NULL) < 0) return 3;
		if (device->SetCooperativeLevel(window, cooperativeLevel) < 0) return 3;

		DWORD primaryFlags = DSBCAPS_PRIMARYBUFFER;
		if (enable3D) primaryFlags |= DSBCAPS_CTRL3D;
		DSBUFFERDESC description;
		std::memset(&description, 0, sizeof(description));
		description.dwSize = sizeof(description);
		description.dwFlags = primaryFlags;
		description.dwBufferBytes = 0;
		description.lpwfxFormat = NULL;
		if (device->CreateSoundBuffer(&description, &created, NULL) < 0) return 4;

		WAVEFORMATEX format;
		std::memset(&format, 0, sizeof(format));
		format.wFormatTag = WAVE_FORMAT_PCM;
		format.nChannels = static_cast<WORD>(channels);
		format.nSamplesPerSec = sampleRate;
		format.nBlockAlign = static_cast<WORD>((static_cast<WORD>(bitsPerSample) >> 3) * channels);
		format.nAvgBytesPerSec = sampleRate * format.nBlockAlign;
		format.wBitsPerSample = static_cast<WORD>(bitsPerSample);
		if (created->SetFormat(&format) < 0) return 3;

		const JE::Net::UInt32 queryListener = enable3D;
		primary = created;
		if (queryListener && created->QueryInterface(IID_IDirectSound3DListener,
			reinterpret_cast<void**>(&listener)) < 0) return 3;
		primary->Play(0, 0, DSBPLAY_LOOPING);
		const JE::Net::UInt32 prepareScratch = enable3D;
		this->bitsPerSample = bitsPerSample;
		this->sampleRate = sampleRate;
		this->channels = channels;
		if (prepareScratch) PrepareScratch3D();
		initialized = 1;
		return 0;
	}

	int SoundSystem::PrepareScratch3D()
	{
		if (!primary) return 1;
		WAVEFORMATEX format;
		std::memset(&format, 0, sizeof(format));
		format.wFormatTag = WAVE_FORMAT_PCM;
		format.nChannels = 1;
		format.nSamplesPerSec = sampleRate;
		format.wBitsPerSample = static_cast<WORD>(bitsPerSample);
		format.nBlockAlign = format.wBitsPerSample >> 3;
		format.nAvgBytesPerSec = format.nSamplesPerSec * format.nBlockAlign;
		DSBUFFERDESC description;
		std::memset(&description, 0, sizeof(description));
		description.dwSize = sizeof(description);
		description.dwFlags = DSBCAPS_STATIC | DSBCAPS_CTRL3D;
		description.dwBufferBytes = 64;
		description.lpwfxFormat = &format;
		if (device->CreateSoundBuffer(&description, &scratch, NULL) != 0) return 1;
		if (scratch->QueryInterface(IID_IKsPropertySet,
			reinterpret_cast<void**>(&scratchProperties)) != 0)
		{
			scratch->Release();
			scratch = NULL;
			return 1;
		}
		return 0;
	}

	namespace SoundDeviceServices
	{
		HRESULT Create(LPCGUID device, IDirectSound8** output, IUnknown* outer)
		{
			return ::DirectSoundCreate8(device, output, outer);
		}
	}
}
