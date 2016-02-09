#define _CRT_SECURE_NO_WARNINGS

#include "stdio.h"
#include "dxwnd.h"
#include "dxwcore.hpp"
#include "syslibs.h"
#include "dxhook.h"
#include "gl\gl.h"
#include "gl\wglext.h"
#include "gl\glext.h"

#define DXWDECLARATIONS TRUE
#include "glhook.h"
#undef DXWDECLARATIONS

#ifndef COMPRESSED_RGB_S3TC_DXT1_EXT
#define COMPRESSED_RGB_S3TC_DXT1_EXT                   0x83F0
#define COMPRESSED_RGBA_S3TC_DXT1_EXT                  0x83F1
#define COMPRESSED_RGBA_S3TC_DXT3_EXT                  0x83F2
#define COMPRESSED_RGBA_S3TC_DXT5_EXT                  0x83F3
#endif

//void WINAPI extglDrawPixels(GLsizei, GLsizei, GLenum, GLenum, const GLvoid *);
//typedef void (WINAPI *glDrawPixels_Type)(GLsizei, GLsizei, GLenum, GLenum, const GLvoid *);
//glDrawPixels_Type pglDrawPixels = NULL;

static HookEntryEx_Type Hooks[]={
	{HOOK_IAT_CANDIDATE, 0, "glGetError", NULL, (FARPROC *)&pglGetError, (FARPROC)extglGetError},
	{HOOK_IAT_CANDIDATE, 0, "glViewport", NULL, (FARPROC *)&pglViewport, (FARPROC)extglViewport},
	{HOOK_IAT_CANDIDATE, 0, "glScissor", NULL, (FARPROC *)&pglScissor, (FARPROC)extglScissor},
	{HOOK_IAT_CANDIDATE, 0, "glGetIntegerv", NULL, (FARPROC *)&pglGetIntegerv, (FARPROC)&extglGetIntegerv},
	{HOOK_IAT_CANDIDATE, 0, "glDrawBuffer", NULL, (FARPROC *)&pglDrawBuffer, (FARPROC)extglDrawBuffer},
	{HOOK_IAT_CANDIDATE, 0, "glPolygonMode", NULL, (FARPROC *)&pglPolygonMode, (FARPROC)extglPolygonMode},
	{HOOK_IAT_CANDIDATE, 0, "glGetFloatv", NULL, (FARPROC *)&pglGetFloatv, (FARPROC)extglGetFloatv},
	{HOOK_IAT_CANDIDATE, 0, "glClear", NULL, (FARPROC *)&pglClear, (FARPROC)extglClear},
	{HOOK_IAT_CANDIDATE, 0, "wglCreateContext", NULL, (FARPROC *)&pwglCreateContext, (FARPROC)extwglCreateContext},
	{HOOK_IAT_CANDIDATE, 0, "wglMakeCurrent", NULL, (FARPROC *)&pwglMakeCurrent, (FARPROC)extwglMakeCurrent},
	{HOOK_IAT_CANDIDATE, 0, "wglGetProcAddress", NULL, (FARPROC *)&pwglGetProcAddress, (FARPROC)extwglGetProcAddress},
	{HOOK_IAT_CANDIDATE, 0, "glTexImage2D", NULL, (FARPROC *)&pglTexImage2D, (FARPROC)extglTexImage2D},
	//{HOOK_IAT_CANDIDATE, 0, "glDrawPixels", NULL, (FARPROC *)&pglDrawPixels, (FARPROC)extglDrawPixels},
	{HOOK_IAT_CANDIDATE, 0, "glPixelZoom", NULL, (FARPROC *)&pglPixelZoom, (FARPROC)extglPixelZoom},
	//{HOOK_IAT_CANDIDATE, 0, "glBegin", NULL, (FARPROC *)&pglBegin, (FARPROC)extglBegin},
	{HOOK_IAT_CANDIDATE, 0, "glBindTexture", NULL, (FARPROC *)&pglBindTexture, (FARPROC)extglBindTexture},
	{HOOK_IAT_CANDIDATE, 0, 0, NULL, 0, 0} // terminator
};

FARPROC Remap_gl_ProcAddress(LPCSTR proc, HMODULE hModule)
{
	FARPROC addr;
	if(!(dxw.dwFlags2 & HOOKOPENGL)) return NULL; 
	if (addr=RemapLibraryEx(proc, hModule, Hooks)) return addr;
	// NULL -> keep the original call address
	return NULL;
}

