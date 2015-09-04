#pragma once
#include "acl_cpp\lib_acl.hpp"

class CSearchResultMgr :
	public acl::thread
{
public:
	CSearchResultMgr(HWND hwnd, CRedisClient *redis);
	~CSearchResultMgr();

 	//���������ؼ���
 	bool SetSearchWord(const char *keyword);

	//�Ƿ���������
	bool IsSearching(void);

	//ִ���������̺߳���
 	void *run();

private:
	HWND m_hNotifyWnd;        //������������Ĵ��ھ��
	bool m_bSearching;        //�Ƿ�����ִ������
	CRedisClient *m_redis;
	acl::string m_keyword;
	std::map<acl::string, acl::string> m_mapSearchResult;  //�洢�����������URL���룩  ��ʽ:  key:�ļ���|�ļ�MD5��Value���ļ���С
};

