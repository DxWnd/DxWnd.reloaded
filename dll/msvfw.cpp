#define _CRT_SECURE_NO_WARNINGS 1

#include "dxwnd.h"
#include "dxwcore.hpp"
#include "syslibs.h"
#include "dxhook.h"
#include "Vfw.h"

#define DXWDECLARATIONS TRUE
#include "msvfwhook.h"
#undef DXWDECLARATIONS

typedef DWORD (WINAPIV *ICDrawBegin_Type)(HIC, DWORD, HPALETTE, HWND,  HDC, int, int, int, int, LPBITMAPINFOHEADER, int, int, int, int, DWORD, DWORD);
ICDrawBegin_Type pICDrawBegin = NULL;
DWORD WINAPIV extICDrawBegin(HIC, DWORD, HPALETTE, HWND,  HDC, int, int, int, int, LPBITMAPINFOHEADER, int, int, int, int, DWORD, DWORD);
typedef BOOL (WINAPI *DrawDibDraw_Type)(HDRAWDIB, HDC, int, int, int, int, LPBITMAPINFOHEADER, LPVOID, int, int, int, int, UINT);
DrawDibDraw_Type pDrawDibDraw = NULL;
BOOL WINAPI extDrawDibDraw(HDRAWDIB, HDC, int, int, int, int, LPBITMAPINFOHEADER, LPVOID, int, int, int, int, UINT);
typedef BOOL (WINAPI *DrawDibBegin_Type)(HDRAWDIB, HDC, int, int, LPBITMAPINFOHEADER, int, int, UINT);
DrawDibBegin_Type pDrawDibBegin = NULL;
BOOL WINAPI extDrawDibBegin(HDRAWDIB, HDC, int, int, LPBITMAPINFOHEADER, int, int, UINT);
typedef BOOL (WINAPI *DrawDibStart_Type)(HDRAWDIB, LONG);
DrawDibStart_Type pDrawDibStart = NULL;
BOOL WINAPI extDrawDibStart(HDRAWDIB, LONG);
typedef BOOL (WINAPI *DrawDibStop_Type)(HDRAWDIB);
DrawDibStop_Type pDrawDibStop = NULL;
BOOL WINAPI extDrawDibStop(HDRAWDIB);
typedef BOOL (WINAPI *DrawDibEnd_Type)(HDRAWDIB);
DrawDibEnd_Type pDrawDibEnd = NULL;
BOOL WINAPI extDrawDibEnd(HDRAWDIB);

static HookEntryEx_Type Hooks[]={
	//{HOOK_HOT_CANDIDATE, 0, "ICSendMessage", (FARPROC)NULL, (FARPROC *)&pICSendMessage, (FARPROC)extICSendMessage},
	//{HOOK_HOT_CANDIDATE, 0, "ICOpen", (FARPROC)NULL, (FARPROC *)&pICOpen, (FARPROC)extICOpen},
	{HOOK_HOT_CANDIDATE, 0, "MCIWndCreateA", (FARPROC)NULL, (FARPROC *)&pMCIWndCreateA, (FARPROC)extMCIWndCreateA}, // "Man in Black" - beware: this is NOT STDCALL!!!
	{HOOK_HOT_CANDIDATE, 0, "ICGetDisplayFormat", (FARPROC)NULL, (FARPROC *)&pICGetDisplayFormat, (FARPROC)extICGetDisplayFormat}, // "Man in Black" - beware: this is NOT STDCALL!!!
	{HOOK_HOT_CANDIDATE, 0, "ICDrawBegin", (FARPROC)NULL, (FARPROC *)&pICDrawBegin, (FARPROC)extICDrawBegin}, // AoE demo: not a STDCALL !!
	{HOOK_HOT_CANDIDATE, 0, "DrawDibDraw", (FARPROC)NULL, (FARPROC *)&pDrawDibDraw, (FARPROC)extDrawDibDraw}, 
	{HOOK_HOT_CANDIDATE, 0, "DrawDibBegin", (FARPROC)NULL, (FARPROC *)&pDrawDibBegin, (FARPROC)extDrawDibBegin}, 
	{HOOK_HOT_CANDIDATE, 0, "DrawDibStart", (FARPROC)NULL, (FARPROC *)&pDrawDibStart, (FARPROC)extDrawDibStart}, 
	{HOOK_HOT_CANDIDATE, 0, "DrawDibStop", (FARPROC)NULL, (FARPROC *)&pDrawDibStop, (FARPROC)extDrawDibStop}, 
	{HOOK_HOT_CANDIDATE, 0, "DrawDibEnd", (FARPROC)NULL, (FARPROC *)&pDrawDibEnd, (FARPROC)extDrawDibEnd}, 
	{HOOK_IAT_CANDIDATE, 0, 0, NULL, 0, 0} // terminator
};

