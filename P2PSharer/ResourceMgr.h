#pragma once
#include "RedisClient.h"

class CResourceMgr : public acl::thread
{
public:
	CResourceMgr();
	~CResourceMgr();

	//��ʼ��
	bool Init(const char* addr);

	//����:1.�����б� 2.�����ļ��б����� 3.��Redis�Ǽ�/ע���ļ�
	void *run();

	//��ȡ�ļ���Ϣ,����ֵ��ʽ���ļ���|�ļ�·��|MD5|�ļ���С
	acl::string GetFileInfo(acl::string md5);

	//����Redis�ͻ��˶���
	CRedisClient *GetRedisClient();
private:

	//�����ļ��б�
	void LoadResourceList(std::map<acl::string, acl::string > &mapResource);

	//����/�����б�
	bool UpdateResourceList();

	//��Redis�������Ǽ�/ע����Դ��Ϣ
	bool UpdateResourceToRedis();

	//��ȡ��Ҫɨ��Ĵ���
	bool GetDiskDrives(std::vector<acl::string > &vRes);

	std::map<acl::string, acl::string > m_mapResource;       //��Դ�б� key��md5, value:�ļ���|�ļ�·��|MD5|�ļ���С
	std::map<acl::string, acl::string > m_mapResourceTemp;   //��ʱ��Դ�б� key��md5, value:�ļ���|�ļ�·��|MD5|�ļ���С
	CRedisClient m_redisClient;
	acl::string m_macAddr;                                   //����MAC��ַ
};

