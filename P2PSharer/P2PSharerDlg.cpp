
// P2PSharerDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "P2PSharer.h"
#include "P2PSharerDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


CResourceMgr *g_resourceMgr;

// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
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


// CP2PSharerDlg 对话框



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


// CP2PSharerDlg 消息处理程序

BOOL CP2PSharerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
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

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	MessageBox("Start");

	m_listSearchResult.SetExtendedStyle(LVS_EX_FULLROWSELECT);
	m_listSearchResult.InsertColumn(0, "文件名", 0, 350);
	m_listSearchResult.InsertColumn(1, "文件大小", 0, 100);
	m_listSearchResult.InsertColumn(2, "资源数", 0, 100);
	m_listSearchResult.InsertColumn(3, "下载进度", 0, 100);


	g_clientlog.open("mylog.log", "P2PServer");

	Init();

	// TODO:  在此添加额外的初始化代码

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
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

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CP2PSharerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CP2PSharerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

//模块初始化
bool CP2PSharerDlg::Init(void)
{
	//启动资源管理对象
	g_resourceMgr = new CResourceMgr();
	g_resourceMgr->Init("119.29.66.237:6379");
	g_resourceMgr->set_detachable(true);
	g_resourceMgr->start();


	//启动与服务端的连接对象
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
		g_clientlog.error1("查找资源失败！");
		return;
	}

	int iLines = 0;
	m_listSearchResult.DeleteAllItems();

	//更新资源信息到搜索结果列表
	//mapResult格式:  key:文件名|文件MD5，Value：文件大小
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

//获取文件大小以显示在搜索结果列表上，以GB/MB为单位
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
