#include "dxwnd.h"
#include "dxwcore.hpp"
#include "syslibs.h"
#include "dxhook.h"

#define DXWDECLARATIONS TRUE
#include "glhook.h"
#undef DXWDECLARATIONS

FARPROC Remap_gl_ProcAddress(LPCSTR proc, HMODULE hModule)
{
	if (!strcmp(proc,"glViewport") && !pglViewport){
		pglViewport=(glViewport_Type)(*pGetProcAddress)(hModule, proc);
		OutTraceD("GetProcAddress: hooking proc=%s at addr=%x\n", ProcToString(proc), pglViewport);
		return (FARPROC)extglViewport;
	}
	if (!strcmp(proc,"glScissor") && !pglScissor){
		pglScissor=(glScissor_Type)(*pGetProcAddress)(hModule, proc);
		OutTraceD("GetProcAddress: hooking proc=%s at addr=%x\n", ProcToString(proc), pglScissor);
		return (FARPROC)extglScissor;
	}
	if (!strcmp(proc,"glGetIntegerv") && !pglGetIntegerv){
		pglGetIntegerv=(glGetIntegerv_Type)(*pGetProcAddress)(hModule, proc);
		OutTraceD("GetProcAddress: hooking proc=%s at addr=%x\n", ProcToString(proc), pglGetIntegerv);
		return (FARPROC)extglGetIntegerv;
	}
	if (!strcmp(proc,"glDrawBuffer") && !pglDrawBuffer){
		pglDrawBuffer=(glDrawBuffer_Type)(*pGetProcAddress)(hModule, proc);
		OutTraceD("GetProcAddress: hooking proc=%s at addr=%x\n", ProcToString(proc), pglDrawBuffer);
		return (FARPROC)extglDrawBuffer;
	}
	if (!strcmp(proc,"glPolygonMode") && !pglPolygonMode){
		pglPolygonMode=(glPolygonMode_Type)(*pGetProcAddress)(hModule, proc);
		OutTraceD("GetProcAddress: hooking proc=%s at addr=%x\n", ProcToString(proc), pglPolygonMode);
		return (FARPROC)extglPolygonMode;
	}
	if (!strcmp(proc,"glGetFloatv") && !pglGetFloatv){
		pglGetFloatv=(glGetFloatv_Type)(*pGetProcAddress)(hModule, proc);
		OutTraceD("GetProcAddress: hooking proc=%s at addr=%x\n", ProcToString(proc), pglGetFloatv);
		return (FARPROC)extglGetFloatv;
	}
	if (!strcmp(proc,"glClear") && !pglClear){
		pglClear=(glClear_Type)(*pGetProcAddress)(hModule, proc);
		OutTraceD("GetProcAddress: hooking proc=%s at addr=%x\n", ProcToString(proc), pglClear);
		return (FARPROC)extglClear;
	}
	if (!strcmp(proc,"wglCreateContext") && !pwglCreateContext){
		pwglCreateContext=(wglCreateContext_Type)(*pGetProcAddress)(hModule, proc);
		OutTraceD("GetProcAddress: hooking proc=%s at addr=%x\n", ProcToString(proc), pwglCreateContext);
		return (FARPROC)extwglCreateContext;
	}
	if (!strcmp(proc,"wglMakeCurrent") && !pwglMakeCurrent){
		pwglMakeCurrent=(wglMakeCurrent_Type)(*pGetProcAddress)(hModule, proc);
		OutTraceD("GetProcAddress: hooking proc=%s at addr=%x\n", ProcToString(proc), pwglMakeCurrent);
		return (FARPROC)extwglMakeCurrent;
	}
	// NULL -> keep the original call address
	return NULL;
}

