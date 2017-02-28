// VJoyDialog.cpp : implementation file
//

#include "stdafx.h"
#include "dxwndhost.h"
#include "VJoyDialog.h"
#include <math.h> 

#define XSPAN 128
#define YSPAN 128

/*
	// calculates the closest m_Slider value
	int pos, delta, lastdelta;
	lastdelta=200;
	for(pos=-10; pos<=10; pos++){
		int s;
		s = (int)(100 * pow(TICKMULTIPLIER, (float)pos)); 
		delta = abs(s - this->m_VJoySensivity);
		if(delta > lastdelta) break;
		lastdelta = delta;
	}
	this->m_Slider = pos-1; 
*/

int CVJoyDialog::GetSliderPos(int sensivity)
{
	int pos;
	int delta, lastdelta;
	lastdelta = 400; // initial value big enough ...
	for(pos=-20; pos<=20; pos++){
		delta = abs(Multipliers[20+pos] - sensivity);
		if(delta > lastdelta) break;
		lastdelta = delta;
	}
	return pos-1; 
}

int CVJoyDialog::SetSensivity(int slider)
{
	if(slider < -20) slider = -20;
	if(slider >  20) slider =  20;
	return Multipliers[20+slider];
}

void CVJoyDialog::InitSensivity(void)
{
	int Multiplier;
	int pos;
	for(Multiplier = 100, pos=-1; pos>=-20; pos--) Multipliers[20+pos] = Multiplier = (Multiplier * 100) / 105; 
	for(Multiplier = 100, pos= 1; pos<= 20; pos++) Multipliers[20+pos] = Multiplier = (Multiplier * 105) / 100; 
	Multipliers[20] = 100;
}

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
	DDX_Check(pDX, IDC_VJAUTOCENTER, this->m_VJAutoCenter);
	DDX_Check(pDX, IDC_VJMOUSEWHEEL, this->m_VJMouseWheel);
	DDX_Radio(pDX, IDC_VJ_USEMOUSE, this->m_UseMode);
	DDX_Slider(pDX, IDC_SENSIVITY_SLIDER, this->m_Slider);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CVJoyDialog, CDialog)
	ON_WM_TIMER()
END_MESSAGE_MAP()

#define IDVJoyTIMER 3
#define FINDERSIZE 5
#define CROSSSIZE 20
#define TICKMULTIPLIER 1.05f

// CVJoyDialog message handlers

void CVJoyDialog::OnTimer(UINT_PTR nIDEvent)
{
	// IDC_VJOYPOSITION
	int x, y;
	DWORD dwVJoyStatus;
	DWORD dwSensivity;
	CString text;
	static DWORD dwLastSensivity = 0;

	// takes care of external updates of sensivity by mouse wheel
	dwSensivity = GetHookInfo()->VJoySensivity;
	if(dwLastSensivity && (dwSensivity != dwLastSensivity)){
		CSliderCtrl *Slider;
		this->m_Slider = this->GetSliderPos(dwSensivity);
		Slider=(CSliderCtrl *)this->GetDlgItem(IDC_SENSIVITY_SLIDER);
		Slider->SetPos(this->m_Slider);
	}

	CDialog::UpdateData(); // calls DoDataExchange
	this->m_VJoySensivity = this->SetSensivity(m_Slider);
	dwLastSensivity = this->m_VJoySensivity;

	dwVJoyStatus = GetHookInfo()->VJoyStatus;
	this->m_VJoyPresent = (dwVJoyStatus & VJOYPRESENT) ? 1 : 0;
	dwVJoyStatus &= VJOYPRESENT; // clear all BUT VJOYPRESENT!
	if(this->m_VJoyEnabled)		dwVJoyStatus |= VJOYENABLED;
	if(this->m_CrossEnabled)	dwVJoyStatus |= CROSSENABLED;
	if(this->m_InvertYAxis)		dwVJoyStatus |= INVERTYAXIS;
	if(this->m_InvertXAxis)		dwVJoyStatus |= INVERTXAXIS;
	if(this->m_B1AutoFire)		dwVJoyStatus |= B1AUTOFIRE;
	if(this->m_B2AutoFire)		dwVJoyStatus |= B2AUTOFIRE;
	if(this->m_VJAutoCenter)	dwVJoyStatus |= VJAUTOCENTER;
	if(this->m_VJMouseWheel)	dwVJoyStatus |= VJMOUSEWHEEL;
	switch(this->m_UseMode){
		case 0: dwVJoyStatus |= VJMOUSEMAP; break;
		case 1: dwVJoyStatus |= VJKEYBOARDMAP; break;
		case 2: dwVJoyStatus |= (VJKEYBOARDMAP|VJMOUSEMAP); break;
	}

	if(this->m_VJoyEnabled && this->m_VJoyPresent){
		CWnd *JoyPos = this->GetDlgItem(IDC_VJOYPOSITION);
		CDC *dc = JoyPos->GetDC();
		RECT client;
		JoyPos->GetClientRect(&client);
		x = (client.right/2)  + ((GetHookInfo()->joyposx * client.right) / XSPAN);
		y = (client.bottom/2) + ((GetHookInfo()->joyposy * client.bottom) / YSPAN);
		// clear whole square area
		dc->FillRect(&client, Background);
		// draw sensivity circle
		dc->SelectObject(SensivityPen);
		dc->Ellipse(
			(client.right)*(250-dwSensivity)/500, 
			(client.bottom)*(250-dwSensivity)/500, 
			(client.right)*(250+dwSensivity)/500, 
			(client.bottom)*(250+dwSensivity)/500);
		// draw center crossfinder
		dc->SelectObject(CenterPen);
		dc->MoveTo((client.right/2)-CROSSSIZE,(client.bottom/2));
		dc->LineTo((client.right/2)+CROSSSIZE,(client.bottom/2));
		dc->MoveTo((client.right/2),(client.bottom/2)-CROSSSIZE);
		dc->LineTo((client.right/2),(client.bottom/2)+CROSSSIZE);
		// draw joystick crossfinder
		dc->SelectObject(FinderPen);
		int x0, y0;
		if(x<client.left) x = client.left;
		if(x>client.right) x = client.right;
		if(y<client.top) y = client.top;
		if(y>client.bottom) y = client.bottom;
		x0 = (x-FINDERSIZE)<client.left ? client.left : x-FINDERSIZE;
		dc->MoveTo(x0,y);
		x0 = (x+FINDERSIZE)>client.right ? client.right : x+FINDERSIZE;
		dc->LineTo(x0,y);
		y0 = (y-FINDERSIZE)<client.top ? client.top : y-FINDERSIZE;
		dc->MoveTo(x,y0);
		y0 = (y+FINDERSIZE)>client.bottom ? client.bottom : y+FINDERSIZE;
		dc->LineTo(x,y0);
		//dc->MoveTo(20,20);
		text.Format("%d,%d", GetHookInfo()->joyposx, GetHookInfo()->joyposy);
		dc->TextOutA(5, 5, text);
		text.Format("x%d%%", GetHookInfo()->VJoySensivity);
		dc->TextOutA(5, 200, text);
		dc->DeleteDC();
	}
	GetHookInfo()->VJoyStatus = dwVJoyStatus;
	GetHookInfo()->VJoySensivity = this->m_VJoySensivity;
	dwLastSensivity = this->m_VJoySensivity;
}

