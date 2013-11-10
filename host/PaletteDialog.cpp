// StatusDialog.cpp : implementation file
//

#include "stdafx.h"
#include "dxwndhost.h"
#include "PaletteDialog.h"
#include "bmpext.h"

// CPaletteDialog dialog

IMPLEMENT_DYNAMIC(CPaletteDialog, CDialog)

CPaletteDialog::CPaletteDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CPaletteDialog::IDD, pParent)
{
}

CPaletteDialog::~CPaletteDialog()
{
}

void CPaletteDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CPaletteDialog, CDialog)
	ON_WM_TIMER()
END_MESSAGE_MAP()

#define IDPaletteTIMER 2

// CPaletteDialog message handlers

void CPaletteDialog::OnTimer(UINT_PTR nIDEvent)
{
	DXWNDSTATUS DxWndStatus;
	int DxStatus;
	//extern PRIVATEMAP *pTitles; 
	extern TARGETMAP *pTargets;
	RECT Rect;
	int h, w;

	DxStatus=GetHookStatus(&DxWndStatus);
	this->GetDC()->GetWindow()->GetClientRect(&Rect);
	h=Rect.bottom - Rect.top;
	w=Rect.right - Rect.left;

	CDib dib;
	dib.ReadFromResource(IDB_PALETTE);
	if(DxStatus==DXW_RUNNING){
		for(int row=0; row<16; row++){
			for(int col=0; col<16; col++){
				PALETTEENTRY *pe = &DxWndStatus.Palette[16*row+col];
				RGBQUAD rgbq;
				rgbq.rgbBlue=pe->peBlue;
				rgbq.rgbGreen=pe->peGreen;
				rgbq.rgbRed=pe->peRed;
				rgbq.rgbReserved=0;
				dib.SetPixel(col, row, rgbq);
			}
		}
	}
	dib.Draw(this->GetDC(), CRect(0, 0, w, h), CRect(0, 0, 16, 16));
}

BOOL CPaletteDialog::OnInitDialog()
{
	CDialog::OnInitDialog();
	SetTimer(IDPaletteTIMER, 200, NULL);
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CPaletteDialog::OnOK()
{
	// TODO: Add your specialized code here and/or call the base class
	KillTimer(IDPaletteTIMER);
	// stop timer
	CDialog::OnOK();
}
