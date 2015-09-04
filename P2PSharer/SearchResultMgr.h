#pragma once
#include "acl_cpp\lib_acl.hpp"

class CSearchResultMgr :
	public acl::thread
{
public:
	CSearchResultMgr(HWND hwnd, CRedisClient *redis);
	~CSearchResultMgr();

 	//设置搜索关键字
 	bool SetSearchWord(const char *keyword);

	//是否正在搜索
	bool IsSearching(void);

	//执行搜索的线程函数
 	void *run();

private:
	HWND m_hNotifyWnd;        //接收搜索结果的窗口句柄
	bool m_bSearching;        //是否正在执行搜索
	CRedisClient *m_redis;
	acl::string m_keyword;
	std::map<acl::string, acl::string> m_mapSearchResult;  //存储搜索结果（有URL编码）  格式:  key:文件名|文件MD5，Value：文件大小
};

