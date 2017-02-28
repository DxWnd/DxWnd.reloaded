#pragma once


// CStatusDialog dialog

class CStatusDialog : public CDialog
{
	DECLARE_DYNAMIC(CStatusDialog)

public:
	CStatusDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~CStatusDialog();

// Dialog Data
	enum { IDD = IDD_STATUS };

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
