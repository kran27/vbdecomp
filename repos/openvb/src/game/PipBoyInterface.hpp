#pragma once

#include "game/GameInterface.hpp"
#include "entity/MapFog.hpp"
#include <map>
#include <utility>

class AreaMapWindow;
namespace JE { class TaskEntry; }
namespace F3
{
	extern const float g_PipBoyMapZooms[5]; //65BB5C, shared original scale table.
	class CounterControl;
	class PipBoyAddNoteInterface;
	struct PipBoyRectangle
	{
		LONG x, y, width, height;
	};
	//454C30: signed group/task keys; each task keeps borrowed journal entries.
	//The24B value owns two real maps. pair is a compatible source spelling;
	//the original absence of RTTI cannot distinguish pair from a two-member struct.
	typedef std::vector<JE::TaskEntry*> PipBoyTaskEntries;
	typedef std::map<LONG, PipBoyTaskEntries> PipBoyTaskIndex;
	typedef std::pair<PipBoyTaskIndex, PipBoyTaskIndex> PipBoyTaskGroup;
	typedef std::map<LONG, PipBoyTaskGroup> PipBoyTaskGroups;

	//Actual CPipBoyInterface. Normal host fields; not an original-layout overlay.
	//The page0/reset/open/close paths are recovered. Other page owners retain
	//their addressed boundaries before their populated data is accessed.
	class PipBoyInterface : public GameInterface
	{
	public:
		explicit PipBoyInterface(const std::string& name); //455E40.
		virtual ~PipBoyInterface(); //455B90, then real base lifetime.
		virtual int OnEvent(::GUI::Window*, ::GUI::Event*); //455DB0.
		virtual void BeforeUpdate(); //455DA0 ->4518A0.
		void Reset(); //455A70, no stack arguments.
		void SelectPage(DWORD page); //454DC0, RET4.
		LONG CurrentPage() const { return currentPage; }
		void RefreshClock(BYTE force); //44A590.
		void Open(DWORD entity); //455DC0, selected entity and fresh page0.
		void Close(); //44A3B0, hide both interfaces and finish digit transitions.
		void RebuildTasks(); //454C30; grouped open/closed borrowed TaskEntry pointers.
		void RefreshTaskLabels(); //44BE60, task groups and original selection rows.
		void RefreshTaskDescription(); //44EB80, selected journal task text.
		void RefreshDownloads(); //44C790, original prototype holodisc/video rows.
		LONG GetTaskRowCount() const; //44A6C0, signed grouped/task row extent.
		LONG GetMapRowCount() const; //44A420, signed map/area row extent.
		LONG GetDownloadRowCount() const; //44A440, signed prototype row extent.
		JE::AreaMap* SelectedAreaMap(); //44AB40, borrowed selected map orNULL.
		void RefreshMaps(); //450230, actual area/map vectors and authored labels.
		BYTE FinishNoteEdit(BYTE accepted, BYTE remove); //4533F0.
		void SetNoteText(const std::string& text); //44D900; unused return spelling unresolved.
		void SetNoteEditMode(DWORD mode); //452240.
		BYTE UpdateNotePopup(LONG x, LONG y); //4524B0.

