#pragma once

#include "gui/Window.hpp"
#include "gui/Font.hpp"
#include "graphics/UIVertex.hpp"
#include <vector>

namespace GUI
{
	struct LabelLine
	{
		LabelLine() {} //Direct caller field setup leaves the two padding bytes unwritten.
		LabelLine(DWORD id, LONG begin, LONG end, LONG x, LONG y, LONG width,
			LONG height, LONG horizontalScroll, LONG verticalScroll, BYTE hidden, BYTE hardBreak); //4A76E0.
		DWORD id;
		LONG begin, end, x, y, width, height, horizontalScroll, verticalScroll;
		BYTE hidden, hardBreak;
	};

	// Actual GUI::Label. Constructed world labels use these ordinary owners;
	// serialized Label readers remain explicit boundaries.
	class Label : public Window
	{
		friend class Edit; //Actual Edit4 reuses Label's glyph measure/clip family.
	public:
		explicit Label(DWORD type = 3); //4A90E0.
		virtual ~Label(); //4A8F40.
		virtual void Offset(LONG x, LONG y); //4A7940.
		virtual void Resize(LONG width, LONG height); //4AA3C0.
		virtual BYTE ReadResource(JE::Resources::ResourceFileHandle*, int);
		virtual BYTE ReadFile(FILE*, int);
		virtual BYTE WriteFile(FILE*);
		virtual void Draw(); //4A80D0.
		virtual float SetDepth(float depth, float increment); //4A7B30.
		virtual void UpdateGeometry(); //4A7BC0.
		virtual void SetColor(DWORD value); //4A78D0.
		virtual void SetAlpha(BYTE value); //4A7900.
		virtual void SetFrameVisible(BYTE value); //4A7B20,+84; forwards through actual Label+178.
		virtual void ApplySkin(const std::string& value); //4A8B00.
		virtual void SetText(const std::string& value); //4AA250, original+12C.
		virtual void SetStringReference(DWORD value); //4AA150, original+130.
		virtual void AppendText(const std::string& value); //4AA280, original+134; preserves stringReference.
		virtual void UnusedTextOperation(DWORD value); //Shared5A9320,+138, actual one-word no-op; argument meaning unresolved.
		virtual void GetText(std::string& output); //4A8A90,+13C; assigns caller's live string.
		virtual DWORD GetStringReference(DWORD& output) const; //4A7740,+140; required output word.
		virtual void AppendCharacter(char value); //4AA2B0,+144; preserves stringReference.
		virtual void SetFont(const std::string& face, DWORD flags, LONG pointSize, DWORD charset); //4AA2D0,+148.
		virtual void GetFont(std::string& face, DWORD* flags, LONG* pointSize, DWORD* charset); //4A8AB0,+14C.
		virtual void SetFontSize(LONG value); //4AA310,+150.
		virtual LONG GetFontSize() const; //4A7750,+154.
		virtual LONG GetVisibleTextHeight() const; //4A7760,+158, textHeight field.
		virtual LONG GetTextWidth() const; //4A7770,+15C.
		virtual LONG GetTextHeight() const; //4A7780,+160.
		virtual LONG GetMaximumRowWidth() const; //4A7790,+164.
		virtual void SetTextColor(DWORD value); //4A77A0,+168.
		virtual DWORD GetTextColor() const; //4A7810,+16C.
		virtual void SetPadding(LONG left, LONG right, LONG top, LONG bottom); //4AA330,+170.
		virtual void GetPadding(LONG& left, LONG& right, LONG& top, LONG& bottom) const; //4A7820,+174.
		virtual void SetTextFrame(BYTE value); //4A7860,+178.
		virtual BYTE GetTextFrame() const; //4A7890,+17C.
		virtual void SetWrap(BYTE value); //4AA360,+180.
		virtual BYTE GetWrap() const; //4A78A0,+184.
		virtual void SetAlignment(DWORD value); //4AA380,+188.
		virtual DWORD GetAlignment() const; //4A78B0,+18C.
		virtual void SetVerticalAlignment(DWORD value); //4AA3A0,+190.
		virtual DWORD GetVerticalAlignment() const; //4A78C0,+194.
		virtual void SetScroll(LONG horizontal,LONG vertical); //4AA130,+198.
		virtual LONG GetScroll(LONG& horizontal,LONG& vertical) const; //4A7D40,+19C; required output words.
		virtual void SetTabWidth(LONG value); //4588A0,+1A0.
		virtual LONG GetTabWidth() const; //4588B0,+1A4.
		LONG GetTextAdvance(LONG startX); //4A89C0.
		LONG GetTextAdvance(const std::string& value, LONG startX); //4A8900, actual font-resolving overload.
		LONG GetLineCount() const; //434220; complete stored line count.
		LabelLine* GetLine(LONG index); //4A87B0, original bounded row lookup.
		LONG FindLine(LONG character) const; //4A8800, original row-range lookup.
		BYTE GetLineText(LONG index,std::string& output); //4A8B90; existing layout rows, end-exclusive.
		void RebuildText(); //4A93A0.
		void ClearGeometry(); //4A8940.

