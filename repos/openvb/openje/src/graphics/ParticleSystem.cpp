#include "ParticleSystem.hpp"
#include "SystemInterface.hpp"
#include "TextureManager.hpp"
#include "ModelInstance.hpp"
#include "ModelNode.hpp"
#include <float.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdexcept>

namespace
{
	float particleSines[360], particleCosines[360];
	static DWORD CopyParticleData(DWORD size, const BYTE** cursor, void* output, DWORD count) //512D60.
	{
		const DWORD bytes = size * count;
		memcpy(output, *cursor, bytes);
		*cursor += bytes; //Reload after memcpy: an output may alias the cursor itself.
		return bytes;
	}

	BYTE ReadParticleConfiguration(Gfx_ParticleConfiguration* output, const BYTE* cursor) //513890.
	{
		const BYTE* current = cursor;
		DWORD ignored;
		CopyParticleData(sizeof(ignored), &current, &ignored, 1);
		CopyParticleData(sizeof(ignored), &current, &ignored, 1);
		CopyParticleData(sizeof(*output), &current, output, 1);
		return 1; //Both header DWORDs are ignored by the original reader.
	}

	double WrappedPhase(float value, float period)
	{
		double phase = fmod(value, period);
		return value != 0.0f && phase == 0.0f ? period : phase;
	}
	static void ColorAt(LONG count, const float* times, LONG& red, LONG& green,
		LONG& blue, const LONG colors[][3], float time)
	{
		if (count <= 0)
		{
			red = green = blue = 255;
			return;
		}
		LONG high;
		for (high = 0; high < count; ++high)
			if (time < times[high]) break;
		LONG low;
		if (high >= count)
		{
			high = count - 1;
			low = high;
		}
		else low = high > 0 ? high - 1 : high;
		float interval = times[high] - times[low];
		float factor = 0.0f;
		if (interval > 0.0f) factor = (time - times[low]) / interval;
		red = LONG((colors[high][0] - colors[low][0]) * factor + colors[low][0]);
		green = LONG((colors[high][1] - colors[low][1]) * factor + colors[low][1]);
		blue = LONG((colors[high][2] - colors[low][2]) * factor + colors[low][2]);
	}
	double RandomValue(float minimum, float maximum)
	{
		float difference = maximum - minimum;
		return rand() * double(difference) * 0.000030518509f + minimum;
	}
	float RandomRate(const Gfx_ParticleConfiguration& config)
	{
		// The original deliberately draws twice when the first value is nonnegative.
		if (RandomValue(config.minAddRate, config.maxAddRate) < 0.0) return 0;
		return float(RandomValue(config.minAddRate, config.maxAddRate));
	}
	float SignedOuter(float maximum)
	{
		float value = float(rand() * double(maximum) * 0.000030518509f);
		return (2 * rand() / RAND_MAX ? -1 : 1) * value;
	}
	D3DXVECTOR3 BoxPosition(D3DXVECTOR3 minimum, const D3DXVECTOR3& maximum)
	{
		for (int i = 0; i < 3; ++i) if (minimum[i] >= maximum[i]) minimum[i] = maximum[i];
		D3DXVECTOR3 result(-maximum.x, -maximum.y, -maximum.z);
		int first = 3 * rand() / RAND_MAX;
		if (first > 2) first = 2;
		result[first] = SignedOuter(maximum[first]);
		int second, third;
		if (result[first] > -minimum[first] && result[first] < minimum[first])
		{
			int choice = 2 * rand() / RAND_MAX;
			second = first == 0 ? (choice ? 1 : 2) : first == 1 ? (choice ? 0 : 2) : (choice ? 0 : 1);
			third = 3 - first - second;
			result[second] = SignedOuter(maximum[second]);
			if (result[second] > -minimum[second] && result[second] < minimum[second])
			{
				if (2 * rand() / RAND_MAX)
					result[third] = float(RandomValue(-maximum[third], -minimum[third]));
				else result[third] = float(RandomValue(minimum[third], maximum[third]));
			}
			else result[third] = SignedOuter(maximum[third]);
		}
		else
		{
			second = first == 0 ? 1 : 0;
			third = first == 2 ? 1 : 2;
			result[second] = SignedOuter(maximum[second]);
			result[third] = SignedOuter(maximum[third]);
		}
		return result;
	}
}

void Gfx_ParticleSystem::ClearTrail(Gfx_ParticleInfoBase& particle)
{
	if (particle.modelInstance)
	{
		delete particle.modelInstance;
		particle.modelInstance = NULL;
	}
}

