#include "ModelNode.hpp"
#include "ModelMeshPacket.hpp"
#include "SkinMeshPacket.hpp"
#include "ParticleSystem.hpp"
#include "LineSystem.hpp"
#include "Water.hpp"
#include "WaterTile.hpp"
#include "SystemInterface.hpp"
#include <algorithm>
#include <string.h>

std::vector<Gfx_NodeAttribute*> Gfx_ModelNode::GetAttributes() const
{
	return attributes;
}

std::vector<Gfx_ModelNode*> Gfx_ModelNode::GetChildren() const
{
	return children;
}

void Gfx_ModelNode::EnableMaterialByIndex(LONG index, BYTE enabled)
{
	const DWORD attributeCount = DWORD(attributes.size());
	for (DWORD i = 0; i < attributeCount; ++i)
	{
		Gfx_NodeAttribute* attribute = attributes[i];
		Gfx_ModelMeshPkt* mesh = dynamic_cast<Gfx_ModelMeshPkt*>(attribute);
		if (mesh)
		{
			if (index == -1)
			{
				if (!mesh->overrideMaterials) mesh->CloneMaterials();
				Gfx_Primitive* primitive = mesh->instance.mesh->primitives;
				const DWORD count = mesh->instance.mesh->primitiveCount;
				for (DWORD p = 0; p < count; ++p)
				{
					const DWORD material = primitive[p].value00;
					BYTE* flags = mesh->instance.primitiveFlags;
					flags[material] = 1;
				}
			}
			else
			{
				if (!mesh->overrideMaterials) mesh->CloneMaterials();
				mesh->instance.primitiveFlags[index] = enabled;
			}
		}
		Gfx_SkinMeshPkt* skin = dynamic_cast<Gfx_SkinMeshPkt*>(attribute);
		if (skin)
		{
			if (index == -1) skin->EnableAllMaterials();
			else skin->materialEnabled[index] = enabled;
		}
	}
	const DWORD childCount = DWORD(children.size());
	for (DWORD i = 0; i < childCount; ++i) children[i]->EnableMaterialByIndex(index, enabled);
}

Gfx_ModelNode::Gfx_ModelNode()
	: rotation(0,0,0,1), position(0,0,0),
	minimum(0,0,0), maximum(0,0,0), subtreeMinimum(0,0,0), subtreeMaximum(0,0,0)
{
	owner = NULL;
	parent = NULL;
	dirty = 0;
	root = this;
	D3DXMatrixIdentity(&localTransform);
	D3DXMatrixIdentity(&worldTransform);
}

Gfx_ModelNode::~Gfx_ModelNode()
{
	const DWORD attributeCount = DWORD(attributes.size());
	for (DWORD i = 0; i < attributeCount; ++i) delete attributes[i];
	const DWORD childCount = DWORD(children.size());
	for (DWORD i = 0; i < childCount; ++i) delete children[i];
}

void Gfx_ModelNode::SetRotation(const D3DXQUATERNION& value)
{
	rotation = value;
	D3DXVECTOR3 axis;
	float angle;
	D3DXQuaternionToAxisAngle(&rotation, &axis, &angle);
	D3DXMatrixRotationAxis(&localTransform, &axis, angle);
	dirty = 1;
}

void Gfx_ModelNode::SetTransform(const D3DXMATRIX& value)
{
	const D3DXMATRIX snapshot(value);
	localTransform = snapshot;
	D3DXQuaternionRotationMatrix(&rotation, &localTransform);
	position.x = localTransform._41; position.y = localTransform._42; position.z = localTransform._43;
	dirty = 1;
}

