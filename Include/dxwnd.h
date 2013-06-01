#include <ddraw.h>
//#include "dxwcore.hpp"

#define DXW_IDLE		0
#define DXW_ACTIVE		1
#define DXW_RUNNING		2

#define MAXTARGETS			256

#define DXWACTIVATESINGLETASK 1 // comment to allow multiple task activations

// first flags DWORD dwFlags1:
#define UNNOTIFY			0x00000001
#define EMULATESURFACE		0x00000002
#define CLIPCURSOR		 	0x00000004 // Force cursor clipping within window
#define RESETPRIMARY		0x00000008 // reset emulated primary surface when reopening DDRaw object
#define HOOKDI				0x00000010
#define MODIFYMOUSE			0x00000020
#define HANDLEEXCEPTIONS	0x00000040 // Handles exceptions: Div by 0 ....
#define SAVELOAD			0x00000080
#define EMULATEBUFFER		0x00000100
#define HANDLEDC			0x00000200
#define BLITFROMBACKBUFFER	0x00000400
#define SUPPRESSCLIPPING	0x00000800
#define AUTOREFRESH			0x00001000
#define FIXWINFRAME			0x00002000
#define HIDEHWCURSOR		0x00004000
#define SLOWDOWN			0x00008000 
#define ENABLECLIPPING		0x00010000
#define LOCKWINSTYLE		0x00020000
#define MAPGDITOPRIMARY	 	0x00040000 
#define FIXTEXTOUT			0x00080000
#define KEEPCURSORWITHIN	0x00100000
#define USERGB565			0x00200000
#define SUPPRESSDXERRORS	0x00400000 // suppresses some common dx errors (BUSY)
#define PREVENTMAXIMIZE		0x00800000 // struggle to avoid window maximize
#define ONEPIXELFIX			0x01000000 // don't scale while blitting surfaces just 1 pixel different in size!
#define FIXPARENTWIN		0x02000000 // fixes parent window size & position
#define SWITCHVIDEOMEMORY 	0x04000000 // when VIDEO memory is over, switches to SYSTEM memory
#define CLIENTREMAPPING		0x08000000 // hooks CLientToRect, RectToClient, GetClientRect, GetWinRect
#define HANDLEALTF4			0x10000000 // forces quitting the program when receiving Alt-F4 key
#define LOCKWINPOS			0x20000000 // prevent the origram to change its own windows properties
#define HOOKCHILDWIN		0x40000000 // hook CHILD windows to alter placement coordinates (UNUSED)
#define MESSAGEPROC			0x80000000 // process peek/get messages

// second flags DWORD dxw.dwFlags2:
#define RECOVERSCREENMODE	0x00000001 // actively set screen mode to initial state after hooking
#define REFRESHONRESIZE		0x00000002 // forces a refresh (blitting from backbuffer to primary) upon win resize
#define BACKBUFATTACH		0x00000004 // sets backbuf wxh dim. equal to primary surface so that ZBUFFER is attachable.....
#define MODALSTYLE			0x00000008 // window with no borders
#define KEEPASPECTRATIO		0x00000010 // when resizing, keep original aspect ratio
#define INIT8BPP			0x00000020 // simulate a 8BPP initial desktop setting (in GetDeviceCaps API)
#define FORCEWINRESIZE		0x00000040 // adds NCMOUSEUP/DOWN processing to the win procedure if missing
#define INIT16BPP			0x00000080 // simulate a 16BPP initial desktop setting (in GetDeviceCaps API)
#define KEEPCURSORFIXED		0x00000100 // inhibit SetCursorPos operation
#define DISABLEGAMMARAMP	0x00000200 // let the application retrieve the desktop DC (for capability queries)
#define DIFFERENTIALMOUSE	0x00000400 // emulates the 360-degrees-free-running mouse style....
#define FIXNCHITTEST		0x00000800 // fixes WM_NCHITTEST message X,Y coordinates 
#define LIMITFPS			0x00001000 // delays primary blit operations to limit FPS 
#define SKIPFPS				0x00002000 // skips primary blit operations up to limit
#define SHOWFPS				0x00004000 // shows FPS value to status win / log
#define HIDEMULTIMONITOR	0x00008000 // hide multimonitor configurations: GetAdapterCount returns 1.

