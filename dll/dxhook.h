extern int HookDirectDraw(HMODULE, int);
extern int HookDDProxy(HMODULE, int);
extern int HookDirect3D(HMODULE, int);
extern int HookOle32(HMODULE, int);
extern int HookDirectInput(HMODULE, int);
extern void HookImeLib(HMODULE);
extern void InitPosition(int, int, int, int, int, int);
//void InitWindowPos(int, int, int, int);
extern LPCSTR ProcToString(LPCSTR proc);
