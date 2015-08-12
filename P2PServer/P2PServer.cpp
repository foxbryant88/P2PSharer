// P2PServer.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <iostream>
#include <assert.h>
#include <map>
#include "lib_acl.h"
#include "acl_cpp/lib_acl.hpp"
#include "acl_cpp/stdlib/log.hpp"
#include "acl_cpp/stdlib/string.hpp"

acl::log g_log;
static int   __timeout = 0;
std::map<acl::string, acl::aio_socket_stream *> g_clientlist;
std::map<acl::string, acl::string> g_connecting;

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

/**
* 异步客户端流的回调类的子类
*/
class io_callback : public acl::aio_callback
{
public:
	io_callback(acl::aio_socket_stream* client)
		: status_(STATUS_T_HDR)
		, client_(client)
		, i_(0)
	{
	}

	~io_callback()
	{
		std::cout << "delete io_callback now ..." << std::endl;
	}

	/**
	* 实现父类中的虚函数，客户端流的读成功回调过程
	* @param data {char*} 读到的数据地址
	* @param len {int} 读到的数据长度
	* @return {bool} 返回 true 表示继续，否则希望关闭该异步流
	*/
	bool read_callback(char* data, int len)
	{
		// 当前状态是处理数据头时
		if (status_ == STATUS_T_HDR)
		{
			// 检验头部长度是否符合要求
			if (len != sizeof(DAT_HDR))
			{
				printf("invalid len(%d) != DAT_HDR(%d)\r\n",
					len, (int) sizeof(DAT_HDR));
				return false;
			}

			// 取出数据体长度，并读指定长度的数据体

			DAT_HDR* req_hdr = (DAT_HDR*)data;

			// 将网络字节序转为主机字节序
			req_hdr->len = ntohl(req_hdr->len);

			if (!parsehdr(req_hdr))
			{
				g_log.error1("解析数据头失败！ip:%s", client_->get_peer());
				return false;
			}
		}

		if (status_ == STATUS_T_DAT)
		{
			handledat(data, len);
		}


		return true;
	}

	/**
	* 实现父类中的虚函数，客户端流的写成功回调过程
	* @return {bool} 返回 true 表示继续，否则希望关闭该异步流
	*/
	bool write_callback()
	{
		return true;
	}

	/**
	* 实现父类中的虚函数，客户端流的超时回调过程
	*/
	void close_callback()
	{
		// 必须在此处删除该动态分配的回调类对象以防止内存泄露
		delete this;
	}

	/**
	* 实现父类中的虚函数，客户端流的超时回调过程
	* @return {bool} 返回 true 表示继续，否则希望关闭该异步流
	*/
	bool timeout_callback()
	{
		std::cout << "Timeout, delete it ..." << std::endl;
		return false;
	}

private:
	status_t status_;
	acl::aio_socket_stream* client_;
	int   i_;

	bool parsehdr(DAT_HDR *hdr)
	{
		acl::string strcmd = hdr->cmd;

		//处理客户端上线消息
		if (!strcmd.compare("MSG_ONLINE"))
		{
			if (g_clientlist.find(client_->get_peer()) == g_clientlist.end())
			{
				g_log.msg1("客户端上线,IP：%s", client_->get_peer(true));
				g_clientlist[client_->get_peer(true)] = client_;
			}

			// 从异步流读数据包头
			client_->read(sizeof(DAT_HDR), __timeout);

			return true;
		}

		//处理客户端请求P2P连接命令
		if (!strcmd.compare("CMD_CONN"))
		{
			status_ = STATUS_T_DAT;

			// 异步读指定长度的数据
			client_->read(hdr->len, __timeout);
			return true;
		}

		//对目标机确认已发起P2P连接的处理(删除正在连接的记录)
		if (!strcmd.compare("MSG_OK"))
		{
			std::map<acl::string, acl::string>::iterator it = g_connecting.find(client_->get_peer(true));
			if (it != g_connecting.end())
			{
				g_log.msg1("P2P连接转发命令完成,源：%s, 目标：%s", it->second, it->first);
				g_connecting.erase(it);
			}
		}
	}

