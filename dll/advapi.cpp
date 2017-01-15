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
typedef LONG (WINAPI *RegEnumValueA_Type)(HKEY, DWORD, LPTSTR, LPDWORD, LPDWORD, LPDWORD, LPBYTE, LPDWORD);
LONG WINAPI extRegEnumValueA(HKEY, DWORD, LPTSTR, LPDWORD, LPDWORD, LPDWORD, LPBYTE, LPDWORD);
RegEnumValueA_Type pRegEnumValueA = NULL;

static HookEntryEx_Type Hooks[]={
	{HOOK_IAT_CANDIDATE, 0, "RegOpenKeyExA", NULL, (FARPROC *)&pRegOpenKeyEx, (FARPROC)extRegOpenKeyEx},
	{HOOK_IAT_CANDIDATE, 0, "RegCloseKey", NULL, (FARPROC *)&pRegCloseKey, (FARPROC)extRegCloseKey},
	{HOOK_IAT_CANDIDATE, 0, "RegQueryValueA", NULL, (FARPROC *)&pRegQueryValue, (FARPROC)extRegQueryValue},
	{HOOK_IAT_CANDIDATE, 0, "RegQueryValueExA", NULL, (FARPROC *)&pRegQueryValueEx, (FARPROC)extRegQueryValueEx},
	{HOOK_IAT_CANDIDATE, 0, "RegCreateKeyA", NULL, (FARPROC *)&pRegCreateKey, (FARPROC)extRegCreateKey},
	{HOOK_IAT_CANDIDATE, 0, "RegCreateKeyExA", NULL, (FARPROC *)&pRegCreateKeyEx, (FARPROC)extRegCreateKeyEx},
	{HOOK_IAT_CANDIDATE, 0, "RegSetValueExA", NULL, (FARPROC *)&pRegSetValueEx, (FARPROC)extRegSetValueEx},
	{HOOK_IAT_CANDIDATE, 0, "RegFlushKey", NULL, (FARPROC *)&pRegFlushKey, (FARPROC)extRegFlushKey},
	// v2.3.36
	{HOOK_IAT_CANDIDATE, 0, "RegEnumValueA", NULL, (FARPROC *)&pRegEnumValueA, (FARPROC)extRegEnumValueA},
	{HOOK_IAT_CANDIDATE, 0, 0, NULL, 0, 0} // terminator
};
 
void HookAdvApi32(HMODULE module)
{
	if( (dxw.dwFlags3 & EMULATEREGISTRY) || 
		(dxw.dwFlags4 & OVERRIDEREGISTRY) || 
		(dxw.dwFlags6 & (WOW32REGISTRY|WOW64REGISTRY)) || 
		(dxw.dwTFlags & OUTREGISTRY))
	HookLibraryEx(module, Hooks, "ADVAPI32.dll");
}

FARPROC Remap_AdvApi32_ProcAddress(LPCSTR proc, HMODULE hModule)
{
	FARPROC addr;
	if(!(dxw.dwFlags3 & EMULATEREGISTRY) || 
		(dxw.dwFlags4 & OVERRIDEREGISTRY) || 
		(dxw.dwFlags6 & (WOW32REGISTRY|WOW64REGISTRY)) || 
		(dxw.dwTFlags & OUTREGISTRY)) return NULL;
	if (addr=RemapLibraryEx(proc, hModule, Hooks)) return addr;
	return NULL;
}

#define HKEY_FAKE ((HKEY)0x7FFFFFFF)
#define HKEY_MASK 0x7FFFFF00
#define IsFake(hKey) (((DWORD)hKey & HKEY_MASK) == HKEY_MASK)

static FILE *OpenFakeRegistry();
static char *hKey2String(HKEY);
static LONG myRegOpenKeyEx(HKEY, LPCTSTR, PHKEY);

// int ReplaceVar(pData, lplpData, lpcbData): 
// extract the token name from pData beginning up to '}' delimiter
// calculates the value of the token. If the token is unknown, the value is null string.
// if *lplpData, copies the token value string to *lplpData and increments *lplpData
// if lpcbData, increments the key length of the token value length
// returns the length of token label to advance the parsing loop