Gfx_NodeAttribute* Gfx_ModelNode::Clone(const Gfx_ModelNode& source)
{
	Gfx_NodeAttribute* first = NULL;
	const DWORD attributeCount = DWORD(source.attributes.size());
	SetRotation(source.rotation);
	position = source.position;
	localTransform._41 = position.x; localTransform._42 = position.y; localTransform._43 = position.z;
	dirty = 1;
	for (DWORD i = 0; i < attributeCount; ++i)
	{
		Gfx_NodeAttribute* output = NULL;
		Gfx_NodeAttribute* attribute = source.attributes[i];
		Gfx_ModelMesh* mesh = dynamic_cast<Gfx_ModelMesh*>(attribute);
		if (mesh)
		{
			Gfx_ModelMeshPkt* cloned = new Gfx_ModelMeshPkt(mesh);
			attributes.push_back(cloned);
			cloned->owner = this;
			if (!first && !(cloned->source->value24 & 6)) first = cloned;
			continue;
		}
		else
		{
			Gfx_SkinMesh* skin = dynamic_cast<Gfx_SkinMesh*>(attribute);
			if (skin)
			{
				Gfx_SkinMeshPkt* cloned = new Gfx_SkinMeshPkt(skin);
				attributes.push_back(cloned);
				cloned->owner = this;
				if (!first) first = cloned;
				continue;
			}
			else if (Gfx_ParticleSystem* particles = dynamic_cast<Gfx_ParticleSystem*>(attribute))
			{
				Gfx_ParticleSystem* cloned = new Gfx_ParticleSystem;
				cloned->scale = particles->scale;
				cloned->Configure(particles->configuration);
				cloned->SetTexture(particles->texture, 0, 1);
				attributes.push_back(cloned);
				cloned->owner = this;
				cloned->owner = this;
				position = source.position;
				localTransform._41 = position.x;
				localTransform._42 = position.y;
				localTransform._43 = position.z;
				dirty = 1;
				continue; //49FD54; only configuration/scale and a borrowed texture are copied.
			}
			else if (Gfx_Water* water = dynamic_cast<Gfx_Water*>(attribute))
			{
				Gfx_Water* cloned = new Gfx_Water;
				cloned->SetRegistryName(water->registryName);
				cloned->SetTextureName(water->textureName);
				cloned->SetSecondaryTextureName(water->secondaryTextureName);
				cloned->tilesX = water->tilesX;
				cloned->tilesZ = water->tilesZ;
				for (std::list<Gfx_WaterTexturePatch>::const_iterator patch = water->texturePatches.begin();
					patch != water->texturePatches.end(); ++patch)
					cloned->AddTexturePatch(patch->x, patch->z, patch->textureName, patch->tileName);
				cloned->sampleHeight = water->sampleHeight;
				cloned->sampleWidth = water->sampleWidth;
				cloned->cellSize = water->cellSize;
				attributes.push_back(cloned);
				cloned->owner = this;
				cloned->owner = this;
				position = source.position;
				localTransform._41 = position.x;
				localTransform._42 = position.y;
				localTransform._43 = position.z;
				dirty = 1;
				cloned->PrepareSamples(); //49FEE9; after node publication, not before it.
				continue;
			}
			else if (Gfx_WaterTile* water = dynamic_cast<Gfx_WaterTile*>(attribute))
				output = water->Clone(this); //49FF19..4A00AD; does not become first mesh.
			else if (Gfx_LineSystem* lines = dynamic_cast<Gfx_LineSystem*>(attribute))
			{
				//4A00DD: default construction, not the broader LineSystem copy constructor.
				Gfx_LineSystem* cloned = new Gfx_LineSystem;
				const LONG count = lines->GetSegmentCount();
				for (LONG index = 0; index < count; ++index)
				{
					D3DXVECTOR3 from, to;
					LONG r1, g1, b1, a1, r2, g2, b2, a2;
					lines->GetSegment(index, &from.x, &from.y, &from.z,
						&to.x, &to.y, &to.z, &r1, &g1, &b1, &a1, &r2, &g2, &b2, &a2);
					cloned->AddSegment(from.x, from.y, from.z, to.x, to.y, to.z, r1, g1, b1, a1, r2, g2, b2, a2);
				}
				attributes.push_back(cloned);
				cloned->owner = this;
				cloned->owner = this;
				position = source.position;
				localTransform._41 = position.x;
				localTransform._42 = position.y;
				localTransform._43 = position.z;
				dirty = 1;
				continue; //The line branch never becomes the first mesh.
			}
			else continue; //The original skips unknown/non-render attributes.
		}
		attributes.push_back(output);
		output->owner = this;
	}
	const DWORD childCount = DWORD(source.children.size());
	for (DWORD i = 0; i < childCount; ++i)
	{
		Gfx_ModelNode* child = new Gfx_ModelNode;
		child->owner = owner;
		children.push_back(child);
		child->parent = this; child->root = root; child->owner = owner;
		Gfx_NodeAttribute* candidate = child->Clone(*source.children[i]);
		if (!first) first = candidate;
	}
	return first;
}

