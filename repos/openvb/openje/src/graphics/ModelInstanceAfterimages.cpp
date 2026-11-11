#include "ModelInstance.hpp"
#include "ModelData.hpp"
#include "ModelNode.hpp"
#include "ModelMeshPacket.hpp"
#include "SkinMeshPacket.hpp"
#include "AnimationInstance.hpp"
#include <new>
#include <string.h>

void Gfx_ModelNode::SetMaterialStyle(DWORD style)
{
	const DWORD attributeCount = DWORD(attributes.size());
	for (DWORD index = 0; index < attributeCount; ++index)
	{
		Gfx_NodeAttribute* attribute = attributes[index];
		Gfx_ModelMeshPkt* mesh = dynamic_cast<Gfx_ModelMeshPkt*>(attribute);
		if (mesh)
		{
			if (!mesh->overrideMaterials) mesh->CloneMaterials();
			Gfx_MaterialSet* values = mesh->overrideMaterials;
			for (DWORD material = 0; material < values->count; ++material)
			{
				Gfx_Material& value = values->materials[material];
				value.style2 = style;
			}
			mesh->overrideMaterials->RebuildGroups();
		}
		Gfx_SkinMeshPkt* skin = dynamic_cast<Gfx_SkinMeshPkt*>(attribute);
		if (skin) skin->SetMaterialStyle(style);
	}
	const DWORD childCount = DWORD(children.size());
	for (DWORD child = 0; child < childCount; ++child) children[child]->SetMaterialStyle(style);
}

void Gfx_ModelInstance::SetMaterialStyle(DWORD style) { root->SetMaterialStyle(style); }

namespace
{
	DWORD AfterimageColor(const BYTE* color, DWORD alpha)
	{
		return (alpha << 24) | (DWORD(color[1]) << 16) | (DWORD(color[2]) << 8) | color[3];
	}
}

void Gfx_ModelInstance::UpdateAfterimages(float elapsed)
{
	float aging = elapsed;
	if (elapsed < 0) { aging = 0; afterimageTime = 0; }
	for (std::list<Gfx_ModelAfterimage>::iterator i = afterimages.begin(); i != afterimages.end(); )
	{
		double remaining = double(i->remaining) - aging;
		if (!(remaining <= 0.0))
		{
			i->remaining = float(remaining);
			DWORD alpha = DWORD(LONG(double(i->color[0]) / afterimageLifetime * remaining));
			i->model->SetMaterialColors(AfterimageColor(i->color, alpha), i->flag, 1, NULL);
			i->model->SetRenderPhase(i->style);
			++i;
		}
		else
		{
			if (i->model) { delete i->model; i->model = NULL; }
			i = afterimages.erase(i);
		}
	}
	if (afterimagesEnabled)
	{
		double remaining = double(afterimageTime) - aging;
		if (!(remaining <= 0.0)) afterimageTime = float(remaining);
		else if (previousTransform)
		{
			Gfx_AnimationInstance* currentAnimation = animation;
			bool changed = memcmp(&transform, previousTransform, sizeof(transform)) != 0;
			if (!changed && currentAnimation && currentAnimation->playing)
			{
				const float currentTime = currentAnimation->currentTime;
				//4A08EF: the saved float goes through real VC7 _ftol; compare its low unsigned DWORD.
				const DWORD storedTime = DWORD(static_cast<__int64>(previousTime));
				changed = double(currentTime) != double(storedTime)
					|| currentAnimation->group != previousGroup || currentAnimation->animationIndex != previousIndex
					|| currentAnimation->value54 != previousSubAnimation;
			}
			if (changed)
			{
				Gfx_ModelInstance* snapshot = data->CreateModelInstance(); //Actual4A06E0; real registered owner.
				snapshot->SetTransform(*previousTransform);
				snapshot->SetMaterialStyle(20);
				for (DWORD material = 0; material < data->MaterialCount(); ++material)
				{
					Gfx_Material* source = GetMaterial(material);
					Gfx_Material* destination = snapshot->GetMaterial(material);
					if (source && destination && source->textureHandles[0])
						destination->textureHandles[0] = source->textureHandles[0];
				}
				if (currentAnimation && LONG(previousGroup) >= 0
					&& LONG(previousIndex) >= 0 && LONG(previousSubAnimation) >= 0)
				{
					Gfx_AnimationInstance* pose = new Gfx_AnimationInstance(*currentAnimation);
					pose->SelectAnimation(previousGroup, previousIndex, previousSubAnimation,
						0, 0, currentAnimation->GetLoopCount());
					pose->Update(previousTime);
					snapshot->SetAnimation(pose, 1);
				}
				Gfx_ModelAfterimage record;
				record.model = snapshot;
				record.remaining = afterimageLifetime;
				record.color[0] = flagC9 ? BYTE(color >> 24) : BYTE(192);
				record.color[1] = BYTE(color >> 16);
				record.color[2] = BYTE(color >> 8);
				record.color[3] = BYTE(color);
				record.flag = flagC8;
				record.style = value90;
				snapshot->SetMaterialColors(AfterimageColor(record.color, record.color[0]), record.flag, 1, NULL);
				snapshot->SetRenderPhase(value90);
				afterimageTime = afterimagePeriod;
				afterimages.push_back(record);
			}
		}
	}
	root->Update(elapsed);
	if (!previousTransform) previousTransform = new D3DXMATRIX;
	memcpy(previousTransform, &transform, sizeof(transform));
	if (animation)
	{
		previousTime = animation->currentTime;
		previousGroup = animation->group;
		previousIndex = animation->animationIndex;
		previousSubAnimation = animation->value54;
	}
	else
	{
		previousTime = -1;
		previousGroup = previousIndex = previousSubAnimation = DWORD(-1);
	}
}
