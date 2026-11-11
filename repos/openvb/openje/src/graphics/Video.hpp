#pragma once

#include "gui/EngineInterface.hpp"
#include "configuration/ConfigurationTypes.hpp"
#include <cstddef>
#include <list>
#include <string>

namespace GUI { class Picture; }
struct BinkMovie;

class VideoInterface : public virtual JE::EngineInterface
{
public:
	explicit VideoInterface(const std::string& name); //5B4EB0, original global RTTI owner.
	virtual ~VideoInterface(); //5B4F00, ordinary destructor.
	virtual int OnEvent(::GUI::Window* window, ::GUI::Event* event); //5B5A60; compiler vtordisp thunk5B5B00.
	void SetValue(DWORD value); //5B4F20, shared by the controller setter.
	DWORD GetValue() const; //5B4F30, shared four-byte getter; no duplicate credit.
	DWORD value; //04; virtual EngineInterface starts0C under VC7.
};

struct VideoQueueItem
{
	VideoQueueItem(); //5B5220.
	std::string filename; //00..1B.
	LONG x, y, width, height; //1C..28.
	BYTE modal, hideCursor; //2C/2D; padding2E/2F unwritten.
	::GUI::Picture* window; //30; must support original virtual12C.
	DWORD interfaceValue, fadeDuration; //34/38; constructor deliberately leaves unwritten.
};

class Video
{
public:
	Video(); //5B5080, original global RTTI owner, allocation2Ch.
	virtual ~Video(); //5B50D0, ordinary destructor.
	virtual JE::Configuration::Settings* Initialize(); //5B4F40, actual virtual+04.
	virtual void OnInitialize(); //404B70, actual empty virtual+08.
	virtual JE::Configuration::Settings* OnCleanup(); //5B4F70, actual virtual+0C.
	void SetFade(DWORD duration); //5B4F50; unused EAX is not an invented status result.
	void SetInterfaceValue(DWORD value); //5B4F60, includes shared setter tail.
	void Cleanup(); //5B4FC0; leaves borrowed picture/active pointers unchanged.
	void PrepareUI(); //5B52E0.
	void Play(const char* filename, LONG x, LONG y, LONG width, LONG height,
		BYTE modal, BYTE hideCursor, ::GUI::Picture* window); //5B5640; no phantom EDI argument.
	void Stop(); //5B5820.
	void Frame(); //5B5990.
	void Update(); //5B5AE0.
	void Queue(const char* filename, LONG x, LONG y, LONG width, LONG height,
		BYTE modal, BYTE hideCursor, ::GUI::Picture* window); //5B5B10.

	BinkMovie* movie; //04, Bink DLL-owned object.
	DWORD textureHandle; //08, actual Graphics backend handle.
	VideoInterface* interfaceOwner; //0C, owns the GUI roots.
	::GUI::Picture* background; //10, borrowed root owned by interfaceOwner.
	::GUI::Picture* movieWindow; //14, borrowed child of background.
	::GUI::Picture* activeWindow; //18, borrowed; may be supplied by the caller.
	DWORD fadeDuration; //1C, signed comparison by Stop.
	std::list<VideoQueueItem*> queue; //20, actual library nodes; owns payloads.
};

#if defined(_MSC_VER) && _MSC_VER == 1300
typedef char VideoOriginalExtent[(sizeof(Video) == 0x2C) ? 1 : -1];
typedef char VideoOriginalMovie[(offsetof(Video, movie) == 4) ? 1 : -1];
typedef char VideoOriginalTexture[(offsetof(Video, textureHandle) == 8) ? 1 : -1];
typedef char VideoOriginalInterface[(offsetof(Video, interfaceOwner) == 0xC) ? 1 : -1];
typedef char VideoOriginalBackground[(offsetof(Video, background) == 0x10) ? 1 : -1];
typedef char VideoOriginalWindow[(offsetof(Video, movieWindow) == 0x14) ? 1 : -1];
typedef char VideoOriginalActive[(offsetof(Video, activeWindow) == 0x18) ? 1 : -1];
typedef char VideoOriginalFade[(offsetof(Video, fadeDuration) == 0x1C) ? 1 : -1];
typedef char VideoOriginalQueue[(offsetof(Video, queue) == 0x20) ? 1 : -1];
typedef char VideoInterfaceOriginalExtent[(sizeof(VideoInterface) == 0x40) ? 1 : -1];
typedef char VideoInterfaceOriginalValue[(offsetof(VideoInterface, value) == 4) ? 1 : -1];
typedef char VideoQueueOriginalExtent[(sizeof(VideoQueueItem) == 0x3C) ? 1 : -1];
typedef char VideoQueueOriginalX[(offsetof(VideoQueueItem, x) == 0x1C) ? 1 : -1];
typedef char VideoQueueOriginalModal[(offsetof(VideoQueueItem, modal) == 0x2C) ? 1 : -1];
typedef char VideoQueueOriginalCursor[(offsetof(VideoQueueItem, hideCursor) == 0x2D) ? 1 : -1];
typedef char VideoQueueOriginalWindow[(offsetof(VideoQueueItem, window) == 0x30) ? 1 : -1];
typedef char VideoQueueOriginalValue[(offsetof(VideoQueueItem, interfaceValue) == 0x34) ? 1 : -1];
typedef char VideoQueueOriginalFade[(offsetof(VideoQueueItem, fadeDuration) == 0x38) ? 1 : -1];
#endif
