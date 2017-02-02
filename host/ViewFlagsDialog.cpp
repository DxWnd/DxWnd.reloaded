// ViewFlagsDialog.cpp : implementation file
//

#include "stdafx.h"
#include "dxwndhost.h"
#include "ViewFlagsDialog.h"

// CViewFlagsDialog dialog

IMPLEMENT_DYNAMIC(CViewFlagsDialog, CDialog)

CViewFlagsDialog::CViewFlagsDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CViewFlagsDialog::IDD, pParent)
{
}

CViewFlagsDialog::~CViewFlagsDialog()
{
}

void CViewFlagsDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CViewFlagsDialog, CDialog)
END_MESSAGE_MAP()

// CViewFlagsDialog message handlers

BOOL CViewFlagsDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  Add extra initialization here
	CString sflags;
	extern TARGETMAP *ViewTarget; // dirty !!!!
	int i;
	TARGETMAP *t;
	DWORD dword;
	t = ViewTarget;
	sflags.Append("Flags1: ");
	for(i=0, dword = t->flags ; i<32; i++, dword>>=1) if(dword & 0x1) sflags.AppendFormat("%s ", GetFlagCaption(0,i));
	sflags.Append("\nFlags2: ");
	for(i=0, dword = t->flags2; i<32; i++, dword>>=1) if(dword & 0x1) sflags.AppendFormat("%s ", GetFlagCaption(1,i));
	sflags.Append("\nFlags3: ");
	for(i=0, dword = t->flags3; i<32; i++, dword>>=1) if(dword & 0x1) sflags.AppendFormat("%s ", GetFlagCaption(2,i));
	sflags.Append("\nFlags4: ");
	for(i=0, dword = t->flags4; i<32; i++, dword>>=1) if(dword & 0x1) sflags.AppendFormat("%s ", GetFlagCaption(3,i));
	sflags.Append("\nFlags5: ");
	for(i=0, dword = t->flags5; i<32; i++, dword>>=1) if(dword & 0x1) sflags.AppendFormat("%s ", GetFlagCaption(4,i));
	sflags.Append("\nFlags6: ");
	for(i=0, dword = t->flags6; i<32; i++, dword>>=1) if(dword & 0x1) sflags.AppendFormat("%s ", GetFlagCaption(5,i));
	sflags.Append("\nFlags7: ");
	for(i=0, dword = t->flags7; i<32; i++, dword>>=1) if(dword & 0x1) sflags.AppendFormat("%s ", GetFlagCaption(6,i));
	sflags.Append("\nFlags8: ");
	for(i=0, dword = t->flags8; i<32; i++, dword>>=1) if(dword & 0x1) sflags.AppendFormat("%s ", GetFlagCaption(7,i));
	sflags.Append("\nTFlags: ");
	for(i=0, dword = t->tflags; i<32; i++, dword>>=1) if(dword & 0x1) sflags.AppendFormat("%s ", GetFlagCaption(8,i));

	this->SetDlgItemTextA(IDC_DESKTOPINFO, sflags);
	this->SetWindowTextA(t->path);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CViewFlagsDialog::OnOK()
{
	// TODO: Add your specialized code here and/or call the base class

	CDialog::OnOK();
}
