#pragma once

#include "Entity.hpp"
#include "MapGrid.hpp"
#include "MapMetadata.hpp"
#include "MapNavigation.hpp"
#include "MapWater.hpp"
#include <set>
#include <vector>
#include <cstddef>

class OCTREE;
class Octree2Gfx;
class Gfx_Water;
namespace JE
{
	class Water;
	class AreaMap;
	class MapEffect;
	class VFX_EffectInstance;
	class MapSound;
	class Trigger;
	class Map
	{
	public:
		Map(); // 0x57B330; normal containers, original defaults.
		virtual ~Map(); // 0x57AC20.
		//Original Map table699FA4; destructor occupies +00.
		virtual DWORD GetUpdateInterval() const; //46C900, +04.
		virtual void SetUpdateInterval(DWORD interval); //56C470, +08.
		virtual BYTE IsPeriodic() const; //56C480, +0C.
		virtual BOOL NeedsUpdate() const; //56C490, +10; full EAX 0/1 result.
		virtual BYTE UpdateDue(DWORD time) const; //56C4C0, +14.
		virtual MapEntryPoint* NewEntryPoint(); //46CB30, +18.
		virtual Trigger* CreateTrigger(DWORD type); //56E580, +1C.
		virtual Entity* FindNearest(Entity* from,const std::string& role); //56E850, +20.
		virtual void GetCameraBounds(D3DXVECTOR3& low,D3DXVECTOR3& high) const; //56C510, +24.
		virtual void SetActiveMap(DWORD id); //56E970, +28.
		virtual void AddEntity(Entity* entity); //56DED0, +2C.
		virtual void RemoveEntity(Entity* entity); //56DF50, +30.
		virtual void Invalidate(Entity* entity); //56DFE0, +34.
		virtual BYTE ReadChunk(Common::Chunk& chunk, Resources::ResourceMemoryStream* stream); //56C770, +38.
		virtual BYTE ReadMap(Common::Chunk& chunk, Resources::ResourceMemoryStream* stream); //56C9D0, +3C; EMAP.
		virtual BYTE ReadEntryPoint(Common::Chunk& chunk, Resources::ResourceMemoryStream* stream); //573B00, +40.
		virtual BYTE ReadTrigger(Common::Chunk& chunk, Resources::ResourceMemoryStream* stream); //575050, +44.
		virtual BYTE ReadLegacyEntity(Common::Chunk& chunk, Resources::ResourceMemoryStream* stream); //573BF0, +48; EMEN.
		virtual BYTE ReadEntity(Common::Chunk& chunk, Resources::ResourceMemoryStream* stream); //573F30, +4C; EME2.
		virtual BYTE ReadWater(Common::Chunk& chunk, Resources::ResourceMemoryStream* stream); //575A40, +50.
		virtual BYTE ReadCamera(Common::Chunk& chunk, Resources::ResourceMemoryStream* stream); //56CCF0, +54.
		virtual BYTE ReadHeightFog(Common::Chunk& chunk, Resources::ResourceMemoryStream* stream); //56CD70, +58.
		virtual BYTE ReadNote(Common::Chunk& chunk, Resources::ResourceMemoryStream* stream); //5742B0, +5C.
		virtual BYTE ReadNavigation(Common::Chunk& chunk, Resources::ResourceMemoryStream* stream); //575180, +60.
		virtual BYTE ReadWaypoints(Common::Chunk& chunk, Resources::ResourceMemoryStream* stream); //576940, +64.
		virtual BYTE ReadSound(Common::Chunk& chunk, Resources::ResourceMemoryStream* stream); //576C30, +68.
		virtual BYTE ReadEffect(Common::Chunk& chunk, Resources::ResourceMemoryStream* stream); //578FB0, +6C.
		virtual void AssignEntityMap(Entity* entity); //56CF30, +70.
		virtual void ClearEntityMap(Entity* entity); //56CF50, +74.
		void NotifyEntityMoved(Entity* entity); //56C500, ordinary forwarder to virtual+34.
		void Create(const char* name); // 0x57B8A0.
		OCTREE* GetOctree(); // 0x56D960, resource stream callbacks.
		void InitializeGeometry(); // 0x579760.
		void PublishNotes(); //5712A0, publish pending notes then clear values.
		void AttachPendingEntities(); //5736C0, attach then reload/pop pending front.
		void StitchWaterTiles(float tileSize, float gridSize); //0x577290.
		void GenerateFogHeights(); // 0x5763C0.
		void ChangeState(DWORD state); // 0x56C6C0.
		void Update(DWORD time); // 0x56F0A0.
		void OnFirstUpdate(DWORD time); // 0x56DE70.
		void OnSecondUpdate(DWORD time); //56C760, actual state2 callback; argument unread.
		void OnUpdate(DWORD time); // 0x56E9E0.
		void UpdateClient(DWORD elapsed); //573730/574F10, elapsed milliseconds.
		void PackState(Net::Packet& packet); //56D7F0, normal-map start snapshot.
		void UnpackState(DWORD id, Net::Packet& packet); //572F10, invalid-map incoming snapshot.
		BYTE RunScript(const char* name, cell* result, int count, cell* args); // 0x56DD90.
		BYTE AddEntityHeightInfluence(Entity* entity); //56DB90, keyed AreaMap owner then party refresh.
		BYTE RemoveEntityHeightInfluence(Entity* entity); //56DD00.
		void QueueVisibleCell(DWORD entityID, LONG first, LONG row, BYTE value); //56C610->581320.
		BYTE SetExploredCell(LONG first, LONG row, BYTE value); //56C590, queue then reload AreaMap.
		void AttachEntity(Entity* entity); // 0x573160.
		void DetachEntity(Entity* entity); // 0x56E500.
		void RegisterTrigger(Trigger* trigger); // Populated owner boundary573230, list+C8.
		void UnregisterTrigger(Trigger* trigger); // Populated owner boundary56E750.
		BYTE ReadResource(const char* name); // 0x574F60.
		void ClearEntryPoints(); //571000.
		void SetEntryPoint(BYTE id, MapEntryPoint* point); //5731F0, ordinary RET8.
		void GetEntryPosition(BYTE entryID, LONG slot, D3DXVECTOR3& position,
			D3DXVECTOR3& rotation); //56F120, six authored slots; missing ID uses first entry.
		void ClearNavigation(); //571320.
		DWORD GetNavPointCount() const; //56D640.
		DWORD GetNavPointLinks(DWORD index, DWORD* links) const; //56DA40.
		DWORD GetNavPointPosition(DWORD index, D3DXVECTOR3& output) const; //56DAF0.
		DWORD GetNavPointOrientation(DWORD index, D3DXVECTOR3& output) const; //56DB40.
		DWORD GetWayPointPathCount() const; //56D660, actual map count.
		WayPointPath* GetWayPointPath(LONG index); //56E930, signed ordinal, borrowed.
		DWORD GetWayPointCount(const std::string& name); //571050, ordinary RET4.
		DWORD GetWayPointPosition(const std::string& name,DWORD index,D3DXVECTOR3& output); //571070, RET12.
		DWORD GetWayPointOrientation(const std::string& name,DWORD index,D3DXVECTOR3& output); //5710C0, RET12.
		WayPointPath* FindWayPointPath(const std::string& name); //56F860, borrowed ordinary RET4.
		void ClearWaypoints(); //574430.
		void ClearTriggers(); //56E770, unlink before delete.
		void UpdateTriggers(DWORD time); //56EAE2..56EB60, actual Update tail.
		void ClearWaterTiles(); //57ADE0..57AE16, original owning+D4 list.
		void ClearWaterObjects(); //56E7E0, unlink before virtual deletion.
		void ClearSounds(); //5724F0, delete values before clearing table nodes.
		void ClearMapEffects(); //572560, delete values before clearing table nodes.
		void FindEffects(const std::string& name,std::list<MapEffect*>& output); //574500, uppercase substring selection.
		void FindSounds(const std::string& name,std::list<MapSound*>& output); //574690, uppercase substring selection, appends.
		VisualEffect* AttachVisualEffect(const char* name, DWORD id, BYTE flag); // 0x574F20.
		VisualEffect* AttachVisualEffect(VFX_VEGInstance* instance, BYTE release, BYTE own, DWORD id, BYTE flag); // 0x573270.
		BYTE RemoveVisualEffect(VisualEffect* effect, BYTE force); // 0x573430.
		void ClearVisualEffects(); // 0x56F230.
		void AppendVisibleEffects(std::list<VFX_EffectInstance*>& output); //573550, borrowed unfinished children without bounds.
		void CopyHiddenMaterials(std::set<int>& output) const; // 0x572480; clear, then insert.
		void HideMaterial(Entity* owner, DWORD material); //57A620.
		void ShowMaterial(Entity* owner, DWORD material); //579CF0.
		void ApplyHeightFog(Octree2Gfx* renderer); // 0x56D210.
		void ApplyFogSettings(); // 0x56D440.
		void GetCameraFog(float& nearDistance, float& farDistance, DWORD& color,
			float& density, BYTE& enabled) const; //56C680.
		void SetCameraFog(float nearDistance, float farDistance, DWORD color,
			float density, BYTE enabled); //56D4B0.
		void ResetCameraFog(); //56D570; restore resource/constructor backups.
		void GetHeightFog(float& totalHeight, float& baseHeight, DWORD& color,
			float& maximumDensity, BYTE& enabled) const; //56C640.
		void SetHeightFog(float totalHeight, float baseHeight, DWORD color,
			float maximumDensity, BYTE enabled); //56D2B0.
		void ResetHeightFog(); //56D380.

