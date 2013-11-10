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

#if 0
void CPaletteDialog::OnTimer(UINT_PTR nIDEvent)
{
	DXWNDSTATUS DxWndStatus;
	int DxStatus;
	TARGETMAP *Target;
	CBitmap *cPal, *cMiniPal;
	extern PRIVATEMAP *pTitles; 
	extern TARGETMAP *pTargets;
	RECT Rect;
	int h, w;
	//CBitmap cMiniPal;
	//cMiniPal.CreateCompatibleBitmap(this->GetDC(), 16, 16);

	CWnd *Pix;

	DxStatus=GetHookStatus(&DxWndStatus);
	this->GetDC()->GetWindow()->GetWindowRect(&Rect);
	h=Rect.bottom - Rect.top;
	w=Rect.right - Rect.left;
	cPal=(CBitmap *)this->GetDlgItem(IDC_PALETTECOLORS);
	Pix=(CWnd *)this->GetDlgItem(IDC_PALETTEMAP);

	if(DxStatus==DXW_RUNNING){
		for(int row=0; row<16; row++)
			for(int col=0; col<16; col++){
				COLORREF color;
				PALETTEENTRY *pe = &DxWndStatus.Palette[16*row+col];
				//color=pe->peRed | (pe->peGreen << 8) | (pe->peBlue << 16);
				color=RGB(pe->peRed, pe->peGreen, pe->peBlue);

				//Pix->GetDC()->SetPixel(row, col, color);
				//this->GetDC()->StretchBlt(0, 0, w, h, Pix->GetDC(), 0, 0, 16, 16, SRCCOPY);

				this->GetDC()->SetPixel(row, col, color);
			}

	}
	else
		this->GetDC()->StretchBlt(0, 0, w, h, NULL, 0, 0, 0, 0, WHITENESS);
}
#else
void CPaletteDialog::OnTimer(UINT_PTR nIDEvent)
{
	DXWNDSTATUS DxWndStatus;
	int DxStatus;
	extern PRIVATEMAP *pTitles; 
	extern TARGETMAP *pTargets;
	RECT Rect;
	int h, w;

	CBitmap cBmp;
	CBitmap* pBitmap; 
    CDC cMemDC;
    cMemDC.CreateCompatibleDC(this->GetDC()); // Create the memory DC.

    CBitmap* pOld = cMemDC.SelectObject(&cBmp);
    pBitmap = cMemDC.SelectObject(pOld);

	DxStatus=GetHookStatus(&DxWndStatus);
	this->GetDC()->GetWindow()->GetWindowRect(&Rect);
	h=Rect.bottom - Rect.top;
	w=Rect.right - Rect.left;

	CDib dib;
	dib.ReadFromResource(IDB_PALETTE);
	if(DxStatus==DXW_RUNNING){
		for(int row=0; row<dib.Height(); row++){
			for(int col=0; col<dib.Width(); col++){
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
#endif

BOOL CPaletteDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetTimer(IDPaletteTIMER, 1000, NULL);

	// TODO:  Add extra initialization here

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
