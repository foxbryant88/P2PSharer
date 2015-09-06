///////////////////////////////////////////////////
//�ļ������࣬����
//1.���ͷ�Ƭ��������
//2.���շ�Ƭ��������װ�ļ�
#pragma once
#include "acl_cpp\stdlib\thread.hpp"
#include "CommonDefine.h"
#include "ReqSender.h"
#include "FileReciever.h"

#define MAX_CACHE_BLOCKS  2048

class CDownloader : public acl::thread
{
public:
	CDownloader();
	~CDownloader();

	//��ʼ��
	bool Init(T_LOCAL_FILE_INFO &fileinfo, acl::socket_stream &sock);

	//���ӿ������ؽڵ�
	void AddProvider(const char *addr);

	//�����յ��ķֿ�����,ת����FileReciver
	void Recieve(const char *data);

	//���Ʒ�Ƭ������
	void *run;

	//ֹͣ
	void Stop();

private:
	//���ļ����з�Ƭ
	bool SplitFileSizeIntoBlockMap();

	//��ȡһ���ֿ�
	bool GetBlocks(std::vector<DWORD> &blockNums);

	DWORD m_dwLastBlockPos;                     //����ķ�Ƭλ��
	std::vector<acl::string> m_vProvider;       //���п������ؽڵ�
// 	std::vector<DWORD> m_vBlocksCache;          //��ǰ�������Ҫ���صķ�Ƭ
	std::map<DWORD, DWORD> m_mapFileBlocks;     //���·��ķ�Ƭ key:��Ƭλ�� value:��Ƭ�·���ʱ��
	acl::socket_stream *m_sock;

	T_LOCAL_FILE_INFO m_fileInfo;
	acl::ofstream *m_fstream;

	CFileReciever *m_objReciever;
	std::vector<CReqSender *> m_vObjSender;
	bool m_bExit;
};