		AreaMap* areaMap; BYTE ownsAreaMap; //Original+4,+8.
		std::string resourceName; unsigned int value28; //+C,+28.
		std::string octreeName; OCTREE* octree; //+2C,+48.
		std::string gridName, areaTextureName; //+4C,+68.
		MapGrid* terrainGrid; EntityGrid* entityGrid; //+84,+88.
		MapGrid* alternateTerrain; //+8C, Map owns; PathRequest borrows.
		std::string scriptName; Script_Small* script; BYTE scriptDisabled; //+90,+AC,+B0.
		DWORD idB4, state;
		std::map<BYTE,MapEntryPoint*> entryPoints; //+BC, owns current values.
		std::list<Trigger*> triggers; //+C8, owns registered triggers.
		std::list<MapWaterTile*> waterTiles; //+D4, owns records until stitching/destruction.
		std::map<DWORD,MapWaterConfiguration> waterConfigurations; //+E0, prior-size keys.
		std::list<Water*> waterObjects; //+EC, spatial tile owners.
		std::map<std::string,DWORD> waterGroups; //+F8, insertion-assigned indices.
		std::list<Gfx_Water*> waterSampleOwners; //+104, registered shared samples.
		BYTE color[3], ambient[4]; //+110,+113.
		unsigned int visibilityOwners; //+118.
		DWORD updateInterval, lastUpdateTime, nextUpdateTime; //+11C,+120,+124.
		std::list<Entity*> activeEntities; //+128.
		std::list<Entity*> entities; //+134, pending placements.
		std::string musicName; BYTE cameraConstraints; //+140,+15C.
		D3DXVECTOR3 cameraLow, cameraHigh; //+160,+16C.
		BYTE flag178; DWORD value17C; float value180,value184,value188,value18C,value190,value194,value198,value19C;
		DWORD value1A0; float value1A4,value1A8,value1AC; BYTE flag1B0,flag1B1;
		DWORD fogColor; float fogNear,fogFar,fogScale; BYTE fogFlagCopy;
		//ResetCameraFog deliberately converts this float color backup to an integer.
		float fogColorCopy,fogNearCopy,fogFarCopy,fogScaleCopy; BYTE flag1D8,flag1D9;
		std::list<AreaMapNote> mapNotes; //+1DC, owns normal note values.
		std::map<VFX_VEGInstance*,EntityEffectBinding> effects; //+1E8.
		std::map<DWORD,VisualEffect*> effectIDs; //+1F4.
		std::map<VFX_VEGInstance*,EntityEffectBinding> inactiveEffects; //+200.
		D3DXVECTOR3 wind; //+20C, previous-frame wind.
		std::map<int, std::set<DWORD> > hiddenMaterialOwners; //+218, material -> Entity IDs.
		std::vector<NavPoint*> navigation; //+224, owns points.
		std::map<std::string,WayPointPath> waypoints; //+234, owns point arrays.
		std::map<std::string,MapSound*> mapSounds; //+240, uppercase keys; owns values.
		std::map<std::string,MapEffect*> mapEffects; //+24C, uppercase keys; owns values.
	private:
		Map(const Map&);
		Map& operator=(const Map&);
	};

#if defined(_MSC_VER) && _MSC_VER == 1300
	//Actual VC7 member boundaries; implicit alignment remains unwritten.
	typedef char MapOriginalExtent[(sizeof(Map) == 0x258) ? 1 : -1];
	typedef char MapColorExtent[(sizeof(((Map*)0)->color) == 3) ? 1 : -1];
	typedef char MapAmbientExtent[(sizeof(((Map*)0)->ambient) == 4) ? 1 : -1];
#define JE_MAP_OFFSET(member, offset) typedef char MapOffset_##member[(offsetof(Map, member) == offset) ? 1 : -1];
	JE_MAP_OFFSET(areaMap, 0x4)
	JE_MAP_OFFSET(ownsAreaMap, 0x8)
	JE_MAP_OFFSET(resourceName, 0xC)
	JE_MAP_OFFSET(value28, 0x28)
	JE_MAP_OFFSET(octreeName, 0x2C)
	JE_MAP_OFFSET(octree, 0x48)
	JE_MAP_OFFSET(gridName, 0x4C)
	JE_MAP_OFFSET(areaTextureName, 0x68)
	JE_MAP_OFFSET(terrainGrid, 0x84)
	JE_MAP_OFFSET(entityGrid, 0x88)
	JE_MAP_OFFSET(alternateTerrain, 0x8C)
	JE_MAP_OFFSET(scriptName, 0x90)
	JE_MAP_OFFSET(script, 0xAC)
	JE_MAP_OFFSET(scriptDisabled, 0xB0)
	JE_MAP_OFFSET(idB4, 0xB4)
	JE_MAP_OFFSET(state, 0xB8)
	JE_MAP_OFFSET(entryPoints, 0xBC)
	JE_MAP_OFFSET(triggers, 0xC8)
	JE_MAP_OFFSET(waterTiles, 0xD4)
	JE_MAP_OFFSET(waterConfigurations, 0xE0)
	JE_MAP_OFFSET(waterObjects, 0xEC)
	JE_MAP_OFFSET(waterGroups, 0xF8)
	JE_MAP_OFFSET(waterSampleOwners, 0x104)
	JE_MAP_OFFSET(color, 0x110)
	JE_MAP_OFFSET(ambient, 0x113)
	JE_MAP_OFFSET(visibilityOwners, 0x118)
	JE_MAP_OFFSET(updateInterval, 0x11C)
	JE_MAP_OFFSET(lastUpdateTime, 0x120)
	JE_MAP_OFFSET(nextUpdateTime, 0x124)
	JE_MAP_OFFSET(activeEntities, 0x128)
	JE_MAP_OFFSET(entities, 0x134)
	JE_MAP_OFFSET(musicName, 0x140)
	JE_MAP_OFFSET(cameraConstraints, 0x15C)
	JE_MAP_OFFSET(cameraLow, 0x160)
	JE_MAP_OFFSET(cameraHigh, 0x16C)
	JE_MAP_OFFSET(flag178, 0x178)
	JE_MAP_OFFSET(value17C, 0x17C)
	JE_MAP_OFFSET(value180, 0x180)
	JE_MAP_OFFSET(value184, 0x184)
	JE_MAP_OFFSET(value188, 0x188)
	JE_MAP_OFFSET(value18C, 0x18C)
	JE_MAP_OFFSET(value190, 0x190)
	JE_MAP_OFFSET(value194, 0x194)
	JE_MAP_OFFSET(value198, 0x198)
	JE_MAP_OFFSET(value19C, 0x19C)
	JE_MAP_OFFSET(value1A0, 0x1A0)
	JE_MAP_OFFSET(value1A4, 0x1A4)
	JE_MAP_OFFSET(value1A8, 0x1A8)
	JE_MAP_OFFSET(value1AC, 0x1AC)
	JE_MAP_OFFSET(flag1B0, 0x1B0)
	JE_MAP_OFFSET(flag1B1, 0x1B1)
	JE_MAP_OFFSET(fogColor, 0x1B4)
	JE_MAP_OFFSET(fogNear, 0x1B8)
	JE_MAP_OFFSET(fogFar, 0x1BC)
	JE_MAP_OFFSET(fogScale, 0x1C0)
	JE_MAP_OFFSET(fogFlagCopy, 0x1C4)
	JE_MAP_OFFSET(fogColorCopy, 0x1C8)
	JE_MAP_OFFSET(fogNearCopy, 0x1CC)
	JE_MAP_OFFSET(fogFarCopy, 0x1D0)
	JE_MAP_OFFSET(fogScaleCopy, 0x1D4)
	JE_MAP_OFFSET(flag1D8, 0x1D8)
	JE_MAP_OFFSET(flag1D9, 0x1D9)
	JE_MAP_OFFSET(mapNotes, 0x1DC)
	JE_MAP_OFFSET(effects, 0x1E8)
	JE_MAP_OFFSET(effectIDs, 0x1F4)
	JE_MAP_OFFSET(inactiveEffects, 0x200)
	JE_MAP_OFFSET(wind, 0x20C)
	JE_MAP_OFFSET(hiddenMaterialOwners, 0x218)
	JE_MAP_OFFSET(navigation, 0x224)
	JE_MAP_OFFSET(waypoints, 0x234)
	JE_MAP_OFFSET(mapSounds, 0x240)
	JE_MAP_OFFSET(mapEffects, 0x24C)
#undef JE_MAP_OFFSET
#endif
}