PROC Remap_wgl_ProcAddress(LPCSTR proc)
{
	if (!strcmp(proc,"glViewport")){
		pglViewport=(glViewport_Type)(*pwglGetProcAddress)(proc);
		OutTraceD("GetProcAddress: hooking proc=%s at addr=%x\n", ProcToString(proc), pglViewport);
		return (FARPROC)extglViewport;
	}
	if (!strcmp(proc,"glScissor")){
		pglScissor=(glScissor_Type)(*pwglGetProcAddress)(proc);
		OutTraceD("GetProcAddress: hooking proc=%s at addr=%x\n", ProcToString(proc), pglScissor);
		return (FARPROC)extglScissor;
	}
	if (!strcmp(proc,"glGetIntegerv")){
		pglGetIntegerv=(glGetIntegerv_Type)(*pwglGetProcAddress)(proc);
		OutTraceD("GetProcAddress: hooking proc=%s at addr=%x\n", ProcToString(proc), pglGetIntegerv);
		return (FARPROC)extglGetIntegerv;
	}
	if (!strcmp(proc,"glDrawBuffer")){
		pglDrawBuffer=(glDrawBuffer_Type)(*pwglGetProcAddress)(proc);
		OutTraceD("GetProcAddress: hooking proc=%s at addr=%x\n", ProcToString(proc), pglDrawBuffer);
		return (FARPROC)extglDrawBuffer;
	}
	if (!strcmp(proc,"glPolygonMode")){
		pglPolygonMode=(glPolygonMode_Type)(*pwglGetProcAddress)(proc);
		OutTraceD("GetProcAddress: hooking proc=%s at addr=%x\n", ProcToString(proc), pglPolygonMode);
		return (FARPROC)extglPolygonMode;
	}
	if (!strcmp(proc,"glGetFloatv")){
		pglGetFloatv=(glGetFloatv_Type)(*pwglGetProcAddress)(proc);
		OutTraceD("GetProcAddress: hooking proc=%s at addr=%x\n", ProcToString(proc), pglGetFloatv);
		return (FARPROC)extglGetFloatv;
	}
	if (!strcmp(proc,"glClear")){
		pglClear=(glClear_Type)(*pwglGetProcAddress)(proc);
		OutTraceD("GetProcAddress: hooking proc=%s at addr=%x\n", ProcToString(proc), pglClear);
		return (FARPROC)extglClear;
	}
	// NULL -> keep the original call address
	return NULL;
}

void ForceHookOpenGL(HMODULE base) // to do .....
{
	HMODULE hGlLib;
	static int DoOnce=FALSE;

	if(DoOnce) return;
	DoOnce = TRUE;

	hGlLib=(*pLoadLibraryA)("OpenGL32.dll");
	OutTrace("hGlLib=%x\n",hGlLib);
	if(!hGlLib){
		OutTraceE("LoadLibrary(\"OpenGL32.dll\") ERROR: err=%d at %d\n", GetLastError(), __LINE__);
		return;
	}

	pglViewport=(glViewport_Type)GetProcAddress(hGlLib, "glViewport");
	if(pglViewport) {
		HookAPI(base, "opengl32", pglViewport, "glViewport", extglViewport);
		extglViewport(dxw.iPosX,dxw.iPosY,dxw.iSizX,dxw.iSizY);
	}
	pwglGetProcAddress=(wglGetProcAddress_Type)GetProcAddress(hGlLib, "wglGetProcAddress");
	if(pwglGetProcAddress) {
		HookAPI(base, "opengl32", pwglGetProcAddress, "wglGetProcAddress", extwglGetProcAddress);
		extwglGetProcAddress("wglGetProcAddress");
	}
	pglScissor=(glScissor_Type)GetProcAddress(hGlLib, "glScissor");
	if(pglScissor) {
		HookAPI(base, "opengl32", pglScissor, "glScissor", extglScissor);
	}
	pglGetIntegerv=(glGetIntegerv_Type)GetProcAddress(hGlLib, "glGetIntegerv");
	if(pglGetIntegerv) {
		HookAPI(base, "opengl32", pglGetIntegerv, "glGetIntegerv", extglGetIntegerv);
	}
	pglDrawBuffer=(glDrawBuffer_Type)GetProcAddress(hGlLib, "glDrawBuffer");
	if(pglDrawBuffer) {
		HookAPI(base, "opengl32", pglDrawBuffer, "glDrawBuffer", extglDrawBuffer);
	}
	pglPolygonMode=(glPolygonMode_Type)GetProcAddress(hGlLib, "glPolygonMode");
	if(pglPolygonMode) {
		HookAPI(base, "opengl32", pglPolygonMode, "glPolygonMode", extglPolygonMode);
	}
	pglGetFloatv=(glGetFloatv_Type)GetProcAddress(hGlLib, "glGetFloatv");
	if(pglGetFloatv) {
		HookAPI(base, "opengl32", pglGetFloatv, "glGetFloatv", extglGetFloatv);
	}
	pglClear=(glClear_Type)GetProcAddress(hGlLib, "glClear");
	if(pglClear) {
		HookAPI(base, "opengl32", pglClear, "glClear", extglClear);
	}
	pwglCreateContext=(wglCreateContext_Type)GetProcAddress(hGlLib, "wglCreateContext");
	if(pwglCreateContext) {
		HookAPI(base, "opengl32", pwglCreateContext, "wglCreateContext", extwglCreateContext);
	}
	pwglMakeCurrent=(wglMakeCurrent_Type)GetProcAddress(hGlLib, "wglMakeCurrent");
	if(pwglMakeCurrent) {
		HookAPI(base, "opengl32", pwglMakeCurrent, "wglMakeCurrent", extwglMakeCurrent);
	}
}

