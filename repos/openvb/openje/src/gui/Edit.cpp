#include "gui/Edit.hpp"
#include "gui/Event.hpp"
#include <cstdlib>
#include <stdexcept>
#include <mmsystem.h>

namespace GUI
{
	Edit::Edit(DWORD type) : Label(type), readOnly(0), selectionAnchor(0), caretPosition(0),
		insertMode(1), editGeometryDirty(1), editDepthDirty(1), editDepth(0), editDepthIncrement(0),
		blinkTime(0),
		firstSelectionLine(-2), lastSelectionLine(-2), dragging(0)
	{
		//Both ordinary mesh/material-set members have constructed before the
		//original caret-then-selection allocation sequence begins.
		D3DXMatrixIdentity(&caretGeometry.transform);
		caretGeometry.transformEnabled = 1;
		caretGeometry.instances = new Gfx_MeshInstance[1];
		caretGeometry.vertices = 0;
		caretGeometry.vertexCount = caretGeometry.vertexCapacity = 0;
		caretGeometry.indices = 0;
		caretGeometry.indexCount = caretGeometry.indexCapacity = 0;
		caretGeometry.packet = 0;
		caretGeometry.active = 0;
		caretGeometry.packet = new Gfx_PacketData;
		D3DXMatrixIdentity(&selectionGeometry.transform);
		selectionGeometry.transformEnabled = 1;
		selectionGeometry.instances = new Gfx_MeshInstance[1];
		selectionGeometry.vertices = 0;
		selectionGeometry.vertexCount = selectionGeometry.vertexCapacity = 0;
		selectionGeometry.indices = 0;
		selectionGeometry.indexCount = selectionGeometry.indexCapacity = 0;
		selectionGeometry.packet = 0;
		selectionGeometry.active = 0;
		selectionGeometry.packet = new Gfx_PacketData;
		caretColor = 0x80FFFFFF;
		selectionColor = 0x800000FF;
		SetValue3B(0);
	}
	Edit::~Edit() //4C1240: raw owners are released before either mesh member.
	{
		if (caretGeometry.vertices) std::free(caretGeometry.vertices);
		if (caretGeometry.indices) std::free(caretGeometry.indices);
		if (caretGeometry.instances) { delete[] caretGeometry.instances; caretGeometry.instances = 0; }
		if (caretGeometry.packet) { delete caretGeometry.packet; caretGeometry.packet = 0; }
		if (selectionGeometry.vertices) std::free(selectionGeometry.vertices);
		if (selectionGeometry.indices) std::free(selectionGeometry.indices);
		if (selectionGeometry.instances) { delete[] selectionGeometry.instances; selectionGeometry.instances = 0; }
		if (selectionGeometry.packet) { delete selectionGeometry.packet; selectionGeometry.packet = 0; }
	}
	void Edit::Offset(LONG x, LONG y) { Label::Offset(x, y); editGeometryDirty = 1; }
	void Edit::Resize(LONG width, LONG height) { Label::Resize(width, height); editGeometryDirty = 1; }
	void Edit::ApplySkin(const std::string& skin) { Label::ApplySkin(skin); editGeometryDirty = 1; }
	void Edit::UpdateGeometry() { Label::UpdateGeometry(); editGeometryDirty = 1; }
	void Edit::SetFrameVisible(BYTE value) { Label::SetFrameVisible(value); editGeometryDirty = 1; }
	void Edit::SetFont(const std::string& face, DWORD flags, LONG pointSize, DWORD charset)
	{ Label::SetFont(face, flags, pointSize, charset); editGeometryDirty = 1; }
	void Edit::SetFontSize(LONG value) { Label::SetFontSize(value); editGeometryDirty = 1; }
	void Edit::SetPadding(LONG left, LONG right, LONG top, LONG bottom)
	{ Label::SetPadding(left, right, top, bottom); editGeometryDirty = 1; }
	void Edit::SetTextFrame(BYTE value) { Label::SetTextFrame(value); editGeometryDirty = 1; }
	void Edit::SetWrap(BYTE value) { Label::SetWrap(value); editGeometryDirty = 1; }
	void Edit::SetAlignment(DWORD value) { Label::SetAlignment(value); editGeometryDirty = 1; }
	void Edit::SetVerticalAlignment(DWORD value) { Label::SetVerticalAlignment(value); editGeometryDirty = 1; }
	void Edit::SetTabWidth(LONG value) { tabWidth = value; editGeometryDirty = 1; }
	void Edit::OnCaptureChanged(Window*) { dragging = 0; }
	DWORD Edit::GetDepthSlots() { return 2; }
	void Edit::ClampSelection(BYTE followCaret)
	{
		firstSelectionLine = -2;
		std::string value;
		GetText(value);
		const LONG length = LONG(value.size());
		if (!length) { caretPosition = 0; selectionAnchor = 0; SetScroll(0, 0); return; }
		if (selectionAnchor < 0) selectionAnchor = 0;
		else if (selectionAnchor > length) selectionAnchor = length;
		if (caretPosition < 0) caretPosition = 0;
		else if (caretPosition > length) caretPosition = length;
		if (followCaret) FollowCaret();
	}
	void Edit::Refresh()
	{
		Window::Refresh();
		if (editGeometryDirty) { BuildCaretGeometry(); BuildSelectionGeometry(); editGeometryDirty = 0; }
	}
	float Edit::SetDepth(float depth, float increment)
	{
		editDepth = depth;
		editDepthIncrement = increment;
		depth = UpdateEditDepth(depth, increment);
		return Label::SetDepth(depth, increment); //4C07D0 ->4C0510 ->4A7B30.
	}
	void Edit::ResetBlink() { blinkTime = timeGetTime() % 1000; } //4C05A0.

}
