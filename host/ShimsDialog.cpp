// ShimsDialog.cpp : implementation file
//

#include "stdafx.h"
#include "dxwndhost.h"
#include "ShimsDialog.h"

// http://msdn2.microsoft.com/en-us/library/bb432457
// HSDB WINAPI SdbOpenDatabase(
//  LPCTSTR pwszPath,
//  PATH_TYPE eType
//);

// What is HSDB!? Assuming it is a pointer to somewhere => "void*"
typedef void* HSDB;

// http://msdn2.microsoft.com/en-us/library/bb432389.aspx
typedef enum _PATH_TYPE
{
  DOS_PATH,
  NT_PATH
}PATH_TYPE;

// http://msdn2.microsoft.com/en-us/library/bb432488
typedef DWORD TAGID;
#define TAGID_NULL (0)
#define TAGID_ROOT (0)

// TAG !?
typedef DWORD TAG;

// http://msdn2.microsoft.com/en-us/library/bb432489
typedef DWORD TAGREF;
#define TAGREF_NULL (0)
#define TAGREF_ROOT (0)

// http://msdn2.microsoft.com/en-us/library/bb432490
// TAG-Types:
#define TAG_TYPE_NULL 0x1000
#define TAG_TYPE_BYTE 0x2000
#define TAG_TYPE_WORD 0x3000
#define TAG_TYPE_DWORD 0x4000
#define TAG_TYPE_QWORD 0x5000
#define TAG_TYPE_STRINGREF 0x6000
#define TAG_TYPE_LIST 0x7000
#define TAG_TYPE_STRING 0x8000
#define TAG_TYPE_BINARY 0x9000

// It seems that this type is "masked"
#define TAG_TYPE_MASK 0xF000

// http://msdn2.microsoft.com/en-us/library/bb432487
#define TAG_APP_NAME_RC_ID (0x24 | TAG_TYPE_DWORD)
#define TAG_VENDOR_NAME_RC_ID (0x25 | TAG_TYPE_DWORD)
#define TAG_SUMMARY_MSG_RC_ID (0x26 | TAG_TYPE_DWORD)
#define TAG_DESCRIPTION_RC_ID (0x28 | TAG_TYPE_DWORD)
#define TAG_PARAMETER1_RC_ID (0x29 | TAG_TYPE_DWORD)

#define TAG_EXE_ID (0x4 | TAG_TYPE_BINARY)
#define TAG_DATABASE_ID (0x7 | TAG_TYPE_BINARY)
#define TAG_FIX_ID (0x10 | TAG_TYPE_BINARY)
#define TAG_APP_ID (0x11 | TAG_TYPE_BINARY)
#define CONTEXT_PLATFORM_ID (0x8 | TAG_TYPE_BINARY)

#define HID_DOS_PATHS				0x00000001
#define HID_DATABASE_FULLPATH		0x00000002
#define HID_NO_DATABASE				0x00000004
#define HID_DATABASE_TYPE_MASK		0xF00F0000

#define SDB_DATABASE_MAIN_SHIM		0x80030000
#define SDB_DATABASE_MAIN_MSI		0x80020000
#define SDB_DATABASE_MAIN_DRIVERS	0x80040000

typedef struct TAG_RC_Entry
{
  DWORD tag;
  LPCWSTR szName;
} TAG_RC_Entry;

// The following tags returns an reference to an resource... (Vista and later)
static TAG_RC_Entry s_TagRcEntries[] =
{
  {TAG_APP_NAME_RC_ID, L"APP_NAME_RC_ID"},
  {TAG_VENDOR_NAME_RC_ID, L"VENDOR_NAME_RC_ID"},
  {TAG_SUMMARY_MSG_RC_ID, L"SUMMARY_MSG_RC_ID"},
  {TAG_DESCRIPTION_RC_ID, L"DESCRIPTION_RC_ID"},  // at least for this one, MS forgot to add the mapping from TAG to name (SdbTagToString)
  {TAG_PARAMETER1_RC_ID, L"PARAMETER1_RC_ID"},
};

// The following Tags could be displayed as GUIDs (if the len is 0x10)
static DWORD s_TagGuids[] = 
{
  TAG_EXE_ID,
  TAG_DATABASE_ID,
  TAG_FIX_ID,
  TAG_APP_ID,
  CONTEXT_PLATFORM_ID,
};

// APIs:

