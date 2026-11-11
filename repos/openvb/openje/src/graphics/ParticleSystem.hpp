#pragma once

#include "LineSystem.hpp"
#include "ParticleConfiguration.hpp"
#include <list>
#include <vector>

class Gfx_ModelInstance;
class Gfx_Viewport;
class Gfx_Camera;

void Gfx_InitializeParticleTables(); // 0x512E50, called by actual VFX startup5B30C0.
double Gfx_SinDegrees(LONG angle); //512E90, shared initialized integer-degree table.
double Gfx_CosDegrees(LONG angle); //512EC0.
double Gfx_ParticleSin(float radians); //512EF0, existing integer-degree table.
double Gfx_ParticleCos(float radians); //512F10, existing integer-degree table.
double Gfx_ParticleWaveform(LONG kind, float first, float last, float time); //512F70.

// Original nonpolymorphic152-byte batch. Real mesh/material and packet owners
// provide their lifetimes; no original-memory overlay is used.
struct Gfx_ParticleBatch
{
	Gfx_ParticleBatch(); // 0x515040.
	~Gfx_ParticleBatch(); // Cleanup inlined in0x516600.
	Gfx_MeshData mesh;
	Gfx_MeshInstance* instances;
	Gfx_PacketData* packet;
	D3DXMATRIX* matrices;
	DWORD instanceCount;
	void* vertices;
	DWORD vertexCount, vertexCapacity;
	void* indices;
	DWORD indexCount, indexCapacity;
	BYTE flag94;
private:
	Gfx_ParticleBatch(const Gfx_ParticleBatch&);
	Gfx_ParticleBatch& operator=(const Gfx_ParticleBatch&);
};

// Actual particle value, including its original analytic-motion and color state.
class Gfx_ParticleInfoBase
{
public:
	Gfx_ParticleInfoBase() //Inline in514BC0 and the5185D0 AfterImage construction.
	{
		red = green = blue = 0;
		initialRed = initialGreen = initialBlue = 0;
		// Other scalar and matrix fields remain unwritten before their producer.
	}
	Gfx_ParticleInfoBase(const Gfx_ParticleInfoBase& source)
	{
		position.x = source.position.x;
		position.y = source.position.y;
		position.z = source.position.z;
		initialPosition.x = source.initialPosition.x;
		initialPosition.y = source.initialPosition.y;
		initialPosition.z = source.initialPosition.z;
		initialVelocity.x = source.initialVelocity.x;
		initialVelocity.y = source.initialVelocity.y;
		initialVelocity.z = source.initialVelocity.z;
		mass = source.mass;
		resistance = source.resistance;
		memcpy(&red, &source.red, sizeof(red) + sizeof(green) + sizeof(blue));
		alpha = source.alpha;
		memcpy(&initialRed, &source.initialRed, sizeof(initialRed) + sizeof(initialGreen) + sizeof(initialBlue));
		initialAlpha = source.initialAlpha;
		size = source.size;
		scale = source.scale;
		lifetime = source.lifetime;
		birthTime = source.birthTime;
		age = source.age;
		rotation.x = source.rotation.x;
		rotation.y = source.rotation.y;
		rotation.z = source.rotation.z;
		spin.x = source.spin.x;
		spin.y = source.spin.y;
		spin.z = source.spin.z;
		modelInstance = source.modelInstance;
		rotatable = source.rotatable;
		basisMatrix = source.basisMatrix;
		matrix = source.matrix;
	}
	Gfx_ParticleInfoBase& operator=(const Gfx_ParticleInfoBase& source)
	{
		position.x = source.position.x;
		position.y = source.position.y;
		position.z = source.position.z;
		initialPosition.x = source.initialPosition.x;
		initialPosition.y = source.initialPosition.y;
		initialPosition.z = source.initialPosition.z;
		initialVelocity.x = source.initialVelocity.x;
		initialVelocity.y = source.initialVelocity.y;
		initialVelocity.z = source.initialVelocity.z;
		mass = source.mass;
		resistance = source.resistance;
		memcpy(&red, &source.red, sizeof(red) + sizeof(green) + sizeof(blue));
		alpha = source.alpha;
		memcpy(&initialRed, &source.initialRed, sizeof(initialRed) + sizeof(initialGreen) + sizeof(initialBlue));
		initialAlpha = source.initialAlpha;
		size = source.size;
		scale = source.scale;
		lifetime = source.lifetime;
		birthTime = source.birthTime;
		age = source.age;
		rotation.x = source.rotation.x;
		rotation.y = source.rotation.y;
		rotation.z = source.rotation.z;
		spin.x = source.spin.x;
		spin.y = source.spin.y;
		spin.z = source.spin.z;
		modelInstance = source.modelInstance;
		rotatable = source.rotatable;
		basisMatrix = source.basisMatrix;
		matrix = source.matrix;
		return *this;
	}
	virtual ~Gfx_ParticleInfoBase() {} // No owned trail cleanup here.
	D3DXVECTOR3 position, initialPosition, initialVelocity;
	float mass, resistance;
	LONG red, green, blue, alpha;
	LONG initialRed, initialGreen, initialBlue, initialAlpha;
	float size, scale, lifetime, birthTime, age;
	D3DXVECTOR3 rotation, spin;
	Gfx_ModelInstance* modelInstance; //Original+7C, lazily owned model from519E70; not a LineSystem.
	BYTE rotatable, basisMatrix;
	D3DXMATRIX matrix;
};

