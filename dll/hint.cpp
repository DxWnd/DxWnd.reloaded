#include "windows.h"
#include "dxwnd.h"
#include "dxwcore.hpp"

typedef struct {
	BOOL bShowed;
	char *sHintText;
} HintEntry_Type;

HintEntry_Type HintTable[]={
	{FALSE, "Automatic configuration hints enabled. Uncheck SHOWHINTS to turn off"},
	{FALSE, "DirectDraw dependency detected. Check DirectX flags"},
	{FALSE, "Direct3D8 dependency detected. Check DirectX8 flags"},
	{FALSE, "Direct3D9 dependency detected. Check DirectX9 flags"},
	{FALSE, "Direct3D10 dependency detected. Check DirectX10 flags"},
	{FALSE, "Direct3D11 dependency detected. Check DirectX11 flags"},
	{FALSE, "OpenGL dependency detected. Check OpenGL hook and flags"},
	{FALSE, "DirectSound dependency detected. Check DirectSound hook in case of sound problems"},
	{FALSE, "DirectInput dependency detected. Check DirectInput hook in case of input problems"},
	{FALSE, "DirectInput8 dependency detected. Check DirectInput8 hook in case of input problems"},
	{FALSE, "Movie libraries detected. Check multimedia section"},
	{FALSE, "D3D libraries detected. Check DirectX version hook (DirectX7 ?)"},
	{FALSE, "IMAGEHLP dependency detected. In case of error, check NOIMAGEHLP flag"},
	{FALSE, "Compatibility settings for a dfferent OS detected. You may need to clear them"},
	{FALSE, "Executable obfuscation detected. Impossible to hook this file"},
	{FALSE, "SafeDisk dependency detected. Check for compatibility with current OS"},
	{FALSE, "SecuRom dependency detected. Check for compatibility with current OS"},
	{FALSE, ""}
};

void ShowHint(int HintId)
{
	// boundary checks
	if ((HintId < HINT_HINT) || (HintId >= HINT_LAST)) return;

	// show hints just once
	if(HintTable[HintId].bShowed) return; 
	HintTable[HintId].bShowed = TRUE;

	// show hint
	char sMessage[1024];
	strcpy(sMessage, HintTable[HintId].sHintText);
	strcat(sMessage, "\n\nShow next hints?");
	if(MessageBox(NULL, sMessage, "DxWnd hint", 
		MB_OKCANCEL|MB_ICONWARNING|MB_TOPMOST)==IDCANCEL)
		dxw.bHintActive = FALSE;
}
