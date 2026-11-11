#include "VfxWeather.hpp"
#include "VfxWeatherMesh.hpp"
#include "Camera.hpp"
#include "SystemInterface.hpp"

namespace
{
	struct SnowVertex
	{
		float x,y,z;
		DWORD color;
		float u,v;
	};
	void WriteSnowVertex(SnowVertex& output,const D3DXVECTOR3& position,DWORD color,float u,float v)
	{
		output.x=position.x;
		output.y=position.y;
		output.z=position.z;
		output.color=color;
		output.u=u;
		output.v=v;
	}
	double SnowFade(const JE::VFX_SnowData& data,const D3DXVECTOR3& position,float scale)
	{
		double fade=1.0;
		const float margin=float((double(data.emitterYOffset)-data.groundYOffset)*0.1);
		const float topLow=float((double(data.emitterYOffset)-margin)*scale);
		const float topHigh=data.emitterYOffset*scale;
		if (position.y>=topLow && position.y<=topHigh)
			fade=1.0-(double(position.y)-topLow)/(double(topHigh)-topLow);
		const float bottomLow=data.groundYOffset*scale;
		const float bottomHigh=float((double(margin)+data.groundYOffset)*scale);
		if (position.y>=bottomLow && position.y<=bottomHigh)
			fade*=((double(position.y)-bottomLow)/(double(bottomHigh)-bottomLow));
		const double scaledWidth=double(data.emitterSize)*scale;
		const float negativeLow=float(-0.5*scaledWidth);
		const float negativeHigh=float(double(data.emitterSize)*-0.40000001f*scale);
		if (position.x>=negativeLow && position.x<=negativeHigh)
			fade*=((double(position.x)-negativeLow)/(double(negativeHigh)-negativeLow));
		if (position.z>=negativeLow && position.z<=negativeHigh)
			fade*=((double(position.z)-negativeLow)/(double(negativeHigh)-negativeLow));
		const float positiveLow=float((double(data.emitterSize)*0.5-double(data.emitterSize)*0.1)*scale);
		const float positiveHigh=float(scaledWidth*0.5);
		if (position.x>=positiveLow && position.x<=positiveHigh)
			fade*=1.0-(double(position.x)-positiveLow)/(double(positiveHigh)-positiveLow);
		if (position.z>=positiveLow && position.z<=positiveHigh)
			fade*=1.0-(double(position.z)-positiveLow)/(double(positiveHigh)-positiveLow);
		//4E4F00 only fades within these inclusive edge bands. It does not clamp
		//positions outside them, alpha, inverted emitter ranges or zero widths.
		return fade;
	}
}

namespace JE
{
	void VFX_Snow::BuildGeometry(Gfx_Viewport* viewport) //4E4F00.
	{
		Gfx_Camera* camera=WeatherCamera(viewport);
		if (!viewport) (void)g_GfxSystem->DefaultViewport(); //4E4F66, return discarded.
		if (data.maxFlakes<=0)
		{
			if (mesh) mesh->ready=0;
			return;
		}
		if (!mesh)
		{
			mesh=new VFX_WeatherMesh;
			mesh->instanceCount=1;
			mesh->instances=new Gfx_MeshInstance[mesh->instanceCount];
			for (LONG i=0;i<LONG(mesh->instanceCount);++i)
			{
				mesh->instances[i].flag1D=0;
				mesh->instances[i].flag1E=0;
			}
			mesh->vertexCount=4u*DWORD(data.maxFlakes);
			mesh->vertices=::operator new[](24u*mesh->vertexCount);
			mesh->indexCount=6u*DWORD(data.maxFlakes);
			mesh->indices=static_cast<WORD*>(::operator new[](sizeof(WORD)*mesh->indexCount));
			mesh->packet=new Gfx_PacketData;
			mesh->ready=0;
		}
		const D3DXMATRIX cameraMatrix=camera->transform;
		//512D90 copies the three matrix rows; no inverse or replacement basis.
		const D3DXVECTOR3 right(cameraMatrix._11,cameraMatrix._12,cameraMatrix._13);
		const D3DXVECTOR3 up(cameraMatrix._21,cameraMatrix._22,cameraMatrix._23);
		LONG count=LONG(flakes.size());
		if (count>data.maxFlakes) count=data.maxFlakes;
		mesh->vertexCount=DWORD(4*count);
		mesh->indexCount=DWORD(6*count);
		SnowVertex* vertices=static_cast<SnowVertex*>(mesh->vertices);
		DWORD vertex=0,index=0;
		for (std::list<VFX_SnowFlake>::const_iterator flake=flakes.begin();
			flake!=flakes.end() && vertex<DWORD(4*data.maxFlakes);++flake)
		{
			mesh->indices[index++]=WORD(vertex+1);
			mesh->indices[index++]=WORD(vertex);
			mesh->indices[index++]=WORD(vertex+2);
			mesh->indices[index++]=WORD(vertex+1);
			mesh->indices[index++]=WORD(vertex+2);
			mesh->indices[index++]=WORD(vertex+3);
			const double scaledSize=double(flake->size)*scale;
			const float size=float(scaledSize);
			const float left=float(0.5*scaledSize);
			const float rightExtent=float(scaledSize-left);
			const float lower=float(double(size)*0.5);
			const float upper=size-lower;
			const double fade=SnowFade(data,flake->position,scale);
			DWORD alpha=DWORD(static_cast<__int64>(double(flake->alpha)*fade));
			const DWORD color=DWORD(data.color[2]) | (DWORD(data.color[1])<<8)
				| (DWORD(data.color[0])<<16) | (alpha<<24);
			const D3DXVECTOR3& position=flake->position;
			WriteSnowVertex(vertices[vertex++],position-right*left-up*lower,
				color,data.textureUVs[0],data.textureUVs[3]);
			WriteSnowVertex(vertices[vertex++],position+right*rightExtent-up*lower,
				color,data.textureUVs[2],data.textureUVs[3]);
			WriteSnowVertex(vertices[vertex++],position-right*left+up*upper,
				color,data.textureUVs[0],data.textureUVs[1]);
			WriteSnowVertex(vertices[vertex++],position+right*rightExtent+up*upper,
				color,data.textureUVs[2],data.textureUVs[1]);
		}
		FinishWeatherMesh(*mesh,0x142,24,D3DPT_TRIANGLELIST,DWORD(2*count),
			data.renderPhase,19+(data.modulate2x!=0),texture,camera->transform,data.offsetFromCamera,scale);
	}
}
