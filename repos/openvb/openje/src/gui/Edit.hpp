#pragma once

#include "gui/Label.hpp"
#include "graphics/PacketData.hpp"

namespace GUI
{
	struct EditVertex { float x, y, z, rhw; DWORD diffuse; };
	struct EditGeometry
	{
		//Implicit ordinary lifetime constructs/destroys only mesh. Edit performs
		//the original raw-owner allocation/cleanup order for both records.
		Gfx_MeshData mesh;
		Gfx_MeshInstance* instances;
		Gfx_PacketData* packet;
		D3DXMATRIX transform;
		DWORD transformEnabled;
		EditVertex* vertices;
		LONG vertexCount, vertexCapacity;
		WORD* indices;
		LONG indexCount, indexCapacity;
		BYTE active;
	};
	class Edit : public Label
	{
	public:
		explicit Edit(DWORD type = 4); //4C51B0, genuine Label/mesh/packet owners.
		virtual ~Edit(); //4C1240.
		virtual void Offset(LONG x, LONG y); //4C0430.
		virtual void Resize(LONG width, LONG height); //4C0450.
		virtual void ApplySkin(const std::string& skin); //4C0490.
		virtual void UpdateGeometry(); //4C04F0.
		virtual void SetFrameVisible(BYTE value); //4C0470.
		virtual void OnCaptureChanged(Window* next); //4C0590.
		virtual DWORD GetDepthSlots(); //4C13B0.
		virtual void Refresh(); //4C3AC0.
		virtual void Draw(); //4C0720.
		virtual float SetDepth(float depth, float increment); //4C07D0.
		virtual BOOL OnEvent(Event* event); //4C4EE0.
		virtual void SetText(const std::string& value); //4C22D0.
		virtual void SetStringReference(DWORD value); //4C2270.
		virtual void AppendText(const std::string& value); //4C2300.
		virtual void UnusedTextOperation(DWORD value); //4C22A0, actual base no-op then dirty/clamp.
		virtual void AppendCharacter(char value); //4C2330.
		virtual void SetFont(const std::string& face, DWORD flags, LONG pointSize, DWORD charset); //4C0330.
		virtual void SetFontSize(LONG value); //4C0360.
		virtual void SetPadding(LONG left, LONG right, LONG top, LONG bottom); //4C0380.
		virtual void SetTextFrame(BYTE value); //4C03B0.
		virtual void SetWrap(BYTE value); //4C03D0.
		virtual void SetAlignment(DWORD value); //4C03F0.
		virtual void SetVerticalAlignment(DWORD value); //4C0410.
		virtual void SetTabWidth(LONG value); //4C04D0.
		void SetSelection(LONG anchor, LONG caret); //4C2360.
		void ClampSelection(BYTE followCaret); //4C13D0 logical body.
		void SetScroll(LONG horizontal, LONG vertical); //4C04B0, actual Label scroll then dirty.
		BYTE readOnly; //E64.
		EditGeometry caretGeometry, selectionGeometry; //E68/F3C, actual normal owners.
		LONG selectionAnchor, caretPosition; //1010/1014.
		BYTE insertMode, editGeometryDirty, editDepthDirty; //1018..101A.
		float editDepth, editDepthIncrement; //101C/1020.
		DWORD caretColor, selectionColor, blinkTime; //1024..102C.
		LONG firstSelectionLine, lastSelectionLine; //1030/1034.
		BYTE dragging; //1038.
	private:
		void BuildCaretGeometry(); //4C2390.
		void BuildSelectionGeometry(); //4C2F20.
		float UpdateEditDepth(float depth, float increment); //4C0510; actual shared depth pass.
		void FollowCaret(); //4C0EF0.
		void ResetBlink(); //4C05A0.
		void ScrollLines(LONG amount); //4C05C0.
		void ScrollColumns(LONG amount); //4C0660.
		void HandleText(Event* event); //4C14D0, logical entry recovered from raw instructions.
		void HandleKey(Event* event); //4C3AF0.
		void CutSelection(); //4C3AA0.
		void CopySelection(); //4C3AB0.
		void CopySelection(BYTE cut); //4C1FA0, wrappers4C3AA0/4C3AB0.
		void Paste(); //4C1960.
		LONG HitTestCaret(LONG x, LONG y); //4C09A0.
		LONG CaretLine() const;
		LONG LineOrigin(const LabelLine& line) const;
		void ClipLine(const LabelLine& line, LONG& x, LONG& y, LONG& width, LONG& height) const; //4A7DB0.
	};
#if defined(_MSC_VER) && _MSC_VER == 1300
	//Raw owner/member placements observed in4C51B0/4C1240/4C2390/4C2F20;
	//these layout checks alone establish no whole-class byte/EH claim.
	typedef char EditVertexExtent[(sizeof(EditVertex) == 0x14) ? 1 : -1];
	typedef char EditGeometryExtent[(sizeof(EditGeometry) == 0xD4) ? 1 : -1];
	typedef char EditExtent[(sizeof(Edit) == 0x103C) ? 1 : -1];
	typedef char EditGeometryMesh[(offsetof(EditGeometry, mesh) == 0x0) ? 1 : -1];
	typedef char EditGeometryInstances[(offsetof(EditGeometry, instances) == 0x6C) ? 1 : -1];
	typedef char EditGeometryPacket[(offsetof(EditGeometry, packet) == 0x70) ? 1 : -1];
	typedef char EditGeometryTransform[(offsetof(EditGeometry, transform) == 0x74) ? 1 : -1];
	typedef char EditGeometryTransformEnabled[(offsetof(EditGeometry, transformEnabled) == 0xB4) ? 1 : -1];
	typedef char EditGeometryVertices[(offsetof(EditGeometry, vertices) == 0xB8) ? 1 : -1];
	typedef char EditGeometryVertexCount[(offsetof(EditGeometry, vertexCount) == 0xBC) ? 1 : -1];
	typedef char EditGeometryVertexCapacity[(offsetof(EditGeometry, vertexCapacity) == 0xC0) ? 1 : -1];
	typedef char EditGeometryIndices[(offsetof(EditGeometry, indices) == 0xC4) ? 1 : -1];
	typedef char EditGeometryIndexCount[(offsetof(EditGeometry, indexCount) == 0xC8) ? 1 : -1];
	typedef char EditGeometryIndexCapacity[(offsetof(EditGeometry, indexCapacity) == 0xCC) ? 1 : -1];
	typedef char EditGeometryActive[(offsetof(EditGeometry, active) == 0xD0) ? 1 : -1];
	typedef char EditReadOnly[(offsetof(Edit, readOnly) == 0xE64) ? 1 : -1];
	typedef char EditCaretGeometry[(offsetof(Edit, caretGeometry) == 0xE68) ? 1 : -1];
	typedef char EditSelectionGeometry[(offsetof(Edit, selectionGeometry) == 0xF3C) ? 1 : -1];
	typedef char EditSelectionAnchor[(offsetof(Edit, selectionAnchor) == 0x1010) ? 1 : -1];
	typedef char EditCaretPosition[(offsetof(Edit, caretPosition) == 0x1014) ? 1 : -1];
	typedef char EditInsertMode[(offsetof(Edit, insertMode) == 0x1018) ? 1 : -1];
	typedef char EditEditGeometryDirty[(offsetof(Edit, editGeometryDirty) == 0x1019) ? 1 : -1];
	typedef char EditEditDepthDirty[(offsetof(Edit, editDepthDirty) == 0x101A) ? 1 : -1];
	typedef char EditEditDepth[(offsetof(Edit, editDepth) == 0x101C) ? 1 : -1];
	typedef char EditEditDepthIncrement[(offsetof(Edit, editDepthIncrement) == 0x1020) ? 1 : -1];
	typedef char EditCaretColor[(offsetof(Edit, caretColor) == 0x1024) ? 1 : -1];
	typedef char EditSelectionColor[(offsetof(Edit, selectionColor) == 0x1028) ? 1 : -1];
	typedef char EditBlinkTime[(offsetof(Edit, blinkTime) == 0x102C) ? 1 : -1];
	typedef char EditFirstSelectionLine[(offsetof(Edit, firstSelectionLine) == 0x1030) ? 1 : -1];
	typedef char EditLastSelectionLine[(offsetof(Edit, lastSelectionLine) == 0x1034) ? 1 : -1];
	typedef char EditDragging[(offsetof(Edit, dragging) == 0x1038) ? 1 : -1];
#endif

}