void HookOpenGL(HMODULE module, char *customlib) 
{
	void *tmp;
	tmp = HookAPI(module, customlib, NULL, "glViewport", extglViewport);
	if(tmp) pglViewport = (glViewport_Type)tmp;
	tmp = HookAPI(module, customlib, NULL, "glScissor", extglScissor);
	if(tmp) pglScissor = (glScissor_Type)tmp;
	tmp = HookAPI(module, customlib, NULL, "glGetIntegerv", extglGetIntegerv);
	if(tmp) pglGetIntegerv = (glGetIntegerv_Type)tmp;
	tmp = HookAPI(module, customlib, NULL, "glDrawBuffer", extglDrawBuffer);
	if(tmp) pglDrawBuffer = (glDrawBuffer_Type)tmp;	
	tmp = HookAPI(module, customlib, NULL, "glPolygonMode", extglPolygonMode);
	if(tmp) pglPolygonMode = (glPolygonMode_Type)tmp;	
	tmp = HookAPI(module, customlib, NULL, "glGetFloatv", extglGetFloatv);
	if(tmp) pglGetFloatv = (glGetFloatv_Type)tmp;	
	tmp = HookAPI(module, customlib, NULL, "glClear", extglClear);
	if(tmp) pglClear = (glClear_Type)tmp;	
	tmp = HookAPI(module, customlib, NULL, "wglCreateContext", extwglCreateContext);
	if(tmp) pwglCreateContext = (wglCreateContext_Type)tmp;	
	tmp = HookAPI(module, customlib, NULL, "wglGetProcAddress", extwglGetProcAddress);
	if(tmp) pwglGetProcAddress = (wglGetProcAddress_Type)tmp;
	tmp = HookAPI(module, customlib, NULL, "wglMakeCurrent", extwglMakeCurrent);
	if(tmp) pwglMakeCurrent = (wglMakeCurrent_Type)tmp;
}

void HookOpenGLLibs(HMODULE module, char *customlib)
{
	char *DefOpenGLModule="OpenGL32.dll";

	if (!customlib) customlib=DefOpenGLModule;

	OutTraceD("HookOpenGLLibs module=%x lib=\"%s\" forced=%x\n", module, customlib, (dxw.dwFlags3 & FORCEHOOKOPENGL)?1:0);
	if (dxw.dwFlags3 & FORCEHOOKOPENGL) 
		ForceHookOpenGL(module);
	else
		HookOpenGL(module, customlib);

	return;
}

