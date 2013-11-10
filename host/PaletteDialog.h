#pragma once


// CPaletteDialog dialog

class CPaletteDialog : public CDialog
{
	DECLARE_DYNAMIC(CPaletteDialog)

public:
	CPaletteDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~CPaletteDialog();

// Dialog Data
	enum { IDD = IDD_PALETTE };

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
