#include "stdafx.h"
#include "SpecialEdit.h"

#include <afxole.h>
#include <vector>
#include <algorithm>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

BaseFormat::BaseFormat()
{
}

BaseFormat::~BaseFormat()
{
}

void BaseFormat::SetAllowedChars(std::vector<TCHAR> chars)
{
	m_listChars = chars;
}

void BaseFormat::SetAllowedChars(LPCTSTR chars, int size)
{
	m_listChars.erase(m_listChars.begin(), m_listChars.end());
	m_listChars.reserve(size);
	m_listChars.assign(chars, chars+size);
}

RelIntegerFormat::RelIntegerFormat()
{
	LPCTSTR format = _T("-+0123456789");
	SetAllowedChars(format, _tcslen(format));	
}

RelIntegerFormat::~RelIntegerFormat()
{
}

bool BaseFormat::IsCharAllowed(TCHAR nChar)
{
	std::vector<TCHAR>::iterator pos = std::find(m_listChars.begin(), m_listChars.end(), nChar);
	return (pos != m_listChars.end());	
}

CSpecialEdit::CSpecialEdit()
{
	m_formatter = NULL;
}

CSpecialEdit::~CSpecialEdit()
{
}

BEGIN_MESSAGE_MAP(CSpecialEdit, CEdit)
	//{{AFX_MSG_MAP(CSpecialEdit)
	ON_WM_CHAR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSpecialEdit message handlers

void CSpecialEdit::SetFormatter(IFormat *formatter)
{
	ASSERT(formatter != NULL);
	m_formatter = formatter;
}

bool CSpecialEdit::IsCharAllowed(TCHAR nChar)
{
	switch(nChar){
		case _T('\b'):
		case 10:
		case 13:
			return true;
	}

	ASSERT(m_formatter != NULL);	
	return m_formatter->IsCharAllowed(nChar);
}

bool CSpecialEdit::IsClipboardOK()
{
	bool isOK = true;
	COleDataObject	obj;

	if (obj.AttachClipboard()) {
		HGLOBAL hmem = NULL;
		TCHAR *pUniText = NULL;
		DWORD dwLen = 0;
		bool bText = false;

		if (obj.IsDataAvailable(CF_TEXT)){
			hmem = obj.GetGlobalData(CF_TEXT);
			
			char *pCharText = (char*)::GlobalLock(hmem);
#ifdef UNICODE
			int lenA = strlen(pCharText);
			int lenW = MultiByteToWideChar(CP_ACP, 0, pCharText, lenA, 0, 0);
			if (lenW > 0){
				pUniText = ::SysAllocStringLen(0, lenW);
				MultiByteToWideChar(CP_ACP, 0, pCharText, lenA, pUniText, lenW);
				bText = true;
			}
			else{
				::GlobalUnlock(hmem);
				return false;
			}
#else
			pUniText = pCharText;
#endif
		}
#ifdef UNICODE
		else if(obj.IsDataAvailable(CF_UNICODETEXT)){
			hmem = obj.GetGlobalData(CF_UNICODETEXT);
			pUniText = (TCHAR*)::GlobalLock(hmem);
		}
#endif
		if(hmem){	
			DWORD	dwLen = _tcslen(pUniText);
			for(DWORD i=0; i<dwLen && isOK; i++)
				isOK = IsCharAllowed(pUniText[i]);
			::GlobalUnlock(hmem);
#ifdef UNICODE
			if(bText)
				::SysFreeString(pUniText);
#endif
		}
		else
			return false;
	}
	return isOK;
}

LRESULT CSpecialEdit::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	switch(message){
		case WM_PASTE:
			if(!IsClipboardOK()){
				MessageBeep(-1);
				return 0;
			}
	}
	
	return CEdit::WindowProc(message, wParam, lParam);
}

void CSpecialEdit::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	if(GetKeyState(VK_CONTROL) & 0x80000000){
		switch(nChar){
			case 0x03:
				Copy();
				return;
			case 0x16:
				Paste();
				return;
			case 0x18:
				Cut();
				return;
			case 0x1a:
				Undo();
				return;
		}
	}
	
	if(!IsCharAllowed(nChar)){
		MessageBeep(-1);
		return;
	}
	
	CEdit::OnChar(nChar, nRepCnt, nFlags);
}

IFormat::~IFormat()
{
}