// http://msdn.microsoft.com/en-us/library/bb432457.aspx
//HSDB WINAPI SdbOpenDatabase(
//  __in  LPCTSTR pwszPath,
//  __in  PATH_TYPE eType
//);
typedef HSDB (WINAPI *PSdbOpenDatabase)(
  LPCTSTR pwszPath, // Docu wrong? LPCTSTR shouldn't it be LPCWSTR!?
  PATH_TYPE eType
);
PSdbOpenDatabase pSdbOpenDatabase = NULL;

// http://msdn.microsoft.com/en-us/library/bb432452.aspx
//HSDB WINAPI SdbInitDatabase(
//	__in DWORD	dwFlags,
//  __in LPCTSTR pszDatabasePath
//);
typedef HSDB (WINAPI *PSdbInitDatabase)(
  DWORD dwFlags,
  LPCTSTR pszDatabasePath 
);
PSdbInitDatabase pSdbInitDatabase = NULL;

// http://msdn.microsoft.com/en-us/library/cc895520
//void WINAPI SdbCloseDatabase(
//  __inout  HSDB pdb
//);
typedef VOID (WINAPI *PSdbCloseDatabase)(
  HSDB handle  // assuming the passed handle...
);
PSdbCloseDatabase pSdbCloseDatabase = NULL;

// http://msdn2.microsoft.com/en-us/library/bb432446
//TAGID WINAPI SdbGetFirstChild(
//  HSDB pdb,
//  TAGID tiParent
//);
typedef TAGID (WINAPI *PSdbGetFirstChild)(
  HSDB pdb,
  TAGID tiParent
);
PSdbGetFirstChild pSdbGetFirstChild = NULL;

// http://msdn2.microsoft.com/en-us/library/bb432449
//TAGID WINAPI SdbGetNextChild(
//  HSDB pdb,
//  TAGID tiParent,
//  TAGID tiPrev
//);
typedef TAGID (WINAPI *PSdbGetNextChild)(
  HSDB pdb,
  TAGID tiParent,
  TAGID tiPrev
);
PSdbGetNextChild pSdbGetNextChild = NULL;

// http://msdn2.microsoft.com/en-us/library/bb432451
//TAG WINAPI SdbGetTagFromTagID(
//  HSDB pdb,
//  TAGID tiWhich
//);
typedef TAG (WINAPI *PSdbGetTagFromTagID)(
  HSDB pdb,
  TAGID tiWhich
);
PSdbGetTagFromTagID pSdbGetTagFromTagID = NULL;

// http://msdn2.microsoft.com/en-us/library/bb432470
//LPCTSTR WINAPI SdbTagToString(
//  TAG tag
//);
typedef LPCWSTR (WINAPI *PSdbTagToString)( // LPCTSTR => LPCWSTR !?
  TAG tag
);
PSdbTagToString pSdbTagToString = NULL;

// http://msdn2.microsoft.com/en-us/library/bb432464
//BOOL WINAPI SdbReadStringTag(
//  HSDB pdb,
//  TAGID tiWhich,
//  LPTSTR pwszBuffer,
//  DWORD cchBufferSize
//);
typedef BOOL (WINAPI *PSdbReadStringTag)(
  HSDB pdb,
  TAGID tiWhich,
  LPTSTR pwszBuffer,
  DWORD cchBufferSize
);
PSdbReadStringTag pSdbReadStringTag = NULL;

// http://msdn2.microsoft.com/en-us/library/bb432450.aspx
//LPWSTR WINAPI SdbGetStringTagPtr(
//  HSDB pdb,
//  TAGID tiWhich
//);
typedef LPWSTR (WINAPI *PSdbGetStringTagPtr)(
  HSDB pdb,
  TAGID tiWhich
);
PSdbGetStringTagPtr pSdbGetStringTagPtr = NULL;

// http://msdn2.microsoft.com/en-us/library/bb432462.aspx
//DWORD WINAPI SdbReadDWORDTag(
//  HSDB pdb,
//  TAGID tiWhich,
//  DWORD dwDefault
//);
typedef DWORD (WINAPI *PSdbReadDWORDTag)(
  HSDB pdb,
  TAGID tiWhich,
  DWORD dwDefault
);
PSdbReadDWORDTag pSdbReadDWORDTag = NULL;

