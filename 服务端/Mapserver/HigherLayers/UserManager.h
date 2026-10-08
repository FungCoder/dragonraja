// UserManager.h: interface for the CUserManager class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_USERMANAGER_H__7F7661A1_937C_4887_8BD6_FB3F1A3391B4__INCLUDED_)
#define AFX_USERMANAGER_H__7F7661A1_937C_4887_8BD6_FB3F1A3391B4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

///////////////////////////////////////////////////////////////////////////////
// 立加磊 包府 努贰胶
class CUserManager : public TSingleton<CUserManager>
{
public:
	typedef map<string, WORD> HASH_USER;
	typedef HASH_USER::iterator ITOR_USER;
	typedef HASH_USER::value_type PAIR_USER;
	typedef queue<MAP_MOVE_INFO> QUEUE_MAP;

public:
	CUserManager();
	virtual ~CUserManager();

public:
	void AddMember(WORD idUser);
	void DelMember(WORD idUser);

	void Push(MAP_MOVE_INFO infMapMove);
	void Execute();

	void SendPacket(t_packet* pPacket);

	CHARLIST* GetCharList(const char* pName);
	int GetServerID(const char* pName);

public:
	HASH_USER& GetUserSet()
	{
		return m_hash;
	}

	int GetSize() const 
	{ 
		return m_hash.size(); 
	}

	bool IsExist(const char* pName) const
	{
		return (m_hash.find(pName) != m_hash.end()) ? true:false;
	}

private:
	HASH_USER m_hash;
	QUEUE_MAP m_queue;
};
//
///////////////////////////////////////////////////////////////////////////////
#endif // !defined(AFX_USERMANAGER_H__7F7661A1_937C_4887_8BD6_FB3F1A3391B4__INCLUDED_)