typedef enum {
	LABEL_PATH = 0,
	LABEL_WORKDIR,
	LABEL_VOID,
	LABEL_END
};
static char *sTokenLabels[LABEL_END+1]={
	"path}",
	"work}",
	"}",
	NULL
};

static int ReplaceVar(char *pData, LPBYTE *lplpData, LPDWORD lpcbData)
{
	int iTokenLength;
	int iLabelLength;
	int iTokenIndex;
	char sTokenValue[MAX_PATH];
	// search for a matching token
	for(iTokenIndex=0; sTokenLabels[iTokenIndex]; iTokenIndex++){
		if(!strncmp(pData, sTokenLabels[iTokenIndex], strlen(sTokenLabels[iTokenIndex]))) break;
	}
	// set token label length
	iLabelLength = strlen(sTokenLabels[iTokenIndex]);
	// do replacement
	switch(iTokenIndex){
		case LABEL_PATH:
		case LABEL_WORKDIR:
			GetCurrentDirectory(MAX_PATH, sTokenValue);
			break;
		case LABEL_VOID:
		case LABEL_END:
			strcpy(sTokenValue, "");
			break;
	}
	// set output vars if not NULL
	iTokenLength = strlen(sTokenValue);
	OutTrace("REPLACED token=%d val=\"%s\" len=%d\n", iTokenIndex, sTokenValue, iTokenLength);
	if(lplpData) {
		strcpy((char *)*lplpData, sTokenValue);
		*lplpData += iTokenLength;
	}
	if(lpcbData) *lpcbData += iTokenLength;
	// return label length to advance parsing
	return iLabelLength;
}

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
						//sKey[strlen(sKey)-2]=0; // get rid of "]"
						for(int i=strlen(sKey)-1; i; i--){
							if(sKey[i]==']'){
								sKey[i]=0;
								break;
							}
						}
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

static char *Unescape(char *s, char **dest)
{
	if(!*dest)	*dest=(char *)malloc(strlen(s)+100);
	else		*dest=(char *)realloc(*dest, strlen(s)+100); 
	char *t = *dest;
	for(; *s; s++){
		if((*s=='\\') && (*(s+1)=='n')){
			*t++ = '\n';
			s++;
		}
		else{
			*t++ = *s;
		}
	}
	*t=0;
	return *dest;
}

static FILE *OpenFakeRegistry()
{
	DWORD dwAttrib;	
	char sSourcePath[MAX_PATH+1];
	char *p;
	static BOOL LoadFromConfig = TRUE;
	dwAttrib = GetFileAttributes("dxwnd.dll");
	if (dwAttrib != INVALID_FILE_ATTRIBUTES && !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY)) return NULL;
	GetModuleFileName(GetModuleHandle("dxwnd"), sSourcePath, MAX_PATH);
	p=&sSourcePath[strlen(sSourcePath)-strlen("dxwnd.dll")];
	if(LoadFromConfig){
		int Index;
		char key[81];
		char name[MAX_PATH+1];
		char exepath[MAX_PATH+1];
		strcpy(p, "dxwnd.ini");
		GetModuleFileName(0, name, MAX_PATH);
		//for(int i = 0; name[i]; i ++) name[i] = tolower(name[i]);
		for(Index=0; Index<MAXTARGETS; Index++){
			DWORD flags3;
			sprintf_s(key, sizeof(key), "path%i", Index);
			GetPrivateProfileString("target", key, "", exepath, MAX_PATH, sSourcePath);	
			sprintf_s(key, sizeof(key), "flagh%i", Index);
			flags3 = GetPrivateProfileInt("target", key, 0, sSourcePath);
			if(!_stricmp(exepath, name) && (flags3 & HOOKENABLED)) break; // got it!
		}
		if(Index < MAXTARGETS){
			FILE *freg;
			char *RegBuf;
			RegBuf = (char *)malloc(1000000+1); // 1MB!!
			OutTrace("Fake registry: build virtual registry from dxwnd.ini entry #%d\n", Index);
			sprintf_s(key, sizeof(key), "registry%i", Index);
			GetPrivateProfileString("target", key, "", RegBuf, 1000000, sSourcePath);	
			if(strlen(RegBuf)>0){
				char *FileBuf = NULL;
				Unescape(RegBuf, &FileBuf);
				strcpy(p, "dxwnd.reg");
				freg = fopen(sSourcePath,"w");
				fwrite(FileBuf, 1, strlen(FileBuf), freg);
				fputs("\n", freg);
				fclose(freg);
				free(FileBuf);
			}
			free(RegBuf);
		}
		LoadFromConfig = FALSE;
	}
	strcpy(p, "dxwnd.reg");
	return fopen(sSourcePath,"r");
}

