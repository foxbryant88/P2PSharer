#pragma once


typedef enum
{
	STATUS_T_HDR,
	STATUS_T_DAT,
} status_t;

// 数据头
struct DAT_HDR
{
	int  len;		// 数据体长度
	char cmd[64];		// 命令字
};