Gfx_NodeAttribute* Gfx_ModelNode::Initialize(DWORD kind, std::vector<DWORD>& textures, DWORD shadow)
{
	Gfx_NodeAttribute* first = NULL;
	const BYTE stored = BYTE(kind & 1);
	const DWORD attributeCount = DWORD(attributes.size());
	for (DWORD i = 0; i < attributeCount; ++i)
	{
		Gfx_NodeAttribute* attribute = attributes[i];
		Gfx_ModelMesh* mesh = dynamic_cast<Gfx_ModelMesh*>(attribute);
		if (mesh)
		{
			BYTE selected = 0;
			if (!first && !(mesh->value24 & 6)) { selected = 1; first = mesh; }
			mesh->Initialize(stored, BYTE(shadow & selected));
			const DWORD count = mesh->mesh.primitiveCount;
			for (DWORD material = 0; material < count; ++material)
			{
				DWORD texture = 0;
				if (material < mesh->mesh.primitiveCount)
					texture = mesh->mesh.materialSet.materials[material].textureHandles[0];
				const DWORD textureCount = DWORD(textures.size());
				DWORD textureIndex = 0;
				while (textureIndex < textureCount)
				{
					if (textures[textureIndex] == texture) goto nextStaticTexture;
					++textureIndex;
				}
				textures.push_back(texture);
				nextStaticTexture:;
			}
		}
		else
		{
			Gfx_SkinMesh* skin = dynamic_cast<Gfx_SkinMesh*>(attribute);
			if (skin)
			{
				if (!first) first = skin;
				skin->Register();
				const DWORD count = skin->materialCount;
				for (DWORD material = 0; material < count; ++material)
				{
					DWORD texture = skin->GetPrimaryTexture(material);
					const DWORD textureCount = DWORD(textures.size());
					DWORD textureIndex = 0;
					while (textureIndex < textureCount)
					{
						if (textures[textureIndex] == texture) goto nextSkinTexture;
						++textureIndex;
					}
					textures.push_back(texture);
					nextSkinTexture:;
				}
			}
		}
	}
	const DWORD childCount = DWORD(children.size());
	if (childCount)
	{
		for (DWORD i = 0; i < childCount; ++i)
		{
			Gfx_ModelNode* child = children[i];
			Gfx_NodeAttribute* candidate = child->Initialize(kind, textures, shadow);
			if (!first) first = candidate;
			if (!i)
			{
				memcpy(&subtreeMinimum, &child->subtreeMinimum, sizeof(subtreeMinimum));
				memcpy(&subtreeMaximum, &child->subtreeMaximum, sizeof(subtreeMaximum));
			}
			else
			{
				D3DXVECTOR3 childMinimum, childMaximum;
				memcpy(&childMinimum, &child->subtreeMinimum, sizeof(childMinimum));
				memcpy(&childMaximum, &child->subtreeMaximum, sizeof(childMaximum));
				if (childMinimum.x < subtreeMinimum.x) subtreeMinimum.x = childMinimum.x;
				if (childMaximum.x > subtreeMaximum.x) subtreeMaximum.x = childMaximum.x;
				if (childMinimum.y < subtreeMinimum.y) subtreeMinimum.y = childMinimum.y;
				if (childMaximum.y > subtreeMaximum.y) subtreeMaximum.y = childMaximum.y;
				if (childMinimum.z < subtreeMinimum.z) subtreeMinimum.z = childMinimum.z;
				if (childMaximum.z > subtreeMaximum.z) subtreeMaximum.z = childMaximum.z;
			}
		}
	}
	else
	{
		memcpy(&subtreeMinimum, &minimum, sizeof(subtreeMinimum));
		memcpy(&subtreeMaximum, &maximum, sizeof(subtreeMaximum));
	}
	return first;
}

