#include "dxwnd.h"
#include "syslibs.h"
#include "dxhook.h"
#include "glhook.h"

glViewport_Type pglViewport;
glScissor_Type pglScissor;
glGetIntegerv_Type pglGetIntegerv;

//extern GLenum WINAPI glGetError();

extern short iPosX, iPosY, iSizX, iSizY;

void WINAPI extglViewport(GLint  x,  GLint  y,  GLsizei  width,  GLsizei  height)
{
	RECT client;
	(*pGetClientRect)(hWnd, &client);
	OutTraceD("glViewport: declared pos=(%d,%d) size=(%d,%d)\n", x, y, width, height);
	if(IsDebug) OutTrace("glViewport: DEBUG win=(%d,%d) screen=(%d,%d)\n",
		client.right, client.bottom, VirtualScr.dwWidth, VirtualScr.dwHeight);
	x = (x * (GLint)client.right) / (GLint)VirtualScr.dwWidth;
	y = (y * (GLint)client.bottom) / (GLint)VirtualScr.dwHeight;
	width = (width * (GLint)client.right) / (GLint)VirtualScr.dwWidth;
	height = (height * (GLint)client.bottom) / (GLint)VirtualScr.dwHeight;
	OutTraceD("glViewport: remapped pos=(%d,%d) size=(%d,%d)\n", x, y, width, height);
	(*pglViewport)(x, y, width, height);
}

void WINAPI extglScissor(GLint  x,  GLint  y,  GLsizei  width,  GLsizei  height)
{
	RECT client;
	(*pGetClientRect)(hWnd, &client);
	OutTraceD("glScissor: declared pos=(%d,%d) size=(%d,%d)\n", x, y, width, height);
	x = (x * (GLint)client.right) / (GLint)VirtualScr.dwWidth;
	y = (y * (GLint)client.bottom) / (GLint)VirtualScr.dwHeight;
	width = (width * (GLint)client.right) / (GLint)VirtualScr.dwWidth;
	height = (height * (GLint)client.bottom) / (GLint)VirtualScr.dwHeight;
	OutTraceD("glScissor: remapped pos=(%d,%d) size=(%d,%d)\n", x, y, width, height);
	(*pglScissor)(x, y, width, height);
}

void WINAPI extglGetIntegerv(GLenum pname, GLint *params)
{
	(*pglGetIntegerv)(pname, params);
	OutTraceD("glGetIntegerv: pname=%d\n", pname);
//	if(glGetError()!=GL_NO_ERROR) OutTraceE("glGetIntegerv: ERROR %d at %d\n", glGetError(), __LINE__);
#if 0
	if (pname==GL_VIEWPORT){
		OutTraceD("glGetIntegerv: pname=GL_VIEWPORT real viewport=(%d,%d)-(%d,%d)\n", 
			params[0], params[1], params[2], params[3]);
		//params[0]=(GLboolean)iPosX;
		//params[1]=(GLboolean)iPosY;
		params[0]=(GLint)0;
		params[1]=(GLint)0;
		params[2]=(GLint)iSizX;
		params[3]=(GLint)iSizY;
		OutTraceD("glGetIntegerv: pname=GL_VIEWPORT remapped viewport=(%d,%d)-(%d,%d)\n", 
			params[0], params[1], params[2], params[3]);
	}
#endif
}

FARPROC Remap_gl_ProcAddress(LPCSTR proc, HMODULE hModule)
{
	if (!strcmp(proc,"glViewport")){
		pglViewport=(glViewport_Type)(*pGetProcAddress)(hModule, proc);
		OutTraceD("GetProcAddress: hooking proc=%s at addr=%x\n", ProcToString(proc), pglViewport);
		return (FARPROC)extglViewport;
	}
	if (!strcmp(proc,"glScissor")){
		pglScissor=(glScissor_Type)(*pGetProcAddress)(hModule, proc);
		OutTraceD("GetProcAddress: hooking proc=%s at addr=%x\n", ProcToString(proc), pglScissor);
		return (FARPROC)extglScissor;
	}
	if (!strcmp(proc,"glGetIntegerv")){
		pglGetIntegerv=(glGetIntegerv_Type)(*pGetProcAddress)(hModule, proc);
		OutTraceD("GetProcAddress: hooking proc=%s at addr=%x\n", ProcToString(proc), pglGetIntegerv);
		return (FARPROC)extglGetIntegerv;
	}
	// NULL -> keep the original call address
	return NULL;
}

void HookOpenGLLibs(int dxversion, char *module)
{
	void *tmp;
	char *DefOpenGLModule="OpenGL32.dll";

	if (dxversion!=10) return;
	if (!module) module=DefOpenGLModule;

	tmp = HookAPI(module, NULL, "glViewport", extglViewport);
	if(tmp) pglViewport = (glViewport_Type)tmp;
	tmp = HookAPI(module, NULL, "glScissor", extglScissor);
	if(tmp) pglScissor = (glScissor_Type)tmp;
	tmp = HookAPI(module, NULL, "glGetIntegerv", extglGetIntegerv);
	if(tmp) pglGetIntegerv = (glGetIntegerv_Type)tmp;

	return;
}

