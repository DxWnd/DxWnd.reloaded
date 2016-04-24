#pragma once
#include <vector>

class IFormat  
{
public:
	virtual ~IFormat();
	virtual bool IsCharAllowed(TCHAR nChar) = 0;
};

class BaseFormat : public IFormat  
{
protected:
	std::vector<TCHAR>	m_listChars;
	BaseFormat();
	virtual ~BaseFormat();
public:
	void SetAllowedChars(std::vector<TCHAR> chars);
	void SetAllowedChars(LPCTSTR chars, int size);
	virtual bool IsCharAllowed(TCHAR nChar);
};

class RelIntegerFormat : public BaseFormat  
{
public:
	RelIntegerFormat();
	virtual ~RelIntegerFormat();
};

class CSpecialEdit : public CEdit
{
	IFormat	*m_formatter;
	bool IsClipboardOK();
	bool IsCharAllowed(TCHAR nChar);

public:
	CSpecialEdit();
	virtual ~CSpecialEdit();
	void SetFormatter(IFormat *formatter);
	
protected:
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSpecialEdit)
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CSpecialEdit)
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};