		DWORD stringReference;
		std::string text, fontName;
		DWORD fontFlags;
		LONG fontSize;
		DWORD fontCharset, textColor;
		BYTE textFrame, wrap;
		DWORD alignment, displayedCharacters;
		RECT* glyphRectangles; //D74, scalar operator-new storage released with real free.
		Gfx_MeshData textMesh;
		Gfx_Primitive* textPrimitives; //DE4, borrowed alias of textMesh.primitives; ClearGeometry leaves it unwritten.
		DWORD textVertexCount; //DE8, six vertices per complete source character, not emitted glyph count.
		JE::Gfx::UIVertex* textVertices; //DEC, owned scalar operator-new storage.
		Gfx_MeshInstance textInstance;
		BYTE textGeometryDirty;
		LONG paddingLeft, paddingRight, paddingTop, paddingBottom;
		LONG totalHeight, maximumRowWidth, textHeight, textWidth;
		LONG horizontalScroll, verticalScroll;
		DWORD verticalAlignment;
		LONG tabWidth;
		std::vector<LabelLine> lines;
		LONG firstVisibleCharacter;
		DWORD visibleLines;
	private:
		void Measure(Font*, char, FontGlyph&, LONG); //4A7D60, output record precedes X.
		LONG MeasureAdvance(const std::string& value, Font* font, LONG x); //4A8200, actual Label helper.
		BYTE WordBoundary(const std::string&, LONG); //4A8280, actual member call; this is unused.
		LONG MeasureWord(const std::string&, Font*, LONG, LONG); //4A8340.
		LONG EmitLine(DWORD destination, LONG firstVisible, LONG begin, LONG end,
			LONG x, LONG y, LONG width, LONG sx, LONG sy, BYTE hiddenLine); //4A8490.
		void ClipGlyph(const FontGlyph&, LONG, LONG, LONG, LONG,
			LONG&, LONG&, LONG&, LONG&, float*); //4A7EC0.
		Label(const Label&);
		Label& operator=(const Label&);
	};

#if defined(_MSC_VER) && _MSC_VER == 1300
	//Observed VC7 member placements, including actual geometry owners. These
	//assertions do not establish constructor EH, complete behavior or whole ABI.
	typedef char LabelOriginalStringReference[(offsetof(Label, stringReference) == 0xD1C) ? 1 : -1];
	typedef char LabelOriginalText[(offsetof(Label, text) == 0xD20) ? 1 : -1];
	typedef char LabelOriginalFontName[(offsetof(Label, fontName) == 0xD3C) ? 1 : -1];
	typedef char LabelOriginalFontFlags[(offsetof(Label, fontFlags) == 0xD58) ? 1 : -1];
	typedef char LabelOriginalFontSize[(offsetof(Label, fontSize) == 0xD5C) ? 1 : -1];
	typedef char LabelOriginalFontCharset[(offsetof(Label, fontCharset) == 0xD60) ? 1 : -1];
	typedef char LabelOriginalTextColor[(offsetof(Label, textColor) == 0xD64) ? 1 : -1];
	typedef char LabelOriginalTextFrame[(offsetof(Label, textFrame) == 0xD68) ? 1 : -1];
	typedef char LabelOriginalWrap[(offsetof(Label, wrap) == 0xD69) ? 1 : -1];
	typedef char LabelOriginalAlignment[(offsetof(Label, alignment) == 0xD6C) ? 1 : -1];
	typedef char LabelOriginalDisplayedCharacters[(offsetof(Label, displayedCharacters) == 0xD70) ? 1 : -1];
	typedef char LabelOriginalGlyphRectangles[(offsetof(Label, glyphRectangles) == 0xD74) ? 1 : -1];
	typedef char LabelOriginalMesh[(offsetof(Label, textMesh) == 0xD78) ? 1 : -1];
	typedef char LabelOriginalMeshSize[(sizeof(Gfx_MeshData) == 0x6C) ? 1 : -1];
	typedef char LabelOriginalPrimitives[(offsetof(Label, textPrimitives) == 0xDE4) ? 1 : -1];
	typedef char LabelOriginalVertexCount[(offsetof(Label, textVertexCount) == 0xDE8) ? 1 : -1];
	typedef char LabelOriginalVertices[(offsetof(Label, textVertices) == 0xDEC) ? 1 : -1];
	typedef char LabelOriginalInstance[(offsetof(Label, textInstance) == 0xDF0) ? 1 : -1];
	typedef char LabelOriginalInstanceSize[(sizeof(Gfx_MeshInstance) == 0x28) ? 1 : -1];
	typedef char LabelOriginalGeometryDirty[(offsetof(Label, textGeometryDirty) == 0xE18) ? 1 : -1];
	typedef char LabelOriginalPaddingLeft[(offsetof(Label, paddingLeft) == 0xE1C) ? 1 : -1];
	typedef char LabelOriginalPaddingRight[(offsetof(Label, paddingRight) == 0xE20) ? 1 : -1];
	typedef char LabelOriginalPaddingTop[(offsetof(Label, paddingTop) == 0xE24) ? 1 : -1];
	typedef char LabelOriginalPaddingBottom[(offsetof(Label, paddingBottom) == 0xE28) ? 1 : -1];
	typedef char LabelOriginalTotalHeight[(offsetof(Label, totalHeight) == 0xE2C) ? 1 : -1];
	typedef char LabelOriginalMaximumRowWidth[(offsetof(Label, maximumRowWidth) == 0xE30) ? 1 : -1];
	typedef char LabelOriginalTextHeight[(offsetof(Label, textHeight) == 0xE34) ? 1 : -1];
	typedef char LabelOriginalTextWidth[(offsetof(Label, textWidth) == 0xE38) ? 1 : -1];
	typedef char LabelOriginalHorizontalScroll[(offsetof(Label, horizontalScroll) == 0xE3C) ? 1 : -1];
	typedef char LabelOriginalVerticalScroll[(offsetof(Label, verticalScroll) == 0xE40) ? 1 : -1];
	typedef char LabelOriginalVerticalAlignment[(offsetof(Label, verticalAlignment) == 0xE44) ? 1 : -1];
	typedef char LabelOriginalTabWidth[(offsetof(Label, tabWidth) == 0xE48) ? 1 : -1];
	typedef char LabelOriginalLines[(offsetof(Label, lines) == 0xE4C) ? 1 : -1];
	typedef char LabelOriginalFirstVisibleCharacter[(offsetof(Label, firstVisibleCharacter) == 0xE5C) ? 1 : -1];
	typedef char LabelOriginalVisibleLines[(offsetof(Label, visibleLines) == 0xE60) ? 1 : -1];
	typedef char LabelOriginalLineSize[(sizeof(LabelLine) == 0x28) ? 1 : -1];
	typedef char LabelOriginalExtent[(sizeof(Label) == 0xE64) ? 1 : -1];
#endif
}
