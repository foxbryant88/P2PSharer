
// P2PSharerDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "P2PSharer.h"
#include "P2PSharerDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


CResourceMgr *g_resourceMgr;
ServerEX g_serEx;
std::map<acl::string, CDownloader *> g_mapFileDownloader;    //key���ļ�MD5 value���ļ����ض���
std::map<acl::string, CFileServer *> g_mapFileServer;    //key���ļ�MD5 value���ļ��ṩ�߶���

// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// �Ի�������
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CP2PSharerDlg �Ի���



CP2PSharerDlg::CP2PSharerDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CP2PSharerDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CP2PSharerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_KEYWORD, m_editKeyword);
	DDX_Control(pDX, IDC_LIST_RESOURCE, m_listSearchResult);
}

BEGIN_MESSAGE_MAP(CP2PSharerDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_SEARCH, &CP2PSharerDlg::OnBnClickedButtonSearch)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_RESOURCE, &CP2PSharerDlg::OnNMDblclkListResource)
END_MESSAGE_MAP()


// CP2PSharerDlg ��Ϣ�������

BOOL CP2PSharerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ��������...���˵�����ӵ�ϵͳ�˵��С�

	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
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

	// ���ô˶Ի����ͼ�ꡣ  ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	//MessageBox("Start");

	m_listSearchResult.SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);
	m_listSearchResult.InsertColumn(0, "�ļ���", 0, 350);
	m_listSearchResult.InsertColumn(1, "�ļ���С", 0, 100);
	m_listSearchResult.InsertColumn(2, "��Դ��", 0, 100);
	m_listSearchResult.InsertColumn(3, "���ؽ���", 0, 100);
	m_listSearchResult.InsertColumn(4, "������MAC", 0, 200);


	g_clientlog.open("mylog.log", "P2PServer");

	Init();

	// TODO:  �ڴ���Ӷ���ĳ�ʼ������

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

void CP2PSharerDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ  ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CP2PSharerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CP2PSharerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

//ģ���ʼ��
bool CP2PSharerDlg::Init(void)
{
	//������Դ�������
	g_resourceMgr = new CResourceMgr();
	g_resourceMgr->Init("119.29.66.237:6379");
	g_resourceMgr->set_detachable(true);
	g_resourceMgr->start();


	//���������˵����Ӷ���
	//acl::string addr("127.0.0.1:8888");
	//acl::string addr("192.168.1.102:8888");
	acl::string addr("119.29.66.237:8888");

	g_serEx.Init(addr);
	g_serEx.set_detachable(true);
	g_serEx.start();

	g_serEx.SendMsg_UserLogin();

	return true;
}


void CP2PSharerDlg::OnBnClickedButtonSearch()
{
	CString keyword = "";
	m_editKeyword.GetWindowTextA(keyword);

	//g_serEx.SendMsg_P2PData("hello", keyword.GetBuffer());
	////////g_serEx.SendMsg_UserLogin();
	////////m_serEx.SendMsg_GetIPofMAC("22-22-22-22-22-22");
	//return;

	if (keyword != "")
	{
		if (NULL == m_objSearchMgr)
		{
			m_objSearchMgr = new CSearchResultMgr(m_hWnd, g_resourceMgr->GetRedisClient());
		}

		//�ȴ��ϴ�����������������߳�ͬʱ��������
		if (m_objSearchMgr->IsSearching())
		{
			MessageBox("���������У���ȴ�����������");
			return;
		}


		//�ͷžɵ��������
		std::map<int, T_SEARCH_RESULT_INFO *> ::iterator itTmp = m_mapSearchResult.begin();
		for (; itTmp != m_mapSearchResult.end(); ++itTmp)
		{
			delete itTmp->second;    //���ڴ���SearchResultMgr�з��䣬��ɾ��
		}
		m_mapSearchResult.clear();
		m_iSeachResultItems = 0;
		m_listSearchResult.DeleteAllItems();
		

		//���������߳�
		m_objSearchMgr->SetSearchWord(keyword.GetBuffer());
		m_objSearchMgr->set_detachable(true);
		m_objSearchMgr->start();
	}
}

