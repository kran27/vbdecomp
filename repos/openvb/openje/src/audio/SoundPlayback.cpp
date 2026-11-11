#include "Sound.hpp"
#include "SoundSystem.hpp"
#include "SoundInstanceServices.hpp"
#include "resources/ResourceSystem.hpp"
#include "model/CommonException.hpp"
#include "diagnostics/DebugOutput.hpp"
#include <cstring>
#include <cstddef>

namespace JE
{
	BYTE SoundData::Silence() const //4C68F0.
	{
		if (format && format->wBitsPerSample == 8) return 0x80;
		return 0;
	}

	int CSound::CanFillStreamBlock(int playingBlock, int nextBlock) const //4C7080, support0.
	{
		if (playingBlock == nextBlock) return 0;
		return static_cast<LONG>(DWORD(playingBlock) + DWORD(1)) % 8 != nextBlock;
	}

	// Original SDK policy, including two Restore calls on each retry pass.
	HRESULT CSound::RestoreBuffer(IDirectSoundBuffer* target, BOOL* restored)
	{
		if (!target) return CO_E_NOTINITIALIZED;
		if (restored) *restored = FALSE;
		DWORD status;
		HRESULT result = target->GetStatus(&status);
		if (result < 0) return result;
		if (!(status & DSBSTATUS_BUFFERLOST)) return S_FALSE;
		do
		{
			if (target->Restore() == DSERR_BUFFERLOST) Sleep(10);
		}
		while (target->Restore() != DS_OK);
		if (restored) *restored = TRUE;
		return S_OK;
	}

	namespace
	{
		typedef char SoundDataSize[(sizeof(SoundData) == 28) ? 1 : -1];
		typedef char SoundDataFormat[(offsetof(SoundData, format) == 12) ? 1 : -1];
#if defined(_MSC_VER) && _MSC_VER == 1300 && defined(_M_IX86)
		typedef char SoundSize[(sizeof(CSound) == 248) ? 1 : -1];
#endif
		// The native SetAllParameters input borrows this four-word member block.
		typedef char ReverbParameterExtent[sizeof(DSFXWavesReverb) == 16 ? 1 : -1];
#if defined(_MSC_VER) && _MSC_VER == 1300 && defined(_M_IX86)
		typedef char ReverbInputOffset[offsetof(CSound, reverbInputGainBits) == 0x4C ? 1 : -1];
		typedef char ReverbMixOffset[offsetof(CSound, reverbMixBits) == 0x4C + offsetof(DSFXWavesReverb, fReverbMix) ? 1 : -1];
		typedef char ReverbTimeOffset[offsetof(CSound, reverbTime) == 0x4C + offsetof(DSFXWavesReverb, fReverbTime) ? 1 : -1];
		typedef char ReverbRatioOffset[offsetof(CSound, reverbHighFrequencyRatio) == 0x4C + offsetof(DSFXWavesReverb, fHighFreqRTRatio) ? 1 : -1];
		typedef char ReverbBlockEnd[offsetof(CSound, lastFilledStreamBlock) == 0x4C + sizeof(DSFXWavesReverb) ? 1 : -1];
#endif

	}

	int CSound::CreatePlaybackBuffer(SoundSystem& system,
		const DSBUFFERDESC& description, const BYTE* samples, DWORD count)
	{
		const DWORD configurationFlags = loadFlags;
		IDirectSoundBuffer* created = NULL;
		if (system.device->CreateSoundBuffer(&description, &created, NULL) < 0)
			return 4;
		if (soundData->format)
		{
			bitsPerSample = soundData->format->wBitsPerSample;
			sampleRate = soundData->format->nSamplesPerSec;
			channels = soundData->format->nChannels;
		}
		ConfigureBuffer(created, configurationFlags);
		if (samples)
		{
			// Explicit provider seam for already-read sample storage. The real
			// Load path passes NULL and uses the original virtual reader below.
			void* destination = NULL;
			DWORD bytes = 0;
			if (RestoreBuffer(created, NULL) >= 0 && created->Lock(0,
				description.dwBufferBytes, &destination, &bytes, NULL, NULL, 0) >= 0)
			{
				std::memcpy(destination, samples, count);
				created->Unlock(destination, bytes, NULL, 0);
				lastFilledStreamBlock = 7;
			}
		}
		else
			FillBuffer(created);
		created->SetCurrentPosition(0);
		previousPlayCursor = streamWrapCount = 0;
		buffer = created;
		return 0;
	}

