#include "stdafx.h"
#include "FileScan.h"


acl::log g_scanlog;
CFileScan::CFileScan()
{
	g_scanlog.open("scanlog.log");
	if (!m_lockFilelist.open(NAME_FILE_INFO_LIST))
	{
		m_errmsg.format("创建文件锁失败：%s, err:%d", NAME_FILE_INFO_LIST, acl::last_error());
		g_scanlog.error1(m_errmsg);

		ShowError(m_errmsg);
	}
}


CFileScan::~CFileScan()
{
	g_scanlog.close();
}


void *CFileScan::run()
{
	acl::scan_dir scan;

	Scan(scan, m_scandir, true, true);

	return NULL;
}

//扫描指定目录/磁盘下的所有视频文件
void CFileScan::Scan(acl::scan_dir &scan, const char *dir, bool brecursive, bool bfullpath)
{
	if (!scan.open(dir, brecursive))
	{
		g_scanlog.error1("打开目录【%ds】失败，err:%d", dir, acl::last_error());
		return;
	}

	const char *filename;
	bool isFile;
	while ((filename = scan.next(true, &isFile)) != NULL)
	{
		if (isFile)
		{
			if (IsVideoFile(filename))
			{
				CacheFileInfo(filename);
			}
		}
	}
}

//缓存当前扫描到的文件信息
void CFileScan::CacheFileInfo(const char *fullfile)
{
	acl::ifstream infile;
	if (!infile.create(fullfile))
	{
		g_scanlog.error1("获取文件长度时，不能打开文件【%s】!err;%d", fullfile, acl::last_error());
		return;
	}

	long long filesize = infile.fsize();;
	infile.close();

	//格式：文件名|全路径|MD5|文件大小
	acl::string temp;
	temp.basename(fullfile);

	acl::string fileInfo;
// 	fileInfo.format("%s%s%s%s%s%s%I64d", 
// 		temp.basename(fullfile), 
// 		SPLITOR_OF_FILE_INFO,
// 		fullfile, 
// 		SPLITOR_OF_FILE_INFO, 
// 		CalcMd5(fullfile),
// 		SPLITOR_OF_FILE_INFO, 
// 		filesize);
	
	char buf[20] = { 0 };
	_snprintf_s(buf, 20, "%I64d", filesize);
	fileInfo.format("%s|%s|%s|%s",
		"aaa",//temp,
		"bbb",//fullfile,
		"ccc",//CalcMd5(fullfile),
		buf);


	g_scanlog.msg1("发现视频文件：%s", fileInfo);

	//小于100条时缓存，否则写入文件
	if (m_vfileList.size() < 100)
		m_vfileList.push_back(fileInfo);
	else
		WriteCacheToFile();
}

//将缓存的文件信息写入文件
void CFileScan::WriteCacheToFile()
{
	m_lockFilelist.lock();
	acl::ofstream file;
	file.open_append(NAME_FILE_INFO_LIST);

	for (int i = 0; i < m_vfileList.size(); i++)
	{
		file.puts(m_vfileList[i]);
	}

	file.close();
	m_vfileList.clear();
	m_lockFilelist.unlock();
}

//是否是视频文件
bool CFileScan::IsVideoFile(const char *filename)
{
	acl::string file(filename);
	char *ext = file.rfind(".");

	if (!_stricmp(ext, ".avi") ||
		!_stricmp(ext, ".rmvb") ||
		!_stricmp(ext, ".wmv") ||
		!_stricmp(ext, ".mkv") ||
		!_stricmp(ext, ".mp4") ||
		!_stricmp(ext, ".mov") ||
		!_stricmp(ext, ".mpeg") ||
		!_stricmp(ext, ".3gp") ||
		!_stricmp(ext, ".asf"))
	{
		return true;
	}

	return false;
}

//计算文件MD5值
acl::string CFileScan::CalcMd5(const char *fullfile)
{
	char md5val[33] = { 0 };
	if (acl::md5::md5_file(fullfile, NULL, NULL, md5val, 33) > 0)
	{
		g_scanlog.error1("计算文件【%s】MD5值失败, err:%d", fullfile, acl::last_error());
		return md5val;
	}

	return "";
}