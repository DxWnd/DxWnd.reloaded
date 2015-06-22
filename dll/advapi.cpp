#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include "dxwnd.h"
#include "dxwcore.hpp"
#include "syslibs.h"
#include "dxhook.h"
#include "dxhelper.h"

typedef LONG (WINAPI *RegFlushKey_Type)(HKEY);
LONG WINAPI extRegFlushKey(HKEY);
RegFlushKey_Type pRegFlushKey = NULL;

static HookEntry_Type Hooks[]={
	{HOOK_IAT_CANDIDATE, "RegOpenKeyExA", NULL, (FARPROC *)&pRegOpenKeyEx, (FARPROC)extRegOpenKeyEx},
	{HOOK_IAT_CANDIDATE, "RegCloseKey", NULL, (FARPROC *)&pRegCloseKey, (FARPROC)extRegCloseKey},
	{HOOK_IAT_CANDIDATE, "RegQueryValueExA", NULL, (FARPROC *)&pRegQueryValueEx, (FARPROC)extRegQueryValueEx},
	{HOOK_IAT_CANDIDATE, "RegCreateKeyA", NULL, (FARPROC *)&pRegCreateKey, (FARPROC)extRegCreateKey},
	{HOOK_IAT_CANDIDATE, "RegCreateKeyExA", NULL, (FARPROC *)&pRegCreateKeyEx, (FARPROC)extRegCreateKeyEx},
	{HOOK_IAT_CANDIDATE, "RegSetValueExA", NULL, (FARPROC *)&pRegSetValueEx, (FARPROC)extRegSetValueEx},
	{HOOK_IAT_CANDIDATE, "RegFlushKey", NULL, (FARPROC *)&pRegFlushKey, (FARPROC)extRegFlushKey},
	{HOOK_IAT_CANDIDATE, 0, NULL, 0, 0} // terminator
};

void HookAdvApi32(HMODULE module)
{
	HookLibrary(module, Hooks, "ADVAPI32.dll");
}

FARPROC Remap_AdvApi32_ProcAddress(LPCSTR proc, HMODULE hModule)
{
	FARPROC addr;
	if (addr=RemapLibrary(proc, hModule, Hooks)) return addr;
	return NULL;
}

#define HKEY_FAKE ((HKEY)0x7FFFFFFF)
#define HKEY_MASK 0x7FFFFF00
#define IsFake(hKey) (((DWORD)hKey & HKEY_MASK) == HKEY_MASK)

static FILE *OpenFakeRegistry();
static char *hKey2String(HKEY);
static LONG myRegOpenKeyEx(HKEY, LPCTSTR, PHKEY);

static char *hKey2String(HKEY hKey)
{
	char *skey;
	static char sKey[MAX_PATH+1];
	static char skeybuf[10];
	if(IsFake(hKey)) {
		FILE *regf;
		char RegBuf[MAX_PATH+1];
		regf=OpenFakeRegistry();
		if(regf!=NULL){
			HKEY hLocalKey=HKEY_FAKE;
			fgets(RegBuf, 256, regf);
			while (!feof(regf)){
				if(RegBuf[0]=='['){
					if(hLocalKey == hKey){
						//OutTrace("building fake Key=\"%s\" hKey=%x\n", sKey, hKey);
						fclose(regf);
						strcpy(sKey, &RegBuf[1]);
						sKey[strlen(sKey)-2]=0; // get rid of "]"
						return sKey;
					}
					else {
						hLocalKey--;
					}
				}
				fgets(RegBuf, 256, regf);
			}
			fclose(regf);
		}
		return "HKEY_NOT_FOUND";	
	}
	switch((ULONG)hKey){
		case HKEY_CLASSES_ROOT:		skey="HKEY_CLASSES_ROOT"; break;
        case HKEY_CURRENT_CONFIG:	skey="HKEY_CURRENT_CONFIG"; break;
        case HKEY_CURRENT_USER:		skey="HKEY_CURRENT_USER"; break;
        case HKEY_LOCAL_MACHINE:	skey="HKEY_LOCAL_MACHINE"; break;
        case HKEY_USERS:			skey="HKEY_USERS"; break;
		default:					sprintf(skeybuf, "0x%x", hKey); skey=skeybuf; break;
	}
	return skey;
}

