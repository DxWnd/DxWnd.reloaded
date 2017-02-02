//------------------------------------------------------------------------------
// Tiny WinG32 Proxy DLL v1.0 [30.01.2017]
// Copyright 2010 Evgeny Vrublevsky <veg@tut.by>, 2017 GHO
//------------------------------------------------------------------------------
#define APP_NAME		"Tiny WinG32 Proxy"
#define APP_VERSION		"1.0 [28.05.2010]"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <ctype.h>

void InitDxWnd(void);
void DxWndEndHook(void);

struct wing32_dll
{
	HMODULE dll;
	FARPROC	WinGCreateDC;
	FARPROC	WinGCreateBitmap;
	FARPROC	WinGBitBlt;
	FARPROC	WinGStretchBlt;
	FARPROC	WinGRecommendDIBFormat;
	FARPROC	WinGGetDIBPointer;
	FARPROC	WinGSetDIBColorTable;
	FARPROC	WinGGetDIBColorTable;
	FARPROC	WinGCreateHalftonePalette;
	FARPROC	WinGCreateHalftoneBrush;
} wing32;

__declspec(naked) void FakeWinGCreateDC()					{ _asm { jmp [wing32.WinGCreateDC] } }
__declspec(naked) void FakeWinGCreateBitmap()				{ _asm { jmp [wing32.WinGCreateBitmap] } }
__declspec(naked) void FakeWinGBitBlt()						{ _asm { jmp [wing32.WinGBitBlt] } }
__declspec(naked) void FakeWinGStretchBlt()					{ _asm { jmp [wing32.WinGStretchBlt] } }
__declspec(naked) void FakeWinGRecommendDIBFormat()			{ _asm { jmp [wing32.WinGRecommendDIBFormat] } }
__declspec(naked) void FakeWinGGetDIBPointer()				{ _asm { jmp [wing32.WinGGetDIBPointer] } }
__declspec(naked) void FakeWinGSetDIBColorTable()			{ _asm { jmp [wing32.WinGSetDIBColorTable] } }
__declspec(naked) void FakeWinGGetDIBColorTable()			{ _asm { jmp [wing32.WinGGetDIBColorTable] } }
__declspec(naked) void FakeWinGCreateHalftonePalette()		{ _asm { jmp [wing32.WinGCreateHalftonePalette] } }
__declspec(naked) void FakeWinGCreateHalftoneBrush()		{ _asm { jmp [wing32.WinGCreateHalftoneBrush] } }

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	char path[MAX_PATH];
	switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
		{
			CopyMemory(path+GetSystemDirectory(path,MAX_PATH-10), "\\wing32.dll",11);
			wing32.dll = LoadLibrary(path);
			if (wing32.dll == false)
			{
				MessageBox(0, "Cannot load original wing32.dll library", APP_NAME, MB_ICONERROR);
				ExitProcess(0);
			}
			wing32.WinGCreateDC					= GetProcAddress(wing32.dll, "WinGCreateDC");
			wing32.WinGCreateBitmap				= GetProcAddress(wing32.dll, "WinGCreateBitmap");
			wing32.WinGBitBlt					= GetProcAddress(wing32.dll, "WinGBitBlt");
			wing32.WinGStretchBlt				= GetProcAddress(wing32.dll, "WinGStretchBlt");
			wing32.WinGRecommendDIBFormat		= GetProcAddress(wing32.dll, "WinGRecommendDIBFormat");
			wing32.WinGGetDIBPointer			= GetProcAddress(wing32.dll, "WinGGetDIBPointer");
			wing32.WinGSetDIBColorTable			= GetProcAddress(wing32.dll, "WinGSetDIBColorTable");
			wing32.WinGGetDIBColorTable			= GetProcAddress(wing32.dll, "WinGGetDIBColorTable");
			wing32.WinGCreateHalftonePalette	= GetProcAddress(wing32.dll, "WinGCreateHalftonePalette");
			wing32.WinGCreateHalftoneBrush		= GetProcAddress(wing32.dll, "WinGCreateHalftoneBrush");

			InitDxWnd();
			break;
		}
		case DLL_PROCESS_DETACH:
		{
			DxWndEndHook();
			FreeLibrary(wing32.dll);
		}
		break;
	}
	return TRUE;
}