		DWORD value4; //+04, selected entity; constructor0, Open receives Client+E0.
		LONG lilTextureWidth, lilTextureHeight; //+08/+0C.
		PipBoyRectangle lilPip1, lilPip2; //+10/+20, borrowed widget rectangles.
		std::string lilTextureName; //+30.
		LONG currentPage; //+4C, constructor-unwritten; Reset publishes it.
		LONG alarmEmptyMetric; //+50,4A7790 captured after temporarily clearing alarm text.
		std::string alarmText; //+54.
		DWORD alarmColor; //+70, actual Label::textColor getter4A7810.
		DWORD* labelReferences; //+74,14 ordinary DWORDs, owned allocation.
		LONG value78, value7C;
		DWORD taskColor; //+80.
		LONG taskTop, taskScroll; //+84/+88.
		PipBoyTaskGroups taskGroups; //+8C, owning tree; group.first open, second closed.
		DWORD value98, value9C;
		BYTE mapMode; //+A0, raw BYTE.
		LONG valueA4, valueA8;
		DWORD mapsColor; //+AC.
		LONG mapTop, valueB4, valueB8, valueBC;
		PipBoyAddNoteInterface* addNote; //+C0, separate registered Interface; dtor omits delete.
		AreaMapWindow* primaryMap; //+C4, owned when actually constructed.
		AreaMapWindow* secondaryMap; //+C8.
		LONG mapX, mapY, minimumX, maximumX, minimumY, maximumY, noteIndex; //+CC..E4.
		JE::AreaMapNote note; //+E8, real embedded note owner57C400/57C450.
		LONG noteIconIndex, mapZoomIndex; //+154/+158, selected icon1/map zoom2; not two zoom indices.
		std::vector<DWORD> areaIDs; //+15C;450230 copies distinct AreaMap::value4.
		std::vector<JE::AreaMap*> areaMaps; //+16C; borrowed PlayerAreaMaps values.
		DWORD cachedTexture; //+17C.
		D3DFORMAT cachedMaskFormat; //+180,44AB80 producer; constructor-unwritten.
		DWORD cachedMapID; //+184.
		std::string cachedMapName; //+188.
		LONG value1A4, value1A8;
		DWORD downloadsColor; //+1AC.
		LONG downloadsTop, value1B4, value1B8; //+1B0/+1B4/+1B8: top, videos, holodiscs.
		DWORD value1BC, value1C0, holdDelay, repeatDelay; //+1BC..1C8.
		CounterControl* dayCounter; //+1CC.
		CounterControl* monthCounter; //+1D0, actual twelve-texture numeric owner.
		CounterControl* yearCounter; //+1D4.
		CounterControl* timeCounter; //+1D8.
	private:
		void HandleButtonInput(::GUI::Window* window, LONG eventType); //451B80; incidental return unused.
		void HandleLabelInput(::GUI::Window* window, LONG eventType); //452910; RET8.
		void HandleMapInput(AreaMapWindow* window, LONG x, LONG y); //452EB0; RET12.
		LONG SelectedAlarmIndex() const; //44AAC0; original Rest click only queries this value.
		void ToggleMapMode(); //451650.
		void ToggleNoteEditMode(LONG mode); //452490.
		void SetMapZoom(DWORD index, BYTE anchored, LONG x, LONG y); //452560.
		D3DXVECTOR3 MapToWorld(AreaMapWindow* window, float x, float y); //44A4D0.
		void UpdateDraggedNote(LONG x, LONG y); //44AE30.
		void QueryMapBounds(); //44A730; original results remain local.
		void RefreshDownloadDescription(); //44EE00; owner in existing GS-enabled TU.
		void RefreshAlarm(); //44E7D0, two original maximum-health calls.
		void RefreshAlarmChoices(); //44B960, actual authored selection rows.
		void SetupMapWindows(); //44EF20, actual two AreaMapWindow owners.
		std::string SelectedMapTextureName(); //44D960.
		void AllocateMapMask(); //44AB80.
		BYTE RefreshMapMask(); //44AC50.
		BYTE SelectedExploredBounds(RECT& output); //44A470.
		void CenterMap(); //44D710.
		void ClampMapOrigin(BYTE clamp, LONG dx, LONG dy); //44D460.
		BYTE ConfigureNotePopup(AreaMapWindow* window, LONG x, LONG y); //450080.
		void ShowLilPip(BYTE shown, BYTE split); //44DEC0, RET8; no phantom EDI input.
		void RefreshButtons(); //44CEA0; populated task/download branches remain addressed.
		PipBoyInterface(const PipBoyInterface&);
		PipBoyInterface& operator=(const PipBoyInterface&);
	};
}
