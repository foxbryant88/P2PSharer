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

////连接Redis服务器
bool CRedisClient::Init(const char *addr)
{
	acl::acl_cpp_init();

	m_addr = addr;

	if (NULL != m_redis)
	{
		delete m_redis;
		m_redis = NULL;
	}
		
	m_redis = new acl::redis_client(m_addr);

	if (NULL != m_redis)
	{
		g_cli_redislog.msg1("Redis初始化成功，addr:%s", m_addr);
		return true;
	}

	g_cli_redislog.msg1("Redis初始化失败，addr:%s", m_addr);
	return false;
}

//将资源添加到Hash表 key：HashResList，filed：文件名|文件MD5，Value：文件大小
bool CRedisClient::AddResourceToHashList(acl::string &field, acl::string &value)
{
	acl::redis_hash redis(m_redis);

	int ret = 0;
	int iretry = 0; 

RETRY:
	redis.clear();
	if ((ret = redis.hsetnx(REDIS_KEY_RESOURCE_HASH_LIST, field, value)) < 0)
	{
		g_cli_redislog.error1("向Hash表添加文件信息失败！文件名：%s, err:%d", field, acl::last_error());

		//连接失败重试3次
		if (iretry < 3)
		{
			Init(m_addr);
			iretry++;
			goto RETRY;
		}

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
	
	pattern.format("*%s*", keyword.c_str());   //文件名中包含key，忽略MD5

	int iretry = 0;


	do 
	{
		mapTemp.clear();

	RETRY:
		cursor = redis.hscan(REDIS_KEY_RESOURCE_HASH_LIST, cursor, mapTemp, pattern);	
		if (cursor == -1)
		{
			g_cli_redislog.error1("查找资源文件失败！err:%d", acl::last_error());

			//连接失败重试3次
			if (iretry < 3)
			{
				Init(m_addr);
				iretry++;
				goto RETRY;
			}

			return false;
		}

		//排除关键字仅在MD5中存在的记录
		std::map<acl::string, acl::string>::iterator itMap = mapTemp.begin();
		for (; itMap != mapTemp.end(); ++itMap)
		{
			//若关键词的位置在文件名与MD5值的分隔符后出现则排除
			if (itMap->first.find(keyword) > itMap->first.find(SPLITOR_OF_FILE_INFO))
				continue;
			
			mapResult.insert(*itMap);
		}

	} while (cursor > 0);


	return true;
}

//查找资源文件可用的下载服务器个数
//key : 文件MD5
//返回拥有该文件的客户端个数
int CRedisClient::GetResourceOwnersID(acl::string &key)
{
	acl::redis_set redis(m_redis);

	int iretry = 0;

RETRY:
	int num = redis.scard(key);
	if (-1 == num)
	{
		g_cli_redislog.error1("查询[%s]可用的客户端数量失败！err:%d", key, acl::last_error());

		//连接失败重试3次
		if (iretry < 3)
		{
			Init(m_addr);
			iretry++;
			goto RETRY;
		}

		return 0;
	}

	return num;
}

//查找资源文件可用的下载服务器
//key : 文件MD5
//返回拥有该文件的客户端个数
int CRedisClient::GetResourceOwnersID(acl::string &key, std::vector<acl::string> &vRes)
{
	if (key == "")
	{
		ShowError("redis key is 空");
	}
	acl::redis_set redis(m_redis);

	int iretry = 0;

RETRY:
	int num = redis.smembers(key, &vRes);
	if (-1 == num)
	{
		g_cli_redislog.error1("查询[%s]可用的客户端失败！err:%d", key, acl::last_error());

		//连接失败重试3次
		if (iretry < 3)
		{
			Init(m_addr);
			iretry++;
			goto RETRY;
		}

		return 0;
	}

	return num;
}

//向资源文件的地址池中添加MAC（以set形式存储，key:文件MD5 members:MAC地址
bool CRedisClient::AddMACToResourceSet(acl::string &key, acl::string &mac)
{
	acl::redis_set redis(m_redis);
	std::vector<acl::string> members;
	members.push_back(mac);

	int iretry = 0;

RETRY:
 	if (-1 == redis.sadd(key, members))
	{
		g_cli_redislog.error1("向文件[%s]的地址池中添加MAC失败！err:%d", key, acl::last_error());

		//连接失败重试3次
		if (iretry < 3)
		{
			Init(m_addr);
			iretry++;
			goto RETRY;
		}

		return false;
	}
	
	return true;
}

//从资源文件的地址池中删除MAC（如本地文件变更）
//key:文件MD5 mac:MAC地址
bool CRedisClient::RemoveMACFromResourceSet(acl::string &key, acl::string &mac)
{
	acl::redis_set redis(m_redis);
	std::vector<acl::string> members;
	members.push_back(mac);

	int iretry = 0;

RETRY:
	if (-1 == redis.srem(key, members))
	{
		g_cli_redislog.error1("从文件[%s]的地址池中删除MAC地址失败！err:%d", key, acl::last_error());

		//连接失败重试3次
		if (iretry < 3)
		{
			Init(m_addr);
			iretry++;
			goto RETRY;
		}

		return false;
	}

	return true;
}