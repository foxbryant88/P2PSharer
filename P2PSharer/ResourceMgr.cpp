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

//��ʼ��
bool CResourceMgr::Init(const char *addr)
{

	if (m_redisClient.Init(addr))
		g_resourceMgrlog.msg1("��ʼ��Redis�ɹ���");
	else
		g_resourceMgrlog.msg1("��ʼ��Redisʧ�ܣ�redis server:%s��", addr);

	m_macAddr = GetMacAddr();

	return true;
}

//����:1.�����б� 2.�����ļ��б����� 3.��Redis�Ǽ�/ע���ļ�
void *CResourceMgr::run()
{
	////�ȼ��ؾɵ��ļ��б�
	//LoadResourceList(m_mapResource);

	//�����ļ��б�
	UpdateResourceList();

	//�����µ��ļ��б���ص���ʱmap
	LoadResourceList(m_mapResourceTemp);

	////�ȶ��¾���Դ������Redis����
	UpdateResourceToRedis();

	return NULL;
}

//��ȡ�ļ���Ϣ
acl::string CResourceMgr::GetFileInfo(acl::string md5)
{
	return m_mapResource[md5];
}

//����Redis�ͻ��˶���
CRedisClient *CResourceMgr::GetRedisClient()
{
	return &m_redisClient;
}

//�����ļ��б�
void CResourceMgr::LoadResourceList(std::map<acl::string, acl::string > &mapResource)
{
	acl::ifstream infile;
	if (infile.open_read(NAME_FILE_INFO_LIST))
	{
		acl::string content;

		while (infile.gets(content))
		{
			//content��ʽ���ļ���|�ļ�·��|MD5|�ļ���С
			std::vector<acl::string> vField = content.split2(SPLITOR_OF_FILE_INFO);
			if (vField.size() > 3)
			{
				//key��md5, value:�ļ���|�ļ�·��|MD5|�ļ���С
				mapResource[vField[2]] = content;
			}
		}

		infile.close();
	}
}

//����/�����б�
bool CResourceMgr::UpdateResourceList()
{
	std::vector<acl::string> vDiskDrive;
	if (!GetDiskDrives(vDiskDrive))
	{
		g_resourceMgrlog.error1("��ȡ����������ʧ��!");
		return false;
	}

	//////����ɨ����������ļ�ɨ��
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


	//�ȴ�����ɨ�����
	for (int i = 0; i < vScanObject.size(); i++)
	{
		vScanObject[i]->wait(NULL);
	}

	return true;
}

//��Redis�������Ǽ�/ע����Դ��Ϣ
bool CResourceMgr::UpdateResourceToRedis()
{
	std::map<acl::string, acl::string >::iterator itMapResource = m_mapResource.begin();
	std::map<acl::string, acl::string >::iterator itTemp = m_mapResourceTemp.begin();

	for (; itTemp != m_mapResourceTemp.end(); ++itTemp)
	{
		//��ʱ�б��д��ڣ�����ʽ�б��в����ڵ�˵���������ļ�����Ҫ��ӵ�Redis
		if (m_mapResource.find(itTemp->first) == m_mapResource.end())
		{
			g_resourceMgrlog.msg1("���������ļ���%s,�ύRedis��", itTemp->second.c_str());

 			acl::string fileInfo = itTemp->second;
			std::vector<acl::string> vRes = fileInfo.split2(SPLITOR_OF_FILE_INFO);
						
			if (vRes.size() != 4)
			{
				g_resourceMgrlog.msg1("�����ļ���Ϣʧ��,�õ�%d�����ݣ��ļ���%s", vRes.size(), itTemp->second.c_str());
				continue;
			}

			acl::string field;
			field << vRes[0];
			field << SPLITOR_OF_FILE_INFO;
			field << vRes[2];

			if (!m_redisClient.AddResourceToHashList(field, vRes[3]))
				g_resourceMgrlog.msg1("��[%s]��ӵ�Redis HashListʧ�ܣ�", field.c_str());

			if (!m_redisClient.AddMACToResourceSet(vRes[2], m_macAddr))
				g_resourceMgrlog.msg1("��MAC��ַ��ӵ�����Ϊ[%s]��SETʧ�ܣ�", itTemp->first.c_str());
		}
	}

	g_resourceMgrlog.msg1("��Redis�������Ǽ�/ע����Դ��Ϣ��ϣ�");

	//��δ��ע���ļ����������ܵ��»����б����û��޷����ص���Ч�ļ���Ϣ
	//...

	//���������ļ��б�
	m_mapResource.swap(m_mapResourceTemp);
	return true;
}

//��ȡ��Ҫɨ��Ĵ���
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
				g_resourceMgrlog.msg1("���̣�%s, ����Ϊ���̶����̣�����ɨ�裡", drive.buf());
				break;
			case DRIVE_REMOVABLE:
				g_resourceMgrlog.msg1("���̣�%s, ����Ϊ���ƶ�����,����", drive.buf());
				break;
			case DRIVE_REMOTE:
				g_resourceMgrlog.msg1("���̣�%s, ����Ϊ��Զ�̴���,����", drive.buf());
				break;
			case DRIVE_CDROM:
				g_resourceMgrlog.msg1("���̣�%s, ����Ϊ������,����", drive.buf());
				break;
			case DRIVE_RAMDISK:
				g_resourceMgrlog.msg1("���̣�%s, ����Ϊ��RAMDISK,����", drive.buf());
				break;
			default:
				g_resourceMgrlog.msg1("���̣�%s, ����Ϊ��unknown,����", drive.buf());
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
