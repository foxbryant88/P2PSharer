#pragma once
#include "acl_cpp/lib_acl.hpp"

class CRedisClient
{
public:
	CRedisClient();
	~CRedisClient();

	//测试接口
	void Test();

	//初始化
	bool Init(const char *addr);

	//将资源添加到Hash表 key：HashResList，filed：文件名|文件MD5，Value：文件大小 
	bool AddResourceToHashList(acl::string &field, acl::string &value);

	//在资源列表中查找资源文件
	//参数：keyword 关键字
	//      mapResult 所有文件名中包含key的文件列表信息
	bool FindResource(acl::string &keyword, std::map<acl::string, acl::string> &mapRes);

	//查找资源文件可用的下载服务器个数
	int GetResourceOwners(acl::string &key);

	//向资源文件的地址池中添加MAC（以set形式存储，key:文件MD5 members:MAC地址
	bool AddMACToResourceSet(acl::string &key, acl::string &mac);

	//从资源文件的地址池中删除MAC（如本地文件变更）
	//key:文件MD5 mac:MAC地址
	bool RemoveMACFromResourceSet(acl::string &key, acl::string &mac);

private:

	//连接Redis服务器
	bool ConnectToRedisServer();

	acl::redis_client *m_redis;
	acl::string m_addr;
};

