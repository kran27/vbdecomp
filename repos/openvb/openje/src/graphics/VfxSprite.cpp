#include "VfxSprite.hpp"
#include "GfxVector3.hpp"
#include "VfxLightningMesh.hpp"
#include "ParticleSystem.hpp"
#include "Camera.hpp"
#include "Viewport.hpp"
#include "TextureManager.hpp"
#include "SystemInterface.hpp"
#include "TextureResource.hpp"
#include "GraphicsScene.hpp"
#include "Octree.hpp"
#include "resources/ResourceMemoryStream.hpp"
#include <float.h>
#include <cmath>
#include <new>
#include <stdlib.h>
#include <string.h>
#include <stdexcept>

namespace
{
	float NormalizedTime(float time, float duration)
	{
		if (duration > 0)
		{
			float value = float(double(time) / duration);
			if (value >= 0) return value > 1 ? 1 : value;
		}
		return 0;
	}
	double Curve(BYTE overLifetime, float time, float duration, LONG waveform, float frequency, float phase)
	{
		double value = overLifetime && duration > 0 ? NormalizedTime(time, duration) : time;
		float argument = float(value * frequency + phase);
		return Gfx_ParticleWaveform(waveform,0,1,argument);
	}
	void WriteVertex(JE::VFX_LightningVertex& output, const D3DXVECTOR3& position,
		DWORD color, float u, float v)
	{
		output.x = position.x; output.y = position.y; output.z = position.z;
		output.color = color; output.u = u; output.v = v;
	}
}

