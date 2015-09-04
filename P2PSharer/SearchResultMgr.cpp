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


//设置搜索关键字
bool CSearchResultMgr::SetSearchWord(const char *keyword)
{
	m_keyword = keyword;

	return true;
}

//是否正在搜索
bool CSearchResultMgr::IsSearching(void)
{
	return m_bSearching;
}

void *CSearchResultMgr::run()
{
	//对关键字进行URL编码
	acl::string key;
	if (m_keyword != "*")
		key.url_encode(m_keyword);
	else
		key = m_keyword;

	std::map<acl::string, acl::string> mapTemp;

	m_bSearching = true;
	if (!m_redis->FindResource(key, mapTemp))
	{
		g_clientlog.error1("查找资源失败！");
		m_bSearching = false;
		return NULL;
	}
	
	//对结果进行转换以使界面直接显示
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

		//发送给UI显示
		SendMessage(m_hNotifyWnd, UM_UPDATE_SEARCH_RESULT, (WPARAM)item, NULL);
	}

	m_bSearching = false;

	return NULL;
}