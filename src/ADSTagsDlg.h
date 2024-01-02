#pragma once
#include "TaggerCore.h"

int GetItemImage(CListCtrl &lst, int item);
void SetItemImage(CListCtrl &lst, int item, int image);

// CADSTagsDlg dialog
class CADSTagsDlg : public CDialogEx
{
// Construction
public:
	CADSTagsDlg(CWnd* pParent = nullptr);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ADSTAGS_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;
	CImageList m_imageList;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

	void LoadIni();
	void SaveIni();
	void LoadLists();
	void LoadUsedTags(TagIter b, TagIter e);
	void LoadFreeTags(TagIter b, TagIter e, int chk);
	void LoadPaths();
	void SaveLists();
	void AddTags();
public:
	CListCtrl m_wndUsedTags;
	CListCtrl m_wndFreeTags;
	afx_msg void OnSize(UINT nType, int cx, int cy);
	CButton m_wndAdd;
	CEdit m_wndEdit;
	CButton m_wndOK;
	CButton m_wndCancel;
	afx_msg void OnNMClickUsedList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMClickFreeList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedAddTags();
	CButton m_wndCPath;

	afx_msg void OnLvnItemchangedFreeList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedCopyPath();
	afx_msg void OnLvnItemchangedUsedList(NMHDR *pNMHDR, LRESULT *pResult);
	CComboBox m_wndPaths;
};
