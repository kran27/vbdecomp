#include "ParticleSystemOrdering.hpp"
#include "Camera.hpp"
#include "GfxVector3.hpp"
#include "D3DState.hpp"
#include "ModelInstance.hpp"
#include "ModelNode.hpp"
#include "SystemInterface.hpp"
#include "Viewport.hpp"
#include <math.h>
#include <new>
#include <stdlib.h>
#include <string.h>
#include <stdexcept>

namespace GfxParticleDraw
{
	D3DXMATRIX particleCamera, particleTransform;
}

namespace
{
	struct ParticleVertex
	{
		float x, y, z;
		DWORD color;
		float u, v;
	};



	static void CaptureRenderStates(LONG count, DWORD* values,
		const D3DRENDERSTATETYPE* states) // 0x513260, private renderer API.
	{
		for (LONG i = 0; i < count; ++i)
			g_D3D.GetRenderState(states[i], &values[i]);
	}

	void WriteVertex(ParticleVertex& vertex, const D3DXVECTOR3& position,
		DWORD color, float u, float v)
	{
		vertex.x = position.x; vertex.y = position.y; vertex.z = position.z;
		vertex.color = color; vertex.u = u; vertex.v = v;
	}
}


void Gfx_ParticleSystem::BuildGeometry(Gfx_Camera* camera) // 0x51C320.
{
	lines[currentBatchIndex].Clear();
	std::list<Gfx_ParticleBatch*>& pool = pools[currentBatchIndex];
	for (std::list<Gfx_ParticleBatch*>::iterator i = pool.begin(); i != pool.end(); ++i)
		delete *i;
	pool.clear();
	if (configuration.flags & 2)
		BuildModelGeometry(camera);
	else if (configuration.flags & 1)
		BuildPointGeometry(camera);
	else
		BuildQuadGeometry(camera);
	lines[currentBatchIndex].value104 = configuration.trailRenderPhase;
}

void Gfx_ParticleSystem::SortParticlesForDraw(std::list<Gfx_ParticleInfo>& selected)
{
	D3DXMatrixInverse(&particleCamera, NULL, &particleCamera);
	if (owner)
		particleTransform = static_cast<Gfx_ModelNode*>(owner)->localTransform;
	else
		D3DXMatrixIdentity(&particleTransform);
	D3DXMATRIX rotation;
	D3DXMatrixRotationYawPitchRoll(&rotation, configuration.rotationX,
		configuration.rotationY, configuration.rotationZ);
	D3DXMatrixMultiply(&particleTransform, &particleTransform, &rotation);
	D3DXMatrixMultiply(&particleTransform, &particleTransform, &secondTransform);
	selected.sort(); // Native ordinary list::sort uses the particle value ordering.
}

