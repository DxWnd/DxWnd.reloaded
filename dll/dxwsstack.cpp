/* ------------------------------------------------------------------ */
// DirectDraw Surface Stack implementation
/* ------------------------------------------------------------------ */

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include "dxwnd.h"
#include "dxwcore.hpp"

//#define DXW_SURFACE_STACK_TRACING

dxwSStack::dxwSStack()
{
	lpDDSPrimary = NULL;
	lpDDSBackBuffer = NULL;
	memset(SurfaceDB, 0, sizeof(SurfaceDB));
}

dxwSStack::~dxwSStack()
{
}

static char *sRole(USHORT role)
{
	char *s;
	switch (role){
		case 0: s="(NULL)"; break; // should never happen ...
		case SURFACE_ROLE_PRIMARY: s="(PRIM)"; break;
		case SURFACE_ROLE_BACKBUFFER: s="(BACK)"; break;
		default: s="??"; break; // should never happen ...
	}
	return s;
}

#ifdef DXW_SURFACE_STACK_TRACING
static void CheckSurfaceList(SurfaceDB_Type *SurfaceDB)
{
	char sMsg[81];
	int iPCount = 0;
	int iBCount = 0;
	for (int i=0;i<DDSQLEN;i++) {
		if (SurfaceDB[i].lpdds == NULL) break;
		if ((SurfaceDB[i].uRole == SURFACE_ROLE_PRIMARY)	&& SurfaceDB[i].uRef) iPCount++;
		if ((SurfaceDB[i].uRole == SURFACE_ROLE_BACKBUFFER) && SurfaceDB[i].uRef) iBCount++;
	}
	if(iPCount > 1) {
		sprintf(sMsg, "Primary count = %d", iPCount);
		MessageBox(0, sMsg, "DxWnd SurfaceList", MB_OK | MB_ICONEXCLAMATION);
	}
	if(iBCount > 1) {
		sprintf(sMsg, "Backbuffer count = %d", iPCount);
		MessageBox(0, sMsg, "DxWnd SurfaceList", MB_OK | MB_ICONEXCLAMATION);
	}
}

static void DumpSurfaceList(SurfaceDB_Type *SurfaceDB)
{
	for (int i=0;i<DDSQLEN;i++) {
		if (SurfaceDB[i].lpdds == NULL) break;
		OutTrace("--- SURFACELIST DUMP: i=%d lpssd=%x%s ref=%d vers=%d\n", i, 
			SurfaceDB[i].lpdds, sRole(SurfaceDB[i].uRole), SurfaceDB[i].uRef, SurfaceDB[i].uVersion);
	}
}
#endif

char *dxwSStack::ExplainSurfaceRole(LPDIRECTDRAWSURFACE ps)
{
	int i;
	for (i=0;i<DDSQLEN;i++) {
		if (SurfaceDB[i].lpdds==ps) return sRole(SurfaceDB[i].uRole);
		if (SurfaceDB[i].lpdds==0) break;
	}
	// this should NEVER happen, but ...
	extern LPDIRECTDRAWSURFACE lpDDSEmu_Back, lpDDSEmu_Prim;
	if(lpDDSEmu_Back && (ps==lpDDSEmu_Back)) return "(emu.BACK)";
	if(lpDDSEmu_Prim && (ps==lpDDSEmu_Prim)) return "(emu.PRIM)";
	// ... just in case!
	return "";
}

void dxwSStack::ClearSurfaceList()
{
#ifdef DXW_SURFACE_STACK_TRACING
	OutTrace(">>> SURFACELIST CLEAR ALL\n");
#endif
	for (int i=0;i<DDSQLEN;i++) {
		SurfaceDB[i].lpdds = NULL;
		SurfaceDB[i].uRef = FALSE;
		SurfaceDB[i].uRole = 0;
		SurfaceDB[i].uVersion = 0;
	}
	lpDDSPrimary = NULL;
	lpDDSBackBuffer = NULL;
}

void dxwSStack::UnrefSurface(LPDIRECTDRAWSURFACE ps)
{
	int i;
	// look for entry
	for (i=0;i<DDSQLEN;i++) 
		if ((SurfaceDB[i].lpdds==ps) || SurfaceDB[i].lpdds==0) break; 
	// if found, delete it by left-copying each entry until end of array
	if (SurfaceDB[i].lpdds == ps) {
#ifdef DXW_SURFACE_STACK_TRACING
		OutTraceSDB(">>> SURFACELIST UNREF: lpdds=%x%s ref=%x vers=%d\n", ps, sRole(SurfaceDB[i].uRole), SurfaceDB[i].uRef, SurfaceDB[i].uVersion);
#endif
		SurfaceDB[i].uRef = FALSE;
		if(ps == lpDDSPrimary) lpDDSPrimary = GetSurfaceByRole(SURFACE_ROLE_PRIMARY);
		if(ps == lpDDSBackBuffer) lpDDSBackBuffer = GetSurfaceByRole(SURFACE_ROLE_BACKBUFFER);
	}
#ifdef DXW_SURFACE_STACK_TRACING
	DumpSurfaceList(SurfaceDB);
#endif
}