	bool handledat(char* data, int len)
	{
		//处理打洞流程

		//检查目标是否在线
		if (g_clientlist.find(data) == g_clientlist.end())
		{
			g_log.warn1("客户端已下线：%s", client_->get_peer(true));
			return false;
		}

		////////////////////////////////////////////////
		//向目标下发P2P连接指令
		acl::string cmd = "CMD_START_P2P";
		DAT_HDR req_hdr;
		req_hdr.len = htonl(cmd.length());        //长度直接设置为消息长度
		ACL_SAFE_STRNCPY(req_hdr.cmd, cmd, sizeof(req_hdr.cmd));

		acl::aio_socket_stream *ptarget = g_clientlist[data];
		ptarget->write(&req_hdr, sizeof(req_hdr));           //写入请求头

		acl::string addrsrc = client_->get_peer(true);
		ptarget->write(&addrsrc, addrsrc.length());          //写入P2P连接目标地址


		//保存正在尝试P2P连接的记录（收到目标机确认消息后删除）
		g_connecting[ptarget->get_peer(true)] = client_->get_peer(true);

		return true;
	}
};

/**
* 异步监听流的回调类的子类
*/
class io_accept_callback : public acl::aio_accept_callback
{
public:
	io_accept_callback() {}
	~io_accept_callback()
	{
		printf(">>io_accept_callback over!\n");
	}

	/**
	* 基类虚函数，当有新连接到达后调用此回调过程
	* @param client {aio_socket_stream*} 异步客户端流
	* @return {bool} 返回 true 以通知监听流继续监听
	*/
	bool accept_callback(acl::aio_socket_stream* client)
	{
		g_log.msg1("收到连接：%s", client->get_peer());

		// 创建异步客户端流的回调对象并与该异步流进行绑定
		io_callback* callback = new io_callback(client);

		// 注册异步流的读回调过程
		client->add_read_callback(callback);

		// 注册异步流的写回调过程
		client->add_write_callback(callback);

		// 注册异步流的关闭回调过程
		client->add_close_callback(callback);

		// 注册异步流的超时回调过程
		client->add_timeout_callback(callback);

		// 从异步流读数据包头
		client->read(sizeof(DAT_HDR), __timeout);

		return (true);
	}
};


int _tmain(int argc, _TCHAR* argv[])
{
	bool use_kernel = false;
	int  ch;
	acl::string addr(":1900");
	g_log.open("P2PServer.log", "P2PServer");

	acl::log::stdout_open(true);

	// 构建异步引擎类对象
	acl::aio_handle handle(use_kernel ? acl::ENGINE_KERNEL : acl::ENGINE_SELECT);

	// 创建监听异步流
	acl::aio_listen_stream* sstream = new acl::aio_listen_stream(&handle);

	// 初始化ACL库(尤其是在WIN32下一定要调用此函数，在UNIX平台下可不调用)
	acl::acl_cpp_init();

	// 监听指定的地址
	if (sstream->open(addr.c_str()) == false)
	{
		std::cout << "open " << addr.c_str() << " error!" << std::endl;
		sstream->close();
		g_log.error1("open %s error!", addr.c_str());

		// XXX: 为了保证能关闭监听流，应在此处再 check 一下
		handle.check();

		getchar();
		return (1);
	}

	// 创建回调类对象，当有新连接到达时自动调用此类对象的回调过程
	io_accept_callback callback;
	sstream->add_accept_callback(&callback);
	std::cout << "Listen: " << addr.c_str() << " ok!" << std::endl;

	while (true)
	{
		// 如果返回 false 则表示不再继续，需要退出
		if (handle.check() == false)
		{
			std::cout << "pkg_server stop now ..." << std::endl;
			break;
		}
	}

	// 关闭监听流并释放流对象
	sstream->close();

	// XXX: 为了保证能关闭监听流，应在此处再 check 一下
	handle.check();


	return 0;
}

