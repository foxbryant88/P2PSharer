/////////////////////////////////
//�������ݷ�����ǣ�ͨ��mapʵ��
///////////////////////////////////

#pragma once

class CFlagMgr
{
public:
	CFlagMgr();
	~CFlagMgr();

	//���ݸ�ʽ�ַ�������׺���ر��
	acl::string GetFlag(const char *formatstr, const char *suffix);

	//ѭ��������Ƿ�Ϊ1
	bool WaitFlag(const acl::string &flag);

	//���ñ��Ϊָ��ֵ
	void SetFlag(acl::string &flag, byte val);
	void SetFlag(const char *formatstr, const char *suffix, byte val);

	//�Ƴ����
	void RMFlag(acl::string &flag);
	void RMFlag(const char *formatstr, const char *suffix);

private:
	acl::locker m_lockFlag;                  //���ͱ����

	std::map<acl::string, byte> m_mapFlags;  //���ڱ������������Ƿ�ɹ��ĸ��ֱ��
										     //key:������ǵ����� value��1��ʾ��ȷ�� 0��ʾδȷ��

};