void Gfx_ParticleSystem::ClearAfterImages(Gfx_ParticleInfo& particle)
{
	for (std::list<Gfx_AfterImageInfo>::iterator i = particle.afterImages.begin();
		i != particle.afterImages.end(); ++i)
		ClearTrail(*i);
	particle.afterImages.clear();
}

void Gfx_ParticleSystem::ClearParticle(Gfx_ParticleInfo& particle)
{
	ClearAfterImages(particle);
	ClearTrail(particle);
}

void Gfx_InitializeParticleTables()
{
	for (int angle = 0; angle < 360; ++angle)
	{
		double radians = angle * 0.017453292f;
		particleSines[angle] = float(sin(radians));
		particleCosines[angle] = float(cos(radians));
	}
}

double Gfx_SinDegrees(LONG angle) //512E90.
{
	if (angle < 0) angle = 360 - (-angle % 360);
	return particleSines[angle % 360];
}
double Gfx_CosDegrees(LONG angle) //512EC0.
{
	if (angle < 0) angle = 360 - (-angle % 360);
	return particleCosines[angle % 360];
}

double Gfx_ParticleSin(float radians)
{
	return Gfx_SinDegrees(LONG(radians * 57.295776f));
}
double Gfx_ParticleCos(float radians)
{
	return Gfx_CosDegrees(LONG(radians * 57.295776f));
}
double Gfx_ParticleWaveform(LONG kind, float first, float last, float time)
{
	double result = last;
	double phase;
	switch (kind)
	{
	case 1:
		result = WrappedPhase(time, 1.0f) * (last - double(first)) + first;
		break;
	case 2:
		result = WrappedPhase(time, 1.0f) < 0.5 ? first : last;
		break;
	case 3:
		phase = WrappedPhase(time, 1.0f);
		if (phase < 0.5f)
			result = phase * (last - double(first)) * 2.0f + first;
		else
			result = (1.0f - phase) * (last - double(first)) * 2.0f + first;
		break;
	case 4:
		time = time * 6.2831855f - 1.5707964f;
		result = (Gfx_ParticleSin(time) + 1.0f)
			* (last - double(first)) * 0.5f + first;
		break;
	}
	return result;
}

Gfx_ParticleBatch::Gfx_ParticleBatch()
	: instances(NULL), packet(NULL), matrices(NULL), instanceCount(0),
	vertices(NULL), vertexCount(0), vertexCapacity(0), indices(NULL),
	indexCount(0), indexCapacity(0), flag94(0)
{
}

Gfx_ParticleBatch::~Gfx_ParticleBatch()
{
	if (vertices)
	{
		free(vertices);
		vertices = NULL;
	}
	if (indices)
	{
		free(indices);
		indices = NULL;
	}
	delete[] instances;
	instances = NULL;
	if (matrices)
	{
		delete[] matrices;
		matrices = NULL;
	}
	if (packet)
	{
		delete packet;
		packet = NULL;
	}
	// The original aggregate performs one Free then destroys MaterialSet; the
	// ordinary MeshData member destructor supplies that same single Free.
}

Gfx_ParticleInfo::Gfx_ParticleInfo(const Gfx_ParticleInfo& source)
	: Gfx_ParticleInfoBase(source), afterImages(source.afterImages), flagD0(source.flagD0)
{
}

Gfx_ParticleInfo::~Gfx_ParticleInfo()
{
}

Gfx_ParticleSystem::Gfx_ParticleSystem()
{
	geometryDirty = 1;
	D3DXMatrixIdentity(&secondTransform);
	texture = 0;
	ownsTexture = 0;
	model = 0;
	trailTexture = 0;
	ownsTrailTexture = 0;
	enabled = 1;
	emissionTime = NULL;
	emissionRate = 0;
	scale = 1.0f;
	lastFieldOfView = -1.0f;
	// Working transform+58 is constructor-unwritten; only secondTransform+AC
	// receives the identity before the first simulation update.
	batches.resize(2, static_cast<Gfx_ParticleBatch*>(NULL));
	pools.resize(2, std::list<Gfx_ParticleBatch*>());
	lines.resize(2, Gfx_LineSystem());
	for (LONG i = static_cast<LONG>(batches.size()); i > 0; --i)
	{
		Gfx_ParticleBatch* batch = new Gfx_ParticleBatch;
		batches[i - 1] = batch;
		batch->instanceCount = 1;
		batch->instances = new Gfx_MeshInstance[batch->instanceCount];
		batch->matrices = new D3DXMATRIX[batch->instanceCount];
		for (LONG j = 0; j < static_cast<LONG>(batch->instanceCount); ++j)
		{
			D3DXMatrixIdentity(&batch->matrices[j]);
			batch->instances[j].flag1D = 1;
			batch->instances[j].flag1E = 1;
		}
		// Re-publish empty geometry after instance/matrix construction, as native.
		batch->vertexCount = 0;
		batch->vertexCapacity = 0;
		batch->vertices = NULL;
		batch->indexCount = 0;
		batch->indexCapacity = 0;
		batch->indices = NULL;
		batch->packet = new Gfx_PacketData;
		batch->flag94 = 0;
	}
	currentBatchIndex = 0;
}

