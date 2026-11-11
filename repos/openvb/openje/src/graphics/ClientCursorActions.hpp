#pragma once

#include <d3dx8.h>

namespace F3
{
	// Borrowed outer Client/GUI requests used by the recovered cursor methods.
	// Defaults retain required children as addressed errors. A preview may capture
	// an unconsumed UI request explicitly while sending attacks over real peers.
	class GameCursorActionServices
	{
	public:
		virtual ~GameCursorActionServices() {}
		virtual void Feedback(const char* text); //434B70.
		virtual void SelfSpeech(DWORD entityID, const char* text, DWORD duration,
			BYTE red, BYTE green, BYTE blue); //587A60.
		virtual void SendAttack(DWORD actorID, DWORD targetID, DWORD part,
			const D3DXVECTOR3* point); //45F370.
		virtual void SendPickup(DWORD actorID, DWORD targetID); //45FDB0, reliable packet82.
		virtual void SendOperate(DWORD actorID, DWORD targetID); //45FA30, reliable packet78.
		virtual void SendUse(DWORD actorID, DWORD targetID); //45F4F0, reliable packet72.
		virtual void SendTalk(DWORD actorID, DWORD targetID); //45F5D0.
		virtual void SendHostile(DWORD actorID, DWORD targetID); //45F6B0, reliable74; original server only resolves IDs.
		virtual void OpenSkillMenu(DWORD actorID, DWORD targetID, LONG x, LONG y); //45EF70.
		virtual void OpenContextMenu(DWORD actorID, DWORD targetID, LONG x, LONG y); //587A80.
		virtual void Tooltip(DWORD entityID); //436840, request only until real GUI is bound.
	};
}
