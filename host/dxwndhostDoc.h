// dxwndhostDoc.h : CDxwndhostDoc class definitions and interface declarations.
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_DXWNDHOSTDOC_H__07C9FF34_8B9B_49CC_BE82_B76813FE88FE__INCLUDED_)
#define AFX_DXWNDHOSTDOC_H__07C9FF34_8B9B_49CC_BE82_B76813FE88FE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class CDxwndhostDoc : public CDocument
{
protected: // Create from serialization only features.
	CDxwndhostDoc();
	DECLARE_DYNCREATE(CDxwndhostDoc)

// Attributes
public:

// Operation
public:

//Override
	// ClassWizard Generates a virtual function overrides.
	//{{AFX_VIRTUAL(CDxwndhostDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CDxwndhostDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CDxwndhostDoc)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DXWNDHOSTDOC_H__07C9FF34_8B9B_49CC_BE82_B76813FE88FE__INCLUDED_)
