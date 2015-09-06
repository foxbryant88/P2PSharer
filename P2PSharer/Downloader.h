///////////////////////////////////////////////////
//文件下载类，负责：
//1.发送分片下载请求
//2.接收分片，重新组装文件
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

	//初始化
	bool Init(T_LOCAL_FILE_INFO &fileinfo, acl::socket_stream &sock);

	//增加可用下载节点
	void AddProvider(const char *addr);

	//处理收到的分块数据,转发给FileReciver
	void Recieve(const char *data);

	//控制分片及请求
	void *run;

	//停止
	void Stop();

private:
	//对文件进行分片
	bool SplitFileSizeIntoBlockMap();

	//获取一批分块
	bool GetBlocks(std::vector<DWORD> &blockNums);

	DWORD m_dwLastBlockPos;                     //最近的分片位置
	std::vector<acl::string> m_vProvider;       //所有可用下载节点
// 	std::vector<DWORD> m_vBlocksCache;          //当前缓存的需要下载的分片
	std::map<DWORD, DWORD> m_mapFileBlocks;     //已下发的分片 key:分片位置 value:分片下发的时间
	acl::socket_stream *m_sock;

	T_LOCAL_FILE_INFO m_fileInfo;
	acl::ofstream *m_fstream;

	CFileReciever *m_objReciever;
	std::vector<CReqSender *> m_vObjSender;
	bool m_bExit;
};

