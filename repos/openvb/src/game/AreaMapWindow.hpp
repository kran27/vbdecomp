#pragma once
#include "gui/Picture.hpp"
#include "graphics/MeshData.hpp"
#include "graphics/PacketData.hpp"
#include <d3dx8.h>
#include <stddef.h>
#include <list>
#include <vector>

namespace JE { class AreaMap; class AreaMapNote; }
namespace GUI { class LineSystem; }
class AreaMapIcon;

class AreaMapWindowServices
{
public:
	virtual ~AreaMapWindowServices() {}
	// Actual41EA11 reads the visible Actionbar child and its absolute top.
	virtual BYTE GetVisibleActionbarTop(LONG& top) = 0;
};
AreaMapWindowServices* SetAreaMapWindowServices(AreaMapWindowServices* services);

struct AreaMapVertex
{
	float x, y, z, rhw;
	DWORD color;
	float u, v, maskU, maskV;
};

// Ordinary members factoring the two original mesh/instance/packet groups.
struct AreaMapWindowBatch
{
	AreaMapWindowBatch();
	~AreaMapWindowBatch();
	void Clear();
	Gfx_MeshData mesh;
	Gfx_MeshInstance* instances;
	Gfx_PacketData* packet;
	D3DXMATRIX transform;
	DWORD instanceCount;
	AreaMapVertex* vertices;
	DWORD vertexCount, vertexCapacity;
	WORD* indices;
	DWORD indexCount, indexCapacity;
	BYTE ready;
};

class AreaMapWindow : public ::GUI::Picture
{
public:
	explicit AreaMapWindow(JE::AreaMap* areaMap); //422D40; borrowed.
	virtual ~AreaMapWindow(); //420730.
	virtual void Refresh(); //423D40.
	virtual void Draw(); //421740.
	virtual float SetDepth(float depth, float increment); //41F3D0.
	virtual float GetDepth(); //41D200.
	virtual void UpdateGeometry(); //41D340.
	virtual BOOL OnEvent(::GUI::Event* event); //41F410.

	void SetAreaMap(JE::AreaMap* value); //41D4D0.
	void SetOrigin(LONG x, LONG y); //41D220.
	BYTE WorldToMap(float x, float z, float& mapX, float& mapY); //41D2D0.
	BYTE WorldToIntegerMap(float x, float z, LONG& mapX, LONG& mapY); //41D250; actual PipBoy callers.
	BYTE GetExploredBounds(RECT& output, float unusedZoom); //41D360; RET8.
	void RebuildGeometry(); //4231B0.
	void RebuildVisibleSamples(); //4235A0; original note/icon list.
	void UpdatePartyHover(LONG x, LONG y); //41F080.
	void UpdateNoteHover(LONG x, LONG y); //41EF80.
	void SetNoteHoverEnabled(BYTE value); //41F480, normalizes to0/1.
	void SetPartyHoverEnabled(BYTE value); //41F4D0, normalizes to0/1.
	AreaMapIcon* HitNoteIcon(LONG x, LONG y, LONG* index); //41E820; optional original index output.
	JE::AreaMapNote* HitNote(LONG x, LONG y, LONG* index); //41E8D0; borrowed note.
	AreaMapIcon* HitPartyIcon(LONG x, LONG y, DWORD* index); //41E900; showParty gate.
	BYTE ComputeNoteRectangle(JE::AreaMapNote*, LONG&, LONG&, LONG&, LONG&); //41DBD0, actual PipBoy caller.