// logging Tflags DWORD:
#define OUTTRACE			0x00000001 // enables tracing to dxwnd.log in general
#define OUTDDRAWTRACE		0x00000002 // traces DxWnd directdraw screen handling
#define OUTWINMESSAGES		0x00000004 // traces windows messages
#define OUTCURSORTRACE		0x00000008 // traces windows messages
#define OUTPROXYTRACE		0x00000010 // warning: it also enables proxy functions !!!!
#define DXPROXED			0x00000020 // hook DX proxy methods to log each call in original behaviour
#define ASSERTDIALOG		0x00000040 // show assert messages in Dialog Box
#define OUTIMPORTTABLE		0x00000080 // dump import table contents
#define OUTDEBUG			0x00000100 // detailed debugging indormation

#define EMULATEFLAGS		(EMULATEBUFFER | EMULATESURFACE)

// DxWnd host app data to be passed to the hook callback
typedef struct TARGETMAP
{
	char path[MAX_PATH];
	char module[60+1];
	int dxversion;
	int flags;
	int flags2;
	int tflags;
	short initx;
	short inity;
	short minx;
	short miny;
	short maxx;
	short maxy;
	short posx;
	short posy;
	short sizx;
	short sizy;
	short MaxFPS;
}TARGETMAP;

typedef struct
{
	short Status;
	short TaskIdx;
	short IsFullScreen;
	short Width, Height;
	short ColorDepth;
	short DXVersion;
	HWND hWnd;
	DWORD dwPid;
	BOOL isLogging;
	DWORD FPSCount;
} DXWNDSTATUS;

extern DXWNDSTATUS DxWndStatus;

int SetTarget(TARGETMAP *);
int StartHook(void);
int EndHook(void);
void GetDllVersion(char *);
int GetHookStatus(DXWNDSTATUS *);
void SetHookStatus(DXWNDSTATUS *);
int HookInit(TARGETMAP *, HWND);

void *SetHook(void *, void *);
void SetHook(void *, void *, void **, char *);
void OutTrace(const char *, ...);
void *HookAPI(const char *, void *, const char *, void *);
void AdjustWindowFrame(HWND, DWORD, DWORD);
LRESULT CALLBACK extWindowProc(HWND, UINT, WPARAM, LPARAM);

// defines below to condition debug message handling

#define OutTraceW if(dxw.dwTFlags & OUTWINMESSAGES) OutTrace
#define OutTraceX if(dxw.dwTFlags & OUTPROXYTRACE) OutTrace
#define OutTraceD if(dxw.dwTFlags & OUTDDRAWTRACE) OutTrace
#define OutTraceC if(dxw.dwTFlags & OUTCURSORTRACE) OutTrace
#define OutTraceP OutTrace
#define OutTraceE OutTrace

#define IsTraceW (dxw.dwTFlags & OUTWINMESSAGES)
#define IsTraceX (dxw.dwTFlags & OUTPROXYTRACE)
#define IsTraceD (dxw.dwTFlags & OUTDDRAWTRACE)
#define IsTraceC (dxw.dwTFlags & OUTCURSORTRACE)
#define IsTraceP (TRUE)
#define IsTraceE (TRUE)
#define IsDebug  (dxw.dwTFlags & OUTDEBUG)
#define IsAssertEnabled (dxw.dwTFlags & ASSERTDIALOG)
#define STEP OutTrace("STEP at %s:%d\n", __MODULE__, __LINE__)

extern void WhndStackPush(HWND, WNDPROC);
extern WNDPROC WhndGetWindowProc(HWND );


