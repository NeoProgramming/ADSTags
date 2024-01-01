
// ADSTagsDlg.cpp : implementation file
// _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING;

#include "pch.h"
#include "framework.h"
#include "ADSTags.h"
#include "ADSTagsDlg.h"
#include "afxdialogex.h"
#include "cvt.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CADSTagsDlg dialog



CADSTagsDlg::CADSTagsDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_ADSTAGS_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CADSTagsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_wndUsedTags);
	DDX_Control(pDX, IDC_LIST2, m_wndFreeTags);
	DDX_Control(pDX, IDC_BUTTON1, m_wndAdd);
	DDX_Control(pDX, IDC_EDIT1, m_wndEdit);
	DDX_Control(pDX, IDOK, m_wndOK);
	DDX_Control(pDX, IDCANCEL, m_wndCancel);
	DDX_Control(pDX, IDC_BUTTON2, m_wndCPath);
	DDX_Control(pDX, IDC_MFCBUTTON1, m_wndUp);
	DDX_Control(pDX, IDC_MFCBUTTON2, m_wndDn);
}

BEGIN_MESSAGE_MAP(CADSTagsDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_SIZE()
	ON_NOTIFY(NM_CLICK, IDC_LIST1, &CADSTagsDlg::OnNMClickUsedList)
	ON_NOTIFY(NM_CLICK, IDC_LIST2, &CADSTagsDlg::OnNMClickFreeList)
	ON_BN_CLICKED(IDOK, &CADSTagsDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_BUTTON1, &CADSTagsDlg::OnBnClickedAddTags)

	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST2, &CADSTagsDlg::OnLvnItemchangedFreeList)
	ON_BN_CLICKED(IDC_BUTTON2, &CADSTagsDlg::OnBnClickedCopyPath)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST1, &CADSTagsDlg::OnLvnItemchangedUsedList)
END_MESSAGE_MAP()


// CADSTagsDlg message handlers

BOOL CADSTagsDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// create imagelist for checkboxes
	m_imageList.Create(IDB_BITMAP1, 16, 0, RGB(255,0,255));
	m_wndUsedTags.SetImageList(&m_imageList, LVSIL_SMALL);
	m_wndFreeTags.SetImageList(&m_imageList, LVSIL_SMALL);

	m_wndUsedTags.SetExtendedStyle(LVS_EX_FULLROWSELECT);
	m_wndFreeTags.SetExtendedStyle(LVS_EX_FULLROWSELECT);

	Core.init();
	LoadIni();
	LoadLists();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CADSTagsDlg::LoadIni()
{
	Core.loadIni();
	SetWindowPos(0, Core.Cfg.x1, Core.Cfg.y1, Core.Cfg.x2-Core.Cfg.x1+1, Core.Cfg.y2-Core.Cfg.y1+1, SWP_NOZORDER);
}

void CADSTagsDlg::SaveIni()
{
	RECT rect;
	GetWindowRect(&rect);
	Core.Cfg.x1 = rect.left;
	Core.Cfg.y1 = rect.top;
	Core.Cfg.x2 = rect.right;
	Core.Cfg.y2 = rect.bottom;
	Core.saveIni();
}


void CADSTagsDlg::LoadUsedTags(TagIter b, TagIter e)
{
	LV_ITEM lvi;
	lvi.mask = LVIF_IMAGE | LVIF_TEXT;

	for (int i = 0; b != e; ++b, ++i) {
		if (b->chk != 0) {
			auto s = string_to_wide_string(b->tag);
			lvi.iItem = i;
			lvi.iSubItem = 0;
			lvi.state = LVIS_SELECTED | LVIS_FOCUSED;
			lvi.stateMask = LVIS_SELECTED | LVIS_FOCUSED;
			lvi.pszText = (LPWSTR) s.c_str();
			lvi.iImage = b->chk;
			int nNewItem = m_wndUsedTags.InsertItem(&lvi);
					
			s = string_to_wide_string(b->val);
			m_wndUsedTags.SetItemText(nNewItem, 1, s.c_str());
		
			TagPtr p = &(*b);
			m_wndUsedTags.SetItemData(nNewItem, reinterpret_cast<DWORD_PTR>(p));
		}
	}
}

void CADSTagsDlg::LoadFreeTags(TagIter b, TagIter e, int chk)
{
	LV_ITEM lvi;
	lvi.mask = LVIF_IMAGE | LVIF_TEXT;

	for (int i=0; b != e; ++b, ++i) {
		if (b->chk == chk) {
			auto s = string_to_wide_string(b->tag);
			lvi.iItem = i;
			lvi.iSubItem = 0;
			lvi.state = LVIS_SELECTED | LVIS_FOCUSED;
			lvi.stateMask = LVIS_SELECTED | LVIS_FOCUSED;
			lvi.pszText = (LPWSTR) s.c_str();
			lvi.iImage = b->chk;
			int nNewItem = m_wndFreeTags.InsertItem(&lvi);
					
			TagPtr p = &(*b);
			m_wndFreeTags.SetItemData(nNewItem, reinterpret_cast<DWORD_PTR>(p));
		}
	}
}

