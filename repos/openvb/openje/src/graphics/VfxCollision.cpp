#include "VfxCollision.hpp"
#include <string.h>
#include "VfxParticles.hpp"
#include "ParticleSystem.hpp"
#include <float.h>
#include <cstdlib>
#include <stdexcept>
#include <math.h>
#include <new>

//521FA0; observed wide result/transport, historical nominal return type unknown.
double GfxDistanceSquared3D(const float* first, const float* second);

namespace
{
	//4EECF0 recovers the analytic epoch
	//after a collision has chosen a position and instantaneous velocity.
	void RestoreCollisionMotion(float mass, float resistance, float scale,
		const D3DXVECTOR3& acceleration, const D3DXVECTOR3& position,
		const D3DXVECTOR3& velocity, float elapsed,
		D3DXVECTOR3& initialPosition, D3DXVECTOR3& initialVelocity)
	{
		if (mass == 0)
		{
			initialPosition = position;
			initialVelocity = velocity;
			return;
		}
		if (fabs(resistance) < 0.000008)
			resistance = float((resistance >= 0 ? 1 : -1) * 0.000008);
		double inverse = 1.0 / resistance;
		float storedInverse = float(inverse);
		double decay = 1.0 - exp(-double(resistance) * elapsed);
		for (int axis = 0; axis < 3; ++axis)
		{
			float force = mass * scale * acceleration[axis];
			float velocityAtEpoch = velocity[axis] - acceleration[axis] * elapsed;
			initialVelocity[axis] = velocityAtEpoch;
			initialPosition[axis] = float(position[axis]
				- ((velocityAtEpoch - force * inverse) * storedInverse * decay
					+ force * inverse * elapsed));
		}
	}

	//4EEBC0; complete by-value stack protocol, helper ownership unresolved.
	int __stdcall OverlapsCollisionBox(JE::VFX_Vector dimensions, D3DXMATRIX transform,
		float x, float y, float z, float radius)
	{
		if (!(double(transform._41) - dimensions.xy.values[0] < double(x) + radius)) return 0;
		if (!(double(transform._41) + dimensions.xy.values[0] > double(x) - radius)) return 0;
		if (!(double(transform._42) - dimensions.xy.values[1] < double(y) + radius)) return 0;
		if (!(double(transform._42) + dimensions.xy.values[1] > double(y) - radius)) return 0;
		if (!(double(transform._43) - dimensions.z < double(z) + radius)) return 0;
		if (!(double(transform._43) + dimensions.z > double(z) - radius)) return 0;
		return 1;
	}

	//4EEC70; two copied points and rounded combined radius precede the real API.
	int __stdcall OverlapsCollisionSphere(float cx, float cy, float cz, float radius,
		float x, float y, float z, float particleRadius)
	{
		const float combined = radius + particleRadius;
		const float center[3] = {cx, cy, cz};
		const float point[3] = {x, y, z};
		const double distance = GfxDistanceSquared3D(center, point);
		return double(combined) * combined > distance;
	}

	void NormalizeCollisionVector(D3DXVECTOR3& value)
	{
		//522000, also used by Sprite/particle targeting. Unlike D3DX normalize,
		//the observed API divides by zero for a zero vector. Reuse CRT sqrt.
		const double x = value.x, y = value.y, z = value.z;
		const double inverse = 1.0 / sqrt(y*y + x*x + z*z);
		value.y = float(y*inverse);
		value.z = float(z*inverse);
		value.x = float(x*inverse);
	}

