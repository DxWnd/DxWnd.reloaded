/* ------------------------------------------------------------------ */
// DirectDraw Surface Stack implementation
/* ------------------------------------------------------------------ */

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include "dxwnd.h"
#include "dxwcore.hpp"

// uncomment line below to activate surface stack tracing
//#define DXW_SURFACE_STACK_TRACING

#define OutTraceSDB OutTrace

dxwSStack::dxwSStack()
{
	// three variables used as cache ...
	lpDDSPrimary = NULL;
	lpDDSBackBuffer = NULL;
	lpDDSZBuffer = NULL;
	lpDDS3DRef = NULL;
	memset(SurfaceDB, 0, sizeof(SurfaceDB));
}

dxwSStack::~dxwSStack()
{
}

static char *sRole(USHORT role)
{
	char *s;
	switch (role){
		case SURFACE_ROLE_PRIMARY: s="(PRIM)"; break;
		case SURFACE_ROLE_BACKBUFFER: s="(BACK)"; break;
		case SURFACE_ROLE_ZBUFFER: s="(ZBUF)"; break;
		case SURFACE_ROLE_3DREF: s="(3DREF)"; break;
		default: s="??"; break; // should never happen ...
	}
	return s;
}

#ifdef DXW_SURFACE_STACK_TRACING
extern char*ExplainDDSCaps(DWORD);
static void CheckSurfaceList(SurfaceDB_Type *SurfaceDB)
{
	char sMsg[81];
	int iPCount = 0;
	int iBCount = 0;
	int iZCount = 0;
	for (int i=0;i<DDSQLEN;i++) {
		if (SurfaceDB[i].lpdds == NULL) break;
		if ((SurfaceDB[i].uRole == SURFACE_ROLE_PRIMARY)	&& SurfaceDB[i].uRef) iPCount++;
		if ((SurfaceDB[i].uRole == SURFACE_ROLE_BACKBUFFER) && SurfaceDB[i].uRef) iBCount++;
		if ((SurfaceDB[i].uRole == SURFACE_ROLE_ZBUFFER)    && SurfaceDB[i].uRef) iZCount++;
	}
	if(iPCount > 1) {
		sprintf(sMsg, "Primary count = %d", iPCount);
		MessageBox(0, sMsg, "DxWnd SurfaceList", MB_OK | MB_ICONEXCLAMATION);
	}
	if(iBCount > 1) {
		sprintf(sMsg, "Backbuffer count = %d", iPCount);
		MessageBox(0, sMsg, "DxWnd SurfaceList", MB_OK | MB_ICONEXCLAMATION);
	}
	if(iZCount > 1) {
		sprintf(sMsg, "Zbuffer count = %d", iZCount);
		MessageBox(0, sMsg, "DxWnd SurfaceList", MB_OK | MB_ICONEXCLAMATION);
	}
}