static LONG SeekFakeKey(FILE *regf, HKEY hKey)
{
	LONG res;
	res = ERROR_FILE_NOT_FOUND;
	char RegBuf[MAX_PATH+1];
	HKEY hCurKey=HKEY_FAKE+1;
	fgets(RegBuf, 256, regf);
	while (!feof(regf)){
		if(RegBuf[0]=='['){
			hCurKey--;
		}
		if(hCurKey==hKey) {
			//OutTraceB("DEBUG: SeekFakeKey fount key at line=%s\n", RegBuf);
			res = ERROR_SUCCESS;
			break;
		}
		fgets(RegBuf, 256, regf);
	}
	return res;
}

static LONG SeekValueName(FILE *regf, LPCTSTR lpValueName)
{
	LONG res;
	char RegBuf[MAX_PATH+1];
	long KeySeekPtr;
	res = ERROR_FILE_NOT_FOUND;
	// v2.04.01: fix to handle the '@' case properly
	if(lpValueName) if(!lpValueName[0]) lpValueName=NULL; 
	KeySeekPtr = ftell(regf);
	fgets(RegBuf, 256, regf);
	while (!feof(regf)){
		if((RegBuf[0]=='"') &&
			!_strnicmp(lpValueName, &RegBuf[1], strlen(lpValueName)) &&
			(RegBuf[strlen(lpValueName)+1]=='"') &&
			(RegBuf[strlen(lpValueName)+2]=='='))
			{
			fseek(regf, KeySeekPtr, SEEK_SET);
			return ERROR_SUCCESS;
		}
		if(!lpValueName && (RegBuf[0]=='@')){
			fseek(regf, KeySeekPtr, SEEK_SET);
			return ERROR_SUCCESS;
		}
		if(RegBuf[0]=='[') return res;
		KeySeekPtr = ftell(regf);
		fgets(RegBuf, 256, regf);
	}
	return res;
}

static LONG SeekValueIndex(FILE *regf, DWORD dwIndex, LPCTSTR lpValueName, LPDWORD lpcchValueName)
{
	LONG res;
	char RegBuf[MAX_PATH+1];
	long KeySeekPtr;
	res = ERROR_NO_MORE_ITEMS;
	KeySeekPtr = ftell(regf);
	fgets(RegBuf, 256, regf);
	dwIndex++;
	while (!feof(regf) && dwIndex){
		if(RegBuf[0]=='"') dwIndex--;
		if(dwIndex == 0){
			fseek(regf, KeySeekPtr, SEEK_SET);
			//sscanf(RegBuf, "\"%s\"=", lpValueName);
			strncpy((char *)lpValueName, strtok(&RegBuf[1], "\""), *lpcchValueName);
			*lpcchValueName = strlen(lpValueName);
			//OutTrace("DEBUG: lpValueName=%s len=%d\n", lpValueName, *lpcchValueName);
			return ERROR_SUCCESS;
		}
		if(RegBuf[0]=='[') return res;
		KeySeekPtr = ftell(regf);
		fgets(RegBuf, 256, regf);
	}
	return res;
}

