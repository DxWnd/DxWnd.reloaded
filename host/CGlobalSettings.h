#pragma once

typedef struct {
	int iLabelResourceId;
	char *sLabel;
	int iComboResourceId;
	char *sIniLabel;
	int iComboSelection;
} Key_Type;

typedef struct {
	DWORD dwVKeyCode;
	char *sVKeyLabel;
} KeyCombo_Type;

// CGlobalSettings dialog

class CGlobalSettings : public CDialog
{
	DECLARE_DYNAMIC(CGlobalSettings)

public:
	CGlobalSettings(CWnd* pParent = NULL);   // standard constructor
	virtual ~CGlobalSettings();

// Dialog Data
	enum { IDD = IDD_GLOBAL_CONFIG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	BOOL m_DebugMode;
	BOOL m_UpdatePaths;
	BOOL m_AutoHideMode;
	BOOL m_CheckAdminRights;
	BOOL m_NameFromFolder;
	int m_TexMinX;
	int m_TexMinY;
	int m_TexMaxX;
	int m_TexMaxY;

	DECLARE_MESSAGE_MAP()
public:
public:
	virtual BOOL OnInitDialog();
protected:
	virtual void OnOK();
};