PROC Remap_wgl_ProcAddress(LPCSTR proc)
{
	int i;
	HookEntryEx_Type *Hook;
	if(!(dxw.dwFlags2 & HOOKOPENGL)) return NULL; 
	for(i=0; Hooks[i].APIName; i++){
		Hook=&Hooks[i];
		if (!strcmp(proc,Hook->APIName)){
			if (Hook->StoreAddress) { // avoid clearing function pointers
				PROC Addr = (*pwglGetProcAddress)(proc);
				if(Addr) *(Hook->StoreAddress)=Addr;
			}
			OutTraceDW("GetProcAddress: hooking proc=%s at addr=%x\n", ProcToString(proc), (Hook->StoreAddress) ? *(Hook->StoreAddress) : 0);
			return Hook->HookerAddress;
		}
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

	int i;
	HookEntryEx_Type *Hook;
	for(i=0; Hooks[i].APIName; i++){
		Hook=&Hooks[i];
		Hook->OriginalAddress = GetProcAddress(hGlLib, Hook->APIName);
		if(Hook->OriginalAddress) {
			HookAPI(base, "opengl32", Hook->StoreAddress, Hook->APIName, Hook->HookerAddress);
		}
	}
	return;
}

void HookOpenGL(HMODULE module, char *customlib)
{
	if(!(dxw.dwFlags2 & HOOKOPENGL)) return;

	char *DefOpenGLModule="OpenGL32.dll";

	if (!customlib) customlib=DefOpenGLModule;

	OutTraceDW("HookOpenGL module=%x lib=\"%s\" forced=%x\n", module, customlib, (dxw.dwFlags3 & FORCEHOOKOPENGL)?1:0);
	if (dxw.dwFlags3 & FORCEHOOKOPENGL) 
		ForceHookOpenGL(module);
	else
		HookLibraryEx(module, Hooks, customlib);

	return;
}

GLenum WINAPI extglGetError()
{
	// to avoid dependencies on opengl32.dll
	if (pglGetError) return (*pglGetError)();
	return GL_NO_ERROR;
}

void WINAPI extglViewport(GLint  x,  GLint  y,  GLsizei  width,  GLsizei  height)
{
	RECT client;
	POINT p={0,0};
	HWND hwnd;
	//if (dxw.dwFlags2 & HANDLEFPS) if(dxw.HandleFPS()) return;
	hwnd=dxw.GethWnd();
	(*pGetClientRect)(hwnd, &client);
	OutTraceDW("glViewport: declared pos=(%d,%d) size=(%d,%d)\n", x, y, width, height);
	if(IsDebug) OutTrace("glViewport: DEBUG hwnd=%x win=(%d,%d) screen=(%d,%d)\n",
		hwnd, client.right, client.bottom, dxw.GetScreenWidth(), dxw.GetScreenHeight());
	if(x==CW_USEDEFAULT) x=0;
	if(y==CW_USEDEFAULT) y=0;
	x = (x * (GLint)client.right) / (GLint)dxw.GetScreenWidth();
	y = (y * (GLint)client.bottom) / (GLint)dxw.GetScreenHeight();
	width = (width * (GLint)client.right) / (GLint)dxw.GetScreenWidth();
	height = (height * (GLint)client.bottom) / (GLint)dxw.GetScreenHeight();
	OutTraceDW("glViewport: remapped pos=(%d,%d) size=(%d,%d)\n", x, y, width, height);
	(*pglViewport)(x, y, width, height);
}

void WINAPI extglScissor(GLint  x,  GLint  y,  GLsizei  width,  GLsizei  height)
{
	RECT client;
	POINT p={0,0};
	//if (dxw.dwFlags2 & HANDLEFPS) if(dxw.HandleFPS()) return;
	(*pGetClientRect)(dxw.GethWnd(), &client);
	OutTraceDW("glScissor: declared pos=(%d,%d) size=(%d,%d)\n", x, y, width, height);
	x = (x * (GLint)client.right) / (GLint)dxw.GetScreenWidth();
	y = (y * (GLint)client.bottom) / (GLint)dxw.GetScreenHeight();
	width = (width * (GLint)client.right) / (GLint)dxw.GetScreenWidth();
	height = (height * (GLint)client.bottom) / (GLint)dxw.GetScreenHeight();
	OutTraceDW("glScissor: remapped pos=(%d,%d) size=(%d,%d)\n", x, y, width, height);
	(*pglScissor)(x, y, width, height);
}

void WINAPI extglGetIntegerv(GLenum pname, GLint *params)
{
	(*pglGetIntegerv)(pname, params);
	OutTraceB("glGetIntegerv: pname=%d\n", pname);
	if(pname == GL_VIEWPORT){
		RECT client;
		OutTraceDW("glGetIntegerv(GL_VIEWPORT): pos=(%i,%i) siz=(%i,%i)\n", params[0], params[1], params[2], params[3]);
		(*pGetClientRect)(dxw.GethWnd(), &client);
		params[0] = (params[0] * dxw.GetScreenWidth()) / client.right;
		params[1] = (params[1] * dxw.GetScreenHeight()) / client.bottom;
		params[2] = (params[2] * dxw.GetScreenWidth()) / client.right;
		params[3] = (params[3] * dxw.GetScreenHeight()) / client.bottom;
		OutTraceDW("glGetIntegerv(GL_VIEWPORT): FIXED pos=(%i,%i) siz=(%i,%i)\n", params[0], params[1], params[2], params[3]);
	}
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
	dxw.ShowOverlay();
}

void WINAPI extglPolygonMode(GLenum face, GLenum mode)
{
	OutTraceDW("glPolygonMode: face=%x mode=%x\n", face, mode);
	//OutTraceDW("glPolygonMode: extglPolygonMode=%x pglPolygonMode=%x\n", extglPolygonMode, pglPolygonMode);
	if(dxw.dwFlags2 & WIREFRAME) mode = GL_LINE; // trick to set wireframe mode....
	(*pglPolygonMode)(face, mode);
	return;
}

void WINAPI extglGetFloatv(GLenum pname, GLboolean *params)
{
	OutTraceDW("glGetFloatv: pname=%x\n", pname);
	(*pglGetFloatv)(pname, params);
	return;
}

void WINAPI extglClear(GLbitfield mask)
{
	(*pglClear)(mask);
	return;
}

//BEWARE: SetPixelFormat must be issued on the same hdc used by OpenGL wglCreateContext, otherwise 
// a failure err=2000 ERROR INVALID PIXEL FORMAT occurs!!

HGLRC WINAPI extwglCreateContext(HDC hdc)
{
	HGLRC ret;
	OutTraceDW("wglCreateContext: hdc=%x\n", hdc);
	// v2.02.31: don't let it use desktop hdc
	if(dxw.IsRealDesktop(WindowFromDC(hdc))){
		HDC oldhdc = hdc;
		hdc=(*pGDIGetDC)(dxw.GethWnd());
		OutTraceDW("wglCreateContext: remapped desktop hdc=%x->%x hWnd=%x\n", oldhdc, hdc, dxw.GethWnd());
	}
	ret=(*pwglCreateContext)(hdc);
	if(ret){
		HWND hwnd;
		hwnd=WindowFromDC(hdc);
		dxw.SethWnd(hwnd);
		OutTraceDW("wglCreateContext: SET hwnd=%x\n", hwnd);
	}
	else {
		OutTraceDW("wglCreateContext: ERROR err=%d\n", GetLastError());
	}
	return ret;
}

PROC WINAPI extwglGetProcAddress(LPCSTR proc)
{
	PROC procaddr;

	OutTraceDW("wglGetProcAddress: proc=%s\n", proc);
	procaddr=Remap_wgl_ProcAddress(proc);
	if (!procaddr) procaddr=(*pwglGetProcAddress)(proc);
	return procaddr;
}

BOOL WINAPI extwglMakeCurrent(HDC hdc, HGLRC hglrc)
{
	BOOL ret;

	OutTraceDW("wglMakeCurrent: hdc=%x hglrc=%x\n", hdc, hglrc);
	// v2.02.31: don't let it use desktop hdc
	if(dxw.IsDesktop(WindowFromDC(hdc))){
		HDC oldhdc = hdc;
		hdc=(*pGDIGetDC)(dxw.GethWnd());
		OutTraceDW("wglMakeCurrent: remapped desktop hdc=%x->%x\n", oldhdc, hdc);
	}
	ret=(*pwglMakeCurrent)(hdc, hglrc);
	if(ret){
		HWND hWnd;
		hWnd = WindowFromDC(hdc);
		OutTraceDW("wglMakeCurrent: setting hwnd=%x\n", hWnd);
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

static unsigned int Hash(BYTE *buf, int len)
{
   unsigned int b    = 378551;
   unsigned int a    = 63689;
   DWORD hash = 0;
   for(int i = 0; i < len; i++){
      hash = hash * a + buf[i];
      a    = a * b;
   }
   return hash;
}

static void glTextureDump(GLint internalFormat, GLenum Format, GLsizei w, GLsizei h, GLenum type, const GLvoid * data)
{
	int iSurfaceSize, iScanLineSize;
	static int MinTexX, MinTexY, MaxTexX, MaxTexY;
	static BOOL DoOnce = TRUE;
	char pszFile[MAX_PATH];
	WORD dwRGBBitCount;
	DWORD dwRBitMask, dwGBitMask, dwBBitMask, dwABitMask;
	char *sType;

	if(DoOnce){
		char sProfilePath[MAX_PATH];
		sprintf_s(sProfilePath, MAX_PATH, "%s\\dxwnd.ini", GetDxWndPath());
		MinTexX=GetPrivateProfileInt("Texture", "MinTexX", 0, sProfilePath);
		MaxTexX=GetPrivateProfileInt("Texture", "MaxTexX", 0, sProfilePath);
		MinTexY=GetPrivateProfileInt("Texture", "MinTexY", 0, sProfilePath);
		MaxTexY=GetPrivateProfileInt("Texture", "MaxTexY", 0, sProfilePath);
		OutTrace("TextureDump: size min=(%dx%d) max=(%dx%d)\n", MinTexX, MinTexY, MaxTexX, MaxTexY);
		sprintf_s(pszFile, MAX_PATH, "%s\\texture.out", GetDxWndPath());
		CreateDirectory(pszFile, NULL);
		DoOnce = FALSE;
	}

	if((w < MinTexX) || (h < MinTexY)) return;
	if((w > MaxTexX) || (h > MaxTexY)) return;

	if(internalFormat != 4) return; // the only safe for now ....

	// temporary ....
					dwRBitMask = 0x000000FF;
					dwGBitMask = 0x0000FF00;
					dwBBitMask = 0x00FF0000;
					dwABitMask = 0xFF000000;
					dwRGBBitCount = 32;

	// format: GL_RED?, GL_GREEN?, GL_BLUE?, GL_RG?, GL_RGB?, GL_BGR?, GL_RGBA?, GL_BGRA
	// type: GL_UNSIGNED_BYTE?, GL_BYTE?, GL_UNSIGNED_SHORT?, GL_SHORT?, GL_UNSIGNED_INT?, GL_INT?, 
	// GL_FLOAT?, GL_UNSIGNED_BYTE_3_3_2?, GL_UNSIGNED_BYTE_2_3_3_REV?, GL_UNSIGNED_SHORT_5_6_5?, GL_UNSIGNED_SHORT_5_6_5_REV?,
	// GL_UNSIGNED_SHORT_4_4_4_4?, GL_UNSIGNED_SHORT_4_4_4_4_REV?, GL_UNSIGNED_SHORT_5_5_5_1?, GL_UNSIGNED_SHORT_1_5_5_5_REV?, 
	// GL_UNSIGNED_INT_8_8_8_8?, GL_UNSIGNED_INT_8_8_8_8_REV?, GL_UNSIGNED_INT_10_10_10_2?, GL_UNSIGNED_INT_2_10_10_10_REV
	sType = "unknown";
	dwRGBBitCount = 0;
	switch(Format){
		case GL_RGB:
		case GL_RGBA:
			switch(type){
				case GL_BYTE:
				case GL_UNSIGNED_BYTE: // Daikatana, Doom 3
					dwRBitMask = 0x000000FF;
					dwGBitMask = 0x0000FF00;
					dwBBitMask = 0x00FF0000;
					dwABitMask = (Format == GL_RGBA) ? 0xFF000000 : 0x00000000;
					dwRGBBitCount = 32;
					sType = (Format == GL_RGBA) ? "RGBA888" : "RGB888";
					break;
			}
			break;
		case GL_BGR:
		case GL_BGRA:
			switch(type){
				case GL_BYTE:
				case GL_UNSIGNED_BYTE: // ???
					dwRBitMask = 0x00FF0000;
					dwGBitMask = 0x0000FF00;
					dwBBitMask = 0x000000FF;
					dwABitMask = (Format == GL_RGBA) ? 0xFF000000 : 0x00000000;
					dwRGBBitCount = 32;
					sType = (Format == GL_RGBA) ? "RGBA888" : "RGB888";
					break;
			}
			break;
	}

	if(!dwRGBBitCount) return;
	iSurfaceSize = w * h;

	while (TRUE) {
		FILE *hf;
		BITMAPFILEHEADER hdr;       // bitmap file-header 
		BITMAPV4HEADER pbi;			// bitmap info-header  

		memset((void *)&pbi, 0, sizeof(BITMAPV4HEADER));
		pbi.bV4Size = sizeof(BITMAPV4HEADER); 
		pbi.bV4Width = w;
		pbi.bV4Height = h;
		pbi.bV4BitCount = dwRGBBitCount;
		pbi.bV4SizeImage = ((w * dwRGBBitCount + 0x1F) & ~0x1F)/8 * h; 
		pbi.bV4Height = - h;
		pbi.bV4Planes = 1;
		pbi.bV4V4Compression = BI_BITFIELDS;
		if(pbi.bV4BitCount == 8) pbi.bV4V4Compression = BI_RGB;
		pbi.bV4XPelsPerMeter = 1;
		pbi.bV4YPelsPerMeter = 1;
		pbi.bV4ClrUsed = 0;
		if(pbi.bV4BitCount == 8) pbi.bV4ClrUsed = 256;
		pbi.bV4ClrImportant = 0;
		pbi.bV4RedMask = dwRBitMask;
		pbi.bV4GreenMask = dwGBitMask;
		pbi.bV4BlueMask = dwBBitMask;
		pbi.bV4AlphaMask = dwABitMask;
		pbi.bV4CSType = LCS_CALIBRATED_RGB;
		iScanLineSize = ((w * dwRGBBitCount + 0x1F) & ~0x1F)/8; 

		// calculate the bitmap hash
		OutTraceB("glTextureDump: hash linesize=%d h=%d\n", iScanLineSize, h);
		DWORD hash = Hash((BYTE *)data, iScanLineSize * h);
		
		// Create the .BMP file. 
		//sprintf_s(pszFile, MAX_PATH, "%s\\texture.out\\texture.%03d.%03d.%s.%08X.bmp", 
		//	GetDxWndPath(), w, h, sType, hash);
		sprintf_s(pszFile, MAX_PATH, "%s\\texture.out\\texture.I%x.F%x.T%x.%03d.%03d.%08X.bmp", 
			GetDxWndPath(), internalFormat, Format, type, w, h, hash);
		OutTrace("writing to %s\n", pszFile);
		hf = fopen(pszFile, "wb");
		if(!hf) break;

		hdr.bfType = 0x4d42;        // 0x42 = "B" 0x4d = "M"  
		// Compute the size of the entire file.  
		hdr.bfSize = (DWORD) (sizeof(BITMAPFILEHEADER) + pbi.bV4Size + pbi.bV4ClrUsed * sizeof(RGBQUAD) + pbi.bV4SizeImage); 
		hdr.bfReserved1 = 0; 
		hdr.bfReserved2 = 0; 

		// Compute the offset to the array of color indices.  
		hdr.bfOffBits = (DWORD) sizeof(BITMAPFILEHEADER) + pbi.bV4Size + pbi.bV4ClrUsed * sizeof (RGBQUAD); 

		// Copy the BITMAPFILEHEADER into the .BMP file.  
		fwrite((LPVOID)&hdr, sizeof(BITMAPFILEHEADER), 1, hf);

		// Copy the BITMAPINFOHEADER array into the file.  
		fwrite((LPVOID)&pbi, sizeof(BITMAPV4HEADER), 1, hf);

		// Copy the array of color indices into the .BMP file.  
		//for(int y=0; y<(int)ddsd.dwHeight; y++)
		//	fwrite((BYTE *)ddsd.lpSurface + (y*ddsd.lPitch), iScanLineSize, 1, hf);
		fwrite((BYTE *)data, iScanLineSize, h, hf);

		// Close the .BMP file.  
		fclose(hf);
		break;
	}
}


void WINAPI extglTexImage2D(
	GLenum target,
  	GLint level,
  	GLint internalFormat,
  	GLsizei width,
  	GLsizei height,
  	GLint border,
  	GLenum format,
  	GLenum type,
  	const GLvoid * data)
{
	OutTraceDW("glTexImage2D: TEXTURE target=%x level=%x internalformat=%x format=%x type=%x size=(%dx%d)\n", 
		target, level, internalFormat, format, type, width, height);

	switch(target){
		//case GL_PROXY_TEXTURE_RECTANGLE:
		//case GL_PROXY_TEXTURE_2D:
		case GL_TEXTURE_2D:
		case GL_TEXTURE_RECTANGLE:
			switch(dxw.dwFlags5 & TEXTUREMASK){
				default:
				case TEXTUREHIGHLIGHT: 
					//glTextureHighlight(s);
					break;
				case TEXTUREDUMP: 
					glTextureDump(internalFormat, format, width, height, type, data);
					break;
				case TEXTUREHACK:
					//glTextureHack(...);
					break;
				case TEXTURETRANSP:
					//glTextureTransp(...);
					break;
			}
			break;
	}

	if(dxw.dwFlags4 & NOTEXTURES) return;

	return (*pglTexImage2D)(target, level, internalFormat, width, height, border, format, type, data);
}

#if 0
char *ExplainDrawPixelsFormat(DWORD c)
{
	static char *eb;
	switch(c)
	{
		case GL_COLOR_INDEX: 				eb="GL_COLOR_INDEX"; break;
		case GL_STENCIL_INDEX: 				eb="GL_STENCIL_INDEX"; break;
		case GL_DEPTH_COMPONENT: 			eb="GL_DEPTH_COMPONENT"; break;
		case GL_RGB: 						eb="GL_RGB"; break;
		case GL_BGR: 						eb="GL_BGR"; break;
		case GL_RGBA: 						eb="GL_RGBA"; break;
		case GL_BGRA: 						eb="GL_BGRA"; break;
		case GL_RED: 						eb="GL_RED"; break;
		case GL_GREEN: 						eb="GL_GREEN"; break;
		case GL_BLUE: 						eb="GL_BLUE"; break;
		case GL_ALPHA: 						eb="GL_ALPHA"; break;
		case GL_LUMINANCE: 					eb="GL_LUMINANCE"; break;
		case GL_LUMINANCE_ALPHA: 			eb="GL_LUMINANCE_ALPHA"; break;
		default: 							eb="unknown"; break;
	}
	return eb;
}

void WINAPI extglDrawPixels(GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *data)
{
	GLenum glerr;
	OutTrace("glDrawPixels: size=(%dx%d) format=%x(%s) type=%d data=%x\n", 
		width, height, format, ExplainDrawPixelsFormat(format), type, data);

	(*pglDrawPixels)(width, height, format, type, data);
	if ((glerr=extglGetError())!= GL_NO_ERROR) OutTrace("GLERR %d ad %d\n", glerr, __LINE__);
	return;
}
#endif

void WINAPI extglPixelZoom(GLfloat xfactor, GLfloat yfactor)
{
	GLenum glerr;
	OutTraceDW("glPixelZoom: x,y factor=(%f,%f)\n", xfactor, yfactor);

	if(dxw.dwFlags6 & FIXPIXELZOOM){
		RECT desktop;
		(*pGetClientRect)(dxw.GethWnd(), &desktop);
		xfactor = (xfactor * desktop.right) / dxw.GetScreenWidth();
		yfactor = (yfactor * desktop.bottom) / dxw.GetScreenHeight();
		OutTraceDW("glPixelZoom: FIXED x,y factor=(%f,%f)\n", xfactor, yfactor);
	}
	(*pglPixelZoom)(xfactor, yfactor);
	if ((glerr=extglGetError())!= GL_NO_ERROR) OutTrace("GLERR %d ad %d\n", glerr, __LINE__);
	return;
}
void WINAPI extglBegin(GLenum mode)
{
	GLenum glerr;
	OutTraceDW("glBegin: mode=%x\n", mode);

	//if(mode == GL_QUADS) mode = GL_TRIANGLES;
	(*pglBegin)(mode);
	if ((glerr=extglGetError())!= GL_NO_ERROR) OutTrace("GLERR %d ad %d\n", glerr, __LINE__);
	return;
}

void WINAPI extglBindTexture(GLenum target, GLuint texture)
{
	GLenum glerr;
	OutTraceDW("glBindTexture: target=%x texture=%x\n", target, texture);

	if(dxw.dwFlags7 & FIXBINDTEXTURE) {
		static GLuint uiLastTex = 0;
		if(uiLastTex) (*pglBindTexture)(target, 0);
		uiLastTex = texture;
	}

	(*pglBindTexture)(target, texture);
	if ((glerr=extglGetError())!= GL_NO_ERROR) OutTrace("GLERR %d ad %d\n", glerr, __LINE__);
	return;
}