// http://msdn2.microsoft.com/en-us/library/bb432463.aspx
//ULONGLONG WINAPI SdbReadQWORDTag(
//  HSDB pdb,
//  TAGID tiWhich,
//  ULONGLONG qwDefault
//);
typedef ULONGLONG (WINAPI *PSdbReadQWORDTag)(
  HSDB pdb,
  TAGID tiWhich,
  ULONGLONG qwDefault
);
PSdbReadQWORDTag pSdbReadQWORDTag = NULL;

// http://msdn2.microsoft.com/en-us/library/bb432455
//HSDB WINAPI SdbOpenApphelpDetailsDatabase(
//  LPCWSTR pwsDetailsDatabasePath
//);
typedef HSDB (WINAPI *PSdbOpenApphelpDetailsDatabase)(
  LPCWSTR pwsDetailsDatabasePath
);
PSdbOpenApphelpDetailsDatabase pSdbOpenApphelpDetailsDatabase = NULL;

// http://msdn2.microsoft.com/en-us/library/bb432456
//HMODULE WINAPI SdbOpenApphelpResourceFile(
//  LPCWSTR pwszACResourceFile
//);
typedef HMODULE (WINAPI *PSdbOpenApphelpResourceFile)(
  LPCWSTR pwszACResourceFile
);
PSdbOpenApphelpResourceFile pSdbOpenApphelpResourceFile = NULL;

// http://msdn.microsoft.com/en-us/library/bb432443
//PVOID WINAPI SdbGetBinaryTagData(
//  __in  HSDB pdb,
//  __in  TAGID tiWhich
//);
typedef PVOID (WINAPI *PSdbGetBinaryTagData)(
  HSDB pdb,
  TAGID tiWhich
);
PSdbGetBinaryTagData pSdbGetBinaryTagData = NULL;

// ???
//DWORD WINAPI SdbGetTagDataSize(
//  __in  HSDB pdb,
//  __in  TAGID tiWhich
//);
typedef DWORD (WINAPI *PSdbGetTagDataSize)(
  HSDB pdb,
  TAGID tiWhich
);
PSdbGetTagDataSize pSdbGetTagDataSize = NULL;

// http://msdn.microsoft.com/en-us/library/bb432459
#define SDB_MAX_EXES 16
#define SDB_MAX_LAYERS 8
#define SDB_MAX_SDBS 16
typedef struct tagSDBQUERYRESULT {
  TAGREF atrExes[SDB_MAX_EXES];
  DWORD  adwExeFlags[SDB_MAX_EXES];
  TAGREF atrLayers[SDB_MAX_LAYERS];
  DWORD  dwLayerFlags;
  TAGREF trApphelp;
  DWORD  dwExeCount;
  DWORD  dwLayerCount;
  GUID   guidID;
  DWORD  dwFlags;
  DWORD  dwCustomSDBMap;
  GUID   rgGuidDB[SDB_MAX_SDBS];
} SDBQUERYRESULT, *PSDBQUERYRESULT;

// http://msdn.microsoft.com/en-us/library/bb432448
typedef BOOL (WINAPI *PSdbGetMatchingExe)(
  HSDB hSDB,
  LPCWSTR szPath,
  LPCWSTR szModuleName,
  LPCWSTR pszEnvironment,
  DWORD dwFlags,
  PSDBQUERYRESULT pQueryResult
);
PSdbGetMatchingExe pSdbGetMatchingExe = NULL;

#define SDBGMEF_IGNORE_ENVIRONMENT 1

#define SHIMREG_DISABLE_SHIM 0x1
#define SHIMREG_DISABLE_APPHELP 0x2
#define SHIMREG_APPHELP_NOUI 0x4
#define SHIMREG_APPHELP_CANCEL 0x10000000
#define SHIMREG_DISABLE_SXS 0x10
#define SHIMREG_DISABLE_LAYER 0x20
#define SHIMREG_DISABLE_DRIVER 0x40

typedef void (WINAPI* PSdbReleaseMatchingExe)(
  HSDB hSDB,
  TAGREF trExe
);
PSdbReleaseMatchingExe pSdbReleaseMatchingExe = NULL;



// CShimsDialog dialog

IMPLEMENT_DYNAMIC(CShimsDialog, CDialog)

