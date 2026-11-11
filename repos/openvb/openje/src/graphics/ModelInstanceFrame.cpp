#include "ModelInstance.hpp"
#include "ModelMeshPacket.hpp"
#include "SkinMeshPacket.hpp"
#include "AnimationInstance.hpp"
#include "AnimationObject.hpp"
#include "ModelNode.hpp"
#include <string.h>
#include <stdexcept>

void Gfx_ModelInstance::ResetMaterialStates()
{
	Gfx_ModelMeshPkt* mesh = firstMesh;
	flagC8 = 0;
	flagC9 = 0;
	if (mesh && mesh->overrideMaterials)
	{
		mesh->overrideMaterials->ResetValues();
		mesh->overrideMaterials->RebuildGroups();
	}
	if (firstSkin) firstSkin->ResetMaterialValues();
}

void Gfx_ModelInstance::SetMaterialFlagBC(BYTE value)
{
	if (firstMesh) firstMesh->SetMaterialFlagBC(value);
	if (firstSkin) firstSkin->SetMaterialFlagBC(value);
}

D3DXMATRIX* Gfx_ModelInstance::GetWorldTransform(D3DXMATRIX& output, LONG semanticBone) const
{
	Gfx_ModelInstance* parentModel = parent;
	DWORD parentIndex = parentBone;
	D3DXMATRIX result;
	D3DXMatrixIdentity(&result);
	if (parentModel)
	{
		D3DXMATRIX parentTransform;
		result = *parentModel->GetWorldTransform(parentTransform, static_cast<LONG>(parentIndex));
	}
	else result = transform;
	if (semanticBone && animation)
	{
		// The original signed JGE212 only selects root0 for >=212. Negative
		// semantic indices are its unchecked invalid-memory domain, not clamped.
		LONG index = semanticBone < 212
			? animation->animationObject->semanticBoneIndices[semanticBone] : 0;
		const D3DXMATRIX* selected = animation->worldMatrices + index;
		if (selected)
		{
			D3DXMATRIX bone = *selected, combined;
			D3DXMatrixMultiply(&combined, &bone, &result);
			result = combined;
		}
	}
	output = result;
	return &output;
}

void Gfx_ModelNode::Update(float elapsed)
{
	for (std::vector<Gfx_NodeAttribute*>::iterator i = attributes.begin(); i != attributes.end(); ++i)
		(*i)->Update(elapsed);
	for (std::vector<Gfx_ModelNode*>::iterator i = children.begin(); i != children.end(); ++i)
		(*i)->Update(elapsed);
}

