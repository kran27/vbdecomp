#pragma once

#include "ModelNode.hpp"
#include "GraphicsBaseObject.hpp"
#include <list>
#include <cstddef>

class Gfx_ModelInstance;
class G3D_Model;
class G3D_Node;
class Gfx_SkinMesh;
class Gfx_Water;

class Gfx_ModelBone : public Gfx_BaseObject
{
public:
	Gfx_ModelBone() : value60(0) {}
	virtual ~Gfx_ModelBone(); //524330; derived vptr, name, then base lifetime.
	std::string name;
	DWORD value60;
};

// Actual nonvirtual model-data owner526620, using ordinary source containers.
class Gfx_ModelData526620
{
public:
	explicit Gfx_ModelData526620(DWORD kind);
	~Gfx_ModelData526620(); // 0x5CB370, named water release before node cleanup.
	void AddWater(Gfx_Water* water); //49F4A0; registration precedes name append.
	void SetExtraNameCount(DWORD count); //524390; delete old, publish count, allocate strings.
	void Initialize(DWORD shadow); //4A03A0; forwards the complete incoming word.
	DWORD MaterialCount() const; // 0x49CAF0; static primitive count before skin material count.
	LONG GetBoneCount() const; //4E0ED0, signed caller count of actual100B records.
	Gfx_ModelInstance* CreateModelInstance(); // 0x4A06E0; registered handle required.
	DWORD handle;
	std::string name;
	DWORD kind;
	Gfx_ModelNode root;
	DWORD references;
	std::vector<DWORD> textures;
	std::vector<Gfx_ModelBone> bones;
	Gfx_SkinMesh* firstSkin;
	Gfx_ModelMesh* firstMesh;
	DWORD extraNameCount;
	std::string* extraNames;
	std::list<std::string> environmentResources;
private:
	Gfx_ModelData526620(const Gfx_ModelData526620&);
	Gfx_ModelData526620& operator=(const Gfx_ModelData526620&);
};

#if defined(_MSC_VER) && _MSC_VER == 1300 && defined(_M_IX86)
//Only the observed bone-record/window layout is asserted here.
typedef char GfxModelBoneOriginalSize[(sizeof(Gfx_ModelBone) == 100) ? 1 : -1];
typedef char GfxModelBoneNameOffset[(offsetof(Gfx_ModelBone, name) == 0x44) ? 1 : -1];
typedef char GfxModelBoneValueOffset[(offsetof(Gfx_ModelBone, value60) == 0x60) ? 1 : -1];
typedef char GfxModelDataBonesOffset[(offsetof(Gfx_ModelData526620, bones) == 0x134) ? 1 : -1];
#endif

//Actual nonpolymorphic24B source/export owner; historical class spelling unknown.
class Gfx_G3D_Model
{
public:
	Gfx_G3D_Model(); //5240C0.
	~Gfx_G3D_Model(); //529D20; owned source before owned extra names.
	void LoadResource(const std::string& name); //529380.
	BYTE SetSource(G3D_Model* value, BYTE mode); //529450; borrowed source, raw mode return.
	Gfx_ModelData526620* Build(DWORD fvf, DWORD kind, BYTE textures, DWORD shadow, float scale); //529490.
	BYTE leftHanded, resources; //00/01; alignment padding stays unwritten.
	G3D_Model* source; //04.
	BYTE ownsSource; //08; alignment padding stays unwritten.
	DWORD boneCount; //0C, captured before bone traversal.
	DWORD extraNameCount; //10.
	std::string* extraNames; //14, owned independently of output model names.
private:
	void BuildNode(G3D_Node& source, Gfx_ModelNode& target, DWORD fvf, float scale,
		Gfx_ModelData526620& data, BYTE textures, DWORD shadow); //527FD0,7 words.
	Gfx_G3D_Model(const Gfx_G3D_Model&);
	Gfx_G3D_Model& operator=(const Gfx_G3D_Model&);
};

#if defined(_MSC_VER) && _MSC_VER == 1300 && defined(_M_IX86)
typedef char GfxG3DModelSize[(sizeof(Gfx_G3D_Model) == 24) ? 1 : -1];
#define GFX_G3D_MODEL_OFFSET(member, offset) \
	typedef char GfxG3DModel_##member[(offsetof(Gfx_G3D_Model, member) == offset) ? 1 : -1]
GFX_G3D_MODEL_OFFSET(leftHanded, 0x00);
GFX_G3D_MODEL_OFFSET(resources, 0x01);
GFX_G3D_MODEL_OFFSET(source, 0x04);
GFX_G3D_MODEL_OFFSET(ownsSource, 0x08);
GFX_G3D_MODEL_OFFSET(boneCount, 0x0C);
GFX_G3D_MODEL_OFFSET(extraNameCount, 0x10);
GFX_G3D_MODEL_OFFSET(extraNames, 0x14);
#undef GFX_G3D_MODEL_OFFSET
typedef char GfxModelExtraNameCountOffset[(offsetof(Gfx_ModelData526620, extraNameCount) == 0x14C) ? 1 : -1];
typedef char GfxModelExtraNamesOffset[(offsetof(Gfx_ModelData526620, extraNames) == 0x150) ? 1 : -1];
#endif

//Actual4B8190 optional callback frontdoor; statically initialized at6FAFF4.
typedef DWORD (__cdecl *Gfx_ModelDataLoadCallback)(const char* name, DWORD kind,
	DWORD textures, DWORD shadow, float scale, DWORD fvf);
extern Gfx_ModelDataLoadCallback g_GfxModelDataLoadCallback;
DWORD Gfx_LoadModelData(const char* name, DWORD kind, DWORD textures,
	DWORD shadow, float scale, DWORD fvf);
//Native529E30 resource entry; no output-pointer argument.
DWORD Gfx_LoadModelData(const std::string& name, DWORD kind, BYTE textures,
	DWORD shadow, float scale, DWORD fvf);
//529D80, borrowed source; output is published only after a successful Build.
DWORD Gfx_LoadModelData(G3D_Model* source, DWORD kind, DWORD textures, DWORD shadow,
	float scale, DWORD fvf, Gfx_ModelData526620** output, BYTE resources);
void Gfx_DeferModelDataRelease(DWORD handle); // 0x4B86E0; queues an in-use handle.
