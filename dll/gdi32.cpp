#define _CRT_SECURE_NO_WARNINGS

#include "dxwnd.h"
#include "dxwcore.hpp"
#include "syslibs.h"
#include "hddraw.h"
#include "dxhook.h"
#include "dxhelper.h"
#include "shareddc.hpp"

#include "stdio.h"

static BOOL bGDIRecursionFlag = FALSE;

static void Stopper(char *s, int line)
{
	char sMsg[81];
	sprintf_s(sMsg, 80, "break: \"%s\"", s);
	MessageBox(0, sMsg, "break", MB_OK | MB_ICONEXCLAMATION);
}

//#define STOPPER_TEST // comment out to eliminate
#ifdef STOPPER_TEST
#define STOPPER(s) Stopper(s, __LINE__)
#elsedxw
#define STOPPER(s)
#endif

#ifdef TRACEPALETTE
typedef BOOL (WINAPI *ResizePalette_Type)(HPALETTE, UINT);
ResizePalette_Type pResizePalette = NULL;
BOOL WINAPI extResizePalette(HPALETTE, UINT);
#endif

#define _Warn(s) MessageBox(0, s, "to do", MB_ICONEXCLAMATION)

/*
typedef COLORREF (WINAPI *SetBkColor_Type)(HDC, COLORREF);
typedef COLORREF (WINAPI *SetTextColor_Type)(HDC hdc, COLORREF crColor);
typedef int (WINAPI *SetBkMode_Type)(HDC, int);

SetBkColor_Type pSetBkColor = NULL;
SetTextColor_Type pSetTextColor = NULL;
SetBkMode_Type pSetBkMode = NULL;

COLORREF WINAPI extSetBkColor(HDC, COLORREF);
COLORREF WINAPI extSetTextColor(HDC hdc, COLORREF crColor);
int WINAPI extSetBkMode(HDC, int);
*/
typedef int (WINAPI *SetDIBits_Type)(HDC, HBITMAP, UINT, UINT, const VOID *, const BITMAPINFO *, UINT);
int WINAPI extSetDIBits(HDC, HBITMAP, UINT, UINT, const VOID *, const BITMAPINFO *, UINT);
SetDIBits_Type pSetDIBits = NULL;
typedef int (WINAPI *OffsetRgn_Type)(HRGN, int, int);
OffsetRgn_Type pOffsetRgn = NULL;
int WINAPI extOffsetRgn(HRGN, int, int);
typedef COLORREF (WINAPI *GetPixel_Type)(HDC, int, int);
GetPixel_Type pGetPixel = NULL;
COLORREF WINAPI extGetPixel(HDC, int, int);
typedef BOOL (WINAPI *PlgBlt_Type)(HDC, const POINT *, HDC, int, int, int, int, HBITMAP, int, int);
PlgBlt_Type pPlgBlt = NULL;
BOOL WINAPI extPlgBlt(HDC, const POINT *, HDC, int, int, int, int, HBITMAP, int, int);
typedef BOOL (WINAPI *SetPixelV_Type)(HDC, int, int, COLORREF);
SetPixelV_Type pSetPixelV = NULL;
BOOL WINAPI extSetPixelV(HDC, int, int, COLORREF);
typedef BOOL (WINAPI *Chord_Type)(HDC, int, int, int, int, int, int, int, int);
Chord_Type pChord = NULL;
BOOL WINAPI extChord(HDC, int, int, int, int, int, int, int, int);
typedef BOOL (WINAPI *PolyTextOutA_Type)(HDC, const POLYTEXTA *, int);
PolyTextOutA_Type pPolyTextOutA = NULL;
BOOL WINAPI extPolyTextOutA(HDC, const POLYTEXTA *, int);
typedef BOOL (WINAPI *PolyTextOutW_Type)(HDC, const POLYTEXTW *, int);
PolyTextOutW_Type pPolyTextOutW = NULL;
BOOL WINAPI extPolyTextOutW(HDC, const POLYTEXTW *, int);
typedef int (WINAPI *GetDIBits_Type)(HDC, HBITMAP, UINT, UINT, LPVOID, LPBITMAPINFO, UINT);
GetDIBits_Type pGetDIBits = NULL;
int WINAPI extGetDIBits(HDC, HBITMAP, UINT, UINT, LPVOID, LPBITMAPINFO, UINT);
typedef HBITMAP (WINAPI *CreateDIBitmap_Type)(HDC, BITMAPINFOHEADER *, DWORD, const VOID *, const BITMAPINFO *, UINT);
CreateDIBitmap_Type pCreateDIBitmap = NULL;
HBITMAP WINAPI extCreateDIBitmap(HDC, BITMAPINFOHEADER *, DWORD, const VOID *, const BITMAPINFO *, UINT);
typedef HBITMAP (WINAPI *CreateDIBSection_Type)(HDC, const BITMAPINFO *, UINT, VOID **, HANDLE, DWORD);
CreateDIBSection_Type pCreateDIBSection = NULL;
HBITMAP WINAPI extCreateDIBSection(HDC, const BITMAPINFO *, UINT, VOID **, HANDLE, DWORD);
typedef HBITMAP (WINAPI *CreateDiscardableBitmap_Type)(HDC, int, int);
CreateDiscardableBitmap_Type pCreateDiscardableBitmap = NULL;
HBITMAP WINAPI extCreateDiscardableBitmap(HDC, int, int);
typedef BOOL (WINAPI *ExtFloodFill_Type)(HDC, int, int, COLORREF, UINT);
ExtFloodFill_Type pExtFloodFill = NULL;
BOOL WINAPI extExtFloodFill(HDC, int, int, COLORREF, UINT);
typedef BOOL (WINAPI *GdiAlphaBlend_Type)(HDC, int, int, int, int, HDC, int, int, int, int, BLENDFUNCTION);
GdiAlphaBlend_Type pGdiAlphaBlend = NULL;
BOOL WINAPI extGdiAlphaBlend(HDC, int, int, int, int, HDC, int, int, int, int, BLENDFUNCTION);
typedef BOOL (WINAPI *GdiGradientFill_Type)(HDC, PTRIVERTEX, ULONG, PVOID, ULONG, ULONG);
GdiGradientFill_Type pGdiGradientFill = NULL;
BOOL WINAPI extGdiGradientFill(HDC, PTRIVERTEX, ULONG, PVOID, ULONG, ULONG);
typedef BOOL (WINAPI *GdiTransparentBlt_Type)(HDC, int, int, int, int, HDC, int, int, int, int, UINT);
GdiTransparentBlt_Type pGdiTransparentBlt = NULL;
BOOL WINAPI extGdiTransparentBlt(HDC, int, int, int, int, HDC, int, int, int, int, UINT);
typedef BOOL (WINAPI *Pie_Type)(HDC, int, int, int, int, int, int, int, int);
Pie_Type pPie = NULL;
BOOL WINAPI extPie(HDC, int, int, int, int, int, int, int, int);
typedef BOOL (WINAPI *AngleArc_Type)(HDC, int, int, DWORD, FLOAT, FLOAT);
AngleArc_Type pAngleArc = NULL;
BOOL WINAPI extAngleArc(HDC, int, int, DWORD, FLOAT, FLOAT);
typedef BOOL (WINAPI *PolyPolyline_Type)(HDC, const POINT *, const DWORD *, DWORD);
PolyPolyline_Type pPolyPolyline = NULL;
BOOL WINAPI extPolyPolyline(HDC, const POINT *, const DWORD *, DWORD);
typedef BOOL (WINAPI *FillRgn_Type)(HDC, HRGN, HBRUSH);
FillRgn_Type pFillRgn = NULL;
BOOL WINAPI extFillRgn(HDC, HRGN, HBRUSH);
typedef BOOL (WINAPI *FrameRgn_Type)(HDC, HRGN, HBRUSH, int, int);
FrameRgn_Type pFrameRgn = NULL;
BOOL WINAPI extFrameRgn(HDC, HRGN, HBRUSH, int, int);
typedef BOOL (WINAPI *InvertRgn_Type)(HDC, HRGN);
InvertRgn_Type pInvertRgn = NULL;
BOOL WINAPI extInvertRgn(HDC, HRGN);
typedef BOOL (WINAPI *PaintRgn_Type)(HDC, HRGN);
PaintRgn_Type pPaintRgn = NULL;
BOOL WINAPI extPaintRgn(HDC, HRGN);
typedef int (WINAPI *SetMapMode_Type)(HDC, int);
SetMapMode_Type pSetMapMode = NULL;
int WINAPI extSetMapMode(HDC, int);
typedef BOOL (WINAPI *RoundRect_Type)(HDC, int, int, int, int, int, int);
RoundRect_Type pRoundRect = NULL;
BOOL WINAPI extRoundRect(HDC, int, int, int, int, int, int);
typedef BOOL (WINAPI *PolyPolygon_Type)(HDC, const POINT *, const INT *, int);
PolyPolygon_Type pPolyPolygon = NULL;
BOOL WINAPI extPolyPolygon(HDC, const POINT *, const INT *, int);

static HookEntryEx_Type Hooks[]={

	{HOOK_IAT_CANDIDATE, 0, "GetDeviceCaps", (FARPROC)GetDeviceCaps, (FARPROC *)&pGDIGetDeviceCaps, (FARPROC)extGetDeviceCaps},
	{HOOK_IAT_CANDIDATE, 0, "ScaleWindowExtEx", (FARPROC)ScaleWindowExtEx, (FARPROC *)&pGDIScaleWindowExtEx, (FARPROC)extScaleWindowExtEx},
	{HOOK_IAT_CANDIDATE, 0, "SaveDC", (FARPROC)SaveDC, (FARPROC *)&pGDISaveDC, (FARPROC)extGDISaveDC},
	{HOOK_IAT_CANDIDATE, 0, "RestoreDC", (FARPROC)RestoreDC, (FARPROC *)&pGDIRestoreDC, (FARPROC)extGDIRestoreDC},
	{HOOK_HOT_CANDIDATE, 0, "AnimatePalette", (FARPROC)AnimatePalette, (FARPROC *)&pAnimatePalette, (FARPROC)extAnimatePalette},
	{HOOK_HOT_CANDIDATE, 0, "CreatePalette", (FARPROC)CreatePalette, (FARPROC *)&pGDICreatePalette, (FARPROC)extGDICreatePalette},
	{HOOK_HOT_CANDIDATE, 0, "SelectPalette", (FARPROC)SelectPalette, (FARPROC *)&pGDISelectPalette, (FARPROC)extSelectPalette},
	{HOOK_HOT_CANDIDATE, 0, "RealizePalette", (FARPROC)RealizePalette, (FARPROC *)&pGDIRealizePalette, (FARPROC)extRealizePalette},
	{HOOK_HOT_CANDIDATE, 0, "GetSystemPaletteEntries", (FARPROC)GetSystemPaletteEntries, (FARPROC *)&pGDIGetSystemPaletteEntries, (FARPROC)extGetSystemPaletteEntries},
	{HOOK_HOT_CANDIDATE, 0, "SetSystemPaletteUse", (FARPROC)SetSystemPaletteUse, (FARPROC *)&pSetSystemPaletteUse, (FARPROC)extSetSystemPaletteUse},
	{HOOK_IAT_CANDIDATE, 0, "SetPixelFormat", (FARPROC)NULL, (FARPROC *)&pGDISetPixelFormat, (FARPROC)extGDISetPixelFormat},
	{HOOK_IAT_CANDIDATE, 0, "GetPixelFormat", (FARPROC)NULL, (FARPROC *)&pGDIGetPixelFormat, (FARPROC)extGDIGetPixelFormat},
	{HOOK_IAT_CANDIDATE, 0, "ChoosePixelFormat", (FARPROC)NULL, (FARPROC *)&pChoosePixelFormat, (FARPROC)extChoosePixelFormat},
	{HOOK_IAT_CANDIDATE, 0, "DescribePixelFormat", (FARPROC)NULL, (FARPROC *)&pDescribePixelFormat, (FARPROC)extDescribePixelFormat},
	{HOOK_HOT_CANDIDATE, 0, "GetPaletteEntries", (FARPROC)GetPaletteEntries, (FARPROC *)&pGetPaletteEntries, (FARPROC)extGetPaletteEntries},
	{HOOK_HOT_CANDIDATE, 0, "GetSystemPaletteUse", (FARPROC)GetSystemPaletteUse, (FARPROC *)&pGetSystemPaletteUse, (FARPROC)extGetSystemPaletteUse},
	{HOOK_HOT_CANDIDATE, 0, "CreateICA", (FARPROC)CreateICA, (FARPROC *)&pCreateICA, (FARPROC)extCreateICA}, // Riven
#ifdef TRACEPALETTE
	{HOOK_IAT_CANDIDATE, 0, "ResizePalette", (FARPROC)ResizePalette, (FARPROC *)&pResizePalette, (FARPROC)extResizePalette},
#endif	
	{HOOK_IAT_CANDIDATE, 0, 0, NULL, 0, 0} // terminator
}; 
 
static HookEntryEx_Type RemapHooks[]={
	{HOOK_IAT_CANDIDATE, 0, "SetViewportOrgEx", (FARPROC)SetViewportOrgEx, (FARPROC *)&pSetViewportOrgEx, (FARPROC)extSetViewportOrgEx}, // needed in ShowBanner
	{HOOK_IAT_CANDIDATE, 0, "SetViewportExtEx", (FARPROC)SetViewportExtEx, (FARPROC *)&pSetViewportExtEx, (FARPROC)extSetViewportExtEx},
	{HOOK_IAT_CANDIDATE, 0, "GetViewportOrgEx", (FARPROC)GetViewportOrgEx, (FARPROC *)&pGetViewportOrgEx, (FARPROC)extGetViewportOrgEx},
	{HOOK_IAT_CANDIDATE, 0, "GetViewportExtEx", (FARPROC)GetViewportExtEx, (FARPROC *)&pGetViewportExtEx, (FARPROC)extGetViewportExtEx},
	{HOOK_IAT_CANDIDATE, 0, "GetWindowOrgEx", (FARPROC)GetWindowOrgEx, (FARPROC *)&pGetWindowOrgEx, (FARPROC)extGetWindowOrgEx},
	{HOOK_IAT_CANDIDATE, 0, "SetWindowOrgEx", (FARPROC)SetWindowOrgEx, (FARPROC *)&pSetWindowOrgEx, (FARPROC)extSetWindowOrgEx},
	{HOOK_IAT_CANDIDATE, 0, "GetCurrentPositionEx", (FARPROC)GetCurrentPositionEx, (FARPROC *)&pGetCurrentPositionEx, (FARPROC)extGetCurrentPositionEx},
	{HOOK_IAT_CANDIDATE, 0, "GetRgnBox", (FARPROC)GetRgnBox, (FARPROC *)&pGetRgnBox, (FARPROC)extGetRgnBox},
	//{HOOK_IAT_CANDIDATE, 0, "GetRegionData", (FARPROC)NULL, (FARPROC *)&pGetRegionData, (FARPROC)extGetRegionData},
	{HOOK_IAT_CANDIDATE, 0, "CreateCompatibleDC", (FARPROC)CreateCompatibleDC, (FARPROC *)&pGDICreateCompatibleDC, (FARPROC)extGDICreateCompatibleDC}, /* to check */
	//TODO {HOOK_IAT_CANDIDATE, 0, "DrawEscape", (FARPROC)DrawEscape, (FARPROC *)&pDrawEscape, (FARPROC)extDrawEscape}, /* to check */
	{HOOK_IAT_CANDIDATE, 0, 0, NULL, 0, 0} // terminator 
};

static HookEntryEx_Type SyscallHooks[]={
	{HOOK_IAT_CANDIDATE, 0, "StretchDIBits", (FARPROC)StretchDIBits, (FARPROC *)&pStretchDIBits, (FARPROC)extStretchDIBits},
	{HOOK_HOT_CANDIDATE, 0, "SetDIBits", (FARPROC)SetDIBits, (FARPROC *)&pSetDIBits, (FARPROC)extSetDIBits},
	{HOOK_HOT_CANDIDATE, 0, "GetDIBits", (FARPROC)GetDIBits, (FARPROC *)&pGetDIBits, (FARPROC)extGetDIBits},
	{HOOK_IAT_CANDIDATE, 0, "CreateCompatibleBitmap", (FARPROC)CreateCompatibleBitmap, (FARPROC *)&pCreateCompatibleBitmap, (FARPROC)extCreateCompatibleBitmap}, 
	{HOOK_IAT_CANDIDATE, 0, "CreateDIBitmap", (FARPROC)NULL, (FARPROC *)&pCreateDIBitmap, (FARPROC)extCreateDIBitmap}, 
	{HOOK_IAT_CANDIDATE, 0, "CreateDIBSection", (FARPROC)NULL, (FARPROC *)&pCreateDIBSection, (FARPROC)extCreateDIBSection}, 
	{HOOK_IAT_CANDIDATE, 0, "CreateDiscardableBitmap", (FARPROC)NULL, (FARPROC *)&pCreateDiscardableBitmap, (FARPROC)extCreateDiscardableBitmap}, 
	{HOOK_IAT_CANDIDATE, 0, "ExtFloodFill", (FARPROC)NULL, (FARPROC *)&pExtFloodFill, (FARPROC)extExtFloodFill}, 
	{HOOK_IAT_CANDIDATE, 0, "GdiAlphaBlend", (FARPROC)NULL, (FARPROC *)&pGdiAlphaBlend, (FARPROC)extGdiAlphaBlend}, 
	{HOOK_IAT_CANDIDATE, 0, "GdiGradientFill", (FARPROC)NULL, (FARPROC *)&pGdiGradientFill, (FARPROC)extGdiGradientFill},
	{HOOK_IAT_CANDIDATE, 0, "GdiTransparentBlt", (FARPROC)NULL, (FARPROC *)&pGdiTransparentBlt, (FARPROC)extGdiTransparentBlt}, 
	{HOOK_IAT_CANDIDATE, 0, "Pie", (FARPROC)NULL, (FARPROC *)&pPie, (FARPROC)extPie},
	{HOOK_IAT_CANDIDATE, 0, "AngleArc", (FARPROC)NULL, (FARPROC *)&pAngleArc, (FARPROC)extAngleArc}, 
	{HOOK_IAT_CANDIDATE, 0, "PolyPolyline", (FARPROC)NULL, (FARPROC *)&pPolyPolyline, (FARPROC)extPolyPolyline},
	{HOOK_IAT_CANDIDATE, 0, "FillRgn", (FARPROC)NULL, (FARPROC *)&pFillRgn, (FARPROC)extFillRgn},
	{HOOK_IAT_CANDIDATE, 0, "FrameRgn", (FARPROC)NULL, (FARPROC *)&pFrameRgn, (FARPROC)extFrameRgn}, 
	{HOOK_IAT_CANDIDATE, 0, "InvertRgn", (FARPROC)NULL, (FARPROC *)&pInvertRgn, (FARPROC)extInvertRgn}, 
	{HOOK_IAT_CANDIDATE, 0, "PaintRgn", (FARPROC)NULL, (FARPROC *)&pPaintRgn, (FARPROC)extPaintRgn}, 
	//{HOOK_IAT_CANDIDATE, 0, "SetMapMode", (FARPROC)NULL, (FARPROC *)NULL, (FARPROC)extSetMapMode}, // crashes ???
	{HOOK_IAT_CANDIDATE, 0, "SetDIBitsToDevice", (FARPROC)SetDIBitsToDevice, (FARPROC *)&pSetDIBitsToDevice, (FARPROC)extSetDIBitsToDevice}, // does the stretching
	{HOOK_IAT_CANDIDATE, 0, "Polyline", (FARPROC)Polyline, (FARPROC *)&pPolyline, (FARPROC)extPolyline},
	{HOOK_IAT_CANDIDATE, 0, "BitBlt", (FARPROC)BitBlt, (FARPROC *)&pGDIBitBlt, (FARPROC)extGDIBitBlt},
	{HOOK_IAT_CANDIDATE, 0, "StretchBlt", (FARPROC)StretchBlt, (FARPROC *)&pGDIStretchBlt, (FARPROC)extGDIStretchBlt},
	{HOOK_IAT_CANDIDATE, 0, "PatBlt", (FARPROC)PatBlt, (FARPROC *)&pGDIPatBlt, (FARPROC)extGDIPatBlt},
	{HOOK_IAT_CANDIDATE, 0, "MaskBlt", (FARPROC)MaskBlt, (FARPROC *)&pMaskBlt, (FARPROC)extMaskBlt},
	{HOOK_IAT_CANDIDATE, 0, "TextOutA", (FARPROC)TextOutA, (FARPROC *)&pGDITextOutA, (FARPROC)extTextOutA},
	{HOOK_IAT_CANDIDATE, 0, "TextOutW", (FARPROC)TextOutW, (FARPROC *)&pGDITextOutW, (FARPROC)extTextOutW},
	{HOOK_IAT_CANDIDATE, 0, "Rectangle", (FARPROC)Rectangle, (FARPROC *)&pGDIRectangle, (FARPROC)extRectangle},
	{HOOK_IAT_CANDIDATE, 0, "RoundRect", (FARPROC)RoundRect, (FARPROC *)&pRoundRect, (FARPROC)extRoundRect},
	{HOOK_IAT_CANDIDATE, 0, "Polygon", (FARPROC)Polygon, (FARPROC *)&pPolygon, (FARPROC)extPolygon},
	{HOOK_IAT_CANDIDATE, 0, "PolyPolygon", (FARPROC)PolyPolygon, (FARPROC *)&pPolyPolygon, (FARPROC)extPolyPolygon},
	{HOOK_IAT_CANDIDATE, 0, "PolyBezier", (FARPROC)PolyBezier, (FARPROC *)&pPolyBezier, (FARPROC)extPolyBezier},
	{HOOK_IAT_CANDIDATE, 0, "PolyBezierTo", (FARPROC)PolyBezierTo, (FARPROC *)&pPolyBezierTo, (FARPROC)extPolyBezierTo},
	{HOOK_IAT_CANDIDATE, 0, "PolylineTo", (FARPROC)PolylineTo, (FARPROC *)&pPolylineTo, (FARPROC)extPolylineTo},
	{HOOK_IAT_CANDIDATE, 0, "PolyDraw", (FARPROC)PolyDraw, (FARPROC *)&pPolyDraw, (FARPROC)extPolyDraw},
	{HOOK_IAT_CANDIDATE, 0, "GetPixel", (FARPROC)GetPixel, (FARPROC *)&pGetPixel, (FARPROC)extGetPixel},
	{HOOK_IAT_CANDIDATE, 0, "PlgBlt", (FARPROC)PlgBlt, (FARPROC *)&pPlgBlt, (FARPROC)extPlgBlt},
	{HOOK_IAT_CANDIDATE, 0, "SetPixel", (FARPROC)SetPixel, (FARPROC *)&pSetPixel, (FARPROC)extSetPixel},
	{HOOK_IAT_CANDIDATE, 0, "SetPixelV", (FARPROC)SetPixelV, (FARPROC *)&pSetPixelV, (FARPROC)extSetPixelV},
	{HOOK_IAT_CANDIDATE, 0, "Chord", (FARPROC)Chord, (FARPROC *)&pChord, (FARPROC)extChord},
	{HOOK_IAT_CANDIDATE, 0, "Ellipse", (FARPROC)Ellipse, (FARPROC *)&pEllipse, (FARPROC)extEllipse},
	{HOOK_IAT_CANDIDATE, 0, "ExtTextOutA", (FARPROC)ExtTextOutA, (FARPROC *)&pExtTextOutA, (FARPROC)extExtTextOutA},
	{HOOK_IAT_CANDIDATE, 0, "ExtTextOutW", (FARPROC)ExtTextOutW, (FARPROC *)&pExtTextOutW, (FARPROC)extExtTextOutW},
	{HOOK_IAT_CANDIDATE, 0, "PolyTextOutA", (FARPROC)PolyTextOutA, (FARPROC *)&pPolyTextOutA, (FARPROC)extPolyTextOutA},
	{HOOK_IAT_CANDIDATE, 0, "PolyTextOutW", (FARPROC)PolyTextOutA, (FARPROC *)&pPolyTextOutA, (FARPROC)extPolyTextOutA},
	{HOOK_IAT_CANDIDATE, 0, "ArcTo", (FARPROC)ArcTo, (FARPROC *)&pArcTo, (FARPROC)extArcTo},
	{HOOK_IAT_CANDIDATE, 0, "LineTo", (FARPROC)LineTo, (FARPROC *)&pLineTo, (FARPROC)extLineTo},
	{HOOK_IAT_CANDIDATE, 0, "Arc", (FARPROC)Arc, (FARPROC *)&pArc, (FARPROC)extArc},
	{HOOK_IAT_CANDIDATE, 0, "MoveToEx", (FARPROC)MoveToEx, (FARPROC *)&pMoveToEx, (FARPROC)extMoveToEx},
	{HOOK_IAT_CANDIDATE, 0, "GetClipBox", (FARPROC)GetClipBox, (FARPROC *)&pGDIGetClipBox, (FARPROC)extGetClipBox},
	{HOOK_IAT_CANDIDATE, 0, "IntersectClipRect", (FARPROC)IntersectClipRect, (FARPROC *)&pIntersectClipRect, (FARPROC)extIntersectClipRect}, // Riven !!
	{HOOK_IAT_CANDIDATE, 0, "DeleteDC", (FARPROC)DeleteDC, (FARPROC *)&pGDIDeleteDC, (FARPROC)extGDIDeleteDC}, // for tracing only!
	{HOOK_IAT_CANDIDATE, 0, "CreateDCA", (FARPROC)CreateDCA, (FARPROC *)&pGDICreateDCA, (FARPROC)extGDICreateDCA}, 
	{HOOK_IAT_CANDIDATE, 0, "CreateDCW", (FARPROC)CreateDCW, (FARPROC *)&pGDICreateDCW, (FARPROC)extGDICreateDCW}, 
	// CreateDCW .....	
	{HOOK_IAT_CANDIDATE, 0, 0, NULL, 0, 0} // terminator 
};