	HRESULT CSound::ConfigureBuffer(IDirectSoundBuffer* target, DWORD requestedFlags)
	{
		const BYTE configurationFlags = static_cast<BYTE>(requestedFlags);
		HRESULT result;
		if (configurationFlags & 0x40)
		{
			result = target->QueryInterface(IID_IDirectSoundBuffer8,
				reinterpret_cast<void**>(&buffer8));
			if (result < 0) return result;
			result = buffer8->QueryInterface(IID_IDirectSound3DBuffer,
				reinterpret_cast<void**>(&buffer3D));
			if (result < 0) return result;
		}
		if (!(configurationFlags & 8)) return S_OK;
		result = target->QueryInterface(IID_IDirectSoundBuffer8,
			reinterpret_cast<void**>(&buffer8));
		if (result < 0) return result;
		DSEFFECTDESC description;
		std::memset(&description, 0, sizeof(description));
		description.dwSize = sizeof(description);
		description.dwFlags = 0;
		description.guidDSFXClass = GUID_DSFX_WAVES_REVERB;
		result = buffer8->SetFX(1, &description, &requestedFlags);
		if (result < 0) return result;
		result = buffer8->GetObjectInPath(description.guidDSFXClass, 0,
			IID_IDirectSoundFXWavesReverb, reinterpret_cast<void**>(&effect));
		if (result < 0) return result;
		static_cast<IDirectSoundFXWavesReverb*>(effect)->SetAllParameters(
			reinterpret_cast<const DSFXWavesReverb*>(&reverbInputGainBits));
		return S_OK;
	}

	HRESULT CSound::FillBuffer(IDirectSoundBuffer* target)
	{
		void* destination = NULL;
		DWORD bytes = 0;
		if (!target) return CO_E_NOTINITIALIZED;
		HRESULT result = RestoreBuffer(target, NULL);
		if (result < 0) return result;
		result = target->Lock(0, soundData->bufferBytes, &destination, &bytes, NULL, NULL, 0);
		if (result < 0) return result;
		if (stream)
		{
			if (soundData->bytesRead)
				SeekSamples(stream, soundData->dataOffset, 0);
			ReadSamples(destination, soundData->bufferBytes, stream);
			soundData->bytesRead = soundData->bufferBytes;
		}
		else
		{
			stream = resourceOwner->OpenStream(resourceName.c_str(), 1);
			RewindSamples();
			if (stream)
			{
				if (formatKind == 2) SeekSamples(stream, soundData->dataOffset, 0);
				ReadSamples(destination, soundData->bufferBytes, stream);
				resourceOwner->Release(stream, 0);
				ResetData();
				stream = NULL;
				// Original4C77A5 reloads SoundData after ResetData. That
				// invalidated PCM recovery branch remains an original quirk.
				soundData->bytesRead = soundData->bufferBytes;
			}
			else
				std::memset(destination, soundData->Silence(), soundData->bufferBytes);
		}
		target->Unlock(destination, bytes, NULL, 0);
		lastFilledStreamBlock = 7;
		return S_OK;
	}

	int CSound::SetBaseVolume(int volume)
	{
		if (volume < 0) volume = 0;
		if (volume > 100) volume = 100;
		baseVolume = volume;
		return volume;
	}
	void CSound::SetVolumeOffset(int volume) { volumeOffset = volume; }
	int CSound::SetBasePitch(int pitch)
	{
		if (pitch < 50) pitch = 50;
		if (pitch > 200) pitch = 200;
		basePitch = pitch;
		return pitch;
	}
	void CSound::SetPitchOffset(int pitch) { pitchOffset = pitch; }

	void CSound::SetMixFadeInDuration(DWORD duration) { mixFadeInDuration = duration; }
	void CSound::SetMixFadeOutDuration(DWORD duration) { mixFadeOutDuration = duration; }
	int CSound::SetMixVolume(int volume) { requestedMixVolume = volume; return 0; }
	int CSound::ResetMixVolume(int volume)
	{
		requestedMixVolume = volume;
		mixVolume = volume;
		previousMixVolumeRequest = volume;
		flags &= ~0xC00;
		return 0;
	}
	BOOL CSound::CanDelete() const
	{
		if ((playRequestFlags&2) && !(playRequestFlags&1))
		{
			if (flags&0x40) return 1;
			if (flags&8) return 1;
		}
		return 0;
	}
	int CSound::Prepare()
	{
		IDirectSoundBuffer* target = buffer;
		if (flags & 1) return 8;
		if (!AudioBackend::g_System->initialized) return 1;
		if (!target) return 4;
		target->SetCurrentPosition(0);
		previousPlayCursor = 0;
		streamWrapCount = 0;
		return 0;
	}

