#pragma once

#include "gui/Base.hpp"
#include "gui/HeapTracker.hpp"
#include "gui/Fragment.hpp"
#include <list>
#include <string>
#include <vector>

class Gfx_Viewport;

namespace GUI
{
	class Interface;
	struct Event;

	// Actual RTTI Window : Base@0, HeapTracker@18; primary table66E4D4 has75 slots.
	// H-TYPE/H-RETURN: descriptive names and unconsumed return spellings below
	// are source hypotheses. Every declaration corresponds to an observed slot;
	// unresolved operations stay undefined, with no invented successful lifetime.
	class Window : public Base, public HeapTracker
	{
	public:
		Window(DWORD type); // 4B0810; complete ordinary lifetime recovered separately.
		virtual ~Window(); // +00,4B0A50; compiler deleting/secondary thunks are not manual APIs.
		// +04/+08 inherit real Base::SetPosition/GetPosition; original4209A0/B0 are thunks.
		virtual void Offset(LONG x, LONG y); // +0C,4AF830; H-RETURN.
		virtual void Resize(LONG width, LONG height); // +10,4AF080; H-RETURN.
		// +14 inherits actual Base::GetSize4B27C0.
		virtual bool Contains(LONG x, LONG y); // +18,4AF170; AL consumed.
		virtual BYTE ReadResource(JE::Resources::ResourceFileHandle* file, int version); // +1C,4B10F0.
		virtual BYTE ReadFile(FILE* file, int version); // +20,4B0E10.
		virtual BYTE WriteFile(FILE* file); // +24,4AFC70.
		virtual void Refresh(); // +28,4AFA90; actual update slot; H-RETURN.
		virtual void Draw(); // +2C,4AFB30; H-RETURN.
		virtual float SetDepth(float depth, float increment); // +30,4AFC00; consumed ST0.
		virtual float GetDepth(); // +34,4AF5A0; H-TYPE coherent with actual Fragment.
		virtual void UpdateGeometry(); // +38,4AF5B0; H-RETURN.
		virtual void SetClientPosition(LONG x, LONG y); // +3C,4AF020; H-TYPE/H-RETURN.
		virtual void GetClientPosition(LONG& x, LONG& y); // +40,4AF040; H-RETURN.
		virtual void SetAbsolutePosition(LONG x, LONG y); // +44,4AF060, aliases Base body; H-TYPE.
		virtual void GetAbsolutePosition(LONG& x, LONG& y); // +48,4209B0, aliases Base getter; H-TYPE.
		virtual void SetName(const std::string& value); // +4C,4B02C0; H-RETURN.
		virtual const char* GetName(); // +50,4AF890; actual C-string consumers.
		virtual void SetHidden(BYTE value); // +54,4AF190; raw BYTE/H-RETURN.
		virtual BYTE GetHidden(); // +58,4AF1A0.
		virtual void SetColor(DWORD value); // +5C,4AF1B0; H-RETURN.
		virtual DWORD GetColor(); // +60,4AF1E0.
		virtual void SetAlpha(BYTE value); // +64,4AF1F0; H-RETURN.
		virtual DWORD GetAlpha(); // +68,4AF250.
		virtual void SetValue60(BYTE value); // +6C,4AF260; H-TYPE/H-RETURN.
		virtual BYTE GetValue60(); // +70,4AF270.
		virtual void SetMaterialModes(DWORD value); // +74,4AF290; H-RETURN.
		virtual DWORD GetMaterialModes(); // +78,4AF2D0.
		virtual void SetValue68(BYTE value); // +7C,4AF280; H-TYPE/H-RETURN.
		virtual BYTE GetValue68(); // +80,5C0350.
		virtual void SetFrameVisible(BYTE value); // +84,4AF2E0; H-TYPE/H-RETURN.
		virtual BYTE GetFrameVisible(); // +88,4AF2F0.
		virtual void SetValue3C(BYTE value); // +8C,4AF300; H-TYPE/H-RETURN.
		virtual BYTE GetValue3C(); // +90,4AF310.
		virtual void SetOwner(Interface* value); // +94,4AF320; borrowed owner/H-RETURN.
		virtual Interface* GetOwner(); // +98,4AF380.
		virtual void SetParent(Window* value); // +9C,4AF390; borrowed parent/H-RETURN.
		virtual Window* GetParent(); // +A0,4AF460.
		virtual void AddChild(Window* value); // +A4,4B0CA0; H-RETURN.
		virtual void RemoveChild(Window* value); // +A8,4AFF30; H-RETURN.
		virtual bool ContainsDescendant(Window* value); // +AC,4AF8A0; AL consumed.
		virtual void ClearChildren(); // +B0,4B0000; clears nodes/H-RETURN.
		virtual Window* FindWindow(const std::string& value); // +B4,4B02E0.
		virtual std::list<Window*>* GetChildren(); // +B8,4AF470; borrowed container.
		virtual Window* FindAt(LONG x, LONG y, BYTE overrideSelf); // +BC,4AF8F0; low-byte flag.
		virtual void BringChildToFront(Window* value); // +C0,4B0D50; H-RETURN.
		virtual void BringToFront(); // +C4,4AF480; owner/parent focus/order chain, H-RETURN.
		virtual void ApplyProfile(const std::string& skin, const std::string& profile); // +C8,4B06C0; H-RETURN.
		virtual void ApplySkin(const std::string& skin); // +CC,4B0480; H-TYPE/H-RETURN.
		virtual void GetSkin(std::string& value); // +D0,4B06A0; H-RETURN.
		virtual void SetString(int index, const std::string& value); // +D4,4B13F0; H-RETURN.
		virtual void GetString(int index, std::string& value); // +D8,4B0520; H-RETURN.
		virtual void ClearString(int index); // +DC,4B05A0; H-TYPE.
		virtual void SetViewport(Gfx_Viewport* value); // +E0,4AF980; H-RETURN.
		virtual Gfx_Viewport* GetViewport(); // +E4,4AF4C0.
		virtual void SetValue3B(BYTE value); // +E8,4AF4D0; H-TYPE/H-RETURN.
		virtual BYTE GetValue3B(); // +EC,4AF4E0.
		virtual void Scale(float x, float y); // +F0,4AF9C0; H-RETURN.
		virtual void Center(); // +F4,4AF4F0; H-RETURN.
		virtual void SetValueCD4(DWORD value); // +F8,4AF580; H-TYPE/H-RETURN.
		virtual void SetStringCB8(const std::string& value); // +FC,4B0640; H-TYPE/H-RETURN.
		virtual void SetValueCF4(DWORD value); // +100,4AF590; H-TYPE/H-RETURN.
		virtual void SetStringCD8(const std::string& value); // +104,4B0660; H-TYPE/H-RETURN.
		virtual void SetStringCF8(const std::string& value); // +108,4B0680; H-TYPE/H-RETURN.
		virtual std::string GetStringCB8(); // +10C,4B0040; hidden result initializes real string/H-TYPE.
		virtual std::string GetStringCD8(); // +110,4B0160; hidden result initializes real string/H-TYPE.
		virtual std::string GetStringCF8(); // +114,4B0280; hidden result initializes real string/H-TYPE.
		virtual void OnCaptureChanged(Window* next); // +118,shared5A9320; H-TYPE/H-RETURN.
		virtual BOOL OnEvent(Event* event); // +11C,4AFFA0; full EAX consumed/H-TYPE.
		virtual void SetCenterVisible(BYTE value); // +120,4AF810; H-TYPE/H-RETURN.
		virtual BYTE GetCenterVisible(); // +124,4AF820.
		virtual DWORD GetDepthSlots(); // +128,420990; full EAX count consumed/H-TYPE.

		std::string name; // +1C.
		BYTE hidden, frameVisible, centerVisible, value3B, value3C; // +38..3C; pad3D..3F unwritten.
		std::string skin; // +40; actual named-profile source, name H-TYPE.
		DWORD color; // +5C.
		BYTE value60; // +60; padding61..63 unwritten.
		DWORD materialModes; // +64.
		BYTE value68; // +68; padding69..6B unwritten.
		Interface* owner; // +6C, borrowed.
		Window* parent; // +70, borrowed.
		RECT clientRectangle; // +74.
		std::list<Window*> children; // +84; real container, selective child ownership.
		Fragment fragments[9]; // +90..CA7; nine actual344-byte members.
		std::vector<std::string> strings; // +CA8..CB7; actual string elements, not raw triplet.
		std::string stringCB8; // +CB8..CD3; actual allocator atCB8, payloadCBC.
		DWORD valueCD4; // +CD4.
		std::string stringCD8; // +CD8..CF3.
		DWORD valueCF4; // +CF4.
		std::string stringCF8; // +CF8..D13.
		Gfx_Viewport* viewport; // +D14, borrowed.
		DWORD type; // +D18; complete original x86 sizeof3356.
	};
}
