#include "Sound.hpp"
#include "SoundSystem.hpp"
#include "SoundInstanceServices.hpp"
#include "resources/ResourceSystem.hpp"
#include "model/CommonException.hpp"
#include <cstring>

namespace JE
{
	int CSound::ScheduleStop(DWORD fadeOut)
	{
		if (!(flags & 0x200))
		{
			flags |= 0x200;
			FadeOut(fadeOut, 0, -1, 0);
		}
		return 0;
	}

	DWORD CSound::GetRequestedVolume() const { return requestedVolume; }

	namespace
	{
		DWORD nextSoundID = 1; // Original initialized DWORD at 6FB0A4.
	}

	SoundData::SoundData()
	{
		value00 = 0;
		bufferBytes = 0;
		totalDataBytes = 0;
		value10 = 0;
		format = NULL;
		dataOffset = 0;
		bytesRead = 0;
	}

	SoundData::~SoundData()
	{
		if (format)
		{
			delete format;
			format = NULL;
		}
	}

	CSound::CSound() : resourceName()
	{
		resourceOwner = AudioBackend::g_System->resources;
		resourceName = "";
		stream = NULL;
		formatKind = 0;
		loadFlags = 0;
		playRequestFlags = 0;
		flags = 0;
		soundData = NULL;
		buffer = NULL;
		effect = NULL;
		buffer8 = NULL;
		buffer3D = NULL;
		bitsPerSample = AudioBackend::g_System->bitsPerSample;
		sampleRate = AudioBackend::g_System->sampleRate;
		channels = AudioBackend::g_System->channels;
		std::memset(&reverbInputGainBits, 0, sizeof(DSFXWavesReverb));
		reverbInputGainBits = 0;
		id = nextSoundID++;
		reverbMixBits = 0;
		reverbTime = 1000.0f;
		reverbHighFrequencyRatio = 0.001f;
		baseVolume = 100;
		basePitch = 100;
		volumeOffset = 0;
		pitchOffset = 0;
		mixFadeInDuration = 500;
		mixFadeOutDuration = 500;
		fadeStartTime = 0;
		fadeDuration = 0;
		fadeStartVolume = 0;
		fadeTargetVolume = 0;
		playFadeInDuration = 0;
		playStartTime = 0;
		scheduledPlayTime = 0;
		requestedMixVolume = 100;
		previousMixVolumeRequest = 100;
		mixFadeStartTime = 0;
		mixFadeDuration = 0;
		mixFadeStartVolume = 0;
		mixFadeTargetVolume = 0;
		requestedVolume = 100;
		//Native GetVolume precedes the first writes of fadeVolume/mixVolume.
		effectiveVolume = GetVolume();
		appliedVolume = 100;
		fadeVolume = 100;
		mixVolume = 100;
		requestedPitch = 100;
		effectivePitch = GetPitch();
		appliedPitch = 100;
		requestedPan = 0;
		effectivePan = static_cast<LONG>(GetPan());
		appliedPan = 0;
		lastFilledStreamBlock = 0;
		previousPlayCursor = 0;
		streamWrapCount = 0;
		decoder = NULL;
		decoderContext = NULL;
		AudioBackend::g_System->RegisterSound(this);
	}
	CSound::~CSound()
	{
		if (stream)
		{
			resourceOwner->Release(stream, 0);
			ResetData();
			stream = NULL;
		}
		AudioBackend::g_System->UnregisterSound(this);
		SoundSystem* device = AudioBackend::g_System;
		if (device && device->initialized)
		{
			if (buffer3D) { buffer3D->Release(); buffer3D = NULL; }
			if (buffer8) { buffer8->Release(); buffer8 = NULL; }
			if (buffer) { buffer->Release(); buffer = NULL; }
		}
		// Original4C7CF0 does not release the optional effect interface.
		if (soundData)
		{
			delete soundData;
			soundData = NULL;
		}
	}

