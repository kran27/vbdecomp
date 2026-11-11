#include "Water.hpp"
#include "ParticleSystem.hpp"
#include "TextureManager.hpp"
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <new>

namespace
{
	Gfx_WaterRefraction refractionTable[256]; //Original70A600.
	Gfx_WaterRefraction flatRefractionTable[256]; //Original709A00.
	bool MaskBit(const BYTE* sampleMask,LONG maskStride,LONG x,LONG row)
	{
		LONG index=x/8+row*maskStride;
		//The original performs these raw edge reads without a bounds check.
		return (sampleMask[index] & (1<<(x%8)))!=0;
	}
	double RandomRange(float first, float last)
	{
		float difference = last - first;
		return std::rand() * double(difference) * 0.000030518509f + first;
	}
}

Gfx_Water::Gfx_Water()
	: rippleMinimumLocation(0, 0)
{
	//51E550 constructs its base, strings and real list before these defaults.
	rippleMaximumLocation = D3DXVECTOR2(1, 1);
	textureHandle = 0;
	forceUpdate = 1;
	value2C = 0; value28 = 0; value24 = 0;
	sampleHeight = 0; sampleWidth = 0;
	cellSize = 0;
	samples = NULL; refraction = NULL;
	flagBC = 0; sampleMask = NULL;
	tilesX = 0; tilesZ = 0;
	elapsedTime = 0;
	disturbanceMode = 1;
	updateInterval = 0.025f;
	rippleMinimumAmount = 0.1f; rippleMaximumAmount = 0.2f;
	rippleMinimumInterval = 0.025f; rippleMaximumInterval = 0.025f;
	rippleNext = 0.025f;
	lineMinimumOrigin = D3DXVECTOR2(0, 0);
	lineMaximumOrigin = D3DXVECTOR2(0, 0);
	lineMinimumDirection = D3DXVECTOR2(0, 0);
	lineMaximumDirection = D3DXVECTOR2(0, 0);
	lineMinimumInterval = 0.025f; lineMaximumInterval = 0.025f;
	lineNext = 0.025f;
	orbitRange[6] = 0.025f; orbitRange[7] = 0.025f;
	orbitNext = 0.025f;
	lineMinimumAmount = 1; lineMaximumAmount = 1;
	orbitRange[0] = 1; orbitRange[1] = 1;
	orbitRange[2] = 1; orbitRange[3] = 1;
	orbitRange[4] = 1; orbitRange[5] = 1;
	rippleTime = 0; lineTime = 0; orbitTime = 0; orbitAngle = 0;
	flag174 = 1;
	//ownsTexture, value98/A8/AC, amplitude/inverses and meanHeight stay unwritten.
}

Gfx_Water::~Gfx_Water()
{
	//The original frees samples/mask before releasing the optional texture.
	if (samples) ::operator delete[](samples);
	if (sampleMask)
	{
		::operator delete[](sampleMask);
		sampleMask = NULL;
	}
	if (ownsTexture && textureHandle) g_TextureManager->Release(textureHandle);
}

void Gfx_Water::SetTexture(DWORD handle, BYTE owned)
{
	if (ownsTexture && textureHandle) g_TextureManager->Release(textureHandle);
	textureHandle = handle;
	ownsTexture = owned;
}

void Gfx_Water::SetRegistryName(std::string value) //49EAC0.
{
	registryName = value;
}

void Gfx_Water::SetTextureName(std::string value) //49EB20.
{
	textureName = value;
}

void Gfx_Water::SetSecondaryTextureName(std::string value) //49EB80.
{
	secondaryTextureName = value;
}

//49EA30: both string members are empty before the supplied values are assigned.
Gfx_WaterTexturePatch::Gfx_WaterTexturePatch(LONG sourceX, LONG sourceZ,
	const std::string& texture, const std::string& tile)
{
	x = sourceX; z = sourceZ;
	textureName = texture; tileName = tile;
}

void Gfx_Water::AddTexturePatch(LONG x, LONG z, const std::string& texture, const std::string& tile)
{
	texturePatches.push_back(Gfx_WaterTexturePatch(x, z, texture, tile));
}

