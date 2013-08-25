#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include "dxwnd.h"
#include "dxwcore.hpp"
#include "syslibs.h"
#include "dxhook.h"
#include "dxhelper.h"

static HookEntry_Type Hooks[]={
	{"RegOpenKeyExA", NULL, (FARPROC *)&pRegOpenKeyEx, (FARPROC)extRegOpenKeyEx},
	{"RegCloseKey", NULL, (FARPROC *)&pRegCloseKey, (FARPROC)extRegCloseKey},
	{"RegQueryValueExA", NULL, (FARPROC *)&pRegQueryValueEx, (FARPROC)extRegQueryValueEx},
	{"RegCreateKeyA", NULL, (FARPROC *)&pRegCreateKey, (FARPROC)extRegCreateKey},
	{"RegCreateKeyExA", NULL, (FARPROC *)&pRegCreateKeyEx, (FARPROC)extRegCreateKeyEx},
	{"RegSetValueExA", NULL, (FARPROC *)&pRegSetValueEx, (FARPROC)extRegSetValueEx},
	{0, NULL, 0, 0} // terminator
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

static char *hKey2String(HKEY hKey)
{
	char *skey;
	static char skeybuf[10];
	if(IsFake(hKey)) return "HKEY_FAKE";
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

// ---------------------------------------------------------------------------------

LONG WINAPI extRegOpenKeyEx(
				HKEY hKey,
				LPCTSTR lpSubKey,
				DWORD ulOptions,
				REGSAM samDesired,
				PHKEY phkResult)
{
	LONG res;
	char RegBuf[256+1];

	OutTraceR("RegOpenKeyEx: hKey=%x(%s) SubKey=\"%s\" Options=%x\n", 
		hKey, hKey2String(hKey), lpSubKey, ulOptions);
	res=(*pRegOpenKeyEx)(hKey, lpSubKey, ulOptions, samDesired, phkResult);
	OutTraceR("RegOpenKeyEx: res=%x\n", res); 

	if((res==ERROR_SUCCESS) || !(dxw.dwFlags3 & EMULATEREGISTRY)) return res;
	
	if(phkResult) *phkResult=HKEY_FAKE;
	FILE *regf;
	char sKey[256+1];
	sprintf(sKey,"%s\\%s", hKey2String(hKey), lpSubKey);
	OutTraceD("RegOpenKeyEx: searching for key=\"%s\"\n", sKey);
	regf=fopen("dxwnd.reg","r");
	if(regf==NULL) return ERROR_FILE_NOT_FOUND;
	fgets(RegBuf, 256, regf);
	while (!feof(regf)){
		if(RegBuf[0]=='['){
			if((!strncmp(&RegBuf[1],sKey,strlen(sKey))) && (RegBuf[strlen(sKey)+1]==']')){
				OutTrace("RegOpenKeyEx: found fake Key=\"%s\" hkResult=%x\n", sKey, *phkResult);
				fclose(regf);
				return ERROR_SUCCESS;
			}
			else (*phkResult)--;
		}
		fgets(RegBuf, 256, regf);
	}
	fclose(regf);
	return ERROR_FILE_NOT_FOUND;
}

LONG WINAPI extRegQueryValueEx(
				HKEY hKey, 
				LPCTSTR lpValueName, 
				LPDWORD lpReserved, 
				LPDWORD lpType, 
				LPBYTE lpData, 
				LPDWORD lpcbData)
{
	LONG res;

	OutTraceR("RegQueryValueEx: hKey=%x(%s) ValueName=\"%s\" Reserved=%x\n", hKey, hKey2String(hKey), lpValueName, lpReserved);
	if (!IsFake(hKey)){
		res=(*pRegQueryValueEx)(hKey, lpValueName, lpReserved, lpType, lpData, lpcbData);
		if(IsTraceR){
			if (res==ERROR_SUCCESS){
				OutTrace("RegQueryValueEx: size=%d type=%x(%s) ", 
					lpcbData?*lpcbData:0, lpType?*lpType:0, lpType?ExplainRegType(*lpType):"none");
				if(lpType) switch(*lpType){
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

	//if(!(dxw.dwFlags3 & EMULATEREGISTRY)) return res;

	// try emulated registry
	FILE *regf;
	char RegBuf[256+1];
	char *pData;
	HKEY hCurKey=HKEY_FAKE+1;
	regf=fopen("dxwnd.reg","r");
	if(regf==NULL) return ERROR_FILE_NOT_FOUND;
	if(!lpValueName)lpValueName="";
	fgets(RegBuf, 256, regf);
	while (!feof(regf)){
		if(RegBuf[0]=='['){
			hCurKey--;
		}
		else {
			if(hCurKey==hKey){

				//OutTraceD("loop: \"%s\"\n", RegBuf);
				if((RegBuf[0]=='"') &&
					!strncmp(lpValueName, &RegBuf[1], strlen(lpValueName)) &&
					(RegBuf[strlen(lpValueName)+1]=='"') &&
					(RegBuf[strlen(lpValueName)+2]=='='))
				{
					res=ERROR_FILE_NOT_FOUND;
					pData=&RegBuf[strlen(lpValueName)+3];
					if(*pData=='"'){ // string value
						//strcpy((char *)lpData, &RegBuf[strlen(lpValueName)+4]);
						//lpData[strlen((char *)lpData)-2]=0; // eliminates " and \n
						//if(lpType) *lpType=REG_SZ;
						//*lpcbData=strlen((char *)lpData)+1;
						*lpcbData=0;
						pData++;
						while(*pData && (*pData != '"')){
							if(*pData=='\\') pData++;
							*lpData++=*pData++;
							*lpcbData++;
						}
						if(lpType) *lpType=REG_SZ;
						//
						OutTraceD("RegQueryValueEx: Data=\"%s\" type=REG_SZ\n", lpData);
						res=ERROR_SUCCESS;
					}
					if(!strncmp(pData,"dword:",strlen("dword:"))){ //dword value
						DWORD val;
						pData+=strlen("dword:");
						sscanf(pData, "%x", &val);
						memcpy(lpData, &val, sizeof(DWORD));
						if(lpType) *lpType=REG_DWORD;
						*lpcbData=sizeof(DWORD);
						OutTraceD("RegQueryValueEx: Data=0x%x type=REG_DWORD\n", val);
						res=ERROR_SUCCESS;
					}
					if(!strncmp(pData,"hex:",strlen("hex:"))){ //dword value
						pData+=strlen("hex:");
						lpData[strlen((char *)lpData)-1]=0; // eliminates \n
						if(lpType) *lpType=REG_BINARY;
						*lpcbData=0;
						OutTraceD("RegQueryValueEx: Data=");
						while(strlen(pData)>1){
							sscanf(pData, "%x,", (char *)lpData);
							OutTraceD("%02.2x,", *(unsigned char *)lpData);
							pData+=3;
							lpData++;
							(*lpcbData)++;
						}
						OutTraceD(" type=REG_BINARY cbData=%d\n", *lpcbData);
						res=ERROR_SUCCESS;
					}
					fclose(regf);
					return res;
				}
			}
		}
		fgets(RegBuf, 256, regf);
	}
	fclose(regf);
	return ERROR_FILE_NOT_FOUND;
}

LONG WINAPI extRegCloseKey(HKEY hKey)
{
	OutTraceR("RegCloseKey: hKey=%x\n", hKey);
	if (IsFake(hKey)) return ERROR_SUCCESS;
	return (*pRegCloseKey)(hKey);
}


LONG WINAPI extRegSetValueEx(HKEY hKey, LPCTSTR lpValueName, DWORD Reserved, DWORD dwType, const BYTE *lpData, DWORD cbData)
{
	if (IsTraceR){
		OutTrace("RegSetValueEx: hKey=%x ValueName=\"%s\" Type=%x(%s) cbData=%d ", hKey, lpValueName, dwType, ExplainRegType(dwType), cbData);
		switch(dwType){
			case REG_DWORD: OutTrace("Data=%x\n", *(DWORD *)lpData); break;
			case REG_NONE: OutTrace("Data=\"%s\"\n", lpData); break;
			case REG_BINARY: {
				DWORD i;
				OutTrace("Data=%02.2X,", *lpData);
				for(i=1; i<cbData; i++) OutTrace("%02.2X", lpData[i]);
				OutTrace("\n");
				};
				break;
			default: OutTrace("\n");
		}
	}
	if(IsFake(hKey) && (dxw.dwFlags3 & EMULATEREGISTRY)) return ERROR_SUCCESS;
	return (*pRegSetValueEx)(hKey, lpValueName, Reserved, dwType, lpData, cbData);
}

LONG WINAPI extRegCreateKeyEx(HKEY hKey, LPCTSTR lpSubKey, DWORD Reserved, LPTSTR lpClass, DWORD dwOptions, REGSAM samDesired,
				LPSECURITY_ATTRIBUTES lpSecurityAttributes, PHKEY phkResult, LPDWORD lpdwDisposition)
{
	OutTraceR("RegCreateKeyEx: hKey=%x(%s) SubKey=\"%s\" Class=%x\n", hKey, hKey2String(hKey), lpSubKey, lpClass);
	if (dxw.dwFlags3 & EMULATEREGISTRY){
		*phkResult = HKEY_FAKE;
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
		return ERROR_SUCCESS;
	}
	else
		return (*pRegCreateKey)(hKey, lpSubKey, phkResult);
}
