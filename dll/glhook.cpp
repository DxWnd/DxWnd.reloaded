#include "dxwnd.h"
#include "dxwcore.hpp"
#include "syslibs.h"
#include "dxhook.h"
#include "glhook.h"

glViewport_Type pglViewport;
glScissor_Type pglScissor;
glGetIntegerv_Type pglGetIntegerv;

void WINAPI extglViewport(GLint  x,  GLint  y,  GLsizei  width,  GLsizei  height)
{
	RECT client;
	(*pGetClientRect)(dxw.GethWnd(), &client);
	OutTraceD("glViewport: declared pos=(%d,%d) size=(%d,%d)\n", x, y, width, height);
	if(IsDebug) OutTrace("glViewport: DEBUG win=(%d,%d) screen=(%d,%d)\n",
		client.right, client.bottom, dxw.GetScreenWidth(), dxw.GetScreenHeight());
	x = (x * (GLint)client.right) / (GLint)dxw.GetScreenWidth();
	y = (y * (GLint)client.bottom) / (GLint)dxw.GetScreenHeight();
	width = (width * (GLint)client.right) / (GLint)dxw.GetScreenWidth();
	height = (height * (GLint)client.bottom) / (GLint)dxw.GetScreenHeight();
	OutTraceD("glViewport: remapped pos=(%d,%d) size=(%d,%d)\n", x, y, width, height);
	(*pglViewport)(x, y, width, height);
}

void WINAPI extglScissor(GLint  x,  GLint  y,  GLsizei  width,  GLsizei  height)
{
	RECT client;
	(*pGetClientRect)(dxw.GethWnd(), &client);
	OutTraceD("glScissor: declared pos=(%d,%d) size=(%d,%d)\n", x, y, width, height);
	x = (x * (GLint)client.right) / (GLint)dxw.GetScreenWidth();
	y = (y * (GLint)client.bottom) / (GLint)dxw.GetScreenHeight();
	width = (width * (GLint)client.right) / (GLint)dxw.GetScreenWidth();
	height = (height * (GLint)client.bottom) / (GLint)dxw.GetScreenHeight();
	OutTraceD("glScissor: remapped pos=(%d,%d) size=(%d,%d)\n", x, y, width, height);
	(*pglScissor)(x, y, width, height);
}

void WINAPI extglGetIntegerv(GLenum pname, GLint *params)
{
	(*pglGetIntegerv)(pname, params);
	OutTraceD("glGetIntegerv: pname=%d\n", pname);
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

void HookOpenGLLibs(char *module, char *customlib)
{
	void *tmp;
	char *DefOpenGLModule="OpenGL32.dll";

	if (!customlib) customlib=DefOpenGLModule;

	tmp = HookAPI(module, customlib, NULL, "glViewport", extglViewport);
	if(tmp) pglViewport = (glViewport_Type)tmp;
	tmp = HookAPI(module, customlib, NULL, "glScissor", extglScissor);
	if(tmp) pglScissor = (glScissor_Type)tmp;
	tmp = HookAPI(module, customlib, NULL, "glGetIntegerv", extglGetIntegerv);
	if(tmp) pglGetIntegerv = (glGetIntegerv_Type)tmp;

	return;
}

