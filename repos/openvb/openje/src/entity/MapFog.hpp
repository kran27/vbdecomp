#pragma once

#include <windows.h>
#include <d3dx8.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <list>
#include <map>

namespace JE
{
	struct AreaMapBits
	{
		AreaMapBits() : size(0), data(NULL) {}
		~AreaMapBits(); // 0x5AD6C0; real CRT buffer ownership.
		void Clear(); // 0x5AD7B0.
		BYTE GetBit(LONG index) const; //5AD760; actual raw-buffer receiver.
		void SetBit(LONG index, BYTE value); //5AD6D0; resize then write one bit.
		void Read(FILE* input); //5AD7E0, original empty-buffer/error quirks retained.
		void Write(FILE* output); //5AD850, original size/header checks.
		DWORD size;
		BYTE* data;
	private:
		AreaMapBits(const AreaMapBits&);
		AreaMapBits& operator=(const AreaMapBits&);
	};

	// Actual108-byte note owner. Its two slots are FILE read/write; destruction
	// is nonvirtual. Saved-note serialization uses the original WORD string fields.
	class AreaMapNote
	{
	public:
		AreaMapNote(); // 0x57C400.
		AreaMapNote(const AreaMapNote& other); // 0x57D7C0.
		~AreaMapNote(); // 0x57C450.
		AreaMapNote& operator=(const AreaMapNote& other); // 0x57D3A0.
		virtual void Read(FILE* input); // 0x57D440.
		virtual void Write(FILE* output); // 0x57C8D0.
		void ClearTexture(); // 0x57C060.
		BOOL LoadTexture(); //57C610, normalized handle-presence result; ordinary ECX/no stack arguments.
		std::string name; // +04.
		DWORD value20, value24, value28, value2C;
		LONG value30;
		std::string text; // +34.
		float position[3]; // +50.
		BYTE flag5C, flag5D;
		DWORD texture, textureWidth, textureHeight; // +60/+64/+68.
	};

	struct AreaMapHeight
	{
		LONG first, row;
		float height;
	};
	struct AreaMapRectangle
	{
		LONG first, row, last, lastRow;
		float height;
	};
	struct AreaMapLayer
	{
		AreaMapLayer() : disabled(0) {}
		std::list<AreaMapHeight> cells;
		BYTE disabled;
	};

	// Actual AreaMap RTTI/table69A52C,180-byte VC7 owner. The map owner calls
	// this a fog/map-mask resource; it is not a stand-in for a GUI Picture.
	class AreaMap
	{
	public:
		AreaMap(DWORD value4, DWORD value8, const std::string& textureName,
			float minX, float minZ, float maxX, float maxZ, LONG screenWidth, LONG screenHeight); // 57FAD0.
		virtual ~AreaMap(); // 57F760.
		virtual void Read(FILE* input); // 57FF40, original note/bit-array snapshot.
		virtual void Write(FILE* output); // 57C140.
		float GetCellSpacing() const; //57BEF0; physical AreaMap receiver, nominal API inferred.
		void ResetHeights(); // 57ECE0.
		void ResetSnapshot(); // Source forwarding for Map56DE70's visible.Clear().
		void CellToWorld(LONG first, LONG row, float* x, float* z, BYTE center); // 57BE60.
		void WorldToCell(float x, float y, float z, LONG* first, LONG* row) const; //57BDC0; RET20.
		void WorldToCell(const D3DXVECTOR3& position, LONG* first, LONG* row) const; //Convenience adapter.
		BYTE IsVisible(LONG first, LONG row) const; //57BD40/5AD760.
		BYTE SetVisible(LONG first, LONG row, BYTE value); //57BD80, RET12.
		BYTE IsExplored(LONG first, LONG row) const; //57BC90.
		BYTE SetExplored(LONG first, LONG row, BYTE value); //57BCD0, changes bit0 on a changed cell.
		BYTE SetHeight(LONG first, LONG row, float height); // 57C100.
		void RebuildHeights(); //57DF50, base copy then ordered influence writes.
		float GetHeight(LONG first, LONG row); //57E390, rebuild when dirty.
		BYTE TestLOS(LONG fromFirst, LONG fromRow, float fromHeight,
			LONG toFirst, LONG toRow, const float* targetHeight); //57E450.
		void AddLayerHeight(LONG layer, LONG first, LONG row, float height); // 5802C0.
		BYTE SetLayerVisible(LONG layer, BYTE visible); //57C5C0, find only.
		BYTE RemoveHeightList(DWORD id); //57EB40.
		BYTE RemoveHeightRectangle(DWORD id); //57D4F0.
		BYTE SetHeightList(DWORD id, const std::list<AreaMapHeight>& cells); //580260.
		BYTE SetHeightRectangle(DWORD id, float lowX, float lowY, float lowZ,
			float highX, float highY, float highZ); //57DE50, seven scalar words; RET28.
		LONG AddNote(const std::string& name, DWORD value20, DWORD value24,
			DWORD value28, DWORD value2C, LONG value30, const float* position, BYTE flag); //57FE20.
		LONG AddNote(const std::string& name, DWORD value20, DWORD value24,
			DWORD value28, DWORD value2C, const std::string& text, const float* position, BYTE flag); //57FCF0.
		AreaMapNote* FindNote(LONG index); //41F2B0, borrowed existing element.
		BYTE RemoveNote(LONG index); //57E3F0, actual vector erasure and bit1 dirty.