Gfx_ParticleSystem::~Gfx_ParticleSystem()
{
	for (std::vector<Gfx_ParticleBatch*>::iterator i = batches.begin(); i != batches.end(); ++i)
		delete *i;
	for (std::vector<std::list<Gfx_ParticleBatch*> >::iterator i = pools.begin();
		i != pools.end(); ++i)
	{
		for (std::list<Gfx_ParticleBatch*>::iterator j = i->begin(); j != i->end(); ++j)
			delete *j;
	}
	if (ownsTexture && texture)
		g_TextureManager->Release(texture);
	if (ownsTrailTexture && trailTexture)
		g_TextureManager->Release(trailTexture);
	if (emissionTime)
		::operator delete(emissionTime);
	ClearParticles();
	ClearModelInstances();
	ClearRetired();
	if (model && ownsModel)
		g_GfxSystem->ReleaseModelData(&model);
}

void Gfx_ParticleSystem::Configure(const Gfx_ParticleConfiguration& input)
{
	float* previous = emissionTime;
	emissionRate = 0;
	// This is a nonowning value copy, including original unwritten padding.
	// It does not overlay any class, runtime container or virtual table.
	memcpy(&configuration, &input, sizeof(configuration));
	if (previous)
	{
		::operator delete(previous);
		emissionTime = NULL;
	}
}

void Gfx_ParticleSystem::LoadConfiguration(const BYTE* data, DWORD size) //514410.
{
	(void)size; //Original RET8 accepts the size but never reads it.
	emissionRate = 0;
	Gfx_ParticleConfiguration input;
	if (ReadParticleConfiguration(&input, data))
		memcpy(&configuration, &input, sizeof(configuration));
	if (ownsTexture && texture)
	{
		g_TextureManager->Release(texture);
		texture = 0;
	}
	texture = 0;
	ownsTexture = 0;
	if (emissionTime)
	{
		::operator delete(emissionTime);
		emissionTime = NULL;
	}
}

void Gfx_ParticleSystem::SetEnabled(BYTE value)
{
	enabled = value;
	if (!value && emissionTime)
	{
		::operator delete(emissionTime);
		emissionTime = NULL;
	}
}

void Gfx_ParticleSystem::SetTexture(DWORD handle, BYTE own, BYTE releasePrevious)
{
	if (releasePrevious && ownsTexture && texture)
		g_TextureManager->Release(texture);
	texture = handle;
	ownsTexture = own;
}

void Gfx_ParticleSystem::SetModel(DWORD handle, BYTE own, BYTE releasePrevious)
{
	ClearModelInstances();
	if (releasePrevious && ownsModel && model)
		g_GfxSystem->ReleaseModelData(&model);
	model = handle;
	ownsModel = own;
}

void Gfx_ParticleSystem::ClearParticles()
{
	for (std::list<Gfx_ParticleInfo>::iterator i = particles.begin(); i != particles.end(); ++i)
		ClearParticle(*i);
	particles.clear();
	geometryDirty = 1;
}

void Gfx_ParticleSystem::ClearRetired()
{
	for (std::list<Gfx_ParticleInfo>::iterator i = retiredParticles.begin();
		i != retiredParticles.end(); ++i)
		ClearParticle(*i);
	retiredParticles.clear();
}

void Gfx_ParticleSystem::Reset()
{
	ClearParticles();
	if (emissionTime)
	{
		::operator delete(emissionTime);
		emissionTime = NULL;
	}
}