void dxwSStack::PushSurface(LPDIRECTDRAWSURFACE ps, USHORT role, USHORT version)
{
	int i;
	SurfaceDB_Type *e;
#ifdef DXW_SURFACE_STACK_TRACING
	OutTraceSDB(">>> SURFACELIST MARK: lpdds=%x%s vers=%d\n", ps, sRole(role), version);
#endif
	for (i=0;i<DDSQLEN;i++) {
		e=&SurfaceDB[i];
		if ((e->lpdds==ps) || (e->lpdds==(DWORD)0)) break; // got matching entry or end of the list
	}
	if(i == DDSQLEN) {
		MessageBox(0, "Surface stack is full", "DxWnd SurfaceList", MB_OK | MB_ICONEXCLAMATION);
		return;
	}
	e->lpdds=ps;
	e->uRole = role;
	e->uRef = TRUE;
	e->uVersion = version;
#ifdef DXW_SURFACE_STACK_TRACING
	DumpSurfaceList(SurfaceDB);
#endif
}
	
void dxwSStack::PushPrimarySurface(LPDIRECTDRAWSURFACE ps, int version)
{
	PushSurface(ps, SURFACE_ROLE_PRIMARY, (USHORT)version);
	lpDDSPrimary = ps;
}

void dxwSStack::PushBackBufferSurface(LPDIRECTDRAWSURFACE ps, int version)
{
	PushSurface(ps, SURFACE_ROLE_BACKBUFFER, (USHORT)version);
	lpDDSBackBuffer = ps;
}

void dxwSStack::PopSurface(LPDIRECTDRAWSURFACE ps)
{
	int i;
	// look for entry
	for (i=0;i<DDSQLEN;i++) {
		if (SurfaceDB[i].lpdds==0) return;
		if (SurfaceDB[i].lpdds==ps) break; 
	}
	// if found, delete it by left-copying each entry until end of array
	if (SurfaceDB[i].lpdds==ps){
#ifdef DXW_SURFACE_STACK_TRACING
		OutTraceSDB(">>> SURFACELIST CLEAR: i=%d lpdds=%x%s ref=%x vers=%d\n", 
			i, ps, sRole(SurfaceDB[i].uRole), SurfaceDB[i].uRef, SurfaceDB[i].uVersion);
#endif
		for (; i<DDSQLEN; i++){
			SurfaceDB[i]=SurfaceDB[i+1];
			if (SurfaceDB[i].lpdds==0) break;
		}
		SurfaceDB[DDSQLEN].lpdds=0;
		SurfaceDB[DDSQLEN].uRole=0;
		SurfaceDB[DDSQLEN].uRef=0;
		SurfaceDB[DDSQLEN].uVersion=0;
		if (ps == lpDDSPrimary) lpDDSPrimary = GetSurfaceByRole(SURFACE_ROLE_PRIMARY);
		if (ps == lpDDSBackBuffer) lpDDSBackBuffer = GetSurfaceByRole(SURFACE_ROLE_BACKBUFFER);
	}
#ifdef DXW_SURFACE_STACK_TRACING
	DumpSurfaceList(&SurfaceDB[0]);
#endif
}

// Note: since MS itself declares that the object refcount is not reliable and should
// be used for debugging only, it's not safe to rely on refcount==0 when releasing a
// surface to terminate its classification as primary. As an alternate and more reliable
// way, we use UnmarkPrimarySurface each time you create a new not primary surface, 
// eliminating the risk that a surface previously classified as primary and then closed
// had the same surface handle than this new one that is not primary at all.

BOOL dxwSStack::IsAPrimarySurface(LPDIRECTDRAWSURFACE ps)
{
	int i;
	// treat NULL surface ptr as a non primary
	if(!ps) return FALSE;
	for (i=0;i<DDSQLEN;i++) {
		if (SurfaceDB[i].lpdds==0) return FALSE;
		if (SurfaceDB[i].lpdds==ps) return (SurfaceDB[i].uRole == SURFACE_ROLE_PRIMARY);
	}
	return FALSE;
}

BOOL dxwSStack::IsABackBufferSurface(LPDIRECTDRAWSURFACE ps)
{
	int i;
	// treat NULL surface ptr as a non primary
	if(!ps) return FALSE;
	for (i=0;i<DDSQLEN;i++) {
		if (SurfaceDB[i].lpdds==0) return FALSE;
		if (SurfaceDB[i].lpdds==ps) return (SurfaceDB[i].uRole == SURFACE_ROLE_BACKBUFFER);
	}
	return FALSE;
}

LPDIRECTDRAWSURFACE dxwSStack::GetSurfaceByRole(USHORT role)
{
	// Get a surface marked for the desired role (either PRIMARY or BACKBUFFER) and
	// whith a not null reference counter. In case of multiple choices, it has to
	// return the most recent reference!!!
	// tested with "101 the Airborne Invasion of Normandy" and "Armored Fist 3"
	int i;
	LPDIRECTDRAWSURFACE ret = NULL;
#ifdef DXW_SURFACE_STACK_TRACING
	if (IsAssertEnabled) CheckSurfaceList(SurfaceDB);
#endif
	for (i=0;i<DDSQLEN;i++) {
		if ((SurfaceDB[i].uRole == role) && (SurfaceDB[i].uRef)) ret = SurfaceDB[i].lpdds;
		if (SurfaceDB[i].lpdds==NULL) break;
	}
	return ret;
}

LPDIRECTDRAWSURFACE dxwSStack::GetPrimarySurface(void)
{
	//return GetSurfaceByRole(SURFACE_ROLE_PRIMARY);
	return lpDDSPrimary;
}

LPDIRECTDRAWSURFACE dxwSStack::GetBackBufferSurface(void)
{
	//return GetSurfaceByRole(SURFACE_ROLE_BACKBUFFER);
	return lpDDSBackBuffer;
}
