#include "game/ActionbarInterface.hpp"
#include "game/GameEntity.hpp"
#include "gui/Button.hpp"
#include "gui/Label.hpp"
#include "gui/Picture.hpp"

namespace F3
{
    void ActionbarInterface::SetPartyMember(LONG index, DWORD id) //417370.
    {
        //Original index is unchecked; normal callers supply one of the6 slots.
        if (!partyPictures[index]) return;
        if (!id)
            partyPictures[index]->SetHidden(1);
        else
        {
            JE::Entity* source = JE::GetEntityPtr(id);
            GameEntity* entity = dynamic_cast<GameEntity*>(source);
            if (entity)
            {
                partyPictures[index]->SetHidden(0);
                partyNames[index]->SetText(entity->GetDisplayName());
            }
            //Original failed lookup/cast preserves previous visibility/text.
        }
    }

    void ActionbarInterface::SetSelectedPartyMember(LONG selected) //417A50.
    {
        for (LONG index = 0; index < 6; ++index)
        {
            if (index == selected)
                partyPictures[index]->LoadTexture("f3_actionbar_PC_up.tga",
                    0, 0.0f, 0.0f, -1.0f, -1.0f, 1);
            else
                partyPictures[index]->LoadTexture("f3_actionbar_PC_down.tga",
                    0, 0.0f, 0.0f, -1.0f, -1.0f, 1);
        }
    }

    void ActionbarInterface::ClearStatusButtons() //416FB0.
    {
        popupValue290 = 0;
        popupValue294 = 0;
        for (LONG index = 0; index < 6; ++index)
            popups[index]->button->SetHidden(1);
    }
}