void Gfx_Water::SetRippleRange(D3DXVECTOR2 minimumLocation, D3DXVECTOR2 maximumLocation,
	float minimumAmount, float maximumAmount, float minimumInterval, float maximumInterval)
{
	rippleMinimumLocation.x = minimumLocation.x; rippleMinimumLocation.y = minimumLocation.y;
	rippleMaximumLocation.x = maximumLocation.x; rippleMaximumLocation.y = maximumLocation.y;
	rippleMinimumAmount = minimumAmount; rippleMaximumAmount = maximumAmount;
	rippleMinimumInterval = minimumInterval; rippleMaximumInterval = maximumInterval;
}

void Gfx_Water::SetLineRange(D3DXVECTOR2 minimumOrigin, D3DXVECTOR2 maximumOrigin,
	D3DXVECTOR2 minimumDirection, D3DXVECTOR2 maximumDirection,
	float minimumAmount, float maximumAmount, float minimumInterval, float maximumInterval)
{
	lineMinimumOrigin.x = minimumOrigin.x; lineMinimumOrigin.y = minimumOrigin.y;
	lineMaximumOrigin.x = maximumOrigin.x; lineMaximumOrigin.y = maximumOrigin.y;
	lineMinimumDirection.x = minimumDirection.x; lineMinimumDirection.y = minimumDirection.y;
	lineMaximumDirection.x = maximumDirection.x; lineMaximumDirection.y = maximumDirection.y;
	lineMinimumAmount = minimumAmount; lineMaximumAmount = maximumAmount;
	lineMinimumInterval = minimumInterval; lineMaximumInterval = maximumInterval;
}

void Gfx_Water::SetOrbitRange(float minimumRadius, float maximumRadius,
	float minimumSpeed, float maximumSpeed, float minimumAmount,
	float maximumAmount, float minimumInterval, float maximumInterval)
{
	orbitRange[0] = minimumRadius; orbitRange[1] = maximumRadius;
	orbitRange[2] = minimumSpeed; orbitRange[3] = maximumSpeed;
	orbitRange[4] = minimumAmount; orbitRange[5] = maximumAmount;
	orbitRange[6] = minimumInterval; orbitRange[7] = maximumInterval;
}

void Gfx_Water::PrepareSamples()
{
	std::srand(GetTickCount());
	const DWORD sampleCount=DWORD(sampleHeight)*DWORD(sampleWidth);
	if (!samples)
	{
		const size_t bytes = sizeof(Gfx_WaterSample) * sampleCount;
		samples = static_cast<Gfx_WaterSample*>(::operator new(bytes));
		if (!samples) return;
		std::memset(samples,0,bytes);
	}
	const double firstTiles=double(tilesX);
	// Native captures this selector before publishing either inverse scale.
	Gfx_WaterRefraction* capturedRefraction=refraction;
	amplitude = 20.0f;
	inverseWidth = float(1.0 / (firstTiles * cellSize));
	inverseHeight = float(1.0 / (double(tilesZ) * cellSize));
	if (!capturedRefraction)
	{
		refraction = refractionTable;
		for (DWORD index=0; index<256; ++index)
		{
			const double x = index * 0.00390625;
			const float storedX = float(x);
			const double sine = std::sin(std::acos(x));
			const float storedSine = float(sine);
			const double bend = sine * 0.75018758f;
			const float refracted = float(bend);
			refraction[index].x = float(std::cos(std::asin(bend)) / refracted * storedSine - storedX);
			refraction[index].y = -(refracted / storedSine);
			refraction[index].color = 0xFFFFFF;
			//51D975..51D986 deliberately copies entry0 into every flat entry.
			flatRefractionTable[index] = refractionTable[0];
		}
	}
	if (ownsTexture && textureHandle)
	{
		g_TextureManager->Release(textureHandle);
		textureHandle = 0;
	}
}

void Gfx_Water::Reset() //51CC60, shared original chunk inside4EDCE0.
{
	forceUpdate = 1;
	elapsedTime = 0;
	rippleTime = 0;
	lineTime = 0;
	orbitTime = 0;
	if (samples) std::memset(samples,0,sizeof(Gfx_WaterSample)*(sampleWidth*sampleHeight));
	orbitAngle = 0;
}

LONG Gfx_Water::SampleIndex(LONG x, LONG z) const
{
	while (x<0) x+=sampleWidth;
	while (z<0) z+=sampleHeight;
	//51C944 SETZ followed by TEST, rather than the expected byte-and-bit test.
	if (sampleMask
		&& ((sampleMask[x/8 + z*(sampleWidth/8+(sampleWidth%8!=0))] == 0)
			& (1<<(x%8))) != 0) return -1;
	return x%sampleWidth + sampleWidth*(z%sampleHeight);
}