static HookEntryEx_Type ScaledHooks[]={
	// commented out since they alter text on screen...... (see Imperialism II difficulty level menu)
	// v2.03.47 - restored: needed for "688(I) Hunter Killer" periscope ....
	{HOOK_IAT_CANDIDATE, 0, "CreateEllipticRgn", (FARPROC)CreateEllipticRgn, (FARPROC *)&pCreateEllipticRgn, (FARPROC)extCreateEllipticRgn},
	{HOOK_IAT_CANDIDATE, 0, "CreateEllipticRgnIndirect", (FARPROC)CreateEllipticRgnIndirect, (FARPROC *)&pCreateEllipticRgnIndirect, (FARPROC)extCreateEllipticRgnIndirect},
	// CreateRectRgn must be hooked in scaled mode to let Avernum work correctly!
	{HOOK_IAT_CANDIDATE, 0, "CreateRectRgn", (FARPROC)CreateRectRgn, (FARPROC *)&pCreateRectRgn, (FARPROC)extCreateRectRgn},
	{HOOK_IAT_CANDIDATE, 0, "CreateRectRgnIndirect", (FARPROC)CreateRectRgnIndirect, (FARPROC *)&pCreateRectRgnIndirect, (FARPROC)extCreateRectRgnIndirect},
	{HOOK_IAT_CANDIDATE, 0, "CreatePolygonRgn", (FARPROC)CreatePolygonRgn, (FARPROC *)&pCreatePolygonRgn, (FARPROC)extCreatePolygonRgn},
	// same as emulated GDI ...

	{HOOK_IAT_CANDIDATE, 0, "SetRectRgn", (FARPROC)SetRectRgn, (FARPROC *)&pSetRectRgn, (FARPROC)extSetRectRgn}, 
	{HOOK_IAT_CANDIDATE, 0, 0, NULL, 0, 0} // terminator
};

static HookEntryEx_Type EmulateHooks[]={
	{HOOK_IAT_CANDIDATE, 0, "GetObjectType", (FARPROC)GetObjectType, (FARPROC *)&pGetObjectType, (FARPROC)extGetObjectType},
	{HOOK_IAT_CANDIDATE, 0, 0, NULL, 0, 0} // terminator
};

static HookEntryEx_Type TextHooks[]={
	{HOOK_IAT_CANDIDATE, 0, "CreateFontA", (FARPROC)CreateFont, (FARPROC *)&pGDICreateFont, (FARPROC)extCreateFont},
	{HOOK_IAT_CANDIDATE, 0, "CreateFontIndirectA", (FARPROC)CreateFontIndirectA, (FARPROC *)&pGDICreateFontIndirect, (FARPROC)extCreateFontIndirect},
	{HOOK_IAT_CANDIDATE, 0, 0, NULL, 0, 0} // terminator
};

static HookEntryEx_Type GammaHooks[]={
	{HOOK_IAT_CANDIDATE, 0, "SetDeviceGammaRamp", (FARPROC)SetDeviceGammaRamp, (FARPROC *)&pGDISetDeviceGammaRamp, (FARPROC)extSetDeviceGammaRamp},
	{HOOK_IAT_CANDIDATE, 0, "GetDeviceGammaRamp", (FARPROC)GetDeviceGammaRamp, (FARPROC *)&pGDIGetDeviceGammaRamp, (FARPROC)extGetDeviceGammaRamp},
	{HOOK_IAT_CANDIDATE, 0, 0, NULL, 0, 0} // terminator
};

static HookEntryEx_Type FontHooks[]={
	{HOOK_IAT_CANDIDATE, 0, "CreateScalableFontResourceA", (FARPROC)CreateScalableFontResourceA, (FARPROC *)&pCreateScalableFontResourceA, (FARPROC)extCreateScalableFontResourceA},
	{HOOK_IAT_CANDIDATE, 0, "CreateScalableFontResourceW", (FARPROC)CreateScalableFontResourceW, (FARPROC *)&pCreateScalableFontResourceW, (FARPROC)extCreateScalableFontResourceW},
	{HOOK_IAT_CANDIDATE, 0, "AddFontResourceA", (FARPROC)AddFontResourceA, (FARPROC *)&pAddFontResourceA, (FARPROC)extAddFontResourceA},
	{HOOK_IAT_CANDIDATE, 0, "AddFontResourceW", (FARPROC)AddFontResourceW, (FARPROC *)&pAddFontResourceW, (FARPROC)extAddFontResourceW},
	{HOOK_IAT_CANDIDATE, 0, 0, NULL, 0, 0} // terminator
};

/*
static HookEntryEx_Type LockWHooks[]={
	{HOOK_HOT_CANDIDATE, 0, "SetBkColor", (FARPROC)SetBkColor, (FARPROC *)&pSetBkColor, (FARPROC)extSetBkColor},
	{HOOK_HOT_CANDIDATE, 0, "SetBkMode", (FARPROC)SetBkMode, (FARPROC *)&pSetBkMode, (FARPROC)extSetBkMode},
	{HOOK_HOT_CANDIDATE, 0, "SetTextColor", (FARPROC)SetTextColor, (FARPROC *)&pSetTextColor, (FARPROC)extSetTextColor},
	{HOOK_IAT_CANDIDATE, 0, 0, NULL, 0, 0} // terminator
};
*/

extern HRESULT WINAPI extDirectDrawCreate(GUID FAR *, LPDIRECTDRAW FAR *, IUnknown FAR *);
extern HRESULT WINAPI extDirectDrawCreateEx(GUID FAR *, LPDIRECTDRAW FAR *, REFIID, IUnknown FAR *);

static char *libname = "gdi32.dll";

void HookGDI32Init()
{
	HookLibInitEx(Hooks);
	HookLibInitEx(RemapHooks);
	HookLibInitEx(ScaledHooks);
	HookLibInitEx(SyscallHooks);
	HookLibInitEx(EmulateHooks);
	HookLibInitEx(TextHooks);
	HookLibInitEx(GammaHooks);
	//HookLibInitEx(LockWHooks);
}

void HookGDI32(HMODULE module)
{
	HookLibraryEx(module, Hooks, libname);

	if (dxw.GDIEmulationMode != GDIMODE_NONE) HookLibraryEx(module, SyscallHooks, libname);
	if (dxw.dwFlags1 & CLIENTREMAPPING)		HookLibraryEx(module, RemapHooks, libname);
	if (dxw.dwFlags2 & GDISTRETCHED)		HookLibraryEx(module, ScaledHooks, libname);
	if (dxw.dwFlags3 & GDIEMULATEDC)		HookLibraryEx(module, EmulateHooks, libname);	
	if (dxw.dwFlags1 & FIXTEXTOUT)			HookLibraryEx(module, TextHooks, libname);
	if (dxw.dwFlags2 & DISABLEGAMMARAMP)	HookLibraryEx(module, GammaHooks, libname);
	// v2.02.33 - for "Stratego" compatibility option
	if(dxw.dwFlags3 & FONTBYPASS)			HookLibraryEx(module, FontHooks, libname);
	//if(1)									HookLibraryEx(module, LockWHooks, libname);
}

FARPROC Remap_GDI32_ProcAddress(LPCSTR proc, HMODULE hModule)
{
	FARPROC addr;

	if(addr=RemapLibraryEx(proc, hModule, Hooks)) return addr;

	if (dxw.GDIEmulationMode != GDIMODE_NONE) if(addr=RemapLibraryEx(proc, hModule, SyscallHooks)) return addr;
	if (dxw.dwFlags1 & CLIENTREMAPPING)		if(addr=RemapLibraryEx(proc, hModule, RemapHooks)) return addr;
	if (dxw.dwFlags2 & GDISTRETCHED)		if (addr=RemapLibraryEx(proc, hModule, ScaledHooks)) return addr;
	if (dxw.dwFlags3 & GDIEMULATEDC)		if (addr=RemapLibraryEx(proc, hModule, EmulateHooks)) return addr;
	if (dxw.dwFlags1 & FIXTEXTOUT)			if(addr=RemapLibraryEx(proc, hModule, TextHooks)) return addr;
	if (dxw.dwFlags2 & DISABLEGAMMARAMP)	if(addr=RemapLibraryEx(proc, hModule, GammaHooks)) return addr;
	// v2.02.33 - for "Stratego" compatibility option
	if (dxw.dwFlags3 & FONTBYPASS)			if(addr=RemapLibraryEx(proc, hModule, FontHooks)) return addr;
	//if (1)									if(addr=RemapLibraryEx(proc, hModule, LockWHooks)) return addr;

	return NULL;
}

//--------------------------------------------------------------------------------------------
//
// extern and common functions
//
//--------------------------------------------------------------------------------------------

extern DWORD PaletteEntries[256];
extern Unlock4_Type pUnlockMethod(LPDIRECTDRAWSURFACE);
extern HRESULT WINAPI sBlt(char *, LPDIRECTDRAWSURFACE, LPRECT, LPDIRECTDRAWSURFACE, LPRECT, DWORD, LPDDBLTFX, BOOL);

extern GetDC_Type pGetDC;
extern ReleaseDC_Type pReleaseDC;

//--------------------------------------------------------------------------------------------
//
// API hookers
//
//--------------------------------------------------------------------------------------------

int WINAPI extGetDeviceCaps(HDC hdc, int nindex)
{
	DWORD res;
	
	res = (*pGDIGetDeviceCaps)(hdc, nindex);
	if(IsTraceDDRAW){
		char sInfo[1024];
		sprintf(sInfo, "GetDeviceCaps: hdc=%x index=%x(%s)", hdc, nindex, ExplainDeviceCaps(nindex));
		switch(nindex){
			case RASTERCAPS:
				sprintf(sInfo, "%s res=0x%04x(%s)\n", sInfo, res, ExplainRasterCaps(res)); break;
			case BITSPIXEL:
			case COLORRES:
			case VERTRES:
			case HORZRES:
			case SIZEPALETTE:
			case NUMRESERVED:
				sprintf(sInfo, "%s res=%d\n", sInfo, res); break;
			default:
				sprintf(sInfo, "%s res=0x%04x\n", sInfo, res); break;
		}
		OutTrace(sInfo);
	}

	switch(nindex){
	case VERTRES:
		if(dxw.Windowize){
			if(dxw.IsDesktop(WindowFromDC(hdc))) 
				res= dxw.GetScreenHeight();
			else {
				if(OBJ_DC == (*pGetObjectType)(hdc)){
					switch(dxw.GDIEmulationMode){
						case GDIMODE_SHAREDDC:
						case GDIMODE_EMULATED:
						default:
							break;
						case GDIMODE_STRETCHED:
							int dummy = 0;
							dxw.UnmapClient(&dummy, (int *)&res);
							break;
					}
				}
			}
			OutTraceDW("GetDeviceCaps: fix(2) VERTRES cap=%d\n", res);
		}
		break;
	case HORZRES:
		if(dxw.Windowize){
			if(dxw.IsDesktop(WindowFromDC(hdc))) 
				res= dxw.GetScreenWidth();
			else {
				if(OBJ_DC == (*pGetObjectType)(hdc)){
					switch(dxw.GDIEmulationMode){
						case GDIMODE_SHAREDDC:
						case GDIMODE_EMULATED:
						default:
							break;
						case GDIMODE_STRETCHED:
							int dummy = 0;
							dxw.UnmapClient((int *)&res, &dummy);
							break;
					}
				}
			}
			OutTraceDW("GetDeviceCaps: fix(2) HORZRES cap=%d\n", res);
		}
		break;



		if(dxw.Windowize){
			res= dxw.GetScreenWidth();
		}
		break;
	// WARNING: in no-emu mode, the INIT8BPP and INIT16BPP flags expose capabilities that
	// are NOT implemented and may cause later troubles!
	case RASTERCAPS:
		if(dxw.dwFlags2 & INIT8BPP) {
			res |= RC_PALETTE; // v2.02.12
			OutTraceDW("GetDeviceCaps: fix(2) RASTERCAPS setting RC_PALETTE cap=%x\n", res);
		}
		break;
	case BITSPIXEL:
	case COLORRES:
		if(dxw.dwFlags2 & (INIT8BPP|INIT16BPP)){ // v2.02.32 fix
			if(dxw.dwFlags2 & INIT8BPP) res = 8;
			if(dxw.dwFlags2 & INIT16BPP) res = 16;
			OutTraceDW("GetDeviceCaps: fix(2) BITSPIXEL/COLORRES cap=%d\n", res);
		}
		break;
	//case NUMCOLORS: // numcolors windows bug fix....
	//	if(res == -1) res=1;
	//	return res;
	}

	if(dxw.dwFlags1 & EMULATESURFACE){
		switch(nindex){
		case RASTERCAPS:
			if((dxw.VirtualPixelFormat.dwRGBBitCount==8) || (dxw.dwFlags2 & INIT8BPP)){
				res |= RC_PALETTE;
				OutTraceDW("GetDeviceCaps: fix(3) RASTERCAPS setting RC_PALETTE cap=%x(%s)\n", res, ExplainRasterCaps(res));
			}
			break;
		case BITSPIXEL:
		case COLORRES:
			int PrevRes;
			PrevRes=res;
			if(dxw.VirtualPixelFormat.dwRGBBitCount!=0) res = dxw.VirtualPixelFormat.dwRGBBitCount;
			if(dxw.dwFlags2 & INIT8BPP) res = 8;
			if(dxw.dwFlags2 & INIT16BPP) res = 16;
			if(PrevRes != res) OutTraceDW("GetDeviceCaps: fix(3) BITSPIXEL/COLORRES cap=%d\n", res);
			break;
		case SIZEPALETTE:
			res = 256;
			OutTraceDW("GetDeviceCaps: fix(3) SIZEPALETTE cap=%x\n", res);
			break;
		case NUMRESERVED:
			res = 0;
			OutTraceDW("GetDeviceCaps: fix(3) NUMRESERVED cap=%x\n", res);
			break;
		}
	}
	return res;
}

BOOL WINAPI extTextOutA(HDC hdc, int nXStart, int nYStart, LPCTSTR lpString, int cchString)
{
	BOOL ret;
	extern BOOL gFixed;
	OutTraceDW("TextOutA: hdc=%x xy=(%d,%d) str=(%d)\"%.*s\"\n", hdc, nXStart, nYStart, cchString, cchString, lpString);

	if (!gFixed && dxw.IsToRemap(hdc)){

		switch(dxw.GDIEmulationMode){
			case GDIMODE_SHAREDDC:
				sdc.GetPrimaryDC(hdc);
				ret=(*pGDITextOutA)(sdc.GetHdc(), nXStart, nYStart, lpString, cchString);
				// update whole screen to avoid make calculations about text position & size
				sdc.PutPrimaryDC(hdc, TRUE);
				return ret;
				break;
			case GDIMODE_STRETCHED: 
				dxw.MapClient(&nXStart, &nYStart);
				OutTraceDW("TextOut: fixed dest=(%d,%d)\n", nXStart, nYStart);
				break;
		}
	}

	ret=(*pGDITextOutA)(hdc, nXStart, nYStart, lpString, cchString);
	if(!ret) OutTraceE("TextOutA: ERROR ret=%x\n", ret); 
	return ret;
}

BOOL WINAPI extTextOutW(HDC hdc, int nXStart, int nYStart, LPCWSTR lpString, int cchString)
{
	BOOL ret;
	extern BOOL gFixed;
	OutTraceDW("TextOutW: hdc=%x xy=(%d,%d) str=(%d)\"%.*ls\"\n", hdc, nXStart, nYStart, cchString, cchString, lpString);

	if (!gFixed && dxw.IsToRemap(hdc)){

		switch(dxw.GDIEmulationMode){
			case GDIMODE_SHAREDDC:
				sdc.GetPrimaryDC(hdc);
				ret=(*pGDITextOutW)(sdc.GetHdc(), nXStart, nYStart, lpString, cchString);
				// update whole screen to avoid make calculations about text position & size
				sdc.PutPrimaryDC(hdc, TRUE);
				return ret;
				break;
			case GDIMODE_STRETCHED: 
				dxw.MapClient(&nXStart, &nYStart);
				OutTraceDW("TextOutW: fixed dest=(%d,%d)\n", nXStart, nYStart);
				break;
		}
	}

	ret=(*pGDITextOutW)(hdc, nXStart, nYStart, lpString, cchString);
	if(!ret) OutTraceE("TextOutW: ERROR ret=%x\n", ret); 
	return ret;
}

BOOL WINAPI extScaleWindowExtEx(HDC hdc, int Xnum, int Xdenom, int Ynum, int Ydenom, LPSIZE lpSize)
{
	OutTraceDW("ScaleWindowExtEx: hdc=%x num=(%d,%d) denom=(%d,%d) lpSize=%d\n",
		hdc, Xnum, Ynum, Xdenom, Ydenom, lpSize);

	MessageBox(0, "ScaleWindowExtEx", "to fix", MB_OK | MB_ICONEXCLAMATION);

	return (*pGDIScaleWindowExtEx)(hdc, Xnum, Xdenom, Ynum, Ydenom, lpSize);
}

BOOL WINAPI extRectangle(HDC hdc, int nLeftRect, int nTopRect, int nRightRect, int nBottomRect)
{
	int ret;

	OutTraceDW("Rectangle: hdc=%x xy=(%d,%d)-(%d,%d)\n", hdc, nLeftRect, nTopRect, nRightRect, nBottomRect);

	if (dxw.IsToRemap(hdc)){
		switch(dxw.GDIEmulationMode){
			case GDIMODE_SHAREDDC:
				sdc.GetPrimaryDC(hdc);
				ret=(*pGDIRectangle)(sdc.GetHdc(), nLeftRect, nTopRect, nRightRect, nBottomRect);
				sdc.PutPrimaryDC(hdc, TRUE, nLeftRect, nTopRect, nRightRect-nLeftRect, nBottomRect-nTopRect);
				return ret;
				break;
			case GDIMODE_STRETCHED: 
				dxw.MapClient(&nLeftRect, &nTopRect, &nRightRect, &nBottomRect);
				OutTraceDW("Rectangle: fixed dest=(%d,%d)-(%d,%d)\n", nLeftRect, nTopRect, nRightRect, nBottomRect);
				break;
		}
	}

	ret=(*pGDIRectangle)(hdc, nLeftRect, nTopRect, nRightRect, nBottomRect);
	if(!ret) OutTraceE("Rectangle: ERROR ret=%x\n", ret); 
	return ret;
}

int WINAPI extGDISaveDC(HDC hdc)
{
	int ret;
	ret=(*pGDISaveDC)(hdc);
	OutTraceDW("GDI.SaveDC: hdc=%x ret=%x\n", hdc, ret);
	return ret;
}

BOOL WINAPI extGDIRestoreDC(HDC hdc, int nSavedDC)
{
	BOOL ret;
	ret=(*pGDIRestoreDC)(hdc, nSavedDC);
	OutTraceDW("GDI.RestoreDC: hdc=%x nSavedDC=%x ret=%x\n", hdc, nSavedDC, ret);
	return ret;
}

/* --------------------------------------------------------------------------- */
/*     Palette handling                                                        */
/* --------------------------------------------------------------------------- */

extern HDC hFlippedDC;
extern BOOL bFlippedDC;
extern void mySetPalette(int, int, LPPALETTEENTRY);

// v2.1.75: Hooking for GDI32 CreatePalette, SelectPalette, RealizePalette: 
// maps the GDI palette to the buffered DirectDraw one. This fixes the screen 
// output for "Dementia" (a.k.a. "Armed & Delirious").

// In emulated mode (when color depyth is 8BPP ?) it may happen that the game
// expects to get the requested system palette entries, while the 32BPP screen
// returns 0. "Mission Force Cyberstorm" is one of these. Returning the same
// value as nEntries, even though lppe is untouched, fixes the problem.

