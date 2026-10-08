// AdventManager.cpp: implementation of the CAdventManager class.
//
//////////////////////////////////////////////////////////////////////

#include "..\stdafx.h"
#include "AdventManager.h"
#include "EventMonsterReader.h"

#include "..\LowerLayers\Servertable.h"		
#include "Dr_Network.h"
#include "UserManager.h"

extern HENV					hEnv;		
extern HDBC					hDBC;

int LoadGeneration()
{
	if (!g_pAdventManager->LoadGenerationData())
	{
		g_pAdventManager->ClearGenerationData();
		return FALSE;
	}
	if (!g_pAdventManager->LoadGroupInfoData())
	{
		JustMsg("Event Monster Table was Loaded But EM_GroupInfo Not Loaded!!");
		g_pAdventManager->ClearGenerationData();
		return FALSE;
	}

	return TRUE;
}

void UpdateEventMonster()
{
	g_pAdventManager->UpdateEventMonster();		// LTS AI2
}

void CheckBossAndDeActiveGroupNo(CHARLIST* Npc)
{
	g_pAdventManager->CheckBossAndDeActiveGroupNo(Npc);
}

int CheckActivedGroup()
{
	return g_pAdventManager->CheckActivedGroup();
}

void ReloadEventMonsterData()
{
	if (LoadGeneration())
	{
		MyLog(0,"Event Monster Generation Data LoadComplete!!");
	}
	else
	{
		MyLog(0,"FAILURE!!!  Event Monster Generation Data Load!!");
	}
}

///////////////////////////////////////////////////////////////////////////////
// Construction/Destruction
///////////////////////////////////////////////////////////////////////////////

CAdventManager::CAdventManager()
{
	m_iDataCount=0;												// Table Row Count
	memset(&m_piActived,0,MAX_GROUP_NO*sizeof(int));			// Actived Check
	memset(&m_piGroupNo,0,MAX_GROUP_NO*sizeof(int));			// GroupNo Save
	memset(&m_piGroupDataCount,0,MAX_GROUP_NO*sizeof(int));		// GroupData Count Save
	m_pGPos=NULL;												// Table Data
	m_iGroupCount=0;
	ClearGroupInfoData();
}

CAdventManager::~CAdventManager()
{
	ClearGenerationData();
}

///////////////////////////////////////////////////////////////////////////////
// Public Method
///////////////////////////////////////////////////////////////////////////////

void CAdventManager::ClearGenerationData()			// 努锟斤拷锟斤拷 锟斤拷锟斤拷锟斤拷
{
	if (m_pGPos)
	{
		for (int i=0;i<m_iGroupCount;i++)
		{
			if (m_pGPos[i])
			{
					delete [] m_pGPos[i];
				m_pGPos[i]=NULL;
			}
		}
		delete [] m_pGPos;
		m_pGPos=NULL;
	}
	m_iGroupCount = 0;
	m_iDataCount = 0;
	memset(m_piGroupNo, 0, sizeof(m_piGroupNo));
	memset(m_piGroupDataCount, 0, sizeof(m_piGroupDataCount));
	memset(m_piActived, 0, sizeof(m_piActived));
	ClearGroupInfoData();
}

void CAdventManager::ClearGroupInfoData()
{
	for (int i=0;i<MAX_GROUP_NO;i++)
	{
		m_tGroupInfo[i].ExecType=0;
		m_tGroupInfo[i].DayofWeek=0;
		m_tGroupInfo[i].DHour=0;
		m_tGroupInfo[i].KilledGroup=0;
	}
}

int CAdventManager::GetDataCount()
{
	return m_iDataCount;
}

