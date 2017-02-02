#pragma once


// CViewFlagsDialog dialog

class CViewFlagsDialog : public CDialog
{
	DECLARE_DYNAMIC(CViewFlagsDialog)

public:
	CViewFlagsDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~CViewFlagsDialog();

	// Dialog Data
	enum { IDD = IDD_VIEWFLAGS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
public:
	virtual BOOL OnInitDialog();
protected:
	virtual void OnOK();
};