void Gfx_ParticleSystem::BuildQuadGeometry(Gfx_Camera* camera) // 0x51B1E0.
{
	Gfx_ParticleBatch& batch = *batches[currentBatchIndex];
	batch.flag94 = 0;
	std::list<Gfx_ParticleInfo>& selected = configuration.particleTrailType == 1
		? retiredParticles : particles;
	if (selected.empty()) return;
	D3DXMatrixInverse(&particleCamera, NULL, &particleCamera);
	if (owner)
		particleTransform = static_cast<Gfx_ModelNode*>(owner)->localTransform;
	else
		D3DXMatrixIdentity(&particleTransform);
	D3DXMATRIX sortRotation;
	D3DXMatrixRotationYawPitchRoll(&sortRotation, configuration.rotationX,
		configuration.rotationY, configuration.rotationZ);
	D3DXMatrixMultiply(&particleTransform, &particleTransform, &sortRotation);
	D3DXMatrixMultiply(&particleTransform, &particleTransform, &secondTransform);
	selected.sort(); //51ABF0, actual SDK list sort with the private value ordering.

	D3DXMATRIX rotation;
	D3DXVECTOR3 right, up, forward;
	if (!configuration.rotatableParticles)
	{
		D3DXMATRIX cameraTransform=camera->transform;
		GfxGetMatrixAxes(cameraTransform, right, up, forward);
	}
	LONG quadCount;
	if (configuration.particleTrailType==2 || configuration.particleTrailType==3)
	{
		quadCount=0;
		for (std::list<Gfx_ParticleInfo>::const_iterator i=selected.begin();i!=selected.end();++i)
		{
			Gfx_ParticleInfo particle=*i; //51B40B owns this counting-pass snapshot.
			if (particle.age<=particle.lifetime) ++quadCount;
		}
	}
	else quadCount=static_cast<LONG>(selected.size());
	if (quadCount>0)
	{
		const LONG requiredVertices=static_cast<LONG>(4u*static_cast<DWORD>(quadCount));
		if (requiredVertices>static_cast<LONG>(batch.vertexCapacity))
		{
			if (batch.vertices) { free(batch.vertices); batch.vertices=NULL; }
			LONG capacity=static_cast<LONG>(batch.vertexCapacity+20);
			if (requiredVertices>capacity) capacity=requiredVertices;
			batch.vertexCapacity=capacity;
			if (capacity>0) batch.vertices=operator new(sizeof(ParticleVertex)*capacity);
		}
		batch.vertexCount=requiredVertices;
	}
	else
	{
		if (configuration.particleTrailType!=2 && configuration.particleTrailType!=3) return;
		batch.vertexCount=0;
	}
	ParticleVertex* vertices = static_cast<ParticleVertex*>(batch.vertices);
	BYTE rotated = 0;
	LONG emittedQuadCount = 0;
	DWORD index = 0;
	for (std::list<Gfx_ParticleInfo>::iterator i = selected.begin(); i != selected.end(); ++i)
	{
		Gfx_ParticleInfo& particle = *i;
		if (particle.rotatable)
		{
			rotated = 1;
			if (!configuration.faceVelocityVectorUsed || particle.basisMatrix)
			{
				D3DXMatrixRotationYawPitchRoll(&rotation, particle.rotation.x,
					particle.rotation.y, particle.rotation.z);
				if (particle.basisMatrix)
					D3DXMatrixMultiply(&rotation, &rotation, &particle.matrix);
				D3DXVECTOR3 angles = particle.rotation;
				if (particle.spin.x != 0.0f) angles.x = particle.age * particle.spin.x;
				if (particle.spin.y != 0.0f) angles.y = particle.age * particle.spin.y;
				if (particle.spin.z != 0.0f) angles.z = particle.age * particle.spin.z;
				if (particle.spin.x != 0.0f || particle.spin.y != 0.0f || particle.spin.z != 0.0f)
				{
					D3DXMATRIX spin;
					D3DXMatrixRotationYawPitchRoll(&spin, angles.x, angles.y, angles.z);
					D3DXMatrixMultiply(&rotation, &rotation, &spin);
				}
				GfxGetMatrixAxes(rotation, right, up, forward);
			}
			else
			{
				float resistance = particle.resistance;
				if (fabs(resistance) < 0.000007999999979801942)
					resistance = (resistance >= 0.0f ? 1 : -1) * 0.000008f;
				const D3DXVECTOR3 acceleration = Acceleration();
				const float mass = particle.mass * scale;
				const double decay = exp(-(static_cast<double>(resistance) * particle.age));
				const double inverseResistance = 1.0 / resistance;
				D3DXVECTOR3 velocity;
				velocity.x = static_cast<float>(mass * acceleration.x * inverseResistance
					+ (particle.initialVelocity.x - mass * acceleration.x * inverseResistance) * decay);
				velocity.y = static_cast<float>(mass * acceleration.y * inverseResistance
					+ (particle.initialVelocity.y - mass * acceleration.y * inverseResistance) * decay);
				velocity.z = static_cast<float>(mass * acceleration.z * inverseResistance
					+ (particle.initialVelocity.z - mass * acceleration.z * inverseResistance) * decay);
				forward = -velocity;
				D3DXVec3Normalize(&forward, &forward);
				if (velocity.x == 0.0f && velocity.z == 0.0f)
				{
					const D3DXVECTOR3 vertical(0, 0, -1);
					D3DXVec3Cross(&right, &vertical, &forward);
					D3DXVec3Normalize(&right, &right);
					// This special branch uses the opposite cross order in51B7C2.
					D3DXVec3Cross(&up, &right, &forward);
				}
				else
				{
					const D3DXVECTOR3 vertical(0, 1, 0);
					D3DXVec3Cross(&right, &vertical, &forward);
					D3DXVec3Normalize(&right, &right);
					D3DXVec3Cross(&up, &forward, &right);
				}
				D3DXMATRIX basis(right.x, right.y, right.z, 0,
					up.x, up.y, up.z, 0, forward.x, forward.y, forward.z, 0,
					0, 0, 0, 1);
				D3DXMatrixRotationYawPitchRoll(&rotation, particle.rotation.x,
					particle.rotation.y, particle.rotation.z);
				D3DXMatrixMultiply(&rotation, &rotation, &basis);
				GfxGetMatrixAxes(rotation, right, up, forward);
			}
		}
		else if (rotated)
		{
			D3DXMATRIX cameraTransform=camera->transform;
			GfxGetMatrixAxes(cameraTransform, right, up, forward);
		}

		if ((configuration.particleTrailType==2 || configuration.particleTrailType==3)
			&& !(particle.age<=particle.lifetime))
		{
			if (configuration.particleTrailType==2) BuildAfterimageGeometry(particle,camera);
			else BuildRibbonGeometry(particle,camera);
			continue;
		}
		const float halfSize = particle.scale * particle.size * scale * 0.5f;
		++emittedQuadCount;
		const D3DXVECTOR3 a = right * halfSize, b = up * halfSize;
		const DWORD color = static_cast<DWORD>(particle.blue)
			| ((static_cast<DWORD>(particle.green)
			| (((static_cast<DWORD>(particle.alpha) << 8) | static_cast<DWORD>(particle.red)) << 8)) << 8);
		WriteVertex(vertices[index++], particle.position - a - b, color,
			configuration.textureUVsX, configuration.textureUVsW);
		WriteVertex(vertices[index++], particle.position + a - b, color,
			configuration.textureUVsZ, configuration.textureUVsW);
		WriteVertex(vertices[index++], particle.position - a + b, color,
			configuration.textureUVsX, configuration.textureUVsY);
		WriteVertex(vertices[index++], particle.position + a + b, color,
			configuration.textureUVsZ, configuration.textureUVsY);
		if (configuration.particleTrailType==2) BuildAfterimageGeometry(particle,camera);
		else if (configuration.particleTrailType==3) BuildRibbonGeometry(particle,camera);
	}

	const LONG requiredIndices=static_cast<LONG>(6u*static_cast<DWORD>(emittedQuadCount));
	if (requiredIndices<=0)
	{
		batch.vertexCount=batch.indexCount=0;
		return; //51C05E, ribbons may still have produced independent line geometry.
	}
	if (requiredIndices>static_cast<LONG>(batch.indexCapacity))
	{
		if (batch.indices) { free(batch.indices); batch.indices = NULL; }
		LONG capacity=static_cast<LONG>(batch.indexCapacity+30);
		if (requiredIndices > capacity) capacity = requiredIndices;
		batch.indexCapacity = capacity;
		if (capacity > 0) batch.indices = operator new(sizeof(WORD) * capacity);
	}
	batch.indexCount = requiredIndices;
	WORD* indices = static_cast<WORD*>(batch.indices);
	for (LONG q = 0; q < emittedQuadCount; ++q)
	{
		indices[6*q] = static_cast<WORD>(4*q+1);
		indices[6*q+1] = static_cast<WORD>(4*q);
		indices[6*q+2] = static_cast<WORD>(4*q+2);
		indices[6*q+3] = static_cast<WORD>(4*q+1);
		indices[6*q+4] = static_cast<WORD>(4*q+2);
		indices[6*q+5] = static_cast<WORD>(4*q+3);
	}
	Gfx_MeshData& mesh = batch.mesh;
	mesh.vertices = batch.vertices;
	mesh.value00 = 0;
	mesh.value04 = static_cast<DWORD>(-1);
	mesh.value10 = reinterpret_cast<DWORD>(batch.indices);
	mesh.SetVertexDesc(0x142, D3DXGetFVFVertexSize(0x142), batch.vertexCount);
	mesh.SetIndexDesc(2, batch.indexCount);
	mesh.materialSet.count = 0;
	mesh.materialSet.ResetGroups();
	delete[] mesh.materialSet.materials;
	mesh.materialSet.materials = NULL;
	Gfx_Primitive* primitive = mesh.SetPrimitiveCount(1);
	primitive->value00 = 0;
	primitive->type = D3DPT_TRIANGLELIST;
	primitive->value24 = 1;
	Gfx_Material material;
	material.style1 = material.style2 = 19 + (configuration.modulate2x != 0);
	material.mode24 = material.mode28 = configuration.renderPhase;
	material.flagBC = 0;
	material.flagBD = 1;
	if (rotated) material.flagB9 = 0;
	material.textureHandles[0] = texture;
	material.textureHandles[1] = 0;
	mesh.materialSet.materials[0] = material;
	primitive->value28 = 0;
	primitive->value30 = 2 * emittedQuadCount;
	batch.packet->transform = batch.matrices;
	batch.packet->transformEnabled = 1;
	mesh.materialSet.RebuildGroups();
	mesh.value04 = static_cast<DWORD>(-1);
	batch.instances[0].SetMesh(&mesh);
	batch.instances[0].value0C = reinterpret_cast<DWORD>(batch.packet);
	batch.flag94 = 1;
}