void Gfx_ParticleSystem::GetBounds(D3DXVECTOR3& low, D3DXVECTOR3& high) const
{
	low.x = FLT_MAX;
	low.y = FLT_MAX;
	low.z = FLT_MAX;
	high.x = -FLT_MAX;
	high.y = -FLT_MAX;
	high.z = -FLT_MAX;
	for (std::list<Gfx_ParticleInfo>::const_iterator i = particles.begin(); i != particles.end(); ++i)
	{
		if (!(i->lifetime <= 0.0f))
		{
			const float radius = scale * i->scale * i->size * 0.5f;
			low.x = i->position.x - radius < low.x ? i->position.x - radius : low.x;
			low.y = i->position.y - radius < low.y ? i->position.y - radius : low.y;
			low.z = i->position.z - radius < low.z ? i->position.z - radius : low.z;
			high.x = i->position.x + radius > high.x ? i->position.x + radius : high.x;
			high.y = i->position.y + radius > high.y ? i->position.y + radius : high.y;
			high.z = i->position.z + radius > high.z ? i->position.z + radius : high.z;
		}
	}
}

void Gfx_ParticleSystem::SetTransform(const D3DXMATRIX& value)
{
	secondTransform = value;
}

void Gfx_ParticleSystem::EvaluateSystem(float time, float& fraction, LONG& red, LONG& green, LONG& blue,
	float& systemScale, float& systemAlpha, float* minimumX, float* minimumY,
	float* minimumZ, float* maximumX, float* maximumY, float* maximumZ) const
{
	const Gfx_ParticleConfiguration& c = configuration;
	fraction = 0;
	if (!(c.flags & 4)) fraction = float(WrappedPhase(time, 1.0f));
	else if (c.emitterLifetime > 0 && time < c.emitterLifetime)
		fraction = float(double(time) / c.emitterLifetime);
	else fraction = 1;
	red = green = blue = 0;
	if ((c.flags & 0x200) && c.adjustSystemColor)
	{
		float phase = float((!c.adjustColorOverLifetime || c.emitterLifetime == 0
			? double(time) : fraction) * c.colorFreq + c.colorPhase);
		ColorAt(c.numColorsUsed, c.colorTimes, red, green, blue, c.colors,
			float(Gfx_ParticleWaveform(c.colorWaveform, 0, 1, phase)));
	}
	systemScale = 1;
	if (c.flags & 0x20)
	{
		float phase = float((!c.adjustSystemScaleOverLifetime || c.emitterLifetime == 0
			? double(time) : fraction) * c.systemScaleFreq + c.systemScalePhase);
		systemScale = float(Gfx_ParticleWaveform(c.systemScaleWaveform, c.startSystemScale, c.endSystemScale, phase));
	}
	systemAlpha = 1;
	if (c.flags & 0x80)
	{
		float phase = float((!c.adjustSystemAlphaOverLifetime || c.emitterLifetime == 0
			? double(time) : fraction) * c.systemAlphaFreq + c.systemAlphaPhase);
		systemAlpha = float(Gfx_ParticleWaveform(c.systemAlphaWaveform, c.startSystemAlpha, c.endSystemAlpha, phase));
	}
	if (minimumX)
	{
		*minimumX = c.minDimensionsX;
		if (c.flags & 0x400)
		{
			float phase = float((!c.adjustMinDimXOverLifetime || c.emitterLifetime == 0
				? double(time) : fraction) * c.minDimXFreq + c.minDimXPhase);
			*minimumX = float(Gfx_ParticleWaveform(c.minDimXWaveform, c.minDimensionsX, c.finalMinDimX, phase));
		}
	}
	if (maximumX)
	{
		*maximumX = c.maxDimensionsX;
		if (c.flags & 0x2000)
		{
			float phase = float((!c.adjustMaxDimXOverLifetime || c.emitterLifetime == 0
				? double(time) : fraction) * c.maxDimXFreq + c.maxDimXPhase);
			*maximumX = float(Gfx_ParticleWaveform(c.maxDimXWaveform, c.maxDimensionsX, c.finalMaxDimX, phase));
		}
	}
	if (minimumY)
	{
		*minimumY = c.minDimensionsY;
		if (c.flags & 0x800)
		{
			float phase = float((!c.adjustMinDimYOverLifetime || c.emitterLifetime == 0
				? double(time) : fraction) * c.minDimYFreq + c.minDimYPhase);
			*minimumY = float(Gfx_ParticleWaveform(c.minDimYWaveform, c.minDimensionsY, c.finalMinDimY, phase));
		}
	}
	if (maximumY)
	{
		*maximumY = c.maxDimensionsY;
		if (c.flags & 0x4000)
		{
			float phase = float((!c.adjustMaxDimYOverLifetime || c.emitterLifetime == 0
				? double(time) : fraction) * c.maxDimYFreq + c.maxDimYPhase);
			*maximumY = float(Gfx_ParticleWaveform(c.maxDimYWaveform, c.maxDimensionsY, c.finalMaxDimY, phase));
		}
	}
	if (minimumZ)
	{
		*minimumZ = c.minDimensionsZ;
		if (c.flags & 0x1000)
		{
			float phase = float((!c.adjustMinDimZOverLifetime || c.emitterLifetime == 0
				? double(time) : fraction) * c.minDimZFreq + c.minDimZPhase);
			*minimumZ = float(Gfx_ParticleWaveform(c.minDimZWaveform, c.minDimensionsZ, c.finalMinDimZ, phase));
		}
	}
	if (maximumZ)
	{
		*maximumZ = c.maxDimensionsZ;
		if (c.flags & 0x8000)
		{
			float phase = float((!c.adjustMaxDimZOverLifetime || c.emitterLifetime == 0
				? double(time) : fraction) * c.maxDimZFreq + c.maxDimZPhase);
			*maximumZ = float(Gfx_ParticleWaveform(c.maxDimZWaveform, c.maxDimensionsZ, c.finalMaxDimZ, phase));
		}
	}
}

