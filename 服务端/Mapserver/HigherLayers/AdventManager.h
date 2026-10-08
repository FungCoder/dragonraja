// AdventManager.h: interface for the CAdventManager class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ADVENTMANAGER_H__07F9F24A_2D41_4A38_ABA7_762A9BB5602D__INCLUDED_)
#define AFX_ADVENTMANAGER_H__07F9F24A_2D41_4A38_ABA7_762A9BB5602D__INCLUDED_
#include "EventGroupIndex.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define	MAX_GROUP_NO			10
// DB EM_GROUPINFO抛捞喉狼 EXECTYPE俊 扁废登绰 蔼
#define	EM_TYPE_NONE				0		// 胶农赋飘	
#define EM_TYPE_DAY					1		// EM_GROUPTYPE狼 WAYOFWEEK 客 DHOUR狼 矫埃捞 登菌阑锭
#define EM_TYPE_GROUP_KILLED		2		// EM_GROUPTYPE狼 KILLEDGROUP 锅龋狼 弊缝焊胶啊 磷菌阑锭

struct NPCGenerationPos  				// LTS AI2
{
	short int	CurNPC;
	char		GroupNo;
	char		isBoss;
	short int	NPCNo;
	short int	LocationX;
	short int	LocationY;
	short int	EventNo;
	short int	MaxNo;

	NPCGenerationPos()
	{
		CurNPC=0;
		GroupNo=0;
		isBoss=0;
		NPCNo=0;
		LocationX=0;
		LocationY=0;
		EventNo=0;
		MaxNo=0;
	}
};

struct EMGroupInfo
{
	int ExecType;
	int DayofWeek;
	int DHour;
	int KilledGroup;

	EMGroupInfo()
	{
		ExecType=0;
		DayofWeek=0;
		DHour=0;
		KilledGroup=0;
	}
};

///////////////////////////////////////////////////////////////////////////////
//
class CAdventManager : public TSingleton<CAdventManager>
{
public:
	CAdventManager();
	virtual ~CAdventManager();

public:
	int	LoadGenerationData();
	int	LoadGroupInfoData();
	void ClearGenerationData();
	void ClearGroupInfoData();
	int	GetDataCount();

	void SetActiveByGroupNo(int GroupNo);
	void SetActiveByIndex(int Index);

	void SetDeActiveByGroupNo(int GroupNo);
	void SetDeActiveByIndex(int Index);

	int GetDataCountByGroupNo(int GroupNo);
	int GetDataCountByIndex(int Index);

	int CheckActivedGroup();
	
	NPCGenerationPos* GetGenerationPosByGroupNo(int GroupNo,int Pos);
	NPCGenerationPos* GetGenerationPosByIndex(int Index,int Pos);
			
	void RemoveEventMonster(CHARLIST* ch);
	void CheckBossAndDeActiveGroupNo(CHARLIST* ch);
	void CheckDayAndActive(int Index);
	void CheckKilledGroup(int Index);

	void GetDayofWeek();
	void UpdateEventMonster();

public:
	int GetGroupCount() const
	{
		return m_iGroupCount;
	}

	int GetGroupIndex(int GroupNo) const
	{
		return FindEventGroupIndex(m_piGroupNo, m_iGroupCount, GroupNo);
	}


	int GetActiveStatusByGroupNo(int GroupNo) const
	{
		int tempIndex=GetGroupIndex(GroupNo);
		if (tempIndex>=0)
		{
			return m_piActived[tempIndex];
		}
		return -1;
	}

	int GetActiveStatusByIndex(int Index) const
	{
		if (!IsEventIndexValid(Index, m_iGroupCount))
		{
			return -1;
		}
		return m_piActived[Index];
	}

private:
	int					m_piActived[MAX_GROUP_NO];
	int					m_iDataCount;								// Total Table Row Count 
//	int					m_iActiveGroupNo;							// Actived Group Number
	int					m_piGroupNo[MAX_GROUP_NO];					// Save Group Number 
	int					m_piGroupDataCount[MAX_GROUP_NO];			// Group Data Number
	int					m_iGroupCount;								// Table Group Count

	EMGroupInfo			m_tGroupInfo[MAX_GROUP_NO];
	NPCGenerationPos**	m_pGPos;									// Table Data
	TimeData			m_tTimeData;
};
//
///////////////////////////////////////////////////////////////////////////////
#endif // !defined(AFX_ADVENTMANAGER_H__07F9F24A_2D41_4A38_ABA7_762A9BB5602D__INCLUDED_)
