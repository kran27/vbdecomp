#pragma once

#include "game/GameInterface.hpp"
#include <d3dx8.h>
#include <vector>

class Gfx_Viewport;
class Gfx_CamCtrl_Orbit;
class Gfx_Light;
namespace GUI { class Label; }

namespace F3
{
	class GamePlayer;
	class CounterControl;
	class MessageBoxInterface;
	class NameAgeSexInterface;
	class CharacterGenerationChoices;

	class CharacterGenerationInterface : public GameInterface
	{
		friend class NameAgeSexInterface;
	public:
		CharacterGenerationInterface(); //42A780, actual fixed CharacterGeneration name.
		virtual ~CharacterGenerationInterface(); //42CA70 ordinary phase.
		void Open(); //42F9E0, normal Mainmenu New Game opener.
		virtual int OnEvent(::GUI::Window* window, ::GUI::Event* event); //430750 logical body.
		virtual void BeforeUpdate(); //42C6C0 ->425960.
		virtual void BeforeDraw(); //42C6A0, actual model submission.
		//OnCursor42C6B0 inherits the actual GameInterface policy and adjustment.
		void Commit(); //42A190, validate/save/close then actual RequestNewGame queue.
		void Cancel(); //42A200, actual confirmation or main-menu return.
		void Close(); //424A70, hide/finish counters/delete preview character.
		void Refresh(); //42F5A0, actual complete text/counter/appearance refresh.
		void SetRace(DWORD race, BYTE resetAppearance); //42E870.
		GamePlayer* Character() const { return character; } //Borrowed host inspection only.

		GamePlayer* character; //Original+04, owned preview instance, not a World substitute.
		MessageBoxInterface* messageBox; //+08, allocated/registered helper;dtor omits delete.
		NameAgeSexInterface* nameAgeSex; //+0C, allocated/registered helper;dtor omits delete.
		CounterControl* attributes[7]; //+10..28, owned actual controls.
		CounterControl* attributePoints; //+2C.
		CounterControl* tagSkillPoints; //+30.
		CharacterGenerationChoices* choices[7]; //+34..4C, actual owned list controls.
		std::string outputFilename; //+50, pending character-file overwrite confirmation.
		D3DXVECTOR3 field6C; //+6C, original authored camera/model pose data.
		float field78;
		D3DXVECTOR3 cameraTarget; //+7C, actual424BE0 input.
		float cameraAzimuth, cameraElevation, cameraDistance, cameraFov; //+88..94.
		D3DXVECTOR3 alternateCameraTarget; //+98.
		float alternateAzimuth, alternateElevation, alternateDistance, alternateFov; //+A4..B0.
		const char* selectedResource; //+B4, borrowed actual static template name identity.
		Gfx_Viewport* modelViewport; //+B8, owned real graphics owner.
		Gfx_CamCtrl_Orbit* modelCamera; //+BC, owned; viewport borrows it.
		float modelAzimuth, modelElevation, modelDistance; //+C0..C8.
		DWORD modelTime; //+CC.
		BYTE modelVisible; //+D0.
		Gfx_Light* modelLight; //+D4, owned real lower light.
		BYTE modelDragging; //+D8.
		LONG mouseX, mouseY; //+DC/+E0.
		::GUI::Window* optionsOverlay; //+E4, actual tracked Window attached to this Interface.
		LONG appearancePage, skinTexture, bodyIndex, headIndex; //+E8..F4.
		std::vector<LONG> bodyVariations; //+F8, actual discovered model variation indices.
		std::vector<LONG> headVariations; //+108.
		LONG hairIndex, beardIndex, paletteIndex; //+118..120.

	private:
		LONG RemainingAttributePoints() const; //424C60.
		void RefreshAttributePoints(BYTE immediate); //424CA0.
		void RefreshTagSkillPoints(BYTE immediate); //424CD0.
		void SetAttribute(LONG index, LONG value); //427040.
		void ApplyNameAgeSex(); //426F80, snapshot NAS outputs before character publication.
		void SetNameLabel(const std::string& name); //426C20.
		void SetAgeLabel(LONG age); //426CE0.
		void SetSexLabel(LONG sex); //426E40.
		void RefreshSkills(); //425BB0.
		void RefreshDerived(); //427560.
		void RefreshHealth(); //427910.
		void RefreshArmor(); //427A40.
		void RefreshTraits(); //429B20.
		void RefreshSkill(LONG index); //425520.
		void RefreshTrait(LONG index); //427480.
		void ToggleSkill(LONG index); //425B50.
		void ToggleTrait(LONG index); //429AB0.
		void ShowTooltip(::GUI::Window* window); //4271A0.
		void ApplyCamera(); //424BE0.
		void DiscoverBodyVariations(); //42C6D0.
		void DiscoverHeadVariations(); //42C860.
		void ResetAppearance(); //42F710, race reset then four actual appearance lists.
		void BuildRaceAppearanceList(); //42CDB0.
		void BuildSkinAppearanceList(); //42D130.
		void BuildBodyAppearanceList(); //42D4C0.
		void BuildHeadAppearanceList(); //42D880.
		void BuildHairAppearanceList(); //42DC40.
		void BuildBeardAppearanceList(); //42E070.
		void BuildPaletteAppearanceList(); //42E4A0.
		void BuildAppearanceList(LONG index); //42CDB0..42E4A0.
		void RefreshRaceSelection(); //425000.
		void RefreshSkinSelection(); //425020.
		void RefreshBodySelection(); //425040.
		void RefreshHeadSelection(); //425090.
		void RefreshHairSelection(); //4250E0.
		void RefreshBeardSelection(); //425100.
		void RefreshPaletteSelection(); //425120.
		void SelectAppearance(LONG value); //42FD60 and its actual source family.
		DWORD BodyReference(LONG variation) const; //424D00, signed0..19 or-1.
		DWORD HeadReference(LONG variation) const; //424D20, signed0..2 or7.
		DWORD BodyPalette() const; //424D40.
		DWORD HeadPalette() const; //424D50.
		DWORD HairPalette() const; //424D60.
		DWORD BeardPalette() const; //424D70.
		BYTE CanCommit(); //429DD0.
		BYTE SaveCharacter(); //429E20, actual AL result; writer result itself is ignored.
		void PrepareSave(); //428B60.
		void OnMessageBox(LONG mode, LONG answer); //429900, caller captures actual globals.
		static void MessageBoxCallback(LONG answer); //429DB0.
	};
}
