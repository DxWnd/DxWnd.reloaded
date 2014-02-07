// TimeSliderDialog.cpp : implementation file
//

#include "stdafx.h"
#include "dxwndhost.h"
#include "TimeSliderDialog.h"

// CTimeSliderDialog dialog

IMPLEMENT_DYNAMIC(CTimeSliderDialog, CDialog)

CTimeSliderDialog::CTimeSliderDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CTimeSliderDialog::IDD, pParent)
{
	LoadString(AfxGetResourceHandle(), DXW_STRING_IDLE, IdleString, sizeof(IdleString));
}

CTimeSliderDialog::~CTimeSliderDialog()
{
	MessageBoxEx(0, "Time Slider destructor", "Warning", MB_OK | MB_ICONEXCLAMATION, NULL);
	//CTimeSliderDialog::OnOK(); // kill timer....
}

void CTimeSliderDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTargetDlg)
	DDX_Slider(pDX, IDC_TIMESLIDER, i_TimeSlider);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTimeSliderDialog, CDialog)
	ON_WM_TIMER()
END_MESSAGE_MAP()

#define IDTimeSiderTIMER 2

// CTimeSliderDialog message handlers

void CTimeSliderDialog::OnTimer(UINT_PTR nIDEvent)
{
	CSliderCtrl *Slider;
	CStatic *Text;
	char sMsg[81];
	static int iLastPos=-1;
	extern char *GetTSCaption(int);

	Slider=(CSliderCtrl *)this->GetDlgItem(IDC_TIMESLIDER);
	Text=(CStatic *)this->GetDlgItem(IDC_TIMESPEED);

	CDialog::OnTimer(nIDEvent);
	i_TimeSlider=Slider->GetPos();
	if(GetHookStatus(NULL)!=DXW_RUNNING) {
		Slider->SetPos(0);
		Text->SetWindowTextA(IdleString);
		return;
	}
	if(i_TimeSlider==iLastPos){
		// no change, check for keyboard input changes
		i_TimeSlider=GetHookInfo()->TimeShift;
		if(i_TimeSlider != iLastPos) Slider->SetPos(i_TimeSlider);
	}
	else {
		// slider position changed, set the new value
		if (i_TimeSlider < -8) i_TimeSlider=-8;
		if (i_TimeSlider >  8) i_TimeSlider= 8;
		GetHookInfo()->TimeShift=i_TimeSlider;
	}
	iLastPos = i_TimeSlider;
	sprintf(sMsg, "Time speed %s", GetTSCaption(i_TimeSlider));
	Text->SetWindowTextA(sMsg);
}

BOOL CTimeSliderDialog::OnInitDialog()
{
	CSliderCtrl *Slider;
	CDialog::OnInitDialog();

	Slider=(CSliderCtrl *)this->GetDlgItem(IDC_TIMESLIDER);
	Slider->SetRange(-8, +8, 0);
	Slider->SetTicFreq(1);
	Slider->SetPos(GetHookInfo()->TimeShift);
	SetTimer(IDTimeSiderTIMER, 1000, NULL);

	// TODO:  Add extra initialization here

	return TRUE;  // return TRUE unless you set the focus to a control
}

void CTimeSliderDialog::OnOK()
{
	// TODO: Add your specialized code here and/or call the base class

	// stop timer
	// MessageBoxEx(0, "Stopping Time Slider dialog", "Warning", MB_OK | MB_ICONEXCLAMATION, NULL);
	KillTimer(IDTimeSiderTIMER);
	GetHookInfo()->TimeShift=i_TimeSlider;
	CDialog::OnOK();
}
