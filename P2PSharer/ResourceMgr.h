#pragma once
#include "RedisClient.h"

class CResourceMgr : public acl::thread
{
public:
	CResourceMgr();
	~CResourceMgr();

	//初始化
	bool Init(const char* addr);

	//功能:1.生成列表 2.更新文件列表到缓存 3.向Redis登记/注销文件
	void *run();

	//获取文件信息,返回值格式：文件名|文件路径|MD5|文件大小
	acl::string GetFileInfo(acl::string md5);

	//返回Redis客户端对象
	CRedisClient *GetRedisClient();
private:

	//加载文件列表
	void LoadResourceList(std::map<acl::string, acl::string > &mapResource);

	//生成/更新列表
	bool UpdateResourceList();

	//向Redis服务器登记/注销资源信息
	bool UpdateResourceToRedis();

	//获取需要扫描的磁盘
	bool GetDiskDrives(std::vector<acl::string > &vRes);

	std::map<acl::string, acl::string > m_mapResource;       //资源列表 key：md5, value:文件名|文件路径|MD5|文件大小
	std::map<acl::string, acl::string > m_mapResourceTemp;   //临时资源列表 key：md5, value:文件名|文件路径|MD5|文件大小
	CRedisClient m_redisClient;
	acl::string m_macAddr;                                   //本机MAC地址
};