PALETTEENTRY DefaultSystemPalette[256]={ // default palette, captured on my PC with video mode set to 8BPP
	{0x00,0x00,0x00,0x00},{0x80,0x00,0x00,0x00},{0x00,0x80,0x00,0x00},{0x80,0x80,0x00,0x00},
	{0x00,0x00,0x80,0x00},{0x80,0x00,0x80,0x00},{0x00,0x80,0x80,0x00},{0xc0,0xc0,0xc0,0x00},
	{0xa0,0xa0,0xa0,0x00},{0xf0,0xf0,0xf0,0x00},{0xc0,0xdc,0xc0,0x00},{0xa6,0xca,0xf0,0x00},
	{0x04,0x04,0x04,0x00},{0x08,0x08,0x08,0x00},{0x0c,0x0c,0x0c,0x00},{0x11,0x11,0x11,0x00},
	{0x16,0x16,0x16,0x00},{0x1c,0x1c,0x1c,0x00},{0x22,0x22,0x22,0x00},{0x29,0x29,0x29,0x00},
	{0x55,0x55,0x55,0x00},{0x4d,0x4d,0x4d,0x00},{0x42,0x42,0x42,0x00},{0x39,0x39,0x39,0x00},
	{0xff,0x7c,0x80,0x00},{0xff,0x50,0x50,0x00},{0xd6,0x00,0x93,0x00},{0xcc,0xec,0xff,0x00},
	{0xef,0xd6,0xc6,0x00},{0xe7,0xe7,0xd6,0x00},{0xad,0xa9,0x90,0x00},{0x33,0x00,0x00,0x00},
	{0x66,0x00,0x00,0x00},{0x99,0x00,0x00,0x00},{0xcc,0x00,0x00,0x00},{0x00,0x33,0x00,0x00},
	{0x33,0x33,0x00,0x00},{0x66,0x33,0x00,0x00},{0x99,0x33,0x00,0x00},{0xcc,0x33,0x00,0x00},
	{0xff,0x33,0x00,0x00},{0x00,0x66,0x00,0x00},{0x33,0x66,0x00,0x00},{0x66,0x66,0x00,0x00},
	{0x99,0x66,0x00,0x00},{0xcc,0x66,0x00,0x00},{0xff,0x66,0x00,0x00},{0x00,0x99,0x00,0x00},
	{0x33,0x99,0x00,0x00},{0x66,0x99,0x00,0x00},{0x99,0x99,0x00,0x00},{0xcc,0x99,0x00,0x00},
	{0xff,0x99,0x00,0x00},{0x00,0xcc,0x00,0x00},{0x33,0xcc,0x00,0x00},{0x66,0xcc,0x00,0x00},
	{0x99,0xcc,0x00,0x00},{0xcc,0xcc,0x00,0x00},{0xff,0xcc,0x00,0x00},{0x66,0xff,0x00,0x00},
	{0x99,0xff,0x00,0x00},{0xcc,0xff,0x00,0x00},{0x00,0x00,0x33,0x00},{0x33,0x00,0x33,0x00},
	{0x66,0x00,0x33,0x00},{0x99,0x00,0x33,0x00},{0xcc,0x00,0x33,0x00},{0xff,0x00,0x33,0x00},
	{0x00,0x33,0x33,0x00},{0x33,0x33,0x33,0x00},{0x66,0x33,0x33,0x00},{0x99,0x33,0x33,0x00},
	{0xcc,0x33,0x33,0x00},{0xff,0x33,0x33,0x00},{0x00,0x66,0x33,0x00},{0x33,0x66,0x33,0x00},
	{0x66,0x66,0x33,0x00},{0x99,0x66,0x33,0x00},{0xcc,0x66,0x33,0x00},{0xff,0x66,0x33,0x00},
	{0x00,0x99,0x33,0x00},{0x33,0x99,0x33,0x00},{0x66,0x99,0x33,0x00},{0x99,0x99,0x33,0x00},
	{0xcc,0x99,0x33,0x00},{0xff,0x99,0x33,0x00},{0x00,0xcc,0x33,0x00},{0x33,0xcc,0x33,0x00},
	{0x66,0xcc,0x33,0x00},{0x99,0xcc,0x33,0x00},{0xcc,0xcc,0x33,0x00},{0xff,0xcc,0x33,0x00},
	{0x33,0xff,0x33,0x00},{0x66,0xff,0x33,0x00},{0x99,0xff,0x33,0x00},{0xcc,0xff,0x33,0x00},
	{0xff,0xff,0x33,0x00},{0x00,0x00,0x66,0x00},{0x33,0x00,0x66,0x00},{0x66,0x00,0x66,0x00},
	{0x99,0x00,0x66,0x00},{0xcc,0x00,0x66,0x00},{0xff,0x00,0x66,0x00},{0x00,0x33,0x66,0x00},
	{0x33,0x33,0x66,0x00},{0x66,0x33,0x66,0x00},{0x99,0x33,0x66,0x00},{0xcc,0x33,0x66,0x00},
	{0xff,0x33,0x66,0x00},{0x00,0x66,0x66,0x00},{0x33,0x66,0x66,0x00},{0x66,0x66,0x66,0x00},
	{0x99,0x66,0x66,0x00},{0xcc,0x66,0x66,0x00},{0x00,0x99,0x66,0x00},{0x33,0x99,0x66,0x00},
	{0x66,0x99,0x66,0x00},{0x99,0x99,0x66,0x00},{0xcc,0x99,0x66,0x00},{0xff,0x99,0x66,0x00},
	{0x00,0xcc,0x66,0x00},{0x33,0xcc,0x66,0x00},{0x99,0xcc,0x66,0x00},{0xcc,0xcc,0x66,0x00},
	{0xff,0xcc,0x66,0x00},{0x00,0xff,0x66,0x00},{0x33,0xff,0x66,0x00},{0x99,0xff,0x66,0x00},
	{0xcc,0xff,0x66,0x00},{0xff,0x00,0xcc,0x00},{0xcc,0x00,0xff,0x00},{0x00,0x99,0x99,0x00},
	{0x99,0x33,0x99,0x00},{0x99,0x00,0x99,0x00},{0xcc,0x00,0x99,0x00},{0x00,0x00,0x99,0x00},
	{0x33,0x33,0x99,0x00},{0x66,0x00,0x99,0x00},{0xcc,0x33,0x99,0x00},{0xff,0x00,0x99,0x00},
	{0x00,0x66,0x99,0x00},{0x33,0x66,0x99,0x00},{0x66,0x33,0x99,0x00},{0x99,0x66,0x99,0x00},
	{0xcc,0x66,0x99,0x00},{0xff,0x33,0x99,0x00},{0x33,0x99,0x99,0x00},{0x66,0x99,0x99,0x00},
	{0x99,0x99,0x99,0x00},{0xcc,0x99,0x99,0x00},{0xff,0x99,0x99,0x00},{0x00,0xcc,0x99,0x00},
	{0x33,0xcc,0x99,0x00},{0x66,0xcc,0x66,0x00},{0x99,0xcc,0x99,0x00},{0xcc,0xcc,0x99,0x00},
	{0xff,0xcc,0x99,0x00},{0x00,0xff,0x99,0x00},{0x33,0xff,0x99,0x00},{0x66,0xcc,0x99,0x00},
	{0x99,0xff,0x99,0x00},{0xcc,0xff,0x99,0x00},{0xff,0xff,0x99,0x00},{0x00,0x00,0xcc,0x00},
	{0x33,0x00,0x99,0x00},{0x66,0x00,0xcc,0x00},{0x99,0x00,0xcc,0x00},{0xcc,0x00,0xcc,0x00},
	{0x00,0x33,0x99,0x00},{0x33,0x33,0xcc,0x00},{0x66,0x33,0xcc,0x00},{0x99,0x33,0xcc,0x00},
	{0xcc,0x33,0xcc,0x00},{0xff,0x33,0xcc,0x00},{0x00,0x66,0xcc,0x00},{0x33,0x66,0xcc,0x00},
	{0x66,0x66,0x99,0x00},{0x99,0x66,0xcc,0x00},{0xcc,0x66,0xcc,0x00},{0xff,0x66,0x99,0x00},
	{0x00,0x99,0xcc,0x00},{0x33,0x99,0xcc,0x00},{0x66,0x99,0xcc,0x00},{0x99,0x99,0xcc,0x00},
	{0xcc,0x99,0xcc,0x00},{0xff,0x99,0xcc,0x00},{0x00,0xcc,0xcc,0x00},{0x33,0xcc,0xcc,0x00},
	{0x66,0xcc,0xcc,0x00},{0x99,0xcc,0xcc,0x00},{0xcc,0xcc,0xcc,0x00},{0xff,0xcc,0xcc,0x00},
	{0x00,0xff,0xcc,0x00},{0x33,0xff,0xcc,0x00},{0x66,0xff,0x99,0x00},{0x99,0xff,0xcc,0x00},
	{0xcc,0xff,0xcc,0x00},{0xff,0xff,0xcc,0x00},{0x33,0x00,0xcc,0x00},{0x66,0x00,0xff,0x00},
	{0x99,0x00,0xff,0x00},{0x00,0x33,0xcc,0x00},{0x33,0x33,0xff,0x00},{0x66,0x33,0xff,0x00},
	{0x99,0x33,0xff,0x00},{0xcc,0x33,0xff,0x00},{0xff,0x33,0xff,0x00},{0x00,0x66,0xff,0x00},
	{0x33,0x66,0xff,0x00},{0x66,0x66,0xcc,0x00},{0x99,0x66,0xff,0x00},{0xcc,0x66,0xff,0x00},
	{0xff,0x66,0xcc,0x00},{0x00,0x99,0xff,0x00},{0x33,0x99,0xff,0x00},{0x66,0x99,0xff,0x00},
	{0x99,0x99,0xff,0x00},{0xcc,0x99,0xff,0x00},{0xff,0x99,0xff,0x00},{0x00,0xcc,0xff,0x00},
	{0x33,0xcc,0xff,0x00},{0x66,0xcc,0xff,0x00},{0x99,0xcc,0xff,0x00},{0xcc,0xcc,0xff,0x00},
	{0xff,0xcc,0xff,0x00},{0x33,0xff,0xff,0x00},{0x66,0xff,0xcc,0x00},{0x99,0xff,0xff,0x00},
	{0xcc,0xff,0xff,0x00},{0xff,0x66,0x66,0x00},{0x66,0xff,0x66,0x00},{0xff,0xff,0x66,0x00},
	{0x66,0x66,0xff,0x00},{0xff,0x66,0xff,0x00},{0x66,0xff,0xff,0x00},{0xa5,0x00,0x21,0x00},
	{0x5f,0x5f,0x5f,0x00},{0x77,0x77,0x77,0x00},{0x86,0x86,0x86,0x00},{0x96,0x96,0x96,0x00},
	{0xcb,0xcb,0xcb,0x00},{0xb2,0xb2,0xb2,0x00},{0xd7,0xd7,0xd7,0x00},{0xdd,0xdd,0xdd,0x00},
	{0xe3,0xe3,0xe3,0x00},{0xea,0xea,0xea,0x00},{0xff,0xfb,0xf0,0x00},{0xa0,0xa0,0xa4,0x00},
	{0x80,0x80,0x80,0x00},{0xff,0x00,0x00,0x00},{0x00,0xff,0x00,0x00},{0xff,0xff,0x00,0x00},
	{0x00,0x00,0xff,0x00},{0xff,0x00,0xff,0x00},{0x00,0xff,0xff,0x00},{0xff,0xff,0xff,0x00}
};

HPALETTE WINAPI extGDICreatePalette(CONST LOGPALETTE *plpal)
{
	HPALETTE ret;

	OutTraceDW("GDI.CreatePalette: plpal=%x version=%x NumEntries=%d\n", plpal, plpal->palVersion, plpal->palNumEntries);
	if(IsDebug) dxw.DumpPalette(plpal->palNumEntries, (LPPALETTEENTRY)plpal->palPalEntry);
	ret=(*pGDICreatePalette)(plpal);
	OutTraceDW("GDI.CreatePalette: hPalette=%x\n", ret);
	return ret;
}

HPALETTE hDesktopPalette=NULL;

HPALETTE WINAPI extSelectPalette(HDC hdc, HPALETTE hpal, BOOL bForceBackground)
{
	HPALETTE ret;

	OutTraceDW("GDI.SelectPalette: hdc=%x hpal=%x ForceBackground=%x\n", hdc, hpal, bForceBackground);
	if(hdc==dxw.RealHDC) hdc= dxw.VirtualHDC;

	if((dxw.dwFlags1 & EMULATESURFACE)  && (dxw.dwFlags6 & SYNCPALETTE) && bFlippedDC){
		hDesktopPalette=hpal;
		if(hFlippedDC){
			hdc = hFlippedDC;
			ret=(*pGDISelectPalette)(hdc, hpal, bForceBackground);
		}
		else{
			LPDIRECTDRAWSURFACE lpDDSPrim;
			lpDDSPrim = dxwss.GetPrimarySurface();
			(*pGetDC)(lpDDSPrim, &hdc);
			ret=(*pGDISelectPalette)(hdc, hpal, bForceBackground);
			(*pReleaseDC)(lpDDSPrim, hdc);
		}
	}
	else{
		ret=(*pGDISelectPalette)(hdc, hpal, bForceBackground);
		OutTraceDW("GDI.SelectPalette: ret=%x\n", ret);
	}

	return ret;
}

BOOL WINAPI extAnimatePalette(HPALETTE hpal, UINT iStartIndex, UINT cEntries, const PALETTEENTRY *ppe)
{
	// Invoked by "Pharaoh's Ascent 1.4"
	// STOPPER("AnimatePalette");
	return TRUE;
}

UINT WINAPI extRealizePalette(HDC hdc) 
{
	UINT ret;

	OutTraceDW("GDI.RealizePalette: hdc=%x\n", hdc);

	if((dxw.dwFlags1 & EMULATESURFACE) && (dxw.dwFlags6 & SYNCPALETTE)){
		PALETTEENTRY PalEntries[256];
		UINT nEntries;
		if(bFlippedDC) hdc = hFlippedDC;
		ret=(*pGDIRealizePalette)(hdc);
		OutTraceDW("GDI.RealizePalette: RealizePalette on hdc=%x ret=%d\n", hdc, ret);
		nEntries=(*pGetPaletteEntries)(hDesktopPalette, 0, 256, PalEntries);
		OutTraceDW("GDI.RealizePalette: GetPaletteEntries on hdc=%x ret=%d\n", hdc, nEntries);
		mySetPalette(0, nEntries, PalEntries); 
		if(IsDebug && nEntries) dxw.DumpPalette(nEntries, PalEntries);  
		ret=nEntries;

		HRESULT res;
		extern LPDIRECTDRAWPALETTE lpDDP;
		extern SetEntries_Type pSetEntries;
		if(lpDDP && pSetEntries) res=(*pSetEntries)(lpDDP, 0, 0, 256, PalEntries);
	}
	else
		ret=(*pGDIRealizePalette)(hdc);

	OutTraceDW("GDI.RealizePalette: hdc=%x nEntries=%d\n", hdc, ret);
	return ret;
}

UINT WINAPI extGetSystemPaletteEntries(HDC hdc, UINT iStartIndex, UINT nEntries, LPPALETTEENTRY lppe)
{
	int ret;

	OutTraceDW("GetSystemPaletteEntries: hdc=%x start=%d num=%d\n", hdc, iStartIndex, nEntries);
	ret=(*pGDIGetSystemPaletteEntries)(hdc, iStartIndex, nEntries, lppe);
	OutTraceDW("GetSystemPaletteEntries: ret=%d\n", ret);
	if((ret == 0) && (dxw.dwFlags1 & EMULATESURFACE) && (dxw.dwFlags6 & SYNCPALETTE)) {
		// use static default data...
		for(UINT idx=0; idx<nEntries; idx++) lppe[idx]=DefaultSystemPalette[iStartIndex+idx]; 
		ret = nEntries;
		OutTraceDW("GetSystemPaletteEntries: FIXED ret=%d\n", ret);
	}
	if(IsDebug) dxw.DumpPalette(nEntries, &lppe[iStartIndex]);
	return ret;
}

UINT WINAPI extSetSystemPaletteUse(HDC hdc, UINT uUsage)
{
	//BOOL res;
	OutTraceDW("GDI.SetSystemPaletteUse: hdc=%x Usage=%x(%s)\n", hdc, uUsage, ExplainPaletteUse(uUsage));
	return SYSPAL_NOSTATIC256;
}

UINT WINAPI extGetPaletteEntries(HPALETTE hpal, UINT iStartIndex, UINT nEntries, LPPALETTEENTRY lppe)
{
	UINT res;
	OutTraceDW("GDI.GetPaletteEntries: hpal=%x iStartIndex=%d nEntries=%d\n", hpal, iStartIndex, nEntries);
	res=(*pGetPaletteEntries)(hpal, iStartIndex, nEntries, lppe);
	OutTraceDW("GDI.GetPaletteEntries: res-nEntries=%d\n", res);
	if((res < nEntries) && (dxw.dwFlags6 & SYNCPALETTE)) { 
		res = nEntries;
		OutTraceDW("GDI.GetPaletteEntries: faking missing entries=%d\n", res);
	}
	if(IsDebug && res) dxw.DumpPalette(res, &lppe[iStartIndex]);
	//mySetPalette(0, nEntries, lppe); 
	return res;
}

UINT WINAPI extGetSystemPaletteUse(HDC hdc)
{
	UINT res;
	OutTraceDW("GDI.GetSystemPaletteUse: hdc=%x\n", hdc);
	res=(*pGetSystemPaletteUse)(hdc);

	if((res == SYSPAL_ERROR) && (dxw.dwFlags6 & SYNCPALETTE)) res = SYSPAL_NOSTATIC;

	OutTraceDW("GetSystemPaletteUse: res=%x(%s)\n", res, ExplainPaletteUse(res));
	return res;
}

HDC WINAPI extGDICreateDCA(LPSTR lpszDriver, LPSTR lpszDevice, LPSTR lpszOutput, CONST DEVMODE *lpdvmInit)
{
	HDC WinHDC, RetHDC;
	OutTraceDW("GDI.CreateDCA: Driver=%s Device=%s Output=%s InitData=%x\n", 
		lpszDriver?lpszDriver:"(NULL)", lpszDevice?lpszDevice:"(NULL)", lpszOutput?lpszOutput:"(NULL)", lpdvmInit);

	if (!lpszDriver || !strncmp(lpszDriver,"DISPLAY",7)) {
		if(dxw.GDIEmulationMode == GDIMODE_EMULATED){
			RetHDC=dxw.AcquireEmulatedDC(dxw.GethWnd());
		}
		else {
			OutTraceDW("GDI.CreateDCA: returning window surface DC\n");
			WinHDC=(*pGDIGetDC)(dxw.GethWnd());
			RetHDC=(*pGDICreateCompatibleDC)(WinHDC);
			(*pGDIReleaseDC)(dxw.GethWnd(), WinHDC);
		}

	}
	else{
		RetHDC=(*pGDICreateDCA)(lpszDriver, lpszDevice, lpszOutput, lpdvmInit);
	}
	if(RetHDC)
		OutTraceDW("GDI.CreateDCA: returning HDC=%x\n", RetHDC);
	else
		OutTraceE("GDI.CreateDCA ERROR: err=%d at %d\n", GetLastError(), __LINE__);
	return RetHDC;
}

HDC WINAPI extGDICreateDCW(LPWSTR lpszDriver, LPWSTR lpszDevice, LPWSTR lpszOutput, CONST DEVMODE *lpdvmInit)
{
	HDC WinHDC, RetHDC;
	OutTraceDW("GDI.CreateDCW: Driver=%ls Device=%ls Output=%ls InitData=%x\n", 
		lpszDriver?lpszDriver:L"(NULL)", lpszDevice?lpszDevice:L"(NULL)", lpszOutput?lpszOutput:L"(NULL)", lpdvmInit);

	if (!lpszDriver || !wcsncmp(lpszDriver,L"DISPLAY",7)) {
		if(dxw.GDIEmulationMode == GDIMODE_EMULATED){
			RetHDC=dxw.AcquireEmulatedDC(dxw.GethWnd());
		}
		else {
			OutTraceDW("GDI.CreateDCW: returning window surface DC\n");
			WinHDC=(*pGDIGetDC)(dxw.GethWnd());
			RetHDC=(*pGDICreateCompatibleDC)(WinHDC);
			(*pGDIReleaseDC)(dxw.GethWnd(), WinHDC);
		}

	}
	else{
		RetHDC=(*pGDICreateDCW)(lpszDriver, lpszDevice, lpszOutput, lpdvmInit);
	}
	if(RetHDC)
		OutTraceDW("GDI.CreateDCW: returning HDC=%x\n", RetHDC);
	else
		OutTraceE("GDI.CreateDCW ERROR: err=%d at %d\n", GetLastError(), __LINE__);
	return RetHDC;
}

HDC WINAPI extCreateICA(LPCTSTR lpszDriver, LPCTSTR lpszDevice, LPCTSTR lpszOutput, const DEVMODE *lpdvmInit)
{
	HDC WinHDC, RetHDC;
	OutTraceDW("GDI.CreateIC: Driver=%s Device=%s Output=%s InitData=%x\n", 
		lpszDriver?lpszDriver:"(NULL)", lpszDevice?lpszDevice:"(NULL)", lpszOutput?lpszOutput:"(NULL)", lpdvmInit);

	// EverQuest Tutorial.exe calls CreateICA passing "Tutorial"
	if(!lpszDriver || !_stricmp("DISPLAY", lpszDriver)){
		if(dxw.GDIEmulationMode == GDIMODE_EMULATED){
			RetHDC=dxw.AcquireEmulatedDC(dxw.GethWnd());
		}
		else {
			OutTraceDW("CreateIC: returning window surface DC\n");
			WinHDC = (*pGDIGetDC)(dxw.GethWnd());
			RetHDC = (*pGDICreateCompatibleDC)(WinHDC);
			(*pGDIReleaseDC)(dxw.GethWnd(), WinHDC);
		}
	}
	else{
		// proxy
		RetHDC = (*pCreateICA)(lpszDriver, lpszDevice, lpszOutput, lpdvmInit);
	}

	OutTraceDW("CreateIC: ret=%x\n", RetHDC);
	return RetHDC;
}

HDC WINAPI extGDICreateCompatibleDC(HDC hdc)
{
	HDC RetHdc;
	DWORD LastError;

	OutTraceDW("GDI.CreateCompatibleDC: hdc=%x\n", hdc);
	if(hdc==0){
		hdc=(*pGDIGetDC)(dxw.GethWnd());
		if(dxw.dwFlags6 & CREATEDESKTOP){
			extern HWND hDesktopWindow;
			hdc=(*pGDIGetDC)(hDesktopWindow);
		}
		OutTraceDW("GDI.CreateCompatibleDC: duplicating win HDC hWnd=%x\n", dxw.GethWnd()); 
	}

	// eliminated error message for errorcode 0.
	SetLastError(0);
	RetHdc=(*pGDICreateCompatibleDC)(hdc);
	LastError=GetLastError();
	if(LastError == 0){
		OutTraceDW("GDI.CreateCompatibleDC: returning HDC=%x\n", RetHdc);
	}
	else{
		OutTraceE("GDI.CreateCompatibleDC ERROR: err=%d at %d\n", LastError, __LINE__);
	}
	return RetHdc;
}

/*----------------------------------------*/
HBITMAP VirtualPic;

static HDC FillVirtualDC(HDC hdc)
{
	HDC VirtualHDC;
	HBITMAP VirtualPic;

	if(!(VirtualHDC=CreateCompatibleDC(hdc)))
		OutTraceE("CreateCompatibleDC: ERROR err=%d at=%d\n", GetLastError(), __LINE__);

	if(!(VirtualPic=CreateCompatibleBitmap(hdc, dxw.GetScreenWidth(), dxw.GetScreenHeight())))
		OutTraceE("dxwSDC::GetPrimaryDC: CreateCompatibleBitmap ERROR err=%d at=%d\n", GetLastError(), __LINE__);

	if(!SelectObject(VirtualHDC, VirtualPic))
		OutTraceE("dxwSDC::GetPrimaryDC: SelectObject ERROR err=%d at=%d\n", GetLastError(), __LINE__);

	if(!(*pGDIBitBlt)(VirtualHDC, 0, 0, dxw.GetScreenWidth(), dxw.GetScreenHeight(), hdc, 0, 0, SRCCOPY))
		OutTraceE("dxwSDC::GetPrimaryDC: StretchBlt ERROR err=%d at=%d\n", GetLastError(), __LINE__);

	return VirtualHDC;
}

static void FlushVirtualDC(HDC VirtualHDC)
{
	DeleteObject(VirtualHDC);
	DeleteObject(VirtualPic);
}

/*-------------------------------------------*/