D3DXVECTOR3 Gfx_ParticleSystem::Acceleration() const
{
	const Gfx_ParticleConfiguration& c = configuration;
	D3DXVECTOR3 first;
	first.x = c.acceleration1X;
	first.y = c.acceleration1Y;
	first.z = c.acceleration1Z;
	D3DXMATRIX matrix;
	if ((first.x != 0 || first.y != 0 || first.z != 0) && c.rotateAccel1WithTarget)
	{
		matrix = secondTransform;
		matrix._41 = 0;
		matrix._42 = 0;
		matrix._43 = 0;
		D3DXVec3TransformCoord(&first, &first, &matrix);
	}
	D3DXVECTOR3 second;
	second.x = c.acceleration2X;
	second.y = c.acceleration2Y;
	second.z = c.acceleration2Z;
	if (second.x != 0 || second.y != 0 || second.z != 0)
	{
		if (c.rotationX != 0 || c.rotationY != 0 || c.rotationZ != 0)
		{
			D3DXMatrixRotationYawPitchRoll(&matrix, c.rotationX, c.rotationY, c.rotationZ);
			matrix._41 = 0;
			matrix._42 = 0;
			matrix._43 = 0;
			D3DXVec3TransformCoord(&second, &second, &matrix);
		}
		if (c.rotateAccel2WithTarget)
		{
			matrix = secondTransform;
			matrix._41 = 0;
			matrix._42 = 0;
			matrix._43 = 0;
			D3DXVec3TransformCoord(&second, &second, &matrix);
		}
	}
	D3DXVECTOR3 result;
	result.x = first.x + second.x;
	result.y = first.y + second.y;
	result.z = first.z + second.z;
	return result;
}