static FILE *OpenFakeRegistry()
{
	DWORD dwAttrib;	
	char sSourcePath[MAX_PATH+1];
	char *p;
	dwAttrib = GetFileAttributes("dxwnd.dll");
	if (dwAttrib != INVALID_FILE_ATTRIBUTES && !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY)) return NULL;
	GetModuleFileName(GetModuleHandle("dxwnd"), sSourcePath, MAX_PATH);
	p=&sSourcePath[strlen(sSourcePath)-strlen("dxwnd.dll")];
	strcpy(p, "dxwnd.reg");
	return fopen(sSourcePath,"r");
}

// ---------------------------------------------------------------------------------

static LONG myRegOpenKeyEx(
				HKEY hKey,
				LPCTSTR lpSubKey,
				PHKEY phkResult)
{
	FILE *regf;
	char sKey[MAX_PATH+1];
	char RegBuf[MAX_PATH+1];

	sprintf(sKey,"%s\\%s", hKey2String(hKey), lpSubKey);
	OutTraceR("RegOpenKeyEx: searching for key=\"%s\"\n", sKey);

	regf=OpenFakeRegistry();
	if(regf!=NULL){
		if(phkResult) *phkResult=HKEY_FAKE;
		fgets(RegBuf, 256, regf);
		while (!feof(regf)){
			if(RegBuf[0]=='['){
				// beware: registry keys are case insensitive. Must use _strnicmp instead of strncmp
				if((!_strnicmp(&RegBuf[1],sKey,strlen(sKey))) && (RegBuf[strlen(sKey)+1]==']')){
					OutTrace("RegOpenKeyEx: found fake Key=\"%s\" hkResult=%x\n", sKey, phkResult ? *phkResult : 0);
					fclose(regf);
					return ERROR_SUCCESS;
				}
				else {
					if(phkResult) (*phkResult)--;
				}
			}
			fgets(RegBuf, 256, regf);
		}
		fclose(regf);
	}
	return ERROR_FILE_NOT_FOUND;
}


LONG WINAPI extRegOpenKeyEx(
				HKEY hKey,
				LPCTSTR lpSubKey,
				DWORD ulOptions,
				REGSAM samDesired,
				PHKEY phkResult)
{
	LONG res;

	OutTraceR("RegOpenKeyEx: hKey=%x(%s) SubKey=\"%s\" Options=%x\n", 
		hKey, hKey2String(hKey), lpSubKey, ulOptions);

	if(dxw.dwFlags4 & OVERRIDEREGISTRY){
		res = myRegOpenKeyEx(hKey, lpSubKey, phkResult);
		if(res == ERROR_SUCCESS) return res;
	}

	if(dxw.dwFlags6 & WOW64REGISTRY) ulOptions |= KEY_WOW64_64KEY;

	res=(*pRegOpenKeyEx)(hKey, lpSubKey, ulOptions, samDesired, phkResult);
	OutTraceR("RegOpenKeyEx: res=%x phkResult=%x\n", res, phkResult ? *phkResult : 0); 

	if((res==ERROR_SUCCESS) || !(dxw.dwFlags3 & EMULATEREGISTRY) || (dxw.dwFlags4 & OVERRIDEREGISTRY)) return res;
	
	return myRegOpenKeyEx(hKey, lpSubKey, phkResult);
}