BOOL WINAPI extGDIBitBlt(HDC hdcDest, int nXDest, int nYDest, int nWidth, int nHeight, HDC hdcSrc, int nXSrc, int nYSrc, DWORD dwRop)
{
	BOOL res;
	BOOL IsToScreen;
	BOOL IsFromScreen;
	BOOL IsDCLeakageSrc = FALSE;
	BOOL IsDCLeakageDest = FALSE;
	int Flux;

	OutTraceDW("GDI.BitBlt: HDC=%x(type=%s) nXDest=%d nYDest=%d nWidth=%d nHeight=%d hdcSrc=%x(type=%s) nXSrc=%d nYSrc=%d dwRop=%x(%s)\n", 
		hdcDest, GetObjectTypeStr(hdcDest), nXDest, nYDest, nWidth, nHeight, 
		hdcSrc, GetObjectTypeStr(hdcSrc), nXSrc, nYSrc, dwRop, ExplainROP(dwRop));

	OutTraceB("GDI.BitBlt: DEBUG FullScreen=%x target hdctype=%x(%s) hwnd=%x\n", 
		dxw.IsFullScreen(), (*pGetObjectType)(hdcDest), ExplainDCType((*pGetObjectType)(hdcDest)), WindowFromDC(hdcDest));

	// beware: HDC could refer to screen DC that are written directly on screen, or memory DC that will be scaled to
	// the screen surface later on, on ReleaseDC or ddraw Blit / Flip operation. Scaling of rect coordinates is 
	// needed only in the first case, and must be avoided on the second, otherwise the image would be scaled twice!

	if(hdcDest == NULL){
		// happens in Reah, hdc is NULL despite the fact that BeginPaint returns a valid DC. Too bad, we recover here ...
		hdcDest = (*pGDIGetDC)(dxw.GethWnd());
		OutTraceB("GDI.StretchBlt: DEBUG hdc dest=NULL->%x\n", hdcDest);
		IsDCLeakageDest = TRUE;
	}
	if(hdcSrc == NULL){
		hdcSrc = (*pGDIGetDC)(dxw.GethWnd());
		OutTraceB("GDI.StretchBlt: DEBUG hdc src=NULL->%x\n", hdcSrc);
		IsDCLeakageSrc = TRUE;
	}

	IsToScreen=(OBJ_DC == (*pGetObjectType)(hdcDest));
	IsFromScreen=(OBJ_DC == (*pGetObjectType)(hdcSrc));
	Flux = (IsToScreen ? 1 : 0) + (IsFromScreen ? 2 : 0); 
	if (IsToScreen && (dxw.dwFlags3 & NOGDIBLT)) return TRUE;

	if(dxw.IsFullScreen()){
		//int nWSrc, nHSrc, 
		int nWDest, nHDest;
		switch(dxw.GDIEmulationMode){
			case GDIMODE_SHAREDDC:
				switch(Flux){
					case 0: // memory to memory
						res=(*pGDIBitBlt)(hdcDest, nXDest, nYDest, nWidth, nHeight, hdcSrc, nXSrc, nYSrc, dwRop);
						break;
					case 1: // memory to screen
					case 3: // screen to screen
						sdc.GetPrimaryDC(hdcDest);
						res=(*pGDIBitBlt)(sdc.GetHdc(), nXDest, nYDest, nWidth, nHeight, hdcSrc, nXSrc, nYSrc, dwRop);
						sdc.PutPrimaryDC(hdcDest, TRUE, nXDest, nYDest, nWidth, nHeight);
						break;
					case 2: // screen to memory
						sdc.GetPrimaryDC(hdcSrc);
						res=(*pGDIBitBlt)(hdcDest, nXDest, nYDest, nWidth, nHeight, sdc.GetHdc(), nXSrc, nYSrc, dwRop);
						sdc.PutPrimaryDC(hdcSrc, FALSE);
						break;
				}
				break;
			case GDIMODE_STRETCHED: 
				nWDest= nWidth;
				nHDest= nHeight;
				switch(Flux){
					case 1: // memory to screen
						dxw.MapClient(&nXDest, &nYDest, &nWDest, &nHDest);
						break;
					case 2: // screen to memory
						dxw.MapClient(&nXSrc, &nYSrc, &nWidth, &nHeight);
						break;
					default:
						break;
				}
				res=(*pGDIStretchBlt)(hdcDest, nXDest, nYDest, nWDest, nHDest, hdcSrc, nXSrc, nYSrc, nWidth, nHeight, dwRop);
				OutTraceB("GDI.BitBlt: DEBUG DC dest=(%d,%d) size=(%d,%d)\n", nXDest, nYDest, nWDest, nHDest);
				break;
			case GDIMODE_EMULATED:
				if (hdcDest==dxw.RealHDC) hdcDest=dxw.VirtualHDC;
			    res=(*pGDIBitBlt)(hdcDest, nXDest, nYDest, nWidth, nHeight, hdcSrc, nXSrc, nYSrc, dwRop);
				OutTraceB("GDI.BitBlt: DEBUG emulated hdc dest=%x->%x\n", dxw.RealHDC, hdcDest);
				break;
			default:
				res=(*pGDIBitBlt)(hdcDest, nXDest, nYDest, nWidth, nHeight, hdcSrc, nXSrc, nYSrc, dwRop);
				break;
		}
	}
	else {
		res=(*pGDIBitBlt)(hdcDest, nXDest, nYDest, nWidth, nHeight, hdcSrc, nXSrc, nYSrc, dwRop);
	}

	if(IsDCLeakageSrc) (*pGDIReleaseDC)(dxw.GethWnd(), hdcSrc);
	if(IsDCLeakageDest) (*pGDIReleaseDC)(dxw.GethWnd(), hdcDest);
	if(res && IsToScreen) dxw.ShowOverlay(hdcDest);
	if(!res) OutTraceE("GDI.BitBlt: ERROR err=%d at %d\n", GetLastError(), __LINE__);
	return res;
}

BOOL WINAPI extGDIStretchBlt(HDC hdcDest, int nXDest, int nYDest, int nWidth, int nHeight, 
							 HDC hdcSrc, int nXSrc, int nYSrc, int nWSrc, int nHSrc, DWORD dwRop)
{
	BOOL res;
	BOOL IsToScreen;
	BOOL IsFromScreen;
	BOOL IsDCLeakageSrc = FALSE;
	BOOL IsDCLeakageDest = FALSE;
	int Flux;

	OutTraceDW("GDI.StretchBlt: HDC=%x nXDest=%d nYDest=%d nWidth=%d nHeight=%d hdcSrc=%x nXSrc=%d nYSrc=%d nWSrc=%d nHSrc=%d dwRop=%x(%s)\n", 
		hdcDest, nXDest, nYDest, nWidth, nHeight, hdcSrc, nXSrc, nYSrc, nWSrc, nHSrc, dwRop, ExplainROP(dwRop));

	OutTraceB("GDI.StretchBlt: DEBUG FullScreen=%x target hdctype=%x(%s) hwnd=%x\n", 
		dxw.IsFullScreen(), (*pGetObjectType)(hdcDest), ExplainDCType((*pGetObjectType)(hdcDest)), WindowFromDC(hdcDest));

	if(dxw.GDIEmulationMode == GDIMODE_EMULATED){
		if (hdcDest==dxw.RealHDC) hdcDest=dxw.VirtualHDC;
		OutTraceB("GDI.StretchBlt: DEBUG emulated hdc dest=%x->%x\n", dxw.RealHDC, hdcDest);
	}

	if(hdcDest == NULL){
		// happens in Reah, hdc is NULL despite the fact that BeginPaint returns a valid DC. Too bad, we recover here ...
		hdcDest = (*pGDIGetDC)(dxw.GethWnd());
		OutTraceB("GDI.StretchBlt: DEBUG hdc dest=NULL->%x\n", hdcDest);
		IsDCLeakageDest = TRUE;
	}
	if(hdcSrc == NULL){
		hdcSrc = (*pGDIGetDC)(dxw.GethWnd());
		OutTraceB("GDI.StretchBlt: DEBUG hdc src=NULL->%x\n", hdcSrc);
		IsDCLeakageSrc = TRUE;
	}

	IsToScreen=(OBJ_DC == (*pGetObjectType)(hdcDest));
	IsFromScreen=(OBJ_DC == (*pGetObjectType)(hdcSrc));
	Flux = (IsToScreen ? 1 : 0) + (IsFromScreen ? 2 : 0); 
	if (IsToScreen && (dxw.dwFlags3 & NOGDIBLT)) return TRUE;

	if(dxw.IsToRemap(hdcDest) && (hdcDest != hdcSrc)){
		switch(dxw.GDIEmulationMode){
			case GDIMODE_SHAREDDC:
				switch(Flux){
					case 0: // memory to memory
						res=(*pGDIStretchBlt)(hdcDest, nXDest, nYDest, nWidth, nHeight, hdcSrc, nXSrc, nYSrc, nWSrc, nHSrc, dwRop);
						break;
					case 1: // memory to screen
					case 3: // screen to screen
						sdc.GetPrimaryDC(hdcDest);
						res=(*pGDIStretchBlt)(sdc.GetHdc(), nXDest, nYDest, nWidth, nHeight, hdcSrc, nXSrc, nYSrc, nWSrc, nHSrc, dwRop);
						sdc.PutPrimaryDC(hdcDest, TRUE, nXDest, nYDest, nWidth, nHeight);
						break;
					case 2: // screen to memory using virtual screen
						sdc.GetPrimaryDC(hdcSrc);
						res=(*pGDIStretchBlt)(hdcDest, nXDest, nYDest, nWidth, nHeight, sdc.GetHdc(), nXSrc, nYSrc, nWSrc, nHSrc, dwRop);
						sdc.PutPrimaryDC(hdcSrc, FALSE, nXSrc, nYSrc, nWSrc, nHSrc);
						break;
				}
				break;
			case GDIMODE_STRETCHED: {
				int nWDest, nHDest;
				nWDest= nWidth;
				nHDest= nHeight;
				switch(Flux){
					case 1: // memory to screen
						dxw.MapClient(&nXDest, &nYDest, &nWDest, &nHDest);
						break;
					case 2: // screen to memory
						dxw.MapClient(&nXSrc, &nYSrc, &nWidth, &nHeight);
						break;
					default:
						break;
				}
				res=(*pGDIStretchBlt)(hdcDest, nXDest, nYDest, nWDest, nHDest, hdcSrc, nXSrc, nYSrc, nWSrc, nHSrc, dwRop);
				OutTraceB("GDI.StretchBlt: DEBUG DC dest=(%d,%d) size=(%d,%d)\n", nXDest, nYDest, nWDest, nHDest);
				}
				break;
			case GDIMODE_EMULATED:
			default:
				res=(*pGDIStretchBlt)(hdcDest, nXDest, nYDest, nWidth, nHeight, hdcSrc, nXSrc, nYSrc, nWSrc, nHSrc, dwRop);
				break;
		}
	}
	else {
		res=(*pGDIStretchBlt)(hdcDest, nXDest, nYDest, nWidth, nHeight, hdcSrc, nXSrc, nYSrc, nWSrc, nHSrc, dwRop);
	}

	if(IsDCLeakageSrc) (*pGDIReleaseDC)(dxw.GethWnd(), hdcSrc);
	if(IsDCLeakageDest) (*pGDIReleaseDC)(dxw.GethWnd(), hdcDest);
	if(res && IsToScreen) dxw.ShowOverlay(hdcDest);
	if(!res) OutTraceE("GDI.StretchBlt: ERROR err=%d at %d\n", GetLastError(), __LINE__);
	return res;
}

BOOL WINAPI extGDIPatBlt(HDC hdcDest, int nXDest, int nYDest, int nWidth, int nHeight, DWORD dwRop)
{
	BOOL res;
	BOOL IsToScreen;
	BOOL IsDCLeakageDest = FALSE;

	OutTraceDW("GDI.PatBlt: HDC=%x nXDest=%d nYDest=%d nWidth=%d nHeight=%d dwRop=%x(%s)\n", 
		hdcDest, nXDest, nYDest, nWidth, nHeight, dwRop, ExplainROP(dwRop));

	OutTraceB("GDI.PatBlt: DEBUG FullScreen=%x target hdctype=%x(%s) hwnd=%x\n", 
		dxw.IsFullScreen(), (*pGetObjectType)(hdcDest), ExplainDCType((*pGetObjectType)(hdcDest)), WindowFromDC(hdcDest));

	if(dxw.GDIEmulationMode == GDIMODE_EMULATED){
		if (hdcDest==dxw.RealHDC) hdcDest=dxw.VirtualHDC;
		OutTraceB("GDI.PatBlt: DEBUG emulated hdc dest=%x->%x\n", dxw.RealHDC, hdcDest);
	}

	if(hdcDest == NULL){
		// happens in Reah, hdc is NULL despite the fact that BeginPaint returns a valid DC. Too bad, we recover here ...
		hdcDest = (*pGDIGetDC)(dxw.GethWnd());
		OutTraceB("GDI.PatBlt: DEBUG hdc dest=NULL->%x\n", hdcDest);
		IsDCLeakageDest = TRUE;
	}

	IsToScreen=(OBJ_DC == (*pGetObjectType)(hdcDest));

	if (IsToScreen && (dxw.dwFlags3 & NOGDIBLT)) return TRUE;

	if(dxw.IsToRemap(hdcDest)) {

		switch(dxw.GDIEmulationMode){
			case GDIMODE_SHAREDDC:
				sdc.GetPrimaryDC(hdcDest);
				res=(*pGDIPatBlt)(sdc.GetHdc(),  nXDest, nYDest, nWidth, nHeight, dwRop);
				sdc.PutPrimaryDC(hdcDest, TRUE, nXDest, nYDest, nWidth, nHeight);
				return res;
				break;
			case GDIMODE_STRETCHED: {
				int nWDest, nHDest;
				nWDest= nWidth;
				nHDest= nHeight;
				dxw.MapClient(&nXDest, &nYDest, &nWDest, &nHDest);
				res=(*pGDIPatBlt)(hdcDest, nXDest, nYDest, nWDest, nHDest, dwRop);
				OutTraceB("GDI.PatBlt: DEBUG DC dest=(%d,%d) size=(%d,%d)\n", nXDest, nYDest, nWDest, nHDest);
				}
				break;
			case GDIMODE_EMULATED:
			default:
				res=(*pGDIPatBlt)(hdcDest,  nXDest, nYDest, nWidth, nHeight, dwRop);
				break;
		}
	}
	else {
		res=(*pGDIPatBlt)(hdcDest,  nXDest, nYDest, nWidth, nHeight, dwRop);
	}

	if(IsDCLeakageDest) (*pGDIReleaseDC)(dxw.GethWnd(), hdcDest);
	if(res && IsToScreen) dxw.ShowOverlay(hdcDest);
	if(!res) OutTraceE("GDI.PatBlt: ERROR err=%d at %d\n", GetLastError(), __LINE__);
	return res;
}

BOOL WINAPI extGDIDeleteDC(HDC hdc)
{
	BOOL res;
	OutTraceDW("GDI.DeleteDC: hdc=%x\n", hdc);
	res=(*pGDIDeleteDC)(hdc);
	if(!res) OutTraceE("GDI.DeleteDC: ERROR err=%d at %d\n", GetLastError(), __LINE__);
	return res;
}

HFONT WINAPI extCreateFont(int nHeight, int nWidth, int nEscapement, int nOrientation, int fnWeight,
				 DWORD fdwItalic, DWORD fdwUnderline, DWORD fdwStrikeOut, DWORD fdwCharSet,
				 DWORD fdwOutputPrecision, DWORD fdwClipPrecision, DWORD fdwQuality,
				 DWORD fdwPitchAndFamily, LPCTSTR lpszFace)
{
	OutTraceDW("CreateFont: h=%d w=%d face=\"%s\"\n", nHeight, nWidth, lpszFace);
	if(dxw.dwFlags1 & FIXTEXTOUT) {
		if(nHeight > 0) dxw.MapClient(&nWidth, &nHeight);
		else {
			nHeight= -nHeight;
			dxw.MapClient(&nWidth, &nHeight);
			nHeight= -nHeight;
		}
	}
	return (*pGDICreateFont)(nHeight, nWidth, nEscapement, nOrientation, fnWeight,
				 fdwItalic, fdwUnderline, fdwStrikeOut, fdwCharSet,
				 fdwOutputPrecision, fdwClipPrecision, NONANTIALIASED_QUALITY,
				 fdwPitchAndFamily, lpszFace);
}

// CreateFontIndirect hook routine to avoid font aliasing that prevents reverse blitting working on palettized surfaces

HFONT WINAPI extCreateFontIndirect(const LOGFONT* lplf)
{
	LOGFONT lf;
	HFONT retHFont;
	OutTraceDW("CreateFontIndirect: h=%d w=%d face=\"%s\"\n", lplf->lfHeight, lplf->lfWidth, lplf->lfFaceName);
	memcpy((char *)&lf, (char *)lplf, sizeof(LOGFONT));
	lf.lfQuality=NONANTIALIASED_QUALITY;
	if(dxw.dwFlags1 & FIXTEXTOUT) {
		if(lf.lfHeight > 0) dxw.MapClient((int *)&lf.lfWidth, (int *)&lf.lfHeight);
		else {
			lf.lfHeight= -lf.lfHeight;
			dxw.MapClient((int *)&lf.lfWidth, (int *)&lf.lfHeight);
			lf.lfHeight= -lf.lfHeight;
		}
	}
	retHFont=((*pGDICreateFontIndirect)(&lf));
	if(retHFont)
		OutTraceDW("CreateFontIndirect: hfont=%x\n", retHFont);
	else
		OutTraceDW("CreateFontIndirect: error=%d at %d\n", GetLastError(), __LINE__);
	return retHFont;
}

BOOL WINAPI extSetDeviceGammaRamp(HDC hDC, LPVOID lpRamp)
{
	BOOL ret;
	OutTraceDW("SetDeviceGammaRamp: hdc=%x\n", hDC);
	if(dxw.dwFlags2 & DISABLEGAMMARAMP) {
		OutTraceDW("SetDeviceGammaRamp: SUPPRESS\n");
		return TRUE;
	}
	ret=(*pGDISetDeviceGammaRamp)(hDC, lpRamp);
	if(!ret) OutTraceE("SetDeviceGammaRamp: ERROR err=%d\n", GetLastError());
	return ret;
}

BOOL WINAPI extGetDeviceGammaRamp(HDC hDC, LPVOID lpRamp)
{
	BOOL ret;
	OutTraceDW("GetDeviceGammaRamp: hdc=%x\n", hDC);
	ret=(*pGDIGetDeviceGammaRamp)(hDC, lpRamp);
	if(!ret) OutTraceE("GetDeviceGammaRamp: ERROR err=%d\n", GetLastError());
	return ret;
}

int WINAPI extGetClipBox(HDC hdc, LPRECT lprc)
{
	// v2.02.31: needed in "Imperialism II" to avoid blit clipping
	int ret;
	OutTraceDW("GetClipBox: hdc=%x\n", hdc);

	if (dxw.IsToRemap(hdc)){
		switch(dxw.GDIEmulationMode){
			case GDIMODE_SHAREDDC:
				sdc.GetPrimaryDC(hdc);
				ret=(*pGDIGetClipBox)(sdc.GetHdc(), lprc);
				sdc.PutPrimaryDC(hdc, FALSE);
				return ret;
				break;
			case GDIMODE_STRETCHED: 
				ret=(*pGDIGetClipBox)(hdc, lprc);
				OutTraceDW("GetClipBox: scaling main win coordinates (%d,%d)-(%d,%d)\n",
					lprc->left, lprc->top, lprc->right, lprc->bottom);
				dxw.UnmapClient(lprc);
				break;
			default:
				ret=(*pGDIGetClipBox)(hdc, lprc);
				break;
		}
	}
	else 
		ret=(*pGDIGetClipBox)(hdc, lprc);

	OutTraceDW("GetClipBox: ret=%x(%s) rect=(%d,%d)-(%d,%d)\n", 
		ret, ExplainRegionType(ret), lprc->left, lprc->top, lprc->right, lprc->bottom);
	return ret;
}

int WINAPI extIntersectClipRect(HDC hdc, int nLeftRect, int nTopRect, int nRightRect, int nBottomRect)
{
	int ret;
	OutTraceDW("IntersectClipRect: hdc=%x rect=(%d,%d)-(%d,%d)\n", hdc, nLeftRect, nTopRect, nRightRect, nBottomRect);

	if (dxw.IsToRemap(hdc)){
		switch(dxw.GDIEmulationMode){
			case GDIMODE_SHAREDDC:
				sdc.GetPrimaryDC(hdc);
				ret=(*pIntersectClipRect)(sdc.GetHdc(), nLeftRect, nTopRect, nRightRect, nBottomRect);
				sdc.PutPrimaryDC(hdc, FALSE);
				return ret;
				break;
			case GDIMODE_STRETCHED:
				dxw.MapClient(&nLeftRect, &nTopRect, &nRightRect, &nBottomRect);
				OutTraceDW("IntersectClipRect: fixed rect=(%d,%d)-(%d,%d)\n", nLeftRect, nTopRect, nRightRect, nBottomRect);
				break;
		}							
	}

	ret=(*pIntersectClipRect)(hdc, nLeftRect, nTopRect, nRightRect, nBottomRect);
	OutTraceE("IntersectClipRect: ret=%x(%s)\n", ret, ExplainRegionType(ret)); 
	return ret;
}

int WINAPI extGetRgnBox(HRGN hrgn, LPRECT lprc)
{
	int ret;
	OutTraceDW("GetRgnBox: hrgn=%x\n", hrgn);
	ret=(*pGetRgnBox)(hrgn, lprc);
	if (dxw.IsFullScreen() && (ret!=ERROR)){
		switch(dxw.GDIEmulationMode){
			case GDIMODE_STRETCHED:
			OutTraceDW("GetRgnBox: scaling main win coordinates (%d,%d)-(%d,%d)\n",
				lprc->left, lprc->top, lprc->right, lprc->bottom);
			dxw.UnmapClient(lprc);
			break;
		}
	}
	OutTraceDW("GetRgnBox: ret=%x(%s) rect=(%d,%d)-(%d,%d)\n", 
		ret, ExplainRegionType(ret), lprc->left, lprc->top, lprc->right, lprc->bottom);
	return ret;
}

BOOL WINAPI extPolyline(HDC hdc, const POINT *lppt, int cPoints)
{
	// LOGTOBEFIXED
	BOOL ret;
	if(IsTraceDDRAW){
		int i;
		OutTrace("Polyline: hdc=%x cPoints=%d pt=", hdc, cPoints); 
		for(i=0; i<cPoints; i++) OutTrace("(%d,%d) ", lppt[i].x, lppt[i].y);
		OutTrace("\n");
	}

	if (dxw.IsToRemap(hdc)){
		switch(dxw.GDIEmulationMode){
			case GDIMODE_SHAREDDC:
				sdc.GetPrimaryDC(hdc);
				ret=(*pPolyline)(sdc.GetHdc(), lppt, cPoints);
				sdc.PutPrimaryDC(hdc, TRUE);
				return ret;
				break;
			case GDIMODE_STRETCHED:
				int i;
				OutTrace("Polyline: fixed cPoints=%d pt=", cPoints); 
				for(i=0; i<cPoints; i++) {
					dxw.MapClient((LPPOINT)&lppt[i]);
					OutTrace("(%d,%d) ", lppt[i].x, lppt[i].y);
				}
				OutTrace("\n");
				break;
		}
	}
	ret=(*pPolyline)(hdc, lppt, cPoints);
	if(!ret)OutTraceE("Polyline: ERROR ret=%x\n", ret); 
	return ret;
}

