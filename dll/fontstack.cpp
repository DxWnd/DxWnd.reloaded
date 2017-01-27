/* ------------------------------------------------------------------ */
// Font Stack implementation
/* ------------------------------------------------------------------ */

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include "dxwnd.h"
#include "dxwcore.hpp"

dxwFStack::dxwFStack()
{
	FontDB = (FontDB_Type *)malloc(sizeof(FontDB_Type)*(MAXFONTS+1));
	memset(FontDB, 0, sizeof(FontDB));
}

dxwFStack::~dxwFStack()
{
	free(FontDB);
}

void dxwFStack::Push(HFONT font, HFONT scaled)
{
	int i;
	FontDB_Type *e;
	for (i=0;i<MAXFONTS;i++) {
		e=&FontDB[i];
		if (e->font == 0) break; // got matching entry or end of the list
	}
	if(i == DDSQLEN) {
		MessageBox(0, "Font stack is full", "DxWnd", MB_OK | MB_ICONEXCLAMATION);
		return;
	}
	e->font=font;
	e->scaled=scaled;
}
	
HFONT dxwFStack::GetScaledFont(HFONT font)
{
	int i;
	FontDB_Type *e;
	for (i=0;i<MAXFONTS;i++) {
		e=&FontDB[i];
		if (e->font == font) return e->scaled; // got matching entry 
		if (e->font == 0) return 0; // got end of the list
	}
	return 0; // got max entries
}

HFONT dxwFStack::DeleteFont(HFONT font)
{
	int i;
	HFONT scaled;
	FontDB_Type *e;
	for (i=0;i<MAXFONTS;i++) {
		e=&FontDB[i];
		if (e->font == font) {
			scaled = e->scaled;
			break; // got matching entry 
		}
	}
	if(i==MAXFONTS) return NULL;
	for (; i<MAXFONTS-1; i++) {
		FontDB[i]=FontDB[i+1];
	}
	FontDB[i].font = 0;
	FontDB[i].scaled = 0;
	return scaled;
}