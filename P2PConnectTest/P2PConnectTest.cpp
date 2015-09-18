// P2PConnectTest.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "ServerEX.h"


int _tmain(int argc, _TCHAR* argv[])
{
	const char *ip = "fdsafs";
	acl::string arr[10];
	for (int i = 0; i < 10; i++)
	{
		arr[i] = ip;
	}

	acl::string srcName;
	printf("请输入用户名:\r\n");
	cin.getline(srcName.c_str(), 50);

	ServerEX ex;
	//ex.Init("127.0.0.1:8888", srcName.c_str());
	ex.Init("119.29.66.237:8888", srcName.c_str());

	ex.set_detachable(true);
	ex.start();

	ex.SendMsg_UserLogin();
	getchar();

	acl::string tarName;
	printf("请输入目标用户名:\r\n");
	//tarName = cin.getline();
	cin.getline(tarName.c_str(), 50);

	ex.SendMsg_P2PData("hello", tarName.c_str());
	system("pause");
	return 0;
}

