#include "GameAmmo.hpp"
#include "GamePlayer.hpp"
#include "entity/Effect.hpp"
#include "entity/EntityEnvironment.hpp"
#include "entity/World.hpp"
#include "model/CommonException.hpp"
#include <stdexcept>

namespace F3
{
	void GameEntity::CopyFrom(JE::Entity* source) //468350.
	{
		GameEntity* game=dynamic_cast<GameEntity*>(source);
		if (game)
		{
			maximumHealth=game->maximumHealth;
			currentHealth=game->currentHealth;
			for (DWORD event=0;event<5;++event)
				for (std::list<JE::Effect*>::iterator i=game->effects[event].begin();
					i!=game->effects[event].end();++i)
				{
					JE::Effect* original=*i;
					if (original->effectID) continue; //4683B6 copies only unassigned templates.
					if (!(JE::GetEntityEnvironment().NetworkMode()&1))
						throw ::Common::Exception("Invalid attempt to retrieve world server instance.");
					JE::Effect* effect=JE::g_World->NewEffect(original->type);
					effect->CopyFrom(*original);
					effects[event].push_back(effect); //Append; no activation/owner change or destination clear.
				}
		}
		JE::Entity::CopyFrom(source); //The original qualified base call is unconditional.
	}

	void GameCreature::CopyFrom(JE::Entity* source) //40ACC0.
	{
		GameCreature* creature=dynamic_cast<GameCreature*>(source);
		if (creature)
		{
			portrait=creature->portrait;
			value294=creature->value294;
			value298=creature->value298;
			value2B4=creature->value2B4;
			materialMaskMode=creature->materialMaskMode;
			for (DWORD i=0;i<12;++i) materialNames[i]=creature->materialNames[i];
			for (DWORD i=0;i<13;++i)
			{
				sockets[i].model=creature->sockets[i].model;
				sockets[i].texture=creature->sockets[i].texture;
			}
			//The original copies these raw bytes in this straight-line order.
			socketFlags[0]=creature->socketFlags[0];
			socketFlags[1]=creature->socketFlags[1];
			socketFlags[2]=creature->socketFlags[2];
			socketFlags[3]=creature->socketFlags[3];
			socketFlags[4]=creature->socketFlags[4];
			socketFlags[5]=creature->socketFlags[5];
			socketFlags[6]=creature->socketFlags[6];
			socketFlags[7]=creature->socketFlags[7];
			socketFlags[8]=creature->socketFlags[8];
			socketFlags[9]=creature->socketFlags[9];
			socketFlags[10]=creature->socketFlags[10];
			socketFlags[11]=creature->socketFlags[11];
		}
		GameEntity::CopyFrom(source);
	}

	void GamePlayer::CopyFrom(JE::Entity* source) //411C50, shared implementation411C00.
	{
		GamePlayer* player=dynamic_cast<GamePlayer*>(source);
		if (player) characterName=player->characterName;
		GameCreature::CopyFrom(source);
	}

	void GameItem::CopyFrom(JE::Entity* source) //411330.
	{
		GameItem* item=dynamic_cast<GameItem*>(source);
		if (item)
		{
			value160=item->value160;
			value164=item->value164;
			values168=item->values168; //Real list assignment4112C0, unlike effect-list append.
			socketModel1 = item->socketModel1;
			socketTexture1 = item->socketTexture1;
			hideHair = item->hideHair;
			hideBeard = item->hideBeard;
			hideMoustache = item->hideMoustache;
			hideEyes = item->hideEyes;
			hidePonytail = item->hidePonytail;
			socketModel2 = item->socketModel2;
			socketTexture2 = item->socketTexture2;
			socketFlag2 = item->socketFlag2;
			socketModel5 = item->socketModel5;
			socketTexture5 = item->socketTexture5;
			socketFlag5 = item->socketFlag5;
			socketModel4 = item->socketModel4;
			socketTexture4 = item->socketTexture4;
			socketFlag4 = item->socketFlag4;
			socketModel6 = item->socketModel6;
			socketTexture6 = item->socketTexture6;
			socketFlag6 = item->socketFlag6;
			socketModel3 = item->socketModel3;
			socketTexture3 = item->socketTexture3;
			socketFlag3 = item->socketFlag3;
			socketModel7 = item->socketModel7;
			socketTexture7 = item->socketTexture7;
			socketModel10 = item->socketModel10;
			socketTexture10 = item->socketTexture10;
			socketModel8 = item->socketModel8;
			socketTexture8 = item->socketTexture8;
			hideVanity = item->hideVanity;
			socketModel9 = item->socketModel9;
			socketTexture9 = item->socketTexture9;
			socketModel11 = item->socketModel11;
			socketTexture11 = item->socketTexture11;
			socketModel12 = item->socketModel12;
			socketTexture12 = item->socketTexture12;
			socketFlag12 = item->socketFlag12;
			heldModel=item->heldModel;
			heldTexture=item->heldTexture;
		}
		GameEntity::CopyFrom(source);
	}

	void GameAmmo::CopyFrom(JE::Entity* source) //Raw body408200..40829F; adjustor4082C0.
	{
		GameAmmo* ammo=dynamic_cast<GameAmmo*>(source);
		if (ammo)
		{
			entityClass=ammo->entityClass;
			caliber=ammo->caliber;
			modifier=ammo->modifier;
			value952=ammo->value952;
			value956=ammo->value956;
			value960=ammo->value960;
			value964=ammo->value964;
			value968=ammo->value968;
			value972=ammo->value972;
			value976=ammo->value976;
		}
		GameItem::CopyFrom(source);
	}
}
