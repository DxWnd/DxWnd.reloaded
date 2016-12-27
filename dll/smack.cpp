#define  _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include "dxwnd.h"
#include "dxwcore.hpp"
#include "syslibs.h"
#include "dxhook.h"

#if 0
#pragma pack(1)
typedef struct {
  UINT32 Version;
  UINT32 Width;
  UINT32 Height;
  UINT32 Frame;
  UINT32 mspf;
  char unknown[864];
  UINT32 FrameNum;
} Smack;
#else
#include "smack.h"
#endif

typedef Smack * (WINAPI *SmackOpen_Type)(HANDLE, UINT32, INT32);
typedef Smack * (WINAPI *Smacker_Type)(HANDLE);
typedef Smack * (WINAPI *Smacker2_Type)(HANDLE, UINT32);
typedef Smack * (WINAPI *SmackColorRemap_Type)(HANDLE, const void PTR4 *, u32, u32);
typedef Smack * (WINAPI *SmackColorRemapWithTrans_Type)(HANDLE, const void PTR4 *, u32, u32, u32);

SmackOpen_Type pSmackOpen;
Smacker_Type pSmackClose, pSmackWait, pSmackDoFrame, pSmackNextFrame, pSmackSoundUseMSS, pSmackSoundUseDirectSound;
Smacker2_Type pSmackSoundOnOff, pSmackGoto;
SmackColorRemap_Type pSmackColorRemap;
SmackColorRemapWithTrans_Type pSmackColorRemapWithTrans;

Smack * WINAPI extSmackOpen(HANDLE, UINT32, INT32);
Smack * WINAPI extSmackClose(HANDLE);
Smack * WINAPI extSmackWait(HANDLE);
Smack * WINAPI extSmackDoFrame(HANDLE);
Smack * WINAPI extSmackNextFrame(HANDLE);
Smack * WINAPI extSmackSoundUseMSS(HANDLE);
Smack * WINAPI extSmackSoundUseDirectSound(HANDLE);
Smack * WINAPI extSmackSoundOnOff(HANDLE, UINT32);
Smack * WINAPI extSmackGoto(HANDLE, UINT32);
Smack * WINAPI extSmackColorRemap(HANDLE, const void PTR4 *, u32, u32);
Smack * WINAPI extSmackColorRemapWithTrans(HANDLE, const void PTR4 *, u32, u32, u32);

static HookEntryEx_Type Hooks[]={
	{HOOK_IAT_CANDIDATE, 0x000E, "_SmackOpen@12", (FARPROC)NULL, (FARPROC *)&pSmackOpen, (FARPROC)extSmackOpen},
	{HOOK_IAT_CANDIDATE, 0x0012, "_SmackClose@4", (FARPROC)NULL, (FARPROC *)&pSmackClose, (FARPROC)extSmackClose},
	//{HOOK_IAT_CANDIDATE, 0x0020, "_SmackWait@4", (FARPROC)NULL, (FARPROC *)&pSmackWait, (FARPROC)extSmackWait},
	{HOOK_IAT_CANDIDATE, 0x0013, "_SmackDoFrame@4", (FARPROC)NULL, (FARPROC *)&pSmackDoFrame, (FARPROC)extSmackDoFrame},
	{HOOK_IAT_CANDIDATE, 0x0021, "_SmackSoundUseMSS@4", (FARPROC)NULL, (FARPROC *)&pSmackSoundUseMSS, (FARPROC)extSmackSoundUseMSS},
	{HOOK_IAT_CANDIDATE, 0x0011, "_SmackSoundOnOff@8", (FARPROC)NULL, (FARPROC *)&pSmackSoundOnOff, (FARPROC)extSmackSoundOnOff},
	{HOOK_IAT_CANDIDATE, 0x001B, "_SmackGoto@8", (FARPROC)NULL, (FARPROC *)&pSmackGoto, (FARPROC)extSmackGoto},
	{HOOK_IAT_CANDIDATE, 0x0015, "_SmackNextFrame@4", (FARPROC)NULL, (FARPROC *)&pSmackNextFrame, (FARPROC)extSmackNextFrame},
	{HOOK_IAT_CANDIDATE, 0x0026, "_SmackSoundUseDirectSound@4", (FARPROC)NULL, (FARPROC *)&pSmackSoundUseDirectSound, (FARPROC)extSmackSoundUseDirectSound},
	{HOOK_IAT_CANDIDATE, 0x0000, "_SmackColorRemap@16", (FARPROC)NULL, (FARPROC *)&pSmackColorRemap, (FARPROC)extSmackColorRemap},
	{HOOK_IAT_CANDIDATE, 0x0000, "_SmackColorRemapWithTrans@20", (FARPROC)NULL, (FARPROC *)&pSmackColorRemapWithTrans, (FARPROC)extSmackColorRemapWithTrans},
	{HOOK_IAT_CANDIDATE, 0, 0, NULL, 0, 0} // terminator
};

