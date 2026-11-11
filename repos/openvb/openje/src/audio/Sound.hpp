#pragma once

#include <windows.h>
#include <mmsystem.h>
#include <dsound.h>
#include <string>

namespace JE
{
	class SoundSystem;
	namespace Resources
	{
		class ResourceSystem;
		struct ResourceFileHandle;
	}

	struct SoundData
	{
		SoundData(); // 4C68B0.
		~SoundData(); // 4C68D0.
		BYTE Silence() const; //4C68F0, null format yields0.
		DWORD value00;
		DWORD bufferBytes;
		DWORD totalDataBytes;
		WAVEFORMATEX* format;
		DWORD value10;
		DWORD dataOffset;
		DWORD bytesRead;
	};

	class CSound
	{
	public:
		CSound(); // 4C8100.
		~CSound(); // 4C7CF0, original destructor is nonvirtual.
		virtual int Load(const char* name, DWORD loadFlags); // 4C82E0.
		virtual void Update(); //4C8070, callers discard the unconstrained native EAX.
		virtual BYTE ReadSamples(void* output, DWORD count,
			Resources::ResourceFileHandle* handle); // 4C70B0.
		SoundData* ReadWave(Resources::ResourceFileHandle* handle); // 4C6E50.
		SoundData* ReadAcm(Resources::ResourceFileHandle* handle); //4C7450, authentic adecode C API.
		SoundData* ReadHeader(Resources::ResourceFileHandle* handle); // 4C7670.
		void ResetData(); // 4C7000.
		DWORD GetBufferSize(); // 4C6E20.
		BYTE SeekSamples(Resources::ResourceFileHandle* handle, LONG offset, int origin); // 4C7600.
		int CreatePlaybackBuffer(SoundSystem& system,
			const DSBUFFERDESC& description, const BYTE* samples, DWORD count); // 4C82E0 suffix.
		int Play(DWORD requestedFlags, DWORD fadeIn, DWORD delay); // 4C7EB0.
		int Prepare(); // 4C6910, rewind a nonstreaming voice.
		int Stop(DWORD fadeOut); // 4C79E0.
		int SetVolume(int volume, BYTE force = 0); // 4C7DD0.
		DWORD GetRequestedVolume() const; //4C6B60, raw requested word at+80.
		int GetVolume(); // 4C7A80, live fade/group/master combination.
		int GetPitch(); //4C6B70 signed32 quotient of low32 product.
		int SetPitch(int pitch, BYTE force = 0); // 4C72D0.
		DWORD GetPan(); //4C6BA0 raw requested word.
		int RefreshPan(BYTE force); //4C7430, actual requested-word forwarding.
		int RefreshPitch(BYTE force); //4C7370.
		int RefreshVolume(BYTE force); //4C7E60.
		int SetPan(int pan, BYTE force = 0); // 4C7390.
		int SetBaseVolume(int volume); // 4C70E0.
		void SetVolumeOffset(int volume); // 4C7110.
		int SetBasePitch(int pitch); // 4C7120.
		void SetPitchOffset(int pitch); // 4C7150.
		void SetMixFadeInDuration(DWORD duration); //4C7160, actual one-word duration API.
		void SetMixFadeOutDuration(DWORD duration); //4C7170.
		int SetMixVolume(int volume); // 4C6B20.
		BOOL CanDelete() const; // 4C7060.
		HRESULT FillBuffer(IDirectSoundBuffer* target); // 4C76A0.

		Resources::ResourceSystem* resourceOwner; // Original +4, borrowed.
		std::string resourceName; // +8.
		Resources::ResourceFileHandle* stream; // +24.
		DWORD formatKind; // +28.
		DWORD loadFlags; // +2C.
		DWORD playRequestFlags;
		DWORD flags; // +34, original software status bits, not loadFlags.
		SoundData* soundData; // +38.
		IDirectSoundBuffer* buffer; // +3C.
		IDirectSoundBuffer8* buffer8; // +40.
		IDirectSound3DBuffer* buffer3D; // +44.
		IUnknown* effect; // +48, IDirectSoundFXWavesReverb when loadFlags bit8 is set.
		DWORD reverbInputGainBits, reverbMixBits;
		float reverbTime, reverbHighFrequencyRatio;
		DWORD lastFilledStreamBlock, streamWrapCount, previousPlayCursor;
		void* decoder; //+68, opaque owned decoder at the original adecode C API boundary.
		void* decoderContext; // +6C.
		DWORD id; // +70.
		DWORD bitsPerSample, sampleRate, channels; // +74..7C.
		DWORD requestedVolume;
		LONG effectiveVolume;
		DWORD appliedVolume, fadeVolume, mixVolume;
		DWORD fadeStartTime, fadeDuration, fadeStartVolume, fadeTargetVolume, playFadeInDuration, playStartTime, scheduledPlayTime;
		DWORD requestedMixVolume, previousMixVolumeRequest, mixFadeStartTime, mixFadeDuration, mixFadeStartVolume, mixFadeTargetVolume;
		DWORD requestedPitch;
		LONG effectivePitch;
		DWORD appliedPitch, requestedPan;
		LONG effectivePan;
		DWORD appliedPan, baseVolume, basePitch, volumeOffset, pitchOffset, mixFadeInDuration, mixFadeOutDuration;

	private:
		int ClassifySoundFormat(const char* name); //4C6C30, native callers supply this.
		DWORD SoundBufferFlags(DWORD flags); //4C6CE0, unused receiver/RET4.
		int ScheduleStop(DWORD fadeOut); //4C72A0, queue one fade-out before Stop.
		HRESULT RestoreBuffer(IDirectSoundBuffer* target, BOOL* restored); //4C6BB0, SDK policy.
		HRESULT ConfigureBuffer(IDirectSoundBuffer* target, DWORD requestedFlags); // 4C6D20.
		int CanFillStreamBlock(int playingBlock, int nextBlock) const; //4C7080, receiver unused.
		void UpdateStreamBuffers(); // 4C7810.
		int ResetMixVolume(int volume); // 4C6B30.
		int FadeIn(DWORD duration, DWORD delay, int from, int to); // 4C6960.
		int FadeOut(DWORD duration, DWORD delay, int from, int to); // 4C69D0.
		int FadeMixIn(DWORD duration, DWORD delay, int from, int to); // 4C6A40.
		int FadeMixOut(DWORD duration, DWORD delay, int from, int to); // 4C6AB0.
		void RewindSamples(); // 4C75B0.
		CSound(const CSound&);
		CSound& operator=(const CSound&);
	};

}
