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
}

CTimeSliderDialog::~CTimeSliderDialog()
{
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

#define IDTIMER 1

// CTimeSliderDialog message handlers

void CTimeSliderDialog::OnTimer(UINT_PTR nIDEvent)
{
	DXWNDSTATUS Status;
	CSliderCtrl *Slider;
	CStatic *Text;
	char *sTSCaption[17]={"x16","x12","x8","x6","x4","x3","x2","x1.5","x1",":1.5",":2",":3",":4",":6",":8",":12",":16"};
	char sMsg[81];
	static int iLastPos=-1;

	Slider=(CSliderCtrl *)this->GetDlgItem(IDC_TIMESLIDER);
	Text=(CStatic *)this->GetDlgItem(IDC_TIMESPEED);

	CDialog::OnTimer(nIDEvent);
	i_TimeSlider=Slider->GetPos();
	if(GetHookStatus(&Status)!=DXW_RUNNING) {
		Slider->SetPos(0);
		Text->SetWindowTextA("idle");
		return;
	}
	if(i_TimeSlider==iLastPos){
		// no change, check for keyboard input changes
		i_TimeSlider=Status.iTimeShift;
		if(i_TimeSlider != iLastPos) Slider->SetPos(i_TimeSlider);
	}
	else {
		// slider position changed, set the new value
		if (i_TimeSlider < -8) i_TimeSlider=-8;
		if (i_TimeSlider >  8) i_TimeSlider= 8;
		Status.iTimeShift=i_TimeSlider;
		SetHookStatus(&Status);
	}
	iLastPos = i_TimeSlider;
	sprintf(sMsg, "Time speed: %s", sTSCaption[i_TimeSlider+8]);
	Text->SetWindowTextA(sMsg);
}

BOOL CTimeSliderDialog::OnInitDialog()
{
	CSliderCtrl *Slider;
	CDialog::OnInitDialog();
	DXWNDSTATUS Status;

	GetHookStatus(&Status);
	Slider=(CSliderCtrl *)this->GetDlgItem(IDC_TIMESLIDER);
	Slider->SetRange(-8, +8, 0);
	Slider->SetTicFreq(1);
	Slider->SetPos(Status.iTimeShift);
	SetTimer(IDTIMER, 1000, NULL);

	// TODO:  Add extra initialization here

	return TRUE;  // return TRUE unless you set the focus to a control
}

void CTimeSliderDialog::OnOK()
{
	// TODO: Add your specialized code here and/or call the base class

	KillTimer(IDTIMER);
	// stop timer

	DXWNDSTATUS Status;
	GetHookStatus(&Status);
	Status.iTimeShift=i_TimeSlider;
	SetHookStatus(&Status);

	CDialog::OnOK();
}