BOOL CVJoyDialog::OnInitDialog()
{
	DWORD dwVJoyStatus;
	CSliderCtrl *Slider;

	// TODO:  Add extra initialization here
	Background = new(CBrush);
	Background->CreateSolidBrush(RGB(255, 255, 255));	// white
	CenterPen = new(CPen);
	CenterPen->CreatePen(PS_DOT, 1, RGB(255, 0, 0)); // red
	FinderPen = new(CPen);
	FinderPen->CreatePen(PS_SOLID, 1, RGB(0, 0, 0)); // black
	SensivityPen = new(CPen);
	SensivityPen->CreatePen(PS_DOT, 1, RGB(127, 127, 255)); // clear blue

	dwVJoyStatus = GetHookInfo()->VJoyStatus;
	this->m_VJoyPresent   = (dwVJoyStatus & VJOYPRESENT) ? 1 : 0;
	this->m_VJoyEnabled   = (dwVJoyStatus & VJOYENABLED) ? 1 : 0;
	this->m_CrossEnabled  = (dwVJoyStatus & CROSSENABLED) ? 1 : 0;
	this->m_InvertYAxis   = (dwVJoyStatus & INVERTYAXIS) ? 1 : 0;
	this->m_InvertXAxis   = (dwVJoyStatus & INVERTXAXIS) ? 1 : 0;
	this->m_B1AutoFire    = (dwVJoyStatus & B1AUTOFIRE) ? 1 : 0;
	this->m_B2AutoFire    = (dwVJoyStatus & B2AUTOFIRE) ? 1 : 0;
	this->m_VJAutoCenter  = (dwVJoyStatus & VJAUTOCENTER) ? 1 : 0;
	this->m_VJMouseWheel  = (dwVJoyStatus & VJMOUSEWHEEL) ? 1 : 0;
	this->m_UseMode = (dwVJoyStatus & VJKEYBOARDMAP) ? ((dwVJoyStatus & VJMOUSEMAP) ? 2 : 1) : 0;
	this->m_VJoySensivity = GetHookInfo()->VJoySensivity;
	// calculates the m_Slider values
	this->InitSensivity();
	this->m_Slider = this->GetSliderPos(this->m_VJoySensivity); 
	Slider=(CSliderCtrl *)this->GetDlgItem(IDC_SENSIVITY_SLIDER);
	Slider->SetRange(-20, 20, 1);

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
	if(this->m_VJAutoCenter)	dwVJoyStatus |= VJAUTOCENTER;
	if(this->m_VJMouseWheel)	dwVJoyStatus |= VJMOUSEWHEEL;
	switch(this->m_UseMode){
		case 0: dwVJoyStatus |= VJMOUSEMAP; break;
		case 1: dwVJoyStatus |= VJKEYBOARDMAP; break;
		case 2: dwVJoyStatus |= (VJKEYBOARDMAP|VJMOUSEMAP); break;
	}
	this->m_VJoySensivity = (int)(100 * pow(TICKMULTIPLIER, (float)this->m_Slider)); 
	sprintf_s(val, sizeof(val), "%i", dwVJoyStatus);
	WritePrivateProfileString("joystick", "flags", val, gInitPath);
	sprintf_s(val, sizeof(val), "%i", this->m_VJoySensivity);
	WritePrivateProfileString("joystick", "sensivity", val, gInitPath);
	GetHookInfo()->VJoyStatus = dwVJoyStatus;
	CDialog::OnOK();
}