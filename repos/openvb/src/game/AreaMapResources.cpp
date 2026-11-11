#include "game/AreaMapResources.hpp"
#include "entity/MapFog.hpp"
#include "entity/MapSound.hpp"
#include "graphics/G3DModel.hpp"
#include "graphics/SystemInterface.hpp"
#include "graphics/Texture.hpp"
#include "resources/ResourceSystem.hpp"

BOOL JE::AreaMapNote::LoadTexture() //57C610.
{
	if (this->texture)
	{ g_GfxSystem->DeleteTexture(this->texture); this->texture=0; }
	this->textureHeight=0; this->textureWidth=0;
	DWORD handle=0; LONG width=0,height=0;
	if (!this->name.empty())
	{
		JE::Resources::ResourceData* resource=JE::Resources::g_pResourceSystem->Demand(this->name.c_str(),1);
		if (resource)
		{
			bool dimensionsFromHeader=false;
			const LONG length=LONG(this->name.size());
			if (length>=4 && this->name.c_str()[length-4]=='.'
				&& JE::UpperMapSoundByte(this->name[length-3])=='T'
				&& JE::UpperMapSoundByte(this->name[length-2])=='G'
				&& JE::UpperMapSoundByte(this->name[length-1])=='A')
			{
				G3D_Texture image;
				image.SetWidth(-1);
				if (image.LoadImage(static_cast<const BYTE*>(resource->data),0) || LONG(image.width)>0)
				{ width=LONG(image.width); height=LONG(image.height); dimensionsFromHeader=true; }
			}
			if (!g_GfxSystem->AcquireTextureByName(&handle,this->name))
			{
				if (FAILED(g_GfxSystem->LoadTextureFromBuffer(&handle,this->name,
					static_cast<const BYTE*>(resource->data),resource->size,
					D3DFMT_UNKNOWN,D3DPOOL_MANAGED,1,1,5,0))) handle=0;
			}
			JE::Resources::g_pResourceSystem->ReleaseResource(resource);
			if (!dimensionsFromHeader && handle)
			{
				Gfx_Texture* texture=g_GfxSystem->LookupTexture(handle);
				if (texture)
				{
					const LONG frameCount=texture->frameCount;
					if (frameCount<=0) width=0;
					else width=texture->frames[0].width;
					if (frameCount<=0) height=0;
					else height=texture->frames[0].height;
				}
			}
		}
	}
	this->texture=handle; this->textureWidth=DWORD(width); this->textureHeight=DWORD(height);
	if (LONG(this->value28)<0) this->value28=DWORD(width);
	if (LONG(this->value2C)<0) this->value2C=DWORD(height);
	if (LONG(this->value20)<0) this->value20=DWORD(width/2);
	if (LONG(this->value24)<0) this->value24=DWORD(height/2);
	return handle!=0;
}

