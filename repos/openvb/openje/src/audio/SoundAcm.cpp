#include "Sound.hpp"
#include "resources/ResourceSystem.hpp"
#include <cstddef>
#include <cstring>
#include <new>

// Genuine distributed adecode.obj exports. The decoder remains opaque.
extern "C"
{
	typedef int (__cdecl *AudioDecoderReadProc)(void*, void*, unsigned int);
	void* __cdecl Create_AudioDecoder(AudioDecoderReadProc read, void* context,
		unsigned int* channels, unsigned int* rate, unsigned int* samples);
	unsigned int __cdecl AudioDecoder_Read(void* decoder, void* output, unsigned int count);
	void __cdecl AudioDecoder_Close(void* decoder);
}

namespace
{
	struct AcmResourceContext
	{
		AcmResourceContext() : owner(NULL), handle(NULL), position(0), length(0) {}
		JE::Resources::ResourceSystem* owner;
		JE::Resources::ResourceFileHandle* handle;
		DWORD position, length;
	};
	typedef char ContextSize[(sizeof(AcmResourceContext) == 16) ? 1 : -1];
	typedef char ContextOwner[(offsetof(AcmResourceContext, owner) == 0) ? 1 : -1];
	typedef char ContextHandle[(offsetof(AcmResourceContext, handle) == 4) ? 1 : -1];
	typedef char ContextPosition[(offsetof(AcmResourceContext, position) == 8) ? 1 : -1];
	typedef char ContextLength[(offsetof(AcmResourceContext, length) == 12) ? 1 : -1];

	int ReadAcmResource(void* opaque, void* output, unsigned int count) //4C6FC0.
	{
		AcmResourceContext* context = static_cast<AcmResourceContext*>(opaque);
		const DWORD remaining = context->length - context->position;
		if (count > remaining) count = remaining;
		context->owner->Read(output, count, context->handle);
		context->position += count; //The resource reader's failure BYTE is ignored.
		return count;
	}
}

namespace JE
{
	SoundData* CSound::ReadAcm(Resources::ResourceFileHandle* handle) //4C7450.
	{
		if (!handle) return NULL;
		ResetData();
		void* block = ::operator new(sizeof(AcmResourceContext));
		decoderContext = block ? new (block) AcmResourceContext : NULL;
		AcmResourceContext* context = static_cast<AcmResourceContext*>(decoderContext);
		context->owner = resourceOwner;
		context->handle = handle;
		context->position = 0;
		const DWORD previous = resourceOwner->Tell(handle);
		resourceOwner->Seek(handle, 0, 2);
		const DWORD length = resourceOwner->Tell(handle);
		static_cast<AcmResourceContext*>(decoderContext)->length = length;
		resourceOwner->Seek(handle, previous, 0);

		unsigned int channelCount, rate, samples;
		decoder = Create_AudioDecoder(ReadAcmResource, decoderContext, &channelCount, &rate, &samples);
		if (!decoder) return NULL; //The context remains owned even on open failure.
		void* dataBlock = ::operator new(sizeof(SoundData));
		SoundData* data = dataBlock ? new (dataBlock) SoundData : NULL;
		if (!data) return NULL;
		data->format = new WAVEFORMATEX;
		std::memset(data->format, 0, sizeof(WAVEFORMATEX));
		data->format->wFormatTag = WAVE_FORMAT_PCM;
		data->format->nChannels = WORD(channelCount);
		data->format->nSamplesPerSec = rate;
		data->format->nBlockAlign = WORD(2 * channelCount);
		data->format->nAvgBytesPerSec = rate * data->format->nBlockAlign;
		data->format->wBitsPerSample = 16;
		data->format->cbSize = 0;
		data->dataOffset = 0;
		data->totalDataBytes = 2 * samples;
		return data;
	}

	BYTE CSound::ReadSamples(void* output, DWORD count, Resources::ResourceFileHandle* handle) //4C70B0.
	{
		BYTE result = 0;
		switch (formatKind)
		{
		case 2: return resourceOwner->Read(output, count, handle);
		case 3:
			AudioDecoder_Read(decoder, output, count);
			result = 1;
			break;
		}
		return result;
	}

	void CSound::ResetData() //4C7000.
	{
		if (decoder)
		{
			AudioDecoder_Close(decoder);
			decoder = NULL;
		}
		if (decoderContext)
		{
			delete static_cast<AcmResourceContext*>(decoderContext);
			decoderContext = NULL;
		}
		if (soundData)
		{
			delete soundData;
			soundData = NULL;
		}
	}

	void CSound::RewindSamples() //4C75B0.
	{
		if (formatKind == 3)
		{
			ResetData();
			if (stream)
			{
				resourceOwner->Seek(stream, 0, 0);
				soundData = ReadAcm(stream);
				soundData->bufferBytes = GetBufferSize();
			}
			else soundData = NULL;
		}
	}

	BYTE CSound::SeekSamples(Resources::ResourceFileHandle* handle, LONG offset, int origin) //4C7600.
	{
		BYTE result = 0;
		switch (formatKind)
		{
		case 2: return resourceOwner->Seek(handle, offset, origin);
		case 3:
			if (!origin)
			{
				RewindSamples();
				if (offset > 0)
				{
					BYTE* discarded = new BYTE[offset];
					AudioDecoder_Read(decoder, discarded, offset);
					delete[] discarded;
				}
			}
			break;
		}
		return result;
	}
}