	int CSound::FadeIn(DWORD duration, DWORD delay, int from, int to)
	{
		flags = (flags & ~0x100) | 0x80;
		fadeStartTime = AudioBackend::g_System->now + delay;
		fadeDuration = duration;
		if (from == -1)
		{
			fadeStartVolume = fadeVolume;
			fadeTargetVolume = to;
			return 0;
		}
		fadeStartVolume = from;
		fadeTargetVolume = to;
		return 0;
	}
	int CSound::FadeOut(DWORD duration, DWORD delay, int from, int to)
	{
		flags = (flags & ~0x80) | 0x100;
		fadeStartTime = AudioBackend::g_System->now + delay;
		fadeDuration = duration;
		if (from == -1)
		{
			fadeStartVolume = fadeVolume;
			fadeTargetVolume = to;
			return 0;
		}
		fadeStartVolume = from;
		fadeTargetVolume = to;
		return 0;
	}
	int CSound::FadeMixIn(DWORD duration, DWORD delay, int from, int to)
	{
		flags = (flags & ~0x800) | 0x400;
		mixFadeStartTime = AudioBackend::g_System->now + delay;
		mixFadeDuration = duration;
		if (from == -1)
		{
			mixFadeStartVolume = mixVolume;
			mixFadeTargetVolume = to;
			return 0;
		}
		mixFadeStartVolume = from;
		mixFadeTargetVolume = to;
		return 0;
	}
	int CSound::FadeMixOut(DWORD duration, DWORD delay, int from, int to)
	{
		flags = (flags & ~0x400) | 0x800;
		mixFadeStartTime = AudioBackend::g_System->now + delay;
		mixFadeDuration = duration;
		if (from == -1)
		{
			mixFadeStartVolume = mixVolume;
			mixFadeTargetVolume = to;
			return 0;
		}
		mixFadeStartVolume = from;
		mixFadeTargetVolume = to;
		return 0;
	}

	int CSound::GetVolume()
	{
		DWORD now = AudioBackend::g_System->now;
		if (flags & 0x80)
		{
			if (now >= fadeStartTime)
			{
				if (now >= fadeStartTime + fadeDuration)
				{
					fadeVolume = fadeTargetVolume;
					flags &= ~0x80;
				}
				else fadeVolume = fadeStartVolume + (now - fadeStartTime) * (fadeTargetVolume - fadeStartVolume) / fadeDuration;
			}
		}
		else if (flags & 0x100)
		{
			if (now >= fadeStartTime)
			{
				if (now >= fadeStartTime + fadeDuration)
				{
					fadeVolume = fadeTargetVolume;
					flags &= ~0x100;
					if (flags & 0x200) { flags &= ~0x200; Stop(0); }
				}
				else fadeVolume = fadeStartVolume - (now - fadeStartTime) * (fadeStartVolume - fadeTargetVolume) / fadeDuration;
			}
		}
		if (static_cast<LONG>(fadeVolume) < 0) fadeVolume = 0;
		if (static_cast<LONG>(fadeVolume) > 100) fadeVolume = 100;
		if (previousMixVolumeRequest != requestedMixVolume)
		{
			if (static_cast<LONG>(mixVolume) > static_cast<LONG>(requestedMixVolume))
				FadeMixOut(mixFadeOutDuration, 0, -1, requestedMixVolume);
			else
				FadeMixIn(mixFadeInDuration, 0, -1, requestedMixVolume);
			previousMixVolumeRequest = requestedMixVolume;
		}
		if (flags & 0x400)
		{
			if (now >= mixFadeStartTime)
			{
				if (now >= mixFadeStartTime + mixFadeDuration)
				{
					mixVolume = mixFadeTargetVolume;
					flags &= ~0x400;
				}
				else mixVolume = mixFadeStartVolume + (now - mixFadeStartTime) * (mixFadeTargetVolume - mixFadeStartVolume) / mixFadeDuration;
			}
		}
		else if (flags & 0x800)
		{
			if (now >= mixFadeStartTime)
			{
				if (now >= mixFadeStartTime + mixFadeDuration)
				{
					mixVolume = mixFadeTargetVolume;
					flags &= ~0x800;
				}
				else mixVolume = mixFadeStartVolume - (now - mixFadeStartTime) * (mixFadeStartVolume - mixFadeTargetVolume) / mixFadeDuration;
			}
		}
		if (static_cast<LONG>(mixVolume) < 0) mixVolume = 0;
		if (static_cast<LONG>(mixVolume) > 100) mixVolume = 100;
		LONG volume = static_cast<LONG>(baseVolume * requestedVolume) / 100 + static_cast<LONG>(volumeOffset);
		return static_cast<LONG>(volume * AudioBackend::g_System->masterVolume * mixVolume * fadeVolume) / 1000000;
	}

