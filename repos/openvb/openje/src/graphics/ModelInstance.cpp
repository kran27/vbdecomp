#include "ModelInstance.hpp"
#include "ModelData.hpp"
#include "ModelMeshPacket.hpp"
#include "SkinMeshPacket.hpp"
#include "AnimationInstance.hpp"
#include "Viewport.hpp"
#include "SystemInterface.hpp"
namespace JE
{
	void __cdecl FatalError(const char* format, ...); //49D2AB retains its continuation.
}

void Gfx_ModelInstance::EnableMaterialByIndex(LONG index, BYTE enabled)
{
	root->EnableMaterialByIndex(index, enabled);
}

void Gfx_ModelInstance::SetShadow(BYTE value)
{
	if (firstMesh) firstMesh->instance.flag1C = value;
	if (firstSkin) firstSkin->castsShadow = value;
}

void Gfx_ModelInstance::GetBounds(D3DXVECTOR3& minimum, D3DXVECTOR3& maximum)
{
	const Gfx_ModelNode& source = data->root;
	minimum = source.subtreeMinimum;
	maximum = source.subtreeMaximum;
	minimum.x = scale.x * minimum.x;
	minimum.y = minimum.y * scale.y;
	minimum.z = scale.z * minimum.z;
	maximum.x = scale.x * maximum.x;
	maximum.y = maximum.y * scale.y;
	maximum.z = scale.z * maximum.z;
}

Gfx_Material* Gfx_ModelInstance::GetMaterial(DWORD index)
{
	Gfx_Material* result = NULL;
	if (firstMesh) result = firstMesh->GetMaterial(index);
	if (firstSkin) result = firstSkin->GetMaterial(index);
	return result;
}

namespace
{
	struct ModelSortEntry
	{
		Gfx_ModelInstance* model;
	};

	D3DXMATRIX modelSortTransform; // Original707DB0; copied from the live camera.
	bool __cdecl operator<(const ModelSortEntry& a, const ModelSortEntry& b) // 0x49CBC0.
	{
		D3DXVECTOR3 left, right;
		left.x = a.model->transform._41;
		left.y = a.model->transform._42;
		left.z = a.model->transform._43;
		right.x = b.model->transform._41;
		right.y = b.model->transform._42;
		right.z = b.model->transform._43;
		D3DXVec3TransformCoord(&left, &left, &modelSortTransform);
		D3DXVec3TransformCoord(&right, &right, &modelSortTransform);
		return left.z > right.z;
	}
}

Gfx_ModelInstance::Gfx_ModelInstance(Gfx_ModelData526620* model)
	: parent(NULL), parentBone(0)
{
	scale = D3DXVECTOR3(1, 1, 1);
	mask = 1;
	data = model;
	animation = NULL;
	root = new Gfx_ModelNode;
	root->owner = data;
	Gfx_NodeAttribute* first = root->Clone(data->root);
	firstMesh = dynamic_cast<Gfx_ModelMeshPkt*>(first);
	firstSkin = dynamic_cast<Gfx_SkinMeshPkt*>(first);
	previousGroup = DWORD(-1);
	previousIndex = DWORD(-1);
	previousSubAnimation = DWORD(-1);
	afterimagesEnabled = 0;
	afterimageTime = 0;
	previousTransform = NULL;
	flagC8 = 0;
	flagC9 = 0;
	afterimageLifetime = 0.5f;
	afterimagePeriod = 0.04f;
	value90 = 1;
	valueA8 = 1;
	previousTime = -1;
	color = 0xFF808080;
	// ownsAnimation is unwritten at4A0400; first binding establishes it.
}