	JE::AreaMap* areaMap; //Original+D2C; borrowed.
	LONG originX, originY; //D30/D34.
	AreaMapWindowBatch background, scanlines; //D38/E0C original groups.
	::GUI::LineSystem* cameraOutline; //EE0; owning.
	std::list<AreaMapIcon*> noteIcons; //EE4; owning.
	std::vector<AreaMapIcon*> partyIcons; //EF0; owns all6.
	std::list<AreaMapIcon*> visiblePartyIcons; //F00; borrows partyIcons.
	BYTE overPartyIcon, dirty; //F0C/F0D.
	D3DXVECTOR3 cameraCorners[4], previousCameraCorners[4]; //F10/F40, unwritten initially.
	BYTE depthDirty; //F70, initialized0; three following bytes unwritten.
	float mapDepth, mapDepthIncrement; //F74/F78, original ctor-unwritten.
	BYTE noteHoverEnabled; //F7C.
	DWORD maskTexture; //F80, borrowed.
	float zoom; //F84.
	DWORD scanlineTexture; //F88, acquired/owned.
	BYTE partyHoverEnabled, flagF8D, flagF8E; //F8C/F8D/F8E.
	LONG scanlineWidth, scanlineHeight; //F90/F94; initial2,2.
	float scanlineUScale, scanlineVScale; //F98/F9C; initial1,1.
	BYTE showParty, showCamera; //FA0/FA1; initial1,1.

private:
	BYTE ComputeImageLayout(LONG& x, LONG& y, LONG& width, LONG& height,
		float& u, float& v, float& uvWidth, float& uvHeight,
		float& maskU, float& maskV, float& maskWidth, float& maskHeight,
		LONG& sourceX, LONG& sourceY, LONG& sourceWidth, LONG& sourceHeight); //41D7B0.
	BYTE ComputeNoteSource(JE::AreaMapNote*, float&, float&, float&, float&); //41DD90.
	BYTE ComputePartyRectangle(float x, float z, LONG iconWidth, LONG iconHeight,
		LONG&, LONG&, LONG&, LONG&, float&, float&, float&, float&); //41DFA0.
	void UpdatePartyIcons(); //421880.
	void BuildImageGeometry(); //4209D0.
	void UpdateCameraFootprint(); //41E9B0.
	void RebuildCameraOutline(); //41E1F0.
	float AssignContentDepth(float depth, float increment); //41E6B0.
	void ClearNoteIcons(); //41F6E0.
	AreaMapWindow(const AreaMapWindow&);
	AreaMapWindow& operator=(const AreaMapWindow&);
};


#if defined(_MSC_VER) && _MSC_VER == 1300 && defined(_M_IX86)
//Original allocation422D40 callers and critical constructor/store boundaries.
typedef char AreaMapWindowBatchSize[(sizeof(AreaMapWindowBatch)==0xD4)?1:-1];
typedef char AreaMapWindowBatchMatrix[(offsetof(AreaMapWindowBatch,transform)==0x74)?1:-1];
typedef char AreaMapWindowBatchReady[(offsetof(AreaMapWindowBatch,ready)==0xD0)?1:-1];
typedef char AreaMapWindowSize[(sizeof(AreaMapWindow)==0xFA4)?1:-1];
#define AREA_MAP_WINDOW_OFFSET(member,offset) \
	typedef char AreaMapWindow_##member[(offsetof(AreaMapWindow,member)==offset)?1:-1]
AREA_MAP_WINDOW_OFFSET(background,0xD38);
AREA_MAP_WINDOW_OFFSET(scanlines,0xE0C);
AREA_MAP_WINDOW_OFFSET(cameraOutline,0xEE0);
AREA_MAP_WINDOW_OFFSET(partyIcons,0xEF0);
AREA_MAP_WINDOW_OFFSET(overPartyIcon,0xF0C);
AREA_MAP_WINDOW_OFFSET(depthDirty,0xF70);
AREA_MAP_WINDOW_OFFSET(zoom,0xF84);
AREA_MAP_WINDOW_OFFSET(scanlineTexture,0xF88);
AREA_MAP_WINDOW_OFFSET(flagF8D,0xF8D);
AREA_MAP_WINDOW_OFFSET(showCamera,0xFA1);
#undef AREA_MAP_WINDOW_OFFSET
#endif
