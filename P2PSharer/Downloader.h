///////////////////////////////////////////////////
//文件下载类，负责：
//1.发送分片下载请求
//2.接收分片，重新组装文件
//3.根据MD5值信息自动搜索可下载节点
#pragma once
#include "acl_cpp\stdlib\thread.hpp"
#include "CommonDefine.h"
#include "ReqSender.h"
#include "FileReciever.h"

#define MAX_CACHE_BLOCKS  2048
#define BLOCK_REQUEST_TIME_OUT          300000    //分片下载超时时间（默认5分钟）
#define UPDATE_SERVICE_PROVIDER_TIME    60000     //更新服务下载节点的时间
class CDownloader : public acl::thread
{
public:
	CDownloader();
	~CDownloader();

	//初始化
	bool Init(T_LOCAL_FILE_INFO &fileinfo, acl::socket_stream &sock, CRedisClient *redis);

	//处理收到的分块数据,转发给FileReciver
	void Recieve(void *data);

	//控制分片及请求
	void *run();

	//停止
	void Stop();

private:
	//对文件进行分片
	bool SplitFileSizeIntoBlockMap();

	//对超过5分钟未响应的分片重新压入分片列表
	void DealTimeoutBlockRequests();

	//获取一批分块
	bool GetBlocks(std::vector<DWORD> &blockNums);

	//每隔1分钟重新搜索一次服务节点
	bool UpdateServiceProvider(void);

	//增加可用下载节点
	void AddProvider(acl::string &addr);


	DWORD m_dwLastBlockPos;                     //最近的分片位置
	std::vector<acl::string> m_vProvider;       //所有可用下载节点
// 	std::vector<DWORD> m_vBlocksCache;          //当前缓存的需要下载的分片
	std::map<DWORD, DWORD> m_mapFileBlocks;     //已下发的分片 key:分片位置 value:分片下发的时间
	acl::socket_stream *m_sock;
    CRedisClient *m_redis;

	T_LOCAL_FILE_INFO m_fileInfo;
	acl::ofstream *m_fstream;

	CFileReciever *m_objReciever;
	std::vector<CReqSender *> m_vObjSender;
	bool m_bExit;
};

