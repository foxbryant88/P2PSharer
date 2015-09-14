#pragma once

typedef struct T_SEARCH_RESULT_INFO
{
	acl::string filemd5;              //MD5
	acl::string filename;             //�ļ����ƣ������ģ�
	acl::string filesize;             //�ļ���С
	acl::string resource_count;       //��Դ����
	acl::string owerMac;              //�����ã�������
};

typedef struct T_LOCAL_FILE_INFO
{
	acl::string filemd5;              //MD5
	acl::string filename;             //�ļ����ƣ������ģ�
	acl::string fullpath;             //�ļ�·���������ģ�
	long long filesize;               //�ļ���С
};
