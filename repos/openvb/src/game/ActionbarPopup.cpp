#include "game/ActionbarInterface.hpp"
#include "game/ActionbarServices.hpp"
#include "game/GameInventory.hpp"
#include "game/GameWeapon.hpp"
#include "game/GameCreature.hpp"
#include "gui/Button.hpp"
#include "gui/Picture.hpp"
#include "gui/Label.hpp"
#include <cstdio>

namespace F3
{
	void ActionbarInterface::ShowAttackModes(DWORD index) //419C70.
	{
		LONG rootX,rootY,weaponX,weaponY,width,height;
		rootWindow->GetAbsolutePosition(rootX,rootY);
		weaponButtons[index]->GetAbsolutePosition(weaponX,weaponY);
		weaponButtons[index]->GetSize(width,height);
		std::string image;
		weaponIconsUp[index]->GetTextureName(image);
		const BYTE alpha=BYTE(weaponIconsUp[index]->GetAlpha());
		const DWORD modes=weaponIconsUp[index]->GetMaterialModes();
		GameInventory* inventory=GetInventory();
		if (!inventory) return;
		GameItem* item=NULL;
		if (index==0) item=inventory->GetItem(2);
		else if (index==1) item=inventory->GetItem(3);
		GameWeapon* weapon=dynamic_cast<GameWeapon*>(item);
		//Original also resolves the bound Creature and the published GameClient;
		//neither result gates popup population.
		dynamic_cast<GameCreature*>(JE::GetEntityPtr(entityID));
		if (!weapon) return;
		for (LONG i=1;i<=6;++i)
		{
			AttackModeIterator current=weapon->currentAttackMode;
			AttackModeIterator nextMode=weapon->PeekAttackMode(i);
			ActionbarServices::GetClient();
			if (nextMode==current) break;
			AttackMode* next=nextMode==AttackModeIterator() ? NULL : &*nextMode;
			ActionbarPopUpButton* popup=popups[i-1];
			popup->button->SetHidden(0);
			//Physical stack operands, after real Window/SDK RET cleanup:
			//rootY - weaponHeight*i, at weaponX. HexRays mislabels these locals.
			popup->button->SetAbsolutePosition(weaponX,rootY-height*i);
			popup->iconUp->LoadTexture(image,0,0.0f,0.0f,-1.0f,-1.0f,1);
			popup->iconDown->LoadTexture(image,0,0.0f,0.0f,-1.0f,-1.0f,1);
			popup->iconUp->SetAlpha(alpha);
			popup->iconDown->SetAlpha(alpha);
			popup->iconUp->SetMaterialModes(modes);
			popup->iconDown->SetMaterialModes(modes);
			popup->modeUp->SetStringReference(next ? next->nameRef : 389);
			popup->modeDown->SetStringReference(next ? next->nameRef : 389);
			char text[28]; std::sprintf(text,"AP%d",next ? next->actionPoints : 6);
			popup->apUp->SetText(text);
			popup->apDown->SetText(text);
			popup->calledShotUp->SetHidden(next ? !next->calledShot : 1);
			popup->calledShotDown->SetHidden(next ? !next->calledShot : 1);
		}
	}
}