namespace JE
{
	VFX_SpriteData::VFX_SpriteData()
	{
	}
	void VFX_SpriteData::Read(const VFX_ParameterHolder& parameters)
	{
		const VFX_Parameter* input;
		if ((input = parameters.Find("Texture")) != NULL)
		{
			texture.type = input->GetResourceType(); texture.name = input->GetResourceName();
		}
		if ((input = parameters.Find("TextureUVs")) != NULL) { const JE::VFX_TextureCoordinates converted = input->GetTextureUVs(); memcpy(textureUVs, &converted, sizeof(converted)); }
		if ((input = parameters.Find("Color")) != NULL) { const JE::VFX_ColorBytes converted = input->GetColor(); memcpy(color, converted.values, sizeof(converted.values)); }
		if ((input = parameters.Find("FinalColor")) != NULL) { const JE::VFX_ColorBytes converted = input->GetColor(); memcpy(finalColor, converted.values, sizeof(converted.values)); }
		if ((input = parameters.Find("Rotatable")) != NULL) rotatable = input->GetBool();
		if ((input = parameters.Find("Width")) != NULL) width = float(input->GetFloat());
		if ((input = parameters.Find("Height")) != NULL) height = float(input->GetFloat());
		if ((input = parameters.Find("RenderPhase")) != NULL) renderPhase = input->GetEnum();
		if ((input = parameters.Find("Modulate2x")) != NULL) modulate2x = input->GetBool();
		if ((input = parameters.Find("_DisableZBuffer")) != NULL) disableZBuffer = input->GetBool();
		if ((input = parameters.Find("AdjustColor")) != NULL) adjustColor = input->GetBool();
		if ((input = parameters.Find("AdjustColorOverLifetime")) != NULL) adjustColorOverLifetime = input->GetBool();
		if ((input = parameters.Find("ColorWaveform")) != NULL) colorWaveform = input->GetEnum();
		if ((input = parameters.Find("ColorFreq")) != NULL) colorFreq = float(input->GetFloat());
		if ((input = parameters.Find("ColorPhase")) != NULL) colorPhase = float(input->GetFloat());
		if ((input = parameters.Find("Alpha")) != NULL) alpha = BYTE(input->GetInteger());
		if ((input = parameters.Find("AdjustAlpha")) != NULL) adjustAlpha = input->GetBool();
		if ((input = parameters.Find("AdjustAlphaOverLifetime")) != NULL) adjustAlphaOverLifetime = input->GetBool();
		if ((input = parameters.Find("FinalAlpha")) != NULL) finalAlpha = BYTE(input->GetInteger());
		if ((input = parameters.Find("AlphaWaveform")) != NULL) alphaWaveform = input->GetEnum();
		if ((input = parameters.Find("AlphaFreq")) != NULL) alphaFreq = float(input->GetFloat());
		if ((input = parameters.Find("AlphaPhase")) != NULL) alphaPhase = float(input->GetFloat());
		if ((input = parameters.Find("AdjustScale")) != NULL) adjustScale = input->GetBool();
		if ((input = parameters.Find("AdjustScaleOverLifetime")) != NULL) adjustScaleOverLifetime = input->GetBool();
		if ((input = parameters.Find("InitialScaleX")) != NULL) initialScaleX = float(input->GetFloat());
		if ((input = parameters.Find("InitialScaleY")) != NULL) initialScaleY = float(input->GetFloat());
		if ((input = parameters.Find("FinalScaleX")) != NULL) finalScaleX = float(input->GetFloat());
		if ((input = parameters.Find("FinalScaleY")) != NULL) finalScaleY = float(input->GetFloat());
		if ((input = parameters.Find("ScaleWaveform")) != NULL) scaleWaveform = input->GetEnum();
		if ((input = parameters.Find("ScaleFreq")) != NULL) scaleFreq = float(input->GetFloat());
		if ((input = parameters.Find("ScalePhase")) != NULL) scalePhase = float(input->GetFloat());
	}
	VFX_Sprite::VFX_Sprite()
	{
		dirty = 1;
		D3DXMatrixIdentity(&transform);
		lastFieldOfView = -1;
		texture = 0;
		ownsTexture = 1;
		time = 0;
		meshes.resize(2,NULL);
		for (LONG index = LONG(meshes.size()); index > 0; --index)
		{
			VFX_LightningMesh* mesh = new VFX_LightningMesh;
			meshes[index-1] = mesh;
			mesh->instanceCount = 1;
			mesh->instances = new Gfx_MeshInstance[1];
			for (LONG i = 0; i < LONG(mesh->instanceCount); ++i)
			{
				mesh->instances[i].flag1D = 1;
				mesh->instances[i].flag1E = 1;
			}
			mesh->vertexCount = 4;
			mesh->vertices = new VFX_LightningVertex[4];
			mesh->indexCount = 4;
			mesh->indices = new WORD[4];
			mesh->packet = new Gfx_PacketData;
			mesh->flagC8 = 0;
		}
		currentBucket = 0;
		occlusionVisible = 0;
		occlusionChangedTime = 0;
	}
	VFX_Sprite::~VFX_Sprite()
	{
		for (std::vector<VFX_LightningMesh*>::iterator i = meshes.begin(); i != meshes.end(); ++i)
		{
			VFX_LightningMesh* owned = *i;
			if (owned->vertices) delete[] owned->vertices;
			if (owned->indices) delete[] owned->indices;
			if (owned->instances)
			{
				delete[] owned->instances;
				owned->instances = NULL;
			}
			if (owned->packet)
			{
				delete owned->packet;
				owned->packet = NULL;
			}
			delete owned;
		}
		if (texture && ownsTexture) g_TextureManager->Release(texture);
	}
	VFX_Sprite& VFX_Sprite::CopyFrom(const VFX_Sprite& other) //0x4F6390.
	{
		if (texture && ownsTexture) g_TextureManager->Release(texture);
		VFX_EffectInstance::CopyFrom(other);
		data = other.data;
		texture = other.texture;
		ownsTexture = 0;
		return *this;
	}
	void VFX_Sprite::SetData(const VFX_BaseData* input)
	{
		VFX_EffectInstance::SetData(input);
		data = *static_cast<const VFX_SpriteData*>(input);
		if (texture && ownsTexture)
		{
			g_TextureManager->Release(texture);
		}
		texture = Gfx_RequestTextureResource(data.texture.name.c_str(), UINT(-1), 3, 5, 0);
		ownsTexture = 1;
		dirty = 1;
	}
	void VFX_Sprite::Reset()
	{
		active = 0; occlusionVisible = 0; occlusionChangedTime = 0;
	}
	BYTE VFX_Sprite::GetFlag() const { return 0; }
	BYTE VFX_Sprite::SkipBounds() const { return 0; }
	void VFX_Sprite::Update(float value) { Update(value,VFX_UpdateContext()); }
	void VFX_Sprite::Update(float value, Gfx_ModelInstance* source, Gfx_ModelInstance* target, VFX_VEGInstance* group,
		const D3DXMATRIX* sourceTransform, const D3DXMATRIX* targetTransform)
	{
		VFX_EffectInstance::Update(value,source,target,group,sourceTransform,targetTransform);
		if (time != value) { time = value; dirty = 1; }
	}
	void VFX_Sprite::GetScale(float& x, float& y) const
	{
		x = y = 1;
		if (data.adjustScale)
		{
			double phase = Curve(data.adjustScaleOverLifetime,time,data.duration,
				data.scaleWaveform,data.scaleFreq,data.scalePhase);
			x = float((double(data.finalScaleX)-data.initialScaleX)*phase+data.initialScaleX);
			y = float((double(data.finalScaleY)-data.initialScaleY)*phase+data.initialScaleY);
		}
	}
	void VFX_Sprite::GetBounds(D3DXVECTOR3& low, D3DXVECTOR3& high) const
	{
		D3DXVECTOR3 center;
		memcpy(&center, &transform._41, sizeof(center));
		float phase;
		if (data.duration > 0)
		{
			phase = time / data.duration;
			if (phase < 0) phase = 0;
			else if (phase > 1) phase = 1;
		}
		else phase = 0;
		double value = phase;
		float x = 1, y = 1;
		if (data.adjustScale)
		{
			if (!data.adjustScaleOverLifetime || !(data.duration > 0)) value = time;
			float argument = float(value * data.scaleFreq + data.scalePhase);
			value = Gfx_ParticleWaveform(data.scaleWaveform, 0, 1, argument);
			x = float((data.finalScaleX - data.initialScaleX) * value + data.initialScaleX);
			y = float((data.finalScaleY - data.initialScaleY) * value + data.initialScaleY);
		}
		float width = data.width * scale * x;
		float height = data.height * scale * y;
		if (!(width > height)) width = height;
		float radius = width * 0.5f;
		float bound = center.x - radius;
		low.x = bound < FLT_MAX ? bound : FLT_MAX;
		bound = center.y - radius;
		low.y = bound < FLT_MAX ? bound : FLT_MAX;
		bound = center.z - radius;
		low.z = bound < FLT_MAX ? bound : FLT_MAX;
		bound = center.x + radius;
		high.x = bound > -FLT_MAX ? bound : -FLT_MAX;
		bound = center.y + radius;
		high.y = bound > -FLT_MAX ? bound : -FLT_MAX;
		bound = center.z + radius;
		high.z = bound > -FLT_MAX ? bound : -FLT_MAX;
	}
	BYTE VFX_Sprite::TestOcclusion(Gfx_Camera* camera) const //4F4EB0.
	{
		OCTREE* tree = GetSceneOctree();
		if (!tree) return 1;
		Gfx_Vector3f position = { transform._41, transform._42, transform._43 };
		Gfx_Vector3f origin = { camera->transform._41, camera->transform._42, camera->transform._43 };
		Gfx_Vector3f direction = { position.value[0] - origin.value[0],
			position.value[1] - origin.value[1], position.value[2] - origin.value[2] };
		const float distance = float(direction.Length());
		direction.Normalize();
		OctreeRay ray(origin.value, direction.value);
		OctreeGroundHit hit;
		if (!tree->QueryGroundRay(ray, &hit, 0, 0)) return 1;
		return !(hit.distance < distance);
	}
	void VFX_Sprite::Draw() { Draw(NULL); }
	void VFX_Sprite::Draw(Gfx_Viewport* viewport)
	{
		Gfx_Camera* camera = viewport ? viewport->activeCamera : g_GfxSystem->Camera();
		if (data.disableZBuffer)
		{
			BYTE visible = TestOcclusion(camera);
			if (occlusionChangedTime == 0 || time < occlusionChangedTime) occlusionChangedTime = time-0.25f;
			else if ((visible!=0)!=(occlusionVisible!=0)) occlusionChangedTime = time;
			occlusionVisible = visible;
			if (!visible && double(time)-occlusionChangedTime >= 0.25) return;
		}
		if (viewport != lastViewport)
		{
			dirty = 1;
			if (LONG(++currentBucket) >= LONG(meshes.size())) currentBucket = 0;
		}
		D3DXMATRIX cameraTransform = camera->transform;
		if (memcmp(&cameraTransform,&lastCameraTransform,sizeof(cameraTransform))
			|| camera->fieldOfViewY != lastFieldOfView) dirty = 1;
		if (dirty)
		{
			BuildGeometry(viewport ? viewport->activeCamera : g_GfxSystem->Camera());
			dirty = 0;
		}
		lastViewport = viewport;
		lastCameraTransform = cameraTransform;
		lastFieldOfView = camera->fieldOfViewY;
		VFX_LightningMesh* mesh = meshes[currentBucket];
		if (mesh->flagC8) g_GfxSystem->QueueMeshInstances(mesh->instances,viewport,mesh->instanceCount);
	}
	void VFX_Sprite::BuildGeometry(Gfx_Camera* camera)
	{
		VFX_LightningMesh& result = *meshes[currentBucket];
		result.flagC8 = 0;
		D3DXMATRIX cameraTransform;
		const D3DXMATRIX* axes = &transform;
		if (!data.rotatable)
		{
			cameraTransform = camera->transform;
			axes = &cameraTransform;
		}
		D3DXVECTOR3 right, up, forward;
		GfxGetMatrixAxes(*axes, right, up, forward);
		float normalizedTime = 0;
		if (data.duration > 0)
		{
			normalizedTime = time / data.duration;
			if (normalizedTime < 0) normalizedTime = 0;
			else if (normalizedTime > 1) normalizedTime = 1;
		}
		float red,green,blue;
		if (data.adjustColor)
		{
			double phase = normalizedTime;
			if (!data.adjustColorOverLifetime || !(data.duration > 0)) phase = time;
			float argument = float(phase * data.colorFreq + data.colorPhase);
			phase = Gfx_ParticleWaveform(data.colorWaveform, 0, 1, argument);
			red = float((LONG(data.finalColor[0])-data.color[0])*phase+data.color[0]);
			green = float((LONG(data.finalColor[1])-data.color[1])*phase+data.color[1]);
			blue = float((LONG(data.finalColor[2])-data.color[2])*phase+data.color[2]);
		}
		else { red=data.color[0]; green=data.color[1]; blue=data.color[2]; }
		float fade = 1;
		if (data.disableZBuffer)
		{
			double elapsed = double(time)-occlusionChangedTime;
			if (elapsed < 0.25) fade = float((occlusionVisible ? elapsed : 0.25-elapsed)*4.0);
		}
		double alpha = data.alpha;
		if (data.adjustAlpha)
		{
			double phase = normalizedTime;
			if (!data.adjustAlphaOverLifetime || !(data.duration > 0)) phase = time;
			float argument = float(phase * data.alphaFreq + data.alphaPhase);
			phase = Gfx_ParticleWaveform(data.alphaWaveform, 0, 1, argument);
			alpha = (LONG(data.finalAlpha)-data.alpha)*phase+data.alpha;
		}
		float normalizedAlpha = float(alpha*0.0039215689f*fade);
		DWORD color = DWORD(LONG(blue)) | (DWORD(LONG(green))<<8) | (DWORD(LONG(red))<<16)
			| (DWORD(LONG(double(normalizedAlpha)*255.0))<<24);
		float x=fade,y=fade;
		if (data.adjustScale)
		{
			double value = normalizedTime;
			if (!data.adjustScaleOverLifetime || !(data.duration > 0)) value = time;
			float argument = float(value * data.scaleFreq + data.scalePhase);
			float phase = float(Gfx_ParticleWaveform(data.scaleWaveform, 0, 1, argument));
			x = float(((double(data.finalScaleX)-data.initialScaleX)*phase+data.initialScaleX)*fade);
			y = float(((double(data.finalScaleY)-data.initialScaleY)*phase+data.initialScaleY)*fade);
		}
		double width = double(x)*(double(data.width)*scale);
		double height = double(data.height)*scale*y;
		float halfWidth = float(width*0.5),otherWidth = float(width-halfWidth);
		float halfHeight = float(height*0.5),otherHeight = float(height-halfHeight);
		D3DXVECTOR3 position(transform._41,transform._42,transform._43);
		WriteVertex(result.vertices[0],position-right*halfWidth-up*halfHeight,color,data.textureUVs[0],data.textureUVs[3]);
		WriteVertex(result.vertices[1],position+right*otherWidth-up*halfHeight,color,data.textureUVs[2],data.textureUVs[3]);
		WriteVertex(result.vertices[2],position-right*halfWidth+up*otherHeight,color,data.textureUVs[0],data.textureUVs[1]);
		WriteVertex(result.vertices[3],position+right*otherWidth+up*otherHeight,color,data.textureUVs[2],data.textureUVs[1]);
		result.indices[0]=0; result.indices[1]=2; result.indices[2]=1; result.indices[3]=3;
		Gfx_MeshData& mesh = result.mesh;
		mesh.value00=0; mesh.value04=DWORD(-1);
		mesh.vertices=result.vertices; mesh.value10=reinterpret_cast<DWORD>(result.indices);
		mesh.SetVertexDesc(0x142,D3DXGetFVFVertexSize(0x142),result.vertexCount);
		mesh.SetIndexDesc(2,result.indexCount);
		mesh.materialSet.count=0; mesh.materialSet.ResetGroups();
		if (mesh.materialSet.materials) { delete[] mesh.materialSet.materials; mesh.materialSet.materials=NULL; }
		Gfx_Primitive* primitive=mesh.SetPrimitiveCount(1);
		primitive->value00=0; primitive->type=D3DPT_TRIANGLESTRIP; primitive->value24=1;
		Gfx_Material material;
		material.style1=material.style2=19+(data.modulate2x!=0);
		if (data.rotatable) material.flagB9=0;
		material.textureHandles[0]=texture; material.textureHandles[1]=0;
		material.mode28=data.renderPhase; material.flagBC=0; material.flagBB=data.disableZBuffer==0;
		mesh.materialSet.materials[0]=material;
		primitive->value28=0; primitive->value2C=0; primitive->value30=2;
		result.packet->transform=&result.transform; result.packet->transformEnabled=1;
		mesh.materialSet.RebuildGroups(); mesh.value04=DWORD(-1);
		result.instances[0].SetMesh(&mesh);
		result.instances[0].value0C=reinterpret_cast<DWORD>(result.packet);
		result.flagC8=1;
	}
}