static DWORD GetKeyValue(
				FILE *regf,
				char *ApiName, 
				LPCTSTR lpValueName, 
				LPDWORD lpType, // beware: could be NULL
				LPBYTE lpData,  // beware: could be NULL
				LPDWORD lpcbData)
{
	LONG res;
	LPBYTE lpb;
	char *pData;
	char RegBuf[MAX_PATH+1];
	DWORD cbData=0;

	//OutTrace("GetKeyValue: ValueName=%s\n", lpValueName);
	fgets(RegBuf, 256, regf);
	if(RegBuf[0]=='@')
		pData=&RegBuf[2];
	else
		pData=&RegBuf[strlen(lpValueName)+3];
	lpb = lpData;
	if(lpcbData) {
		cbData = *lpcbData;
		*lpcbData=0;
	}
	do {
		if(*pData=='"'){ // string value
			if(lpType) *lpType=REG_SZ;
			pData++;
			while(*pData && (*pData != '"')){
				if(*pData=='\\') {
					pData++;
					switch(*pData){
						case '{':{
							pData++; // skip '{'
							pData += ReplaceVar(pData, &lpb, lpcbData);
							}
							break;
						default: 
							break; // skip first '\'
					}
				}
				if(lpData && lpcbData) if(*lpcbData < cbData) *lpb++=*pData;
				pData++;
				if(lpcbData) (*lpcbData)++;
			}
			if(lpcbData) (*lpcbData)++; // extra space for string terminator ?
			if(lpData && lpcbData) if(*lpcbData < cbData) *lpb = 0; // string terminator
			OutTraceR("%s: type=REG_SZ cbData=%x Data=\"%s\"\n", 
				ApiName, lpcbData ? *lpcbData : 0, lpData ? (char *)lpData : "(NULL)");
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
			else 
				res=ERROR_SUCCESS; // data not needed
			if (lpcbData) *lpcbData=sizeof(DWORD);
			OutTraceR("%s: type=REG_DWORD cbData=%x Data=0x%x\n", 
				ApiName, lpcbData ? *lpcbData : 0, val);
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
			OutTraceR("%s: type=REG_BINARY cbData=%d Data=%s\n", 
				ApiName,
				lpcbData ? *lpcbData : 0, 
				lpData ? "(NULL)" : hexdump(lpData, *lpcbData));
			res=(*lpcbData > cbData) ? ERROR_MORE_DATA : ERROR_SUCCESS;
			break;
		}
	} while(FALSE);
	return res;
} 

static void LogKeyValue(char *ApiName, LONG res, LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData)
{
	char sInfo[1024];
	if(res) {
		OutTrace("%s: ERROR res=%x\n", ApiName, res);
		return;
	}
	sprintf(sInfo, "%s: res=0 size=%d type=%x(%s)", 
		ApiName, lpcbData?*lpcbData:0, lpType?*lpType:0, lpType?ExplainRegType(*lpType):"none");
	if(lpType && lpData && lpcbData) {
		DWORD cbData = *lpcbData;
		switch(*lpType){
			case REG_SZ: sprintf(sInfo, "%s Data=\"%*.*s\"\n", sInfo, cbData-1, cbData-1, lpData); break; 
			case REG_DWORD: sprintf(sInfo, "%s Data=0x%x\n", sInfo, *(DWORD *)lpData); break;
			case REG_BINARY: sprintf(sInfo, "%s Data=%s\n", sInfo, hexdump((BYTE *)lpData, cbData)); break;
			case REG_NONE: sprintf(sInfo, "%s Data=\"%s\"\n", sInfo, lpData); break;
			default: sprintf(sInfo, "%s Data=???\n", sInfo); break;
		}
	}
	else 
		sprintf(sInfo, "%s\n", sInfo);
	OutTrace(sInfo);
} 

