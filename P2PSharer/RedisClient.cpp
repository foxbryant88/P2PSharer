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

////����Redis������
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
		g_cli_redislog.msg1("Redis��ʼ���ɹ���addr:%s", m_addr);
		return true;
	}

	g_cli_redislog.msg1("Redis��ʼ��ʧ�ܣ�addr:%s", m_addr);
	return false;
}

//����Դ��ӵ�Hash�� key��HashResList��filed���ļ���|�ļ�MD5��Value���ļ���С
bool CRedisClient::AddResourceToHashList(acl::string &field, acl::string &value)
{
	acl::redis_hash redis(m_redis);

	int ret = 0;
	int iretry = 0; 

RETRY:
	redis.clear();
	if ((ret = redis.hsetnx(REDIS_KEY_RESOURCE_HASH_LIST, field, value)) < 0)
	{
		g_cli_redislog.error1("��Hash������ļ���Ϣʧ�ܣ��ļ�����%s, err:%d", field, acl::last_error());

		//����ʧ������3��
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

//����Դ�б��в�����Դ�ļ�
//������keyword �ؼ���
//      mapResult �����ļ����а���key���ļ��б���Ϣ
bool CRedisClient::FindResource(acl::string &keyword, std::map<acl::string, acl::string> &mapResult)
{
	acl::redis_hash redis(m_redis);
	int cursor = 0;

	std::map <acl::string, acl::string> mapTemp;
	acl::string pattern;
	
	pattern.format("*%s*", keyword.c_str());   //�ļ����а���key������MD5

	int iretry = 0;


	do 
	{
		mapTemp.clear();

	RETRY:
		cursor = redis.hscan(REDIS_KEY_RESOURCE_HASH_LIST, cursor, mapTemp, pattern);	
		if (cursor == -1)
		{
			g_cli_redislog.error1("������Դ�ļ�ʧ�ܣ�err:%d", acl::last_error());

			//����ʧ������3��
			if (iretry < 3)
			{
				Init(m_addr);
				iretry++;
				goto RETRY;
			}

			return false;
		}

		//�ų��ؼ��ֽ���MD5�д��ڵļ�¼
		std::map<acl::string, acl::string>::iterator itMap = mapTemp.begin();
		for (; itMap != mapTemp.end(); ++itMap)
		{
			//���ؼ��ʵ�λ�����ļ�����MD5ֵ�ķָ�����������ų�
			if (itMap->first.find(keyword) > itMap->first.find(SPLITOR_OF_FILE_INFO))
				continue;
			
			mapResult.insert(*itMap);
		}

	} while (cursor > 0);


	return true;
}

//������Դ�ļ����õ����ط���������
//key : �ļ�MD5
//����ӵ�и��ļ��Ŀͻ��˸���
int CRedisClient::GetResourceOwnersID(acl::string &key)
{
	acl::redis_set redis(m_redis);

	int iretry = 0;

RETRY:
	int num = redis.scard(key);
	if (-1 == num)
	{
		g_cli_redislog.error1("��ѯ[%s]���õĿͻ�������ʧ�ܣ�err:%d", key, acl::last_error());

		//����ʧ������3��
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

//������Դ�ļ����õ����ط�����
//key : �ļ�MD5
//����ӵ�и��ļ��Ŀͻ��˸���
int CRedisClient::GetResourceOwnersID(acl::string &key, std::vector<acl::string> &vRes)
{
	if (key == "")
	{
		ShowError("redis key is ��");
	}
	acl::redis_set redis(m_redis);

	int iretry = 0;

RETRY:
	int num = redis.smembers(key, &vRes);
	if (-1 == num)
	{
		g_cli_redislog.error1("��ѯ[%s]���õĿͻ���ʧ�ܣ�err:%d", key, acl::last_error());

		//����ʧ������3��
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

//����Դ�ļ��ĵ�ַ�������MAC����set��ʽ�洢��key:�ļ�MD5 members:MAC��ַ
bool CRedisClient::AddMACToResourceSet(acl::string &key, acl::string &mac)
{
	acl::redis_set redis(m_redis);
	std::vector<acl::string> members;
	members.push_back(mac);

	int iretry = 0;

RETRY:
 	if (-1 == redis.sadd(key, members))
	{
		g_cli_redislog.error1("���ļ�[%s]�ĵ�ַ�������MACʧ�ܣ�err:%d", key, acl::last_error());

		//����ʧ������3��
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

//����Դ�ļ��ĵ�ַ����ɾ��MAC���籾���ļ������
//key:�ļ�MD5 mac:MAC��ַ
bool CRedisClient::RemoveMACFromResourceSet(acl::string &key, acl::string &mac)
{
	acl::redis_set redis(m_redis);
	std::vector<acl::string> members;
	members.push_back(mac);

	int iretry = 0;

RETRY:
	if (-1 == redis.srem(key, members))
	{
		g_cli_redislog.error1("���ļ�[%s]�ĵ�ַ����ɾ��MAC��ַʧ�ܣ�err:%d", key, acl::last_error());

		//����ʧ������3��
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