int CAdventManager::LoadGenerationData()
{
	HSTMT	hStmt=NULL;
	RETCODE	ret;
	char	query_stmt[MAX_PATH]={0,};
	SQLLEN	cbValue;
	int		RowCount;
	int		tGroupNo;
	int		tGroupDataCount;

	
	if (m_pGPos)								// 锟教癸拷 锟斤拷锟斤拷锟酵帮拷 锟街促革拷 锟斤拷锟斤拷锟斤拷锟截达拷..
	{
		ClearGenerationData();
	}

	SERVER_DATA *pData = g_pServerTable->GetOwnServerData();

	SQLAllocStmt(hDBC,&hStmt);

	wsprintf(query_stmt,"select DISTINCT GroupNo from Event_Monster where Map_Port=%d order by GroupNo",pData->wPort);

	ret=SQLExecDirect(hStmt,(UCHAR*)query_stmt,SQL_NTS);
 
	if (ret!=SQL_SUCCESS_WITH_INFO && ret !=SQL_SUCCESS)
	{
		MyLog(0,"Event_Monster Table Query Error!!");
		SQLFreeStmt(hStmt,SQL_DROP);
		return FALSE;
	}

	ret=SQLFetch(hStmt);
	if (ret == SQL_NO_DATA && m_pGPos == NULL)
	{
		m_iGroupCount = 0;
		m_iDataCount = 0;
		SQLFreeStmt(hStmt, SQL_DROP);
		MyLog(LOG_NORMAL, "No event monster groups for map port %d; SKB/script generation is independent", pData->wPort);
		return TRUE;
	}
	if (ret!=SQL_SUCCESS_WITH_INFO && ret !=SQL_SUCCESS)
	{
		MyLog(0,"Event_Monster Table Fetch Error!!");
		SQLFreeStmt(hStmt,SQL_DROP);
		return false;
	}

	m_iGroupCount=0;
	while (SQL_SUCCEEDED(ret))
	{
		if (m_iGroupCount >= MAX_GROUP_NO)
		{
			SQLFreeStmt(hStmt, SQL_DROP);
			MyLog(LOG_NORMAL, "Event monster group capacity exceeded");
			return FALSE;
		}
		ret=SQLGetData(hStmt,1,SQL_C_SLONG,&m_piGroupNo[m_iGroupCount],sizeof(int),&cbValue);		// 锟筋俺锟斤拷 锟阶凤拷锟斤拷 锟街达拷锟斤拷 犬锟斤拷
		if (!SQL_SUCCEEDED(ret) || cbValue == SQL_NULL_DATA)
		{
			MyLog(0,"Event_Monster Table SQL Return Error(%d)!!",ret);
			SQLFreeStmt(hStmt,SQL_DROP);
			return false;
		}                
		if (m_piGroupNo[m_iGroupCount] < 0 || m_piGroupNo[m_iGroupCount] >= MAX_GROUP_NO)
		{
			MyLog(0,"Event_Monster Group Count Error!!");
			SQLFreeStmt(hStmt,SQL_DROP);
			return false;
		}
		m_iGroupCount++;
		ret=SQLFetch(hStmt);
	}

	if (ret != SQL_NO_DATA)
	{
		SQLFreeStmt(hStmt, SQL_DROP);
		return FALSE;
	}
	m_pGPos=new NPCGenerationPos*[m_iGroupCount]();
	if (!m_pGPos)
	{
		MyLog(0,"Event_Monster Data Memory Alloc Error!!");
		SQLFreeStmt(hStmt,SQL_DROP);
		return false;
	}

	int i;
	for (i=0;i<m_iGroupCount;i++)	//锟绞憋拷拳 
	{
		m_pGPos[i]=NULL;
	}


	SQLFreeStmt(hStmt,SQL_DROP);

	m_iDataCount=0;

	for (i=0;i<m_iGroupCount;i++)
	{
		SQLAllocStmt(hDBC,&hStmt);
		tGroupNo=m_piGroupNo[i];
		wsprintf(query_stmt,"select count(Map_Port) as RowCount from Event_Monster where Map_Port=%d and GroupNo=%d",pData->wPort,tGroupNo);   
		ret=SQLExecDirect(hStmt,(UCHAR*)query_stmt,SQL_NTS);
 
		if (ret!=SQL_SUCCESS_WITH_INFO && ret !=SQL_SUCCESS)
		{
			MyLog(0,"Event_Monster Table Query Error!!");
			SQLFreeStmt(hStmt,SQL_DROP);
			return FALSE;
		}

		ret=SQLFetch(hStmt);
		if (ret!=SQL_SUCCESS_WITH_INFO && ret !=SQL_SUCCESS)
		{
			MyLog(0,"Event_Monster Table Fetch Error!!");
			SQLFreeStmt(hStmt,SQL_DROP);
			return false;
		}
		ret=SQLGetData(hStmt,1,SQL_C_SLONG,&RowCount,sizeof(int),&cbValue);		// 锟阶缝俊 锟筋俺锟斤拷 锟斤拷锟斤拷锟酵帮拷 锟街达拷锟斤拷 犬锟斤拷
		if (ret!=SQL_SUCCESS_WITH_INFO && ret!=SQL_SUCCESS)
		{
			MyLog(0,"Event_Monster Table SQL Return Error(%d)!!",ret);
			SQLFreeStmt(hStmt,SQL_DROP);
			return false;
		}                

		if (RowCount <= 0)
		{
			SQLFreeStmt(hStmt, SQL_DROP);
			return FALSE;
		}
		m_pGPos[i]=new NPCGenerationPos[RowCount];
		if (!m_pGPos[i])
		{
			MyLog(0,"Event_Monster Data Memory Alloc Error!!");
			SQLFreeStmt(hStmt,SQL_DROP);
			return false;
		}

		SQLFreeStmt(hStmt,SQL_DROP);
		SQLAllocStmt(hDBC,&hStmt);

		wsprintf(query_stmt,"select * from Event_Monster where Map_Port=%d and GroupNo=%d order by `Index`",pData->wPort,tGroupNo);   
		ret=SQLExecDirect(hStmt,(UCHAR*)query_stmt,SQL_NTS);
 
		if (ret!=SQL_SUCCESS_WITH_INFO && ret !=SQL_SUCCESS)
		{
			MyLog(0,"Event_Monster Table Query Error!!");
			SQLFreeStmt(hStmt,SQL_DROP);
			return FALSE;
		}

		ret=SQLFetch(hStmt);
		if (ret!=SQL_SUCCESS_WITH_INFO && ret !=SQL_SUCCESS)
		{
			MyLog(0,"Event_Monster Table Fetch Error!!");
			SQLFreeStmt(hStmt,SQL_DROP);
			return false;
		}

		m_piGroupDataCount[i]=0;
		while (SQL_SUCCEEDED(ret))
		{
			tGroupDataCount=m_piGroupDataCount[i];
			if (!IsEventIndexValid(tGroupDataCount, RowCount))
			{
				SQLFreeStmt(hStmt, SQL_DROP);
				return FALSE;
			}
			NPCGenerationPos& position = m_pGPos[i][tGroupDataCount];
			const bool rowValid = ReadEventMonsterRow(hStmt, position);
			if (!rowValid || position.GroupNo != tGroupNo || position.NPCNo < 0 ||
				position.NPCNo >= Num_Of_NPC_Generation || position.MaxNo < 0 ||
				position.LocationX < 0 || position.LocationX >= g_Map.file.wWidth ||
				position.LocationY < 0 || position.LocationY >= g_Map.file.wHeight)
			{
				MyLog(0,"Event_Monster Table SQL Return Error(%d)!!",ret);
				SQLFreeStmt(hStmt,SQL_DROP);
				return FALSE;
			}              
			m_piGroupDataCount[i]++;
			m_iDataCount++;
			ret=SQLFetch(hStmt);
		}
		if (ret != SQL_NO_DATA || m_piGroupDataCount[i] != RowCount)
		{
			SQLFreeStmt(hStmt, SQL_DROP);
			return FALSE;
		}
		SQLFreeStmt(hStmt,SQL_DROP);
	}
	return TRUE;
}