void WINAPI extglViewport(GLint  x,  GLint  y,  GLsizei  width,  GLsizei  height)
{
	RECT client;
	POINT p={0,0};
	//if (dxw.dwFlags2 & HANDLEFPS) if(dxw.HandleFPS()) return;
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
	POINT p={0,0};
	//if (dxw.dwFlags2 & HANDLEFPS) if(dxw.HandleFPS()) return;
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

void WINAPI extglDrawBuffer(GLenum mode)
{
	if (IsDebug) OutTrace("glDrawBuffer: mode=%x\n", mode);
	if(dxw.dwFlags2 & WIREFRAME) (*pglClear)(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT) ; // clear screen for wireframe mode....
	// handle FPS only to backbuffer updates (if stereo, on left backbuffer...)
	// using the frontbuffer seems less reliable: Return to Castle Wolfenstein doesn't use it at all!
	if (dxw.dwFlags2 & HANDLEFPS){
		switch (mode){
			//case GL_FRONT_LEFT:
			case GL_BACK_LEFT:
			//case GL_FRONT:
			case GL_BACK:
			case GL_LEFT:
			case GL_FRONT_AND_BACK:
				if(dxw.HandleFPS()) return;
		}
	}
	(*pglDrawBuffer)(mode);
}

void WINAPI extglPolygonMode(GLenum face, GLenum mode)
{
	OutTraceD("glPolygonMode: face=%x mode=%x\n", face, mode);
	//OutTraceD("glPolygonMode: extglPolygonMode=%x pglPolygonMode=%x\n", extglPolygonMode, pglPolygonMode);
	if(dxw.dwFlags2 & WIREFRAME) mode = GL_LINE; // trick to set wireframe mode....
	(*pglPolygonMode)(face, mode);
	return;
}

void WINAPI extglGetFloatv(GLenum pname, GLboolean *params)
{
	OutTraceD("glGetFloatv: pname=%x\n", pname);
	(*pglGetFloatv)(pname, params);
	return;
}

void WINAPI extglClear(GLbitfield mask)
{
	(*pglClear)(mask);
	return;
}

HGLRC WINAPI extwglCreateContext(HDC hdc)
{
	HGLRC ret;
	ret=(*pwglCreateContext)(hdc);
	if(ret!=NULL){
		HWND hwnd;
		hwnd=WindowFromDC(hdc);
		dxw.SethWnd(hwnd);
		OutTraceD("wglCreateContext: hdc=%x hwnd=%x\n", hdc, hwnd);
	}
	else {
		OutTraceD("wglCreateContext: ERROR hdc=%x err=%x\n", hdc, GetLastError());
	}
	return ret;
}

PROC WINAPI extwglGetProcAddress(LPCSTR proc)
{
	PROC procaddr;

	OutTraceD("wglGetProcAddress: proc=%s\n", proc);
	procaddr=Remap_wgl_ProcAddress(proc);
	if (!procaddr) procaddr=(*pwglGetProcAddress)(proc);
	return procaddr;
}

int WINAPI extChoosePixelFormat(HDC hdc, const PIXELFORMATDESCRIPTOR *ppfd)
{
	// to do ....
	return 0;
}

BOOL WINAPI extwglMakeCurrent(HDC hdc, HGLRC hglrc)
{
	BOOL ret;

	OutTraceD("wglMakeCurrent: hdc=%x hglrc=%x\n", hdc, hglrc);
	ret=(*pwglMakeCurrent)(hdc, hglrc);
	if(ret){
		HWND hWnd;
		hWnd = WindowFromDC(hdc);
		OutTraceD("wglMakeCurrent: setting hwnd=%x\n", hWnd);
		dxw.SethWnd(hWnd);
	}
	return ret;
}

// to do:
//	glutSetWindow - save current window handle
//	glutInitWindowPosition, glutInitWindowSize
//	glutInitDisplayMode
//  glutCreateWindow,  glutCreateSubWindow
//	glutPositionWindow,  glutReshapeWindow
//	glGetFloatv ( GL_SCISSOR_BOX - GL_VIEWPORT )

