#include "stdafx.h"
#include "FileScan.h"


acl::log g_scanlog;
CFileScan::CFileScan()
{
	g_scanlog.open("scanlog.log");
	if (!m_lockFilelist.open(NAME_FILE_INFO_LIST))
	{
		m_errmsg.format("�����ļ���ʧ�ܣ�%s, err:%d", NAME_FILE_INFO_LIST, acl::last_error());
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

//ɨ��ָ��Ŀ¼/�����µ�������Ƶ�ļ�
void CFileScan::Scan(acl::scan_dir &scan, const char *dir, bool brecursive, bool bfullpath)
{
	if (!scan.open(dir, brecursive))
	{
		g_scanlog.error1("��Ŀ¼��%ds��ʧ�ܣ�err:%d", dir, acl::last_error());
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

//���浱ǰɨ�赽���ļ���Ϣ
void CFileScan::CacheFileInfo(const char *fullfile)
{
	acl::ifstream infile;
	if (!infile.create(fullfile))
	{
		g_scanlog.error1("��ȡ�ļ�����ʱ�����ܴ��ļ���%s��!err;%d", fullfile, acl::last_error());
		return;
	}

	long long filesize = infile.fsize();;
	infile.close();

	//��ʽ���ļ���|ȫ·��|MD5|�ļ���С
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


	g_scanlog.msg1("������Ƶ�ļ���%s", fileInfo);

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
		file.puts(m_vfileList[i]);
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

//�����ļ�MD5ֵ
acl::string CFileScan::CalcMd5(const char *fullfile)
{
	char md5val[33] = { 0 };
	if (acl::md5::md5_file(fullfile, NULL, NULL, md5val, 33) > 0)
	{
		g_scanlog.error1("�����ļ���%s��MD5ֵʧ��, err:%d", fullfile, acl::last_error());
		return md5val;
	}

	return "";
}