static LONG myRegOpenKeyEx(
			    LPCSTR label,
				HKEY hKey,
				LPCTSTR lpSubKey,
				PHKEY phkResult)
{
	FILE *regf;
	char sKey[MAX_PATH+1];
	char RegBuf[MAX_PATH+1];

	sprintf(sKey,"%s\\%s", hKey2String(hKey), lpSubKey);
	OutTraceR("%s: searching for key=\"%s\"\n", label, sKey);

	regf=OpenFakeRegistry();
	if(regf!=NULL){
		if(phkResult) *phkResult=HKEY_FAKE;
		fgets(RegBuf, 256, regf);
		while (!feof(regf)){
			if(RegBuf[0]=='['){
				// beware: registry keys are case insensitive. Must use _strnicmp instead of strncmp
				if((!_strnicmp(&RegBuf[1],sKey,strlen(sKey))) && (RegBuf[strlen(sKey)+1]==']')){
					OutTrace("%s: found fake Key=\"%s\" hkResult=%x\n", label, sKey, phkResult ? *phkResult : 0);
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

// ---------------------------------------------------------------------------------

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
		res = myRegOpenKeyEx("RegOpenKeyEx", hKey, lpSubKey, phkResult);
		if(res == ERROR_SUCCESS) return res;
	}

	if(dxw.dwFlags6 & WOW64REGISTRY){
		ulOptions &= ~KEY_WOW64_32KEY;
		ulOptions |= KEY_WOW64_64KEY;
	}
	if(dxw.dwFlags6 & WOW32REGISTRY){
		ulOptions &= ~KEY_WOW64_64KEY;
		ulOptions |= KEY_WOW64_32KEY;
	}

	res=(*pRegOpenKeyEx)(hKey, lpSubKey, ulOptions, samDesired, phkResult);
	OutTraceR("RegOpenKeyEx: res=%x phkResult=%x\n", res, phkResult ? *phkResult : 0); 

	if((res==ERROR_SUCCESS) || !(dxw.dwFlags3 & EMULATEREGISTRY) || (dxw.dwFlags4 & OVERRIDEREGISTRY)) return res;
	
	return myRegOpenKeyEx("RegOpenKeyEx", hKey, lpSubKey, phkResult);
}

// extRegQueryValue: legacy API, almost always replaced by extRegQueryValueEx but referenced
// in "Warhammer 40.000 Shadow of the Horned Rat"

LONG WINAPI extRegQueryValue(
				HKEY hKey, 
				LPCTSTR lpSubKey, 
				LPTSTR lpValue, 
				PLONG lpcbValue)
{
	LONG res;
	FILE *regf;

	OutTraceR("RegQueryValue: hKey=%x(%s) SubKey=\"%s\"\n", hKey, hKey2String(hKey), lpSubKey);
	
	if (!IsFake(hKey)){
		res=(*pRegQueryValue)(hKey, lpSubKey, lpValue, lpcbValue);
		if(IsTraceR) LogKeyValue("RegQueryValue", res, 0, (LPBYTE)lpValue, (LPDWORD)lpcbValue);
		return res;
	}

	regf=OpenFakeRegistry();
	if(regf==NULL) {
		OutTraceR("RegQueryValue: error in OpenFakeRegistry err=%s\n", GetLastError());	
		return ERROR_FILE_NOT_FOUND;
	}
	res = SeekFakeKey(regf, hKey);
	if(res != ERROR_SUCCESS) {
		OutTraceR("RegQueryValue: error in SeekFakeKey res=%x hKey=%x\n", res, hKey);	
		return res;
	}
	res = SeekValueName(regf, lpSubKey);
	if(res != ERROR_SUCCESS) {
		OutTraceR("RegQueryValue: error in SeekValueName res=%x ValueName=%s\n", res, lpSubKey);	
		return res;
	}
	res = GetKeyValue(regf, "RegQueryValue", lpSubKey, NULL, (LPBYTE)lpValue, (LPDWORD)lpcbValue);
	if(IsTraceR) LogKeyValue("RegQueryValue", res, NULL, (LPBYTE)lpValue, (LPDWORD)lpcbValue);
	fclose(regf);
	return res;
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
	FILE *regf;
	DWORD cbData=0;

	OutTraceR("RegQueryValueEx: hKey=%x(\"%s\") ValueName=\"%s\" Reserved=%x lpType=%x lpData=%x lpcbData=%x\n", 
		hKey, hKey2String(hKey), lpValueName, lpReserved, lpType, lpData, lpcbData);
	if (!IsFake(hKey)){
		res=(*pRegQueryValueEx)(hKey, lpValueName, lpReserved, lpType, lpData, lpcbData);
		if(IsTraceR) LogKeyValue("RegQueryValueEx", res, lpType, lpData, lpcbData);
		return res;
	}

	regf=OpenFakeRegistry();
	if(regf==NULL) {
		OutTraceR("RegQueryValueEx: error in OpenFakeRegistry err=%s\n", GetLastError());	
		return ERROR_FILE_NOT_FOUND;
	}
	res = SeekFakeKey(regf, hKey);
	if(res != ERROR_SUCCESS) {
		OutTraceR("RegQueryValueEx: error in SeekFakeKey res=%x hKey=%x\n", res, hKey);	
		return res;
	}
	res = SeekValueName(regf, lpValueName);
	if(res != ERROR_SUCCESS) {
		OutTraceR("RegQueryValueEx: error in SeekValueName res=%x ValueName=%s\n", res, lpValueName);	
		return res;
	}
	res = GetKeyValue(regf, "RegQueryValueEx", lpValueName, lpType, lpData, lpcbData);
	if(IsTraceR) LogKeyValue("RegQueryValueEx", res, lpType, lpData, lpcbData);
	fclose(regf);
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
        if(dxw.dwFlags4 & OVERRIDEREGISTRY) myRegOpenKeyEx("RegCreateKeyEx", hKey, lpSubKey, phkResult);
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
        if(dxw.dwFlags4 & OVERRIDEREGISTRY) myRegOpenKeyEx("RegCreateKey", hKey, lpSubKey, phkResult);
		return ERROR_SUCCESS;
	}
	else
		return (*pRegCreateKey)(hKey, lpSubKey, phkResult);
}

LONG WINAPI extRegEnumValueA(HKEY hKey, DWORD dwIndex, LPTSTR lpValueName, LPDWORD lpcchValueName, LPDWORD lpReserved, LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData)
{
	LONG res;

	OutTraceR("RegEnumValue: hKey=%x(\"%s\") index=%d cchValueName=%d Reserved=%x lpType=%x lpData=%x lpcbData=%x\n", 
		hKey, hKey2String(hKey), dwIndex, *lpcchValueName, lpReserved, lpType, lpData, lpcbData);
	if (!IsFake(hKey)){
		res=(*pRegEnumValueA)(hKey, dwIndex, lpValueName, lpcchValueName, lpReserved, lpType, lpData, lpcbData);
		if(IsTraceR) LogKeyValue("RegQueryValueEx", res, lpType, lpData, lpcbData);
		return res;
	}

	// try emulated registry
	FILE *regf;
	regf=OpenFakeRegistry();
	if(regf==NULL) return ERROR_FILE_NOT_FOUND;
	res = SeekFakeKey(regf, hKey);
	if(res != ERROR_SUCCESS) return res;
	res = SeekValueIndex(regf, dwIndex, lpValueName, lpcchValueName);
	if(res != ERROR_SUCCESS) return res;
	res = GetKeyValue(regf, "RegEnumValue", lpValueName, lpType, lpData, lpcbData);
	if(IsTraceR) LogKeyValue("RegEnumValue", res, lpType, lpData, lpcbData);
	fclose(regf);
	return res;
}

#ifdef TOBEDONE
LONG WINAPI RegQueryInfoKey(
  _In_        HKEY      hKey,
  _Out_opt_   LPTSTR    lpClass,
  _Inout_opt_ LPDWORD   lpcClass,
  _Reserved_  LPDWORD   lpReserved,
  _Out_opt_   LPDWORD   lpcSubKeys,
  _Out_opt_   LPDWORD   lpcMaxSubKeyLen,
  _Out_opt_   LPDWORD   lpcMaxClassLen,
  _Out_opt_   LPDWORD   lpcValues,
  _Out_opt_   LPDWORD   lpcMaxValueNameLen,
  _Out_opt_   LPDWORD   lpcMaxValueLen,
  _Out_opt_   LPDWORD   lpcbSecurityDescriptor,
  _Out_opt_   PFILETIME lpftLastWriteTime
);
#endif