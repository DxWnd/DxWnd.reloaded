// VJoyDialog.cpp : implementation file
//

#include "stdafx.h"
#include "dxwndhost.h"
#include "VJoyDialog.h"

#define XSPAN 128
#define YSPAN 128
#define PICWIDTH 140
#define PICHEIGHT 140

// CVJoyDialog dialog

IMPLEMENT_DYNAMIC(CVJoyDialog, CDialog)

CVJoyDialog::CVJoyDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CVJoyDialog::IDD, pParent)
{
	//MessageBoxEx(0, "VJoy constructor", "Warning", MB_OK | MB_ICONEXCLAMATION, NULL);
}

CVJoyDialog::~CVJoyDialog()
{
	//MessageBoxEx(0, "VJoy destructor", "Warning", MB_OK | MB_ICONEXCLAMATION, NULL);
	//CVJoyDialog::OnOK(); // kill timer....
}

void CVJoyDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTargetDlg)
	DDX_Check(pDX, IDC_VJOYENABLED, this->m_VJoyEnabled);
	DDX_Check(pDX, IDC_CROSSENABLED, this->m_CrossEnabled);
	DDX_Check(pDX, IDC_INVERTXAXIS, this->m_InvertXAxis);
	DDX_Check(pDX, IDC_INVERTYAXIS, this->m_InvertYAxis);
	DDX_Check(pDX, IDC_B1AUTOFIRE, this->m_B1AutoFire);
	DDX_Check(pDX, IDC_B2AUTOFIRE, this->m_B2AutoFire);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CVJoyDialog, CDialog)
	ON_WM_TIMER()
END_MESSAGE_MAP()

#define IDVJoyTIMER 3
#define FINDERSIZE 5
#define CROSSSIZE 20

// CVJoyDialog message handlers

void CVJoyDialog::OnTimer(UINT_PTR nIDEvent)
{
	// IDC_VJOYPOSITION
	int x, y;
	DWORD dwVJoyStatus;
	CDialog::UpdateData(); // calls DoDataExchange
	dwVJoyStatus = GetHookInfo()->VJoyStatus;
	this->m_VJoyPresent = (dwVJoyStatus & VJOYPRESENT) ? 1 : 0;
	dwVJoyStatus &= VJOYPRESENT; // clear all BUT VJOYPRESENT!
	if(this->m_VJoyEnabled)		dwVJoyStatus |= VJOYENABLED;
	if(this->m_CrossEnabled)	dwVJoyStatus |= CROSSENABLED;
	if(this->m_InvertYAxis)		dwVJoyStatus |= INVERTYAXIS;
	if(this->m_InvertXAxis)		dwVJoyStatus |= INVERTXAXIS;
	if(this->m_B1AutoFire)		dwVJoyStatus |= B1AUTOFIRE;
	if(this->m_B2AutoFire)		dwVJoyStatus |= B2AUTOFIRE;
	if(this->m_VJoyEnabled && this->m_VJoyPresent){
		CWnd *JoyPos = this->GetDlgItem(IDC_VJOYPOSITION);
		CDC *dc = JoyPos->GetDC();
		RECT client;
		CString coord;
		JoyPos->GetClientRect(&client);
		x = (client.right/2)  + ((GetHookInfo()->joyposx * client.right) / XSPAN);
		y = (client.bottom/2) + ((GetHookInfo()->joyposy * client.bottom) / YSPAN);
		dc->FillRect(&client, Background);
		dc->SelectObject(CenterPen);
		dc->MoveTo((client.right/2)-CROSSSIZE,(client.bottom/2));
		dc->LineTo((client.right/2)+CROSSSIZE,(client.bottom/2));
		dc->MoveTo((client.right/2),(client.bottom/2)-CROSSSIZE);
		dc->LineTo((client.right/2),(client.bottom/2)+CROSSSIZE);
		dc->SelectObject(FinderPen);
		int x0, y0;
		x0 = (x-FINDERSIZE)<client.left ? client.left : x-FINDERSIZE;
		dc->MoveTo(x0,y);
		x0 = (x+FINDERSIZE)>client.right ? client.right : x+FINDERSIZE;
		dc->LineTo(x0,y);
		y0 = (y-FINDERSIZE)<client.top ? client.top : y-FINDERSIZE;
		dc->MoveTo(x,y0);
		y0 = (y+FINDERSIZE)>client.bottom ? client.bottom : y+FINDERSIZE;
		dc->LineTo(x,y0);
		dc->MoveTo(20,20);
		coord.Format("%d,%d", GetHookInfo()->joyposx, GetHookInfo()->joyposy);
		dc->TextOutA(5, 5, coord);
		dc->DeleteDC();
	}
	GetHookInfo()->VJoyStatus = dwVJoyStatus;
}

BOOL CVJoyDialog::OnInitDialog()
{
	DWORD dwVJoyStatus;

	// TODO:  Add extra initialization here
	Background = new(CBrush);
	Background->CreateSolidBrush(RGB(255, 255, 255));	// white
	CenterPen = new(CPen);
	CenterPen->CreatePen(PS_DOT, 1, RGB(255, 0, 0));
	FinderPen = new(CPen);
	FinderPen->CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
	dwVJoyStatus = GetHookInfo()->VJoyStatus;
	this->m_VJoyPresent = (dwVJoyStatus & VJOYPRESENT) ? 1 : 0;
	this->m_VJoyEnabled = (dwVJoyStatus & VJOYENABLED) ? 1 : 0;
	this->m_CrossEnabled= (dwVJoyStatus & CROSSENABLED) ? 1 : 0;
	this->m_InvertYAxis = (dwVJoyStatus & INVERTYAXIS) ? 1 : 0;
	this->m_InvertXAxis = (dwVJoyStatus & INVERTXAXIS) ? 1 : 0;
	this->m_B1AutoFire  = (dwVJoyStatus & B1AUTOFIRE) ? 1 : 0;
	this->m_B2AutoFire  = (dwVJoyStatus & B2AUTOFIRE) ? 1 : 0;
	CDialog::OnInitDialog();
	SetTimer(IDVJoyTIMER, 40, NULL);

	return TRUE;  // return TRUE unless you set the focus to a control
}

void CVJoyDialog::OnOK()
{
	char val[80];
	DWORD dwVJoyStatus;
	// stop timer
	KillTimer(IDVJoyTIMER);
	// update joystick flags
	CDialog::UpdateData(); // calls DoDataExchange
	dwVJoyStatus = GetHookInfo()->VJoyStatus;
	dwVJoyStatus &= VJOYPRESENT; // clear all BUT VJOYPRESENT!
	if(this->m_VJoyEnabled)		dwVJoyStatus |= VJOYENABLED;
	if(this->m_CrossEnabled)	dwVJoyStatus |= CROSSENABLED;
	if(this->m_InvertYAxis)		dwVJoyStatus |= INVERTYAXIS;
	if(this->m_InvertXAxis)		dwVJoyStatus |= INVERTXAXIS;
	if(this->m_B1AutoFire)		dwVJoyStatus |= B1AUTOFIRE;
	if(this->m_B2AutoFire)		dwVJoyStatus |= B2AUTOFIRE;
	sprintf_s(val, sizeof(val), "%i", dwVJoyStatus);
	WritePrivateProfileString("joystick", "flags", val, gInitPath);
	GetHookInfo()->VJoyStatus = dwVJoyStatus;
	CDialog::OnOK();
}