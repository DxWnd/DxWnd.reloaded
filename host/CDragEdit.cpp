#include "stdafx.h"
#include "CDragEdit.h"

CDragEdit::CDragEdit()
{
}

CDragEdit::~CDragEdit()
{
}

BOOL CDragEdit::OnInitDialog()
{
	DragAcceptFiles();
	return TRUE;
}

void CDragEdit::OnDropFiles(HDROP dropInfo)
{
	CString sFile;
	DWORD nBuffer = 0;
	// Get number of files
	UINT nFilesDropped = DragQueryFile(dropInfo, 0xFFFFFFFF, NULL, 0);
	if(nFilesDropped > 0){
		nBuffer = DragQueryFile(dropInfo, 0, NULL, 0);
		DragQueryFile(dropInfo, 0, sFile.GetBuffer(nBuffer+1), nBuffer+1);
		SetWindowTextA(sFile.GetBuffer());
		sFile.ReleaseBuffer();
	}
	DragFinish(dropInfo);
}

BEGIN_MESSAGE_MAP(CDragEdit, CEdit)
	ON_WM_DROPFILES()
END_MESSAGE_MAP()