	D3DXVECTOR3 CollisionVelocity(const Gfx_ParticleInfo& particle,
		const D3DXVECTOR3& initialVelocity, const D3DXVECTOR3& acceleration, float scale, float age)
	{
		//4EF0CC uses stored particle age;4F018D uses time minus birth. Both
		//evaluate the drag law even for mass zero, unlike attract/repel.
		float resistance = particle.resistance;
		if (fabs(resistance) < 0.000008)
			resistance = float((resistance >= 0 ? 1 : -1) * 0.000008);
		float mass = scale * particle.mass;
		const double inverse = 1.0 / resistance;
		const double decay = exp(-double(resistance) * age);
		D3DXVECTOR3 result;
		for (int axis = 0; axis < 3; ++axis)
		{
			double force = double(acceleration[axis]) * mass;
			if (axis == 0) force = float(force); //Explicit first-component store.
			double terminal = force * inverse;
			result[axis] = float(terminal + (initialVelocity[axis] - terminal) * decay);
		}
		return result;
	}

	//Every branch returns0,1,2; the final realCRT rand range preserves that domain.
	int CollisionBoxAxis(const D3DXVECTOR3& position, const D3DXVECTOR3& center,
		const JE::VFX_Vector& dimensions, bool preferY)
	{
		float x = float(fabs(double(position.x) - center.x) / dimensions.xy.values[0]);
		float y = float(fabs(double(position.y) - center.y) / dimensions.xy.values[1]);
		//4EFEEB/4F06CB really subtract center.X for the Z ratio.
		float z = float(fabs(double(position.z) - center.x) / dimensions.z);
		if (preferY && y >= x && y >= z) return 1;
		if (x >= y && x >= z) return 0;
		if (!preferY && y >= x && y >= z) return 1;
		if (z >= x && z >= y) return 2;
		return 2 * rand() / 32767; //4B82E0(1,3), minus one; consumes real CRT RNG.
	}

	void SetCollisionNormal(JE::VFX_Collision& collision, Gfx_ParticleInfo& particle,
		D3DXVECTOR3 normal)
	{
		//4EF6B6 gates on the response's actual nonzero normal, not the flag alone.
		particle.rotatable = particle.basisMatrix = 1;
		D3DXMATRIX transform;
		collision.GetTransform(transform);
		transform._41 = transform._42 = transform._43 = 0;
		D3DXVec3TransformCoord(&normal, &normal, &transform);
		D3DXVECTOR3 forward = -normal, right, up;
		D3DXVec3Normalize(&forward, &forward);
		if (normal.x == 0 && normal.z == 0)
		{
			const D3DXVECTOR3 axis(0,0,-1);
			D3DXVec3Cross(&right, &forward, &axis);
			D3DXVec3Normalize(&right, &right);
			D3DXVec3Cross(&up, &right, &forward);
		}
		else
		{
			const D3DXVECTOR3 axis(0,1,0);
			D3DXVec3Cross(&right, &axis, &forward);
			D3DXVec3Normalize(&right, &right);
			D3DXVec3Cross(&up, &forward, &right);
		}
		D3DXMATRIX& output = particle.matrix;
		output._11=right.x; output._12=right.y; output._13=right.z; output._14=0;
		output._21=up.x; output._22=up.y; output._23=up.z; output._24=0;
		output._31=forward.x; output._32=forward.y; output._33=forward.z; output._34=0;
		output._41=output._42=output._43=0; output._44=1;
	}
}

