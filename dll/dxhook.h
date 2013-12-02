extern int HookDirectDraw(char *, int);
extern int HookDDProxy(char *, int);
extern int HookDirect3D(char *, int);
extern int HookOle32(char *, int);
extern int HookDirectInput(char *, int);
extern void HookImeLib(char *);
extern void InitPosition(int, int, int, int, int, int);
//void InitWindowPos(int, int, int, int);
extern LPCSTR ProcToString(LPCSTR proc);