	int CSound::SetVolume(int volume, BYTE force)
	{
		IDirectSoundBuffer* target = buffer;
		if (!AudioBackend::g_System->initialized) return 1;
		requestedVolume = volume;
		const int volumeValue = GetVolume();
		effectiveVolume = volumeValue;
		if (!target) return 4;
		if (volumeValue != static_cast<LONG>(appliedVolume) || force == 1)
		{
			const LONG directSoundVolume = AudioBackend::g_System->LookupVolume(volumeValue);
			if (target->SetVolume(directSoundVolume))
				LogDebugString("SetVolume() - error setting DirectSound volume.\n");
			appliedVolume = effectiveVolume;
		}
		return 0;
	}
	int CSound::RefreshVolume(BYTE force) { return SetVolume(requestedVolume, force); } //4C7E60.

	int CSound::GetPitch()
	{
		return static_cast<LONG>(basePitch * requestedPitch) / 100 + pitchOffset;
	}
	int CSound::SetPitch(int pitch, BYTE force)
	{
		IDirectSoundBuffer* target = buffer;
		if (!AudioBackend::g_System->initialized) return 1;
		if (!target) return 4;
		requestedPitch = pitch;
		const LONG pitchValue = GetPitch();
		effectivePitch = pitchValue;
		if (pitchValue != static_cast<LONG>(appliedPitch) || force == 1)
		{
			if (pitchValue == 100)
			{
				target->SetFrequency(DSBFREQUENCY_ORIGINAL);
				appliedPitch = effectivePitch;
				return 0;
			}
			target->SetFrequency(pitchValue * sampleRate / 100);
			appliedPitch = effectivePitch;
		}
		return 0;
	}
	int CSound::RefreshPitch(BYTE force) { return SetPitch(requestedPitch, force); } //4C7370.

	DWORD CSound::GetPan()
	{
		return requestedPan;
	}
	int CSound::SetPan(int pan, BYTE force)
	{
		IDirectSoundBuffer* target = buffer;
		if (!AudioBackend::g_System->initialized) return 1;
		if (!target) return 4;
		requestedPan = pan;
		const LONG panValue = static_cast<LONG>(GetPan());
		effectivePan = panValue;
		if (panValue != static_cast<LONG>(appliedPan) || force == 1)
		{
			LONG amount;
			if (panValue < 0)
			{
				amount = panValue * 100;
				if (amount < -10000) amount = -10000;
			}
			else if (panValue > 0)
			{
				amount = panValue * 100;
				if (amount > 10000) amount = 10000;
			}
			else amount = 0;
			target->SetPan(amount);
			appliedPan = effectivePan;
		}
		return 0;
	}

	int CSound::RefreshPan(BYTE force) { return SetPan(requestedPan, force); } //4C7430.

	int CSound::Play(DWORD requestedFlags, DWORD fadeIn, DWORD delay)
	{
		DWORD playbackFlags = 0;
		IDirectSoundBuffer* target = buffer;
		if (!AudioBackend::g_System->initialized) return 1;
		if (!target) return 4;
		if (!(flags & 2)) return 2;
		if (flags & 4) return 0;
		BOOL restored;
		if (RestoreBuffer(target, &restored) < 0) return 4;
		if (restored)
		{
			if (FillBuffer(target) < 0) return 4;
			target->SetCurrentPosition(0);
			previousPlayCursor = 0;
			streamWrapCount = 0;
		}
		playRequestFlags = requestedFlags;
		playFadeInDuration = fadeIn;
		if (static_cast<LONG>(delay) > 0)
		{
			const DWORD startTime = AudioBackend::g_System->now + delay;
			flags = (flags & 0xFFFFEF83) | 0x1020;
			scheduledPlayTime = startTime;
			return 0;
		}
		if (requestedFlags & 1) playbackFlags = DSBPLAY_LOOPING;
		if (flags & 1)
		{
			playbackFlags |= DSBPLAY_LOOPING;
			if (!stream)
			{
				stream = resourceOwner->OpenStream(resourceName.c_str(), 1);
				RewindSamples();
			}
		}
		const DWORD initialMixVolume = requestedMixVolume;
		playStartTime = AudioBackend::g_System->now;
		ResetMixVolume(initialMixVolume);
		const DWORD fadeFlags = flags & 0xFFFFFE7F;
		fadeVolume = 100;
		fadeStartVolume = 0;
		fadeTargetVolume = 0;
		fadeStartTime = 0;
		fadeDuration = 0;
		flags = fadeFlags;
		if (static_cast<LONG>(fadeIn) > 0) FadeIn(fadeIn, 0, 0, 100);
		RefreshPan(1);
		RefreshPitch(1);
		RefreshVolume(1);
		target->Play(0, 0, playbackFlags);
		flags = (flags & 0xFFFFEF83) | 0x24;
		return 0;
	}