FARPROC Remap_smack_ProcAddress(LPCSTR proc, HMODULE hModule)
{
	FARPROC addr;
	if(!(dxw.dwFlags7 & HOOKSMACKW32)) return NULL;
	if (addr=RemapLibraryEx(proc, hModule, Hooks)) return addr;
	// NULL -> keep the original call address
	return NULL;
}

static char *libname = "smackw32.dll";

void HookSmackW32(HMODULE hModule)
{
	if(!(dxw.dwFlags7 & HOOKSMACKW32)) return;
	HookLibraryEx(hModule, Hooks, libname);
	return;
}

char *ExplainSmackFlags(DWORD c)
{
	static char eb[256];
	unsigned int l;
	strcpy(eb,"SMACK");
	if (c & SMACKNEEDPAN) strcat(eb, "NEEDPAN+");
	if (c & SMACKNEEDVOLUME) strcat(eb, "NEEDVOLUME+");
	if (c & SMACKFRAMERATE) strcat(eb, "FRAMERATE+");
	if (c & SMACKLOADEXTRA) strcat(eb, "LOADEXTRA+");
	if (c & SMACKPRELOADALL) strcat(eb, "PRELOADALL+");
	if (c & SMACKNOSKIP) strcat(eb, "NOSKIP+");
	if (c & SMACKSIMULATE) strcat(eb, "SIMULATE+");
	if (c & SMACKTRACK1) strcat(eb, "TRACK1+");
	if (c & SMACKTRACK2) strcat(eb, "TRACK2+");
	if (c & SMACKTRACK3) strcat(eb, "TRACK3+");
	if (c & SMACKTRACK4) strcat(eb, "TRACK4+");
	if (c & SMACKTRACK5) strcat(eb, "TRACK5+");
	if (c & SMACKTRACK6) strcat(eb, "TRACK6+");
	if (c & SMACKTRACK7) strcat(eb, "TRACK7+");
	if (c & SMACKBUFFERREVERSED) strcat(eb, "BUFFERREVERSED+");
	if (c & SMACKFILEISSMK) strcat(eb, "FILEISSMK+");
	if ((c & SMACKBUFFER16) == SMACKBUFFER555) strcat(eb, "BUFFER555+");
	if ((c & SMACKBUFFER16) == SMACKBUFFER565) strcat(eb, "BUFFER565+");
	if ((c & SMACKYNONE) == SMACKYNONE) strcat(eb, "YNONE+");
	if ((c & SMACKYNONE) == SMACKYDOUBLE) strcat(eb, "YDOUBLE+");
	if ((c & SMACKYNONE) == SMACKYINTERLACE) strcat(eb, "YINTERLACE+");
	l=strlen(eb);
	if (l>strlen("SMACK")) eb[l-1]=0; // delete last '+' if any
	else eb[0]=0;
	return(eb);
}

static void DumpSmack(Smack *s)
{
	if(!IsDebug) return;
	OutTrace("Smack_%x: size=(%dx%d) frame=%d/%d rate=%d type=%x palette=%x rect=(%d,%d)(%d-%d) offset=(%d,%d) err=%x\n",
		s, s->Width, s->Height, s->FrameNum, s->Frames, s->MSPerFrame, s->SmackerType, s->NewPalette,
		s->LastRectx, s->LastRecty, s->LastRectw, s->LastRecth, s->LeftOfs, s->TopOfs, s->ReadError);
}

