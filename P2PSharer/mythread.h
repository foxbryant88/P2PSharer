#pragma once
#include "define.h"
#include <iostream>
#include "lib_acl.h"
#include "acl_cpp\lib_acl.hpp"

static int __timeout = 10;

class mythread :
	public acl::thread
{
public:
	mythread(const char* addr, int count, int length)
		: addr_(addr)
		, count_(count)
		, length_(length)
	{
		req_dat_ = (char *)malloc(length);
		res_dat_ = (char *)malloc(length);
		res_max_ = length;
		memset(req_dat_, 'X', length_);
	}

	~mythread()
	{
		free(req_dat_);
		free(res_dat_);
	}

protected:
	void *run()
	{
		acl::socket_stream conn;
		acl::acl_cpp_init();

		//连接服务器
		if (conn.open(addr_, __timeout, __timeout) == false)
		{
			msg_.format("connect %s error!", addr_.c_str());
			MessageBox(NULL, msg_.c_str(), "error", MB_OK);
			return NULL;
		}

		for (int i = 0; i < count_; i++)
		{
			if (handle_pkg(conn, i) == false)
				break;
		}

		return NULL;
	}

private:
	bool handle_pkg(acl::socket_stream &conn, int i)
	{
		DAT_HDR req_hdr;
		req_hdr.len = htonl(length_);
		ACL_SAFE_STRNCPY(req_hdr.cmd, "SEND", sizeof(req_hdr.cmd));

		//先写数据头
		if (conn.write(&req_hdr, sizeof(req_hdr) == -1))
		{
			MessageBox(NULL, "write hdr to server failed!", "error", MB_OK);
			return false;
		}

		// 再写数据体
		if (conn.write(req_dat_, length_) == -1)
		{
			MessageBox(NULL, "write dat to server failed!", "error", MB_OK);
			return false;
		}

		//////////////////////////////////////////////////////////////

		// 先读响应头
		DAT_HDR res;
		if (conn.read(&res, sizeof(DAT_HDR)) == -1)
		{
			MessageBox(NULL, "read DAT_HDR error!", "error", MB_OK);
			return false;
		}

		// 将响应体数据长度转为主机字节序
		res.len = ntohl(res.len);
		if (res.len <= 0)
		{
			msg_.format("invalid length: %d\r\n", res.len);
			MessageBox(NULL, msg_.c_str(), "error", MB_OK);

			return false;
		}

		// 检测是否需要重新分配读响应数据的内存
		if (res.len + 1 >= res_max_)
		{
			res_max_ = res.len + 1;
			res_dat_ = (char*)realloc(res_dat_, res_max_);
		}

		// 再读响应体
		if (conn.read(res_dat_, res.len) == -1)
		{
			msg_.format("read data error, len: %d\r\n", res.len);
			MessageBox(NULL, msg_.c_str(), "error", MB_OK);

			return false;
		}

		res_dat_[res.len] = 0;
		if (i < 10)
		{
			msg_.format("thread: %lu, cmd: %s, dat: %s, len: %d\r\n", (unsigned long)thread_id(), res.cmd, res_dat_, res.len);
			MessageBox(NULL, "write dat to server failed!", "error", MB_OK);
		}

		return true;
	}
	

private:
	acl::string addr_;
	int count_;
	int length_;
	char *req_dat_;
	char *res_dat_;
	int res_max_;
	acl::string msg_;
};

