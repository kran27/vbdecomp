#include <winsock2.h>
#include "ActionFloatingSpeechText.hpp"
#include "Entity.hpp"
#include "network/EngineServer.hpp"
#include "startup/EngineStringSystem.hpp"
#include "startup/NetworkComponent.hpp"

namespace JE
{
	DWORD FloatingSpeechDuration(DWORD length)
	{
		//Original0.1 is a FLOAT constant, promoted during x87 evaluation.
		return DWORD(static_cast<__int64>((double(length)*double(0.1f)+1.0)*1000.0));
	}
	ActionFloatingSpeechText::ActionFloatingSpeechText()
		: red(128),green(128),blue(128),completed(0),sent(0) {}
	BYTE ActionFloatingSpeechText::IsComplete() const { return completed; }
	void ActionFloatingSpeechText::Cancel() {}
	int ActionFloatingSpeechText::Start(DWORD) { return 0; }
	int ActionFloatingSpeechText::Finish(DWORD) { return 0; }
	int ActionFloatingSpeechText::Update(DWORD time,DWORD)
	{
		if (!sent)
		{
			Server* server=static_cast<Server*>(F3::g_pNetworkInstance->server);
			server->FloatingSpeechText(owner->entityID,stringReference,duration,red,green,blue);
			if (!duration)
			{
				std::string text;
				F3::GetString(stringReference,text); //Actual56C280 including token resolution.
				duration=DWORD((DWORD(text.size())*0.1f+1.0f)*1000.0f);
			}
			deadline=time+duration;
			sent=1;
		}
		if (time>deadline) SetCompleted(1); //Strict unsigned comparison, original wrap arithmetic.
		return 0;
	}
	void ActionFloatingSpeechText::SetCompleted(BYTE value)
	{
		completed=value;
	}
}