FARPROC Remap_vfw_ProcAddress(LPCSTR proc, HMODULE hModule)
{
	FARPROC addr;
	if (addr=RemapLibraryEx(proc, hModule, Hooks)) return addr;
	// NULL -> keep the original call address
	return NULL;
}

void HookMSV4WLibs(HMODULE module)
{
	HookLibraryEx(module, Hooks, "MSVFW32.dll");
}

LRESULT WINAPI extICSendMessage(HIC hic, UINT wMsg, DWORD_PTR dw1, DWORD_PTR dw2)
{
	LRESULT res;
	OutTraceDW("ICSendMessage: hic=%x wMsg=%x dw1=%x dw2=%x\n", hic, wMsg, dw1, dw2);
	res=(*pICSendMessage)(hic, wMsg, dw1, dw2);
	OutTraceDW("ICSendMessage: ret=%x\n", res);
	return res;
}

HIC WINAPI extICOpen(DWORD fccType, DWORD fccHandler, UINT wMode)
{
	HIC res;
	OutTraceDW("ICOpen: fccType=%x fccHandler=%x wMode=%x\n", fccType, fccHandler, wMode);
	res=(*pICOpen)(fccType, fccHandler, wMode);
	OutTraceDW("ICOpen: ret=%x\n", res);
	return res;
}

/*
ICDrawBegin function

The ICDrawBegin function initializes the renderer and prepares the drawing destination for drawing.
Syntax
DWORD ICDrawBegin(
  HIC hic,
  DWORD dwFlags,
  HPALETTE hpal,
  HWND hwnd,
  HDC hdc,
  int xDst,
  int yDst,
  int dxDst,
  int dyDst,
  LPBITMAPINFOHEADER lpbi,
  int xSrc,
  int ySrc,
  int dxSrc,
  int dySrc,
  DWORD dwRate,
  DWORD dwScale
);


Parameters

hic						Handle to the decompressor to use.

dwFlags
    Decompression flags. The following values are defined.
    Value 				Meaning
    ICDRAW_ANIMATE		Application can animate the palette.
    ICDRAW_CONTINUE		Drawing is a continuation of the previous frame.
    ICDRAW_FULLSCREEN	Draws the decompressed data on the full screen.
    ICDRAW_HDC			Draws the decompressed data to a window or a DC.
    ICDRAW_MEMORYDC		DC is off-screen.
    ICDRAW_QUERY		Determines if the decompressor can decompress the data. The driver does not decompress the data.
    ICDRAW_UPDATING		Current frame is being updated rather than played.

hpal					Handle to the palette used for drawing.

hwnd					Handle to the window used for drawing.

hdc						DC used for drawing.

xDst					The x-coordinate of the upper right corner of the destination rectangle.
yDst					The y-coordinate of the upper right corner of the destination rectangle.
dxDst					Width of the destination rectangle.
dyDst					Height of the destination rectangle.

lpbi					Pointer to a BITMAPINFOHEADER structure containing the format of the input data to be decompressed.

xSrc					The x-coordinate of the upper right corner of the source rectangle.
ySrc					The y-coordinate of the upper right corner of the source rectangle.
dxSrc					Width of the source rectangle.
dySrc					Height of the source rectangle.

dwRate					Frame rate numerator. The frame rate, in frames per second, is obtained by dividing dwRate by dwScale.
dwScale					Frame rate denominator. The frame rate, in frames per second, is obtained by dividing dwRate by dwScale.

Return value

Returns ICERR_OK if the renderer can decompress the data or ICERR_UNSUPPORTED otherwise.
Remarks

The ICDRAW_HDC and ICDRAW_FULLSCREEN flags are mutually exclusive. If an application sets the ICDRAW_HDC flag in dwFlags, the decompressor uses hwnd, hdc, and the parameters defining the destination rectangle (xDst, yDst, dxDst, and dyDst). Your application should set these parameters to the size of the destination rectangle. Specify destination rectangle values relative to the current window or DC.
If an application sets the ICDRAW_FULLSCREEN flag in dwFlags, the hwnd and hdc parameters are not used and should be set to NULL. Also, the destination rectangle is not used and its parameters can be set to zero.
The source rectangle is relative to the full video frame. The portion of the video frame specified by the source rectangle is stretched or shrunk to fit the destination rectangle.
The dwRate and dwScale parameters specify the decompression rate. The integer value specified for dwRate divided by the integer value specified for dwScale defines the frame rate in frames per second. This value is used by the renderer when it is responsible for timing frames during playback.

Requirements
Minimum supported client
	Windows 2000 Professional [desktop apps only]
Minimum supported server
	Windows 2000 Server [desktop apps only]

Header					Vfw.h
Library					Vfw32.lib
DLL						Msvfw32.dll 

*/
static char *ExplainMCIWndCreateFlags(DWORD c)
{
	static char eb[256];
	unsigned int l;
	strcpy(eb,"MCIWNDF_");
	if (c & MCIWNDF_NOAUTOSIZEWINDOW) strcat(eb, "NOAUTOSIZEWINDOW+");
	if (c & MCIWNDF_NOPLAYBAR) strcat(eb, "NOPLAYBAR+");
	if (c & MCIWNDF_NOAUTOSIZEMOVIE) strcat(eb, "NOAUTOSIZEMOVIE+");
	if (c & MCIWNDF_NOMENU) strcat(eb, "NOMENU+");
	if (c & MCIWNDF_SHOWNAME) strcat(eb, "SHOWNAME+");
	if (c & MCIWNDF_SHOWPOS) strcat(eb, "SHOWPOS+");
	if (c & MCIWNDF_SHOWMODE) strcat(eb, "SHOWMODE+");
	if (c & MCIWNDF_NOTIFYMODE) strcat(eb, "NOTIFYMODE+");
	if (c & MCIWNDF_NOTIFYPOS) strcat(eb, "NOTIFYPOS+");
	if (c & MCIWNDF_NOTIFYSIZE) strcat(eb, "NOTIFYSIZE+");
	if (c & MCIWNDF_NOTIFYERROR) strcat(eb, "NOTIFYERROR+");
	if (c & MCIWNDF_NOTIFYMEDIAW) strcat(eb, "NOTIFYMEDIAW+");
	if (c & MCIWNDF_NOTIFYANSI) strcat(eb, "NOTIFYANSI+");
	if (c & MCIWNDF_RECORD) strcat(eb, "RECORD+");
	if (c & MCIWNDF_NOERRORDLG) strcat(eb, "NOERRORDLG+");
	if (c & MCIWNDF_NOOPEN) strcat(eb, "NOOPEN+");
	l=strlen(eb);
	if (l>strlen("MCIWNDF_")) eb[l-1]=0; // delete last '+' if any
	else eb[0]=0;
	return(eb);
}

