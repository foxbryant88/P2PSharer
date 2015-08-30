#pragma once
#include "acl_cpp\lib_acl.hpp"

class CFileScan: public acl::thread
{
public:
	CFileScan();
	~CFileScan();

	//初始化扫描对象
	void Init(acl::string dir, std::map<acl::string, acl::string > *oldMapResource);

	void *run();

private:
	//扫描指定目录/磁盘下的所有视频文件
	void ScanVideo(acl::scan_dir &scan, const char *dir, bool brecursive, bool bfullpath);

	//是否是视频文件
	bool IsVideoFile(const char *file);

	//计算文件MD5值
	acl::string CalcMd5(const char *fullfile);

	//缓存当前扫描到的文件信息
	void CacheFileInfo(const char *fullfile);

	//将缓存的文件信息写入文件
	void WriteCacheToFile();

	acl::string m_scandir;     //扫描目标

	//暂存扫描到的文件信息，100条写一次文件并清空
	std::vector<acl::string> m_vfileList;        
	acl::locker m_lockFilelist;   //文件锁
	acl::string m_errmsg;

	//旧的资源列表 用于确保写入到列表文件的信息不重复
	//key：md5, value:文件名|文件路径|MD5|文件大小
	std::map<acl::string, acl::string > *m_mapOldResource;       

};