void Gfx_ParticleSystem::UpdateParticle(Gfx_ParticleInfoBase& particle, float time, float systemFraction,
	LONG systemRed, LONG systemGreen, LONG systemBlue, float systemScale, float systemAlpha)
{
	(void)systemFraction; // The original fraction argument is accepted but not read.
	float elapsed = time - particle.birthTime;
	if (particle.age > elapsed)
	{
		Gfx_ParticleInfo* active = dynamic_cast<Gfx_ParticleInfo*>(&particle);
		if (active)
		{
			particle.age = particle.lifetime;
			ClearAfterImages(*active);
		}
	}
	particle.age = elapsed;
	if (elapsed >= particle.lifetime) elapsed = particle.lifetime;
	D3DXVECTOR3 acceleration = Acceleration();
	if (particle.mass == 0) particle.position = particle.initialPosition;
	else
	{
		float resistance = particle.resistance;
		if (fabs(resistance) < 0.000008)
			resistance = float((resistance >= 0 ? 1 : -1) * 0.000008);
		double decay = 1.0 - exp(-double(resistance) * elapsed);
		float inverse = float(1.0 / resistance);
		for (int axis = 0; axis < 3; ++axis)
		{
			double force = double(scale) * particle.mass * acceleration[axis] / resistance;
			particle.position[axis] = float(elapsed * force + particle.initialPosition[axis]
				+ (particle.initialVelocity[axis] - force) * inverse * decay);
		}
	}
	const Gfx_ParticleConfiguration& c = configuration;
	bool changedScale = false, changedAlpha = false, changedColor = false;
	if ((c.flags & 0xFFE0) && particle.lifetime > 0)
	{
		float fraction = elapsed / particle.lifetime;
		if (c.flags & 0x60)
		{
			double factor = 1;
			if (c.flags & 0x40)
			{
				float phase = float((c.adjustParticleScaleOverLifetime ? double(fraction) : elapsed)
					* c.particleScaleFreq + c.particleScalePhase);
				factor = Gfx_ParticleWaveform(c.particleScaleWaveform, c.startParticleScale, c.endParticleScale, phase);
			}
			particle.scale = float(factor * systemScale);
			changedScale = true;
		}
		if (c.flags & 0x180)
		{
			double factor = 1;
			if (c.flags & 0x100)
			{
				float phase = float((c.adjustParticleAlphaOverLifetime ? double(fraction) : elapsed)
					* c.particleAlphaFreq + c.particleAlphaPhase);
				factor = Gfx_ParticleWaveform(c.particleAlphaWaveform, c.startParticleAlpha, c.endParticleAlpha, phase);
			}
			particle.alpha = LONG(factor * systemAlpha * 255.0);
			changedAlpha = true;
		}
		if (c.flags & 0x200)
		{
			if (c.adjustSystemColor)
			{
				particle.red = systemRed;
				particle.green = systemGreen;
				particle.blue = systemBlue;
			}
			else
			{
				float phase = float((c.adjustColorOverLifetime ? double(fraction) : elapsed)
					* c.colorFreq + c.colorPhase);
				ColorAt(c.numColorsUsed, c.colorTimes, particle.red, particle.green, particle.blue, c.colors,
					float(Gfx_ParticleWaveform(c.colorWaveform, 0, 1, phase)));
			}
			changedColor = true;
		}
	}
	if (!changedScale) particle.scale = 1;
	if (!changedAlpha) particle.alpha = particle.initialAlpha;
	if (!changedColor)
	{
		particle.red = particle.initialRed;
		particle.green = particle.initialGreen;
		particle.blue = particle.initialBlue;
	}
}

void Gfx_ParticleSystem::MarkParticleChanged(Gfx_ParticleInfoBase& particle)
{
	float time = particle.age + particle.birthTime;
	float fraction = 0;
	LONG red, green, blue;
	float systemScale = 1, systemAlpha = 1;
	float minimumX, minimumY, minimumZ, maximumX, maximumY, maximumZ;
	EvaluateSystem(time, fraction, red, green, blue, systemScale, systemAlpha,
		&minimumX, &minimumY, &minimumZ, &maximumX, &maximumY, &maximumZ);
	UpdateParticle(particle, time, fraction, red, green, blue, systemScale, systemAlpha);
}

void Gfx_ParticleSystem::AddParticle(D3DXVECTOR3 inputPosition, D3DXVECTOR3 inputVelocity,
	float mass, float resistance, LONG red, LONG green, LONG blue, LONG alpha,
	float lifetime, float birthTime, D3DXVECTOR3 rotation, D3DXVECTOR3 spin,
	BYTE rotatable, float size, BYTE initialize)
{
	D3DXVECTOR3 position = inputPosition, velocity = inputVelocity;
	D3DXMATRIX matrix = secondTransform;
	D3DXVec3TransformCoord(&position, &position, &matrix);
	matrix._41 = 0;
	matrix._42 = 0;
	matrix._43 = 0;
	D3DXVec3TransformCoord(&velocity, &velocity, &matrix);
	if (owner)
	{
		matrix = static_cast<Gfx_ModelNode*>(owner)->worldTransform;
		D3DXVec3TransformCoord(&position, &position, &matrix);
		matrix._41 = 0;
		matrix._42 = 0;
		matrix._43 = 0;
		D3DXVec3TransformCoord(&velocity, &velocity, &matrix);
	}
	Gfx_ParticleInfo particle;
	particle.position.x = position.x;
	particle.position.y = position.y;
	particle.position.z = position.z;
	particle.initialPosition.x = position.x;
	particle.initialPosition.y = position.y;
	particle.initialPosition.z = position.z;
	particle.initialVelocity.x = velocity.x;
	particle.initialVelocity.y = velocity.y;
	particle.initialVelocity.z = velocity.z;
	particle.mass = mass;
	particle.resistance = resistance;
	particle.red = red;
	particle.initialRed = red;
	particle.green = green;
	particle.initialGreen = green;
	particle.blue = blue;
	particle.initialBlue = blue;
	particle.alpha = alpha;
	particle.initialAlpha = alpha;
	particle.size = size;
	particle.scale = 1;
	particle.lifetime = lifetime;
	particle.birthTime = birthTime;
	particle.age = 0;
	particle.rotation.x = rotation.x;
	particle.rotation.y = rotation.y;
	particle.rotation.z = rotation.z;
	particle.spin.x = spin.x;
	particle.spin.y = spin.y;
	particle.spin.z = spin.z;
	particle.modelInstance = NULL;
	particle.rotatable = rotatable;
	particle.basisMatrix = 0;
	particle.flagD0 = 0;
	if (initialize)
	{
		float fraction = 0;
		LONG systemRed, systemGreen, systemBlue;
		float systemScale = 1, systemAlpha = 1;
		EvaluateSystem(birthTime, fraction, systemRed, systemGreen, systemBlue,
			systemScale, systemAlpha, NULL, NULL, NULL, NULL, NULL, NULL);
		UpdateParticle(particle, birthTime, fraction, systemRed, systemGreen,
			systemBlue, systemScale, systemAlpha);
	}
	particles.push_front(particle);
	geometryDirty = 1;
}