CShimsDialog::CShimsDialog(CWnd* pParent /*=NULL*/, char *path)
	: CDialog(CShimsDialog::IDD, pParent)
{
	//ExePath = (LPWSTR)malloc((strlen(path)+1)*sizeof(WCHAR));
	//MultiByteToWideChar(CP_ACP, 0, path, -1, ExePath, strlen(path));
	ExePath = path;
	
}

CShimsDialog::~CShimsDialog()
{
	free(ExePath);
	ExePath=NULL;
}

void CShimsDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CShimsDialog, CDialog)
END_MESSAGE_MAP()

#define IDStatusTIMER 1

// CShimsDialog message handlers

BOOL CShimsDialog::OnInitDialog()
{
	CDialog::OnInitDialog();
	char *MatchExe(char *);
	this->SetDlgItemText(IDC_STATUSINFO, MatchExe(ExePath));	
	return TRUE;  // return TRUE unless you set the focus to a control
}

void CShimsDialog::OnOK()
{
	CDialog::OnOK();
} 

void Flags2String(DWORD dwFlags, char *szFlags, size_t flagsLen)
{
	szFlags[0] = 0;
	if (dwFlags & SHIMREG_DISABLE_SHIM)		strcat_s(szFlags, flagsLen, "Disable-Shim,");
	if (dwFlags & SHIMREG_DISABLE_APPHELP)	strcat_s(szFlags, flagsLen, "Disable-Apphelp,");
	if (dwFlags & SHIMREG_APPHELP_NOUI)		strcat_s(szFlags, flagsLen, "Apphelp-NoUI,");
	if (dwFlags & SHIMREG_APPHELP_CANCEL)	strcat_s(szFlags, flagsLen, "Apphelp-Cancel,");
	if (dwFlags & SHIMREG_DISABLE_SXS)		strcat_s(szFlags, flagsLen, "Disable-SxS,");
	if (dwFlags & SHIMREG_DISABLE_LAYER)	strcat_s(szFlags, flagsLen, "Disable-Layer,");
	if (dwFlags & SHIMREG_DISABLE_DRIVER)	strcat_s(szFlags, flagsLen, "Disable-Driver,");

	if (szFlags[strlen(szFlags)-1]==',') szFlags[strlen(szFlags)-1]=0; 
}

