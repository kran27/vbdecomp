#include "ModelInstance.hpp"
#include "diagnostics/DebugOutput.hpp"
#include "ModelInfo.hpp"
#include "ModelCache.hpp"
#include "AnimationCache.hpp"
#include "graphics/ModelInstance.hpp"
#include "graphics/ModelData.hpp"
#include "graphics/ModelMeshPacket.hpp"
#include "graphics/SkinMeshPacket.hpp"
#include "graphics/AnimationInstance.hpp"
#include "graphics/SystemInterface.hpp"
#include <stdexcept>
#include <stdlib.h>
#include <string.h>

namespace JE
{
	std::string ModelInstance::GetSocketTextureName(DWORD slot, DWORD index, const char* variation) //566FB0.
	{
		return info->GetSocketTextureName(slot, index, variation);
	}
	std::string ModelInstance::GetSocketModelName(DWORD slot, DWORD variation) //5648D0.
	{
		return info->GetSocketModelName(slot, variation);
	}
	BYTE ModelInstance::FindSocketModel(DWORD slot, const std::string& requestedName, DWORD* variation)
	{
		return info->FindSocketModel(slot, requestedName, variation);
	}
	void ModelInstance::GetBounds(D3DXVECTOR3& minimum, D3DXVECTOR3& maximum)
	{
		if (!instances[0]) FatalError("Error: Attempt to get bounding box for an invalid model.  Model did not load properly or is invalid.\n");
		instances[0]->GetBounds(minimum, maximum);
	}

	void ModelInstance::SetShadow(BYTE value)
	{
		const DWORD count = g_pModelCache->slotCount;
		for (DWORD i = 0; i < count; ++i)
			if (instances[i]) instances[i]->SetShadow(value);
		for (std::list<ModelInstance*>::iterator i = attached.begin(); i != attached.end(); ++i)
			(*i)->SetShadow(value);
	}
	ModelInstance::ModelInstance(std::string inputName, DWORD, BYTE loadAnimations)
	{
		DWORD count = g_pModelCache->slotCount;
		instances = new Gfx_ModelInstance*[count];
		variations = new DWORD[count];
		textureNames = new std::string[count];
		char upperName[260];
		strcpy(upperName, inputName.c_str());
		_strupr(upperName);
		name = upperName;
		scale.x = scale.y = scale.z = 1.0f;
		info = NULL;
		animationObject = NULL;
		animation = NULL;
		textureHandle = 0;
		if (!inputName.empty())
		{
			Gfx_AnimationObject* object = g_AnimationCache->AcquireAnimObject(inputName, loadAnimations);
			if (object)
			{
				animation = new Gfx_AnimationInstance(object);
				animationObject = object;
			}
		}
		for (DWORD i = 0; i < count; ++i) { instances[i] = NULL; variations[i] = DWORD(-1); }
	}

	ModelInstance::~ModelInstance()
	{
		DetachAll();
		if (animationObject) g_AnimationCache->ReleaseAnimObject(animationObject);
		if (animation) { delete animation; animation = NULL; }
		const DWORD count = g_pModelCache->slotCount;
		for (DWORD i = 0; i < count; ++i)
		{
			if (instances[i])
			{
				delete instances[i]; instances[i] = NULL;
				if (info) info->Release(i, variations[i]);
				variations[i] = DWORD(-1);
			}
		}
		delete[] textureNames;
		delete[] variations;
		delete[] instances;
		info = NULL;
		if (textureHandle) g_GfxSystem->DeleteTexture(textureHandle);
	}

	void ModelInstance::SetSocket(DWORD slot, DWORD variation, const char* texture)
	{
		RemoveSocket(slot);
		ModelInfo* modelInfo = info;
		if (variation >= modelInfo->GetSocketModelCount(slot)) return;
		instances[slot] = modelInfo->Acquire(slot, variation);
		variations[slot] = variation;
		if (!instances[slot]) return;
		instances[slot]->SetPacketFlag1(1);
		instances[slot]->SetPacketFlag2(1);
		if (animation) instances[slot]->SetAnimation(animation, 0);
		if (slot)
		{
			instances[slot]->SetTransform(instances[0]->transform);
			DWORD count = instances[slot]->data->extraNameCount;
			if (count > 0)
			{
				std::string* name = instances[slot]->data->extraNames;
				do
				{
					instances[0]->EnableMaterial(*name, 0);
					++name;
				} while (--count);
			}
		}
		if (info->separateTextures == 1)
			SetSocketModel(slot, texture);
	}