static void DumpSurfaceList(SurfaceDB_Type *SurfaceDB)
{
	for (int i=0;i<DDSQLEN;i++) {
		if (SurfaceDB[i].lpdds == NULL) break;
		OutTrace("--- SURFACELIST DUMP: i=%d lpssd=%x%s ref=%d vers=%d caps=%x(%s)\n", i, 
			SurfaceDB[i].lpdds, sRole(SurfaceDB[i].uRole), SurfaceDB[i].uRef, SurfaceDB[i].uVersion,
			SurfaceDB[i].dwCaps, ExplainDDSCaps(SurfaceDB[i].dwCaps));
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
	OutTrace(">>> SURFACELIST CLEAR UNREF\n");
#endif
	// v2.03.91.fx5: emptying the list entirely is no good for "Warhammer 40K Rites of War"
	// better leave the last used primary and backbuffer surfaces.
	int i, j;
	SurfaceDB_Type NewEntries[DDSQLEN];
	lpDDSPrimary = NULL;
	lpDDSBackBuffer = NULL;
	lpDDSZBuffer = NULL;
	lpDDS3DRef = NULL;
	// search for valid entries and copy to safe place
	for (i=0, j=0; i<DDSQLEN; i++) {
		if(SurfaceDB[i].lpdds == NULL) break;
		if(SurfaceDB[i].uRef) NewEntries[j++]=SurfaceDB[i];
	}
	NewEntries[j].lpdds = NULL; // terminator
	// move back to original list
	for (i=0; i<DDSQLEN; i++) {
		if(NewEntries[i].lpdds == NULL) break;
		SurfaceDB[i]=NewEntries[i];
		switch(SurfaceDB[i].uRole){
			case SURFACE_ROLE_PRIMARY: lpDDSPrimary = SurfaceDB[i].lpdds; break;
			case SURFACE_ROLE_BACKBUFFER: lpDDSBackBuffer = SurfaceDB[i].lpdds; break;
			case SURFACE_ROLE_ZBUFFER: lpDDSZBuffer = SurfaceDB[i].lpdds; break;
		}
	}
	SurfaceDB[i].lpdds = 0; // terminator
#ifdef DXW_SURFACE_STACK_TRACING
	DumpSurfaceList(SurfaceDB);
#endif
}

void dxwSStack::UnrefSurface(LPDIRECTDRAWSURFACE ps)
{
	int i;
	// look for entry
	for (i=0;i<DDSQLEN;i++) 
		if ((SurfaceDB[i].lpdds==ps) || SurfaceDB[i].lpdds==0) break; 
	// if found, delete it by left-copying each entry until end of array
	if (SurfaceDB[i].lpdds == ps) {
		SurfaceDB[i].uRef = FALSE;
#ifdef DXW_SURFACE_STACK_TRACING
		OutTraceSDB(">>> SURFACELIST UNREF: lpdds=%x%s ref=%x vers=%d\n", ps, sRole(SurfaceDB[i].uRole), SurfaceDB[i].uRef, SurfaceDB[i].uVersion);
#endif
	}
#ifdef DXW_SURFACE_STACK_TRACING
	DumpSurfaceList(SurfaceDB);
#endif
}

void dxwSStack::PushSurface(LPDIRECTDRAWSURFACE ps, USHORT role, USHORT version, DWORD dwCaps)
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
		//MessageBox(0, "Surface stack is full", "DxWnd SurfaceList", MB_OK | MB_ICONEXCLAMATION);
		//return;
		for(int j=0;j<DDSQLEN-1;j++) SurfaceDB[j]=SurfaceDB[j+1]; // scale down the whole stack one entry
		e=&SurfaceDB[DDSQLEN-1];
	}
	e->lpdds=ps;
	e->uRole = role;
	e->uRef = TRUE;
	e->uVersion = version;
	e->dwCaps = dwCaps;
	switch(e->uRole){
		case SURFACE_ROLE_PRIMARY: lpDDSPrimary = e->lpdds; break;
		case SURFACE_ROLE_BACKBUFFER: lpDDSBackBuffer = e->lpdds; break;
		case SURFACE_ROLE_ZBUFFER: lpDDSZBuffer = e->lpdds; break;
	}
#ifdef DXW_SURFACE_STACK_TRACING
	DumpSurfaceList(SurfaceDB);
#endif
}
	
void dxwSStack::PushPrimarySurface(LPDIRECTDRAWSURFACE ps, int version, DWORD dwCaps)
{
	PushSurface(ps, SURFACE_ROLE_PRIMARY, (USHORT)version, dwCaps);
}

void dxwSStack::PushBackBufferSurface(LPDIRECTDRAWSURFACE ps, int version, DWORD dwCaps)
{
	PushSurface(ps, SURFACE_ROLE_BACKBUFFER, (USHORT)version, dwCaps);
}

void dxwSStack::PushZBufferSurface(LPDIRECTDRAWSURFACE ps, int version, DWORD dwCaps)
{
	PushSurface(ps, SURFACE_ROLE_ZBUFFER, (USHORT)version, dwCaps);
}

void dxwSStack::Push3DRefSurface(LPDIRECTDRAWSURFACE ps, int version, DWORD dwCaps)
{
	PushSurface(ps, SURFACE_ROLE_3DREF, (USHORT)version, dwCaps);
}

