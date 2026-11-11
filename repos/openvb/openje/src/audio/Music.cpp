#include "Music.hpp"
#include "SoundSystem.hpp"
#include "resources/ResourceSystem.hpp"
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <new>

namespace JE
{
	void CMusic::MakeSegmentName(char* output, const char* name) //539080.
	{
		std::sprintf(output, "%s.wav", name);
	}

	BYTE CMusic::HasSegment(const char* name) //5390A0.
	{
		if (!name) return false;
		return ::_stricmp(name, "NULL") != 0;
	}

	CJumplist::CJumplist()
	{
		name[0] = 0;
		jumpCount = 0;
		segmentCount = 0;
		endingCount = 0;
		stingerCount = 0;
		jumps = NULL;
		segments = NULL;
		loopSegment = startSegment = -1;
	}

	CJumplist::~CJumplist()
	{
		if (jumps)
		{
			delete[] jumps;
			jumps = NULL;
		}
		if (segments)
		{
			delete[] segments;
			segments = NULL;
		}
	}

	int CJumplist::FindJump(const char* value) const
	{
		for (int index = 0; index < jumpCount; ++index)
			if (!::_stricmp(Jump(index), value))
				return index;
		return -1;
	}

	int CJumplist::FindSegment(const char* value) const
	{
		for (int index = 0; index < segmentCount; ++index)
			if (!::_stricmp(Segment(index, 0), value))
				return index;
		return -1;
	}

	void CJumplist::Parse(const BYTE* input, DWORD size)
	{
		data = input;
		dataSize = size;
		int rowOffset = 0;
		cursor = rowOffset;
		char token[32];
		ReadToken(token, 32);
		std::strncpy(name, token, 9);
		ReadToken(token, 32);
		segmentCount = std::atol(token);
		ReadToken(token, 32); ReadToken(token, 32); ReadToken(token, 32);
		endingCount = std::atol(token);
		ReadToken(token, 32); ReadToken(token, 32); ReadToken(token, 32);
		ReadToken(token, 32);
		stingerCount = std::atol(token);
		ReadToken(token, 32); ReadToken(token, 32); ReadToken(token, 32);
		ReadToken(token, 32);
		jumpCount = std::atol(token);
		if (jumpCount > 0)
		{
			jumps = new char[9 * jumpCount];
			for (int index = 0; index < jumpCount; ++index)
			{
				ReadToken(token, 32);
				std::strncpy(jumps + 9 * index, token, 9);
			}
		}
		ReadToken(token, 32); ReadToken(token, 32); ReadToken(token, 32);
		if (segmentCount > 0)
		{
			const int columns = jumpCount * 2 + 2;
			segments = new char[9 * segmentCount * columns];
			for (int row = 0; row < segmentCount; ++row)
			{
				ReadToken(token, 32);
				if (!::_stricmp(token, "start"))
				{
					startSegment = row;
					ReadToken(token, 32);
				}
				else if (!::_stricmp(token, "loop"))
				{
					loopSegment = row;
					ReadToken(token, 32);
				}
				std::strncpy(segments + rowOffset, token, 9);
				ReadToken(token, 32);
				if (!endingCount)
					std::strncpy(token, "NULL", 9);
				std::strncpy(segments + rowOffset + 9, token, 9);
				int cellOffset = rowOffset + 27;
				for (int jump = 0; jump < jumpCount; ++jump)
				{
					ReadToken(token, 32);
					std::strncpy(segments + cellOffset - 9, token, 9);
					ReadToken(token, 32);
					std::strncpy(segments + cellOffset, token, 9);
					cellOffset += 18;
				}
				rowOffset += 9 * columns;
			}
		}
		ReadToken(token, 32);
	}

	CMasterJumplist::CMasterJumplist()
	{
		// The original leaves both fixed text buffers unwritten until Parse.
		pieces.clear();
	}

	CMasterJumplist::~CMasterJumplist()
	{
		Clear();
		pieces.clear();
	}

	void CMasterJumplist::Clear()
	{
		for (std::list<CJumplist*>::iterator item = pieces.begin(); item != pieces.end(); ++item)
		{
			if (*item)
			{
				delete *item;
				*item = NULL;
			}
		}
		pieces.clear();
	}

	CJumplist* CMasterJumplist::Find(const char* value)
	{
		for (std::list<CJumplist*>::iterator item = pieces.begin(); item != pieces.end(); ++item)
		{
			CJumplist* candidate = *item;
			if (!::_stricmp(candidate->name, value))
				return candidate;
		}
		return NULL;
	}

	void CMasterJumplist::Parse(const BYTE* input, DWORD size)
	{
		dataSize = size;
		data = input;
		cursor = 0;
		char token[32], filename[32];
		ReadToken(token, 32);
		std::strncpy(piecePrefix, token, 16);
		ReadToken(token, 32);
		std::strncpy(segmentHeader, token, 16);
		ReadToken(token, 32);
		while (*token)
		{
			std::sprintf(filename, "%s%s", "", token);
			CJumplist* created = new CJumplist;
			if (created->Read(filename))
				delete created;
			else
				pieces.push_back(created);
			ReadToken(token, 32);
		}
	}

	CMusic::CMusic() : CSound()
	{
		segmentBytes = 0;
		segmentRead = 0;
		segmentIndex = 0;
		musicFlags = 0;
		pieceName[0] = 0;
		pendingPiece[0] = 0;
		piece = NULL;
	}
	CMusic::~CMusic()
	{
	}