HWND extMCIWndCreateA(HWND hwndParent, HINSTANCE hInstance, DWORD dwStyle, LPCTSTR szFile)
{
	HWND g_hwndMCIWnd;
	OutTraceDW("MCIWndCreateA: hwnd=%x hInst=%x style=%x(%s) file=%s\n", 
		hwndParent, hInstance, dwStyle, ExplainMCIWndCreateFlags(dwStyle), szFile);
	if(dxw.dwFlags6 & NOMOVIES) {
		OutTraceDW("MCIWndCreateA: SUPPRESSED\n");
		return NULL;
	}

	g_hwndMCIWnd = (*pMCIWndCreateA)(hwndParent, hInstance, dwStyle, szFile);

	// look at https://msdn.microsoft.com/en-us/library/windows/desktop/dd757178%28v=vs.85%29.aspx

	if(dxw.dwFlags5 & REMAPMCI){
		// since there seem to be no way to stretch the movie, we do an attempt to center 
		// the movie on the screen by shifting the video window. 
		// We assume (but we don't really know for sure ...) that the movie was to be rendered
		// fulscreen using the size of the virtual desktop
		RECT client;
		OutTraceDW("MCIWndCreateA: CENTERED\n");
		(*pGetClientRect)(dxw.GethWnd(), &client);
		(*pSetWindowPos)(g_hwndMCIWnd,					// window to resize 
		NULL,											// z-order: don't care 
		(client.right - dxw.GetScreenWidth()) / 2,		// X pos
		(client.bottom - dxw.GetScreenHeight()) / 2,	// Y pos
		dxw.GetScreenWidth(),							// width
		dxw.GetScreenHeight(),							// height
		SWP_NOZORDER | SWP_NOACTIVATE); 
		//(*pUpdateWindow)(g_hwndMCIWnd);
	}

	return g_hwndMCIWnd;
}