BOOL WINAPI extLineTo(HDC hdc, int nXEnd, int nYEnd)
{
	BOOL ret;
	OutTraceDW("LineTo: hdc=%x pt=(%d,%d)\n", hdc, nXEnd, nYEnd);
	if (dxw.IsToRemap(hdc)){
		switch(dxw.GDIEmulationMode){
			case GDIMODE_SHAREDDC:
				sdc.GetPrimaryDC(hdc);
				ret=(*pLineTo)(sdc.GetHdc(), nXEnd, nYEnd);
				sdc.PutPrimaryDC(hdc, TRUE);
				return ret;
				break;
			case GDIMODE_STRETCHED: 
				dxw.MapClient(&nXEnd, &nYEnd);
				OutTraceDW("LineTo: fixed pt=(%d,%d)\n", nXEnd, nYEnd);
				break;
		}
	}
	ret=(*pLineTo)(hdc, nXEnd, nYEnd);
	if(!ret)OutTraceE("LineTo: ERROR ret=%x\n", ret); 
	return ret;
}

BOOL WINAPI extArcTo(HDC hdc, int nLeftRect, int nTopRect, int nRightRect, int nBottomRect, int nXRadial1, int nYRadial1, int nXRadial2, int nYRadial2)
{
	BOOL ret;
	OutTraceDW("ArcTo: hdc=%x rect=(%d,%d)(%d,%d) radial=(%d,%d)(%d,%d)\n", 
		hdc, nLeftRect, nTopRect, nRightRect, nBottomRect, nXRadial1, nYRadial1, nXRadial2, nYRadial2);
	if (dxw.IsToRemap(hdc)){
		switch(dxw.GDIEmulationMode){
			case GDIMODE_SHAREDDC:
				sdc.GetPrimaryDC(hdc);
				ret=(*pArcTo)(sdc.GetHdc(), nLeftRect, nTopRect, nRightRect, nBottomRect, nXRadial1, nYRadial1, nXRadial2, nYRadial2);
				sdc.PutPrimaryDC(hdc, TRUE);
				return ret;	
				break;
			case GDIMODE_STRETCHED: 
				dxw.MapClient(&nLeftRect, &nTopRect, &nRightRect, &nBottomRect);
				dxw.MapClient(&nXRadial1, &nYRadial1, &nXRadial2, &nYRadial2);
				OutTraceDW("ArcTo: fixed rect=(%d,%d)(%d,%d) radial=(%d,%d)(%d,%d)\n", 
					nLeftRect, nTopRect, nRightRect, nBottomRect, nXRadial1, nYRadial1, nXRadial2, nYRadial2);
				break;
		}
	}
	ret=(*pArcTo)(hdc, nLeftRect, nTopRect, nRightRect, nBottomRect, nXRadial1, nYRadial1, nXRadial2, nYRadial2);
	if(!ret)OutTraceE("ArcTo: ERROR ret=%x\n", ret); 
	return ret;
}

BOOL WINAPI extMoveToEx(HDC hdc, int X, int Y, LPPOINT lpPoint)
{
	BOOL ret;
	OutTraceDW("MoveToEx: hdc=%x pt=(%d,%d)\n", hdc, X, Y);
	if (dxw.IsToRemap(hdc)){
		switch(dxw.GDIEmulationMode){
			case GDIMODE_SHAREDDC:
				sdc.GetPrimaryDC(hdc);
				ret=(*pMoveToEx)(sdc.GetHdc(), X, Y, lpPoint);	
				sdc.PutPrimaryDC(hdc, FALSE);
				return ret; // no need to update the screen!
				break;
			case GDIMODE_STRETCHED: 
				dxw.MapClient(&X, &Y);
				OutTraceDW("MoveToEx: fixed pt=(%d,%d)\n", X, Y);
				break;
		}
	}
	ret=(*pMoveToEx)(hdc, X, Y, lpPoint);
	if(!ret)OutTraceE("MoveToEx: ERROR ret=%x\n", ret); 
	return ret;
}

BOOL WINAPI extPolyDraw(HDC hdc, const POINT *lpPoints, const BYTE *lpbTypes, int cCount)
{
	// LOGTOBEFIXED
	BOOL ret;
	if(IsTraceDDRAW){
		int i;
		OutTrace("PolyDraw: hdc=%x cCount=%d pt=", hdc, cCount); 
		for(i=0; i<cCount; i++) OutTrace("(%x:%d,%d) ", lpbTypes[i], lpPoints[i].x, lpPoints[i].y);
		OutTrace("\n");
	}

	if (dxw.IsToRemap(hdc)){
		int i, size;
		POINT *lpRemPoints;
		
		size = cCount * sizeof(POINT);
		lpRemPoints = (LPPOINT)malloc(size);
		memcpy(lpRemPoints, lpPoints, size);

		switch(dxw.GDIEmulationMode){
			case GDIMODE_SHAREDDC:
				free(lpRemPoints);
				sdc.GetPrimaryDC(hdc);
				ret=(*pPolyDraw)(sdc.GetHdc(), lpPoints, lpbTypes, cCount);
				sdc.PutPrimaryDC(hdc, TRUE);
				return ret;
				break;
			case GDIMODE_STRETCHED: 
				OutTrace("PolyDraw: fixed cCount=%d pt=", cCount); 
				for(i=0; i<cCount; i++) {
					dxw.MapClient(&lpRemPoints[i]);
					OutTrace("(%d,%d) ", &lpRemPoints[i].x, &lpRemPoints[i].y);
				}
				OutTrace("\n");
				break;
			case GDIMODE_EMULATED:
				if (dxw.IsVirtual(hdc)){
					OutTrace("PolyDraw: fixed cCount=%d pt=", cCount); 
					for(i=0; i<cCount; i++) {
						lpRemPoints[i].x += dxw.VirtualOffsetX;
						lpRemPoints[i].y += dxw.VirtualOffsetY;
						OutTrace("(%d,%d) ", &lpPoints[i].x, &lpPoints[i].y);
					}
					OutTrace("\n");
				}
				break;	
		}
		ret=(*pPolyDraw)(hdc, lpRemPoints, lpbTypes, cCount);
		free(lpRemPoints);
	}
	else {
		ret=(*pPolyDraw)(hdc, lpPoints, lpbTypes, cCount);
	}
	if(!ret)OutTraceE("pPolyDraw: ERROR ret=%x\n", ret); 
	return ret;
}

BOOL WINAPI extPolylineTo(HDC hdc, const POINT *lpPoints, DWORD cCount)
{
	// LOGTOBEFIXED
	BOOL ret;
	if(IsTraceDDRAW){
		DWORD i;
		OutTrace("PolylineTo: hdc=%x cCount=%d pt=", hdc, cCount); 
		for(i=0; i<cCount; i++) OutTrace("(%d,%d) ", lpPoints[i].x, lpPoints[i].y);
		OutTrace("\n");
	}

	if (dxw.IsToRemap(hdc)){

		DWORD i, size;
		POINT *lpRemPoints;
		
		size = cCount * sizeof(POINT);
		lpRemPoints = (LPPOINT)malloc(size);
		memcpy(lpRemPoints, lpPoints, size);

		switch(dxw.GDIEmulationMode){
			case GDIMODE_SHAREDDC:
				free(lpRemPoints);
				sdc.GetPrimaryDC(hdc);
				ret=(*pPolylineTo)(sdc.GetHdc(), lpPoints, cCount);
				sdc.PutPrimaryDC(hdc, TRUE);
				return ret;
				break;
			case GDIMODE_STRETCHED: 
				OutTrace("PolylineTo: fixed cCount=%d pt=", cCount); 
				for(i=0; i<cCount; i++) {
					dxw.MapClient(&lpRemPoints[i]);
					OutTrace("(%d,%d) ", &lpRemPoints[i].x, &lpRemPoints[i].y);
				}
				OutTrace("\n");
				break;
			case GDIMODE_EMULATED:
				if (dxw.IsVirtual(hdc)){
					OutTrace("PolylineTo: fixed cCount=%d pt=", cCount); 
					for(i=0; i<cCount; i++) {
						lpRemPoints[i].x += dxw.VirtualOffsetX;
						lpRemPoints[i].y += dxw.VirtualOffsetY;
						OutTrace("(%d,%d) ", &lpPoints[i].x, &lpPoints[i].y);
					}
					OutTrace("\n");
				}
				break;	
		}
		ret=(*pPolylineTo)(hdc, lpRemPoints, cCount);
		free(lpRemPoints);
	}
	else {
		ret=(*pPolylineTo)(hdc, lpPoints, cCount);
	}
	if(!ret)OutTraceE("PolylineTo: ERROR ret=%x\n", ret); 
	return ret;
}

BOOL WINAPI extPolyBezierTo(HDC hdc, const POINT *lpPoints, DWORD cCount)
{
	// LOGTOBEFIXED
	BOOL ret;
	if(IsTraceDDRAW){
		DWORD i;
		OutTrace("PolyBezierTo: hdc=%x cCount=%d pt=", hdc, cCount); 
		for(i=0; i<cCount; i++) OutTrace("(%d,%d) ", lpPoints[i].x, lpPoints[i].y);
		OutTrace("\n");
	}

	if (dxw.IsToRemap(hdc)){
		DWORD i, size;
		POINT *lpRemPoints;
		
		size = cCount * sizeof(POINT);
		lpRemPoints = (LPPOINT)malloc(size);
		memcpy(lpRemPoints, lpPoints, size);

		switch(dxw.GDIEmulationMode){
			case GDIMODE_SHAREDDC:
				free(lpRemPoints);
				sdc.GetPrimaryDC(hdc);
				ret=(*pPolyBezierTo)(sdc.GetHdc(), lpPoints, cCount);
				sdc.PutPrimaryDC(hdc, TRUE);
				return ret;
				break;
			case GDIMODE_STRETCHED: 
				OutTrace("PolyBezierTo: fixed cCount=%d pt=", cCount); 
				for(i=0; i<cCount; i++) {
					dxw.MapClient(&lpRemPoints[i]);
					OutTrace("(%d,%d) ", &lpRemPoints[i].x, &lpRemPoints[i].y);
				}
				OutTrace("\n");
				break;
			case GDIMODE_EMULATED:
				if (dxw.IsVirtual(hdc)){
					OutTrace("PolyBezierTo: fixed cCount=%d pt=", cCount); 
					for(i=0; i<cCount; i++) {
						lpRemPoints[i].x += dxw.VirtualOffsetX;
						lpRemPoints[i].y += dxw.VirtualOffsetY;
						OutTrace("(%d,%d) ", &lpPoints[i].x, &lpPoints[i].y);
					}
					OutTrace("\n");
				}
				break;	
		}
		ret=(*pPolyBezierTo)(hdc, lpRemPoints, cCount);
		free(lpRemPoints);
	}
	else {
		ret=(*pPolyBezierTo)(hdc, lpPoints, cCount);
	}
	if(!ret)OutTraceE("PolyBezierTo: ERROR ret=%x\n", ret); 
	return ret;
}

BOOL WINAPI extPolyBezier(HDC hdc, const POINT *lpPoints, DWORD cCount)
{
	// LOGTOBEFIXED
	BOOL ret;
	if(IsTraceDDRAW){
		DWORD i;
		OutTrace("PolyBezier: hdc=%x cCount=%d pt=", hdc, cCount); 
		for(i=0; i<cCount; i++) OutTrace("(%d,%d) ", lpPoints[i].x, lpPoints[i].y);
		OutTrace("\n");
	}

	if (dxw.IsToRemap(hdc)){
		DWORD i, size;
		POINT *lpRemPoints;
		
		size = cCount * sizeof(POINT);
		lpRemPoints = (LPPOINT)malloc(size);
		memcpy(lpRemPoints, lpPoints, size);

		switch(dxw.GDIEmulationMode){
			case GDIMODE_SHAREDDC:
				free(lpRemPoints);
				sdc.GetPrimaryDC(hdc);
				ret=(*pPolyBezier)(sdc.GetHdc(), lpPoints, cCount);
				sdc.PutPrimaryDC(hdc, TRUE);
				return ret;
				break;
			case GDIMODE_STRETCHED: 
				OutTrace("PolyBezier: fixed cCount=%d pt=", cCount); 
				for(i=0; i<cCount; i++) {
					dxw.MapClient(&lpRemPoints[i]);
					OutTrace("(%d,%d) ", &lpRemPoints[i].x, &lpRemPoints[i].y);
				}
				OutTrace("\n");
				break;
			case GDIMODE_EMULATED:
				if (dxw.IsVirtual(hdc)){
					OutTrace("PolyBezier: fixed cCount=%d pt=", cCount); 
					for(i=0; i<cCount; i++) {
						lpRemPoints[i].x += dxw.VirtualOffsetX;
						lpRemPoints[i].y += dxw.VirtualOffsetY;
						OutTrace("(%d,%d) ", &lpPoints[i].x, &lpPoints[i].y);
					}
					OutTrace("\n");
				}
				break;	
		}
		ret=(*pPolyBezier)(hdc, lpRemPoints, cCount);
		free(lpRemPoints);
	}
	else {
		ret=(*pPolyBezier)(hdc, lpPoints, cCount);
	}
	if(!ret)OutTraceE("PolyBezier: ERROR ret=%x\n", ret); 
	return ret;
}

int WINAPI extStretchDIBits(HDC hdc, int XDest, int YDest, int nDestWidth, int nDestHeight, int XSrc, int YSrc, int nSrcWidth, int nSrcHeight, 
				  const VOID *lpBits, const BITMAPINFO *lpBitsInfo, UINT iUsage, DWORD dwRop)
{
	int ret;
	OutTraceDW("StretchDIBits: hdc=%x dest=(%d,%d)-(%d,%d) src=(%d,%d)-(%d,%d) rop=%x(%s)\n", 
		hdc, XDest, YDest, nDestWidth, nDestHeight, XSrc, YSrc, nSrcWidth, nSrcHeight, dwRop, ExplainROP(dwRop));

	if(dxw.IsToRemap(hdc)){
		switch(dxw.GDIEmulationMode){
			case GDIMODE_SHAREDDC:
				sdc.GetPrimaryDC(hdc);
				ret=(*pStretchDIBits)(sdc.GetHdc(), XDest, YDest, nDestWidth, nDestHeight, XSrc, YSrc, nSrcWidth, nSrcHeight, lpBits, lpBitsInfo, iUsage, dwRop);
				sdc.PutPrimaryDC(hdc, TRUE, XDest, YDest, nDestWidth, nDestHeight);
				return ret;
				break;
			case GDIMODE_STRETCHED: 
				dxw.MapClient(&XDest, &YDest, &nDestWidth, &nDestHeight);
				OutTraceDW("StretchDIBits: fixed STRETCHED dest=(%d,%d)-(%d,%d)\n", XDest, YDest, nDestWidth, nDestHeight);
				break;
			case GDIMODE_EMULATED:
				if(dxw.IsVirtual(hdc)){
					XDest+=dxw.VirtualOffsetX;
					YDest+=dxw.VirtualOffsetY;
					OutTraceDW("StretchDIBits: fixed EMULATED dest=(%d,%d)-(%d,%d)\n", XDest, YDest, nDestWidth, nDestHeight);
				}
				break;
			default:
				break;
		}
	}

	ret=(*pStretchDIBits)(hdc, XDest, YDest, nDestWidth, nDestHeight, XSrc, YSrc, nSrcWidth, nSrcHeight, lpBits, lpBitsInfo, iUsage, dwRop);
	if(!ret || (ret==GDI_ERROR)) OutTraceE("StretchDIBits: ERROR ret=%x err=%d\n", ret, GetLastError()); 
	return ret;
}

int WINAPI extSetDIBits(HDC hdc, HBITMAP hbmp, UINT uStartScan, UINT cScanLines, const VOID *lpvBits, const BITMAPINFO *lpbmi, UINT fuColorUse)
{
	int ret;
	BITMAPINFOHEADER *bmi;
	OutTraceDW("SetDIBits: hdc=%x hbmp=%x lines=(%d,%d) ColorUse=%x\n", hdc, hbmp, uStartScan, cScanLines, fuColorUse);
	bmi=(BITMAPINFOHEADER *)&(lpbmi->bmiHeader);
	OutTraceDW("SetDIBits: BitmapInfo dim=(%dx%d) Planes=%d BPP=%d Compression=%x SizeImage=%x\n",
		bmi->biWidth, bmi->biHeight, bmi->biPlanes, bmi->biBitCount, bmi->biCompression, bmi->biSizeImage);

	if(dxw.IsToRemap(hdc) && !bGDIRecursionFlag){
		//HDC hTempDc;
		//HBITMAP hbmPic;
		switch(dxw.GDIEmulationMode){
			case GDIMODE_SHAREDDC: // this will flicker !!!!
				sdc.GetPrimaryDC(hdc);
				ret=(*pSetDIBits)(sdc.GetHdc(), hbmp, uStartScan, cScanLines, lpvBits, lpbmi, fuColorUse);
				sdc.PutPrimaryDC(hdc, TRUE, 0, 0, bmi->biWidth, bmi->biHeight);
				return ret;
				break;
			case GDIMODE_STRETCHED: 
#if 0
				// blitting to primary surface !!!
				int OrigXDest, OrigYDest;
				OrigWidth=dwWidth;
				OrigHeight=dwHeight;
				OrigXDest=XDest;
				OrigYDest=YDest;
				dxw.MapClient(&XDest, &YDest, (int *)&dwWidth, (int *)&dwHeight);
				OutTraceDW("SetDIBitsToDevice: fixed dest=(%d,%d)-(%dx%d)\n", XDest, YDest, dwWidth, dwHeight);
				if(!(hTempDc=CreateCompatibleDC(hdc)))
					OutTraceE("CreateCompatibleDC: ERROR err=%d at=%d\n", GetLastError(), __LINE__);
				// tricky part: CreateCompatibleBitmap is needed to set the dc size, but it has to be performed
				// against hdc to set for color depth, then selected (through SelectObject) against the temporary
				// dc to assign the needed size and color space to the temporary dc.
				if(!(hbmPic=CreateCompatibleBitmap(hdc, OrigWidth, OrigHeight)))
					OutTraceE("CreateCompatibleBitmap: ERROR err=%d at=%d\n", GetLastError(), __LINE__);
				if(!SelectObject(hTempDc, hbmPic))
					OutTraceE("SelectObject: ERROR err=%d at=%d\n", GetLastError(), __LINE__);
				if(!(*pSetDIBits)(hTempDc, hbmp, uStartScan, cScanLines, lpvBits, lpbmi, fuColorUse))
					OutTraceE("SetDIBitsToDevice: ERROR err=%d at=%d\n", GetLastError(), __LINE__);
				// v2.02.94: set HALFTONE stretching. Fixes "Celtic Kings Rage of War"
				SetStretchBltMode(hdc,HALFTONE);
				if(!(ret=(*pGDIStretchBlt)(hdc, XDest, YDest, dwWidth, dwHeight, hTempDc, 0, 0, OrigWidth, OrigHeight, SRCCOPY)))
					OutTraceE("StretchBlt: ERROR err=%d at=%d\n", GetLastError(), __LINE__);
				if(!(DeleteObject(hbmPic))) // v2.02.32 - avoid resource leakage
					OutTraceE("DeleteObject: ERROR err=%d at=%d\n", GetLastError(), __LINE__);
				if(!(DeleteDC(hTempDc)))
					OutTraceE("DeleteDC: ERROR err=%d at=%d\n", GetLastError(), __LINE__);
				return TRUE;
#endif
				break;
			case GDIMODE_EMULATED:
#if 0
				if (dxw.IsVirtual(hdc)){
					int X, Y;
					X=XDest+dxw.VirtualOffsetX;
					Y=YDest+dxw.VirtualOffsetY;
					OutTraceDW("SetDIBitsToDevice: virtual pos=(%d,%d)+(%d+%d)=(%d,%d)\n",
						XDest, YDest, dxw.VirtualOffsetX, dxw.VirtualOffsetY, X, Y);
					ret=(*pSetDIBits)(sdc.GetHdc(), hbmp, uStartScan, cScanLines, lpvBits, lpbmi, fuColorUse);
					if(!ret || (ret==GDI_ERROR)) OutTraceE("SetDIBitsToDevice: ERROR ret=%x err=%d\n", ret, GetLastError()); 
					return ret;
				}
#endif
				break;
			default:
				break;
		}
	}

	return (*pSetDIBits)(hdc, hbmp, uStartScan, cScanLines, lpvBits, lpbmi, fuColorUse);
	if(!ret || (ret==GDI_ERROR)) OutTraceE("SetDIBits: ERROR err=%d\n", GetLastError()); 
	return ret;
}

int WINAPI extGetDIBits(HDC hdc, HBITMAP hbmp, UINT uStartScan, UINT cScanLines, LPVOID lpvBits, LPBITMAPINFO lpbmi, UINT uUsage)
{
	int ret;
	BITMAPINFOHEADER *bmi;
	OutTraceDW("GetDIBits: hdc=%x hbmp=%x lines=(%d,%d) ColorUse=%x\n", hdc, hbmp, uStartScan, cScanLines, uUsage);
	bmi=(BITMAPINFOHEADER *)&(lpbmi->bmiHeader);
	OutTraceDW("GetDIBits: BitmapInfo dim=(%dx%d) Planes=%d BPP=%d Compression=%x SizeImage=%x\n",
		bmi->biWidth, bmi->biHeight, bmi->biPlanes, bmi->biBitCount, bmi->biCompression, bmi->biSizeImage);

	if(dxw.IsToRemap(hdc) && !bGDIRecursionFlag){
		//HDC hTempDc;
		//HBITMAP hbmPic;
		switch(dxw.GDIEmulationMode){
			case GDIMODE_SHAREDDC: // this will flicker !!!!
				sdc.GetPrimaryDC(hdc);
				ret=(*pGetDIBits)(sdc.GetHdc(), hbmp, uStartScan, cScanLines, lpvBits, lpbmi, uUsage);
				sdc.PutPrimaryDC(hdc, FALSE);
				return ret;
				break;
			case GDIMODE_EMULATED:
			default:
				break;
		}
	}

	return (*pGetDIBits)(hdc, hbmp, uStartScan, cScanLines, lpvBits, lpbmi, uUsage);
	if(!ret || (ret==GDI_ERROR)) OutTraceE("GetDIBits: ERROR err=%d\n", GetLastError()); 
	return ret;
}

