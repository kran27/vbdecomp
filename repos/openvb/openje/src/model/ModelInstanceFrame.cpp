#include "ModelInstance.hpp"
#include "ModelCache.hpp"
#include "graphics/ModelInstance.hpp"
#include "graphics/AnimationInstance.hpp"
#include "graphics/AnimationObject.hpp"

namespace JE
{
	void ModelInstance::ResetMaterialStates()
	{
		DWORD count = g_pModelCache->slotCount;
		for (DWORD slot = 0; slot < count; ++slot)
			if (instances[slot]) instances[slot]->ResetMaterialStates();
		for (std::list<ModelInstance*>::iterator i = attached.begin(); i != attached.end(); ++i)
			(*i)->ResetMaterialStates();
	}

	void ModelInstance::SetMaterialFlagBC(BYTE value)
	{
		DWORD count = g_pModelCache->slotCount;
		for (DWORD slot = 0; slot < count; ++slot)
			if (instances[slot]) instances[slot]->SetMaterialFlagBC(value);
		for (std::list<ModelInstance*>::iterator i = attached.begin(); i != attached.end(); ++i)
			(*i)->SetMaterialFlagBC(value);
	}

	Gfx_ModelInstance* ModelInstance::GetModel(DWORD slot)
	{
		return instances[slot];
	}

	BYTE ModelInstance::IsAttached()
	{
		Gfx_ModelInstance* model = instances[0];
		if (!model) return 0;
		return model->parent ? 1 : 0;
	}

	void ModelInstance::GetAttachedPosition(D3DXVECTOR3& output)
	{
		if (!IsAttached()) { GetPosition(output); return; }
		Gfx_ModelInstance* model = instances[0];
		if (!model || !model->animation) { GetPosition(output); return; }
		Gfx_ModelInstance* parent = model->parent;
		LONG bone = static_cast<LONG>(model->parentBone);
		if (!parent) { GetPosition(output); return; }
		output = D3DXVECTOR3(parent->transform._41, parent->transform._42, parent->transform._43);
		if (parent->animation)
		{
			LONG index = bone < 212 ? parent->animation->animationObject->semanticBoneIndices[bone] : 0;
			const D3DXMATRIX* selected = parent->animation->worldMatrices + index;
			D3DXMATRIX result, world;
			if (selected)
			{
				result = *selected;
				parent->GetWorldTransform(world, 0);
				D3DXMatrixMultiply(&result, &result, &world);
			}
			else result = *parent->GetWorldTransform(world, 0);
			output = D3DXVECTOR3(result._41, result._42, result._43);
		}
	}

	void ModelInstance::UpdateAfterimages(DWORD milliseconds)
	{
		DWORD count = g_pModelCache->slotCount;
		for (DWORD slot = 0; slot < count; ++slot)
			if (instances[slot]) instances[slot]->UpdateAfterimages(float(milliseconds * 0.001f));
		for (std::list<ModelInstance*>::iterator i = attached.begin(); i != attached.end(); ++i)
			(*i)->UpdateAfterimages(milliseconds);
	}
}
