#pragma once


// CDesktopDialog dialog

class CDesktopDialog : public CDialog
{
	DECLARE_DYNAMIC(CDesktopDialog)

public:
	CDesktopDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDesktopDialog();

	// Dialog Data
	enum { IDD = IDD_DESKTOP };

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
