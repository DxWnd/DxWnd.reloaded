#include "Vfw.h"

extern FARPROC Remap_vfw_ProcAddress(LPCSTR, HMODULE);
extern void HookMSV4WLibs(HMODULE);

typedef LRESULT (WINAPI * ICSendMessage_Type)(HIC, UINT, DWORD_PTR, DWORD_PTR);
typedef HIC (WINAPI *ICOpen_Type)(DWORD, DWORD, UINT);

extern LRESULT WINAPI extICSendMessage(HIC, UINT, DWORD_PTR, DWORD_PTR);
extern HIC WINAPI extICOpen(DWORD, DWORD, UINT);

#ifdef DXWDECLARATIONS 
#undef DXWEXTERN
#undef DXWINITIALIZED
#define DXWEXTERN
#define DXWINITIALIZED = NULL
#else
#undef DXWEXTERN
#undef DXWINITIALIZED
#define DXWEXTERN extern
#define DXWINITIALIZED
#endif

// MSVFW32.dll:
DXWEXTERN ICSendMessage_Type pICSendMessage;
DXWEXTERN ICOpen_Type pICOpen;
