// TabDirectX.cpp : implementation file
//

#include "stdafx.h"
#include "TargetDlg.h"
#include "TabCompat.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTabCompat dialog

CTabCompat::CTabCompat(CWnd* pParent /*=NULL*/)
//	: CTargetDlg(pParent)
	: CDialog(CTabCompat::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTabCompat)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

void CTabCompat::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	CTargetDlg *cTarget = ((CTargetDlg *)(this->GetParent()->GetParent()));
	DDX_Check(pDX, IDC_FAKEVERSION, cTarget->m_FakeVersion);
	DDX_LBIndex(pDX, IDC_LISTFAKE, cTarget->m_FakeVersionId);
	DDX_Check(pDX, IDC_SINGLEPROCAFFINITY, cTarget->m_SingleProcAffinity);
	DDX_Check(pDX, IDC_HANDLEEXCEPTIONS, cTarget->m_HandleExceptions);
	DDX_Check(pDX, IDC_LIMITRESOURCES, cTarget->m_LimitResources);
	DDX_Check(pDX, IDC_SUPPRESSIME, cTarget->m_SuppressIME);
	DDX_Check(pDX, IDC_CDROMDRIVETYPE, cTarget->m_CDROMDriveType);
	DDX_Check(pDX, IDC_FONTBYPASS, cTarget->m_FontBypass);
	DDX_Check(pDX, IDC_BUFFEREDIOFIX, cTarget->m_BufferedIOFix);
	DDX_Check(pDX, IDC_NOPERFCOUNTER, cTarget->m_NoPerfCounter);
	DDX_Check(pDX, IDC_HIDECDROMEMPTY, cTarget->m_HideCDROMEmpty);
	DDX_Check(pDX, IDC_DIABLOTWEAK, cTarget->m_DiabloTweak);
	DDX_Check(pDX, IDC_EASPORTSHACK, cTarget->m_EASportsHack);
	DDX_Check(pDX, IDC_LEGACYALLOC, cTarget->m_LegacyAlloc);
	DDX_Check(pDX, IDC_DISABLEMAXWINMODE, cTarget->m_DisableMaxWinMode);
	DDX_Check(pDX, IDC_NOIMAGEHLP, cTarget->m_NoImagehlp);
	DDX_Check(pDX, IDC_REPLACEPRIVOPS, cTarget->m_ReplacePrivOps);

	// Registry management
	DDX_Check(pDX, IDC_EMULATEREGISTRY, cTarget->m_EmulateRegistry);
	DDX_Check(pDX, IDC_OVERRIDEREGISTRY, cTarget->m_OverrideRegistry);
	DDX_Check(pDX, IDC_WOW64REGISTRY, cTarget->m_Wow64Registry);
	DDX_Check(pDX, IDC_WOW32REGISTRY, cTarget->m_Wow32Registry);
}

BEGIN_MESSAGE_MAP(CTabCompat, CDialog)
	//{{AFX_MSG_MAP(CTabCompat)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTabCompat message handlers

static struct {char bMajor; char bMinor; char *sName;} WinVersions[9]=
{
	{4, 0, "Windows 95"},
	{4,10, "Windows 98/SE"},
	{4,90, "Windows ME"},
	{5, 0, "Windows 2000"},
	{5, 1, "Windows XP"},
	{5, 2, "Windows Server 2003"},
	{6, 0, "Windows Vista"},
	{6, 1, "Windows 7"},
	{6, 2, "Windows 8"}
};

BOOL CTabCompat::OnInitDialog()
{
	AfxEnableControlContainer();
	CListBox *List;
	CTargetDlg *cTarget = ((CTargetDlg *)(this->GetParent()->GetParent()));
	int i;
	List=(CListBox *)this->GetDlgItem(IDC_LISTFAKE);
	List->ResetContent();
	for(i=0; i<9; i++) List->AddString(WinVersions[i].sName);
	List->SetCurSel(cTarget->m_FakeVersion);
	CDialog::OnInitDialog();
	return TRUE;
}