int CAdventManager::LoadGroupInfoData()
{
	HSTMT	hStmt=NULL;
	RETCODE	ret;
	char	query_stmt[MAX_PATH]={0,};
	SQLLEN	cbValue;
	int		tGroupNo;
	SERVER_DATA *pData = g_pServerTable->GetOwnServerData();

	ClearGroupInfoData();	// Initialize..

	SQLAllocStmt(hDBC,&hStmt);

	wsprintf(query_stmt,"select * from EM_GroupInfo where MapPort=%d order by GroupNo",pData->wPort);

	ret=SQLExecDirect(hStmt,(UCHAR*)query_stmt,SQL_NTS);
 
	if (ret!=SQL_SUCCESS_WITH_INFO && ret !=SQL_SUCCESS)
	{
		MyLog(0,"EM_GroupInfo Table Query Error!!");
		SQLFreeStmt(hStmt,SQL_DROP);
		return FALSE;
	}

	ret=SQLFetch(hStmt);
	if (ret == SQL_NO_DATA)
	{
		SQLFreeStmt(hStmt, SQL_DROP);
		return TRUE;
	}
	if (ret!=SQL_SUCCESS_WITH_INFO && ret !=SQL_SUCCESS)
	{
		MyLog(0,"EM_GroupInfo Table Fetch Error!!");
		SQLFreeStmt(hStmt,SQL_DROP);
		return false;
	}

	while (SQL_SUCCEEDED(ret))
	{
		ret=SQLGetData(hStmt,3,SQL_C_SLONG,&tGroupNo,sizeof(int),&cbValue);		// 锟阶凤拷锟饺?
		if (!SQL_SUCCEEDED(ret) || cbValue == SQL_NULL_DATA || tGroupNo<0||tGroupNo>=MAX_GROUP_NO)
		{
			JustMsg("EM_GroupInfo Table GroupNo Fail MapPort : %d, GroupNo : %d!!",pData->wPort,tGroupNo);
			SQLFreeStmt(hStmt,SQL_DROP);
			return false;
		}
		const int slot = GetGroupIndex(tGroupNo);
		if (slot < 0)
		{
			MyLog(LOG_NORMAL, "Event group %d has no spawn positions; metadata skipped", tGroupNo);
			ret = SQLFetch(hStmt);
			continue;
		}
		EMGroupInfo& info = m_tGroupInfo[slot];
		const EventColumnBinding fields[] = {
			{SQL_C_SLONG, &info.ExecType, sizeof(info.ExecType)},
			{SQL_C_SLONG, &info.DayofWeek, sizeof(info.DayofWeek)},
			{SQL_C_SLONG, &info.DHour, sizeof(info.DHour)},
			{SQL_C_SLONG, &info.KilledGroup, sizeof(info.KilledGroup)}
		};
		if (!ReadEventColumns(hStmt, 4, fields))
		{
			MyLog(LOG_NORMAL, "Invalid event group metadata for group %d", tGroupNo);
			SQLFreeStmt(hStmt,SQL_DROP);
			return false;
		}                
		ret=SQLFetch(hStmt);
	}
	if (ret != SQL_NO_DATA)
	{
		SQLFreeStmt(hStmt, SQL_DROP);
		return FALSE;
	}
	SQLFreeStmt(hStmt,SQL_DROP);
	return TRUE;
}