#if 0
int WINAPI extSetDIBitsToDevice(HDC hdc, int XDest, int YDest, DWORD dwWidth, DWORD dwHeight, int XSrc, int YSrc, UINT uStartScan, UINT cScanLines, 
					const VOID *lpvBits, const BITMAPINFO *lpbmi, UINT fuColorUse)
{
	int ret;
	BITMAPINFOHEADER *bmi;
	OutTraceDW("SetDIBitsToDevice: hdc=%x dest=(%d,%d)-(%dx%d) src=(%d,%d) lines=(%d,%d)\n", 
		hdc, XDest, YDest, dwWidth, dwHeight, XSrc, YSrc, uStartScan, cScanLines);
	bmi=(BITMAPINFOHEADER *)&(lpbmi->bmiHeader);
	OutTraceDW("SetDIBitsToDevice: BitmapInfo dim=(%dx%d) Planes=%d BPP=%d Compression=%x SizeImage=%x\n",
		bmi->biWidth, bmi->biHeight, bmi->biPlanes, bmi->biBitCount, bmi->biCompression, bmi->biSizeImage);

	bGDIRecursionFlag = TRUE; // beware: it seems that SetDIBitsToDevice calls SetDIBits internally
	if(dxw.IsFullScreen()){
		HDC hTempDc;
		HBITMAP hbmPic;
		DWORD OrigWidth, OrigHeight;
		int OrigXDest, OrigYDest;
		OrigWidth=dwWidth;
		OrigHeight=dwHeight;
		OrigXDest=XDest;
		OrigYDest=YDest;
		switch(dxw.GDIEmulationMode){
			case GDIMODE_SHAREDDC: 
				if(dxw.IsToRemap(hdc)){
					sdc.GetPrimaryDC(hdc);
					ret=(*pSetDIBitsToDevice)(sdc.GetHdc(), XDest, YDest, dwWidth, dwHeight, XSrc, YSrc, uStartScan, cScanLines, lpvBits, lpbmi, fuColorUse);
					sdc.PutPrimaryDC(hdc, TRUE, XDest, YDest, dwWidth, dwHeight);
				}
				else{
					ret=(*pSetDIBitsToDevice)(hdc, XDest, YDest, dwWidth, dwHeight, XSrc, YSrc, uStartScan, cScanLines, lpvBits, lpbmi, fuColorUse);
				}
				bGDIRecursionFlag = FALSE;
				return ret;
				break;
			case GDIMODE_STRETCHED: 
				if(dxw.IsToRemap(hdc)){
					// blitting to primary surface !!!
					dxw.MapClient(&XDest, &YDest, (int *)&dwWidth, (int *)&dwHeight);
					OutTraceDW("SetDIBitsToDevice: fixed dest=(%d,%d)-(%dx%d)\n", XDest, YDest, dwWidth, dwHeight);
					if(!(hTempDc=CreateCompatibleDC(hdc)))
						OutTraceE("CreateCompatibleDC: ERROR err=%d at=%d\n", GetLastError(), __LINE__);
					// tricky part: CreateCompatibleBitmap is needed to set the dc size, but it has to be performed
					// against hdc to set for color depth, then selected (through SelectObject) against the temporary
					// dc to assign the needed size and color space to the temporary dc.
					if(!(hbmPic=CreateCompatibleBitmap(hdc, OrigWidth, OrigHeight)))
						OutTraceE("CreateCompatibleBitmap: ERROR err=%d at=%d\n", GetLastError(), __LINE__);
					if(!SelectObject(hTempDc, hbmPic))
						OutTraceE("SelectObject: ERROR err=%d at=%d\n", GetLastError(), __LINE__);
					if(!(ret=(*pSetDIBitsToDevice)(hTempDc, 0, 0, OrigWidth, OrigHeight, XSrc, YSrc, uStartScan, cScanLines, lpvBits, lpbmi, fuColorUse)))
						OutTraceE("SetDIBitsToDevice: ERROR err=%d at=%d\n", GetLastError(), __LINE__);
					bGDIRecursionFlag = FALSE;
					// v2.02.94: set HALFTONE stretching. Fixes "Celtic Kings Rage of War"
					SetStretchBltMode(hdc,HALFTONE);
					if(!(ret=(*pGDIStretchBlt)(hdc, XDest, YDest, dwWidth, dwHeight, hTempDc, 0, 0, OrigWidth, OrigHeight, SRCCOPY)))
						OutTraceE("StretchBlt: ERROR err=%d at=%d\n", GetLastError(), __LINE__);
					if(!(DeleteObject(hbmPic))) // v2.02.32 - avoid resource leakage
						OutTraceE("DeleteObject: ERROR err=%d at=%d\n", GetLastError(), __LINE__);
					if(!(DeleteDC(hTempDc)))
						OutTraceE("DeleteDC: ERROR err=%d at=%d\n", GetLastError(), __LINE__);
					return ret;
				}
				break;
			case GDIMODE_EMULATED:
				if (dxw.IsVirtual(hdc)){
					int X, Y;
					X=XDest+dxw.VirtualOffsetX;
					Y=YDest+dxw.VirtualOffsetY;
					OutTraceDW("SetDIBitsToDevice: virtual pos=(%d,%d)+(%d+%d)=(%d,%d)\n",
						XDest, YDest, dxw.VirtualOffsetX, dxw.VirtualOffsetY, X, Y);
					ret=(*pSetDIBitsToDevice)(hdc, X, Y, dwWidth, dwHeight, XSrc, YSrc, uStartScan, cScanLines, lpvBits, lpbmi, fuColorUse);
					bGDIRecursionFlag = FALSE;
					if(!ret || (ret==GDI_ERROR)) OutTraceE("SetDIBitsToDevice: ERROR ret=%x err=%d\n", ret, GetLastError()); 
					return ret;
				}
				break;
			default:
				break;
		}
	}

	ret=(*pSetDIBitsToDevice)(hdc, XDest, YDest, dwWidth, dwHeight, XSrc, YSrc, uStartScan, cScanLines, lpvBits, lpbmi, fuColorUse);
	bGDIRecursionFlag = FALSE;
	if(!ret || (ret==GDI_ERROR)) OutTraceE("SetDIBitsToDevice: ERROR ret=%x err=%d\n", ret, GetLastError()); 
	return ret;
}
#else
int WINAPI extSetDIBitsToDevice(HDC hdc, int XDest, int YDest, DWORD dwWidth, DWORD dwHeight, int XSrc, int YSrc, UINT uStartScan, UINT cScanLines, 
					const VOID *lpvBits, const BITMAPINFO *lpbmi, UINT fuColorUse)
{
	int ret;
	BITMAPINFOHEADER *bmi;
	OutTraceDW("SetDIBitsToDevice: hdc=%x dest=(%d,%d)-(%dx%d) src=(%d,%d) lines=(%d,%d)\n", 
		hdc, XDest, YDest, dwWidth, dwHeight, XSrc, YSrc, uStartScan, cScanLines);
	bmi=(BITMAPINFOHEADER *)&(lpbmi->bmiHeader);
	OutTraceDW("SetDIBitsToDevice: BitmapInfo dim=(%dx%d) Planes=%d BPP=%d Compression=%x SizeImage=%x\n",
		bmi->biWidth, bmi->biHeight, bmi->biPlanes, bmi->biBitCount, bmi->biCompression, bmi->biSizeImage);

	bGDIRecursionFlag = TRUE; // beware: it seems that SetDIBitsToDevice calls SetDIBits internally
	if(dxw.IsToRemap(hdc)){
		HDC hTempDc;
		HBITMAP hbmPic;
		DWORD OrigWidth, OrigHeight;
		int OrigXDest, OrigYDest;
		OrigWidth=dwWidth;
		OrigHeight=dwHeight;
		OrigXDest=XDest;
		OrigYDest=YDest;
		switch(dxw.GDIEmulationMode){
			case GDIMODE_SHAREDDC: 
				sdc.GetPrimaryDC(hdc);
				ret=(*pSetDIBitsToDevice)(sdc.GetHdc(), XDest, YDest, dwWidth, dwHeight, XSrc, YSrc, uStartScan, cScanLines, lpvBits, lpbmi, fuColorUse);
				sdc.PutPrimaryDC(hdc, TRUE, XDest, YDest, dwWidth, dwHeight);
				bGDIRecursionFlag = FALSE;
				return ret;
				break;
			case GDIMODE_STRETCHED: 
				// blitting to primary surface !!!
				dxw.MapClient(&XDest, &YDest, (int *)&dwWidth, (int *)&dwHeight);
				OutTraceDW("SetDIBitsToDevice: fixed dest=(%d,%d)-(%dx%d)\n", XDest, YDest, dwWidth, dwHeight);
				if(!(hTempDc=CreateCompatibleDC(hdc)))
					OutTraceE("CreateCompatibleDC: ERROR err=%d at=%d\n", GetLastError(), __LINE__);
				// tricky part: CreateCompatibleBitmap is needed to set the dc size, but it has to be performed
				// against hdc to set for color depth, then selected (through SelectObject) against the temporary
				// dc to assign the needed size and color space to the temporary dc.
				if(!(hbmPic=CreateCompatibleBitmap(hdc, OrigWidth, OrigHeight)))
					OutTraceE("CreateCompatibleBitmap: ERROR err=%d at=%d\n", GetLastError(), __LINE__);
				if(!SelectObject(hTempDc, hbmPic))
					OutTraceE("SelectObject: ERROR err=%d at=%d\n", GetLastError(), __LINE__);
				if(!(ret=(*pSetDIBitsToDevice)(hTempDc, 0, 0, OrigWidth, OrigHeight, XSrc, YSrc, uStartScan, cScanLines, lpvBits, lpbmi, fuColorUse)))
					OutTraceE("SetDIBitsToDevice: ERROR err=%d at=%d\n", GetLastError(), __LINE__);
				bGDIRecursionFlag = FALSE;
				// v2.02.94: set HALFTONE stretching. Fixes "Celtic Kings Rage of War"
				SetStretchBltMode(hdc,HALFTONE);
				if(!(ret=(*pGDIStretchBlt)(hdc, XDest, YDest, dwWidth, dwHeight, hTempDc, 0, 0, OrigWidth, OrigHeight, SRCCOPY)))
					OutTraceE("StretchBlt: ERROR err=%d at=%d\n", GetLastError(), __LINE__);
				if(!(DeleteObject(hbmPic))) // v2.02.32 - avoid resource leakage
					OutTraceE("DeleteObject: ERROR err=%d at=%d\n", GetLastError(), __LINE__);
				if(!(DeleteDC(hTempDc)))
					OutTraceE("DeleteDC: ERROR err=%d at=%d\n", GetLastError(), __LINE__);
				return ret;
				break;
			case GDIMODE_EMULATED:
				int X, Y;
				X=XDest+dxw.VirtualOffsetX;
				Y=YDest+dxw.VirtualOffsetY;
				OutTraceDW("SetDIBitsToDevice: virtual pos=(%d,%d)+(%d+%d)=(%d,%d)\n",
					XDest, YDest, dxw.VirtualOffsetX, dxw.VirtualOffsetY, X, Y);
				ret=(*pSetDIBitsToDevice)(hdc, X, Y, dwWidth, dwHeight, XSrc, YSrc, uStartScan, cScanLines, lpvBits, lpbmi, fuColorUse);
				bGDIRecursionFlag = FALSE;
				if(!ret || (ret==GDI_ERROR)) OutTraceE("SetDIBitsToDevice: ERROR ret=%x err=%d\n", ret, GetLastError()); 
				return ret;
			default:
				break;
		}
	}

	ret=(*pSetDIBitsToDevice)(hdc, XDest, YDest, dwWidth, dwHeight, XSrc, YSrc, uStartScan, cScanLines, lpvBits, lpbmi, fuColorUse);
	bGDIRecursionFlag = FALSE;
	if(!ret || (ret==GDI_ERROR)) OutTraceE("SetDIBitsToDevice: ERROR ret=%x err=%d\n", ret, GetLastError()); 
	return ret;
}
#endif

HBITMAP WINAPI extCreateCompatibleBitmap(HDC hdc, int nWidth, int nHeight)
{
	HBITMAP ret;
	OutTraceDW("CreateCompatibleBitmap: hdc=%x size=(%d,%d)\n", 
		hdc, nWidth, nHeight);

	if (dxw.IsToRemap(hdc)){
		switch(dxw.GDIEmulationMode){
			case GDIMODE_SHAREDDC: 
				sdc.GetPrimaryDC(hdc);
				ret=(*pCreateCompatibleBitmap)(sdc.GetHdc(), nWidth, nHeight);
				sdc.PutPrimaryDC(hdc, FALSE);
				break;
			case GDIMODE_STRETCHED:
				dxw.MapClient(&nWidth, &nHeight);
				OutTraceDW("CreateCompatibleBitmap: fixed size=(%d,%d)\n", nWidth, nHeight);
				break;
			default:
				break;
		}
	}

	ret=(*pCreateCompatibleBitmap)(hdc, nWidth, nHeight);
	if(!ret) OutTraceE("CreateCompatibleBitmap: ERROR ret=%x err=%d\n", ret, GetLastError()); 
	return ret;
}

COLORREF WINAPI extSetPixel(HDC hdc, int X, int Y, COLORREF crColor)
{
	COLORREF ret;
	OutTraceDW("SetPixel: hdc=%x color=%x point=(%d,%d)\n", hdc, crColor, X, Y);

	if(dxw.IsToRemap(hdc)){

		switch(dxw.GDIEmulationMode){
			case GDIMODE_SHAREDDC: 
				sdc.GetPrimaryDC(hdc);
				ret=(*pSetPixel)(sdc.GetHdc(), X, Y, crColor);
				sdc.PutPrimaryDC(hdc, TRUE, X, Y, 1, 1); // ????
				return ret; // this returns a COLORREF type
				break;
			case GDIMODE_STRETCHED: 
				dxw.MapClient(&X, &Y);
				OutTraceDW("SetPixel: fixed pos=(%d,%d)\n", X, Y);
				break;
		}
	}

	ret=(*pSetPixel)(hdc, X, Y, crColor);
	// both 0x00000000 and 0xFFFFFFFF are legitimate colors and therefore valid return codes...
	//if(ret==GDI_ERROR) OutTraceE("SetPixel: ERROR ret=%x err=%d\n", ret, GetLastError()); 
	return ret;
}

BOOL WINAPI extSetPixelV(HDC hdc, int X, int Y, COLORREF crColor)
{
	BOOL ret;
	OutTraceDW("SetPixelV: hdc=%x color=%x point=(%d,%d)\n", hdc, crColor, X, Y);

	if(dxw.IsToRemap(hdc)){
		switch(dxw.GDIEmulationMode){
			case GDIMODE_SHAREDDC: 
				sdc.GetPrimaryDC(hdc);
				ret=(*pSetPixelV)(sdc.GetHdc(), X, Y, crColor);
				sdc.PutPrimaryDC(hdc, TRUE, X, Y, 1, 1); // ????
				return ret; 
				break;
			case GDIMODE_STRETCHED: 
				dxw.MapClient(&X, &Y);
				OutTraceDW("SetPixelV: fixed pos=(%d,%d)\n", X, Y);
				break;
		}
	}

	ret=(*pSetPixelV)(hdc, X, Y, crColor);
	return ret;
}

BOOL WINAPI extEllipse(HDC hdc, int nLeftRect, int nTopRect, int nRightRect, int nBottomRect)
{
	int ret;
	OutTraceDW("Ellipse: hdc=%x rect=(%d,%d)-(%d,%d)\n", hdc, nLeftRect, nTopRect, nRightRect, nBottomRect);

	if (dxw.IsToRemap(hdc)){
		switch(dxw.GDIEmulationMode){
			case GDIMODE_SHAREDDC: 
				sdc.GetPrimaryDC(hdc);
				ret=(*pEllipse)(sdc.GetHdc(),nLeftRect, nTopRect, nRightRect, nBottomRect);
				sdc.PutPrimaryDC(hdc, TRUE, nLeftRect, nTopRect, nRightRect-nLeftRect, nBottomRect-nTopRect);
				return ret; 
				break;
			case GDIMODE_STRETCHED: 
			dxw.MapClient(&nLeftRect, &nTopRect, &nRightRect, &nBottomRect);
			OutTraceDW("Ellipse: fixed dest=(%d,%d)-(%d,%d)\n", nLeftRect, nTopRect, nRightRect, nBottomRect);
			break;
		}
	}

	ret=(*pEllipse)(hdc, nLeftRect, nTopRect, nRightRect, nBottomRect);
	if(!ret) OutTraceE("Ellipse: ERROR ret=%x err=%d\n", ret, GetLastError()); 
	return ret;
}

BOOL WINAPI extPolygon(HDC hdc, const POINT *lpPoints, int cCount)
{
	// LOGTOBEFIXED
	BOOL ret;
	if(IsTraceDDRAW){
		int i;
		OutTrace("Polygon: hdc=%x cCount=%d pt=", hdc, cCount); 
		for(i=0; i<cCount; i++) OutTrace("(%d,%d) ", lpPoints[i].x, lpPoints[i].y);
		OutTrace("\n");
	}
	if (dxw.IsToRemap(hdc)){
		int i, size;
		POINT *lpRemPoints;
		
		size = cCount * sizeof(POINT);
		lpRemPoints = (LPPOINT)malloc(size);
		memcpy(lpRemPoints, lpPoints, size);

		switch(dxw.GDIEmulationMode){
			case GDIMODE_SHAREDDC: 
				free(lpRemPoints);
				sdc.GetPrimaryDC(hdc);
				ret=(*pPolygon)(sdc.GetHdc(), lpPoints, cCount);
				sdc.PutPrimaryDC(hdc, TRUE);
				return ret; 
				break;
			case GDIMODE_STRETCHED: 
				OutTrace("Polygon: fixed cCount=%d pt=", cCount); 
				for(i=0; i<cCount; i++) {
					dxw.MapClient(&lpRemPoints[i]);
					OutTrace("(%d,%d) ", &lpRemPoints[i].x, &lpRemPoints[i].y);
				}
				OutTrace("\n");
				break;
			case GDIMODE_EMULATED:
				if (dxw.IsVirtual(hdc)){
					OutTrace("Polygon: fixed cCount=%d pt=", cCount); 
					for(i=0; i<cCount; i++) {
						lpRemPoints[i].x += dxw.VirtualOffsetX;
						lpRemPoints[i].y += dxw.VirtualOffsetY;
						OutTrace("(%d,%d) ", &lpPoints[i].x, &lpPoints[i].y);
					}
					OutTrace("\n");
				}
				break;	
		}
		ret=(*pPolygon)(hdc, lpRemPoints, cCount);
		free(lpRemPoints);
	}
	else {
		ret=(*pPolygon)(hdc, lpPoints, cCount);
	}
	if(!ret)OutTraceE("Polygon: ERROR ret=%x\n", ret); 
	return ret;
}

BOOL WINAPI extArc(HDC hdc, int nLeftRect, int nTopRect, int nRightRect, int nBottomRect, int nXStartArc, int nYStartArc, int nXEndArc, int nYEndArc)
{
	int ret;
	OutTraceDW("Arc: hdc=%x rect=(%d,%d)-(%d,%d) start=(%d,%d) end=(%d,%d)\n", 
		hdc, nLeftRect, nTopRect, nRightRect, nBottomRect, nXStartArc, nYStartArc, nXEndArc, nYEndArc);

	if (dxw.IsToRemap(hdc)){
		dxw.MapClient(&nLeftRect, &nTopRect, &nRightRect, &nBottomRect);
		dxw.MapClient(&nXStartArc, &nYStartArc, &nXEndArc, &nYEndArc);
		OutTraceDW("Arc: fixed rect=(%d,%d)-(%d,%d) start=(%d,%d) end=(%d,%d)\n", 
			nLeftRect, nTopRect, nRightRect, nBottomRect, nXStartArc, nYStartArc, nXEndArc, nYEndArc);
	}

	ret=(*pArc)(hdc, nLeftRect, nTopRect, nRightRect, nBottomRect, nXStartArc, nYStartArc, nXEndArc, nYEndArc);
	if(!ret) OutTraceE("Arc: ERROR ret=%x err=%d\n", ret, GetLastError()); 
	return ret;
}

HRGN WINAPI extCreateEllipticRgn(int nLeftRect, int nTopRect, int nRightRect, int nBottomRect)
{
	HRGN ret;
	OutTraceDW("CreateEllipticRgn: rect=(%d,%d)-(%d,%d)\n", nLeftRect, nTopRect, nRightRect, nBottomRect);

	if (dxw.IsFullScreen()){
		dxw.MapClient(&nLeftRect, &nTopRect, &nRightRect, &nBottomRect);
		OutTraceDW("CreateEllipticRgn: fixed rect=(%d,%d)-(%d,%d)\n", nLeftRect, nTopRect, nRightRect, nBottomRect);
	}

	ret=(*pCreateEllipticRgn)(nLeftRect, nTopRect, nRightRect, nBottomRect);
	if(!ret) OutTraceE("CreateEllipticRgn: ERROR ret=%x err=%d\n", ret, GetLastError()); 
	return ret;
}

HRGN WINAPI extCreateEllipticRgnIndirect(const RECT *lprc)
{
	HRGN ret;
	OutTraceDW("CreateEllipticRgnIndirect: rect=(%d,%d)-(%d,%d)\n", lprc->left, lprc->top, lprc->right, lprc->bottom);

	if (dxw.IsFullScreen()){
		dxw.MapClient((RECT *)lprc);
		OutTraceDW("CreateEllipticRgnIndirect: fixed rect=(%d,%d)-(%d,%d)\n", lprc->left, lprc->top, lprc->right, lprc->bottom);
	}

	ret=(*pCreateEllipticRgnIndirect)(lprc);
	if(!ret) OutTraceE("CreateEllipticRgnIndirect: ERROR ret=%x err=%d\n", ret, GetLastError()); 
	return ret;
}

HRGN WINAPI extCreateRectRgn(int nLeftRect, int nTopRect, int nRightRect, int nBottomRect)
{
	HRGN ret;
	OutTraceDW("CreateRectRgn: rect=(%d,%d)-(%d,%d)\n", nLeftRect, nTopRect, nRightRect, nBottomRect);

	if (dxw.IsFullScreen()){
		dxw.MapClient(&nLeftRect, &nTopRect, &nRightRect, &nBottomRect);
		OutTraceDW("CreateRectRgn: fixed rect=(%d,%d)-(%d,%d)\n", nLeftRect, nTopRect, nRightRect, nBottomRect);
	}

	ret=(*pCreateRectRgn)(nLeftRect, nTopRect, nRightRect, nBottomRect);
	if(!ret) OutTraceE("CreateRectRgn: ERROR ret=%x err=%d\n", ret, GetLastError()); 
	return ret;
}

HRGN WINAPI extCreateRectRgnIndirect(const RECT *lprc)
{
	HRGN ret;
	OutTraceDW("CreateRectRgnIndirect: rect=(%d,%d)-(%d,%d)\n", lprc->left, lprc->top, lprc->right, lprc->bottom);

	if (dxw.IsFullScreen()){
		dxw.MapClient((RECT *)lprc);
		OutTraceDW("CreateRectRgnIndirect: fixed rect=(%d,%d)-(%d,%d)\n", lprc->left, lprc->top, lprc->right, lprc->bottom);
	}

	ret=(*pCreateRectRgnIndirect)(lprc);
	if(!ret) OutTraceE("CreateRectRgnIndirect: ERROR ret=%x err=%d\n", ret, GetLastError()); 
	return ret;
}

BOOL WINAPI extSetRectRgn(HRGN hrgn, int nLeftRect, int nTopRect, int nRightRect, int nBottomRect)
{
	BOOL ret;
	OutTraceDW("SetRectRgn: hrgn=%x rect=(%d,%d)-(%d,%d)\n", hrgn, nLeftRect, nTopRect, nRightRect, nBottomRect);

	if (dxw.IsFullScreen()){
		dxw.MapClient(&nLeftRect, &nTopRect, &nRightRect, &nBottomRect);
		OutTraceDW("SetRectRgn: fixed rect=(%d,%d)-(%d,%d)\n", nLeftRect, nTopRect, nRightRect, nBottomRect);
	}

	ret=(*pSetRectRgn)(hrgn, nLeftRect, nTopRect, nRightRect, nBottomRect);
	if(!ret) OutTraceE("SetRectRgn: ERROR ret=%x err=%d\n", ret, GetLastError()); 
	return ret;
}

