#include "stdafx.h"
#include <map>
#include<Dbghelp.h>
#include <psapi.h>

#include "shlwapi.h"

#define BUFSIZE 512

//Cstring GetFileNameFromHandle(HANDLE hFile) 
char *GetFileNameFromHandle(HANDLE hFile) 
{
	BOOL bSuccess = FALSE;
	TCHAR pszFilename[MAX_PATH+1];
	HANDLE hFileMap;
	//CString strFilename;
	static char strFilename[MAX_PATH+1];
	// Get the file size.
	DWORD dwFileSizeHi = 0;
	DWORD dwFileSizeLo = GetFileSize(hFile, &dwFileSizeHi); 
	if( dwFileSizeLo == 0 && dwFileSizeHi == 0 ) return FALSE;
	typedef DWORD (WINAPI *GetMappedFileName_Type)(HANDLE, LPVOID, LPTSTR, DWORD);
	static GetMappedFileName_Type pGetMappedFileName = NULL;

	if(!pGetMappedFileName) do { // fake loop
		HMODULE lib;
		// Psapi.dll (if PSAPI_VERSION=1) on Windows 7 and Windows Server 2008 R2;
		// Psapi.dll on Windows Server 2008, Windows Vista, Windows Server 2003, and Windows XP
		lib=LoadLibrary("Psapi.dll");
		pGetMappedFileName=(GetMappedFileName_Type)GetProcAddress(lib,"GetMappedFileNameA");
		if(pGetMappedFileName) break;
		// Kernel32.dll on Windows 7 and Windows Server 2008 R2; 
		FreeLibrary(lib);
		lib=LoadLibrary("Kernel32.dll");
		pGetMappedFileName=(GetMappedFileName_Type)GetProcAddress(lib,"GetMappedFileNameA");
		if(!pGetMappedFileName) return NULL;
	} while(FALSE);

	// Create a file mapping object.
	hFileMap = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 1, NULL);
	if (hFileMap) {
		// Create a file mapping to get the file name.
		void* pMem = MapViewOfFile(hFileMap, FILE_MAP_READ, 0, 0, 1);
		if (pMem) {
			if ((*pGetMappedFileName) (GetCurrentProcess(), pMem, pszFilename, MAX_PATH)){
				// Translate path with device name to drive letters.
				TCHAR szTemp[BUFSIZE];
				szTemp[0] = '\0';
				if (GetLogicalDriveStrings(BUFSIZE-1, szTemp)) {
					TCHAR szName[MAX_PATH];
					TCHAR szDrive[3] = TEXT(" :");
					BOOL bFound = FALSE;
					TCHAR* p = szTemp;
					do {
						// Copy the drive letter to the template string
						*szDrive = *p;
						// Look up each device name
						if (QueryDosDevice(szDrive, szName, MAX_PATH)){
							size_t uNameLen = _tcslen(szName);
							if (uNameLen < MAX_PATH) {
								bFound = _tcsnicmp(pszFilename, szName, uNameLen) == 0;
								//if (bFound) strFilename.Format(L"%s%s",szDrive, pszFilename+uNameLen);
								if (bFound) sprintf(strFilename, "%s%s", szDrive, pszFilename+uNameLen);
							}
						}
						// Go to the next NULL character.
						while (*p++);
					} while (!bFound && *p); // end of string
				}
			}
			bSuccess = TRUE;
			UnmapViewOfFile(pMem);
		} 
		CloseHandle(hFileMap);
	}
	return(strFilename);
}

