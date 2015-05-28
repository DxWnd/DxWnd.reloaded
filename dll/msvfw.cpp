#include "dxwnd.h"
#include "dxwcore.hpp"
#include "syslibs.h"
#include "dxhook.h"
#include "Vfw.h"

#define DXWDECLARATIONS TRUE
#include "msvfwhook.h"
#undef DXWDECLARATIONS

static HookEntry_Type Hooks[]={
	//{HOOK_HOT_CANDIDATE, "ICSendMessage", (FARPROC)NULL, (FARPROC *)&pICSendMessage, (FARPROC)extICSendMessage},
	//{HOOK_HOT_CANDIDATE, "ICOpen", (FARPROC)NULL, (FARPROC *)&pICOpen, (FARPROC)extICOpen},
	{HOOK_IAT_CANDIDATE, 0, NULL, 0, 0} // terminator
};

FARPROC Remap_vfw_ProcAddress(LPCSTR proc, HMODULE hModule)
{
	FARPROC addr;
	if (addr=RemapLibrary(proc, hModule, Hooks)) return addr;
	// NULL -> keep the original call address
	return NULL;
}

void HookMSV4WLibs(HMODULE module)
{
	HookLibrary(module, Hooks, "MSVFW32.dll");
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