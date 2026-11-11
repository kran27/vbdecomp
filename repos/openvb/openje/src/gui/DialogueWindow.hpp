#pragma once

#include "gui/Window.hpp"
#include <map>
#include <string>

class Gfx_Viewport;
class Gfx_CamCtrl_Orbit;
class Gfx_Light;
class Gfx_ModelInstance;
class Gfx_AnimationObject;
class Gfx_AnimationInstance;

namespace GUI { class Label; class Picture; }

namespace JE
{
	class DialogueInterface;
	class ReplyLabel;
	namespace Net { class Packet; }

	// Actual60-byte member snapshot: ordinary map plus original fixed reply order.
	// The default constructor initializes only the real map. Scalar/order storage
	// is unwritten until the original producer writes or copies it.
	struct DialogueWindowMember
	{
		DialogueWindowMember() {}
		DWORD textID;
		LONG replyCount;
		std::map<DWORD, DWORD> replies;
		DWORD replyOrder[10];
	};

	class DialogueWindow : public ::GUI::Window
	{
	public:
		DialogueWindow(BYTE id, BYTE revision, DialogueInterface* source); //596610.
		virtual ~DialogueWindow(); //596450; real compiler supplies596760/596600 glue.
		virtual void SetParticipants(DWORD listenerID, DWORD speakerID); //Added slot12C595010.
		void ClearMembers(); //596010.
		void SetDialogueFlag(BYTE flag); //591C50, raw BYTE.
		void SetMember(DWORD entityID, DWORD textID); //595C40.
		void AddReply(DWORD entityID, DWORD replyID, DWORD textID); //595DB0.
		void SelectReply(DWORD replyID); //5923F0, sibling DialogueWindowInput.cpp.
		void UpdatePortrait(); //591C60, actual animation clock/viewport submission.
		void QueuePortrait(); //591E40, actual model lights/submission.
		void FormatLine(std::string& output, std::string& input,
			DWORD participantID, LONG number); //5945F0; participantID unconsumed.

		::GUI::Label* speakerText; //D1C, owned by Window child graph.
		ReplyLabel* replies[10]; //D20..D44, owned by listenerWindow.
		::GUI::Window* listenerWindow; //D48, owned child.
		::GUI::Label* speakerName; //D4C.
		::GUI::Picture* speakerPicture; //D50.
		::GUI::Label* listenerName; //D54.
		::GUI::Picture* listenerPicture; //D58.
		::GUI::Window* portraitWindow; //D5C.
		BYTE dialogueID, dialogueFlag; //D60/D61.
		std::map<DWORD, DialogueWindowMember> members; //D64.
		DWORD listenerID; //D70.
		DialogueInterface* dialogueInterface; //D74, borrowed original third ctor input.
		Gfx_Viewport* portraitViewport; //D78, owned.
		Gfx_CamCtrl_Orbit* portraitOrbit; //D7C, owned.
		float portraitAzimuth, portraitElevation, portraitDistance; //D80/84/88; elevation degrees.
		Gfx_Light* portraitLight; //D8C, owned.
		DWORD portraitModelDataHandle; //D90, owned graphics registration.
		Gfx_ModelInstance* portraitModel; //D94, owned.
		Gfx_AnimationObject* portraitAnimationObject; //D98, owned.
		Gfx_AnimationInstance* portraitAnimation; //D9C, owned; model borrows it.
		DWORD speakerID, animationTimeOrigin; //DA0/DA4.
		DialogueWindowMember* currentMember; //DA8, owned last layout snapshot.
		std::string lastAnimationName; //DAC..DC7.

	private:
		void BuildLayout(); //593660.
		void PositionToSpeaker(LONG x, LONG y, LONG width, LONG height); //5921B0.
		DialogueWindow(const DialogueWindow&);
		DialogueWindow& operator=(const DialogueWindow&);
	};

	// Actual external owners needed by this source. Root supplies these real
	// bridges; an absent Client/HUD is not represented by successful defaults.
	namespace DialogueWindowServices
	{
		void RequestClose(Net::Packet& packet); //Published Network client->587830.
		void GetScreenRectangle(LONG& x, LONG& y, LONG& width, LONG& height); //Client+A0 virtual20.
	}
}

