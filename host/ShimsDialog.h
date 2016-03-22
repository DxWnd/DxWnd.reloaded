#pragma once


// CShimsDialog dialog

class CShimsDialog : public CDialog
{
	DECLARE_DYNAMIC(CShimsDialog)

public:
	CShimsDialog(CWnd* pParent = NULL, char *path = NULL);   // standard constructor
	virtual ~CShimsDialog();

// Dialog Data
	enum { IDD = IDD_SHIMS };
	int iTimeSlider;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
protected:
	virtual void OnOK();
private:
	char *ExePath;
};
