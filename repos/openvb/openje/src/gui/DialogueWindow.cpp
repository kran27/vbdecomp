#include "gui/DialogueWindow.hpp"
#include "gui/DialogueInterface.hpp"
#include "gui/Label.hpp"
#include "gui/Picture.hpp"
#include "gui/ReplyLabel.hpp"
#include "graphics/Viewport.hpp"
#include "graphics/CameraController.hpp"
#include "graphics/Light.hpp"
#include "graphics/ModelInstance.hpp"
#include "graphics/AnimationObject.hpp"
#include "graphics/AnimationInstance.hpp"
#include "graphics/SystemInterface.hpp"
#include <cstring>

namespace JE
{

	DialogueWindow::DialogueWindow(BYTE id, BYTE revision, DialogueInterface* source)
		: ::GUI::Window(0), portraitViewport(NULL), portraitOrbit(NULL),
		portraitAzimuth(3.1415927410125732421875f), portraitElevation(10.0f),
		portraitDistance(32.0f), portraitLight(NULL), portraitModelDataHandle(0),
		portraitModel(NULL), portraitAnimationObject(NULL), portraitAnimation(NULL),
		speakerID(0), animationTimeOrigin(0), currentMember(NULL)
	{
		dialogueID = id;
		dialogueFlag = revision;
		dialogueInterface = source;
		portraitAnimationObject = new Gfx_AnimationObject;
		listenerID = 0;
		BuildLayout();
	}

	DialogueWindow::~DialogueWindow()
	{
		delete portraitViewport;
		delete portraitOrbit;
		if (portraitLight) { delete portraitLight; portraitLight = NULL; }
		delete portraitModel;
		if (portraitModelDataHandle) g_GfxSystem->ReleaseModelData(&portraitModelDataHandle);
		if (portraitAnimation) { delete portraitAnimation; portraitAnimation = NULL; }
		if (portraitAnimationObject) { delete portraitAnimationObject; portraitAnimationObject = NULL; }
		delete currentMember;
		//Original member order: lastAnimationName then members, followed by real
		//Window cleanup of remaining owned GUI children and normal member owners.
	}

	void DialogueWindow::ClearMembers() { members.clear(); }
	void DialogueWindow::SetDialogueFlag(BYTE flag) { dialogueFlag = flag; }
	void DialogueWindow::SetMember(DWORD id, DWORD textID)
	{
		std::map<DWORD, DialogueWindowMember>::iterator found;
		found = members.find(id);
		if (found == members.end())
		{
			DialogueWindowMember value;
			value.replyCount = 0; //Other raw fields/order remain unwritten before insertion.
			std::pair<std::map<DWORD, DialogueWindowMember>::iterator, bool> result;
			result = members.insert(std::map<DWORD, DialogueWindowMember>::value_type(id, value));
			found = result.first;
		}
		found->second.textID = textID;
	}
	void DialogueWindow::AddReply(DWORD id, DWORD replyID, DWORD textID)
	{
		std::map<DWORD, DialogueWindowMember>::iterator found = members.find(id);
		if (found == members.end())
		{
			DialogueWindowMember value;
			value.replyCount = 0;
			found = members.insert(std::map<DWORD, DialogueWindowMember>::value_type(id, value)).first;
		}
		DialogueWindowMember& value = found->second;
		value.replyOrder[value.replyCount] = replyID; //Original unchecked ten-element capacity.
		value.replyCount = LONG(DWORD(value.replyCount) + 1);
		value.replies.insert(std::make_pair(replyID, textID)); //Original insert, no replacement.
	}
}

