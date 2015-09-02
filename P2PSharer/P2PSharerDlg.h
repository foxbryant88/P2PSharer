
// P2PSharerDlg.h : 头文件
//

#pragma once
#include "ServerEX.h"
#include "afxwin.h"
#include "afxcmn.h"
#include "SearchResultMgr.h"


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

	//模块初始化
	bool Init(void);

	//获取文件大小以显示在搜索结果列表上，以GB/MB为单位
	acl::string GetResourceFileSize(acl::string sizeInByte);


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
};
