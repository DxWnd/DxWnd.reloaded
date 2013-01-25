#include "gl.h"

extern FARPROC Remap_gl_ProcAddress(LPCSTR, HMODULE);
extern void HookOpenGLLibs(char *, char *);

typedef void (WINAPI *glViewport_Type)(GLint, GLint, GLsizei, GLsizei);
typedef void (WINAPI *glScissor_Type)(GLint, GLint, GLsizei, GLsizei);
typedef void (WINAPI *glGetIntegerv_Type)(GLenum, GLint *);
typedef void (WINAPI *glFinish_Type)(void);
typedef void (WINAPI *glFlush_Type)(void);
typedef void (WINAPI *glDrawBuffer_Type)(GLenum);

extern void WINAPI extglViewport(GLint,  GLint,  GLsizei,  GLsizei);
extern void WINAPI extglScissor(GLint,  GLint,  GLsizei,  GLsizei);
extern void WINAPI extglGetIntegerv(GLenum, GLint *);
extern void WINAPI extglFinish(void);
extern void WINAPI extglFlush(void);
extern void WINAPI extglDrawBuffer(GLenum);

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

// GDI32.dll:
DXWEXTERN glViewport_Type pglViewport;
DXWEXTERN glScissor_Type pglScissor;
DXWEXTERN glGetIntegerv_Type pglGetIntegerv;
//DXWEXTERN glFinish_Type pglFinish;
//DXWEXTERN glFlush_Type pglFlush;
DXWEXTERN glDrawBuffer_Type pglDrawBuffer;