LONG WINAPI extRegQueryValueEx(
				HKEY hKey, 
				LPCTSTR lpValueName, 
				LPDWORD lpReserved, 
				LPDWORD lpType, // beware: could be NULL
				LPBYTE lpData,  // beware: could be NULL
				LPDWORD lpcbData)
{
	LONG res;

	OutTraceR("RegQueryValueEx: hKey=%x(\"%s\") ValueName=\"%s\" Reserved=%x lpType=%x lpData=%x lpcbData=%x\n", 
		hKey, hKey2String(hKey), lpValueName, lpReserved, lpType, lpData, lpcbData);
	if (!IsFake(hKey)){
		res=(*pRegQueryValueEx)(hKey, lpValueName, lpReserved, lpType, lpData, lpcbData);
		if(IsTraceR){
			if (res==ERROR_SUCCESS){
				OutTrace("RegQueryValueEx: size=%d type=%x(%s) ", 
					lpcbData?*lpcbData:0, lpType?*lpType:0, lpType?ExplainRegType(*lpType):"none");
				if(lpType && lpData) switch(*lpType){
					case REG_SZ: OutTrace("Data=\"%s\"\n", lpData); break; 
					case REG_DWORD: OutTrace("Data=0x%x\n", *(DWORD *)lpData); break;
					case REG_BINARY:
						{
							DWORD i; 
							unsigned char *p;
							p = lpData;
							OutTrace("Data=%02.2X", p++);
							for(i=1; i<*lpcbData; i++) OutTrace(",%02.2X", *p++);
							OutTrace("\n");
						}
						break;
					default: OutTrace("Data=???\n"); break;
				}
				else 
					OutTrace("\n");
			}
			else
				OutTrace("res=%x\n", res);
		}
		return res;
	}

	// try emulated registry
	res = ERROR_FILE_NOT_FOUND;
	FILE *regf;
	char RegBuf[MAX_PATH+1];
	char *pData;
	HKEY hCurKey=HKEY_FAKE+1;
	DWORD cbData=0;
	regf=OpenFakeRegistry();
	if(regf==NULL) return res;
	if(!lpValueName)lpValueName="";
	fgets(RegBuf, 256, regf);
	while (!feof(regf)){
		if(RegBuf[0]=='['){
			hCurKey--;
		}
		else {
			if(hCurKey==hKey){
				if((RegBuf[0]=='"') &&
					!_strnicmp(lpValueName, &RegBuf[1], strlen(lpValueName)) &&
					(RegBuf[strlen(lpValueName)+1]=='"') &&
					(RegBuf[strlen(lpValueName)+2]=='='))
				{
					LPBYTE lpb;
					res=ERROR_FILE_NOT_FOUND;
					pData=&RegBuf[strlen(lpValueName)+3];
					lpb = lpData;
					if(lpcbData) {
						cbData = *lpcbData;
						*lpcbData=0;
					}
					if(*pData=='"'){ // string value
						if(lpType) *lpType=REG_SZ;
						pData++;
						while(*pData && (*pData != '"')){
							if(*pData=='\\') pData++;
							if(lpData && lpcbData) if(*lpcbData < cbData) *lpb++=*pData;
							pData++;
							if(lpcbData) (*lpcbData)++;
						}
						if(lpcbData) (*lpcbData)++; // extra space for string terminator ?
						if(lpData && lpcbData) if(*lpcbData < cbData) *lpb = 0; // string terminator
						OutTraceR("RegQueryValueEx: type=REG_SZ cbData=%x Data=\"%s\"\n", 
							lpcbData ? *lpcbData : 0, lpData ? (char *)lpData : "(NULL)");
						res=(*lpcbData > cbData) ? ERROR_MORE_DATA : ERROR_SUCCESS;
						break;
					}
					if(!strncmp(pData,"dword:",strlen("dword:"))){ //dword value
						DWORD val;
						if(lpType) *lpType=REG_DWORD;
						pData+=strlen("dword:");
						sscanf(pData, "%x", &val);
						if(lpData) {
							if (cbData >= sizeof(DWORD)) {
								memcpy(lpData, &val, sizeof(DWORD));
								res=ERROR_SUCCESS;
							}
							else
								res=ERROR_MORE_DATA;
						}
						if (lpcbData) *lpcbData=sizeof(DWORD);
						OutTraceR("RegQueryValueEx: type=REG_DWORD cbData=%x Data=0x%x\n", 
							lpcbData ? *lpcbData : 0, val);
						break;
					}
					if(!strncmp(pData,"hex:",strlen("hex:"))){ //hex value
						BYTE *p;
						if(lpType) *lpType=REG_BINARY;
						p = (BYTE *)pData;
						p+=strlen("hex:");
						while(TRUE){
							p[strlen((char *)p)-1]=0; // eliminates \n at the end of line
							while(strlen((char *)p)>1){
								if((*lpcbData < cbData) && lpData){
									sscanf((char *)p, "%x,", (char *)lpb);
									lpb++;
								}
								p+=3;
								if(lpcbData) (*lpcbData)++;
							}
							if(p[strlen((char *)p)-1]=='\\'){
								fgets(RegBuf, 256, regf);
								pData = RegBuf;
								p = (BYTE *)pData;
							}
							else break;
						}
						OutTraceR("RegQueryValueEx: type=REG_BINARY cbData=%d Data=%s\n", 
							lpcbData ? *lpcbData : 0, 
							lpData ? "(NULL)" : hexdump(lpData, *lpcbData));
						res=(*lpcbData > cbData) ? ERROR_MORE_DATA : ERROR_SUCCESS;
						break;
					}
				}
			}
		}
		fgets(RegBuf, 256, regf);
	}
	fclose(regf);
	OutTraceR("RegQueryValueEx: res=%x\n", res);
	return res;
}