void Gfx_ModelNode::SetAnimation(Gfx_AnimationInstance* animation)
{
	const DWORD attributeCount = DWORD(attributes.size());
	for (DWORD i = 0; i < attributeCount; ++i)
	{
		Gfx_NodeAttribute* attribute = attributes[i];
		Gfx_SkinMeshPkt* packet = dynamic_cast<Gfx_SkinMeshPkt*>(attribute);
		if (packet) packet->animation = animation;
	}
	const DWORD childCount = DWORD(children.size());
	for (DWORD i = 0; i < childCount; ++i) children[i]->SetAnimation(animation);
}

void Gfx_ModelNode::Submit(DWORD mask, const D3DXMATRIX* parentMatrix, Gfx_Viewport* viewport,
	BYTE shadow, float value, DWORD color)
{
	DWORD index;
	const D3DXMATRIX* changed = NULL;
	if (!dirty)
	{
		if (parentMatrix)
		{
			D3DXMatrixMultiply(&worldTransform, &localTransform, parentMatrix);
			changed = &worldTransform;
		}
	}
	else
	{
		if (parentMatrix) D3DXMatrixMultiply(&worldTransform, &localTransform, parentMatrix);
		else worldTransform = localTransform;
		changed = &worldTransform;
	}
	const DWORD attributeCount = DWORD(attributes.size());
	for (index = 0; index < attributeCount; ++index)
	{
		Gfx_NodeAttribute* attribute = attributes[index];
		Gfx_ModelMeshPkt* mesh = dynamic_cast<Gfx_ModelMeshPkt*>(attribute);
		if (mesh)
		{
			if (mask & mesh->source->value24)
			{
				Gfx_PacketData* packet = mesh->packet;
				packet->transformEnabled = 1;
				packet->transform = &worldTransform;
				mesh->Submit(viewport, shadow, value, color);
			}
		}
		else
		{
			Gfx_SkinMeshPkt* skin = dynamic_cast<Gfx_SkinMeshPkt*>(attribute);
			if (skin)
			{
				skin->transform = worldTransform;
				skin->Submit(viewport, shadow, value, color);
			}
			else if (Gfx_ParticleSystem* particles = dynamic_cast<Gfx_ParticleSystem*>(attribute))
				particles->Draw(viewport); //49D9F6; no mesh-mask test.
			else if (dynamic_cast<Gfx_Water*>(attribute))
				continue; //49DA0C: standalone Water is intentionally not submitted here.
			else if (Gfx_WaterTile* water = dynamic_cast<Gfx_WaterTile*>(attribute))
				water->Draw(viewport); //49DA34, no mesh-mask test on this branch.
			else if (Gfx_LineSystem* lines = dynamic_cast<Gfx_LineSystem*>(attribute))
				lines->Draw(viewport); //49DA59.
		}
	}
	const DWORD childCount = DWORD(children.size());
	for (DWORD i = 0; i < childCount; ++i)
		children[i]->Submit(mask, changed, viewport, shadow, value, color);
	dirty = 0;
}
