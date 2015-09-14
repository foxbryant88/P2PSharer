#pragma once

typedef struct T_SEARCH_RESULT_INFO
{
	acl::string filemd5;              //MD5
	acl::string filename;             //文件名称（解码后的）
	acl::string filesize;             //文件大小
	acl::string resource_count;       //资源数量
	acl::string owerMac;              //调试用！！！！
};

typedef struct T_LOCAL_FILE_INFO
{
	acl::string filemd5;              //MD5
	acl::string filename;             //文件名称（解码后的）
	acl::string fullpath;             //文件路径（解码后的）
	long long filesize;               //文件大小
};
