#include "stdafx.h"
#include "SearchResultMgr.h"


CSearchResultMgr::CSearchResultMgr(HWND hwnd, CRedisClient *redis)
{
	m_bSearching = false;
	m_hNotifyWnd = hwnd;
	m_redis = redis;
}


CSearchResultMgr::~CSearchResultMgr()
{
}


//���������ؼ���
bool CSearchResultMgr::SetSearchWord(const char *keyword)
{
	m_keyword = keyword;

	return true;
}

//�Ƿ���������
bool CSearchResultMgr::IsSearching(void)
{
	return m_bSearching;
}

void *CSearchResultMgr::run()
{
	//�Թؼ��ֽ���URL����
	acl::string key;
	if (m_keyword != "*")
		key.url_encode(m_keyword);
	else
		key = m_keyword;

	std::map<acl::string, acl::string> mapTemp;

	m_bSearching = true;
	if (!m_redis->FindResource(key, mapTemp))
	{
		g_clientlog.error1("������Դʧ�ܣ�");
		m_bSearching = false;
		return NULL;
	}
	
	//�Խ������ת����ʹ����ֱ����ʾ
	acl::string temp;
	std::map<acl::string, acl::string>::iterator itRes = mapTemp.begin();
	for (; itRes != mapTemp.end(); ++itRes)
	{
		temp = itRes->first;
		std::vector<acl::string> vRes = temp.split2(SPLITOR_OF_FILE_INFO);

		T_SEARCH_RESULT_INFO *item = new T_SEARCH_RESULT_INFO;
		item->filename = temp.url_decode(vRes[0]);
		item->filemd5 = vRes[1];
		item->filesize = itRes->second;
		item->resource_count = m_redis->GetResourceOwners(item->filemd5);

		//���͸�UI��ʾ
		SendMessage(m_hNotifyWnd, UM_UPDATE_SEARCH_RESULT, (WPARAM)item, NULL);
	}

	m_bSearching = false;

	return NULL;
}