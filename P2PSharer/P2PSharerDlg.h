
// P2PSharerDlg.h : ͷ�ļ�
//

#pragma once
#include "ServerEX.h"
#include "afxwin.h"
#include "afxcmn.h"


// CP2PSharerDlg �Ի���
class CP2PSharerDlg : public CDialogEx
{
// ����
public:
	CP2PSharerDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_P2PSHARER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��

private:
	ServerEX m_serEx;

	//ģ���ʼ��
	bool Init(void);

// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonSearch();
	CEdit m_editKeyword;
	CListCtrl m_listSearchResult;
};
