#include "Interface.hpp"
#include "Window.hpp"

namespace GUI
{
    void Interface::RefreshPositions()
    {
        for (std::list<Window*>::iterator entry = roots.begin(); entry != roots.end(); ++entry)
        {
            (*entry)->Offset(0, 0);
        }
    }

    void Interface::Update()
    {
        if (!hidden)
        {
            BeforeUpdate();
            // std::list<Window*>::iterator entry = roots.end();
            //--entry;
            // while (entry != roots.end())
            //{
            //	(*entry)->Refresh();
            //	--entry;
            // }
            auto entry = roots.end();

            while (entry != roots.begin())
            {
                --entry;
                (*entry)->Refresh();
            }
        }
    }

    void Interface::Draw()
    {
        if (!hidden)
        {
            BeforeDraw();
            // std::list<Window*>::iterator entry = roots.end();
            //--entry;
            // while (entry != roots.end())
            //{
            //	(*entry)->Draw();
            //	--entry;
            // }
            auto entry = roots.end();

            while (entry != roots.begin())
            {
                --entry;
                (*entry)->Draw();
            }
        }
    }
} // namespace GUI
