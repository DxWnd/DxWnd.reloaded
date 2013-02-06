extern int HookDirectDraw(HMODULE, int);
extern int HookDDProxy(HMODULE, int);
extern int HookDirect3D(HMODULE, int);
extern int HookOle32(HMODULE, int);
extern int HookDirectInput(HMODULE, int);
extern void HookImeLib(HMODULE);
extern void HookKernel32(HMODULE);

extern void InitPosition(int, int, int, int, int, int);
//void InitWindowPos(int, int, int, int);
extern LPCSTR ProcToString(LPCSTR proc);
extern HRESULT HookDxDiag(REFIID, LPVOID FAR*);

extern FARPROC Remap_ddraw_ProcAddress(LPCSTR, HMODULE);
extern FARPROC Remap_d3d8_ProcAddress(LPCSTR, HMODULE);
extern FARPROC Remap_d3d9_ProcAddress(LPCSTR, HMODULE);
extern FARPROC Remap_d3d10_ProcAddress(LPCSTR, HMODULE);
extern FARPROC Remap_d3d10_1_ProcAddress(LPCSTR, HMODULE);
extern FARPROC Remap_d3d11_ProcAddress(LPCSTR, HMODULE);
extern FARPROC Remap_gl_ProcAddress(LPCSTR, HMODULE);
extern FARPROC Remap_smack_ProcAddress(LPCSTR, HMODULE);
extern FARPROC Remap_user32_ProcAddress(LPCSTR, HMODULE);
extern FARPROC Remap_kernel32_ProcAddress(LPCSTR, HMODULE);
extern FARPROC Remap_ole32_ProcAddress(LPCSTR, HMODULE);