DWORD dxwSStack::DuplicateSurface(LPDIRECTDRAWSURFACE psfrom, LPDIRECTDRAWSURFACE psto, int version)
{
	int i;
	SurfaceDB_Type *e;
	SurfaceDB_Type sentry;
#ifdef DXW_SURFACE_STACK_TRACING
	OutTraceSDB(">>> SURFACELIST DUPL: from=%x to=%x vers=%d\n", psfrom, psto, version);
#endif
	// search for source or empty slot
	for (i=0;i<DDSQLEN;i++) {
		e=&SurfaceDB[i];
		if ((e->lpdds==psfrom) || (e->lpdds==(DWORD)0)) break; // got matching entry or end of the list
	}
	// if not found, return
	if (!e->lpdds) {
#ifdef DXW_SURFACE_STACK_TRACING
		OutTraceSDB("--- NO ENTRY: from=%x\n", psfrom);
#endif
		return 0;
	}
	// save surface entry
	sentry = *e;
	// search for destination or empty slot
	for (i=0;i<DDSQLEN;i++) {
		e=&SurfaceDB[i];
		if ((e->lpdds==psto) || (e->lpdds==(DWORD)0)) break; // got matching entry or end of the list
	}
	if(i == DDSQLEN) {
		//MessageBox(0, "Surface stack is full", "DxWnd SurfaceList", MB_OK | MB_ICONEXCLAMATION);
		//return;
		for(int j=0;j<DDSQLEN-1;j++) SurfaceDB[j]=SurfaceDB[j+1]; // scale down the whole stack one entry
		e=&SurfaceDB[DDSQLEN-1];
	}
	// duplicate the entry with the new lpdds and version but old role / capabilities
	// either overriding the old entry or writing a new one
	*e = sentry;
	e->lpdds = psto;
	e->uVersion = version;
	e->uRef = TRUE;
#ifdef DXW_SURFACE_STACK_TRACING
	DumpSurfaceList(SurfaceDB);
#endif
	return e->dwCaps;
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
		SurfaceDB[DDSQLEN].dwCaps=0;
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

BOOL dxwSStack::IsAZBufferSurface(LPDIRECTDRAWSURFACE ps)
{
	int i;
	// treat NULL surface ptr as a non primary
	if(!ps) return FALSE;
	for (i=0;i<DDSQLEN;i++) {
		if (SurfaceDB[i].lpdds==0) return FALSE;
		if (SurfaceDB[i].lpdds==ps) return (SurfaceDB[i].uRole == SURFACE_ROLE_ZBUFFER);
	}
	return FALSE;
}

BOOL dxwSStack::IsA3DRefSurface(LPDIRECTDRAWSURFACE ps)
{
	int i;
	// treat NULL surface ptr as a non primary
	if(!ps) return FALSE;
	for (i=0;i<DDSQLEN;i++) {
		if (SurfaceDB[i].lpdds==0) return FALSE;
		if (SurfaceDB[i].lpdds==ps) return (SurfaceDB[i].uRole == SURFACE_ROLE_3DREF);
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

LPDIRECTDRAWSURFACE dxwSStack::GetZBufferSurface(void)
{
	//return GetSurfaceByRole(SURFACE_ROLE_BACKBUFFER);
	return lpDDSZBuffer;
}

DWORD dxwSStack::GetCaps(LPDIRECTDRAWSURFACE ps)
{
	for (int i=0;i<DDSQLEN;i++) {
		if (SurfaceDB[i].lpdds==0) return 0;
		if (SurfaceDB[i].lpdds==ps) {
#ifdef DXW_SURFACE_STACK_TRACING
			OutTraceSDB(">>> GETCAPS: lpdds=%x caps=%x(%s)\n", ps, SurfaceDB[i].dwCaps, ExplainDDSCaps(SurfaceDB[i].dwCaps));
#endif		
			return SurfaceDB[i].dwCaps;
		}
	}
	return 0;
}

SurfaceDB_Type *dxwSStack::GetSurface(LPDIRECTDRAWSURFACE ps)
{
	for (int i=0;i<DDSQLEN;i++) {
		if (SurfaceDB[i].lpdds==0) return NULL;
		if (SurfaceDB[i].lpdds==ps) {
#ifdef DXW_SURFACE_STACK_TRACING
			OutTraceSDB(">>> GETCAPS: lpdds=%x caps=%x(%s)\n", ps, SurfaceDB[i].dwCaps, ExplainDDSCaps(SurfaceDB[i].dwCaps));
#endif		
			return &SurfaceDB[i];
		}
	}
	return NULL;
}