void Gfx_ParticleSystem::Draw(Gfx_Viewport* viewport) // 0x51C490.
{
	Gfx_Camera* camera = viewport ? viewport->activeCamera : g_GfxSystem->Camera();
	if (viewport != lastViewport)
	{
		geometryDirty = 1;
		++currentBatchIndex;
		if (static_cast<LONG>(currentBatchIndex) >= static_cast<LONG>(batches.size())) currentBatchIndex = 0;
	}
	if (!geometryDirty && (configuration.flags & 3) == 0)
	{
		D3DXMATRIX snapshot = camera->transform;
		if (memcmp(&snapshot, &transform, sizeof(snapshot)) || camera->fieldOfViewY != lastFieldOfView)
			geometryDirty = 1;
	}
	if (geometryDirty)
	{
		BuildGeometry(camera);
		geometryDirty = 0;
	}
	lastViewport = viewport;
	particleCamera = camera->transform;
	transform = particleCamera;
	lastFieldOfView = camera->fieldOfViewY;
	Gfx_ParticleBatch* batch = batches[currentBatchIndex];
	if (!(batch && batch->flag94) && pools[currentBatchIndex].empty()
		&& modelInstances.empty() && static_cast<LONG>(lines[currentBatchIndex].values108.size()) <= 0) return;
	for (std::list<Gfx_ParticleBatch*>::iterator i = pools[currentBatchIndex].begin();
		i != pools[currentBatchIndex].end(); ++i)
		g_GfxSystem->QueueMeshInstances((*i)->instances, viewport, (*i)->instanceCount);
	batch = batches[currentBatchIndex];
	if (batch && batch->flag94)
	{
		DWORD* saved = NULL;
		if (configuration.flags & 1)
		{
			const D3DRENDERSTATETYPE states[] = { D3DRS_POINTSIZE, D3DRS_POINTSIZE_MAX,
				D3DRS_ZWRITEENABLE, D3DRS_ALPHABLENDENABLE, D3DRS_SRCBLEND, D3DRS_DESTBLEND,
				D3DRS_POINTSPRITEENABLE, D3DRS_POINTSCALEENABLE, D3DRS_POINTSIZE,
				D3DRS_POINTSIZE_MIN, D3DRS_POINTSIZE_MAX, D3DRS_POINTSCALE_A,
				D3DRS_POINTSCALE_B, D3DRS_POINTSCALE_C };
			saved = static_cast<DWORD*>(operator new(sizeof(states)));
			CaptureRenderStates(14, saved, states);
			g_D3D.SetRenderState(D3DRS_ZWRITEENABLE, 0);
			g_D3D.SetRenderState(D3DRS_ALPHABLENDENABLE, 1);
			g_D3D.SetRenderState(D3DRS_SRCBLEND, 2);
			g_D3D.SetRenderState(D3DRS_DESTBLEND, 2);
			g_D3D.SetRenderState(D3DRS_POINTSPRITEENABLE, 1);
			g_D3D.SetRenderState(D3DRS_POINTSCALEENABLE, 1);
			g_D3D.SetRenderState(D3DRS_POINTSIZE, configuration.value250);
			g_D3D.SetRenderState(D3DRS_POINTSIZE_MIN, 0);
			g_D3D.SetRenderState(D3DRS_POINTSCALE_A, 0);
			g_D3D.SetRenderState(D3DRS_POINTSCALE_B, 0);
			g_D3D.SetRenderState(D3DRS_POINTSCALE_C, 0x3F800000);
		}
		g_GfxSystem->QueueMeshInstances(batch->instances, viewport, batch->instanceCount);
		// The actual caller frees the captured states without restoring them.
		if (configuration.flags & 1) free(saved);
	}
	for (std::list<Gfx_ModelInstance*>::iterator i = modelInstances.begin();
		i != modelInstances.end(); ++i)
		(*i)->Submit(viewport, 0, 0.05f, 0xFFFF0000);
	if (configuration.particleTrailType == 3)
		lines[currentBatchIndex].Draw(viewport); //50F4C0, actual line mesh queue.
}
