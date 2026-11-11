#include "ModelData.hpp"
#include "ModelInstance.hpp"
#include "ModelDataManager.hpp"
#include "SkinRender.hpp"
#include "Water.hpp"
#include "WaterSampling.hpp"

Gfx_ModelBone::~Gfx_ModelBone()
{
}

LONG Gfx_ModelData526620::GetBoneCount() const //4E0ED0.
{
	return static_cast<LONG>(bones.size());
}

DWORD Gfx_ModelData526620::MaterialCount() const
{
	if (firstMesh) return firstMesh->mesh.primitiveCount;
	if (firstSkin) return firstSkin->materialCount;
	return 0;
}

Gfx_ModelData526620::Gfx_ModelData526620(DWORD value)
	: kind(value)
{
	handle = 0;
	root.owner = this;
	firstSkin = NULL;
	firstMesh = NULL;
	references = 0;
	extraNameCount = 0;
	extraNames = NULL;
}

Gfx_ModelData526620::~Gfx_ModelData526620()
{
	for (std::list<std::string>::iterator entry = environmentResources.begin();
		entry != environmentResources.end(); ++entry)
	{
		JE::RemoveWaterSampleOwners(*entry);
	}
	if (extraNames) { delete[] extraNames; extraNames = NULL; }
	// Normal member destruction then releases the root and name.
}

void Gfx_ModelData526620::AddWater(Gfx_Water* water)
{
	JE::RegisterWaterSampleOwner(water);
	environmentResources.push_back(water->registryName);
}

void Gfx_ModelData526620::SetExtraNameCount(DWORD count)
{
	if (extraNames) { delete[] extraNames; extraNames = NULL; }
	extraNameCount = count;
	extraNames = new std::string[count];
}

void Gfx_ModelData526620::Initialize(DWORD shadow)
{
	Gfx_NodeAttribute* first = root.Initialize(kind, textures, shadow);
	firstMesh = dynamic_cast<Gfx_ModelMesh*>(first);
	firstSkin = dynamic_cast<Gfx_SkinMesh*>(first);
}

Gfx_ModelInstance* Gfx_ModelData526620::CreateModelInstance()
{
	Gfx_ModelInstance* result = NULL;
	if (handle)
	{
		result = new Gfx_ModelInstance(this);
		if (result) ++references;
	}
	return result;
}

void __fastcall Gfx_DestroyModelData526620(Gfx_ModelData526620* payload)
{
	delete payload;
}
