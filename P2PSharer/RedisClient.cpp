#include "stdafx.h"
#include "RedisClient.h"
#include <map>

acl::log g_cli_redislog;

#define REDIS_KEY_RESOURCE_HASH_LIST "HashResourceList"

CRedisClient::CRedisClient()
{
	m_redis = NULL;

	g_cli_redislog.open("redis.log");
}


CRedisClient::~CRedisClient()
{
}

//初始化
bool CRedisClient::Init(const char *addr)
{
	m_addr = addr;

	if (NULL == m_redis)
		m_redis = new acl::redis_client(m_addr);

	if (m_redis)
	{
		g_cli_redislog.msg1("连接Redis服务器[%s]成功！", addr);
		return true;
	}

	g_cli_redislog.msg1("连接Redis服务器[%s]失败！", addr);
	return false;
}

//将资源添加到Hash表 key：HashResList，filed：文件名|文件MD5，Value：文件大小
bool CRedisClient::AddResourceToHashList(acl::string &field, acl::string &value)
{
	acl::redis_hash redis(m_redis);

	int ret = 0;
	redis.clear();
	if ((ret = redis.hsetnx(REDIS_KEY_RESOURCE_HASH_LIST, field, value)) < 0)
	{
		g_cli_redislog.error1("向Hash表添加文件信息失败！文件名：%s, err:%d", field, acl::last_error());
		return false;
	}

	return true;
}

//在资源列表中查找资源文件
//参数：keyword 关键字
//      mapResult 所有文件名中包含key的文件列表信息
bool CRedisClient::FindResource(acl::string &keyword, std::map<acl::string, acl::string> &mapResult)
{
	acl::redis_hash redis(m_redis);
	int cursor = 0;

	std::map <acl::string, acl::string> mapTemp;
	acl::string pattern;
	pattern.format("*%s*|*)", keyword);   //文件名中包含key，忽略MD5

	do 
	{
		mapTemp.clear();
		cursor = redis.hscan(REDIS_KEY_RESOURCE_HASH_LIST, cursor, mapTemp, pattern);	
		if (cursor == -1)
		{
			g_cli_redislog.error1("查找资源文件失败！err:%d", acl::last_error());
			return false;
		}

		mapResult.insert(mapTemp.begin(), mapTemp.end());

	} while (cursor > 0);

	return true;
}

//向资源文件的地址池中添加MAC（以set形式存储，key:文件MD5 members:MAC地址
bool CRedisClient::AddMACToResourceSet(acl::string &key, acl::string &mac)
{
	acl::redis_set redis(m_redis);
	if (-1 == redis.sadd(key, mac))
	{
		g_cli_redislog.error1("向文件[%s]的地址池中添加MAC失败！err:%d", key, acl::last_error());
		return false;
	}
	
	return true;
}

//从资源文件的地址池中删除MAC（如本地文件变更）
bool CRedisClient::RemoveMACFromResourceSet(acl::string &key, acl::string &mac)
{
	acl::redis_set redis(m_redis);
	if (-1 == redis.srem(key, mac))
	{
		g_cli_redislog.error1("从文件[%s]的地址池中删除MAC地址失败！err:%d", key, acl::last_error());
		return false;
	}

	return true;
}