void CAdventManager::SetActiveByGroupNo(int GroupNo)
{
	t_packet packet;
	int tempIndex=GetGroupIndex(GroupNo);
	if (tempIndex>=0)
	{
		m_piActived[tempIndex]=1;					// Group Number Active
		MyLog(0,"Event Monster Group No : %d Was Actived",tempIndex);

		
		packet.h.header.type=CMD_EVENT_MONSTER_CREATED;
		packet.h.header.size=0;
		g_pUserManager->SendPacket(&packet); // CSD-CN-031213
	}
}

void CAdventManager::SetActiveByIndex(int Index)
{
	t_packet packet;
	
	if (!IsEventIndexValid(Index, m_iGroupCount)) return;
	if (m_piGroupDataCount[Index]<=0) return; // No Data
	
	m_piActived[Index]=1;					// Group Number Active

	MyLog(0,"Event Monster Group No : %d Was Actived",Index);
	
	packet.h.header.type=CMD_EVENT_MONSTER_CREATED;
	packet.h.header.size=0;
	g_pUserManager->SendPacket(&packet); // CSD-CN-031213
}

void CAdventManager::GetDayofWeek()
{
	struct tm* NewTime;
	time_t LongTime;
	time(&LongTime);
	NewTime=localtime(&LongTime);
	m_tTimeData.Year=NewTime->tm_year+1900;
	m_tTimeData.Month=NewTime->tm_mon+1;
	m_tTimeData.Day=NewTime->tm_mday;
	m_tTimeData.DayofWeek=NewTime->tm_wday;
}

void CAdventManager::UpdateEventMonster()		// 券锟斤拷 锟斤拷锟斤拷锟斤拷 锟街促革拷..
{
	static int oldHour=0;
	if (g_hour==oldHour) 
	{
		return;
	}

	GetDayofWeek();
	oldHour=g_hour;

	for (int i=0;i<MAX_GROUP_NO;i++)			// 锟阶凤拷 锟竭伙拷锟教猴拷飘 
	{
		switch(m_tGroupInfo[i].ExecType)		// 锟斤拷锟解俊 锟斤拷锟?锟斤拷锟斤拷锟斤拷 锟斤拷
		{
		case 1 : CheckDayAndActive(i);
		//case 2 : CheckKilledGroup(i);
		}
	}
}

void CAdventManager::SetDeActiveByGroupNo(int GroupNo)		// DeActive By GroupNo
{
	int tempIndex=GetGroupIndex(GroupNo);
	if (tempIndex>=0)
	{
		CheckKilledGroup(tempIndex);				// 锟教弊凤拷锟斤拷 锟斤拷锟斤拷锟角搁辑 锟斤拷锟斤拷锟斤拷 锟竭伙拷锟角匡拷锟斤拷 锟较绰帮拷 锟街促革拷 眉农
		m_piActived[tempIndex]=0;
		MyLog(0,"Event Monster Group No : %d Was DeActived",tempIndex);
	}
}