char * MatchExe(char *FileName)
{
	SDBQUERYRESULT result;
	static char sBuf[10000];
	LPWSTR szFileName = (LPWSTR)malloc((strlen(FileName)+1)*sizeof(WCHAR));
	MultiByteToWideChar(CP_ACP, 0, FileName, -1, szFileName, strlen(FileName));
	BOOL bRet;
	HSDB hSDB;

	HINSTANCE hAppHelp = LoadLibrary(_T("apphelp.dll"));
	if(!hAppHelp){
		sprintf(sBuf, "error %d loading apphelp.dll\n", GetLastError());
		return sBuf;
	}

	pSdbOpenDatabase = (PSdbOpenDatabase) GetProcAddress(hAppHelp, "SdbOpenDatabase");
	pSdbInitDatabase = (PSdbInitDatabase) GetProcAddress(hAppHelp, "SdbInitDatabase");
	pSdbCloseDatabase = (PSdbCloseDatabase) GetProcAddress(hAppHelp, "SdbCloseDatabase");
	pSdbGetFirstChild = (PSdbGetFirstChild) GetProcAddress(hAppHelp, "SdbGetFirstChild");
	pSdbGetNextChild = (PSdbGetNextChild) GetProcAddress(hAppHelp, "SdbGetNextChild");
	pSdbGetTagFromTagID = (PSdbGetTagFromTagID) GetProcAddress(hAppHelp, "SdbGetTagFromTagID");
	pSdbTagToString = (PSdbTagToString) GetProcAddress(hAppHelp, "SdbTagToString");
	pSdbGetStringTagPtr = (PSdbGetStringTagPtr) GetProcAddress(hAppHelp, "SdbGetStringTagPtr");
	//pSdbReadStringTag = (PSdbReadStringTag) GetProcAddress(hAppHelp, "SdbReadStringTag");
	pSdbReadDWORDTag = (PSdbReadDWORDTag) GetProcAddress(hAppHelp, "SdbReadDWORDTag");
	pSdbReadQWORDTag = (PSdbReadQWORDTag) GetProcAddress(hAppHelp, "SdbReadQWORDTag");
	//pSdbOpenApphelpDetailsDatabase = (PSdbOpenApphelpDetailsDatabase) GetProcAddress(hAppHelp, "SdbOpenApphelpDetailsDatabase");
	pSdbOpenApphelpResourceFile = (PSdbOpenApphelpResourceFile) GetProcAddress(hAppHelp, "SdbOpenApphelpResourceFile");

	pSdbGetBinaryTagData = (PSdbGetBinaryTagData) GetProcAddress(hAppHelp, "SdbGetBinaryTagData");
	pSdbGetTagDataSize = (PSdbGetTagDataSize) GetProcAddress(hAppHelp, "SdbGetTagDataSize");
	pSdbGetMatchingExe = (PSdbGetMatchingExe) GetProcAddress(hAppHelp, "SdbGetMatchingExe");
	pSdbReleaseMatchingExe = (PSdbReleaseMatchingExe) GetProcAddress(hAppHelp, "SdbReleaseMatchingExe");

	if(pSdbGetMatchingExe == NULL){
		// rough protection: we assume that if this is found, then all fpointers are there.
		sprintf(sBuf, "Unsupported Shim DB\n", FileName);
		FreeLibrary(hAppHelp);
		return sBuf;
	}

	//hSDB = pSdbInitDatabase(HID_DATABASE_TYPE_MASK|SDB_DATABASE_MAIN_SHIM, NULL);
	hSDB = pSdbInitDatabase(SDB_DATABASE_MAIN_SHIM, NULL);
	if(!hSDB){
		sprintf(sBuf, "Can't initialize shims database\n");
		return (sBuf);
	}

	bRet = pSdbGetMatchingExe(NULL, (LPCWSTR)szFileName, NULL, NULL, 0, &result);
	if(!bRet){
		sprintf(sBuf, "No Shim found for file: %s\n", FileName);
		FreeLibrary(hAppHelp);
		return (sBuf);
	}

	sprintf(sBuf, "Shim found for file: %s\n", FileName);
	const size_t flagsLen = 1024;
	char szFlagsStr[flagsLen];
	Flags2String(result.dwFlags, szFlagsStr, flagsLen);
	sprintf(sBuf, "%sFlags: 0x%x: %s\n", sBuf, result.dwFlags, szFlagsStr);

	if (result.trApphelp != TAGREF_NULL) sprintf(sBuf, "%sAppHelp-Message: 0x%x\n", sBuf, result.trApphelp);

	for(DWORD i=0; i<result.dwExeCount; i++){
		sprintf(sBuf, "%sExe-Shim: 0x%x, ", sBuf, result.atrExes[i]);
		Flags2String(result.adwExeFlags[i], szFlagsStr, flagsLen);
		sprintf(sBuf, "%sFlags: 0x%x: %s\n", sBuf, result.adwExeFlags[i], szFlagsStr);
	}

	for(DWORD i=0; i<result.dwLayerCount; i++){
		sprintf(sBuf, "%sLayer-Shim: 0x%x\n", sBuf, result.atrLayers[i]);
	}

	Flags2String(result.dwLayerFlags, szFlagsStr, flagsLen);
	sprintf(sBuf, "%sLayer-Flags: 0x%x: %s\n", sBuf, result.dwLayerFlags, szFlagsStr);

	sprintf(sBuf, "%sCustomSDBMap: 0x%x\n", sBuf, result.dwCustomSDBMap);
	for(DWORD i=0; i<SDB_MAX_SDBS; i++){
		if (result.rgGuidDB[i] != GUID_NULL){
			sprintf(sBuf, "%sShim-Database: %8.8X-%4.4X-%4.4X-%2.2X%2.2X%2.2X%2.2X%2.2X%2.2X%2.2X%2.2X\n", 
			sBuf,
			result.rgGuidDB[i].Data1,
			result.rgGuidDB[i].Data2,
			result.rgGuidDB[i].Data3,
			result.rgGuidDB[i].Data4[0],
			result.rgGuidDB[i].Data4[1],
			result.rgGuidDB[i].Data4[2],
			result.rgGuidDB[i].Data4[3],
			result.rgGuidDB[i].Data4[4],
			result.rgGuidDB[i].Data4[5],
			result.rgGuidDB[i].Data4[6],
			result.rgGuidDB[i].Data4[7]
			);
		}
	}

	pSdbReleaseMatchingExe(hSDB, (TAGREF)&result);
	FreeLibrary(hAppHelp);
	return sBuf;
}

