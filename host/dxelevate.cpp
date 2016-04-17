#include "stdafx.h"
#include "dxwndhost.h"

#include "MainFrm.h"
#include "dxwndhostDoc.h"
#include "dxwndhostView.h"

extern BOOL IsProcessElevated();
extern BOOL IsUserInAdminGroup();

BOOL DxSelfElevate(CDxwndhostView *view)
{
	BOOL const fInAdminGroup = IsUserInAdminGroup();
	if(!fInAdminGroup) return TRUE;

    // Get and display the process elevation information.
    BOOL const fIsElevated = IsProcessElevated();
	BOOL MustRestart;
	if(fIsElevated) return TRUE;
	MustRestart=MessageBoxLang(DXW_STRING_ADMINCAP, DXW_STRING_WARNING, MB_OKCANCEL | MB_ICONQUESTION);
	if(MustRestart==IDOK){
		extern HANDLE GlobalLocker;
		// Autoelevation at startup has no HostView yet, but nothing to save either
		if (view && view->isUpdated){
			if (MessageBoxLang(DXW_STRING_LISTUPDATE, DXW_STRING_WARNING, MB_YESNO | MB_ICONQUESTION)==IDYES) 
				view->SaveConfigFile();
		}
		CloseHandle(GlobalLocker);
		char szPath[MAX_PATH];
		if (GetModuleFileName(NULL, szPath, ARRAYSIZE(szPath)))
		{
			// Launch itself as administrator.
			SHELLEXECUTEINFO sei = { sizeof(sei) };
			CString args;
			sei.lpVerb = "runas";
			sei.lpFile = szPath;
			//sei.hwnd = (HWND)this->GetMainWnd();
			sei.hwnd = (HWND)NULL; // set to NULL to force the confirmation dialog on top of everything...
			sei.nShow = SW_NORMAL;
			args = "";
			for(int i=1; i<=__argc; i++) {
				args += (LPCSTR)(__argv[i]);
				args += " ";
			}
			sei.lpParameters = args;
			if (!ShellExecuteEx(&sei)){
				DWORD dwError = GetLastError();
				if (dwError == ERROR_CANCELLED){
					// The user refused the elevation.
					// Do nothing ...
				}
			}
			else{
				exit(0); // Quit itself
			}
		}
	}
	return TRUE;
}