	int CMusic::SetPiece(const char* name)
	{
		if (!(flags & 2))
		{
			piece = AudioBackend::g_System->jumps->Find(name);
			if (piece)
			{
				segmentIndex = 0;
				char filename[32];
				MakeSegmentName(filename, piece->Segment(0, 0));
				musicFlags = 0;
				segmentBytes = 0;
				segmentRead = 0;
				CSound::Load(filename, 0x34);
			}
			std::strncpy(pieceName, name, 9);
			Play(0, 0, 0);
			return 0;
		}
		if (flags & 4)
		{
			std::strncpy(pendingPiece, name, 9);
			musicFlags |= 4;
			return 0;
		}
		IDirectSoundBuffer* captured = buffer;
		piece = AudioBackend::g_System->jumps->Find(name);
		if (piece)
		{
			segmentIndex = 0;
			char filename[32];
			MakeSegmentName(filename, piece->Segment(0, 0));
			if (stream)
			{
				resourceOwner->Release(stream, 0);
				if (soundData)
				{
					delete soundData;
					soundData = NULL;
				}
				stream = NULL;
			}
			stream = resourceOwner->OpenStream(filename, 1);
			soundData = ReadHeader(stream);
			soundData->bufferBytes = GetBufferSize();
			resourceName.assign(filename);
			flags &= ~2u;
			musicFlags = 0;
			segmentBytes = 0;
			segmentRead = 0;
			FillBuffer(captured);
			flags |= 2;
			captured->SetCurrentPosition(0);
			previousPlayCursor = 0;
			streamWrapCount = 0;
		}
		std::strncpy(pieceName, name, 9);
		Play(0, 0, 0);
		return 0;
	}

	BYTE CMusic::ReadSamples(void* output, DWORD count, Resources::ResourceFileHandle* handle)
	{
		if (formatKind < 2 || formatKind > 3)
			return 0;
		BYTE* target = static_cast<BYTE*>(output);
		if (musicFlags & 0x40)
		{
			std::memset(target, soundData->Silence(), count);
			return 0;
		}
		BYTE result = 0;
		if (!(flags & 2) || segmentRead + count <= segmentBytes)
		{
			if (!(flags & 2) && count > soundData->totalDataBytes)
			{
				flags |= 2;
				segmentBytes = soundData->totalDataBytes;
				ReadSamples(target, count, handle); // Original discards recursive result.
			}
			else
			{
				result = CSound::ReadSamples(target, count, handle);
				segmentRead += count;
			}
			if (!(flags & 2))
			{
				segmentBytes = soundData->totalDataBytes;
				soundData->totalDataBytes = 0x7FFFFFFF;
			}
			return result;
		}

		const LONG prefix = segmentBytes - segmentRead;
		if (prefix > 0)
		{
			result = CSound::ReadSamples(target, prefix, handle);
			target += prefix;
			segmentRead += prefix;
		}
		const DWORD tail = count - prefix;
		if (musicFlags & 0x20)
		{
			soundData->totalDataBytes = prefix + segmentBytes + soundData->bytesRead;
			std::memset(target, soundData->Silence(), tail);
			musicFlags |= 0x40;
			return result;
		}
		if (!piece)
			return result;

		char destination[12], filename[32];
		if (musicFlags & 4)
		{
			const int jump = piece->FindJump(pendingPiece);
			if (jump == -1)
			{
				pendingPiece[0] = 0;
				musicFlags &= ~0x0Cu;
			}
			else
			{
				if (!(musicFlags & 8))
				{
					std::strncpy(destination, piece->Segment(segmentIndex, jump * 2 + 2), 9);
					if (HasSegment(destination))
					{
						MakeSegmentName(filename, destination);
						musicFlags |= 8;
						goto ChangeStream;
					}
					musicFlags |= 8;
				}
				std::strncpy(destination, piece->Segment(segmentIndex, jump * 2 + 3), 9);
				if (HasSegment(destination))
				{
					MakeSegmentName(filename, destination);
					piece = AudioBackend::g_System->jumps->Find(pendingPiece);
					if (piece)
					{
						segmentIndex = piece->FindSegment(destination);
						std::strncpy(pieceName, pendingPiece, 9);
					}
					pendingPiece[0] = 0;
					musicFlags &= ~0x0Cu;
					goto ChangeStream;
				}
			}
		}
		if (musicFlags & 0x10)
		{
			musicFlags &= ~0x10u;
			goto Ending;
		}
		if (segmentIndex == piece->loopSegment)
			segmentIndex = piece->startSegment;
		else if (segmentIndex >= piece->segmentCount - 1)
			goto Ending;
		else
			++segmentIndex;
		MakeSegmentName(filename, piece->Segment(segmentIndex, 0));
		goto ChangeStream;

	Ending:
		std::strncpy(destination, piece->Segment(segmentIndex, 1), 9);
		if (!HasSegment(destination))
		{
			soundData->totalDataBytes = prefix + soundData->bytesRead;
			std::memset(target, soundData->Silence(), tail);
			musicFlags |= 0x40;
			return result;
		}
		MakeSegmentName(filename, destination);
		musicFlags |= 0x20;

	ChangeStream:
		resourceOwner->Release(stream, 0);
		if (soundData)
		{
			delete soundData;
			soundData = NULL;
		}
		stream = NULL;
		stream = resourceOwner->OpenStream(filename, 1);
		soundData = ReadHeader(stream);
		soundData->bufferBytes = GetBufferSize();
		resourceName.assign(filename);
		segmentBytes = soundData->totalDataBytes;
		soundData->totalDataBytes = 0x7FFFFFFF;
		result = CSound::ReadSamples(target, tail, stream);
		segmentRead = tail;
		return result;
	}
}