	int CSound::Stop(DWORD fadeOut)
	{
		IDirectSoundBuffer* target = buffer;
		if (!AudioBackend::g_System->initialized) return 1;
		if (!target) return 4;
		const DWORD stopFlags = flags;
		if (stopFlags & 8) return 0;
		if (static_cast<LONG>(fadeOut) > 0 && !(stopFlags & 0x10))
			return ScheduleStop(fadeOut);
		flags = (stopFlags & 0xFFFFEDE3) | 8;
		target->Stop();
		if ((flags & 1) && !(playRequestFlags & 2))
		{
			RewindSamples();
			FillBuffer(target);
		}
		target->SetCurrentPosition(0);
		previousPlayCursor = 0;
		streamWrapCount = 0;
		return 0;
	}

	void CSound::UpdateStreamBuffers()
	{
		IDirectSoundBuffer* target = buffer;
		if (!target)
		{
			LogDebugString("UpdateStreamBuffers() - sound buffer is null.\n");
			return;
		}
		if (flags & 8) return;
		DWORD playCursor;
		DWORD writeCursor;
		target->GetCurrentPosition(&playCursor, &writeCursor);
		if (playCursor < previousPlayCursor) ++streamWrapCount;
		previousPlayCursor = playCursor;
		if (!(playRequestFlags & 1) && playCursor + (streamWrapCount << 18) > soundData->totalDataBytes)
		{
			flags = (flags & 0xFFFFFDFB) | 0x48;
			target->Stop();
			if (!(playRequestFlags & 2)) FillBuffer(target);
			target->SetCurrentPosition(0);
			previousPlayCursor = 0;
			streamWrapCount = 0;
			return;
		}
		int nextBlock = static_cast<LONG>(lastFilledStreamBlock + DWORD(1)) % 8;
		int playingBlock = playCursor >> 15;
		//4C7080 excludes both the playing block and the following block.
		if (!CanFillStreamBlock(playingBlock, nextBlock)) return;
		void* destination = NULL;
		DWORD bytes = 0;
		if (target->Lock(nextBlock << 15, 0x8000, &destination, &bytes, NULL, NULL, 0) < 0)
			return;
		if (stream)
		{
			if (soundData->bytesRead + 0x8000 > soundData->totalDataBytes)
			{
				BYTE* remainingOutput = static_cast<BYTE*>(destination);
				DWORD remainingBytes = 0x8000;
				LONG available = soundData->totalDataBytes - soundData->bytesRead;
				if (available > 0)
				{
					ReadSamples(remainingOutput, available, stream);
					remainingOutput += available;
					remainingBytes -= available;
				}
				if (playRequestFlags & 1)
				{
					SeekSamples(stream, soundData->dataOffset, 0);
					ReadSamples(remainingOutput, remainingBytes, stream);
					soundData->bytesRead = remainingBytes;
				}
				else
				{
					std::memset(remainingOutput, soundData->Silence(), remainingBytes);
					soundData->bytesRead = soundData->totalDataBytes;
				}
			}
			else
			{
				ReadSamples(destination, 0x8000, stream);
				soundData->bytesRead += 0x8000;
			}
			++lastFilledStreamBlock;
		}
		target->Unlock(destination, bytes, NULL, 0);
	}

	void CSound::Update()
	{
		if (flags & 0x1000)
		{
			if (AudioBackend::g_System->now < scheduledPlayTime) return;
			Play(playRequestFlags, playFadeInDuration, 0);
		}
		RefreshPan(0);
		RefreshPitch(0);
		RefreshVolume(0);
		if (flags & 1)
			UpdateStreamBuffers();
		else if (flags & 4)
		{
			DWORD status = 0;
			buffer->GetStatus(&status);
			if (!(status & DSBSTATUS_PLAYING)) flags = (flags & 0xFFFFFDFB) | 0x48;
		}
		return;
	}
}
