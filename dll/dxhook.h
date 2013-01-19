int HookDirectDraw(char *, int);
int HookDDProxy(char *, int);
int HookDirect3D(char *, int);
int HookOle32(char *, int);
int HookDirectInput(char *, int);
void InitPosition(int, int, int, int, int, int);
//void InitWindowPos(int, int, int, int);
extern LPCSTR ProcToString(LPCSTR proc);
