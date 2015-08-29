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

//��ʼ��
bool CRedisClient::Init(const char *addr)
{
	m_addr = addr;

	if (NULL == m_redis)
		m_redis = new acl::redis_client(m_addr);

	if (m_redis)
	{
		g_cli_redislog.msg1("����Redis������[%s]�ɹ���", addr);
		return true;
	}

	g_cli_redislog.msg1("����Redis������[%s]ʧ�ܣ�", addr);
	return false;
}

//����Դ��ӵ�Hash�� key��HashResList��filed���ļ���|�ļ�MD5��Value���ļ���С
bool CRedisClient::AddResourceToHashList(acl::string &field, acl::string &value)
{
	acl::redis_hash redis(m_redis);

	int ret = 0;
	redis.clear();
	if ((ret = redis.hsetnx(REDIS_KEY_RESOURCE_HASH_LIST, field, value)) < 0)
	{
		g_cli_redislog.error1("��Hash������ļ���Ϣʧ�ܣ��ļ�����%s, err:%d", field, acl::last_error());
		return false;
	}

	return true;
}

//����Դ�б��в�����Դ�ļ�
//������keyword �ؼ���
//      mapResult �����ļ����а���key���ļ��б���Ϣ
bool CRedisClient::FindResource(acl::string &keyword, std::map<acl::string, acl::string> &mapResult)
{
	acl::redis_hash redis(m_redis);
	int cursor = 0;

	std::map <acl::string, acl::string> mapTemp;
	acl::string pattern;
	pattern.format("*%s*|*)", keyword);   //�ļ����а���key������MD5

	do 
	{
		mapTemp.clear();
		cursor = redis.hscan(REDIS_KEY_RESOURCE_HASH_LIST, cursor, mapTemp, pattern);	
		if (cursor == -1)
		{
			g_cli_redislog.error1("������Դ�ļ�ʧ�ܣ�err:%d", acl::last_error());
			return false;
		}

		mapResult.insert(mapTemp.begin(), mapTemp.end());

	} while (cursor > 0);

	return true;
}

//����Դ�ļ��ĵ�ַ�������MAC����set��ʽ�洢��key:�ļ�MD5 members:MAC��ַ
bool CRedisClient::AddMACToResourceSet(acl::string &key, acl::string &mac)
{
	acl::redis_set redis(m_redis);
	if (-1 == redis.sadd(key, mac))
	{
		g_cli_redislog.error1("���ļ�[%s]�ĵ�ַ�������MACʧ�ܣ�err:%d", key, acl::last_error());
		return false;
	}
	
	return true;
}

//����Դ�ļ��ĵ�ַ����ɾ��MAC���籾���ļ������
bool CRedisClient::RemoveMACFromResourceSet(acl::string &key, acl::string &mac)
{
	acl::redis_set redis(m_redis);
	if (-1 == redis.srem(key, mac))
	{
		g_cli_redislog.error1("���ļ�[%s]�ĵ�ַ����ɾ��MAC��ַʧ�ܣ�err:%d", key, acl::last_error());
		return false;
	}

	return true;
}