void Gfx_ParticleSystem::Emit(float time)
{
	std::list<float> times;
	float difference = 0;
	const Gfx_ParticleConfiguration& c = configuration;
	if (!emissionTime)
	{
		emissionTime = static_cast<float*>(::operator new(sizeof(float)));
		*emissionTime = time;
		emissionRate = RandomRate(c);
		times.push_back(time);
	}
	else
	{
		difference = time - *emissionTime;
		double elapsed = fabs(difference);
		if (!(emissionRate <= 0) && !(elapsed >= 1.0 / emissionRate)) return;
		float remaining = float(elapsed);
		if (emissionRate != 0)
		{
			double cap = 1.0 / emissionRate;
			if (!(cap > c.maxParticleLifetime)) cap = c.maxParticleLifetime;
			if (cap < elapsed) remaining = float(cap);
		}
		float spawnTime = float(elapsed + *emissionTime - remaining);
		for (;;)
		{
			if (!(emissionRate <= 0))
			{
				double interval = 1.0 / emissionRate;
				if (!(remaining >= interval)) break; //Original also exits on unordered comparison.
				if (emissionRate > 0)
				{
					remaining = float(remaining - interval);
					spawnTime = float(spawnTime + interval);
				}
			}
			//Ordered nonpositive rates append without advancing, then reroll.
			//An all-nonpositive sequence can remain unbounded in the original.
			times.push_back(spawnTime);
			emissionRate = RandomRate(c);
		}
	}
	if (times.empty()) return;
	*emissionTime = difference + *emissionTime;
	SystemValues values;
	EvaluateSystem(time, values.fraction, values.red, values.green, values.blue, values.scale, values.alpha, &values.minimum.x, &values.minimum.y, &values.minimum.z, &values.maximum.x, &values.maximum.y, &values.maximum.z);
	LONG limit = c.maxParticles < 0 || c.maxParticles >= 5000 ? 5000 : c.maxParticles;
	while (particles.size() < size_t(limit) && !times.empty())
	{
		float birthTime = times.front();
		times.pop_front();
		LONG count = c.minParticlesPerAdd + (c.maxParticlesPerAdd - c.minParticlesPerAdd) * rand() / RAND_MAX;
		while (particles.size() < size_t(limit) && count > 0)
		{
			--count;
			D3DXVECTOR3 position(0,0,0);
			if (c.emitterShape == 0) position = BoxPosition(values.minimum, values.maximum);
			else if (c.emitterShape == 1)
			{
				int azimuth = 360 * rand() / RAND_MAX;
				int elevation = 360 * rand() / RAND_MAX;
				position.x = float(Gfx_CosDegrees(elevation) * double(Gfx_CosDegrees(azimuth))
					* RandomValue(values.minimum.x, values.maximum.x));
				position.y = float(Gfx_CosDegrees(elevation) * double(Gfx_SinDegrees(azimuth))
					* RandomValue(values.minimum.y, values.maximum.y));
				position.z = float(Gfx_SinDegrees(elevation) * RandomValue(values.minimum.z, values.maximum.z));
			}
			D3DXVECTOR3 velocity(0,0,0);
			if (spawnCallback)
				spawnCallback(position.x, position.y, position.z, c.initialVelocityTargetUsed,
					&velocity.x, &velocity.y, &velocity.z);
			else if (!c.initialVelocityTargetUsed)
			{
				velocity.x = float(RandomValue(c.minInitialVelocityX, c.maxInitialVelocityX));
				velocity.y = float(RandomValue(c.minInitialVelocityY, c.maxInitialVelocityY));
				velocity.z = float(RandomValue(c.minInitialVelocityZ, c.maxInitialVelocityZ));
			}
			if (c.minExplosion != 0 || c.maxExplosion != 0)
			{
				double factor = RandomValue(c.minExplosion, c.maxExplosion);
				velocity.x = float(-position.x * factor + velocity.x);
				velocity.y = float(-position.y * factor + velocity.y);
				velocity.z = float(-position.z * factor + velocity.z);
			}
			if (c.minVibration != 0 || c.maxVibration != 0)
			{
				int azimuth = 360 * rand() / RAND_MAX;
				int elevation = 360 * rand() / RAND_MAX;
				double factor = RandomValue(c.minVibration, c.maxVibration);
				velocity.x = float(Gfx_CosDegrees(elevation) * double(Gfx_CosDegrees(azimuth)) * factor + velocity.x);
				velocity.y = float(Gfx_CosDegrees(elevation) * double(Gfx_SinDegrees(azimuth)) * factor + velocity.y);
				velocity.z = float(Gfx_SinDegrees(elevation) * factor + velocity.z);
			}
			double velocityScale = RandomValue(c.minInitialVelocityScale, c.maxInitialVelocityScale);
			velocity.x = float(velocity.x * velocityScale);
			velocity.y = float(velocity.y * velocityScale);
			velocity.z = float(velocity.z * velocityScale);
			float mass = float(RandomValue(c.minParticleMass, c.maxParticleMass));
			float resistance = float(RandomValue(c.minParticleResistance, c.maxParticleResistance));
			LONG red, green, blue;
			ColorAt(c.numColorsUsed, c.colorTimes, red, green, blue, c.colors,
				float(rand() * double(0.000030518509f)));
			LONG alpha = LONG(RandomValue(c.minParticleAlpha, c.maxParticleAlpha) * 255.0);
			float lifetime = float(RandomValue(c.minParticleLifetime, c.maxParticleLifetime));
			float size = float(RandomValue(c.minParticleSize, c.maxParticleSize));
			D3DXVECTOR3 rotation, spin;
			rotation.x = float(RandomValue(c.minParticleRotationX, c.maxParticleRotationX));
			rotation.y = float(RandomValue(c.minParticleRotationY, c.maxParticleRotationY));
			rotation.z = float(RandomValue(c.minParticleRotationZ, c.maxParticleRotationZ));
			spin.x = float(RandomValue(c.minParticleSpinX, c.maxParticleSpinX));
			spin.y = float(RandomValue(c.minParticleSpinY, c.maxParticleSpinY));
			spin.z = float(RandomValue(c.minParticleSpinZ, c.maxParticleSpinZ));
			AddParticle(position * scale, velocity * scale, mass, resistance, red, green, blue,
				alpha, lifetime, birthTime, rotation, spin, c.rotatableParticles, size, 0);
			UpdateParticle(particles.front(), time, values.fraction, values.red, values.green, values.blue, values.scale, values.alpha);
		}
	}
}

void Gfx_ParticleSystem::Update(float time)
{
	SystemValues values;
	EvaluateSystem(time, values.fraction, values.red, values.green, values.blue, values.scale, values.alpha, NULL, NULL, NULL, NULL, NULL, NULL);
	bool expired = (configuration.flags & 4) && time > configuration.emitterLifetime;
	if (enabled && (configuration.flags & 8) && !expired) Emit(time);
	std::list<Gfx_ParticleInfo>::iterator i = particles.begin();
	while (i != particles.end())
	{
		bool dead = configuration.particleTrailType
			? configuration.trailLifetime + i->lifetime + i->birthTime < time
			: i->lifetime + i->birthTime < time;
		if (dead || time < i->birthTime)
		{
			if (dead && deathCallback)
				deathCallback(i->position.x, i->position.y, i->position.z, time);
			ClearParticle(*i);
			i = particles.erase(i);
			geometryDirty = 1;
			continue;
		}
		if (time - i->birthTime != i->age)
		{
			UpdateParticle(*i, time, values.fraction, values.red, values.green, values.blue, values.scale, values.alpha);
			geometryDirty = 1;
		}
		++i;
	}
	if (geometryDirty && configuration.particleTrailType == 1)
		RebuildTrails();
}