	void ModelInstance::RemoveSocket(DWORD slot)
	{
		Gfx_ModelInstance* model = instances[slot];
		if (!model) return;
		if (model->data->extraNameCount > 0)
		{
			DWORD count = model->data->extraNameCount;
			std::string* name = model->data->extraNames;
			do
			{
				instances[0]->EnableMaterial(*name, 1);
				++name;
			} while (--count);
		}
		delete instances[slot];
		instances[slot] = NULL;
		info->Release(slot, variations[slot]);
		variations[slot] = DWORD(-1);
	}

	void ModelInstance::Detach(ModelInstance* child)
	{
		child->instances[0]->Detach();
	}

	void ModelInstance::DetachAll()
	{
		while (!attached.empty())
		{
			Detach(attached.front());
			attached.pop_front();
		}
	}

	void ModelInstance::GetPosition(D3DXVECTOR3& value)
	{
		Gfx_ModelInstance* model = instances[0];
		value.x = model->transform._41;
		value.y = model->transform._42;
		value.z = model->transform._43;
	}

	void ModelInstance::SetTransform(const D3DXMATRIX& value)
	{
		const DWORD count = g_pModelCache->slotCount;
		for (DWORD i = 0; i < count; ++i)
			if (instances[i]) instances[i]->SetTransform(value);
		for (std::list<ModelInstance*>::iterator i = attached.begin(); i != attached.end(); ++i)
			(*i)->SetTransform(value);
	}

	void ModelInstance::Attach(ModelInstance* child, DWORD bone)
	{
		if (!animation) return;
		child->instances[0]->AttachTo(instances[0], bone);
		D3DXVECTOR3 position;
		GetPosition(position);
		D3DXMATRIX matrix = instances[0]->transform;
		child->SetPosition(position.x, position.y, position.z);
		child->SetTransform(matrix);
		attached.push_back(child);
	}

	void ModelInstance::DetachChild(ModelInstance* child)
	{
		child->instances[0]->Detach();
		std::list<ModelInstance*>::iterator i = attached.begin();
		for (; i != attached.end(); ++i)
			if (*i == child) break;
		if (i != attached.end()) attached.erase(i);
	}

	void ModelInstance::SetPosition(float x, float y, float z)
	{
		const DWORD count = g_pModelCache->slotCount;
		for (DWORD i = 0; i < count; ++i)
			if (instances[i]) instances[i]->SetPosition(x, y, z);
		for (std::list<ModelInstance*>::iterator i = attached.begin(); i != attached.end(); ++i)
			(*i)->SetPosition(x, y, z);
	}

	void ModelInstance::SetRotation(const D3DXVECTOR3& degrees)
	{
		const DWORD count = g_pModelCache->slotCount;
		float yaw = degrees.x * 0.017453292f, pitch = degrees.y * 0.017453292f, roll = degrees.z * 0.017453292f;
		for (DWORD i = 0; i < count; ++i)
			if (instances[i]) instances[i]->SetEulerRotation(yaw, pitch, roll);
		for (std::list<ModelInstance*>::iterator i = attached.begin(); i != attached.end(); ++i)
			(*i)->SetRotation(degrees);
	}

	void ModelInstance::UpdateAnimation(DWORD milliseconds)
	{
		if (animation) animation->Update(float(milliseconds * 0.001f));
		for (std::list<ModelInstance*>::iterator i = attached.begin(); i != attached.end(); ++i)
			(*i)->UpdateAnimation(milliseconds);
	}

	void ModelInstance::ProcessAnimationCues(DWORD milliseconds)
	{
		if (animation) animation->ProcessCuesUntil(float(milliseconds * 0.001f));
		for (std::list<ModelInstance*>::iterator i = attached.begin(); i != attached.end(); ++i)
			(*i)->ProcessAnimationCues(milliseconds);
	}

	void ModelInstance::Submit(Gfx_Viewport* viewport, BYTE shadow, float value, DWORD color)
	{
		const DWORD count = g_pModelCache->slotCount;
		for (DWORD i = 0; i < count; ++i)
		{
			if (instances[i])
			{
				instances[i]->SetScale(scale.x, scale.y, scale.z);
				instances[i]->Submit(viewport, shadow, value, color);
			}
		}
		for (std::list<ModelInstance*>::iterator i = attached.begin(); i != attached.end(); ++i)
			(*i)->Submit(viewport, 0, 0.05f, 0xFF00FFFF);
	}
}
