// CDragEdit class: inherited by DEdit, adds drag & drop of file pathnames 
// within the text field.

class CDragEdit : public CEdit
{
public:
	CDragEdit();
	virtual ~CDragEdit();
	BOOL OnInitDialog();
private:
	afx_msg void OnDropFiles(HDROP hDropInfo);
	DECLARE_MESSAGE_MAP()
};
