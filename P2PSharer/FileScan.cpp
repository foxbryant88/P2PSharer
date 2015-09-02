#include "stdafx.h"
#include "FileScan.h"

acl::log g_scanlog;

// ȥ��·��ǰ�� "./" �� ".\"����Ϊ�� WIN32 ��
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
		m_errmsg.format("�����ļ���ʧ�ܣ�%s, err:%d", NAME_FILE_INFO_LIST_LOCK, acl::last_error());
		g_scanlog.error1(m_errmsg);

		ShowError(m_errmsg);
	}
}


CFileScan::~CFileScan()
{
	g_scanlog.close();
}

//��ʼ��ɨ�����
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

//ɨ��ָ��Ŀ¼/�����µ�������Ƶ�ļ�
void CFileScan::ScanVideo(acl::scan_dir &scan, const char *dir, bool brecursive, bool bfullpath)
{
	if (!scan.open(dir, brecursive))
	{
		g_scanlog.error1("��Ŀ¼��%ds��ʧ�ܣ�err:%d", dir, acl::last_error());
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
			//�ļ���Ϊ�տ�������Ϊ��Ȩ���ʣ���Ŀ¼ͬʱҲΪ��ʱ����Ϊ��������
			if (scan.curr_path() == NULL)
			{
				g_scanlog.error1("��������!");
				break;
			}
		}
	}
}

//���浱ǰɨ�赽���ļ���Ϣ
void CFileScan::CacheFileInfo(const char *fullfile)
{
	struct _stat64 info;
	_stat64(fullfile, &info);
	long long filesize = info.st_size;

	//��ʽ���ļ���|ȫ·��|MD5|�ļ���С
	acl::string temp;
 	acl::string fileInfo;

// 	fileInfo << temp.url_encode(temp.basename(fullfile));        //�ļ�������Ϊ���ģ����Խ��б���
	fileInfo << temp.url_encode(temp.basename(fullfile));        //�ļ�������Ϊ���ģ����Խ��б���
	fileInfo << SPLITOR_OF_FILE_INFO;
	fileInfo << temp.url_encode(fullfile);                       //�ļ�������Ϊ���ģ����Խ��б���
	fileInfo << SPLITOR_OF_FILE_INFO;
	fileInfo << CalcMd5(fullfile);
	fileInfo << SPLITOR_OF_FILE_INFO;
	fileInfo << filesize;

	g_scanlog.msg1("������Ƶ�ļ���%s", fileInfo.buf());

	//С��100��ʱ���棬����д���ļ�
	if (m_vfileList.size() < 100)
		m_vfileList.push_back(fileInfo);
	else
		WriteCacheToFile();
}

//��������ļ���Ϣд���ļ�
void CFileScan::WriteCacheToFile()
{
	m_lockFilelist.lock();
	acl::ofstream file;
	file.open_append(NAME_FILE_INFO_LIST);

	for (int i = 0; i < m_vfileList.size(); i++)
	{
		//��⵽���ļ���д���ļ��б�
		if (m_mapOldResource->find(m_vfileList[i]) == m_mapOldResource->end())
		{
			file.puts(m_vfileList[i]);
		}
	}

	file.close();
	m_vfileList.clear();
	m_lockFilelist.unlock();
}

//�Ƿ�����Ƶ�ļ�
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

//�����ļ�MD5ֵ
acl::string CFileScan::CalcMd5(const char *fullfile)
{
	char md5val[33] = { 0 };
	if (acl::md5::md5_file(fullfile, NULL, NULL, md5val, 33) > 0)
	{
		return md5val;
	}

	g_scanlog.error1("�����ļ���%s��MD5ֵʧ��, err:%d", fullfile, acl::last_error());
	return "";
}