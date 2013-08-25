#include "dxwnd.h"
#include "dxwcore.hpp"
#include "syslibs.h"
#include "dxhook.h"

#define DXWDECLARATIONS TRUE
#include "glhook.h"
#undef DXWDECLARATIONS

static HookEntry_Type Hooks[]={
	{"glViewport", NULL, (FARPROC *)&pglViewport, (FARPROC)extglViewport},
	{"glScissor", NULL, (FARPROC *)&pglScissor, (FARPROC)extglScissor},
	{"glGetIntegerv", NULL, (FARPROC *)&pglGetIntegerv, (FARPROC)&extglGetIntegerv},
	{"glDrawBuffer", NULL, (FARPROC *)&pglDrawBuffer, (FARPROC)extglDrawBuffer},
	{"glPolygonMode", NULL, (FARPROC *)&pglPolygonMode, (FARPROC)extglPolygonMode},
	{"glGetFloatv", NULL, (FARPROC *)&pglGetFloatv, (FARPROC)extglGetFloatv},
	{"glClear", NULL, (FARPROC *)&pglClear, (FARPROC)extglClear},
	{"wglCreateContext", NULL, (FARPROC *)&pwglCreateContext, (FARPROC)extwglCreateContext},
	{"wglMakeCurrent", NULL, (FARPROC *)&pwglMakeCurrent, (FARPROC)extwglMakeCurrent},
	{"wglGetProcAddress", NULL, (FARPROC *)&pwglGetProcAddress, (FARPROC)extwglGetProcAddress},
	{0, NULL, 0, 0} // terminator
};

static HookEntry_Type wglHooks[]={
	{"glViewport", NULL, (FARPROC *)&pglViewport, (FARPROC)extglViewport},
	{"glScissor", NULL, (FARPROC *)&pglScissor, (FARPROC)extglScissor},
	{"glGetIntegerv", NULL, (FARPROC *)&pglGetIntegerv, (FARPROC)&extglGetIntegerv},
	{"glDrawBuffer", NULL, (FARPROC *)&pglDrawBuffer, (FARPROC)extglDrawBuffer},
	{"glPolygonMode", NULL, (FARPROC *)&pglPolygonMode, (FARPROC)extglPolygonMode},
	{"glGetFloatv", NULL, (FARPROC *)&pglGetFloatv, (FARPROC)extglGetFloatv},
	{"glClear", NULL, (FARPROC *)&pglClear, (FARPROC)extglClear},
	{0, NULL, 0, 0} // terminator
};

FARPROC Remap_gl_ProcAddress(LPCSTR proc, HMODULE hModule)
{
	FARPROC addr;
	if(!(dxw.dwFlags2 & HOOKOPENGL)) return NULL; 
	if (addr=RemapLibrary(proc, hModule, Hooks)) return addr;
	// NULL -> keep the original call address
	return NULL;
}

static FARPROC wglRemapLibrary(LPCSTR proc, HookEntry_Type *Hooks)
{
	int i;
	HookEntry_Type *Hook;
	for(i=0; Hooks[i].APIName; i++){
		Hook=&Hooks[i];
		if (!strcmp(proc,Hook->APIName)){
			if (Hook->StoreAddress) *(Hook->StoreAddress)=(*pwglGetProcAddress)(proc);
			OutTraceD("GetProcAddress: hooking proc=%s at addr=%x\n", ProcToString(proc), (Hook->StoreAddress) ? *(Hook->StoreAddress) : 0);
			return Hook->HookerAddress;
		}
	}
	return NULL;
}

PROC Remap_wgl_ProcAddress(LPCSTR proc)
{
	FARPROC addr;
	if(addr=wglRemapLibrary(proc, wglHooks)) return addr;
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
	if (dxw.dwFlags2 & SHOWFPSOVERLAY) dxw.ShowFPS();
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
	OutTraceD("wglCreateContext: hdc=%x\n", hdc);
	// v2.02.31: don't let it use desktop hdc
	if(dxw.IsDesktop(WindowFromDC(hdc))){
		HDC oldhdc = hdc;
		hdc=(*pGDIGetDC)(dxw.GethWnd());
		OutTraceD("wglCreateContext: remapped desktop hdc=%x->%x\n", oldhdc, hdc);
	}
	ret=(*pwglCreateContext)(hdc);
	if(ret){
		HWND hwnd;
		hwnd=WindowFromDC(hdc);
		dxw.SethWnd(hwnd);
		OutTraceD("wglCreateContext: SET hwnd=%x\n", hwnd);
	}
	else {
		OutTraceD("wglCreateContext: ERROR err=%x\n", GetLastError());
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
	// v2.02.31: don't let it use desktop hdc
	if(dxw.IsDesktop(WindowFromDC(hdc))){
		HDC oldhdc = hdc;
		hdc=(*pGDIGetDC)(dxw.GethWnd());
		OutTraceD("wglMakeCurrent: remapped desktop hdc=%x->%x\n", oldhdc, hdc);
	}
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

