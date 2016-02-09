#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include "dxwnd.h"
#include "dxwcore.hpp"
//#include "syslibs.h"
#include "dxhook.h"
#include "dxhelper.h"
#include <Vfw.h>

typedef VOID	(WINAPI *AVIFileInit_Type)(void);
typedef LONG	(WINAPI *AVIFileRelease_Type)(PAVIFILE);
typedef LONG	(WINAPI *AVIStreamRelease_Type)(PAVISTREAM);
typedef PGETFRAME (WINAPI *AVIStreamGetFrameOpen_Type)(PAVISTREAM, LPBITMAPINFOHEADER);
typedef HRESULT (WINAPI *AVIFileOpenA_Type)(PAVIFILE *, LPCTSTR, UINT, LPCLSID); 
typedef HRESULT (WINAPI *AVIFileOpenW_Type)(PAVIFILE *, LPCWSTR, UINT, LPCLSID); 
typedef HRESULT (WINAPI *AVIFileGetStream_Type)(PAVIFILE, PAVISTREAM *, DWORD, LONG);

AVIFileInit_Type pAVIFileInit = NULL;
AVIFileRelease_Type pAVIFileRelease = NULL;
AVIStreamRelease_Type pAVIStreamRelease = NULL;
AVIStreamGetFrameOpen_Type pAVIStreamGetFrameOpen = NULL;
AVIFileOpenA_Type pAVIFileOpenA = NULL;
AVIFileOpenW_Type pAVIFileOpenW = NULL;
AVIFileGetStream_Type pAVIFileGetStream = NULL;

VOID WINAPI extAVIFileInit(void);
LONG WINAPI extAVIFileRelease(PAVIFILE);
LONG WINAPI extAVIStreamRelease(PAVISTREAM);
PGETFRAME WINAPI extAVIStreamGetFrameOpen(PAVISTREAM, LPBITMAPINFOHEADER);
HRESULT extAVIFileOpenA(PAVIFILE *, LPCTSTR, UINT, LPCLSID);
HRESULT extAVIFileOpenW(PAVIFILE *, LPCWSTR, UINT, LPCLSID);
HRESULT WINAPI extAVIFileGetStream(PAVIFILE, PAVISTREAM *, DWORD, LONG);

static HookEntryEx_Type Hooks[]={
	//{HOOK_IAT_CANDIDATE, 0, "AVIFileClose", NULL, (FARPROC *)&pAVIFileClose, (FARPROC)extAVIFileClose},
	{HOOK_IAT_CANDIDATE, 0, "AVIFileInit", NULL, (FARPROC *)&pAVIFileInit, (FARPROC)extAVIFileInit},
	// hooking AVIFileOpenA makes "Die Hard Trilogy" DIEHARD.EXE crash!
	//{HOOK_IAT_CANDIDATE, 0, "AVIFileOpenA", NULL, (FARPROC *)&pAVIFileOpenA, (FARPROC)extAVIFileOpenA}, 
	//{HOOK_IAT_CANDIDATE, 0, "AVIFileOpenW", NULL, (FARPROC *)&pAVIFileOpenW, (FARPROC)extAVIFileOpenW}, 
	{HOOK_IAT_CANDIDATE, 0, "AVIFileRelease", NULL, (FARPROC *)&pAVIFileRelease, (FARPROC)extAVIFileRelease},
	{HOOK_IAT_CANDIDATE, 0, "AVIStreamRelease", NULL, (FARPROC *)&pAVIStreamRelease, (FARPROC)extAVIStreamRelease},
	{HOOK_IAT_CANDIDATE, 0, "AVIStreamGetFrameOpen", NULL, (FARPROC *)&pAVIStreamGetFrameOpen, (FARPROC)extAVIStreamGetFrameOpen},
	{HOOK_IAT_CANDIDATE, 0, 0, NULL, 0, 0} // terminator
};


void HookAVIFil32(HMODULE module)
{
	HookLibraryEx(module, Hooks, "AVIFIL32.dll");
}

FARPROC Remap_AVIFil32_ProcAddress(LPCSTR proc, HMODULE hModule)
{
	FARPROC addr;
	if (addr=RemapLibraryEx(proc, hModule, Hooks)) return addr;
	return NULL;
}