		DWORD value4, value8;
		std::string textureName; // +0C.
		float minX, minZ, maxX, maxZ; // +28..34.
		LONG width, height; // +38/+3C; half-unit extent ceiling.
		AreaMapBits explored; // +40; actual raw buffer owner.
		std::vector<AreaMapNote> notes; // +48.
		LONG screenWidth, screenHeight; // +58/+5C.
		AreaMapBits visible; // +60.
		std::vector<float> heights; // +68.
		DWORD changes; // +78.
		std::map<DWORD, AreaMapRectangle> rectangles; // +7C; unsigned object keys.
		std::map<DWORD, std::list<AreaMapHeight> > heightLists; // +88; unsigned keys.
		BYTE heightsDirty; // +94; implicit padding left unwritten.
		std::vector<float> derivedHeights; // +98.
		std::map<LONG, AreaMapLayer> layers; // +A8; signed layer keys.
	private:
		AreaMap(const AreaMap&);
		AreaMap& operator=(const AreaMap&);
	};

	typedef AreaMap FogOfWar;

#if defined(_MSC_VER) && _MSC_VER == 1300
	typedef char AreaMapBitsOriginalExtent[(sizeof(AreaMapBits) == 8) ? 1 : -1];
	typedef char AreaMapNoteOriginalExtent[(sizeof(AreaMapNote) == 0x6C) ? 1 : -1];
	typedef char AreaMapNoteOriginalName[(offsetof(AreaMapNote, name) == 4) ? 1 : -1];
	typedef char AreaMapNoteOriginalWords[(offsetof(AreaMapNote, value20) == 0x20) ? 1 : -1];
	typedef char AreaMapNoteOriginalText[(offsetof(AreaMapNote, text) == 0x34) ? 1 : -1];
	typedef char AreaMapNoteOriginalPosition[(offsetof(AreaMapNote, position) == 0x50) ? 1 : -1];
	typedef char AreaMapNoteOriginalFlag[(offsetof(AreaMapNote, flag5C) == 0x5C) ? 1 : -1];
	typedef char AreaMapNoteOriginalFlag2[(offsetof(AreaMapNote, flag5D) == 0x5D) ? 1 : -1];
	typedef char AreaMapNoteOriginalTexture[(offsetof(AreaMapNote, texture) == 0x60) ? 1 : -1];
	typedef char AreaMapOriginalExtent[(sizeof(AreaMap) == 0xB4) ? 1 : -1];
	typedef char AreaMapOriginalBounds[(offsetof(AreaMap, minX) == 0x28) ? 1 : -1];
	typedef char AreaMapOriginalWidth[(offsetof(AreaMap, width) == 0x38) ? 1 : -1];
	typedef char AreaMapOriginalExplored[(offsetof(AreaMap, explored) == 0x40) ? 1 : -1];
	typedef char AreaMapOriginalNotes[(offsetof(AreaMap, notes) == 0x48) ? 1 : -1];
	typedef char AreaMapOriginalVisible[(offsetof(AreaMap, visible) == 0x60) ? 1 : -1];
	typedef char AreaMapOriginalChanges[(offsetof(AreaMap, changes) == 0x78) ? 1 : -1];
#endif
}
