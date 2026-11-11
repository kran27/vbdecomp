#include <winsock2.h>
#include "Video.hpp"
#include "Bink.hpp"
#include "SystemInterface.hpp"
#include "Texture.hpp"
#include "ClientCursor.hpp"
#include "network/EngineClient.hpp"
#include "gui/Picture.hpp"
#include "gui/Event.hpp"
#include "gui/Manager.hpp"
#include "startup/NetworkComponent.hpp"
#include "startup/ConfigurationObject.hpp"
#include <cstring>

VideoInterface::VideoInterface(const std::string& name) : EngineInterface(name), value(0) {} //5B4EB0.
VideoInterface::~VideoInterface() {} //5B4F00.
void VideoInterface::SetValue(DWORD next) { value = next; } //5B4F20.
DWORD VideoInterface::GetValue() const { return value; } //5B4F30.

int VideoInterface::OnEvent(::GUI::Window* window, ::GUI::Event* event) //5B5A60.
{
	if (event->type == 17 && std::strcmp(window->GetName(), "[MovieWindow]") != 0)
	{
		if (static_cast<LONG>(value) > 0 && static_cast<DWORD>(event->x) == value)
		{
			value = 0;
			static_cast<JE::Client*>(F3::g_pNetworkInstance->client)->child0xB4->SetFade(0);
			static_cast<JE::Client*>(F3::g_pNetworkInstance->client)->child0xB4->Stop();
		}
	}
	return JE::EngineInterface::OnEvent(window, event);
}

VideoQueueItem::VideoQueueItem() //5B5220.
{
	filename = std::string("");
	x = 0;
	y = 0;
	window = NULL;
	width = 1024;
	height = 768;
	modal = 1;
	hideCursor = 1;
}

Video::Video() //5B5080.
{
	movie = NULL;
	textureHandle = 0;
	interfaceOwner = NULL;
	background = NULL;
	movieWindow = NULL;
	activeWindow = NULL;
	fadeDuration = 0;
	queue.clear();
}

Video::~Video() //5B50D0.
{
	Video::OnCleanup();
	Cleanup();
}

JE::Configuration::Settings* Video::Initialize() //5B4F40.
{
	OnInitialize();
	return F3::GetSettingsObject();
}

void Video::OnInitialize() {} //404B70, original empty hook.
JE::Configuration::Settings* Video::OnCleanup() { return F3::GetSettingsObject(); } //5B4F70.
void Video::SetFade(DWORD duration) { fadeDuration = duration; } //5B4F50.
void Video::SetInterfaceValue(DWORD value) //5B4F60.
{
	if (interfaceOwner) interfaceOwner->SetValue(value);
}

void Video::Cleanup() //5B4FC0.
{
	if (movie)
	{
		BinkClose(movie);
		movie = NULL;
	}
	while (!queue.empty())
	{
		VideoQueueItem* item = queue.front();
		queue.pop_front();
		if (item) delete item;
	}
	if (textureHandle)
	{
		g_GfxSystem->DeleteTexture(textureHandle);
		textureHandle = 0;
	}
	if (interfaceOwner)
	{
		delete interfaceOwner;
		interfaceOwner = NULL;
	}
}

void Video::PrepareUI() //5B52E0.
{
	BinkSetSoundSystem(BinkOpenDirectSound, 0);
	if (!interfaceOwner) interfaceOwner = new VideoInterface("MovieInterface");
	Gfx_Viewport* viewport = g_GfxSystem->PostViewport();
	if (!background)
	{
		background = new ::GUI::Picture(2);
		background->Resize(1024, 768);
		background->ApplySkin("DefaultInterface.ini");
		background->SetColor(0xFF000000);
		background->SetAlpha(255);
		background->SetFrameVisible(0);
		background->SetHidden(1);
		background->SetViewport(viewport);
		background->SetAbsolutePosition(0, 0);
		interfaceOwner->AddWindow(background);
	}
	if (!movieWindow)
	{
		movieWindow = new ::GUI::Picture(2);
		movieWindow->SetName("[MovieWindow]");
		movieWindow->Resize(1024, 768);
		movieWindow->ApplySkin("DefaultInterface.ini");
		movieWindow->SetFrameVisible(0);
		movieWindow->SetHidden(0);
		movieWindow->SetViewport(viewport);
		movieWindow->SetAbsolutePosition(0, 0);
		background->AddChild(movieWindow);
	}
}

