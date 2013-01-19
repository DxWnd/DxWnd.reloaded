#pragma once


// CTimeSlider dialog

class CTimeSliderDialog : public CDialog
{
	DECLARE_DYNAMIC(CTimeSliderDialog)

public:
	CTimeSliderDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~CTimeSliderDialog();

// Dialog Data
	enum { IDD = IDD_TIMESLIDER };
	//CSliderCtrl m_TimeSlider;
	int i_TimeSlider;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnTimer(UINT_PTR nIDEvent);
public:
	virtual BOOL OnInitDialog();
protected:
	virtual void OnOK();
};