namespace JE
{
	VFX_CollisionData::VFX_CollisionData()
	{ }
	void VFX_CollisionData::Read(const VFX_ParameterHolder& parameters)
	{
		const VFX_Parameter* input;
		if((input=parameters.Find("CollisionShape"))!=NULL) collisionShape=input->GetEnum();
		if((input=parameters.Find("CollisionType"))!=NULL) collisionType=input->GetEnum();
		if((input=parameters.Find("Dimensions"))!=NULL) { const JE::VFX_Vector converted = input->GetVector3(); memcpy(&dimensions, &converted, sizeof(converted)); }
		if((input=parameters.Find("Radius"))!=NULL) radius=float(input->GetFloat());
		if((input=parameters.Find("Power"))!=NULL) power=float(input->GetFloat());
		if((input=parameters.Find("ShowShape"))!=NULL) showShape=input->GetBool();
	}
	VFX_Collision::VFX_Collision()
		: velocity(0,0,0)
	{
		D3DXMatrixIdentity(&transform);
		previousPosition = NULL;
		previousTime = NULL;
		value168 = NULL;
	}
	VFX_Collision::~VFX_Collision()
	{
		if(previousPosition) delete previousPosition;
		if(previousTime) delete previousTime;
		if(value168) ::operator delete(value168);
	}
	VFX_Collision& VFX_Collision::CopyFrom(const VFX_Collision& other) //0x4F1120.
	{
		VFX_EffectInstance::CopyFrom(other);
		data = other.data;
		transform = other.transform;
		return *this;
	}
	void VFX_Collision::SetData(const VFX_BaseData* input)
	{ VFX_EffectInstance::SetData(input); data=*static_cast<const VFX_CollisionData*>(input); }
	void VFX_Collision::Reset() { active=0; }
	BYTE VFX_Collision::GetFlag() const { return 0; }
	BYTE VFX_Collision::SkipBounds() const { return 0; }
	void VFX_Collision::GetBounds(D3DXVECTOR3& low,D3DXVECTOR3& high) const
	{
		low.x = FLT_MAX; low.y = FLT_MAX; low.z = FLT_MAX;
		high.x = -FLT_MAX; high.y = -FLT_MAX; high.z = -FLT_MAX;
		const float x = transform._41, y = transform._42;
		const double z = transform._43;
		float minimum, maximum, maximumZ;
		switch (data.collisionShape)
		{
		case 1:
		{
			const double extent = double(data.radius) * scale;
			minimum = x - extent;
			low.x = minimum < low.x ? minimum : low.x;
			minimum = y - extent;
			low.y = minimum < low.y ? minimum : low.y;
			minimum = z - extent;
			low.z = minimum < low.z ? minimum : low.z;
			maximum = x + extent;
			high.x = maximum > high.x ? maximum : high.x;
			maximum = y + extent;
			high.y = maximum > high.y ? maximum : high.y;
			maximumZ = z + extent;
			break;
		}
		case 0:
		{
			D3DXVECTOR3 dimensions;
			memcpy(&dimensions, &data.dimensions, sizeof(dimensions));
			const double extentX = double(dimensions.x) * scale;
			const float extentY = dimensions.y * scale;
			const float extentZ = dimensions.z * scale;
			minimum = x - extentX;
			low.x = minimum < low.x ? minimum : low.x;
			minimum = y - extentY;
			low.y = minimum < low.y ? minimum : low.y;
			minimum = z - extentZ;
			low.z = minimum < low.z ? minimum : low.z;
			maximum = x + extentX;
			high.x = maximum > high.x ? maximum : high.x;
			maximum = y + extentY;
			high.y = maximum > high.y ? maximum : high.y;
			maximumZ = z + extentZ;
			break;
		}
		default: return;
		}
		high.z = maximumZ > high.z ? maximumZ : high.z;
	}
	void VFX_Collision::Update(float time) { Update(time, VFX_UpdateContext()); }
	void VFX_Collision::Update(float time, Gfx_ModelInstance* source, Gfx_ModelInstance* target, VFX_VEGInstance* group,
		const D3DXMATRIX* sourceTransform, const D3DXMATRIX* targetTransform)
	{
		VFX_EffectInstance::Update(time,source,target,group,sourceTransform,targetTransform);
		if (previousTime)
		{
			float elapsed = time - *previousTime;
			if (elapsed > 0)
			{
				*previousTime = time;
				if (previousPosition)
				{
					Gfx_Vector3f position = {{transform._41, transform._42, transform._43}};
					const Gfx_Vector3f difference = position.Subtract(*previousPosition);
					velocity.x = difference.value[0];
					velocity.y = difference.value[1];
					velocity.z = difference.value[2];
					velocity *= 1.0f / elapsed;
				}
				else
				{
					Gfx_Vector3f* position = new Gfx_Vector3f;
					if (position)
					{
						position->value[0] = 0;
						position->value[1] = 0;
						position->value[2] = 0;
					}
					previousPosition = position;
				}
				const Gfx_Vector3f position = {{transform._41, transform._42, transform._43}};
				*previousPosition = position;
			}
		}
		else
		{
			previousTime = new float;
			*previousTime = time;
		}
	}
	BYTE VFX_Collision::IntersectsParticle(float x, float y, float z, float diameter) const
	{
		switch (data.collisionShape)
		{
		case 0:
		{
			VFX_Vector dimensions = data.dimensions;
			dimensions.xy.values[0] *= scale;
			dimensions.xy.values[1] *= scale;
			dimensions.z *= scale;
			const float radius = diameter * 0.5f;
			return OverlapsCollisionBox(dimensions, transform, x, y, z, radius);
		}
		case 1:
		{
			const float particleRadius = diameter * 0.5f;
			const float radius = data.radius * scale;
			return OverlapsCollisionSphere(transform._41, transform._42, transform._43,
				radius, x, y, z, particleRadius);
		}
		default: return 0;
		}
	}
	BYTE VFX_Collision::ApplyParticle(Gfx_ParticleSystem& particles, Gfx_ParticleInfo& particle,
		float diameter, float time, VFX_VEGInstance&, D3DXVECTOR3& outputPosition,
		BYTE faceNormal, BYTE trigger)
	{
		if (!IntersectsParticle(particle.position.x, particle.position.y, particle.position.z, diameter)) return 0;
		const float elapsed = time - particle.birthTime;
		outputPosition.x = 0; outputPosition.y = 0; outputPosition.z = 0;
		D3DXVECTOR3 normal(0,0,0);
		bool normalTail = true;
		if (data.collisionShape == 0)
		{
			if (data.collisionType == 2)
			{
				particle.lifetime = 0;
				if (trigger)
				{
					D3DXVECTOR3 center(transform._41,transform._42,transform._43);
					outputPosition = particle.position;
					int axis = CollisionBoxAxis(particle.position, center, data.dimensions, true);
					//Select the live component after CollisionBoxAxis, whose result is0..2.
					double extent = double(scale) * (axis == 0 ? data.dimensions.xy.values[0]
						: axis == 1 ? data.dimensions.xy.values[1] : data.dimensions.z);
					outputPosition[axis] = float(particle.position[axis] - center[axis] > 0
						? extent + double(diameter)*0.5 + center[axis]
						: center[axis] - extent - double(diameter)*0.5);
				}
				normalTail = false;
			}
			else if (data.collisionType >= 0 && data.collisionType <= 4)
			{
				D3DXMATRIX inverseTransform;
				if (D3DXMatrixInverse(&inverseTransform, NULL, &transform))
				{
					D3DXVECTOR3 position = particle.position;
					D3DXVECTOR3 velocityAtEpoch = particle.initialVelocity;
					D3DXVECTOR3 acceleration = particles.Acceleration();
					D3DXVECTOR3 colliderVelocity = velocity;
					D3DXVec3TransformCoord(&position, &position, &inverseTransform);
					D3DXVECTOR3 center(transform._41,transform._42,transform._43);
					if (data.collisionType < 2)
						D3DXVec3TransformCoord(&center, &center, &inverseTransform);
					inverseTransform._41 = inverseTransform._42 = inverseTransform._43 = 0;
					D3DXVec3TransformCoord(&velocityAtEpoch, &velocityAtEpoch, &inverseTransform);
					D3DXVec3TransformCoord(&acceleration, &acceleration, &inverseTransform);
					D3DXVec3TransformCoord(&colliderVelocity, &colliderVelocity, &inverseTransform);
					D3DXVECTOR3 currentVelocity;
					if (data.collisionType < 2)
					{
						int axis = CollisionBoxAxis(position, center, data.dimensions, false);
						normal[axis] = axis == 0 || position[axis] - center[axis] > 0 ? 1.0f : -1.0f;
						//Select the live component after CollisionBoxAxis, whose result is0..2.
					double extent = double(scale) * (axis == 0 ? data.dimensions.xy.values[0]
						: axis == 1 ? data.dimensions.xy.values[1] : data.dimensions.z);
						position[axis] = float(position[axis] - center[axis] > 0
							? extent + double(diameter)*0.5 + center[axis]
							: center[axis] - extent - double(diameter)*0.5);
						NormalizeCollisionVector(normal);
						D3DXVECTOR3 localNormal = normal;
						D3DXMATRIX rotation = transform;
						rotation._41 = rotation._42 = rotation._43 = 0;
						D3DXVec3TransformCoord(&normal, &normal, &rotation);
						currentVelocity = CollisionVelocity(particle, velocityAtEpoch, acceleration, scale, elapsed);
						if (previousPosition && particle.birthTime < time)
						{
							float projection = -D3DXVec3Dot(&colliderVelocity, &localNormal);
							currentVelocity += localNormal * projection;
						}
						currentVelocity *= data.power * 0.0099999998f;
						const double vx=currentVelocity.x, vy=currentVelocity.y, vz=currentVelocity.z;
						float speed = float(sqrt(vx*vx + vy*vy + vz*vz));
						if (speed != 0) NormalizeCollisionVector(currentVelocity);
						D3DXVECTOR3 projection = localNormal * D3DXVec3Dot(&localNormal, &currentVelocity);
						D3DXVECTOR3 tangent = currentVelocity - projection;
						float reflect = data.collisionType == 1 ? 0.0f : 1.0f;
						currentVelocity = tangent - projection * reflect;
						if (currentVelocity.x != 0 || currentVelocity.y != 0 || currentVelocity.z != 0)
						{
							NormalizeCollisionVector(currentVelocity);
							currentVelocity *= speed;
						}
						outputPosition = position;
						D3DXVec3TransformCoord(&outputPosition, &outputPosition, &transform);
					}
					else
					{
						currentVelocity = velocityAtEpoch + acceleration * elapsed;
						D3DXVECTOR3 impulse = data.collisionType == 3 ? -position : position;
						impulse *= data.power * 0.0099999998f;
						currentVelocity += impulse;
						//4F0BC0 copies the inverse-local position, not the epoch or
						//world position. Singular inverse leaves the wrapper's zero.
						outputPosition = position;
					}
					D3DXVECTOR3 initialPosition, initialVelocity;
					RestoreCollisionMotion(particle.mass, particle.resistance, scale, acceleration,
						position, currentVelocity, elapsed, initialPosition, initialVelocity);
					D3DXVec3TransformCoord(&initialPosition, &initialPosition, &transform);
					D3DXMATRIX forwardRotation = transform;
					forwardRotation._41 = forwardRotation._42 = forwardRotation._43 = 0;
					D3DXVec3TransformCoord(&initialVelocity, &initialVelocity, &forwardRotation);
					particle.initialPosition = initialPosition;
					particle.initialVelocity = initialVelocity;
				}
				if (data.collisionType >= 3) normalTail = false;
			}
		}
		else if (data.collisionShape == 1)
		{
			D3DXVECTOR3 center(transform._41, transform._42, transform._43);
			D3DXVECTOR3 difference = center - particle.position;
			if (data.collisionType == 0)
			{
				D3DXVECTOR3 position = particle.position;
				D3DXVECTOR3 acceleration = particles.Acceleration();
				D3DXVECTOR3 currentVelocity = CollisionVelocity(particle, particle.initialVelocity, acceleration, scale, particle.age);
				D3DXVECTOR3 colliderVelocity = previousPosition ? velocity : D3DXVECTOR3(0,0,0);
				normal = position - center;
				NormalizeCollisionVector(normal);
				D3DXVECTOR3 colliderProjection = normal * D3DXVec3Dot(&normal, &colliderVelocity);
				D3DXVECTOR3 colliderTangent = colliderVelocity - colliderProjection;
				D3DXVECTOR3 inward = center - position;
				NormalizeCollisionVector(inward);
				D3DXVECTOR3 particleProjection = inward * D3DXVec3Dot(&inward, &currentVelocity);
				D3DXVECTOR3 particleTangent = currentVelocity - particleProjection;
				D3DXVECTOR3 differenceA = colliderProjection - particleProjection;
				D3DXVECTOR3 sum = colliderProjection + particleProjection;
				D3DXVECTOR3 exchangedA = (sum - differenceA) * 0.5f;
				D3DXVECTOR3 differenceB = particleProjection - colliderProjection;
				D3DXVECTOR3 exchangedB = (sum - differenceB) * 0.5f;
				D3DXVECTOR3 colliderResult = exchangedA + colliderTangent;
				(void)colliderResult; //Original computes but does not publish collider motion.
				currentVelocity = (exchangedB + particleTangent) * (data.power * 0.0099999998f);
				float responseRadius = float(double(data.radius) * scale + diameter);
				outputPosition = center + normal * responseRadius;
				RestoreCollisionMotion(particle.mass, particle.resistance, scale, acceleration,
					outputPosition, currentVelocity, elapsed, particle.initialPosition, particle.initialVelocity);
			}
			else if (data.collisionType == 1)
			{
				normal = particle.position - center;
				NormalizeCollisionVector(normal);
				float responseRadius = float(double(data.radius) * scale + diameter);
				outputPosition = center + normal * responseRadius;
				D3DXVECTOR3 acceleration = particles.Acceleration();
				RestoreCollisionMotion(particle.mass, particle.resistance, scale, acceleration,
					outputPosition, D3DXVECTOR3(0,0,0), elapsed, particle.initialPosition, particle.initialVelocity);
			}
			else if (data.collisionType == 2)
			{
				if (trigger)
				{
					D3DXVECTOR3 direction = particle.position - center;
					D3DXVec3Normalize(&direction, &direction);
					outputPosition = center + direction * data.radius; //Unscaled original radius.
				}
				particle.lifetime = 0;
				normalTail = false;
			}
			else if (data.collisionType == 3 || data.collisionType == 4)
			{
				D3DXVECTOR3 acceleration = particles.Acceleration();
				D3DXVECTOR3 currentVelocity = particle.initialVelocity + acceleration * elapsed;
				if (data.collisionType == 4) difference = -difference;
				currentVelocity += difference * (data.power * 0.0099999998f);
				RestoreCollisionMotion(particle.mass, particle.resistance, scale, acceleration,
					particle.position, currentVelocity, elapsed, particle.initialPosition, particle.initialVelocity);
				outputPosition = particle.position;
				normalTail = false;
			}
		}
		else return 0; //4EEEB0 rejects unsupported shapes before any mutation.
		if (normalTail && faceNormal && (normal.x != 0 || normal.y != 0 || normal.z != 0))
			SetCollisionNormal(*this, particle, normal);
		particles.MarkParticleChanged(particle); //5154D0, before returning captured trigger.
		return trigger;
	}
	void VFX_Collision::ApplyParticles(VFX_ParticleSystem& effect, float time, VFX_VEGInstance& group)
	{
		Gfx_ParticleSystem* particles = effect.GetParticles();
		if (!particles) return;
		for (std::list<Gfx_ParticleInfo>::iterator i = particles->particles.begin(); i != particles->particles.end(); ++i)
		{
			const BYTE trigger = effect.data.collisionEffect != -1;
			const BYTE faceNormal = effect.data.faceNormal;
			float diameter = i->scale * i->size * particles->scale;
			D3DXVECTOR3 outputPosition(0,0,0);
			if (ApplyParticle(*particles, *i, diameter, time, group, outputPosition, faceNormal, trigger))
				effect.QueueTriggeredEffect(effect.data.collisionEffect, outputPosition, time);
		}
	}
	void VFX_Collision::Draw()
	{ } // Actual vtable6731EC+8 is the shared no-op5A9320.
}