Smack * WINAPI extSmackOpen(HANDLE SmackFile, UINT32 flags, INT32 extrabuf)
{
	Smack *ret;
	OutTraceDW("SmackOpen: SmackFile=%x flags=%x(%s) extrabuf=%x\n", SmackFile, flags, ExplainSmackFlags(flags), extrabuf);
	if(!pSmackOpen) OutTraceE("ASSERT: NULL pSmackOpen\n");
	ret=(*pSmackOpen)(SmackFile, flags, extrabuf);
	OutTraceDW("SmackOpen: ret=%x\n", ret);
	if (ret) {
		OutTraceDW("SmackOpen: version=\"%4.4s\" screen=(%dx%d) frame_count=%d frame_number=%d\n", 
			(char *)&(ret->Version), ret->Width, ret->Height, ret->Frames, ret->FrameNum);
	}
	return ret;
}

Smack * WINAPI extSmackClose(HANDLE h)
{
	OutTraceDW("SmackClose: h=%x\n", h);
	return (*pSmackClose)(h);
}

Smack * WINAPI extSmackWait(HANDLE h)
{
	OutTraceDW("SmackWait: h=%x\n", h);
	DumpSmack((Smack *)h);
	return (*pSmackWait)(h);
}

Smack * WINAPI extSmackDoFrame(HANDLE h)
{
	Smack *ret;
	OutTraceDW("SmackDoFrame: h=%x\n", h);
	DumpSmack((Smack *)h);
	ret = (*pSmackDoFrame)(h);
	if(dxw.dwFlags7 & FIXSMACKLOOP){
		dxw.MessagePump();
		dxw.ScreenRefresh();
	}
	return ret;
}

Smack * WINAPI extSmackNextFrame(HANDLE h)
{
	OutTraceDW("SmackNextFrame: h=%x\n", h);
	DumpSmack((Smack *)h);
	return (*pSmackNextFrame)(h);
}

Smack * WINAPI extSmackSoundUseMSS(HANDLE h)
{
	OutTraceDW("SmackSoundUseMSS: h=%x\n", h);
	DumpSmack((Smack *)h);
	return (*pSmackSoundUseMSS)(h);
}

Smack * WINAPI extSmackSoundUseDirectSound(HANDLE h)
{
	OutTraceDW("SmackSoundUseDirectSound: h=%x\n", h);
	DumpSmack((Smack *)h);
	return (*pSmackSoundUseDirectSound)(h);
}

Smack * WINAPI extSmackSoundOnOff(HANDLE h, UINT32 flag)
{
	OutTraceDW("SmackSoundOnOff: h=%x flag=%x\n", h, flag);
	DumpSmack((Smack *)h);
	return (*pSmackSoundOnOff)(h, flag);
}

Smack * WINAPI extSmackGoto(HANDLE h, UINT32 flag)
{
	OutTraceDW("SmackGoto: h=%x flag=%x\n", h, flag);
	DumpSmack((Smack *)h);
	return (*pSmackGoto)(h, flag);
}

Smack * WINAPI extSmackColorRemap(HANDLE h, const void PTR4 *remappal, u32 numcolors, u32 paltype)
{
	OutTraceDW("SmackColorRemap: h=%x numcolors=%d paltype=%d\n", h, numcolors, paltype);
	DumpSmack((Smack *)h);
	// BYPASS the call to avoid resolution changes
	//return (*pSmackColorRemap)(h, remappal, numcolors, paltype);
	return (Smack *)h;
}

Smack * WINAPI extSmackColorRemapWithTrans(HANDLE h, const void PTR4 *remappal, u32 numcolors, u32 paltype, u32 transindex)
{
	OutTraceDW("SmackColorRemapWithTrans: h=%x numcolors=%d paltype=%d transindex=%d\n", h, numcolors, paltype, transindex);
	DumpSmack((Smack *)h);
	// BYPASS the call to avoid resolution changes
	//return (*pSmackColorRemapWithTrans)(h, remappal, numcolors, paltype);
	return (Smack *)h;
}

/* ---------------------------------------------------------------
Used by M2 Tank Platoon II:
Ordinal 0x0015 name _SmackNextFrame@4
Ordinal 0x000E name _SmackOpen@12
Ordinal 0x0026 name _SmackSoundUseDirectSound@4
Ordinal 0x0012 name _SmackClose@4
Ordinal 0x0013 name _SmackDoFrame@4
Ordinal 0x0017 name _SmackToBuffer@28
Ordinal 0x0020 name _SmackWait@4
/* ------------------------------------------------------------ */