void Gfx_Water::AddSample(LONG x, LONG z, float amount)
{
	if (x>=0 && x<sampleWidth && z>=0 && z<sampleHeight)
	{
		LONG index=SampleIndex(x,z);
		if (index>=0) samples[index].velocity-=amount;
	}
}

void Gfx_Water::AddRipple(LONG x, LONG z, float amount)
{
	float half=amount*0.5f, quarter=half*0.5f;
	AddSample(x,z,amount);
	AddSample(x-1,z,half); AddSample(x+1,z,half);
	AddSample(x,z-1,half); AddSample(x,z+1,half);
	AddSample(x-1,z-1,quarter); AddSample(x+1,z+1,quarter);
	AddSample(x+1,z-1,quarter); AddSample(x-1,z+1,quarter);
}

void Gfx_Water::AddLine(LONG x, LONG z, float amount, float directionX, float directionZ)
{
	LONG index=SampleIndex(x,z);
	if (index>=0) samples[index].velocity-=amount;
	D3DXVECTOR2 direction(-directionZ,directionX);
	D3DXVec2Normalize(&direction,&direction);
	if (direction.x==0.0f && direction.y==0.0f) return;
	// Native rounds both steps and each origin/current coordinate to float32.
	volatile float stepX,stepZ;
	// Equal or unordered magnitudes take the Z-major path.
	if (std::fabs(direction.x)>std::fabs(direction.y))
	{
		stepX=1.0f; stepZ=direction.y/direction.x;
	}
	else { stepZ=1.0f; stepX=direction.x/direction.y; }
	const volatile float initialX=float(x),initialZ=float(z);
	for (LONG pass=0;pass<2;++pass)
	{
		volatile float first=initialX,row=initialZ;
		for (;;)
		{
			if (!pass) { first+=stepX; row+=stepZ; }
			else { first-=stepX; row-=stepZ; }
			if (first<0.0f || sampleWidth<=first || row<0.0f || sampleHeight<=row) break;
			index=SampleIndex(LONG(first),LONG(row));
			if (index>=0) samples[index].velocity-=amount;
		}
	}
}

