#include "gui/DialogueWindow.hpp"
#include "gui/DialogueInterface.hpp"
#include "gui/ReplyLabel.hpp"
#include "gui/Label.hpp"
#include "gui/Picture.hpp"
#include "gui/Manager.hpp"
#include "graphics/Viewport.hpp"
#include "graphics/CameraController.hpp"
#include "graphics/RenderManager.hpp"
#include "graphics/Light.hpp"

namespace JE
{
	namespace
	{
		DWORD ScaleCoordinate(LONG value, float scale)
		{
			return DWORD(static_cast<LONGLONG>(double(value) * scale));
		}
	}

	void DialogueWindow::BuildLayout() //593660.
	{
		const DialogueAppearance& s = dialogueInterface->speaker;
		const DialogueAppearance& l = dialogueInterface->listener;
		const LONG minimumHeight = s.modelHeight + 2 * s.modelY;
		Resize(s.windowWidth, s.windowHeight < minimumHeight ? minimumHeight : s.windowHeight);
		ApplySkin(s.windowSkin);
		SetAbsolutePosition(256, 192);
		SetAlpha(BYTE(s.windowAlpha));
		SetName(std::string("Dialogue Window"));
		SetHidden(1);

		speakerName = new ::GUI::Label(3);
		AddChild(speakerName);
		speakerName->Resize(s.nameLabelWidth, s.nameLabelHeight);
		speakerName->ApplySkin(s.nameLabelSkin);
		speakerName->SetFont(s.nameLabelFontName, s.nameLabelFontFlags, s.nameLabelFontSize, 1);
		speakerName->SetTextColor(0xFF808080);
		speakerName->SetText(std::string("NPC Name"));
		speakerName->SetClientPosition(s.nameLabelX, s.nameLabelY);
		speakerName->SetWrap(0);

		speakerPicture = new ::GUI::Picture(2);
		AddChild(speakerPicture);
		speakerPicture->ApplySkin(s.portraitSkin);
		speakerPicture->SetFrameVisible(s.portraitFrameVisible);
		speakerPicture->Resize(s.portraitWidth, s.portraitHeight);
		speakerPicture->SetClientPosition(s.portraitX, s.portraitY);
		speakerPicture->LoadTexture(std::string("icon_unknown.bmp"), 0, 0, 0, -1, -1, 1);

		speakerText = new ::GUI::Label(3);
		AddChild(speakerText);
		speakerText->Resize(s.textLabelWidth, s.textLabelHeight);
		speakerText->ApplySkin(s.textLabelSkin);
		speakerText->SetFont(s.textLabelFontName, s.textLabelFontFlags, s.textLabelFontSize, 1);
		speakerText->SetTextColor(0xFF808080);
		speakerText->SetText(std::string(""));
		speakerText->SetClientPosition(s.textLabelX, s.textLabelY);
		speakerText->SetWrap(1);

		listenerWindow = new ::GUI::Window(0);
		AddChild(listenerWindow);
		listenerWindow->Resize(l.windowWidth, l.windowHeight);
		listenerWindow->ApplySkin(l.windowSkin);
		listenerWindow->SetClientPosition(0, s.windowHeight);
		listenerWindow->SetAlpha(BYTE(l.windowAlpha));

		listenerName = new ::GUI::Label(3);
		listenerWindow->AddChild(listenerName);
		listenerName->Resize(l.nameLabelWidth, l.nameLabelHeight);
		listenerName->ApplySkin(l.nameLabelSkin);
		listenerName->SetFont(l.nameLabelFontName, l.nameLabelFontFlags, l.nameLabelFontSize, 1);
		listenerName->SetTextColor(0xFF808080);
		listenerName->SetText(std::string("PC Name"));
		listenerName->SetClientPosition(l.nameLabelX, l.nameLabelY);
		listenerName->SetWrap(0);

		listenerPicture = new ::GUI::Picture(2);
		listenerWindow->AddChild(listenerPicture);
		listenerPicture->ApplySkin(l.portraitSkin);
		listenerPicture->SetFrameVisible(l.portraitFrameVisible);
		listenerPicture->Resize(l.portraitWidth, l.portraitHeight);
		listenerPicture->SetClientPosition(l.portraitX, l.portraitY);
		listenerPicture->LoadTexture(std::string("portrait_elf.bmp"), 0, 0, 0, -1, -1, 1);

		for (int i = 0; i < 10; ++i)
		{
			replies[i] = new ReplyLabel;
			listenerWindow->AddChild(replies[i]);
			replies[i]->Resize(l.textLabelWidth, l.textLabelHeight);
			replies[i]->ApplySkin(l.textLabelSkin);
			replies[i]->SetFont(l.textLabelFontName, l.textLabelFontFlags, l.textLabelFontSize, 1);
			replies[i]->SetTextColor(0xFF808080);
			replies[i]->SetText(std::string(""));
			replies[i]->SetClientPosition(l.textLabelX, l.textLabelY);
			replies[i]->SetWrap(1);
			replies[i]->SetHidden(1);
			replies[i]->SetDialogueWindow(this);
		}

		portraitWindow = new ::GUI::Window(0);
		AddChild(portraitWindow);
		portraitWindow->ApplySkin(s.modelSkin);
		portraitWindow->SetFrameVisible(s.modelFrameVisible);
		portraitWindow->Resize(s.modelWidth, s.modelHeight);
		portraitWindow->SetClientPosition(s.modelX, s.modelY);

		portraitViewport = new Gfx_Viewport;
		portraitOrbit = new Gfx_CamCtrl_Orbit;
		const LONG modelX = s.modelX, modelY = s.modelY;
		const LONG modelWidth = s.modelWidth, modelHeight = s.modelHeight;
		float sx, sy;
		::GUI::g_Manager.GetScale(sx, sy);
		D3DVIEWPORT8 descriptor;
		descriptor.Height = ScaleCoordinate(modelHeight, sy);
		descriptor.Width = ScaleCoordinate(modelWidth, sx);
		descriptor.Y = ScaleCoordinate(modelY, sy);
		descriptor.X = ScaleCoordinate(modelX, sx);
		Gfx_Viewport* viewport = portraitViewport;
		if (!descriptor.Width) descriptor.Width = viewport->viewport.Width;
		if (!descriptor.Height) descriptor.Height = viewport->viewport.Height;
		descriptor.MinZ = 0.0f;
		descriptor.MaxZ = 1.0f;
		viewport->viewport = descriptor;
		const float ratio = float(descriptor.Width) / float(descriptor.Height);
		viewport->aspect = ratio;
		viewport->camera.aspect = ratio;
		viewport->camera.projectionDirty = 1;
		viewport->RebuildQuad();
		viewport->renderer->SetViewportQuad(viewport->viewport.X, viewport->viewport.Y,
			viewport->viewport.Width, viewport->viewport.Height);
		portraitViewport->clearColor = 0;
		for (int i = 0; i < 4; ++i) portraitViewport->vertices[i].diffuse = 0;
		portraitViewport->SetCameraController(portraitOrbit);

		D3DLIGHT8 lightDescriptor;
		lightDescriptor.Type = D3DLIGHT_DIRECTIONAL;
		lightDescriptor.Diffuse.r = 1.0f;
		lightDescriptor.Diffuse.g = 1.0f;
		lightDescriptor.Diffuse.b = 1.0f;
		lightDescriptor.Diffuse.a = 1.0f;
		lightDescriptor.Specular.r = 0.0f;
		lightDescriptor.Specular.g = 0.0f;
		lightDescriptor.Specular.b = 1.0f;
		lightDescriptor.Specular.a = 1.0f;
		lightDescriptor.Ambient.r = 1.0f;
		lightDescriptor.Ambient.g = 1.0f;
		lightDescriptor.Ambient.b = 1.0f;
		lightDescriptor.Ambient.a = 1.0f;
		lightDescriptor.Position.x = 0.0f;
		lightDescriptor.Position.y = 0.0f;
		lightDescriptor.Position.z = 0.0f;
		lightDescriptor.Direction.x = 0.0f;
		lightDescriptor.Direction.y = 0.0f;
		lightDescriptor.Direction.z = 1.0f;
		lightDescriptor.Range = 1500.0f;
		lightDescriptor.Falloff = 1.0f;
		lightDescriptor.Attenuation0 = 1.0f;
		lightDescriptor.Attenuation1 = 0.01f;
		lightDescriptor.Attenuation2 = 0.0f;
		lightDescriptor.Theta = 0.0f;
		lightDescriptor.Phi = 0.0f;
		portraitLight = new Gfx_Light(lightDescriptor, 0);
		portraitLight->flag48 = 0;
		portraitLight->transform._41 = 0.0f;
		portraitLight->transform._42 = 100.0f;
		portraitLight->transform._43 = 0.0f;
		portraitLight->light.Position.x = 0.0f;
		portraitLight->light.Position.y = 100.0f;
		portraitLight->light.Position.z = 0.0f;
	}
}