static char *AviErr(HRESULT res)
{
	char *s;
	switch (res){
		case AVIERR_BADFORMAT:		s="AVIERR_BADFORMAT"; break;
		case AVIERR_MEMORY:			s="AVIERR_MEMORY"; break;
		case AVIERR_FILEREAD:		s="AVIERR_FILEREAD"; break;
		case AVIERR_FILEOPEN:		s="AVIERR_FILEOPEN"; break;
		case REGDB_E_CLASSNOTREG:	s="REGDB_E_CLASSNOTREG"; break;
		case AVIERR_NODATA:			s="AVIERR_NODATA"; break;
		case AVIERR_BUFFERTOOSMALL:	s="AVIERR_BUFFERTOOSMALL"; break;
		default:					s="unknown"; break;
	}
	return s;
}

// OF_CREATE			Creates a new file. If the file already exists, it is truncated to zero length.
// OF_PARSE				Skips time-consuming operations, such as building an index. Set this flag if you want the function to return as quickly as possible—for example, if you are going to query the file properties but not read the file.
// OF_READ				Opens the file for reading.
// OF_READWRITE			Opens the file for reading and writing.
// OF_SHARE_DENY_NONE	Opens the file nonexclusively. Other processes can open the file with read or write access. AVIFileOpen fails if another process has opened the file in compatibility mode.
// OF_SHARE_DENY_READ	Opens the file nonexclusively. Other processes can open the file with write access. AVIFileOpen fails if another process has opened the file in compatibility mode or has read access to it.
// OF_SHARE_DENY_WRITE	Opens the file nonexclusively. Other processes can open the file with read access. AVIFileOpen fails if another process has opened the file in compatibility mode or has write access to it.
// OF_SHARE_EXCLUSIVE	Opens the file and denies other processes any access to it. AVIFileOpen fails if any other process has opened the file.
// OF_WRITE				Opens the file for writing.

static char *AviMode(UINT c)
{
	static char eb[256];
	unsigned int l;
	strcpy(eb,"OF_");
	if (c & OF_CREATE) strcat(eb, "CREATE+");
	if (c & OF_PARSE) strcat(eb, "PARSE+");
	if (c & OF_READ) strcat(eb, "READ+");
	if (c & OF_READWRITE) strcat(eb, "READWRITE+");
	if (c & OF_SHARE_DENY_NONE) strcat(eb, "SHARE_DENY_NONE+"); 
	if ((c & OF_SHARE_DENY_READ) == OF_SHARE_DENY_READ) strcat(eb, "SHARE_DENY_READ+"); // 0x30, there are two bits ....
	if (c & OF_SHARE_DENY_WRITE) strcat(eb, "SHARE_DENY_WRITE+");
	if (c & OF_SHARE_EXCLUSIVE) strcat(eb, "SHARE_EXCLUSIVE+");
	if (c & OF_WRITE) strcat(eb, "WRITE+");
	l=strlen(eb);
	if (l>strlen("OF_")) eb[l-1]=0; // delete last '+' if any
	else eb[0]=0;
	return(eb);
}

VOID WINAPI extAVIFileInit(void)
{
	OutTraceDW("AVIFileInit\n");
	(*pAVIFileInit)();
}
 
HRESULT extAVIFileOpenA(PAVIFILE *ppfile, LPCSTR szFile, UINT mode, LPCLSID pclsid)
{
	HRESULT res;
	if(IsTraceDW){
		char sClassId[80];
		if(pclsid) sprintf_s(sClassId, 80, "%x.%x.%x.%x", pclsid->Data1, pclsid->Data2, pclsid->Data3, pclsid->Data4);
		else strcpy(sClassId, "(null)");
		OutTrace("AVIFileOpenA: file=\"%s\" mode=%x(%s) clsid=%s\n", szFile, mode, AviMode(mode), sClassId);
	}
	res = (*pAVIFileOpenA)(ppfile, szFile, mode, pclsid);
	if(res) {
		OutTraceDW("AVIFileOpenA ERROR: res=%x(%s)\n", res, AviErr(res));
	}
	else {
		OutTraceE("AVIFileOpenA: pfile=%x\n", *ppfile);
	}
	return res;
}

