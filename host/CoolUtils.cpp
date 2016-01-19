#include "stdafx.h"
#include "CoolUtils.h"

/****************************************************************************
 *	Function Name	: gShowHideTaskBar()									*
 *	Parameters		: BOOL bHide (flag to toggle Show/Hide of Taskbar)		*
 *	Return type		: void													*
 *	Purpose			: Function is used to Show/Hide the TaskBar				*
 *	Author			: Ashutosh R. Bhatikar (ARB)							*
 *	Date written	: 20th December 2000									*
 *	Modification History :													*
 *	Date of modification					Reason							*
 *  25th December 2000				Added methods to Show/Hide menu			*
 ****************************************************************************/

void gShowHideTaskBar(BOOL bHide /*=FALSE*/)
{
	CRect rectWorkArea = CRect(0,0,0,0);
	CRect rectTaskBar = CRect(0,0,0,0);
	static HMENU hMenu = NULL;
	
	CWnd *pWnd = CWnd::FindWindow("Shell_TrayWnd", "");
	//CWnd *pDesktopWnd = CWnd::GetDesktopWindow();
	CWnd *pStart;
	pStart = NULL;

	if( bHide )
	{
		// Code to Hide the System Task Bar
		SystemParametersInfo(SPI_GETWORKAREA,
							 0,
							 (LPVOID)&rectWorkArea,
							 0);
		
		if( pWnd )
		{
			pWnd->GetWindowRect(rectTaskBar);
			rectWorkArea.bottom += rectTaskBar.Height();
			SystemParametersInfo(SPI_SETWORKAREA,
								 0,
								 (LPVOID)&rectWorkArea,
								 0);

			pWnd->ShowWindow(SW_HIDE);

			//This will retrieve the Handle of Start Button using the above 
			// handle and class name of  start Button.  
			// v2.03.52 fix: Win10 has no such a button!
			pStart = pWnd->FindWindow("Button", NULL);
			if(pStart) pStart->ShowWindow(SW_HIDE);
		}
	}
	else
	{
		// Code to Show the System Task Bar
		SystemParametersInfo(SPI_GETWORKAREA,
							 0,
							 (LPVOID)&rectWorkArea,
							 0);
		if( pWnd )
		{
			pWnd->GetWindowRect(rectTaskBar);
			rectWorkArea.bottom -= rectTaskBar.Height();
			SystemParametersInfo(SPI_SETWORKAREA,
								 0,
								 (LPVOID)&rectWorkArea,
								 0);

			pWnd->ShowWindow(SW_SHOW);

			//This will retrieve the Handle of Start Button using the above 
			// handle and class name of  start Button.  
			// v2.03.52 fix: Win10 has no such a button!
			pStart = pWnd->FindWindow("Button", NULL);
			if(pStart) pStart->ShowWindow(SW_SHOW);
		}
	}
}

/****************************************************************************
 *	Function Name	: gHideMenu()											*
 *	Parameters		: CWnd* pWnd (Window associated with the menu)			*
 *	Return type		: HMENU													*
 *	Purpose			: Function hides the menu associated with pWnd			*
 *	Author			: Ashutosh R. Bhatikar (ARB)							*
 *	Date written	: 25nd December 2000									*
 *	Modification History :													*
 *	Date of modification					Reason							*
 ****************************************************************************/

HMENU gHideMenu(/*in*/CWnd* pWnd)
{
  HMENU hMenu = ::GetMenu(pWnd->GetSafeHwnd());
  if (hMenu != NULL)
  {
    // Invalidate before SetMenu since we are going to replace
    //  the frame's client area anyway
    pWnd->Invalidate();
    pWnd->SetMenu(NULL);
    if( pWnd->IsKindOf(RUNTIME_CLASS(CFrameWnd)) )
    {
       CFrameWnd* pFrameWnd = (CFrameWnd*)pWnd;
       // avoid any idle menu processing
       pFrameWnd->m_nIdleFlags &= ~CFrameWnd::idleMenu;
    }
    return hMenu;
  }
  return NULL;
}

/****************************************************************************
 *	Function Name	: gShowMenuBack()										*
 *	Parameters		: HMENU hMenu (Handle of the menu which was hidden)		*
 *					  CWnd* pWnd (Window associated with the menu)			*
 *	Return type		: BOOL													*
 *	Purpose			: Function shows back the menu associated with pWnd		*
 *	Author			: Ashutosh R. Bhatikar (ARB)							*
 *	Date written	: 25nd December 2000									*
 *	Modification History :													*
 *	Date of modification					Reason							*
 ****************************************************************************/

BOOL gShowMenuBack(/*in*/HMENU hMenu, /*in*/CWnd* pWnd)
{
	// put the menu back in place if it was removed before
  if (hMenu != NULL)
  {
    // Invalidate before SetMenu since we are going to replace
    //  the frame's client area anyway
    pWnd->Invalidate();
    ::SetMenu(pWnd->GetSafeHwnd(), hMenu);
    return TRUE;
  }
  return FALSE;
}
