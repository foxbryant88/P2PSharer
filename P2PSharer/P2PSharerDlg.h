
// P2PSharerDlg.h : 头文件
//

#pragma once
#include "ServerEX.h"
#include "afxwin.h"
#include "afxcmn.h"
#include "SearchResultMgr.h"
#include "Downloader.h"


// CP2PSharerDlg 对话框
class CP2PSharerDlg : public CDialogEx
{
// 构造
public:
	CP2PSharerDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_P2PSHARER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持

private:
	ServerEX m_serEx;
	//std::map<acl::string, acl::string> m_mapSearchResult;      //关键词搜索结果
	CSearchResultMgr *m_objSearchMgr;
	int m_iSeachResultItems;      //搜索结果条目数
	std::map<acl::string, CDownloader *> m_mapFileDownloader;    //key：文件MD5 value：文件下载对象

	std::map<int, T_SEARCH_RESULT_INFO *> m_mapSearchResult;   //保存搜索结果列表条目与文件的对应关系，以便找到用户双击下载的文件MD5值

	//模块初始化
	bool Init(void);

	//获取文件大小以显示在搜索结果列表上，以GB/MB为单位
	acl::string GetResourceFileSize(acl::string sizeInByte);

	//UDP发送数据测试
	void SendUDPTest(CString addr);

// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonSearch();
	CEdit m_editKeyword;
	CListCtrl m_listSearchResult;
	virtual BOOL OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnNMDblclkListResource(NMHDR *pNMHDR, LRESULT *pResult);
};
