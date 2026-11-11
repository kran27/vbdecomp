#include "gui/EngineInterface.hpp"
#include "gui/Window.hpp"
#include "gui/Button.hpp"
#include "gui/Event.hpp"
#include "gui/EngineInterfaceServices.hpp"
#include "audio/Sound.hpp"
#include <new>

namespace JE
{
    namespace EngineInterfaceServices
    {
        namespace
        {
            TransientSoundRequest request=NULL;
            void* requestContext=NULL;
        }
        TransientSoundRequest SetTransientSoundRequest(TransientSoundRequest value,void* context)
        {
            TransientSoundRequest previous=request;
            request=value; requestContext=context;
            return previous;
        }
        void RequestTransientSound(const char* name,BYTE flag,DWORD a,DWORD b,DWORD c)
        {
            if (request)
                request(name,flag,a,b,c,requestContext);
            else
            {
                CSound* sound=new(std::nothrow) CSound;
                if (sound)
                {
                    sound->Load(name,flag);
                    sound->Play(a,b,c);
                    //5803E0 drops this pointer. Its bit2 request transfers the
                    //completed voice to the real SoundSystem worker's cleanup.
                }
            }
        }
    }
    // Replace only existing addressed OnEvent definition, keep ctor/dtor/cursor elsewhere.
    int EngineInterface::OnEvent(::GUI::Window* window,::GUI::Event* event) //5803E0.
    {
        const int type=event->type;
        if (type==4 || type==7)
        {
            if (window->type==1)
            {
                const std::string sound=static_cast< ::GUI::Button*>(window)->soundDown;
                if (sound.length()>0)
                {
                    CSound* voice=new CSound;
                    if (voice) { voice->Load(sound.c_str(),0); voice->Play(2,0,0); }
                }
            }
        }
        else if (type==5)
        {
            if (window->type==1 && static_cast< ::GUI::Button*>(window)->GetState()==2)
            {
                const std::string sound=static_cast< ::GUI::Button*>(window)->soundUp;
                if (sound.length()>0)
                {
                    CSound* voice=new CSound;
                    if (voice) { voice->Load(sound.c_str(),0); voice->Play(2,0,0); }
                }
            }
        }
        else if (type==1)
        {
            if (window->type==1)
            {
                ::GUI::Interface::OnEvent(window,event);
                window->OnEvent(event); //580502, real virtual11C before reloading Button state.
                if (static_cast< ::GUI::Button*>(window)->GetState()==2)
                {
                    const std::string sound=static_cast< ::GUI::Button*>(window)->soundDown;
                    if (sound.length()>0)
                    {
                        CSound* voice=new CSound;
                        if (voice) { voice->Load(sound.c_str(),0); voice->Play(2,0,0); }
                    }
                }
                return 1;
            }
        }
        else if (type==3)
        {
            if (window->type==1 && static_cast< ::GUI::Button*>(window)->GetState()==2)
            {
                //Original5805C3 uses+D40 (soundUp), NOT+D5C soundEnter.
                const std::string sound=static_cast< ::GUI::Button*>(window)->soundUp;
                if (sound.length()>0)
                {
                    CSound* voice=new CSound;
                    if (voice) { voice->Load(sound.c_str(),0); voice->Play(2,0,0); }
                }
            }
        }
        return ::GUI::Interface::OnEvent(window,event);
    }
}

