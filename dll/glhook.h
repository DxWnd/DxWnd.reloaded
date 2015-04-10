#include "gl.h"

extern void HookOpenGLLibs(HMODULE, char *);

typedef void (WINAPI *glViewport_Type)(GLint, GLint, GLsizei, GLsizei);
typedef void (WINAPI *glScissor_Type)(GLint, GLint, GLsizei, GLsizei);
typedef void (WINAPI *glGetIntegerv_Type)(GLenum, GLint *);
typedef void (WINAPI *glFinish_Type)(void);
typedef void (WINAPI *glFlush_Type)(void);
typedef void (WINAPI *glDrawBuffer_Type)(GLenum);
typedef void (WINAPI *glPolygonMode_Type)(GLenum, GLenum);
typedef void (WINAPI *glGetFloatv_Type)(GLenum, GLboolean *);
typedef void (WINAPI *glClear_Type)(GLbitfield);
typedef HGLRC (WINAPI *wglCreateContext_Type)(HDC);
typedef PROC (WINAPI *wglGetProcAddress_Type)(LPCSTR);
typedef BOOL (WINAPI *wglMakeCurrent_Type)(HDC, HGLRC);
typedef void (WINAPI *glTexImage2D_Type)(GLenum, GLint, GLint, GLsizei, GLsizei, GLint, GLenum, GLenum, const GLvoid *);
typedef void (WINAPI *glPixelZoom_Type)(GLfloat, GLfloat);

extern void WINAPI extglViewport(GLint,  GLint,  GLsizei,  GLsizei);
extern void WINAPI extglScissor(GLint,  GLint,  GLsizei,  GLsizei);
extern void WINAPI extglGetIntegerv(GLenum, GLint *);
extern void WINAPI extglFinish(void);
extern void WINAPI extglFlush(void);
extern void WINAPI extglDrawBuffer(GLenum);
extern void WINAPI extglPolygonMode(GLenum, GLenum);
extern void WINAPI extglGetFloatv(GLenum, GLboolean *);
extern void WINAPI extglClear(GLbitfield);
extern HGLRC WINAPI extwglCreateContext(HDC);
extern PROC WINAPI extwglGetProcAddress(LPCSTR);
extern BOOL WINAPI extwglMakeCurrent(HDC, HGLRC);
extern void WINAPI extglTexImage2D(GLenum, GLint, GLint, GLsizei, GLsizei, GLint, GLenum, GLenum, const GLvoid *);
extern void WINAPI extglPixelZoom(GLfloat, GLfloat);

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
DXWEXTERN glPolygonMode_Type pglPolygonMode;
DXWEXTERN glGetFloatv_Type pglGetFloatv;
DXWEXTERN glClear_Type pglClear;
DXWEXTERN wglCreateContext_Type pwglCreateContext;
DXWEXTERN wglGetProcAddress_Type pwglGetProcAddress;
DXWEXTERN wglMakeCurrent_Type pwglMakeCurrent;
DXWEXTERN glTexImage2D_Type pglTexImage2D;
DXWEXTERN glPixelZoom_Type pglPixelZoom;
