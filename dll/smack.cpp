#define  _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include "dxwnd.h"
#include "dxwcore.hpp"
#include "syslibs.h"
#include "dxhook.h"

#include "smack.h"

typedef Smack * (WINAPI *SmackOpen_Type)(HANDLE, UINT32, INT32);
typedef Smack * (WINAPI *Smacker_Type)(Smack *);
typedef Smack * (WINAPI *Smacker2_Type)(Smack *, UINT32);
typedef Smack * (WINAPI *SmackColorRemap_Type)(Smack *, const void PTR4 *, u32, u32);
typedef Smack * (WINAPI *SmackColorRemapWithTrans_Type)(Smack *, const void PTR4 *, u32, u32, u32);
typedef u32		(RADEXPLINK *SmackSetSystemRes_Type)(u32);

SmackOpen_Type pSmackOpen;
Smacker_Type pSmackClose, pSmackWait, pSmackDoFrame, pSmackNextFrame, pSmackSoundUseMSS, pSmackSoundUseDirectSound;
Smacker2_Type pSmackSoundOnOff, pSmackGoto;
SmackColorRemap_Type pSmackColorRemap;
SmackColorRemapWithTrans_Type pSmackColorRemapWithTrans;
SmackSetSystemRes_Type pSmackSetSystemRes;

Smack * WINAPI extSmackOpen(HANDLE, UINT32, INT32);
Smack * WINAPI extSmackClose(Smack *);
Smack * WINAPI extSmackWait(Smack *);
Smack * WINAPI extSmackDoFrame(Smack *);
Smack * WINAPI extSmackNextFrame(Smack *);
Smack * WINAPI extSmackSoundUseMSS(Smack *);
Smack * WINAPI extSmackSoundUseDirectSound(Smack *);
Smack * WINAPI extSmackSoundOnOff(Smack *, UINT32);
Smack * WINAPI extSmackGoto(Smack *, UINT32);
Smack * WINAPI extSmackColorRemap(Smack *, const void PTR4 *, u32, u32);
Smack * WINAPI extSmackColorRemapWithTrans(Smack *, const void PTR4 *, u32, u32, u32);
u32 RADEXPLINK extSmackSetSystemRes(u32 mode);

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
	{HOOK_IAT_CANDIDATE, 0x0000, "_SmackSetSystemRes@4", (FARPROC)NULL, (FARPROC *)&pSmackSetSystemRes, (FARPROC)extSmackSetSystemRes},
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
	Smack *h;
	OutTraceDW("SmackOpen: SmackFile=%x flags=%x(%s) extrabuf=%x\n", SmackFile, flags, ExplainSmackFlags(flags), extrabuf);
	if(!pSmackOpen) OutTraceE("ASSERT: NULL pSmackOpen\n");
	h=(*pSmackOpen)(SmackFile, flags, extrabuf);
	OutTraceDW("SmackOpen: ret=%x\n", h);
	if (!h) return NULL;
		
	DumpSmack(h);
	if(dxw.dwFlags6 & NOMOVIES) {
		OutTraceDW("SmackOpen: NOMOVIES\n");
		h->Frames = 1; // returning NULL or a frame counter below 1 is risky!
		return h;
	}
	return h;
}

Smack * WINAPI extSmackClose(Smack *h)
{
	OutTraceDW("SmackClose: h=%x\n", h);
	return (*pSmackClose)(h);
}

Smack * WINAPI extSmackWait(Smack *h)
{
	OutTraceDW("SmackWait: h=%x\n", h);
	DumpSmack((Smack *)h);
	return (*pSmackWait)(h);
}

Smack * WINAPI extSmackDoFrame(Smack *h)
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

Smack * WINAPI extSmackNextFrame(Smack *h)
{
	OutTraceDW("SmackNextFrame: h=%x\n", h);
	DumpSmack(h);
	return (*pSmackNextFrame)(h);
}

Smack * WINAPI extSmackSoundUseMSS(Smack *h)
{
	OutTraceDW("SmackSoundUseMSS: h=%x\n", h);
	DumpSmack(h);
	return (*pSmackSoundUseMSS)(h);
}

Smack * WINAPI extSmackSoundUseDirectSound(Smack *h)
{
	OutTraceDW("SmackSoundUseDirectSound: h=%x\n", h);
	DumpSmack(h);
	return (*pSmackSoundUseDirectSound)(h);
}

Smack * WINAPI extSmackSoundOnOff(Smack *h, UINT32 flag)
{
	OutTraceDW("SmackSoundOnOff: h=%x flag=%x\n", h, flag);
	DumpSmack(h);
	return (*pSmackSoundOnOff)(h, flag);
}

Smack * WINAPI extSmackGoto(Smack *h, UINT32 flag)
{
	OutTraceDW("SmackGoto: h=%x flag=%x\n", h, flag);
	DumpSmack(h);
	return (*pSmackGoto)(h, flag);
}

Smack * WINAPI extSmackColorRemap(Smack *h, const void PTR4 *remappal, u32 numcolors, u32 paltype)
{
	OutTraceDW("SmackColorRemap: h=%x numcolors=%d paltype=%d\n", h, numcolors, paltype);
	DumpSmack(h);
	// BYPASS the call to avoid resolution changes
	//return (*pSmackColorRemap)(h, remappal, numcolors, paltype);
	return h;
}

Smack * WINAPI extSmackColorRemapWithTrans(Smack *h, const void PTR4 *remappal, u32 numcolors, u32 paltype, u32 transindex)
{
	OutTraceDW("SmackColorRemapWithTrans: h=%x numcolors=%d paltype=%d transindex=%d\n", h, numcolors, paltype, transindex);
	DumpSmack(h);
	// BYPASS the call to avoid resolution changes
	//return (*pSmackColorRemapWithTrans)(h, remappal, numcolors, paltype);
	return h;
}

/* ---------------------------------------------------------------
#define SMACKRESRESET    0
#define SMACKRES640X400  1
#define SMACKRES640X480  2
#define SMACKRES800X600  3
#define SMACKRES1024X768 4

RADEXPFUNC u32 RADEXPLINK SmackSetSystemRes(u32 mode);  // use SMACKRES* values

#define SMACKNOCUSTOMBLIT        128
#define SMACKSMOOTHBLIT          256
#define SMACKINTERLACEBLIT 512
/* ------------------------------------------------------------ */

u32 RADEXPLINK extSmackSetSystemRes(u32 mode)
{
	char *modes[5]={"reset", "640X400", "640X480", "800X600", "1024X768"};
	int width, height;
	static int prevwidth, prevheight;
	OutTraceDW("SmackSetSystemRes: mode=%x(%s)\n", mode, modes[mode % 5]);
	// BYPASS the call to avoid resolution changes
	//return (*pSmackSetSystemRes)(h);
	mode = mode % 5;
	// save previous screen resolution for later mode=0 usage
	if(mode){
		prevwidth=dxw.GetScreenWidth();
		prevheight=dxw.GetScreenHeight();
		dxw.SetFullScreen(TRUE);
	}
	switch(mode){
		case 0: width=prevwidth; height=prevheight; break;
		case 1: width=640; height=400; break;
		case 2: width=640; height=480; break;
		case 3: width=800; height=600; break;
		case 4: width=1024; height=768; break;
	}
	dxw.SetScreenSize(width, height);
	if(dxw.Windowize && dxw.GethWnd()) AdjustWindowFrame(dxw.GethWnd(), width, height);
	return 0;
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

