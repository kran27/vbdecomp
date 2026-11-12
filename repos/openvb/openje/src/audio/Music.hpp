#pragma once

#include "Sound.hpp"
#include "SoundFile.hpp"
#include <list>

namespace JE
{
	// Proprietary piece/transition tables. Their nine-byte strings are an
	// original resource-format boundary, independent of native STL layouts.
	class CJumplist : public SoundFile
	{
	public:
		CJumplist(); // 538B70.
		~CJumplist(); // 538BB0.
		virtual void Parse(const BYTE* input, DWORD size); // 538BF0.
		const char* Jump(int index) const; // 538F30.
		const char* Segment(int row, int column) const; // 538F50.
		int FindJump(const char* name) const; // 538F90.
		int FindSegment(const char* name) const; // 538FE0.

		int jumpCount, segmentCount, endingCount, stingerCount;
		int startSegment, loopSegment;
		char name[9];
		char* jumps;
		char* segments;
	};

	class CMasterJumplist : public SoundFile
	{
	public:
		CMasterJumplist(); // 539120.
		~CMasterJumplist(); //539240, misleading imported locale name is proprietary here.
		virtual void Parse(const BYTE* input, DWORD size); // 539A00.
		CJumplist* Find(const char* name); // 5390D0.
		void Clear(); // 539190.

		char piecePrefix[16], segmentHeader[16];
		std::list<CJumplist*> pieces;
	};

	class CMusic : public CSound
	{
	public:
		CMusic(); // 539030.
		~CMusic(); // 539070, then nonvirtual CSound destructor.
		virtual BYTE ReadSamples(void* output, DWORD count,
			Resources::ResourceFileHandle* handle); // 539570.
		int SetPiece(const char* name); // 539370.

		CJumplist* piece; // Original +F8, borrowed from the master list.
		DWORD musicFlags; // +FC.
		int segmentIndex; // +100.
		DWORD segmentBytes, segmentRead; // +104/+108.
		char pieceName[9], pendingPiece[9]; // +10C/+115.

	private:
		void MakeSegmentName(char* output, const char* name); //539080, native thiscall.
		BYTE HasSegment(const char* name); //5390A0, native one-byte predicate.
	};
}
