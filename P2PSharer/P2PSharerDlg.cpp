
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

	MessageBox("Start");

	m_listSearchResult.SetExtendedStyle(LVS_EX_FULLROWSELECT);
	m_listSearchResult.InsertColumn(0, "�ļ���", 0, 350);
	m_listSearchResult.InsertColumn(1, "�ļ���С", 0, 100);
	m_listSearchResult.InsertColumn(2, "��Դ��", 0, 100);
	m_listSearchResult.InsertColumn(3, "���ؽ���", 0, 100);


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

	m_serEx.Init(addr);
	m_serEx.set_detachable(true);
	m_serEx.start();

	m_serEx.SendMsg_UserLogin();

	return true;
}


void CP2PSharerDlg::OnBnClickedButtonSearch()
{
	acl::log::stdout_open(true);

	CString addr = "";
	m_editKeyword.GetWindowTextA(addr);
// 	m_serEx.SendMsg_P2PData("hello, world", addr.GetBuffer());

	acl::string key = addr.GetBuffer();
	std::map<acl::string, acl::string> mapResult;
	CRedisClient *redis = g_resourceMgr->GetRedisClient();
	if (!redis->FindResource(key, mapResult))
	{
		g_clientlog.error1("������Դʧ�ܣ�");
		return;
	}

	int iLines = 0;
	m_listSearchResult.DeleteAllItems();

	//������Դ��Ϣ����������б�
	//mapResult��ʽ:  key:�ļ���|�ļ�MD5��Value���ļ���С
	std::map<acl::string, acl::string>::iterator itResult = mapResult.begin();
	for (; itResult != mapResult.end(); ++itResult)
	{
		acl::string temp = itResult->first;
		std::vector<acl::string> vRes = temp.split2(SPLITOR_OF_FILE_INFO);

		m_listSearchResult.InsertItem(iLines, "");
		m_listSearchResult.SetItemText(iLines, 0, vRes[0]);
		m_listSearchResult.SetItemText(iLines, 1, GetResourceFileSize(itResult->second));
		m_listSearchResult.SetItemText(iLines, 2, IntToString(redis->GetResourceOwners(vRes[1])));
		iLines++;
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