	int CSound::Load(const char* name, DWORD requestedFlags)
	{
		if (flags & 2)
			return 2;
		if (stream)
			return 7;
		Resources::ResourceFileHandle* const openedStream = resourceOwner->OpenStream(name, 1);
		stream = openedStream;
		if (!openedStream)
			return 6;
		formatKind = ClassifySoundFormat(name);
		soundData = ReadHeader(openedStream);
		if (!soundData)
			return 7; // Original retains the open stream on header failure.
		loadFlags = requestedFlags;
		resourceName.assign(name);
		soundData->bufferBytes = GetBufferSize();
		DSBUFFERDESC description;
		std::memset(&description, 0, sizeof(description));
		IDirectSoundBuffer* created = NULL;
		description.dwSize = sizeof(description);
		description.dwFlags = SoundBufferFlags(requestedFlags);
		description.dwBufferBytes = soundData->bufferBytes;
		description.lpwfxFormat = soundData->format;

		if (AudioBackend::g_System->device->CreateSoundBuffer(&description, &created, NULL) < 0)
		{
			if (soundData)
			{
				delete soundData;
				soundData = NULL;
			}
			resourceOwner->Release(stream, 0);
			ResetData();
			stream = NULL;
			return 4;
		}
		SoundData* const data = soundData;
		WAVEFORMATEX* const format = data->format;
		if (format)
		{
			bitsPerSample = format->wBitsPerSample;
			sampleRate = data->format->nSamplesPerSec;
			channels = data->format->nChannels;
		}
		ConfigureBuffer(created, requestedFlags);
		FillBuffer(created);
		flags |= 2;
		if (!(flags & 1))
		{
			resourceOwner->Release(stream, 0);
			ResetData();
			stream = NULL;
		}
		created->SetCurrentPosition(0);
		previousPlayCursor = 0;
		streamWrapCount = 0;
		buffer = created;
		return 0;
	}

	SoundData* CSound::ReadHeader(Resources::ResourceFileHandle* handle)
	{
		if (handle)
		{
			if (formatKind == 2)
				return ReadWave(handle);
			if (formatKind == 3)
				return ReadAcm(handle);
		}
		return NULL;
	}

	SoundData* CSound::ReadWave(Resources::ResourceFileHandle* handle)
	{
		if (!handle)
			return NULL;
		DWORD riff;
		DWORD ignoredLength;
		DWORD wave;
		resourceOwner->Read(&riff, 4, handle);
		if (riff != 0x46464952)
			return NULL;
		resourceOwner->Read(&ignoredLength, 4, handle);
		resourceOwner->Read(&wave, 4, handle);
		if (wave != 0x45564157)
			return NULL;
		SoundData* result = new SoundData;
		if (!result)
			return NULL;
		DWORD chunk;
		DWORD bytes;
		resourceOwner->Read(&chunk, 4, handle);
		resourceOwner->Read(&bytes, 4, handle);
		if (chunk != 0x20746D66 || bytes < 14)
		{
			delete result;
			return NULL;
		}
		result->format = new WAVEFORMATEX;
		std::memset(result->format, 0, sizeof(WAVEFORMATEX));
		// Original allocation is exactly18 bytes and the incoming fmt length
		// is not capped. Retain that policy; extended fmt input is unvalidated.
		resourceOwner->Read(result->format, bytes, handle);
		DWORD offset = bytes + 20;
		if (bytes & 1)
		{
			BYTE ignoredPad;
			resourceOwner->Read(&ignoredPad, 1, handle);
			++offset;
		}
		resourceOwner->Read(&chunk, 4, handle);
		resourceOwner->Read(&bytes, 4, handle);
		offset += 8;
		if (chunk != 0x61746164)
		{
			delete result;
			return NULL;
		}
		result->dataOffset = offset;
		result->totalDataBytes = bytes;
		return result;
	}

	DWORD CSound::GetBufferSize()
	{
		DWORD result = soundData->totalDataBytes;
		if (loadFlags & 4)
		{
			flags |= 1;
			return 0x40000;
		}
		if ((loadFlags & 2) && result > 0x40000)
		{
			flags |= 1;
			return 0x40000;
		}
		return result;
	}

	int CSound::ClassifySoundFormat(const char* name)
	{
		char extension[3];
		int count = 0;
		bool foundDot = false;
		if (!*name)
			return 0;
		for (;;)
		{
			if (foundDot)
			{
				extension[count++] = *name;
				if (count == 3)
					break;
			}
			if (*name == '.')
				foundDot = true;
			if (!*++name)
				return 0;
		}
		if ((extension[0] == 'W' || extension[0] == 'w')
			&& (extension[1] == 'A' || extension[1] == 'a')
			&& (extension[2] == 'V' || extension[2] == 'v'))
			return 2;
		if ((extension[0] == 'A' || extension[0] == 'a')
			&& (extension[1] == 'C' || extension[1] == 'c')
			&& (extension[2] == 'M' || extension[2] == 'm'))
			return 3;
		return (extension[0] == 'R' || extension[0] == 'r')
			&& (extension[1] == 'A' || extension[1] == 'a')
			&& (extension[2] == 'W' || extension[2] == 'w');
	}

	DWORD CSound::SoundBufferFlags(DWORD flags)
	{
		DWORD result = 128;
		if (!(flags & 0x10)) result = 192;
		if (!(flags & 0x20))
			result |= 0x20;
		if (flags & 8)
			result = (result & ~0x220) | 0x200;
		if (flags & 0x40)
			return 144;
		return result;
	}
}