void Gfx_Water::Update(float elapsed)
{
	if (elapsed>=0.0f) elapsedTime+=elapsed;
	else elapsedTime=rippleTime=lineTime=orbitTime=0;
	switch (disturbanceMode)
	{
	case 1:
		rippleTime+=elapsed;
		if (rippleTime>=rippleNext)
		{
			float x=float(RandomRange(rippleMinimumLocation.x,rippleMaximumLocation.x));
			//The original uses the X range for both coordinates.
			float z=float(RandomRange(rippleMinimumLocation.x,rippleMaximumLocation.x));
			LONG first=LONG(sampleWidth*double(x)),row=LONG(sampleHeight*double(z));
			float strength=float(RandomRange(rippleMinimumAmount,rippleMaximumAmount));
			AddRipple(first,row,strength);
			rippleTime=0; rippleNext=float(RandomRange(rippleMinimumInterval,rippleMaximumInterval));
		}
		break;
	case 2:
		lineTime+=elapsed;
		if (lineTime>=lineNext)
		{
			float x=float(RandomRange(lineMinimumOrigin.x,lineMaximumOrigin.x));
			float z=float(RandomRange(lineMinimumOrigin.x,lineMaximumOrigin.x));
			LONG first=LONG(sampleWidth*double(x)),row=LONG(sampleHeight*double(z));
			float directionX=float(RandomRange(lineMinimumDirection.x,lineMaximumDirection.x));
			float directionZ=float(RandomRange(lineMinimumDirection.y,lineMaximumDirection.y));
			float strength=float(RandomRange(lineMinimumAmount,lineMaximumAmount));
			AddLine(first,row,strength,directionX,directionZ);
			lineTime=0; lineNext=float(RandomRange(lineMinimumInterval,lineMaximumInterval));
		}
		break;
	case 3:
		orbitTime+=elapsed;
		if (orbitTime>=orbitNext)
		{
			float radius=float(RandomRange(orbitRange[0],orbitRange[1]));
			float speed=float(RandomRange(orbitRange[2],orbitRange[3]));
			float strength=float(RandomRange(orbitRange[4],orbitRange[5]));
			orbitAngle=float(double(speed)*orbitTime+orbitAngle);
			while (orbitAngle>=360.0f) orbitAngle-=360.0f;
			float x=sampleWidth*radius,z=sampleHeight*radius;
			AddRipple(LONG(Gfx_SinDegrees(LONG(orbitAngle))*x+sampleWidth*0.5),
				LONG(Gfx_CosDegrees(LONG(orbitAngle))*z+sampleHeight*0.5),strength);
			orbitTime=0; orbitNext=float(RandomRange(orbitRange[6],orbitRange[7]));
		}
		break;
	}
	if (!forceUpdate && elapsedTime<updateInterval) return;
	forceUpdate=0; elapsedTime=0; meanHeight=0;
	const LONG count=sampleWidth*sampleHeight;
	for (LONG row=0;row<sampleHeight;++row)
	{
		LONG before=(row+sampleHeight-1)%sampleHeight*sampleWidth;
		LONG current=row*sampleWidth, after=(row+1)%sampleHeight*sampleWidth;
		for (LONG x=0;x<sampleWidth;++x)
		{
			Gfx_WaterSample& sample=samples[current+x];
			double surrounding;
			if (x<1 || x>sampleWidth-2 || row<1 || row>sampleHeight-2)
			{
				surrounding=0; sample.height=sample.velocity=0;
			}
			else
			{
				LONG left=x-1,right=x+1;
				surrounding=(double(samples[after+right].height)+samples[before+right].height
					+samples[after+left].height+samples[before+left].height)*0.083333336f
					+(double(samples[current+right].height)+samples[after+x].height
					+samples[current+left].height+samples[before+x].height)*0.16666667f;
			}
			double velocity=(surrounding-sample.height)*0.25+sample.velocity*double(0.95999998f);
			sample.velocity=float(velocity);
			meanHeight=float(velocity+meanHeight+sample.height);
		}
	}
	meanHeight/=count;
	const LONG maskStride=sampleWidth/8+(sampleWidth%8!=0);

	for (LONG sampleRow=0;sampleRow<sampleHeight;++sampleRow)
	{
		LONG before=(sampleRow+sampleHeight-1)%sampleHeight*sampleWidth;
		LONG current=sampleRow*sampleWidth,after=(sampleRow+1)%sampleHeight*sampleWidth;
		for (LONG x=0;x<sampleWidth;++x)
		{
			bool advance=sampleMask == NULL;
			if (!advance)
			{
				advance=MaskBit(sampleMask,maskStride,x,sampleRow) || (x+1<sampleWidth && MaskBit(sampleMask,maskStride,x+1,sampleRow))
					|| (sampleRow+1<sampleHeight && MaskBit(sampleMask,maskStride,x,sampleRow+1))
					|| (sampleRow+1<sampleHeight && MaskBit(sampleMask,maskStride,x+1,sampleRow+1)) || MaskBit(sampleMask,maskStride,x-1,sampleRow)
					|| (sampleRow-1<sampleHeight && (MaskBit(sampleMask,maskStride,x,sampleRow-1)||MaskBit(sampleMask,maskStride,x-1,sampleRow-1)))
					|| (sampleRow-1<sampleHeight && MaskBit(sampleMask,maskStride,x+1,sampleRow-1))
					|| (sampleRow+1<sampleHeight && MaskBit(sampleMask,maskStride,x-1,sampleRow+1));
			}
			if (!advance) continue;
			Gfx_WaterSample& sample=samples[current+x];
			sample.height=float(double(sample.velocity)+sample.height-meanHeight);
			LONG left=(x+sampleWidth-1)%sampleWidth,right=(x+1)%sampleWidth;
			double inverse=1.0/cellSize;
			float diagonal=float((double(samples[before+left].height)-samples[after+right].height)*inverse);
			D3DXVECTOR3 normal(diagonal,1,diagonal);
			normal.z=float((double(samples[before+x].height)-samples[after+x].height)*inverse+normal.z);
			double opposite=(double(samples[before+right].height)-samples[after+left].height)*inverse;
			normal.x=float(normal.x-opposite); normal.z=float(normal.z+opposite);
			normal.x=float((double(samples[current+left].height)-samples[current+right].height)*inverse+normal.x);
			D3DXVec3Normalize(&normal,&normal); //51D7B3 writes only the original stack temporary.
		}
	}
	forceUpdate=0;
}