void CAdventManager::SetDeActiveByIndex(int Index)
{
	if (!IsEventIndexValid(Index, m_iGroupCount)) return;

	CheckKilledGroup(Index);				// 锟教弊凤拷锟斤拷 锟斤拷锟斤拷锟角搁辑 锟斤拷锟斤拷锟斤拷 锟竭伙拷锟角匡拷锟斤拷 锟较绰帮拷 锟街促革拷 眉农
	m_piActived[Index]=0;
	MyLog(0,"Event Monster Group No : %d Was DeActived",Index);
}

int CAdventManager::GetDataCountByGroupNo(int GroupNo)
{
	int tempIndex=GetGroupIndex(GroupNo);
	if (tempIndex>=0)
	{
		return m_piGroupDataCount[tempIndex];
	}
	return -1;
}

int CAdventManager::GetDataCountByIndex(int Index)
{
	if (!IsEventIndexValid(Index, m_iGroupCount))
	{
		return -1;
	}
	return m_piGroupDataCount[Index];
}

NPCGenerationPos* CAdventManager::GetGenerationPosByGroupNo(int GroupNo,int Pos)
{
	return GetGenerationPosByIndex(GetGroupIndex(GroupNo), Pos);
}

NPCGenerationPos* CAdventManager::GetGenerationPosByIndex(int Index,int Pos)
{
	if (!m_pGPos || !IsEventIndexValid(Index, m_iGroupCount) || !m_pGPos[Index])
	{
		return NULL;
	}
	if (!IsEventIndexValid(Pos, m_piGroupDataCount[Index]))
	{
		return NULL;
	}
	return &m_pGPos[Index][Pos];			
}

int CAdventManager::CheckActivedGroup()
{
	int tempStatus;
	for (int i=0;i<MAX_GROUP_NO;i++)
	{
		tempStatus=GetActiveStatusByIndex(i);
		if (tempStatus>0)
		{
			return TRUE;
		}
	}
	return FALSE;
}

void	CAdventManager::RemoveEventMonster(CHARLIST* Npc)
{
	NPCGenerationPos* Pos;
	
	MyLog(0,"Delete NPC SPECIAL GroupNo : %d ,Type:%d",Npc->JoinLocalWar,Npc->generationpos);
	
	Pos=GetGenerationPosByIndex(Npc->JoinLocalWar,Npc->generationpos);
	if (!Pos) 
	{
		return;
	}
	Pos->CurNPC--;
//	CheckBossAndDeActiveGroupNo(Npc);
}

void CAdventManager::CheckBossAndDeActiveGroupNo(CHARLIST* Npc)
{
	if (!Npc->IsNpc()) return;		// NPC锟斤拷 锟狡聪革拷 锟斤拷锟斤拷锟窖达拷.
	if (!Npc->GainedFame) return;	//锟斤拷锟斤拷锟斤拷 锟狡聪革拷 锟斤拷锟斤拷锟窖达拷.
	
	MyLog(0,"Delete BOSS GroupNo : %d ,Type:%d",Npc->JoinLocalWar,Npc->generationpos);
	SetDeActiveByIndex(Npc->JoinLocalWar);	// 锟阶凤拷锟饺ｏ拷锟?锟斤拷锟斤拷锟窖达拷.
}

void CAdventManager::CheckDayAndActive(int Index)
{
	if (!IsEventIndexValid(Index, m_iGroupCount)) return;
	if (m_tTimeData.DayofWeek!=m_tGroupInfo[Index].DayofWeek)
	{
		return;
	}

	if (g_hour!=m_tGroupInfo[Index].DHour)
	{
		return;
	}

	MyLog(0,"%d Group Event Monster was Created By DayofWeek And Time",Index);

	SetActiveByIndex(Index);
}

void CAdventManager::CheckKilledGroup(int Index)	// 锟阶凤拷锟斤拷 锟斤拷锟斤拷锟?锟教猴拷飘 
{
	for (int i=0;i<MAX_GROUP_NO;i++)
	{
		if (m_tGroupInfo[i].ExecType==2)					// 锟斤拷锟解俊 锟斤拷锟?锟斤拷锟斤拷锟斤拷 锟斤拷 (switch)
		{
			if (IsEventIndexValid(Index, m_iGroupCount) &&
				m_tGroupInfo[i].KilledGroup == m_piGroupNo[Index])
			{
				MyLog(0,"%d Group Event Monster Was Created By %d Group Event Monster Boss Killed..",i,Index);
				SetActiveByIndex(i);
			}
		}
	}
}