void Video::Play(const char* filename, LONG x, LONG y, LONG width, LONG height,
	BYTE modal, BYTE hideCursor, ::GUI::Picture* window) //5B5640.
{
	if (movie) Stop();
	movie = BinkOpen(filename, 0);
	if (!movie)
	{
		OutputDebugStringA(BinkGetError());
		return;
	}
	SetFade(0);
	SetInterfaceValue(0);
	if (!textureHandle && SUCCEEDED(g_GfxSystem->CreateAndAddTexture(&textureHandle,
		1024, 1024, 1, D3DFMT_X8R8G8B8, D3DPOOL_MANAGED)))
	{
		Gfx_Texture* texture = g_GfxSystem->LookupTexture(textureHandle);
		if (texture)
		{
			std::string frameName("[MovieFrame]");
			texture->name = frameName;
		}
	}
	if (modal)
	{
		background->SetAlpha(255);
		background->SetHidden(0);
		movieWindow->SetHidden(0);
		::GUI::g_Manager.PushModal(interfaceOwner);
		static_cast<JE::Client*>(F3::g_pNetworkInstance->client)->field0x108 = 1;
		activeWindow = movieWindow;
	}
	else
	{
		if (!window)
		{
			background->SetAlpha(0);
			background->SetHidden(0);
			movieWindow->SetHidden(0);
			window = movieWindow;
		}
		activeWindow = window;
	}
	if (activeWindow)
	{
		activeWindow->Resize(width, height);
		activeWindow->SetAbsolutePosition(x, y);
		activeWindow->SetHidden(0);
	}
	if (hideCursor) static_cast<JE::Client*>(F3::g_pNetworkInstance->client)->child0xA4->SetVisible(0);
}

void Video::Stop() //5B5820.
{
	if (movie)
	{
		BinkClose(movie);
		movie = NULL;
		if (!queue.empty())
		{
			VideoQueueItem* item = queue.front();
			queue.pop_front();
			if (item)
			{
				Play(item->filename.c_str(), item->x, item->y, item->width, item->height,
					item->modal, item->hideCursor, item->window);
				SetFade(item->fadeDuration);
				SetInterfaceValue(item->interfaceValue);
				delete item;
				return;
			}
		}
		background->SetHidden(1);
		activeWindow->SetHidden(1);
		static_cast<JE::Client*>(F3::g_pNetworkInstance->client)->child0xA4->SetVisible(1);
		static_cast<JE::Client*>(F3::g_pNetworkInstance->client)->field0x108 = 0;
		if (static_cast<LONG>(fadeDuration) > 0)
			static_cast<JE::Client*>(F3::g_pNetworkInstance->client)->FadeIn(fadeDuration, 0, 0, 0);
	}
	::GUI::g_Manager.RemoveModal(interfaceOwner);
	SetFade(0);
	SetInterfaceValue(0);
	if (textureHandle)
	{
		g_GfxSystem->DeleteTexture(textureHandle);
		textureHandle = 0;
	}
}

void Video::Queue(const char* filename, LONG x, LONG y, LONG width, LONG height,
	BYTE modal, BYTE hideCursor, ::GUI::Picture* window) //5B5B10.
{
	VideoQueueItem* item = new VideoQueueItem;
	item->filename = std::string(filename);
	item->x = x;
	item->y = y;
	item->width = width;
	item->height = height;
	item->modal = modal;
	item->hideCursor = hideCursor;
	item->window = window;
	item->fadeDuration = fadeDuration;
	item->interfaceValue = interfaceOwner->GetValue();
	queue.push_back(item);
}
