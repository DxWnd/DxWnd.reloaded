#ifndef __COOLUTILS_H
#define __COOLUTILS_H
#include "stdafx.h"

void gShowHideTaskBar(/*in*/BOOL bHide = FALSE);

HMENU gHideMenu(/*in*/CWnd* pWnd);

BOOL gShowMenuBack(/*in*/HMENU hMenu, /*in*/CWnd* pWnd);

#endif