class Gfx_AfterImageInfo : public Gfx_ParticleInfoBase
{
public:
	virtual ~Gfx_AfterImageInfo() {}
	Gfx_AfterImageInfo() {} //518AA6; sampleAge remains unwritten.
	float sampleAge; //Original+C4, copied by514800; distinct from base age/birthTime.
private:
	Gfx_AfterImageInfo& operator=(const Gfx_AfterImageInfo&);
};

class Gfx_ParticleInfo : public Gfx_ParticleInfoBase
{
public:
	Gfx_ParticleInfo() {} //514BC0; native first inline and second outlined construction.
	Gfx_ParticleInfo(const Gfx_ParticleInfo& source); // Copy in0x516010.
	virtual ~Gfx_ParticleInfo(); // 0x514C30.
	std::list<Gfx_AfterImageInfo> afterImages;
	BYTE flagD0;
private:
	Gfx_ParticleInfo& operator=(const Gfx_ParticleInfo&);
};

// Actual lower graphics owner constructed by the separate VFX cooker4F87E0.
// Normal containers and child classes own storage. Actual finite timed emission
// and analytic motion support installed dust; trail/other open branches throw.
class Gfx_ParticleSystem : public Gfx_NodeAttribute
{
public:
	//517F66 calls this for its three outputs; the return register is unused.
	typedef void (__cdecl *SpawnCallback)(float, float, float, int, float*, float*, float*);
	typedef void (__cdecl *DeathCallback)(float, float, float, float); //4F87B0; original caller discards EAX.
	Gfx_ParticleSystem(); // 0x51A880.
	virtual ~Gfx_ParticleSystem(); // 0x516600.
	virtual void Update(float time); // Actual second slot0x51A6C0, absolute seconds.
	void Draw(Gfx_Viewport* viewport); // 0x51C490; implemented in ParticleSystemDraw.cpp.
	void Configure(const Gfx_ParticleConfiguration& input); // 0x513070.
	void LoadConfiguration(const BYTE* data, DWORD size); //514410; size is not checked.
	void SetEnabled(BYTE value); // 0x513F80.
	void SetTexture(DWORD handle, BYTE own, BYTE releasePrevious); // 0x5144E0.
	void SetModel(DWORD handle, BYTE own, BYTE releasePrevious); // 0x514CA0.
	void GetBounds(D3DXVECTOR3& low, D3DXVECTOR3& high) const; // 0x5146C0.
	void Reset(); // 0x516550.
	void MarkParticleChanged(Gfx_ParticleInfoBase& particle); // 0x5154D0.
	D3DXVECTOR3 Acceleration() const; // 0x5138E0; used by collision integration.
	void SetTransform(const D3DXMATRIX& value); // Source convenience for the lower target-matrix copy; no independent original entry.
	void SetSpawnCallback(SpawnCallback value) { spawnCallback = value; }
	void SetDeathCallback(DeathCallback value) { deathCallback = value; }

