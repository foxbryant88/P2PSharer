#pragma once


typedef enum
{
	STATUS_T_HDR,
	STATUS_T_DAT,
} status_t;

// ����ͷ
struct DAT_HDR
{
	int  len;		// �����峤��
	char cmd[64];		// ������
};
