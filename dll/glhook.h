#include "gl.h"

extern FARPROC Remap_gl_ProcAddress(LPCSTR, HMODULE);
extern void HookOpenGLLibs(int, char *);

typedef void (WINAPI *glViewport_Type)(GLint, GLint, GLsizei, GLsizei);
typedef void (WINAPI *glScissor_Type)(GLint, GLint, GLsizei, GLsizei);
typedef void (WINAPI *glGetIntegerv_Type)(GLenum, GLint *);

extern void WINAPI extglViewport(GLint,  GLint,  GLsizei,  GLsizei);
extern void WINAPI extglScissor(GLint,  GLint,  GLsizei,  GLsizei);
extern void WINAPI extglGetIntegerv(GLenum, GLint *);

extern glViewport_Type pglViewport;
extern glScissor_Type pglScissor;
extern glGetIntegerv_Type pglGetIntegerv;