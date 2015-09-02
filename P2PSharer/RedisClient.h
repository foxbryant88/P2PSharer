#pragma once
#include "acl_cpp/lib_acl.hpp"

class CRedisClient
{
public:
	CRedisClient();
	~CRedisClient();

	//���Խӿ�
	void Test();

	//��ʼ��
	bool Init(const char *addr);

	//����Դ��ӵ�Hash�� key��HashResList��filed���ļ���|�ļ�MD5��Value���ļ���С 
	bool AddResourceToHashList(acl::string &field, acl::string &value);

	//����Դ�б��в�����Դ�ļ�
	//������keyword �ؼ���
	//      mapResult �����ļ����а���key���ļ��б���Ϣ
	bool FindResource(acl::string &keyword, std::map<acl::string, acl::string> &mapRes);

	//������Դ�ļ����õ����ط���������
	int GetResourceOwners(acl::string &key);

	//����Դ�ļ��ĵ�ַ�������MAC����set��ʽ�洢��key:�ļ�MD5 members:MAC��ַ
	bool AddMACToResourceSet(acl::string &key, acl::string &mac);

	//����Դ�ļ��ĵ�ַ����ɾ��MAC���籾���ļ������
	//key:�ļ�MD5 mac:MAC��ַ
	bool RemoveMACFromResourceSet(acl::string &key, acl::string &mac);

private:

	//����Redis������
	bool ConnectToRedisServer();

	acl::redis_client *m_redis;
	acl::string m_addr;
};