Gfx_ModelInstance::~Gfx_ModelInstance()
{
	if (data->references) --data->references;
	if (root) { delete root; root = NULL; }
	for (std::list<Gfx_ModelAfterimage>::iterator i = afterimages.begin(); i != afterimages.end(); ++i)
	{
		if (i->model) { delete i->model; i->model = NULL; }
	}
	for (std::map<DWORD, Gfx_ModelInstance*>::iterator i = attachedModels.begin(); i != attachedModels.end(); ++i)
		if (i->second) delete i->second;
	if (previousTransform) ::operator delete(previousTransform);
	if (ownsAnimation) delete animation;
}

void Gfx_ModelInstance::SetPacketFlag1(BYTE value)
{
	if (firstMesh) firstMesh->instance.flag1D = value;
	if (firstSkin) firstSkin->flag98 = value;
}

void Gfx_ModelInstance::SetPacketFlag2(BYTE value)
{
	if (firstMesh) firstMesh->instance.flag1E = value;
	if (firstSkin) firstSkin->flag99 = value;
}

void Gfx_ModelInstance::SetScale(float x, float y, float z)
{
	if (firstMesh)
	{
		Gfx_PacketData* packet = firstMesh->packet;
		packet->scale.x = x;
		packet->scale.y = y;
		packet->scale.z = z;
	}
	if (firstSkin) firstSkin->SetScale(x, y, z);
	scale.x = x; scale.y = y; scale.z = z;
}

void Gfx_ModelInstance::Detach()
{
	Gfx_AnimationInstance* currentAnimation = animation;
	if (currentAnimation) { currentAnimation->parent = NULL; currentAnimation->parentBone = 1; }
	Gfx_SkinMeshPkt* currentSkin = firstSkin;
	if (currentSkin)
	{
		currentSkin->parent = NULL;
		currentSkin->self = NULL;
		currentSkin->valueBC = 1;
	}
	parentBone = 1;
	parent = NULL;
}

void Gfx_ModelInstance::AttachTo(Gfx_ModelInstance* value, DWORD bone)
{
	Gfx_SkinMeshPkt* parentSkin = NULL;
	if (value->firstSkin) parentSkin = value->firstSkin;
	else JE::FatalError("Gfx_ModelInstance::AttachTo - Parent ModelInstance does not contain a SkinMeshPkt!\n");
	Detach();
	parent = value;
	parentBone = bone;
	Gfx_AnimationInstance* parentAnimation = value->animation;
	if (parentAnimation)
	{
		Gfx_AnimationInstance* childAnimation = animation;
		childAnimation->parent = parentAnimation; // Original requires the child's animation here.
		childAnimation->parentBone = bone;
	}
	if (firstSkin) firstSkin->AttachTo(parentSkin, bone);
}

void Gfx_ModelInstance::EnableMaterial(const std::string& name, BYTE enabled)
{
	if (firstMesh) firstMesh->EnableMaterial(name, enabled);
	if (firstSkin) firstSkin->EnableMaterial(name, enabled);
}

void Gfx_ModelInstance::Submit(Gfx_Viewport* viewport, BYTE shadow, float value, DWORD renderColor)
{
	root->SetTransform(transform); // Original replaces the root transform, not multiplies it.
	std::list<ModelSortEntry> models;
	for (std::map<DWORD, Gfx_ModelInstance*>::iterator i = attachedModels.begin(); i != attachedModels.end(); ++i)
	{
		const ModelSortEntry entry = { i->second };
		models.push_back(entry);
	}
	for (std::list<Gfx_ModelAfterimage>::iterator i = afterimages.begin(); i != afterimages.end(); ++i)
	{
		const ModelSortEntry entry = { i->model };
		models.push_back(entry);
	}
	Gfx_Camera* camera = viewport ? viewport->activeCamera : g_GfxSystem->Camera();
	modelSortTransform = camera->transform;
	models.sort();
	for (std::list<ModelSortEntry>::iterator i = models.begin(); i != models.end(); ++i)
		i->model->Submit(viewport, 0, 0.05f, 0xFFFF0000);
	root->Submit(mask, NULL, viewport, shadow, value, renderColor);
}
