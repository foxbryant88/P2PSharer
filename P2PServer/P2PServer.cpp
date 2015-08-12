// P2PServer.cpp : �������̨Ӧ�ó������ڵ㡣
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

// ����ͷ
struct DAT_HDR
{
	int  len;		// �����峤��
	char cmd[64];		// ������
};

/**
* �첽�ͻ������Ļص��������
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
	* ʵ�ָ����е��麯�����ͻ������Ķ��ɹ��ص�����
	* @param data {char*} ���������ݵ�ַ
	* @param len {int} ���������ݳ���
	* @return {bool} ���� true ��ʾ����������ϣ���رո��첽��
	*/
	bool read_callback(char* data, int len)
	{
		// ��ǰ״̬�Ǵ�������ͷʱ
		if (status_ == STATUS_T_HDR)
		{
			// ����ͷ�������Ƿ����Ҫ��
			if (len != sizeof(DAT_HDR))
			{
				printf("invalid len(%d) != DAT_HDR(%d)\r\n",
					len, (int) sizeof(DAT_HDR));
				return false;
			}

			// ȡ�������峤�ȣ�����ָ�����ȵ�������

			DAT_HDR* req_hdr = (DAT_HDR*)data;

			// �������ֽ���תΪ�����ֽ���
			req_hdr->len = ntohl(req_hdr->len);

			if (!parsehdr(req_hdr))
			{
				g_log.error1("��������ͷʧ�ܣ�ip:%s", client_->get_peer());
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
	* ʵ�ָ����е��麯�����ͻ�������д�ɹ��ص�����
	* @return {bool} ���� true ��ʾ����������ϣ���رո��첽��
	*/
	bool write_callback()
	{
		return true;
	}

	/**
	* ʵ�ָ����е��麯�����ͻ������ĳ�ʱ�ص�����
	*/
	void close_callback()
	{
		// �����ڴ˴�ɾ���ö�̬����Ļص�������Է�ֹ�ڴ�й¶
		delete this;
	}

	/**
	* ʵ�ָ����е��麯�����ͻ������ĳ�ʱ�ص�����
	* @return {bool} ���� true ��ʾ����������ϣ���رո��첽��
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

		//����ͻ���������Ϣ
		if (!strcmd.compare("MSG_ONLINE"))
		{
			if (g_clientlist.find(client_->get_peer()) == g_clientlist.end())
			{
				g_log.msg1("�ͻ�������,IP��%s", client_->get_peer(true));
				g_clientlist[client_->get_peer(true)] = client_;
			}

			// ���첽�������ݰ�ͷ
			client_->read(sizeof(DAT_HDR), __timeout);

			return true;
		}

		//����ͻ�������P2P��������
		if (!strcmd.compare("CMD_CONN"))
		{
			status_ = STATUS_T_DAT;

			// �첽��ָ�����ȵ�����
			client_->read(hdr->len, __timeout);
			return true;
		}

		//��Ŀ���ȷ���ѷ���P2P���ӵĴ���(ɾ���������ӵļ�¼)
		if (!strcmd.compare("MSG_OK"))
		{
			std::map<acl::string, acl::string>::iterator it = g_connecting.find(client_->get_peer(true));
			if (it != g_connecting.end())
			{
				g_log.msg1("P2P����ת���������,Դ��%s, Ŀ�꣺%s", it->second, it->first);
				g_connecting.erase(it);
			}
		}
	}

	bool handledat(char* data, int len)
	{
		//���������

		//���Ŀ���Ƿ�����
		if (g_clientlist.find(data) == g_clientlist.end())
		{
			g_log.warn1("�ͻ��������ߣ�%s", client_->get_peer(true));
			return false;
		}

		////////////////////////////////////////////////
		//��Ŀ���·�P2P����ָ��
		acl::string cmd = "CMD_START_P2P";
		DAT_HDR req_hdr;
		req_hdr.len = htonl(cmd.length());        //����ֱ������Ϊ��Ϣ����
		ACL_SAFE_STRNCPY(req_hdr.cmd, cmd, sizeof(req_hdr.cmd));

		acl::aio_socket_stream *ptarget = g_clientlist[data];
		ptarget->write(&req_hdr, sizeof(req_hdr));           //д������ͷ

		acl::string addrsrc = client_->get_peer(true);
		ptarget->write(&addrsrc, addrsrc.length());          //д��P2P����Ŀ���ַ


		//�������ڳ���P2P���ӵļ�¼���յ�Ŀ���ȷ����Ϣ��ɾ����
		g_connecting[ptarget->get_peer(true)] = client_->get_peer(true);

		return true;
	}
};

/**
* �첽�������Ļص��������
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
	* �����麯�������������ӵ������ô˻ص�����
	* @param client {aio_socket_stream*} �첽�ͻ�����
	* @return {bool} ���� true ��֪ͨ��������������
	*/
	bool accept_callback(acl::aio_socket_stream* client)
	{
		g_log.msg1("�յ����ӣ�%s", client->get_peer());

		// �����첽�ͻ������Ļص���������첽�����а�
		io_callback* callback = new io_callback(client);

		// ע���첽���Ķ��ص�����
		client->add_read_callback(callback);

		// ע���첽����д�ص�����
		client->add_write_callback(callback);

		// ע���첽���Ĺرջص�����
		client->add_close_callback(callback);

		// ע���첽���ĳ�ʱ�ص�����
		client->add_timeout_callback(callback);

		// ���첽�������ݰ�ͷ
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

	// �����첽���������
	acl::aio_handle handle(use_kernel ? acl::ENGINE_KERNEL : acl::ENGINE_SELECT);

	// ���������첽��
	acl::aio_listen_stream* sstream = new acl::aio_listen_stream(&handle);

	// ��ʼ��ACL��(��������WIN32��һ��Ҫ���ô˺�������UNIXƽ̨�¿ɲ�����)
	acl::acl_cpp_init();

	// ����ָ���ĵ�ַ
	if (sstream->open(addr.c_str()) == false)
	{
		std::cout << "open " << addr.c_str() << " error!" << std::endl;
		sstream->close();
		g_log.error1("open %s error!", addr.c_str());

		// XXX: Ϊ�˱�֤�ܹرռ�������Ӧ�ڴ˴��� check һ��
		handle.check();

		getchar();
		return (1);
	}

	// �����ص�����󣬵��������ӵ���ʱ�Զ����ô������Ļص�����
	io_accept_callback callback;
	sstream->add_accept_callback(&callback);
	std::cout << "Listen: " << addr.c_str() << " ok!" << std::endl;

	while (true)
	{
		// ������� false ���ʾ���ټ�������Ҫ�˳�
		if (handle.check() == false)
		{
			std::cout << "pkg_server stop now ..." << std::endl;
			break;
		}
	}

	// �رռ��������ͷ�������
	sstream->close();

	// XXX: Ϊ�˱�֤�ܹرռ�������Ӧ�ڴ˴��� check һ��
	handle.check();


	return 0;
}