	std::vector<Gfx_ParticleBatch*> batches; // +24, owns.
	DWORD currentBatchIndex; // +34.
	std::vector<std::list<Gfx_ParticleBatch*> > pools; // +38, owns batches.
	std::list<Gfx_ParticleInfo> particles; // +48.
	BYTE geometryDirty; // +54.
	D3DXMATRIX transform; // +58; constructor-unwritten working matrix.
	float lastFieldOfView; // +98; Draw cache, not the simulation clock.
	Gfx_Viewport* lastViewport; // +9C; constructor-unwritten until Draw.
	std::list<Gfx_ParticleInfo> retiredParticles; // +A0.
	D3DXMATRIX secondTransform; // +AC; high-level target transform.
	DWORD texture; // +EC.
	BYTE ownsTexture;
	DWORD model; // +F4.
	BYTE ownsModel; // +F8; constructor-unwritten, SetModel publishes it.
	DWORD trailTexture; // +FC.
	BYTE ownsTrailTexture, enabled;
	float* emissionTime; // +104; allocated scalar, original free API.
	float emissionRate; // +108; random rate, not an integer count.
	Gfx_ParticleConfiguration configuration; // +10C,1520-byte value.
	float scale; // +6FC.
	SpawnCallback spawnCallback; // +700; constructor-unwritten.
	DeathCallback deathCallback; // +704; constructor-unwritten.
	std::list<Gfx_ModelInstance*> modelInstances; // +708, borrows.
	std::vector<Gfx_LineSystem> lines; // +714, two values.
private:
	void BuildGeometry(Gfx_Camera* camera); // 0x51C320; ParticleSystemDraw.cpp.
	void SortParticlesForDraw(std::list<Gfx_ParticleInfo>& selected); //Shared51B249/51AE69 sort prefix.
	void BuildPointGeometry(Gfx_Camera* camera); //51AE20.
	void BuildModelGeometry(Gfx_Camera* camera); //519E70.
	void BuildQuadGeometry(Gfx_Camera* camera); // 0x51B1E0; ParticleSystemDraw.cpp.
	void RebuildTrails(); //519300, original retired-list copies then5192C0 sampling.
	void SampleTrails(std::list<Gfx_ParticleInfo>& output); //5192C0, active particles into caller output.
	void SampleTrail(Gfx_ParticleInfo& particle,std::list<Gfx_ParticleInfo>& output); //5185D0.
	void BuildAfterimageGeometry(Gfx_ParticleInfo& particle,Gfx_Camera* camera); //519490.
	void GetTrailEndpoints(Gfx_Camera* camera,const Gfx_ParticleInfoBase& particle,
		D3DXVECTOR3& first,D3DXVECTOR3& last,float time) const; //513BF0, camera unused; RET20.
	void BuildRibbonGeometry(Gfx_ParticleInfo& particle,Gfx_Camera* camera); //518C60.
	struct SystemValues
	{
		float fraction;
		LONG red, green, blue;
		float scale, alpha;
		D3DXVECTOR3 minimum, maximum;
	};
	void EvaluateSystem(float time, float& fraction, LONG& red, LONG& green, LONG& blue,
		float& systemScale, float& systemAlpha, float* minimumX, float* minimumY,
		float* minimumZ, float* maximumX, float* maximumY, float* maximumZ) const; // 0x513410, actual13args.
	void UpdateParticle(Gfx_ParticleInfoBase& particle, float time, float systemFraction,
		LONG systemRed, LONG systemGreen, LONG systemBlue, float systemScale,
		float systemAlpha); // 0x515110, actual8args.
	void Emit(float time); // 0x516DD0.
	void AddParticle(D3DXVECTOR3 position, D3DXVECTOR3 velocity,
		float mass, float resistance, LONG red, LONG green, LONG blue, LONG alpha,
		float lifetime, float birthTime, D3DXVECTOR3 rotation,
		D3DXVECTOR3 spin, BYTE rotatable, float size, BYTE initialize); //516A40, RET5C.
	void ClearTrail(Gfx_ParticleInfoBase& particle); // 0x513F60.
	void ClearAfterImages(Gfx_ParticleInfo& particle); // 0x514D30.
	void ClearParticle(Gfx_ParticleInfo& particle); // 0x5154B0.
	void ClearModelInstances(); // 0x514AD0; clears borrowed list nodes.
	void ClearParticles(); // 0x516480.
	void ClearRetired(); // 0x5164E0.
	Gfx_ParticleSystem(const Gfx_ParticleSystem&);
	Gfx_ParticleSystem& operator=(const Gfx_ParticleSystem&);
};
