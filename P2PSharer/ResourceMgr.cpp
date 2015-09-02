#include "stdafx.h"
#include "ResourceMgr.h"
#include "FileScan.h"

acl::log g_resourceMgrlog;
CResourceMgr::CResourceMgr()
{
	g_resourceMgrlog.open("resourcemgr.log");
}


CResourceMgr::~CResourceMgr()
{
}

//初始化
bool CResourceMgr::Init(const char *addr)
{

	if (m_redisClient.Init(addr))
		g_resourceMgrlog.msg1("初始化Redis成功！");
	else
		g_resourceMgrlog.msg1("初始化Redis失败，redis server:%s！", addr);

	m_macAddr = GetMacAddr();

	return true;
}

//功能:1.生成列表 2.更新文件列表到缓存 3.向Redis登记/注销文件
void *CResourceMgr::run()
{
	////先加载旧的文件列表
	//LoadResourceList(m_mapResource);

	//更新文件列表
	UpdateResourceList();

	//将最新的文件列表加载到临时map
	LoadResourceList(m_mapResourceTemp);

	////比对新旧资源，更新Redis缓存
	UpdateResourceToRedis();

	return NULL;
}

//获取文件信息
acl::string CResourceMgr::GetFileInfo(acl::string md5)
{
	return m_mapResource[md5];
}

//返回Redis客户端对象
CRedisClient *CResourceMgr::GetRedisClient()
{
	return &m_redisClient;
}

//加载文件列表
void CResourceMgr::LoadResourceList(std::map<acl::string, acl::string > &mapResource)
{
	acl::ifstream infile;
	if (infile.open_read(NAME_FILE_INFO_LIST))
	{
		acl::string content;

		while (infile.gets(content))
		{
			//content格式：文件名|文件路径|MD5|文件大小
			std::vector<acl::string> vField = content.split2(SPLITOR_OF_FILE_INFO);
			if (vField.size() > 3)
			{
				//key：md5, value:文件名|文件路径|MD5|文件大小
				mapResource[vField[2]] = content;
			}
		}

		infile.close();
	}
}

//生成/更新列表
bool CResourceMgr::UpdateResourceList()
{
	std::vector<acl::string> vDiskDrive;
	if (!GetDiskDrives(vDiskDrive))
	{
		g_resourceMgrlog.error1("获取磁盘驱动器失败!");
		return false;
	}

	//////创建扫描对象并启动文件扫描
	std::vector<CFileScan *> vScanObject;
	for (int i = 0; i < vDiskDrive.size(); i++)
	{
		CFileScan *obj = new CFileScan();
		obj->Init(vDiskDrive[i], &m_mapResource);
		obj->set_detachable(false);
		obj->start();

		vScanObject.push_back(obj);
	}
	//CFileScan *obj = new CFileScan();
	//obj->Init("F:\\KuGou", &m_mapResource);
	//obj->set_detachable(false);
	//obj->start();
	//vScanObject.push_back(obj);


	//等待所有扫描结束
	for (int i = 0; i < vScanObject.size(); i++)
	{
		vScanObject[i]->wait(NULL);
	}

	return true;
}

//向Redis服务器登记/注销资源信息
bool CResourceMgr::UpdateResourceToRedis()
{
	std::map<acl::string, acl::string >::iterator itMapResource = m_mapResource.begin();
	std::map<acl::string, acl::string >::iterator itTemp = m_mapResourceTemp.begin();

	for (; itTemp != m_mapResourceTemp.end(); ++itTemp)
	{
		//临时列表中存在，但正式列表中不存在的说明是新增文件，需要添加到Redis
		if (m_mapResource.find(itTemp->first) == m_mapResource.end())
		{
			g_resourceMgrlog.msg1("发现新增文件：%s,提交Redis！", itTemp->second.c_str());

 			acl::string fileInfo = itTemp->second;
			std::vector<acl::string> vRes = fileInfo.split2(SPLITOR_OF_FILE_INFO);
						
			if (vRes.size() != 4)
			{
				g_resourceMgrlog.msg1("解析文件信息失败,得到%d组数据！文件：%s", vRes.size(), itTemp->second.c_str());
				continue;
			}

			acl::string field;
			field << vRes[0];
			field << SPLITOR_OF_FILE_INFO;
			field << vRes[2];

			if (!m_redisClient.AddResourceToHashList(field, vRes[3]))
				g_resourceMgrlog.msg1("将[%s]添加到Redis HashList失败！", field.c_str());

			if (!m_redisClient.AddMACToResourceSet(vRes[2], m_macAddr))
				g_resourceMgrlog.msg1("将MAC地址添加到名称为[%s]的SET失败！", itTemp->first.c_str());
		}
	}

	g_resourceMgrlog.msg1("向Redis服务器登记/注销资源信息完毕！");

	//暂未做注销文件操作，可能导致缓存中保存用户无法下载的无效文件信息
	//...

	//更新最新文件列表
	m_mapResource.swap(m_mapResourceTemp);
	return true;
}

//获取需要扫描的磁盘
bool CResourceMgr::GetDiskDrives(std::vector<acl::string > &vRes)
{
	DWORD dwFlag = GetLogicalDrives();
	char disk = 'A';
	acl::string drive;

	while (dwFlag)
	{
		if (dwFlag & 1)
		{
			drive.format("%c:", disk);
			switch (GetDriveType(drive))
			{
			case DRIVE_FIXED:
				vRes.push_back(drive);
				g_resourceMgrlog.msg1("磁盘：%s, 类型为：固定磁盘，加入扫描！", drive.buf());
				break;
			case DRIVE_REMOVABLE:
				g_resourceMgrlog.msg1("磁盘：%s, 类型为：移动磁盘,忽略", drive.buf());
				break;
			case DRIVE_REMOTE:
				g_resourceMgrlog.msg1("磁盘：%s, 类型为：远程磁盘,忽略", drive.buf());
				break;
			case DRIVE_CDROM:
				g_resourceMgrlog.msg1("磁盘：%s, 类型为：光盘,忽略", drive.buf());
				break;
			case DRIVE_RAMDISK:
				g_resourceMgrlog.msg1("磁盘：%s, 类型为：RAMDISK,忽略", drive.buf());
				break;
			default:
				g_resourceMgrlog.msg1("磁盘：%s, 类型为：unknown,忽略", drive.buf());
				break;
			}
		}

		disk++;
		dwFlag >>= 1;
	}

	if (vRes.size() > 0)
		return true;
	else
		return false;
}