LONG WINAPI extRegCloseKey(HKEY hKey)
{
	OutTraceR("RegCloseKey: hKey=%x\n", hKey);
	if (IsFake(hKey)) return ERROR_SUCCESS;
	return (*pRegCloseKey)(hKey);
}

LONG WINAPI extRegFlushKey(HKEY hKey)
{
	OutTraceR("RegFlushKey: hKey=%x\n", hKey);
	if (IsFake(hKey)) return ERROR_SUCCESS;
	return (*pRegFlushKey)(hKey);
}

LONG WINAPI extRegSetValueEx(HKEY hKey, LPCTSTR lpValueName, DWORD Reserved, DWORD dwType, const BYTE *lpData, DWORD cbData)
{
	if (IsTraceR){
		char sInfo[1024];
		sprintf(sInfo, "RegSetValueEx: hKey=%x ValueName=\"%s\" Type=%x(%s) cbData=%d", hKey, lpValueName, dwType, ExplainRegType(dwType), cbData);
		switch(dwType){
			case REG_DWORD: OutTrace("%s Data=%x\n", sInfo, *(DWORD *)lpData); break;
			case REG_NONE: OutTrace("%s Data=\"%s\"\n", sInfo, lpData); break;
			case REG_BINARY: OutTrace("%s Data=%s\n", sInfo, hexdump((BYTE *)lpData, cbData)); break;
			case REG_SZ: OutTrace("%s Data=\"%*.*s\"\n", sInfo, cbData-1, cbData-1, lpData); break;
			default: OutTrace("%s\n", sInfo);
		}
	}
	if(IsFake(hKey) && (dxw.dwFlags3 & EMULATEREGISTRY)) {
		OutTraceR("RegSetValueEx: SUPPRESS registry key set\n");
		return ERROR_SUCCESS;
	}
	return (*pRegSetValueEx)(hKey, lpValueName, Reserved, dwType, lpData, cbData);
}

LONG WINAPI extRegCreateKeyEx(HKEY hKey, LPCTSTR lpSubKey, DWORD Reserved, LPTSTR lpClass, DWORD dwOptions, REGSAM samDesired,
				LPSECURITY_ATTRIBUTES lpSecurityAttributes, PHKEY phkResult, LPDWORD lpdwDisposition)
{
	OutTraceR("RegCreateKeyEx: hKey=%x(%s) SubKey=\"%s\" Class=%x\n", hKey, hKey2String(hKey), lpSubKey, lpClass);
	if (dxw.dwFlags3 & EMULATEREGISTRY){
		*phkResult = HKEY_FAKE;
        // V2.3.12: return existing fake key if any ....
        if(dxw.dwFlags4 & OVERRIDEREGISTRY) myRegOpenKeyEx(hKey, lpSubKey, phkResult);
		if(lpdwDisposition) *lpdwDisposition=REG_OPENED_EXISTING_KEY;
		return ERROR_SUCCESS;
	}
	else
		return (*pRegCreateKeyEx)(hKey, lpSubKey, Reserved, lpClass, dwOptions, samDesired,
				lpSecurityAttributes, phkResult, lpdwDisposition);
}

LONG WINAPI extRegCreateKey(HKEY hKey, LPCTSTR lpSubKey, PHKEY phkResult)
{
	OutTraceR("RegCreateKey: hKey=%x(%s) SubKey=\"%s\"\n", hKey, hKey2String(hKey), lpSubKey);
	if (dxw.dwFlags3 & EMULATEREGISTRY){
		*phkResult = HKEY_FAKE;
        // V2.3.12: return existing fake key if any ....
        if(dxw.dwFlags4 & OVERRIDEREGISTRY) myRegOpenKeyEx(hKey, lpSubKey, phkResult);
		return ERROR_SUCCESS;
	}
	else
		return (*pRegCreateKey)(hKey, lpSubKey, phkResult);
}
