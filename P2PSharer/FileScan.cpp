#include "stdafx.h"
#include "FileScan.h"

acl::log g_scanlog;

// 去年路径前的 "./" 或 ".\"，因为在 WIN32 下
#define SKIP(ptr) do  \
{  \
if (*ptr == '.' && *(ptr + 1) == '/')  \
	ptr += 2;  \
	else if (*ptr == '.' && *(ptr + 1) == '\\')  \
	ptr += 2;  \
} while (0)

CFileScan::CFileScan()
{
	g_scanlog.open("scanlog.log");
	if (!m_lockFilelist.open(NAME_FILE_INFO_LIST_LOCK))
	{
		m_errmsg.format("创建文件锁失败：%s, err:%d", NAME_FILE_INFO_LIST_LOCK, acl::last_error());
		g_scanlog.error1(m_errmsg);

		ShowError(m_errmsg);
	}
}


CFileScan::~CFileScan()
{
	g_scanlog.close();
}

//初始化扫描对象
void CFileScan::Init(acl::string dir, std::map<acl::string, acl::string > *oldMapResource)
{ 
	m_scandir = dir; 
	m_mapOldResource = oldMapResource;
}

void *CFileScan::run()
{
	acl::scan_dir scan;

	ScanVideo(scan, m_scandir, true, true);

	WriteCacheToFile();

	return NULL;
}

//扫描指定目录/磁盘下的所有视频文件
void CFileScan::ScanVideo(acl::scan_dir &scan, const char *dir, bool brecursive, bool bfullpath)
{
	if (!scan.open(dir, brecursive))
	{
		g_scanlog.error1("打开目录【%ds】失败，err:%d", dir, acl::last_error());
		return;
	}

	const char *filename;
	bool is_file;
	while (true)
	{
		filename = scan.next(true, &is_file);
		if (filename != NULL)
		{
			if (is_file)
			{
				if (IsVideoFile(filename))
				{
					CacheFileInfo(filename);
				}
			}
		}
		else
		{
			//文件名为空可能是因为无权访问，当目录同时也为空时才认为遍历结束
			if (scan.curr_path() == NULL)
			{
				g_scanlog.error1("遍历结束!");
				break;
			}
		}
	}
}

//缓存当前扫描到的文件信息
void CFileScan::CacheFileInfo(const char *fullfile)
{
	struct _stat64 info;
	_stat64(fullfile, &info);
	long long filesize = info.st_size;

	//格式：文件名|全路径|MD5|文件大小
	acl::string temp;
 	acl::string fileInfo;

// 	fileInfo << temp.url_encode(temp.basename(fullfile));        //文件名可能为中文，所以进行编码
	fileInfo << temp.url_encode(temp.basename(fullfile));        //文件名可能为中文，所以进行编码
	fileInfo << SPLITOR_OF_FILE_INFO;
	fileInfo << temp.url_encode(fullfile);                       //文件名可能为中文，所以进行编码
	fileInfo << SPLITOR_OF_FILE_INFO;
	fileInfo << CalcMd5(fullfile);
	fileInfo << SPLITOR_OF_FILE_INFO;
	fileInfo << filesize;

	g_scanlog.msg1("发现视频文件：%s", fileInfo.buf());

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
		//检测到新文件才写入文件列表
		if (m_mapOldResource->find(m_vfileList[i]) == m_mapOldResource->end())
		{
			file.puts(m_vfileList[i]);
		}
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
	if (NULL != ext)
	{
		if (!_stricmp(ext, ".avi") ||
			!_stricmp(ext, ".rmvb") ||
			!_stricmp(ext, ".wmv") ||
			!_stricmp(ext, ".mkv") ||
			!_stricmp(ext, ".mp4") ||
			!_stricmp(ext, ".mov") ||
			!_stricmp(ext, ".mpeg") ||
			!_stricmp(ext, ".3gp") ||
			!_stricmp(ext, ".asf") ||
			!_stricmp(ext, ".mp3"))
		{
			return true;
		}
	}

	return false;
}

//计算文件MD5值
acl::string CFileScan::CalcMd5(const char *fullfile)
{
	char md5val[33] = { 0 };
	if (acl::md5::md5_file(fullfile, NULL, NULL, md5val, 33) > 0)
	{
		return md5val;
	}

	g_scanlog.error1("计算文件【%s】MD5值失败, err:%d", fullfile, acl::last_error());
	return "";
}