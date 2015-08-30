#pragma once
#include "acl_cpp\lib_acl.hpp"

class CFileScan: public acl::thread
{
public:
	CFileScan();
	~CFileScan();

	//��ʼ��ɨ�����
	void Init(acl::string dir, std::map<acl::string, acl::string > *oldMapResource);

	void *run();

private:
	//ɨ��ָ��Ŀ¼/�����µ�������Ƶ�ļ�
	void ScanVideo(acl::scan_dir &scan, const char *dir, bool brecursive, bool bfullpath);

	//�Ƿ�����Ƶ�ļ�
	bool IsVideoFile(const char *file);

	//�����ļ�MD5ֵ
	acl::string CalcMd5(const char *fullfile);

	//���浱ǰɨ�赽���ļ���Ϣ
	void CacheFileInfo(const char *fullfile);

	//��������ļ���Ϣд���ļ�
	void WriteCacheToFile();

	acl::string m_scandir;     //ɨ��Ŀ��

	//�ݴ�ɨ�赽���ļ���Ϣ��100��дһ���ļ������
	std::vector<acl::string> m_vfileList;        
	acl::locker m_lockFilelist;   //�ļ���
	acl::string m_errmsg;

	//�ɵ���Դ�б� ����ȷ��д�뵽�б��ļ�����Ϣ���ظ�
	//key��md5, value:�ļ���|�ļ�·��|MD5|�ļ���С
	std::map<acl::string, acl::string > *m_mapOldResource;       

};