void CADSTagsDlg::LoadLists()
{
	m_wndUsedTags.InsertColumn(0, _T("Used Tags"), 130);
	m_wndUsedTags.InsertColumn(1, _T("Values"), 80);
	LoadUsedTags(Core.m_Tags.begin(), Core.m_Tags.end());

	m_wndFreeTags.InsertColumn(0, _T("Free Tags"), 130);
	LoadFreeTags(Core.m_Tags.begin(), Core.m_Tags.end(), 0);
}

void CADSTagsDlg::SaveLists()
{
	for (auto L : { &m_wndUsedTags, &m_wndFreeTags }) {
		for (int i = 0, n = L->GetItemCount(); i < n; i++) {
			TagPtr p = reinterpret_cast<TagPtr>( L->GetItemData(i) );
			if (p)
				p->chk = GetItemImage(*L, i);
		}
	}
}

void CADSTagsDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CADSTagsDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CADSTagsDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CADSTagsDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);
	if (!m_wndUsedTags.GetSafeHwnd())
		return;

	const int D = 7;
	const int BW = 80;
	const int BH = 22;

	const int LW = (cx - 3 * D) / 2;
	const int LH = cy - 4*D - 2*BH;
	const int BY = cy - D - BH;
	m_wndCPath.MoveWindow(D, D, BW, BH);

	m_wndUsedTags.MoveWindow(D, 2*D+BH, LW, LH);
	m_wndFreeTags.MoveWindow(D + LW + D, 2 * D + BH, LW, LH);
	m_wndAdd.MoveWindow(D, BY, BW, BH);
	m_wndEdit.MoveWindow(2 * D + BW, cy - BH - D, cx - 5*D-3*BW, BH);
	m_wndOK.MoveWindow(cx - 2*BW-2*D, cy-BH-D, BW, BH);
	m_wndCancel.MoveWindow(cx - BW - D, cy - BH - D, BW, BH);
}

int GetItemImage(CListCtrl &lst, int item)
{
	LV_ITEM lvi;
	lvi.mask = LVIF_IMAGE;
	lvi.iItem = item;
	lvi.iSubItem = 0;
	if (!lst.GetItem(&lvi))
		return -1;
	return lvi.iImage;
}

void SetItemImage(CListCtrl &lst, int item, int image)
{
	LV_ITEM lvi;
	lvi.mask = LVIF_IMAGE;
	lvi.iItem = item;
	lvi.iSubItem = 0;
	lvi.iImage = image;
	lst.SetItem(&lvi);
}

void CADSTagsDlg::OnNMClickUsedList(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	
	CRect r;
	BOOL s = m_wndUsedTags.GetSubItemRect(pNMItemActivate->iItem, pNMItemActivate->iSubItem, LVIR_ICON, r);
	if (s && r.PtInRect(pNMItemActivate->ptAction)) {
		int i = GetItemImage(m_wndUsedTags, pNMItemActivate->iItem);
		i = (i + 1) % 3;
		SetItemImage(m_wndUsedTags, pNMItemActivate->iItem, i);
	}

	*pResult = 0;
}


void CADSTagsDlg::OnNMClickFreeList(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	
	CRect r;
	BOOL s = m_wndFreeTags.GetSubItemRect(pNMItemActivate->iItem, pNMItemActivate->iSubItem, LVIR_ICON, r);
	if (s && r.PtInRect(pNMItemActivate->ptAction)) {
		int i = GetItemImage(m_wndFreeTags, pNMItemActivate->iItem);
		i = (i + 1) % 2;
		SetItemImage(m_wndFreeTags, pNMItemActivate->iItem, i);
	}

	*pResult = 0;
}

void CADSTagsDlg::AddTags()
{
	CString str;
	m_wndEdit.GetWindowText(str);
	std::string line = wide_string_to_string((LPCTSTR)str);
	TagIter it = Core.parseTags(line.c_str(), nullptr);
	LoadFreeTags(it, Core.m_Tags.end(), 1);
}


void CADSTagsDlg::OnBnClickedOk()
{
	AddTags();
	SaveLists();
	Core.apply();
	SaveIni();

	// cm_RereadSource
	HWND h = ::FindWindow(_T("TTOTAL_CMD"), 0);
	if (h != INVALID_HANDLE_VALUE)
		::PostMessage(h, EM_DISPLAYBAND, 540, 0);

	CDialogEx::OnOK();
}

void CADSTagsDlg::OnBnClickedAddTags()
{
	AddTags();
}

void CADSTagsDlg::OnLvnItemchangedUsedList(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);

	// for OnNMClick
	*pResult = 1;
}

void CADSTagsDlg::OnLvnItemchangedFreeList(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMIA = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);

	// for OnNMClick
	*pResult = 1;
}

void CADSTagsDlg::OnBnClickedCopyPath()
{
	std::wstring paths;
	for (auto &f : Core.m_Files) {
		if (!paths.empty())
			paths += L" ";
		paths += f.m_fpath;
	}

	const wchar_t* output = paths.c_str();
	const size_t size = paths.length() * 2 + 2;

	HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, size);
	memcpy(GlobalLock(hMem), output, size);
	GlobalUnlock(hMem);
	OpenClipboard();
	EmptyClipboard();
	SetClipboardData(CF_UNICODETEXT, hMem);
	CloseClipboard();
}


