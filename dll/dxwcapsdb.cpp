/* ------------------------------------------------------------------ */
// DirectDraw Objects capability DB
/* ------------------------------------------------------------------ */

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include "dxwnd.h"
#include "dxwcore.hpp"

//#define DXW_SURFACE_STACK_TRACING
//#define OutTraceSDB OutTrace

dxwCapsDB::dxwCapsDB()
{
	memset(CapsDB, 0, sizeof(CapsDB));
}

dxwCapsDB::~dxwCapsDB()
{
}

void dxwCapsDB::PushCaps(LPDIRECTDRAWSURFACE ps, DWORD dwCaps)
{
	int i;
	CapsDB_Type *e;
#ifdef DXW_SURFACE_STACK_TRACING
	OutTraceSDB(">>> CAPSDB MARK: lpdds=%x%s caps=%x(%s)\n", ps, dwCaps, ExplainDDCaps(dwCaps));
#endif
	for (i=0;i<DDSQLEN;i++) {
		e=&CapsDB[i];
		if ((e->lpdds==ps) || (e->lpdds==(DWORD)0)) break; // got matching entry or end of the list
	}
	if(i == DDSQLEN) {
		//MessageBox(0, "Caps stack is full", "DxWnd SurfaceList", MB_OK | MB_ICONEXCLAMATION);
		//return;
		for(int j=0;j<DDSQLEN-1;j++) CapsDB[j]=CapsDB[j+1]; // scale down the whole stack one entry
		e=&CapsDB[DDSQLEN-1];
	}
	e->lpdds=ps;
	e->dwCaps = dwCaps;
}
	
DWORD dxwCapsDB::GetCaps(LPDIRECTDRAWSURFACE ps)
{
	int i;
	// look for entry
	for (i=0;i<DDSQLEN;i++) {
		if (CapsDB[i].lpdds==0) return 0;
		if (CapsDB[i].lpdds==ps) break; 
	}
	// if found, delete it by left-copying each entry until end of array
	if (CapsDB[i].lpdds==ps){
#ifdef DXW_SURFACE_STACK_TRACING
		OutTraceSDB(">>> CAPSDB GETCAPS: i=%d lpdds=%x caps=%x(%s)\n", 
			i, ps, CapsDB[i].dwCaps, ExplainDDCaps(CapsDB[i].dwCaps));
#endif
		return CapsDB[i].dwCaps;
	}
	return 0;
}

