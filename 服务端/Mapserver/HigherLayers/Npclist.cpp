#include "..\stdafx.h"
#include "..\LowerLayers\MyLog.h"
#include "AdventManager.h"
#include "GameplaySafety.h"
#include "LogManager.h"

CHARLIST NPCList[MAX_NPC_LIST];
int CurrentNPCNo = 0; // 泅犁 NPC狼 醚荐.
int Max_NPC_Generation = 0;

void InitNPCList()
{	
	for (int i = 0; i < MAX_NPC_LIST; ++i)
	{
		NPCList[i].bAlive = REMOVE_;
	}
}	
	
int GetAliveNPCList()
{	
	for (int i = NPC_LIST_START; i < MAX_NPC_LIST; ++i)
	{
		if (NPCList[i].bAlive == REMOVE_)
		{
			//MyLog(LOG_NORMAL, "<%d>GetAliveNPCList : %d", i, CurrentNPCNo); // CSD-LOG
			return i;
		}
	}

	return -1;
}
	
int AddNPCList(int npcindex, int x, int y)
{	//< CSD-031013
	return (GetAliveNPCList() < 0) ? -1:0;
}	//> CSD-031013

void RemoveNPCList(int Num)
{	//< CSD-031013
	if (Num < 0 || Num >= MAX_NPC_LIST)
	{
		MyLog(LOG_DEBUG, "Invalid NPC removal index: %d", Num);
		return;
	}
	if (NPCList[Num].bAlive == REMOVE_)
	{
		return;
	}
	// NPC惯积窍绰 困摹狼 泅犁 惯积等 蔼阑 窍唱 皑家 秦霖促. ( 弊贰具 肚 惯积窍瘤 .... )
	CHARLIST* pNpc = &NPCList[Num];

	switch (pNpc->generationtype)
	{								
	case GT_SKB_FILE:	
		{
			const int nIndex = pNpc->generationpos;

			if (nIndex < 0 || nIndex >= NPCgeneratePositionMax)
			{
				MyLog(LOG_DEBUG, "NPCgeneratePosition[%d] is invalided!", nIndex);
				break;
			}

			if (!ReleaseGenerationCount(NPCgeneratePosition[nIndex].curnpc))
			{
				MyLog(LOG_DEBUG, "NPCgeneratePosition[%d].curnpc was inconsistent during removal", nIndex);
			}

			break;
		}
	case GT_EVENT_MONSTER: // LTS AI2
		{
			g_pAdventManager->RemoveEventMonster(pNpc);
			break;
		}
	case GT_SCRIPT:
	case GT_SKILL_SUMMON:
	case GT_SCENARIO_MONSTER:
	case GT_EVENT_NPC:
	case GT_HUNT_MONSTER:
		{
			break;
		}
	}
	
	if (pNpc->eventno > 0)
	{
		g_pLogManager->SaveDeadEventNpc(pNpc);
	}	
	
	// Release the real tile before clearing the coordinates in the NPC slot.
	ClearRemovedNpcSlot(*pNpc, CurrentNPCNo, [](short& x, short& y) {
		SetTileOccupied(x, y, false);
	});
	//MyLog(LOG_NORMAL, "<%d>RemoveNPCList : %d", Num, CurrentNPCNo);	// CSD-LOG		
}	//> CSD-031013
	
//021030 YGI
// 酒捞袍 冻绢 哆府瘤 臼绰促.
// 某腐磐 传菊俊辑 癌磊扁 绝绢柳促.
void DeleteNpc(int index)
{
	if (index < 0 || index >= MAX_NPC_LIST) return;
	CHARLIST *pNpc = &NPCList[index];

	if (REMOVE_ == pNpc->bAlive) //捞固 府公宏 惑怕扼搁 捞巴阑 龋免 窍瘤 臼绰促.
	{
		return;
	}

	pNpc->bAlive = BUFE_;
	pNpc->deadcount = 0;
	SetArea(REMOVE_NPC_AREA, index);
	RemoveNPCList(index);
}
