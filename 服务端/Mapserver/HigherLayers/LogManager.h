// LogManager.h: interface for the CLogManager class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LOGMANAGER_H__564A7676_EFA9_408A_ABA6_9EBB37D1786E__INCLUDED_)
#define AFX_LOGMANAGER_H__564A7676_EFA9_408A_ABA6_9EBB37D1786E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

///////////////////////////////////////////////////////////////////////////////
// 로그 관리 위한 관리 클래스
class CLogManager : public TSingleton<CLogManager>
{
	typedef map<int, string>     HASH_LOG;
	typedef HASH_LOG::iterator   ITOR_LOG;
	typedef HASH_LOG::value_type PAIR_LOG;

public:
	CLogManager();
	~CLogManager();

public:
	void CreateMainPath(const char* pPath);
	void CreateSubPath(int nType, const char* pDir);

	void SaveStrikeAttack(CHARLIST* pCaster, CHARLIST* pTarget);
	void SaveThrowAttack(CHARLIST* pCaster, CHARLIST* pTarget);
	void SaveMagicExecute(BYTE nMagic, CHARLIST* pCaster, CHARLIST* pTarget, int nX, int nY);
	void SaveCombatExecute(BYTE nCombat, CHARLIST* pCaster, CHARLIST* pTarget, int nX, int nY);
	void SaveAccelType(CHARLIST* pTarget, const char* pType);
	void SaveDeadEventNpc(CHARLIST* pTarget);
	void SaveSealStoneInfo(CHARLIST* pSource, CHARLIST* pTarget, int nDamage); // CSD-040316
	void SaveLocalWarInfo(CHARLIST* pTarget); // CSD-040407

	const char* ConvertToDay(int nDay) const;
	const char* ConvertToTactics(int nKind) const;

public:
	void SaveAutionLog(int type,t_SellerItemDelete * pRecordInfo);	// BBD 040226 옥션로그
	void SaveEventItemLostLog(int type, char * szName, int nIndex);		// BBD 040308 이벤트 아이템 로스트 로그
	bool GetLogPath(string& rPath)
	{
		if (!m_strRoot.empty())
		{
			rPath = m_strRoot;
			return true;
		}

		return false;
	}

	bool GetLogPath(int nType, string& rPath)
	{
		if (m_mpBuffer.find(nType) != m_mpBuffer.end())
		{
			rPath = m_mpBuffer[nType];
			return true;
		}

		return false;
	}
private:
	const char* GetTargetName(CHARLIST* pTarget) const;

private:
	string   m_strRoot;
	HASH_LOG m_mpBuffer;
};
//
///////////////////////////////////////////////////////////////////////////////
#endif // !defined(AFX_LOGMANAGER_H__564A7676_EFA9_408A_ABA6_9EBB37D1786E__INCLUDED_)
