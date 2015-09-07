///////////////////////////////////////////////////
//�ļ������࣬����
//1.���ͷ�Ƭ��������
//2.���շ�Ƭ��������װ�ļ�
//3.����MD5ֵ��Ϣ�Զ����������ؽڵ�
#pragma once
#include "acl_cpp\stdlib\thread.hpp"
#include "CommonDefine.h"
#include "ReqSender.h"
#include "FileReciever.h"

#define MAX_CACHE_BLOCKS  2048
#define BLOCK_REQUEST_TIME_OUT          300000    //��Ƭ���س�ʱʱ�䣨Ĭ��5���ӣ�
#define UPDATE_SERVICE_PROVIDER_TIME    60000     //���·������ؽڵ��ʱ��
class CDownloader : public acl::thread
{
public:
	CDownloader();
	~CDownloader();

	//��ʼ��
	bool Init(T_LOCAL_FILE_INFO &fileinfo, acl::socket_stream &sock, CRedisClient *redis);

	//�����յ��ķֿ�����,ת����FileReciver
	void Recieve(void *data);

	//���Ʒ�Ƭ������
	void *run();

	//ֹͣ
	void Stop();

private:
	//���ļ����з�Ƭ
	bool SplitFileSizeIntoBlockMap();

	//�Գ���5����δ��Ӧ�ķ�Ƭ����ѹ���Ƭ�б�
	void DealTimeoutBlockRequests();

	//��ȡһ���ֿ�
	bool GetBlocks(std::vector<DWORD> &blockNums);

	//ÿ��1������������һ�η���ڵ�
	bool UpdateServiceProvider(void);

	//���ӿ������ؽڵ�
	void AddProvider(acl::string &addr);


	DWORD m_dwLastBlockPos;                     //����ķ�Ƭλ��
	std::vector<acl::string> m_vProvider;       //���п������ؽڵ�
// 	std::vector<DWORD> m_vBlocksCache;          //��ǰ�������Ҫ���صķ�Ƭ
	std::map<DWORD, DWORD> m_mapFileBlocks;     //���·��ķ�Ƭ key:��Ƭλ�� value:��Ƭ�·���ʱ��
	acl::socket_stream *m_sock;
    CRedisClient *m_redis;

	T_LOCAL_FILE_INFO m_fileInfo;
	acl::ofstream *m_fstream;

	CFileReciever *m_objReciever;
	std::vector<CReqSender *> m_vObjSender;
	bool m_bExit;
};