HRGN WINAPI extCreatePolygonRgn(const POINT *lpPoints, int cPoints, int fnPolyFillMode)
{
	// LOGTOBEFIXED
	HRGN ret;
	if(IsTraceDDRAW){
		int i;
		OutTrace("CreatePolygonRgn: PolyFillMode=%x cCount=%d pt=", fnPolyFillMode, cPoints); 
		for(i=0; i<cPoints; i++) OutTrace("(%d,%d) ", lpPoints[i].x, lpPoints[i].y);
		OutTrace("\n");
	}
	if (dxw.IsFullScreen()){
		int i;
		OutTrace("CreatePolygonRgn: fixed cCount=%d pt=", cPoints); 
		for(i=0; i<cPoints; i++) {
			dxw.MapClient((LPPOINT)&lpPoints[i]);
			OutTrace("(%d,%d) ", lpPoints[i].x, lpPoints[i].y);
		}
		OutTrace("\n");
	}
	ret=(*pCreatePolygonRgn)(lpPoints, cPoints, fnPolyFillMode);
	if(!ret)OutTraceE("CreatePolygonRgn: ERROR ret=%x\n", ret); 
	return ret;
}

BOOL WINAPI extMaskBlt(HDC hdcDest, int nXDest, int nYDest, int nWidth, int nHeight, HDC hdcSrc,
					 int nXSrc, int nYSrc, HBITMAP hbmMask, int xMask, int yMask, DWORD dwRop)
{
	BOOL ret;
	OutTraceDW("MaskBlt: hdcDest=%x pos=(%d,%d) size=(%dx%d) hdcSrc=%x pos=(%d,%d) hbmMask=%x Mask=(%d,%d) dwRop=%x\n",
		hdcDest, nXDest, nYDest, nWidth, nHeight, hdcSrc, nXSrc, nYSrc, hbmMask, xMask, yMask, dwRop);

	if (dxw.IsToRemap(hdcDest)){
		switch(dxw.GDIEmulationMode){
			case GDIMODE_SHAREDDC: 
				sdc.GetPrimaryDC(hdcDest);
				ret=(*pMaskBlt)(sdc.GetHdc(), nXDest, nYDest, nWidth, nHeight, hdcSrc, nXSrc, nYSrc, hbmMask, xMask, yMask, dwRop);
				sdc.PutPrimaryDC(hdcDest, TRUE, nXDest, nYDest, nWidth, nHeight);
				return ret;
				break;
			case GDIMODE_STRETCHED: 
				dxw.MapClient(&nXDest, &nYDest, &nWidth, &nHeight);
				OutTraceDW("MaskBlt: fixed pos=(%d,%d) size=(%dx%d)\n", nXDest, nYDest, nWidth, nHeight);
				break;
			case GDIMODE_EMULATED:
				// to be implemented
				break;
			default:
				break;
		}
	}

	ret=(*pMaskBlt)(hdcDest, nXDest, nYDest, nWidth, nHeight, hdcSrc, nXSrc, nYSrc, hbmMask, xMask, yMask, dwRop);
	if(!ret) OutTraceE("MaskBlt: ERROR ret=%x err=%d\n", ret, GetLastError()); 
	return ret;
}

BOOL WINAPI extSetViewportOrgEx(HDC hdc, int X, int Y, LPPOINT lpPoint)
{
	BOOL ret;
	OutTraceDW("SetViewportOrgEx: hdc=%x pos=(%d,%d)\n", hdc, X, Y);

	if(dxw.IsToRemap(hdc)){
		switch(dxw.GDIEmulationMode){
			case GDIMODE_EMULATED:
				if(dxw.IsVirtual(hdc)) {
					OutTraceDW("SetViewportOrgEx: virtual hdc\n");
					if(lpPoint){
						lpPoint->x = dxw.VirtualOffsetX;
						lpPoint->y = dxw.VirtualOffsetY;
					}
					dxw.VirtualOffsetX = X;
					dxw.VirtualOffsetY = Y;
					ret = TRUE;
				}
				else
					ret=(*pSetViewportOrgEx)(hdc, X, Y, lpPoint);
				break;
			case GDIMODE_SHAREDDC:
				sdc.GetPrimaryDC(hdc);
				ret=(*pSetViewportOrgEx)(sdc.GetHdc(), X, Y, lpPoint);
				sdc.PutPrimaryDC(hdc, FALSE);				
				break;
			case GDIMODE_STRETCHED:
				dxw.MapClient(&X, &Y);
				OutTraceDW("SetViewportOrgEx: fixed pos=(%d,%d)\n", X, Y);
				ret=(*pSetViewportOrgEx)(hdc, X, Y, lpPoint);
				if(ret && lpPoint) {
					OutTraceDW("SetViewportOrgEx: previous ViewPort=(%d,%d)\n", lpPoint->x, lpPoint->y);
					dxw.UnmapClient(lpPoint);
					OutTraceDW("SetViewportOrgEx: fixed previous ViewPort=(%d,%d)\n", lpPoint->x, lpPoint->y);
				}
				break;
			default:
				ret=(*pSetViewportOrgEx)(hdc, X, Y, lpPoint);
				break;
		}
	}
	else{
		ret=(*pSetViewportOrgEx)(hdc, X, Y, lpPoint);
	}

	if(ret && lpPoint) OutTraceDW("SetViewportOrgEx: previous ViewPort=(%d,%d)\n", lpPoint->x, lpPoint->y);
	if(!ret) OutTraceE("SetViewportOrgEx: ERROR ret=%x err=%d\n", ret, GetLastError()); 
	return ret;
}

BOOL WINAPI extSetViewportExtEx(HDC hdc, int X, int Y, LPSIZE lpSize)
{
	BOOL ret;
	OutTraceDW("SetViewportExtEx: hdc=%x pos=(%d,%d)\n", hdc, X, Y);

	if(dxw.IsToRemap(hdc)){
		switch(dxw.GDIEmulationMode){
			case GDIMODE_EMULATED:
				if(dxw.IsVirtual(hdc)) {
					OutTraceDW("SetViewportExtEx: virtual hdc\n");
					if(lpSize){
						lpSize->cx = dxw.VirtualExtentX;
						lpSize->cy = dxw.VirtualExtentY;
					}
					dxw.VirtualExtentX = X;
					dxw.VirtualExtentY = Y;
					ret = TRUE;
				}
				else
					ret=(*pSetViewportExtEx)(hdc, X, Y, lpSize);
				break;
			case GDIMODE_SHAREDDC:
				sdc.GetPrimaryDC(hdc);
				ret=(*pSetViewportExtEx)(sdc.GetHdc(), X, Y, lpSize);
				sdc.PutPrimaryDC(hdc, FALSE);				
				break;
			case GDIMODE_STRETCHED:
				dxw.MapClient(&X, &Y);
				OutTraceDW("SetViewportExtEx: fixed pos=(%d,%d)\n", X, Y);
				ret=(*pSetViewportExtEx)(hdc, X, Y, lpSize);
				if(ret && lpSize) {
					OutTraceDW("SetViewportExtEx: previous ViewPort=(%d,%d)\n", lpSize->cx, lpSize->cy);
					dxw.UnmapClient((LPPOINT)lpSize);
					OutTraceDW("SetViewportExtEx: fixed previous ViewPort=(%d,%d)\n", lpSize->cx, lpSize->cy);
				}
				break;
			default:
				ret=(*pSetViewportExtEx)(hdc, X, Y, lpSize);
				break;
		}
	}
	else{
		ret=(*pSetViewportExtEx)(hdc, X, Y, lpSize);
	}

	if(ret && lpSize) OutTraceDW("SetViewportOrgEx: previous ViewPort=(%d,%d)\n", lpSize->cx, lpSize->cy);
	if(!ret) OutTraceE("SetViewportOrgEx: ERROR ret=%x err=%d\n", ret, GetLastError()); 
	return ret;
}

BOOL WINAPI extGetViewportOrgEx(HDC hdc, LPPOINT lpPoint)
{
	BOOL ret;
	OutTraceDW("GetViewportOrgEx: hdc=%x\n", hdc);

	if(dxw.IsVirtual(hdc)) {
		lpPoint->x = dxw.VirtualOffsetX;
		lpPoint->y = dxw.VirtualOffsetY;
		return TRUE;
	}

	ret=(*pGetViewportOrgEx)(hdc, lpPoint);
	if(ret) {
		OutTraceDW("GetViewportOrgEx: ViewPort=(%d,%d)\n", lpPoint->x, lpPoint->y);
		if (dxw.IsToRemap(hdc)){
			dxw.UnmapClient(lpPoint);
			OutTraceDW("GetViewportOrgEx: fixed ViewPort=(%d,%d)\n", lpPoint->x, lpPoint->y);
		}
	}

	if(!ret) OutTraceE("GetViewportOrgEx: ERROR ret=%x err=%d\n", ret, GetLastError()); 
	return ret;
}

BOOL WINAPI extGetViewportExtEx(HDC hdc, LPPOINT lpPoint)
{
	BOOL ret;
	OutTraceDW("GetViewportExtEx: hdc=%x\n", hdc);

	if(dxw.IsVirtual(hdc)) {
		lpPoint->x = dxw.VirtualOffsetX;
		lpPoint->y = dxw.VirtualOffsetY;
		return TRUE;
	}

	ret=(*pGetViewportExtEx)(hdc, lpPoint);
	if(ret) {
		OutTraceDW("GetViewportExtEx: ViewPort=(%d,%d)\n", lpPoint->x, lpPoint->y);
		if (dxw.IsToRemap(hdc)){
			dxw.UnmapClient(lpPoint);
			OutTraceDW("GetViewportOrgEx: fixed ViewPort=(%d,%d)\n", lpPoint->x, lpPoint->y);
		}
	}

	if(!ret) OutTraceE("GetViewportExtEx: ERROR ret=%x err=%d\n", ret, GetLastError()); 
	return ret;
}

BOOL WINAPI extGetWindowOrgEx(HDC hdc, LPPOINT lpPoint)
{
	BOOL ret;
	OutTraceDW("GetWindowOrgEx: hdc=%x\n", hdc);

	ret=(*pGetWindowOrgEx)(hdc, lpPoint);
	if(ret) {
		OutTraceDW("GetWindowOrgEx: ViewPort=(%d,%d)\n", lpPoint->x, lpPoint->y);
		if (dxw.IsToRemap(hdc)){
			dxw.UnmapClient(lpPoint);
			OutTraceDW("GetWindowOrgEx: fixed ViewPort=(%d,%d)\n", lpPoint->x, lpPoint->y);
		}
	}

	if(!ret) OutTraceE("GetWindowOrgEx: ERROR ret=%x err=%d\n", ret, GetLastError()); 
	return ret;
}

BOOL WINAPI extSetWindowOrgEx(HDC hdc, int X, int Y, LPPOINT lpPoint)
{
	BOOL ret;
	OutTraceDW("SetWindowOrgEx: hdc=%x pos=(%d,%d)\n", hdc, X, Y);

	if (dxw.IsToRemap(hdc)){
		dxw.MapClient(&X, &Y);
		OutTraceDW("SetWindowOrgEx: fixed pos=(%d,%d)\n", X, Y);
	}

	ret=(*pSetWindowOrgEx)(hdc, X, Y, lpPoint);
	if(ret && lpPoint) {
		OutTraceDW("SetWindowOrgEx: previous ViewPort=(%d,%d)\n", lpPoint->x, lpPoint->y);
		if (dxw.IsToRemap(hdc)){
			dxw.UnmapClient(lpPoint);
			OutTraceDW("SetWindowOrgEx: fixed previous ViewPort=(%d,%d)\n", lpPoint->x, lpPoint->y);
		}
	}
	if(!ret) OutTraceE("SetWindowOrgEx: ERROR ret=%x err=%d\n", ret, GetLastError()); 
	return ret;
}

BOOL WINAPI extSetWindowExtEx(HDC hdc, int X, int Y, LPPOINT lpPoint)
{
	BOOL ret;
	OutTraceDW("SetWindowExtEx: hdc=%x pos=(%d,%d)\n", hdc, X, Y);

	if (dxw.IsToRemap(hdc)){
		dxw.MapClient(&X, &Y);
		OutTraceDW("SetWindowExtEx: fixed pos=(%d,%d)\n", X, Y);
	}

	ret=(*pSetWindowExtEx)(hdc, X, Y, lpPoint);
	if(ret && lpPoint) {
		OutTraceDW("SetWindowExtEx: previous ViewPort=(%d,%d)\n", lpPoint->x, lpPoint->y);
		if (dxw.IsToRemap(hdc)){
			dxw.UnmapClient(lpPoint);
			OutTraceDW("SetWindowExtEx: fixed previous ViewPort=(%d,%d)\n", lpPoint->x, lpPoint->y);
		}
	}
	if(!ret) OutTraceE("SetWindowExtEx: ERROR ret=%x err=%d\n", ret, GetLastError()); 
	return ret;
}

BOOL WINAPI extGetCurrentPositionEx(HDC hdc, LPPOINT lpPoint)
{
	BOOL ret;
	OutTraceDW("GetCurrentPositionEx: hdc=%x\n", hdc);

	ret=(*pGetCurrentPositionEx)(hdc, lpPoint);
	if(ret) {
		OutTraceDW("GetCurrentPositionEx: pos=(%d,%d)\n", lpPoint->x, lpPoint->y);
		if (dxw.IsToRemap(hdc)){
			switch(dxw.GDIEmulationMode){
				case GDIMODE_STRETCHED:
					dxw.UnmapClient(lpPoint);
					break;
				case GDIMODE_SHAREDDC:
					sdc.GetPrimaryDC(hdc);
					ret=(*pGetCurrentPositionEx)(sdc.GetHdc(), lpPoint);
					sdc.PutPrimaryDC(hdc, FALSE);
					return ret;
					break;
				case GDIMODE_EMULATED:
				default:
					break;
			}
			OutTraceDW("GetCurrentPositionEx: fixed pos=(%d,%d)\n", lpPoint->x, lpPoint->y);
		}
	}
	if(!ret) OutTraceE("GetCurrentPositionEx: ERROR ret=%x err=%d\n", ret, GetLastError()); 
	return ret;
}

BOOL WINAPI extCreateScalableFontResourceA(DWORD fdwHidden, LPCTSTR lpszFontRes, LPCTSTR lpszFontFile, LPCTSTR lpszCurrentPath)
{
	BOOL res;
	OutTraceDW("CreateScalableFontResource: hidden=%d FontRes=\"%s\" FontFile=\"%s\" CurrentPath=\"%s\"\n",
		fdwHidden, lpszFontRes, lpszFontFile, lpszCurrentPath);
	if(dxw.dwFlags3 & FONTBYPASS) return TRUE;
	res=(*pCreateScalableFontResourceA)(fdwHidden, lpszFontRes, lpszFontFile, lpszCurrentPath);
	if(!res) OutTraceE("CreateScalableFontResource: ERROR err=%d at=%d\n", GetLastError(), __LINE__);
	return res;
}

BOOL WINAPI extCreateScalableFontResourceW(DWORD fdwHidden, LPCWSTR lpszFontRes, LPCWSTR lpszFontFile, LPCWSTR lpszCurrentPath)
{
	BOOL res;
	OutTraceDW("CreateScalableFontResource: hidden=%d FontRes=\"%ls\" FontFile=\"%ls\" CurrentPath=\"%ls\"\n",
		fdwHidden, lpszFontRes, lpszFontFile, lpszCurrentPath);
	if(dxw.dwFlags3 & FONTBYPASS) return TRUE;
	res=(*pCreateScalableFontResourceW)(fdwHidden, lpszFontRes, lpszFontFile, lpszCurrentPath);
	if(!res) OutTraceE("CreateScalableFontResource: ERROR err=%d at=%d\n", GetLastError(), __LINE__);
	return res;
}

int WINAPI extAddFontResourceA(LPCTSTR lpszFontFile)
{
	BOOL res;
	OutTraceDW("AddFontResource: FontFile=\"%s\"\n", lpszFontFile);
	if(dxw.dwFlags3 & FONTBYPASS) {
		OutTraceDW("AddFontResource: SUPPRESS FontFile=\"%s\"\n", lpszFontFile);
		return TRUE;
	}
	res=(*pAddFontResourceA)(lpszFontFile);
	if(!res) OutTraceE("AddFontResource: ERROR err=%d at=%d\n", GetLastError(), __LINE__);
	return res;
}

int WINAPI extAddFontResourceW(LPCWSTR lpszFontFile)
{
	BOOL res;
	OutTraceDW("AddFontResource: FontFile=\"%ls\"\n", lpszFontFile);
	if(dxw.dwFlags3 & FONTBYPASS) {
		OutTraceDW("AddFontResource: SUPPRESS FontFile=\"%ls\"\n", lpszFontFile);
		return TRUE;
	}
	res=(*pAddFontResourceW)(lpszFontFile);
	if(!res) OutTraceE("AddFontResource: ERROR err=%d at=%d\n", GetLastError(), __LINE__);
	return res;
}

//BEWARE: SetPixelFormat must be issued on the same hdc used by OpenGL wglCreateContext, otherwise 
// a failure err=2000 ERROR INVALID PIXEL FORMAT occurs!!

BOOL WINAPI extGDISetPixelFormat(HDC hdc, int iPixelFormat, const PIXELFORMATDESCRIPTOR *ppfd)
{
	BOOL res;
	OutTraceDW("SetPixelFormat: hdc=%x PixelFormat=%d Flags=%x PixelType=%x(%s) ColorBits=%d RGBdepth=(%d,%d,%d) RGBshift=(%d,%d,%d)\n", 
		hdc, iPixelFormat, 
		ppfd->dwFlags, ppfd->iPixelType, ppfd->iPixelType?"PFD_TYPE_COLORINDEX":"PFD_TYPE_RGBA", ppfd->cColorBits,
		ppfd->cRedBits, ppfd->cGreenBits, ppfd->cBlueBits,
		ppfd->cRedShift, ppfd->cGreenShift, ppfd->cBlueShift);
	//if(dxw.dwFlags1 & EMULATESURFACE) {
	//	OutTraceDW("SetPixelFormat: prevent pixelformat change\n");
	//	return TRUE;
	//}
	if(dxw.IsDesktop(WindowFromDC(hdc))){
		HDC oldhdc = hdc;
		hdc=(*pGDIGetDC)(dxw.GethWnd());
		OutTraceDW("SetPixelFormat: remapped desktop hdc=%x->%x hWnd=%x\n", oldhdc, hdc, dxw.GethWnd());
	}	
	res=(*pGDISetPixelFormat)(hdc, iPixelFormat, ppfd);
	dxw.ActualPixelFormat.dwRGBBitCount = ppfd->cColorBits;
	if(!res) OutTraceE("SetPixelFormat: ERROR err=%d at=%d\n", GetLastError(), __LINE__);
	return res;
}

int WINAPI extGDIGetPixelFormat(HDC hdc)
{
	int res;
	OutTraceDW("GetPixelFormat: hdc=%x\n", hdc);
	if(dxw.IsDesktop(WindowFromDC(hdc))){
		HDC oldhdc = hdc;
		hdc=(*pGDIGetDC)(dxw.GethWnd());
		OutTraceDW("GetPixelFormat: remapped desktop hdc=%x->%x hWnd=%x\n", oldhdc, hdc, dxw.GethWnd());
	}	
	res=(*pGDIGetPixelFormat)(hdc);
	if(!res) OutTraceE("GetPixelFormat: ERROR err=%d at=%d\n", GetLastError(), __LINE__);
	else OutTraceDW("GetPixelFormat: res=%d\n", res);
	return res;
}

int WINAPI extChoosePixelFormat(HDC hdc, const PIXELFORMATDESCRIPTOR *ppfd)
{
	int res;
	OutTraceDW("ChoosePixelFormat: hdc=%x Flags=%x PixelType=%x(%s) ColorBits=%d RGBdepth=(%d,%d,%d) RGBshift=(%d,%d,%d)\n", 
		hdc, 
		ppfd->dwFlags, ppfd->iPixelType, ppfd->iPixelType?"PFD_TYPE_COLORINDEX":"PFD_TYPE_RGBA", ppfd->cColorBits,
		ppfd->cRedBits, ppfd->cGreenBits, ppfd->cBlueBits,
		ppfd->cRedShift, ppfd->cGreenShift, ppfd->cBlueShift);
	//PIXELFORMATDESCRIPTOR myppfd;
	//memcpy(&myppfd, ppfd, sizeof(PIXELFORMATDESCRIPTOR));
	//myppfd.dwFlags |= PFD_DRAW_TO_WINDOW;
	//res=(*pChoosePixelFormat)(hdc, &myppfd);
	res=(*pChoosePixelFormat)(hdc, ppfd);
	if(!res) OutTraceE("ChoosePixelFormat: ERROR err=%d at=%d\n", GetLastError(), __LINE__);
	else OutTraceDW("ChoosePixelFormat: res=%d\n", res);
	return res;
}

int WINAPI extDescribePixelFormat(HDC hdc, int iPixelFormat, UINT nBytes, LPPIXELFORMATDESCRIPTOR ppfd)
{
	int res;
	OutTraceDW("DescribePixelFormat: hdc=%x PixelFormat=%d Bytes=%d\n", hdc, iPixelFormat, nBytes);
	res=(*pDescribePixelFormat)(hdc, iPixelFormat, nBytes, ppfd);
	if(!res){
		OutTraceE("DescribePixelFormat: ERROR err=%d at=%d\n", GetLastError(), __LINE__);
		return res;
	}
	if (ppfd && nBytes==sizeof(PIXELFORMATDESCRIPTOR)){
		OutTraceDW("DescribePixelFormat: res=%d Flags=%x PixelType=%x(%s) ColorBits=%d RGBdepth=(%d,%d,%d) RGBshift=(%d,%d,%d)\n", 
			res, 
			ppfd->dwFlags, ppfd->iPixelType, ppfd->iPixelType?"PFD_TYPE_COLORINDEX":"PFD_TYPE_RGBA", ppfd->cColorBits,
			ppfd->cRedBits, ppfd->cGreenBits, ppfd->cBlueBits,
			ppfd->cRedShift, ppfd->cGreenShift, ppfd->cBlueShift);
	}
	else {
		OutTraceDW("DescribePixelFormat: res=%d\n", res);
	}
	return res;
}

DWORD WINAPI extGetObjectType(HGDIOBJ h)
{
	DWORD res;
	res=(*pGetObjectType)(h);
	OutTraceDW("GetObjectType: h=%x type=%x\n", h, res);
	if(h==dxw.VirtualHDC) {
		OutTraceDW("GetObjectType: REMAP h=%x type=%x->%x\n", h, res, OBJ_DC);
		res=OBJ_DC;
	}
	return res;
}

extern BOOL gFixed;

BOOL WINAPI extExtTextOutA(HDC hdc, int X, int Y, UINT fuOptions, const RECT *lprc, LPCSTR lpString, UINT cbCount, const INT *lpDx)
{
	RECT rc;
	BOOL ret;
	if(IsTraceDW){
		char sRect[81];
		if(lprc) sprintf(sRect, "(%d,%d)-(%d,%d)", lprc->left, lprc->top, lprc->right, lprc->bottom);
		else strcpy(sRect, "NULL");
		OutTrace("ExtTextOutA: hdc=%x pos=(%d,%d) String=\"%s\" rect=%s\n", hdc, X, Y, lpString, sRect);
	}

	if (dxw.IsToRemap(hdc) && !gFixed){
		switch(dxw.GDIEmulationMode){
			case GDIMODE_SHAREDDC: 
				sdc.GetPrimaryDC(hdc);
				ret=(*pExtTextOutA)(sdc.GetHdc(), X, Y, fuOptions, lprc, lpString, cbCount, lpDx);
				if(lprc){
					rc = *lprc;
					sdc.PutPrimaryDC(hdc, TRUE, rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top);
				}
				else
					sdc.PutPrimaryDC(hdc, TRUE);
				return ret;
				break;
			case GDIMODE_STRETCHED: 
				dxw.MapClient(&X, &Y);
				if(lprc) {
					rc = *lprc;
					dxw.MapClient(&rc);
				}
				OutTraceDW("ExtTextOutA: fixed pos=(%d,%d)\n", X, Y);
				break;
		}
	}
	if(lprc)
		return (*pExtTextOutA)(hdc, X, Y, fuOptions, &rc, lpString, cbCount, lpDx);
	else
		return (*pExtTextOutA)(hdc, X, Y, fuOptions, NULL, lpString, cbCount, lpDx);
}

