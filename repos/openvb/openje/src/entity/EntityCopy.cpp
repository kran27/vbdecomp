#include "startup/NetworkComponent.hpp"
#include "Entity.hpp"
#include "EntityEnvironment.hpp"
#include "Effect.hpp"
#include "World.hpp"
#include "model/CommonException.hpp"
#include <stdexcept>

namespace JE
{
	void Entity::CopyFrom(Entity* source) //552420.
	{
		DynamicObject::CopyFrom(source); //Direct base call, not another virtual dispatch.
		enabled=source->enabled;
		shadow=source->shadow;
		flag17A=source->flag17A;
		value17C=source->value17C;
		displayName=source->displayName;
		resourceName=source->resourceName;
		if (source->inUpdateQueue) AddToUpdateQueue(); //No inverse removal when source flag is0.
		rotation.x=source->rotation.x;
		rotation.y=source->rotation.y;
		rotation.z=source->rotation.z;
		roleName=source->roleName;
		modelName=source->modelName;
		textureOverride=source->textureOverride;
		inventoryTexture=source->inventoryTexture;
		activeTexture=source->activeTexture;
		animationEnabled=source->animationEnabled;
		flag296=source->flag296;
		frozenAnimation=source->frozenAnimation;
		frozenAnimationTime=source->frozenAnimationTime;
		value29C=source->value29C;
		scriptName=source->scriptName;
		if (&variables!=&source->variables) variables=source->variables;
		updateInterval=source->updateInterval; //Direct raw store; not SetUpdateInterval's scheduling callback.
		if ((F3::g_pNetworkInstance->GetMode()&1) && !source->effects.empty())
		{
			//5525DF iterates the live source deque; existing destination entries
			//remain. No snapshots, clear, AddEffect initialization or self-copy
			//special case substitute for the original append/refresh behavior.
			for (std::deque<Effect*>::iterator i=source->effects.begin(); i!=source->effects.end(); ++i)
			{
				Effect* input=*i;
				if (!(F3::g_pNetworkInstance->GetMode()&1))
					throw ::Common::Exception("Invalid attempt to retrieve world server instance.");
				IWorldServer* world=g_World;
				Effect* copied=world->NewEffect(input->type);
				copied->CopyFrom(*input);
				copied->SetOwner(entityID);
				effects.push_back(copied);
				AddToUpdateQueue();
				copied->RefreshTree();
			}
		}
		value368=source->value368;
		value36C=source->value36C;
		selectable=source->selectable;
		if (&scriptValues!=&source->scriptValues) scriptValues=source->scriptValues;
		renderEnabled=source->renderEnabled;
		dialogueEnabled=source->dialogueEnabled;
		count956=source->count956;
		SetSelectable(selectable); //Actual live virtual+44, after all preceding copies.
	}
}