HIC WINAPI extICGetDisplayFormat(HIC hic, LPBITMAPINFOHEADER lpbiIn, LPBITMAPINFOHEADER lpbiOut, int BitDepth, int dx, int dy)
{
	HIC ret;
	OutTraceDW("ICGetDisplayFormat: hic=%x bitdepth=%d dx=%d dy=%d indepth=%d\n", hic, BitDepth, dx, dy, lpbiIn->biBitCount);

	ret=(*pICGetDisplayFormat)(hic, lpbiIn, lpbiOut, BitDepth, dx, dy);
	if(ret){
		OutTraceDW("ICGetDisplayFormat: ret=%x outdepth=%d\n", hic, lpbiOut->biBitCount);
		if(dxw.dwFlags6 & FIXMOVIESCOLOR){
			lpbiOut->biBitCount = (WORD)dxw.VirtualPixelFormat.dwRGBBitCount;
			OutTraceDW("ICGetDisplayFormat: FIXED outdepth=%d\n", lpbiOut->biBitCount);
		}
	}
	else {
		OutTraceDW("ICGetDisplayFormat ERROR: err=%d\n", GetLastError());
	}

	return ret;
}
DWORD WINAPIV extICDrawBegin(HIC hic, DWORD dwFlags, HPALETTE hpal, HWND hwnd,  HDC hdc, int xDst, int yDst, int dxDst, int dyDst, LPBITMAPINFOHEADER lpbi, int xSrc, int ySrc, int dxSrc, int dySrc, DWORD dwRate, DWORD dwScale)
{
	OutTrace("ICDrawBegin\n");

	return (*pICDrawBegin)(hic, dwFlags, hpal, hwnd, hdc, xDst, yDst, dxDst, dyDst, lpbi, xSrc, ySrc, dxSrc, dySrc, dwRate, dwScale);
}

BOOL WINAPI extDrawDibDraw(HDRAWDIB hdd, HDC hdc, int xDst, int yDst, int dxDst, int dyDst, LPBITMAPINFOHEADER lpbi, LPVOID lpBits, int xSrc, int ySrc, int dxSrc, int dySrc, UINT wFlags)
{
	// Reah intro movie
	BOOL ret;
	OutTrace("DrawDibDraw: hdd=%x hdc=%x DEST pos=(%d,%d) size=(%d,%d) SRC pos=(%d,%d) size=(%d,%d) flags=%x\n",
		hdd, hdc, xDst, yDst, dxDst, dyDst, xSrc, ySrc, dxSrc, dySrc, wFlags);
	ret = (*pDrawDibDraw)(hdd, hdc, xDst, yDst, dxDst, dyDst, lpbi, lpBits, xSrc, ySrc, dxSrc, dySrc, wFlags);
	return ret;
}

BOOL WINAPI extDrawDibBegin(HDRAWDIB hdd, HDC hdc, int dxDest, int dyDest, LPBITMAPINFOHEADER lpbi, int dxSrc, int dySrc, UINT wFlags)
{
	// Reah game transitions
	BOOL ret;
	OutTrace("DrawDibBegin: hdd=%x hdc=%x DEST size=(%d,%d) SRC size=(%d,%d) flags=%x\n",
		hdd, hdc, dxDest, dyDest, dxSrc, dySrc, wFlags);
	ret = (*pDrawDibBegin)(hdd, hdc, dxDest, dyDest, lpbi, dxSrc, dySrc, wFlags);
	return ret;
}

BOOL WINAPI extDrawDibStart(HDRAWDIB hdd, LONG rate)
{
	// Reah game transitions
	BOOL ret;
	OutTrace("DrawDibStart: hdd=%x rate=%x\n", hdd, rate);
	ret = (*pDrawDibStart)(hdd, rate);
	return ret;
}

BOOL WINAPI extDrawDibStop(HDRAWDIB hdd)
{
	// Reah game transitions
	BOOL ret;
	OutTrace("DrawDibStop: hdd=%x\n", hdd);
	ret = (*pDrawDibStop)(hdd);
	return ret;
}

BOOL WINAPI extDrawDibEnd(HDRAWDIB hdd)
{
	// Reah game transitions
	BOOL ret;
	OutTrace("DrawDibEnd: hdd=%x\n", hdd);
	ret = (*pDrawDibEnd)(hdd);
	return ret;
}