HRESULT extAVIFileOpenW(PAVIFILE *ppfile, LPCWSTR szFile, UINT mode, LPCLSID pclsid)
{
	HRESULT res;
	if(IsTraceDW){
		char sClassId[80];
		if(pclsid) sprintf_s(sClassId, 80, "%x.%x.%x.%x", pclsid->Data1, pclsid->Data2, pclsid->Data3, pclsid->Data4);
		else strcpy(sClassId, "(null)");
		OutTrace("AVIFileOpenW: file=\"%ls\" mode=%x(%s) clsid=%s\n", szFile, mode, AviMode(mode), sClassId);
	}
	return (*pAVIFileOpenW)(ppfile, szFile, mode, pclsid);
	res = (*pAVIFileOpenW)(ppfile, szFile, mode, pclsid);
	if(res) {
		OutTraceDW("AVIFileOpenW ERROR: res=%x(%s)\n", res, AviErr(res));
	}
	else {
		OutTraceE("AVIFileOpenW: pfile=%x\n", *ppfile);
	}
	return res;
}

HRESULT WINAPI extAVIFileGetStream(PAVIFILE pfile, PAVISTREAM *ppavi, DWORD fccType, LONG lParam)
{
	HRESULT res;
	OutTraceDW("AVIFileGetStream: pfile=%x fcctype=%x lparam=%x\n", pfile, fccType, lParam);
	res = (*pAVIFileGetStream)(pfile, ppavi, fccType, lParam);
	if(res) {
		OutTraceDW("AVIFileGetStream ERROR: res=%x(%s)\n", res, AviErr(res));
	}
	else {
		OutTraceE("AVIFileGetStream: pavi=%x\n", *ppavi);
	}
	return res;
}

LONG WINAPI extAVIFileRelease(PAVIFILE pavi)
{
	OutTraceDW("AVIFileRelease: pavi=%x\n", pavi);
	if(pavi==NULL) {
		// intercepting AVIFileRelease(NULL) avoids an exception in "Die Hard Trilogy" !!!
		OutTraceE("AVIFileRelease: pavi=NULL condition - returns OK\n");
		return 0;
	}
	return (*pAVIFileRelease)(pavi);
}

LONG WINAPI extAVIStreamRelease(PAVISTREAM pavi)
{
	OutTraceDW("AVIStreamRelease: pavi=%x\n", pavi);
	if(pavi==NULL) {
		OutTraceE("AVIFileRelease: pavi=NULL condition - returns OK\n");
		return 0;
	}
	return (*pAVIStreamRelease)(pavi);
}

PGETFRAME WINAPI extAVIStreamGetFrameOpen(PAVISTREAM pavi, LPBITMAPINFOHEADER lpbiWanted)
{
	if(IsTraceDW){
		char sColor[12];
		switch ((DWORD)lpbiWanted){
			case 0: strcpy(sColor, "DEFAULT"); break;
			case AVIGETFRAMEF_BESTDISPLAYFMT: strcpy(sColor, "BEST"); break;
			default: sprintf(sColor,"%d", lpbiWanted->biBitCount);
		}
		OutTrace("AVIStreamGetFrameOpen: pavi=%x lpbiwanted=%x bitcount=%s\n", pavi, lpbiWanted, sColor);
	}

	if((dxw.dwFlags6 & FIXMOVIESCOLOR) && (((DWORD)lpbiWanted == 0) || ((DWORD)lpbiWanted == AVIGETFRAMEF_BESTDISPLAYFMT))){
		BITMAPINFOHEADER biWanted;
		memset(&biWanted, 0, sizeof(BITMAPINFOHEADER));
		biWanted.biSize = sizeof(BITMAPINFOHEADER);
		biWanted.biBitCount = (WORD)dxw.VirtualPixelFormat.dwRGBBitCount;
		biWanted.biPlanes = 1;
		if(biWanted.biBitCount < 32) 
		biWanted.biClrUsed = (0x1 << biWanted.biBitCount); // 8 -> 256;
		else
			biWanted.biClrUsed = 0;
		biWanted.biClrImportant = biWanted.biClrUsed;
		return (*pAVIStreamGetFrameOpen)(pavi, &biWanted);
	}

	return (*pAVIStreamGetFrameOpen)(pavi, lpbiWanted);
}