//��ȡ�ļ���С����ʾ����������б��ϣ���GB/MBΪ��λ
acl::string CP2PSharerDlg::GetResourceFileSize(acl::string sizeInByte)
{
	long long bytes = atoll(sizeInByte);
	double kbs = bytes / 1024;
	double mbs = kbs / 1024;
	double gbs = mbs / 1024;

	acl::string result;
	if (gbs > 1)
	{
		result.format("%0.2f GB", gbs);
	}
	else if (mbs > 1)
	{
		result.format("%0.2f MB", mbs);
	}
	else if (kbs > 1)
	{
		result.format("%0.2f KB", kbs);
	}
	else
	{
		result.format("%d B", bytes);
	}

	return result;
}

BOOL CP2PSharerDlg::OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	// TODO:  �ڴ����ר�ô����/����û���
	T_SEARCH_RESULT_INFO *resInfo = NULL;
	int iItem = 0;
	acl::string text;
	std::map<acl::string, CDownloader *>::iterator itDownloader;

	switch (message)
	{
	case UM_UPDATE_SEARCH_RESULT:
		resInfo = (T_SEARCH_RESULT_INFO*)wParam;

		m_iSeachResultItems = m_listSearchResult.GetItemCount();

		m_listSearchResult.InsertItem(m_iSeachResultItems, "");
 		m_listSearchResult.SetItemText(m_iSeachResultItems, 0, resInfo->filename);
		m_listSearchResult.SetItemText(m_iSeachResultItems, 1, GetResourceFileSize(resInfo->filesize));
 		m_listSearchResult.SetItemText(m_iSeachResultItems, 2, resInfo->resource_count);
		m_listSearchResult.SetItemText(m_iSeachResultItems, 4, resInfo->owerMac);
		m_listSearchResult.SetItemData(m_iSeachResultItems, m_iSeachResultItems);         //ÿ��Item��ʶ���������ͬ

		//���������������ʾ��Ŀ�Ķ�Ӧ��ϵ
		m_mapSearchResult[m_iSeachResultItems] = resInfo;
		m_mapFileItemByMD5[resInfo->filemd5] = m_iSeachResultItems;

		m_iSeachResultItems++;

		//delete resInfo;
		break;

	case UM_UPDATE_DOWNLOAD_PROGRESS:
		iItem = m_mapFileItemByMD5[(char *)lParam];
		{
			acl::string text = (char *)wParam;
			m_listSearchResult.SetItemText(iItem, 3, text);
		}

		break;

	case UM_DOWNLOAD_FINISHED:
		itDownloader = g_mapFileDownloader.find((char *)lParam);
		if (itDownloader != g_mapFileDownloader.end())
		{
			itDownloader->second->Stop();
			delete itDownloader->second;
			g_mapFileDownloader.erase(itDownloader);
		}

		break;
	default:
		break;
	}

	return CDialogEx::OnWndMsg(message, wParam, lParam, pResult);
}


BOOL CP2PSharerDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO:  �ڴ����ר�ô����/����û���
	switch (pMsg->message)
	{
	case WM_KEYDOWN:
		if (pMsg->wParam == VK_RETURN)
		{
			if (GetFocus()->GetDlgCtrlID() == IDC_EDIT_KEYWORD)
			{
				OnBnClickedButtonSearch();
			}
				
			return TRUE;  //��ֹ������Ӧ�س�
		}

	default:
		break;
	}
	return CDialogEx::PreTranslateMessage(pMsg);
}


void CP2PSharerDlg::OnNMDblclkListResource(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	
	int iSelectedItem = pNMItemActivate->iItem;
	if (iSelectedItem < 0)
		return;

	int iItemData = m_listSearchResult.GetItemData(iSelectedItem);
	T_SEARCH_RESULT_INFO *pResultInfo = m_mapSearchResult[iItemData];

	T_LOCAL_FILE_INFO loclInfo;
	//g_resourceMgr->GetFileInfo(pResultInfo->filemd5, loclInfo);
	loclInfo.filemd5 = pResultInfo->filemd5;
	loclInfo.filename = pResultInfo->filename;

	long long size = 0;
	StrToInt64Ex(pResultInfo->filesize, STIF_DEFAULT, &size);
	loclInfo.filesize = size;

	CDownloader *objDownloader = new CDownloader;
	g_mapFileDownloader[loclInfo.filemd5] = objDownloader;
	objDownloader->set_detachable(true);
	objDownloader->Init(loclInfo, g_serEx.GetSockStream(), g_resourceMgr->GetRedisClient(), m_hWnd);
	objDownloader->start();

	// TODO:  �ڴ���ӿؼ�֪ͨ����������
	*pResult = 0;
}


//UDP�������ݲ���
void CP2PSharerDlg::SendUDPTest(CString addr)
{

}