BOOL WINAPI extExtTextOutW(HDC hdc, int X, int Y, UINT fuOptions, const RECT *lprc, LPCWSTR lpString, UINT cbCount, const INT *lpDx)
{
	RECT rc;
	BOOL ret;
	if(IsTraceDW){
		char sRect[81];
		if(lprc) sprintf(sRect, "(%d,%d)-(%d,%d)", lprc->left, lprc->top, lprc->right, lprc->bottom);
		else strcpy(sRect, "NULL");
		OutTrace("ExtTextOutW: hdc=%x pos=(%d,%d) String=\"%ls\" rect=%s\n", hdc, X, Y, lpString, sRect);
	}
	
	if (dxw.IsToRemap(hdc) && !gFixed){
		switch(dxw.GDIEmulationMode){
			case GDIMODE_SHAREDDC: 
				sdc.GetPrimaryDC(hdc);
				ret=(*pExtTextOutW)(sdc.GetHdc(), X, Y, fuOptions, lprc, lpString, cbCount, lpDx);
				if(lprc){
					rc = *lprc;
					sdc.PutPrimaryDC(hdc, TRUE, rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top);
				}
				else
					sdc.PutPrimaryDC(hdc, TRUE);
			return ret;
				break;
			case GDIMODE_STRETCHED: 
				dxw.MapClient(&X, &Y);
				if(lprc) {
					rc = *lprc;
					dxw.MapClient(&rc);
				}
				OutTraceDW("ExtTextOutW: fixed pos=(%d,%d)\n", X, Y);
				break;
		}
	}
	if(lprc)
		return (*pExtTextOutW)(hdc, X, Y, fuOptions, &rc, lpString, cbCount, lpDx);
	else
		return (*pExtTextOutW)(hdc, X, Y, fuOptions, NULL, lpString, cbCount, lpDx);
	
}

#ifdef TRACEPALETTE
BOOL WINAPI extResizePalette(HPALETTE hpal, UINT nEntries)
{
	OutTrace("ResizePalette: hpal=%x nEntries=%d\n", hpal, nEntries);
	return (*pResizePalette)(hpal, nEntries);
}
#endif

#if 0
COLORREF WINAPI extSetBkColor(HDC hdc, COLORREF crColor)
{
	OutTrace("SetBkColor: hdc=%x color=%x\n", hdc, crColor);
	if(dxw.Windowize && dxw.IsRealDesktop(WindowFromDC(hdc))) {
		OutTrace("Remap desktop hdc=%x->%x\n", hdc, GetDC(dxw.GethWnd()));
		hdc=GetDC(dxw.GethWnd());
	}
	return (*pSetBkColor)(hdc, crColor);
}

int WINAPI extSetBkMode(HDC hdc, int iBkMode)
{
	OutTrace("SetBkMode: hdc=%x bkmode=%x\n", hdc, iBkMode);
	if(dxw.Windowize && dxw.IsRealDesktop(WindowFromDC(hdc))) {
		OutTrace("Remap desktop hdc=%x->%x\n", hdc, GetDC(dxw.GethWnd()));
		hdc=GetDC(dxw.GethWnd());
	}
	return (*pSetBkMode)(hdc, iBkMode);
}

COLORREF WINAPI extSetTextColor(HDC hdc, COLORREF crColor)
{
	OutTrace("SetTextColor: hdc=%x color=%x\n", hdc, crColor);
	if(dxw.Windowize && dxw.IsRealDesktop(WindowFromDC(hdc))) {
		OutTrace("Remap desktop hdc=%x->%x\n", hdc, GetDC(dxw.GethWnd()));
		hdc=GetDC(dxw.GethWnd());
	}
	return (*pSetTextColor)(hdc, crColor);
}

// unhooked, since quite surprisingly all rectangles showed properly scaled already in RollerCoaster Tycoon !!
DWORD WINAPI extGetRegionData(HRGN hRgn, DWORD dwCount, LPRGNDATA lpRgnData)
{
	DWORD ret;
	RECT *data;
	ret=(*pGetRegionData)(hRgn, dwCount, lpRgnData);
	if(IsDebug){
		OutTrace("GetRegionData: hRgn=%x count=%d RgnData=%x ret=%d\n", hRgn, dwCount, lpRgnData, ret);
		if(lpRgnData && dwCount){
			OutTrace("GetRegionData: size=%d type=%x(%s) count=%d size=%d rect=(%d,%d)-(%d,%d)\n", 
				lpRgnData->rdh.dwSize, lpRgnData->rdh.iType, (lpRgnData->rdh.iType==RDH_RECTANGLES ? "RDH_RECTANGLES" : "unknown"), 
				lpRgnData->rdh.nCount, lpRgnData->rdh.nRgnSize, 
				lpRgnData->rdh.rcBound.left, lpRgnData->rdh.rcBound.top, lpRgnData->rdh.rcBound.right, lpRgnData->rdh.rcBound.bottom);
			data=(RECT *)lpRgnData->Buffer;
			for(DWORD i=0; i<lpRgnData->rdh.nCount; i++)
				OutTrace("GetRegionData: item=%i rect=(%d,%d)-(%d,%d)\n", i, data[i].left, data[i].top, data[i].right, data[i].bottom);
		}
	}

	if(dxw.IsFullScreen() && lpRgnData && dwCount){
        dxw.UnmapClient(&(lpRgnData->rdh.rcBound));
		data=(RECT *)lpRgnData->Buffer;
		for(DWORD i=0; i<lpRgnData->rdh.nCount; i++) dxw.UnmapClient(&(data[i]));
		if(IsDebug){
			OutTrace("GetRegionData: FIXED rect=(%d,%d)-(%d,%d)\n", 
				lpRgnData->rdh.rcBound.left, lpRgnData->rdh.rcBound.top, lpRgnData->rdh.rcBound.right, lpRgnData->rdh.rcBound.bottom);
			data=(RECT *)lpRgnData->Buffer;
			for(DWORD i=0; i<lpRgnData->rdh.nCount; i++)
				OutTrace("GetRegionData: FIXED item=%i rect=(%d,%d)-(%d,%d)\n", 
					i, data[i].left, data[i].top, data[i].right, data[i].bottom);
		}
	}

	return ret;
}
#endif

int WINAPI extOffsetRgn(HRGN hrgn, int nXOffset, int nYOffset)
{
	OutTraceDW("OffsetRgn(hrgn=%x nXOffset=%d nYOffset=%d\n", hrgn, nXOffset, nYOffset);

	if(dxw.IsFullScreen()){
		switch(dxw.GDIEmulationMode){
		case GDIMODE_STRETCHED: 
			dxw.MapClient(&nXOffset, &nYOffset);
			OutTraceDW("OffsetRgn: fixed STRETCHED offset=(%d,%d)\n", nXOffset, nYOffset);
			break;
		default:
			break;
		}
	}

	return (*pOffsetRgn)(hrgn, nXOffset, nYOffset);
}

COLORREF WINAPI extGetPixel(HDC hdc, int nXPos, int nYPos)
{
	COLORREF ret;
	OutTraceDW("CreateDIBitmap: hdc=%x\n", hdc);

	if(dxw.IsToRemap(hdc)) {
		switch(dxw.GDIEmulationMode){
			case GDIMODE_SHAREDDC: 
				sdc.GetPrimaryDC(hdc);
				ret=(*pGetPixel)(sdc.GetHdc(), nXPos, nYPos);
				sdc.PutPrimaryDC(hdc, FALSE);
				return ret;
				break;			
			default:
				// to do .....
				break;
		}
	}
	
	ret=(*pGetPixel)(hdc, nXPos, nYPos);
	if(!ret) OutTraceE("CreateDIBitmap ERROR: err=%d\n", GetLastError());
	return ret;}

BOOL WINAPI extPlgBlt(HDC hdcDest, const POINT *lpPoint, HDC hdcSrc, int nXSrc, int nYSrc, int nWidth, int nHeight, HBITMAP hbmMask, int xMask, int yMask)
{
	MessageBox(0, "PlgBlt", "DxWnd", MB_OK);
	return (COLORREF)0;
}

BOOL WINAPI extChord(HDC hdc, int nLeftRect, int nTopRect, int nRightRect, int nBottomRect, int nXRadial1, int nYRadial1, int nXRadial2, int nYRadial2)
{
	MessageBox(0, "Chord", "DxWnd", MB_OK);
	return (COLORREF)0;
}

BOOL WINAPI extPolyTextOutA(HDC hdc, const POLYTEXTA *pptxt, int cStrings)
{
	MessageBox(0, "PolyTextOutA", "DxWnd", MB_OK);
	return TRUE;
}

BOOL WINAPI extPolyTextOutW(HDC hdc, const POLYTEXTW *pptxt, int cStrings)
{
	MessageBox(0, "PolyTextOutW", "DxWnd", MB_OK);
	return TRUE;
}

HBITMAP WINAPI extCreateDIBitmap(HDC hdc, BITMAPINFOHEADER *lpbmih, DWORD fdwInit, const VOID *lpbInit, const BITMAPINFO *lpbmi, UINT fuUsage)
{
	HBITMAP ret;
	OutTraceDW("CreateDIBitmap: hdc=%x\n", hdc);

	if(dxw.IsToRemap(hdc)) {
		switch(dxw.GDIEmulationMode){
			case GDIMODE_SHAREDDC: 
				sdc.GetPrimaryDC(hdc);
				ret=(*pCreateDIBitmap)(sdc.GetHdc(), lpbmih, fdwInit, lpbInit, lpbmi, fuUsage);
				sdc.PutPrimaryDC(hdc, FALSE);
				return ret;
				break;			
			default:
				break;
		}
	}
		
	ret = (*pCreateDIBitmap)(hdc, lpbmih, fdwInit, lpbInit, lpbmi, fuUsage);
	if(!ret) OutTraceE("CreateDIBitmap ERROR: err=%d\n", GetLastError());
	return ret;
}

HBITMAP WINAPI extCreateDIBSection(HDC hdc, const BITMAPINFO *pbmi, UINT iUsage, VOID **ppvBits, HANDLE hSection, DWORD dwOffset)
{
	HBITMAP ret;
	OutTraceDW("CreateDIBitmap: hdc=%x\n", hdc);

	if(dxw.IsToRemap(hdc)) {
		switch(dxw.GDIEmulationMode){
			case GDIMODE_SHAREDDC: 
				sdc.GetPrimaryDC(hdc);
				ret=(*pCreateDIBSection)(sdc.GetHdc(), pbmi, iUsage, ppvBits, hSection, dwOffset);
				sdc.PutPrimaryDC(hdc, FALSE);
				return ret;
				break;			
			default:
				break;
		}
	}
		
	ret=(*pCreateDIBSection)(hdc, pbmi, iUsage, ppvBits, hSection, dwOffset);
	if(!ret) OutTraceE("CreateDIBitmap ERROR: err=%d\n", GetLastError());
	return ret;
}

HBITMAP WINAPI extCreateDiscardableBitmap(HDC hdc, int nWidth, int nHeight)
{
	HBITMAP ret;
	OutTraceDW("CreateDiscardableBitmap: hdc=%x size=(%dx%d)\n", hdc, nWidth, nHeight);

	if(dxw.IsToRemap(hdc)) {
		switch(dxw.GDIEmulationMode){
			case GDIMODE_SHAREDDC: 
				sdc.GetPrimaryDC(hdc);
				ret=(*pCreateDiscardableBitmap)(sdc.GetHdc(), nWidth, nHeight);
				sdc.PutPrimaryDC(hdc, FALSE);
				return ret;
				break;			
			default:
				break;
		}
	}
		
	ret=(*pCreateDiscardableBitmap)(hdc, nWidth, nHeight);
	if(!ret) OutTraceE("CreateDiscardableBitmap ERROR: err=%d\n", GetLastError());
	return ret;
}

BOOL WINAPI extExtFloodFill(HDC hdc, int nXStart, int nYStart, COLORREF crColor, UINT fuFillType)
{
	BOOL ret;
	OutTraceDW("ExtFloodFill: hdc=%x pos=(%d,%d)\n", hdc, nXStart, nYStart);

	if(dxw.IsToRemap(hdc)) {
		switch(dxw.GDIEmulationMode){
			case GDIMODE_SHAREDDC: 
				sdc.GetPrimaryDC(hdc);
				ret=(*pExtFloodFill)(sdc.GetHdc(), nXStart, nYStart, crColor, fuFillType);
				sdc.PutPrimaryDC(hdc, TRUE);
				return ret;
				break;			
			default:
				break;
		}
	}
		
	ret=(*pExtFloodFill)(hdc, nXStart, nYStart, crColor, fuFillType);
	if(!ret) OutTraceE("ExtFloodFill ERROR: err=%d\n", GetLastError());
	return ret;
}

BOOL WINAPI extGdiAlphaBlend(HDC hdcDest, int xoriginDest, int yoriginDest, int wDest, int hDest, HDC hdcSrc, int xoriginSrc, int yoriginSrc, int wSrc, int hSrc, BLENDFUNCTION ftn)
{
	// to be handled the 4 flux cases .....
	_Warn("GdiAlphaBlend");
	return TRUE;
}

BOOL WINAPI extGdiGradientFill(HDC hdc, PTRIVERTEX pVertex, ULONG nVertex, PVOID pMesh, ULONG nMesh, ULONG ulMode)
{
	_Warn("GdiGradientFill");
	return TRUE;
}

BOOL WINAPI extGdiTransparentBlt(HDC hdcDest, int xoriginDest, int yoriginDest, int wDest, int hDest,
							  HDC  hdcSrc, int xoriginSrc, int yoriginSrc, int wSrc, int hSrc, UINT crTransparent)
{
	// to be handled the 4 flux cases .....
	_Warn("GdiTransparentBlt");
	return TRUE;
}

BOOL WINAPI extPie(HDC hdc, int nLeftRect, int nTopRect, int nRightRect, int nBottomRect, int nXRadial1, int nYRadial1, int nXRadial2, int nYRadial2)
{
	BOOL ret;
	OutTraceDW("Pie: hdc=%x rect=(%d,%d)-(%d,%d)\n", hdc, nLeftRect, nTopRect, nRightRect, nBottomRect);

	if(dxw.IsToRemap(hdc)) {
		switch(dxw.GDIEmulationMode){
			case GDIMODE_SHAREDDC: 
				sdc.GetPrimaryDC(hdc);
				ret=(*pPie)(sdc.GetHdc(), nLeftRect, nTopRect, nRightRect, nBottomRect, nXRadial1, nYRadial1, nXRadial2, nYRadial2);
				sdc.PutPrimaryDC(hdc, TRUE);
				return ret;
				break;			
			default:
				break;
		}
	}
		
	ret=(*pPie)(hdc, nLeftRect, nTopRect, nRightRect, nBottomRect, nXRadial1, nYRadial1, nXRadial2, nYRadial2);
	if(!ret) OutTraceE("Pie ERROR: err=%d\n", GetLastError());
	return ret;
}

BOOL WINAPI extAngleArc(HDC hdc, int X, int Y, DWORD dwRadius, FLOAT eStartAngle, FLOAT eSweepAngle)
{
	BOOL ret;
	OutTraceDW("AngleArc: hdc=%x pos=(%d,%d)\n", hdc, X, Y);

	if(dxw.IsToRemap(hdc)) {
		switch(dxw.GDIEmulationMode){
			case GDIMODE_SHAREDDC: 
				sdc.GetPrimaryDC(hdc);
				ret=(*pAngleArc)(sdc.GetHdc(), X, Y, dwRadius, eStartAngle, eSweepAngle);
				sdc.PutPrimaryDC(hdc, TRUE);
				return ret;
				break;			
			default:
				break;
		}
	}
		
	ret=(*pAngleArc)(hdc, X, Y, dwRadius, eStartAngle, eSweepAngle);
	if(!ret) OutTraceE("AngleArc ERROR: err=%d\n", GetLastError());
	return ret;
}

BOOL WINAPI extPolyPolyline(HDC hdc, const POINT *lppt, const DWORD *lpdwPolyPoints, DWORD cCount)
{
	BOOL ret;
	OutTraceDW("PolyPolyline: hdc=%x\n", hdc);

	if(dxw.IsToRemap(hdc)) {
		switch(dxw.GDIEmulationMode){
			case GDIMODE_SHAREDDC: 
				sdc.GetPrimaryDC(hdc);
				ret=(*pPolyPolyline)(sdc.GetHdc(), lppt, lpdwPolyPoints, cCount);
				sdc.PutPrimaryDC(hdc, TRUE);
				return ret;
				break;			
			default:
				break;
		}
	}
		
	ret=(*pPolyPolyline)(hdc, lppt, lpdwPolyPoints, cCount);
	if(!ret) OutTraceE("PolyPolyline ERROR: err=%d\n", GetLastError());
	return ret;
}

BOOL WINAPI extFillRgn(HDC hdc, HRGN hrgn, HBRUSH hbr)
{
	BOOL ret;
	OutTraceDW("FillRgn: hdc=%x\n", hdc);

	if(dxw.IsToRemap(hdc)) {
		switch(dxw.GDIEmulationMode){
			case GDIMODE_SHAREDDC: 
				sdc.GetPrimaryDC(hdc);
				ret=(*pFillRgn)(sdc.GetHdc(), hrgn, hbr);
				sdc.PutPrimaryDC(hdc, TRUE);
				return ret;
				break;			
			default:
				break;
		}
	}
		
	ret=(*pFillRgn)(hdc, hrgn, hbr);
	if(!ret) OutTraceE("FillRgn ERROR: err=%d\n", GetLastError());
	return ret;
}

BOOL WINAPI extFrameRgn(HDC hdc, HRGN hrgn, HBRUSH hbr, int nWidth, int nHeight)
{
	BOOL ret;
	OutTraceDW("FrameRgn: hdc=%x\n", hdc);

	if(dxw.IsToRemap(hdc)) {
		switch(dxw.GDIEmulationMode){
			case GDIMODE_SHAREDDC: 
				sdc.GetPrimaryDC(hdc);
				ret=(*pFrameRgn)(sdc.GetHdc(), hrgn, hbr, nWidth, nHeight);
				sdc.PutPrimaryDC(hdc, TRUE);
				return ret;
				break;			
			default:
				break;
		}
	}
		
	ret=(*pFrameRgn)(hdc, hrgn, hbr, nWidth, nHeight);
	if(!ret) OutTraceE("FrameRgn ERROR: err=%d\n", GetLastError());
	return ret;
}

BOOL WINAPI extInvertRgn(HDC hdc, HRGN hrgn)
{
	BOOL ret;
	OutTraceDW("InvertRgn: hdc=%x hrgn=%x\n", hdc, hrgn);

	if(dxw.IsToRemap(hdc)) {
		switch(dxw.GDIEmulationMode){
			case GDIMODE_SHAREDDC: 
				sdc.GetPrimaryDC(hdc);
				ret=(*pInvertRgn)(sdc.GetHdc(), hrgn);
				sdc.PutPrimaryDC(hdc, TRUE);
				return ret;
				break;			
			default:
				break;
		}
	}
		
	ret=(*pInvertRgn)(hdc, hrgn);
	if(!ret) OutTraceE("InvertRgn ERROR: err=%d\n", GetLastError());
	return ret;
}

BOOL WINAPI extPaintRgn(HDC hdc, HRGN hrgn)
{
	BOOL ret;
	OutTraceDW("PaintRgn: hdc=%x hrgn=%x\n", hdc, hrgn);

	if(dxw.IsToRemap(hdc)) {
		switch(dxw.GDIEmulationMode){
			case GDIMODE_SHAREDDC: 
				sdc.GetPrimaryDC(hdc);
				ret=(*pPaintRgn)(sdc.GetHdc(), hrgn);
				sdc.PutPrimaryDC(hdc, TRUE);
				return ret;
				break;			
			default:
				break;
		}
	}
		
	ret=(*pPaintRgn)(hdc, hrgn);
	if(!ret) OutTraceE("PaintRgn ERROR: err=%d\n", GetLastError());
	return ret;
}

int WINAPI extSetMapMode(HDC hdc, int fnMapMode)
{
	int ret;
	OutTraceDW("SetMapMode: hdc=%x fnMapMode=%x\n", hdc, fnMapMode);

	if(dxw.IsToRemap(hdc)) {
		switch(dxw.GDIEmulationMode){
			case GDIMODE_SHAREDDC: 
				sdc.GetPrimaryDC(hdc);
				ret=(*pSetMapMode)(sdc.GetHdc(), fnMapMode);
				sdc.PutPrimaryDC(hdc, FALSE);
				return ret;
				break;			
			default:
				break;
		}
	}
		
	ret=(*pSetMapMode)(hdc, fnMapMode);
	if(!ret) OutTraceE("SetMapMode ERROR: err=%d\n", GetLastError());
	return ret;
}

BOOL WINAPI extRoundRect(HDC hdc, int nLeftRect, int nTopRect, int nRightRect, int nBottomRect, int nWidth, int nHeight)
{
	int ret;
	OutTraceDW("RoundRect: hdc=%x rect=(%d,%d)-(%d,%d) ellipse=(%dx%d)\n", hdc, nLeftRect, nTopRect, nRightRect, nBottomRect, nWidth, nHeight);

	if(dxw.IsToRemap(hdc)) {
		switch(dxw.GDIEmulationMode){
			case GDIMODE_SHAREDDC: 
				sdc.GetPrimaryDC(hdc);
				ret=(*pRoundRect)(sdc.GetHdc(), nLeftRect, nTopRect, nRightRect, nBottomRect, nWidth, nHeight);
				sdc.PutPrimaryDC(hdc, TRUE, nLeftRect, nTopRect, nRightRect, nBottomRect);
				return ret;
				break;			
			default:
				break;
		}
	}
		
	ret=(*pRoundRect)(hdc, nLeftRect, nTopRect, nRightRect, nBottomRect, nWidth, nHeight);
	if(!ret) OutTraceE("RoundRect ERROR: err=%d\n", GetLastError());
	return ret;
}

BOOL WINAPI extPolyPolygon(HDC hdc, const POINT *lpPoints, const INT *lpPolyCounts, int nCount)
{
	BOOL ret;
	OutTraceDW("PolyPolygon: hdc=%x\n", hdc);

	if(dxw.IsToRemap(hdc)) {
		switch(dxw.GDIEmulationMode){
			case GDIMODE_SHAREDDC: 
				sdc.GetPrimaryDC(hdc);
				ret=(*pPolyPolygon)(sdc.GetHdc(), lpPoints, lpPolyCounts, nCount);
				sdc.PutPrimaryDC(hdc, TRUE);
				return ret;
				break;			
			default:
				break;
		}
	}
		
	ret=(*pPolyPolygon)(hdc, lpPoints, lpPolyCounts, nCount);
	if(!ret) OutTraceE("PolyPolygon ERROR: err=%d\n", GetLastError());
	return ret;
}
