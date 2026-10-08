// SkillMgr.cpp: implementation of the CSkillMgr class.
//
//////////////////////////////////////////////////////////////////////

#include "../stdafx.h"
#include "CItem.h"
#include "SkillMgr.h"
#include "../../Library/Shared/OperationsRates.h"

extern int	PRAY_RECOVER_DIVINE_TIME; // 030520 kyo
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CSkillMgr  SkillMgr;

LEARNITEMCONVERT LIC[MAX_LEARN_ITEM_CONVERT];
t_RareItemBag RareItemBag[MAX_RAREITEMBAG];
t_BasicItemBag BasicItemBag[MAX_BASICITEMBAG];
t_ItemControlPercent ItemControlPercent[MAX_ITEMCONTROLPERCENT];
t_ItemFallPercent ItemFallPercent[MAX_ITEMFALLPERCENT];
t_ItemControl ItemControl[MAX_ITEMCONTROL];


CSkillMgr::CSkillMgr()
{

}

CSkillMgr::~CSkillMgr()
{

}

void CSkillMgr::AutoSetCanView(CHARLIST *ch)
{
	if(ch->bCanViewGhost)
	{
		if(ch->dwCanViewGhostTime <= global_time)
		{//�� �� ���� �Ǿ����� Ŭ���̾�Ʈ���� �˷� ��� �Ѵ�
			if(RareEM.GetStaticRareGhost(ch->StaticRare))//���� ������ ������� ����
			{
				SetCanViewGhost(ch,STATIC_RARE_VIEW_FORCE_TIME);//���� ���� bCanViewGhost�� true��
			}
			else
			{
				SetCanViewGhost(ch,0);//���� ���� bCanViewGhost�� false��
			}
		}
	}

	if(ch->bCanViewBomb)
	{
		if(ch->dwCanViewBombTime <= global_time)
		{//�� �� ���� �Ǿ����� Ŭ���̾�Ʈ���� �˷� ��� �Ѵ�
			if(RareEM.GetStaticRareLandmine(ch->StaticRare))
			{
				SkillMgr.SetCanViewBomb(ch,STATIC_RARE_VIEW_FORCE_TIME);//020510 lsw
			}
			else
			{
				SkillMgr.SetCanViewBomb(ch,0);//020510 lsw
			}
		}
	}

	if(ch->bCanViewTrap)
	{
		if(ch->dwCanViewTrapTime <= global_time)
		{//�� �� ���� �Ǿ����� Ŭ���̾�Ʈ���� �˷� ��� �Ѵ�
			if(RareEM.GetStaticRareTraping(ch->StaticRare))
			{
				ch->bCanViewTrap = true;
				ch->SendCharInfoBasic( VIEWTRAP, STATIC_RARE_VIEW_FORCE_TIME);//020704 lsw
			}
			else
			{
				ch->bCanViewTrap = false;
				ch->SendCharInfoBasic(  VIEWTRAP, 0);//020704 lsw
			}
		}
	}

	return;
}

bool	CSkillMgr::IsRecoverAble(const int iNow,const int iMax)const//���� ���� �ִ뷮�� ���ؼ� �ִ뷮�� ���纸�� ������ �����ϴٸ� �����Ѵ�
{
	if(iNow <  iMax)
	{
		return true;
	}
	return false;
}

// HP �ڵ�ȸ��
void CSkillMgr::AutoRecoverHp(CHARLIST *ch)
{
	if(	ch->bAlive != ALIVE_ )
	{
		ch->Hp = 0;
		return;
	}

	const int cn	= ch->GetServerID();
	if( global_time  >=  ch->dwHpRecoverTime )//�ð� üũ
	{
		int iCombatRecoverHp = 0;
		int iCombatRecoverHpRate = 0;
		int iCombatRecoverHpMax = 0;
		int iCombatRecoverTime = 0;

		const int nPassive = ch->GetPassiveCombat();

		switch (nPassive)
		{
      case SIMUNIAN_BOSOM:
      { 
        iCombatRecoverHp = ch->GetCombatValue(nPassive);
        iCombatRecoverHpMax	= 50;
        iCombatRecoverTime = ch->GetCombatLevel(nPassive)*5000;
        break;
      }
    }
    //DEFAULT_RECOVER_HP_TIME	�̰��� �ۼ������� ���� �ϸ� �� ���� ȸ�� �ȴ�
    ch->dwHpRecoverTime = global_time + DEFAULT_RECOVER_HP_TIME - iCombatRecoverTime;

		const int	iRecoverHp	= ch->HpMax*(HP_RECOVER_RATE+RareEM.GetStaticRareFatal(ch->StaticRare)+iCombatRecoverHpRate)/100 + iCombatRecoverHp;//�ѹ��� 5% ȸ�� �����ش�//�̹��� ȸ�� �� ��
		const int	iRecoverHpMaxPercent = (DEFAULT_RECOVER_HP_MAX_PERCENT+ iCombatRecoverHpMax/*+RareEM.GetStaticRareLife_recovery(ch->StaticRare)*/);//�ִ� ȸ�� �ۼ�������
		const int	iRecoverHpMax = ch->HpMax * iRecoverHpMaxPercent/100;
		if(IsRecoverAble(ch->Hp,iRecoverHpMax ))
		{
			if(ch->IncLife(iRecoverHp,iRecoverHpMaxPercent))//��ȭ���� �ִٸ�
			{
				ch->SendCharInfoBasic( HP, ch->Hp );//020704 lsw
			}
		}
	}
	return;
}

void CSkillMgr::AutoRecoverMp(CHARLIST *ch)
{//021026 lsw
	if(	ch->bAlive != ALIVE_ )
	{
		ch->Mana = 0;
		return;
	}
	const int cn	= ch->GetServerID();
	//MPȸ��
	if( ch->Spell == WIZARD_SPELL )	// Wizard...
	{
		if( global_time >= ch->dwManaRecoverTime )//�ð� üũ
		{	//DEFAULT_RECOVER_MANA_TIME	�̰��� �ۼ������� ���� �ϸ� �� ���� ȸ�� �ȴ�
			const char cForRookie  = MapInfo[MapNumber].forrookie;//5�� �� ���� ȸ�� �ȴ�
			ch->dwManaRecoverTime = global_time +(( 2 == cForRookie || 3 == cForRookie)?DEFAULT_RECOVER_MANA_TIME/5:DEFAULT_RECOVER_MANA_TIME);

			int iCombatRecoverMp = 0;
			int iCombatRecoverMpRate = 0;
			int iCombatRecoverMpMax	= 0;
			
			const int	iRecoverMana	= ch->ManaMax*(MANA_RECOVER_RATE+RareEM.GetStaticRareMagic(ch->StaticRare)+iCombatRecoverMpRate)/100 + iCombatRecoverMp;//�ѹ��� 5% ȸ�� �����ش�
			const int	iRecoverManaMaxPercent = (DEFAULT_RECOVER_MANA_MAX_PERCENT+RareEM.GetStaticRareMana_recovery(ch->StaticRare)+iCombatRecoverMpMax);
			const int	iRecoverManaMax = ch->ManaMax * iRecoverManaMaxPercent/100;
			if(IsRecoverAble(ch->Mana,iRecoverManaMax))
			{
				if(ch->IncMana(iRecoverMana,iRecoverManaMaxPercent))//��ȭ���� �ִٸ�
				{
					ch->SendCharInfoBasic( MP, ch->Mana );//020704 lsw
				}
			}
		}
	}
	else // Priest..
	{	//DEFAULT_RECOVER_DIVINE_TIME	�̰��� �ۼ������� ���� �ϸ� �� ���� ȸ�� �ȴ�
if( global_time >= ch->dwDivineRecoverTime )//�ð� üũ
		{	//< CSD-TW-030606
			int nPlusTime = 0;

			const char cForRookie = MapInfo[MapNumber].forrookie;

			switch (cForRookie)
			{
			case 2:
			case 3:
				{
					nPlusTime = DEFAULT_RECOVER_DIVINE_TIME/5;
					break;
				}
			default:
				{
					nPlusTime = DEFAULT_RECOVER_DIVINE_TIME;
					break;
				}
			}

			if (ch->IsDivineUp())
			{
				nPlusTime -= Magic_Ref[PRAY].nHDPoint;
			}
			
			ch->dwDivineRecoverTime = global_time + nPlusTime;
			
			int iCombatRecoverDivineRate = 0;
			int iCombatRecoverDivineMax = 0;
			
			const int iRecoverDivine	= ch->ManaMax*(DIVINE_RECOVER_RATE+RareEM.GetStaticRareNature(ch->StaticRare)+iCombatRecoverDivineRate)/100;//�ѹ��� 5% ȸ�� �����ش�
			const int iRecoverDivineMaxPercent = (DEFAULT_RECOVER_DIVINE_MAX_PERCENT + iCombatRecoverDivineMax);
			const int iRecoverDivineMax = ch->ManaMax*iRecoverDivineMaxPercent/100;
			if(IsRecoverAble(ch->Mana,iRecoverDivineMax))
			{
				if(ch->IncMana(iRecoverDivine,iRecoverDivineMaxPercent))//��ȭ���� �ִٸ�
				{
					ch->SendCharInfoBasic( MP, ch->Mana );//020704 lsw
				}
			}
		}	//> CSD-TW-030606
	}
}

//����� �������� �׳� ���� �ʴ´� Ư���� ��Ȳ������ ����(ex>����)
void CSkillMgr::AutoChangeHungry( CHARLIST *ch )
{	
	if( ch->bAlive != ALIVE_ )
	{	
		ch->Hungry =0;
		ch->fChangeHungry = 0;
		return;//��� ���� ������ ����� ������ ��ȭ ����
	}
	
	const int cn = ch->GetServerID();
	bool bSend = false;

	if( global_time > ch->dwHungryRecoverTime  )
	{
		const int	iChangeHungry		= abs((int)ch->fChangeHungry);
		if(!iChangeHungry)	{return;}
		ch->dwHungryRecoverTime = global_time+DEFAULT_CHANGE_HUNGRY_TIME;//üũ�ϴ� term

		if( ch->DecHungry((int)iChangeHungry))
		{
			ch->fChangeHungry -= (float)iChangeHungry;//�켱 ��뷮�� ��� ��� �ϰ� ���ڿ� �ɾ������� ȸ���� �����ش�
			bSend =true;
		}
	
		if(ch->nCurrentAction == 17 )//���� ����
		{
			const int	iRecoverHungry				= ch->HungryMax * HUNGRY_CHANGE_RATE/100;//�ѹ��� 5% ȸ�� �����ش�
			const int	iRecoverHungryMaxPercent	= DEFAULT_CHANGE_HUNGRY_MAX_PERCENT;
			const int	iRecoverHungryMax = ch->HungryMax * iRecoverHungryMaxPercent/100;
			if(IsRecoverAble(ch->Hungry,iRecoverHungryMax))
			{
				if(ch->IncHungry(iRecoverHungry,iRecoverHungryMaxPercent ))	
				{
					bSend	 = true;
				}
			}
		}
		if(bSend)
		{
			ch->SendCharInfoBasic( SP, ch->Hungry );//020704 lsw
		}
	}
	return;
}

// CP �ڵ�ȸ��
void CSkillMgr::AutoRecoverCp(CHARLIST *ch)
{
	if(	ch->bAlive != ALIVE_ )
	{
		ch->Cp = 0;
		return;
	}

	const int cn	= ch->GetServerID();

	if (global_time >= ch->dwCpRecoverTime && ch->Peacests) // �ð� üũ
	{	//< CSD-TW-030624
		// DEFAULT_RECOVER_CP_TIME	�̰��� �ۼ������� ���� �ϸ� �� ���� ȸ��
		ch->dwCpRecoverTime = global_time + DEFAULT_RECOVER_CP_TIME;
				
		int	iRecoverCp = __max(ch->CpMax*CP_CHANGE_RATE/100, 1);

		if (ch->IsRecoveryCombatState())
		{	// ������ų ������ ���� ȸ���ӵ��� �ø���.
			const int nRecoveryCombat = ch->GetRecoveryCombat();
			const int nMinus = ch->HpMax*ch->GetCombatValue(nRecoveryCombat)/100;

			if (ch->Cp < ch->CpMax && nMinus < ch->Hp)
			{
				const int nLife = ch->DecLife(nMinus, 1);

				if (nLife != 0)
				{
					::SendCharacterCondition(ch, -nLife);
					iRecoverCp = __max(ch->CpMax*ch->GetCombatValue(nRecoveryCombat)/100, 1);
				}	
			}
		}
		
		const int iRecoverCpMaxPercent = DEFAULT_CHANGE_CP_MAX_PERCENT;
		const int iRecoverCpMax = ch->CpMax;

		if(IsRecoverAble(ch->Cp,iRecoverCpMax ))
		{
			if(ch->IncCombat(iRecoverCp,iRecoverCpMaxPercent))//��ȭ���� �ִٸ�
			{
				ch->SendCharInfoBasic(CP, ch->Cp);
			}
		}
	}	//> CSD-TW-030624
}

void CSkillMgr::AutoRefreshRareMakeCount(CHARLIST *ch)//���� �������� ���� ���� ���� �ϴ� �Լ�  
{//021108 lsw
	if( ch->bAlive != ALIVE_ ) {return;}

	const int day		=	g_day%8;
	const int month		=	g_mon%8;//0-11 ����
	bool bSendFlag		= false;

	for ( int i =0; i < 7 ; i++ )
	{//��������� ���� 
		t_skillexp2 *skillexp2 = ((t_skillexp2*)&ch->skillexp[i]); 
		if( skillexp2->day != day)
		{
			skillexp2->day = day;
			bSendFlag = true;
		}
		if( skillexp2->month != month)
		{
			skillexp2->month = month;
			bSendFlag = true;
		}
		if(bSendFlag)
		{
			bSendFlag = false;
			skillexp2->makecount = 0;
			SendSkillExp(i, ch->skillexp[i], ch->GetServerID());
		}
	}	
	//�⼺ ����� ���� //��� ��¥�� ���÷� ���� �����̴�
	t_SkillExp3 *pSkillexp3 = (t_SkillExp3*)&ch->skillexp[SKILL_MASTER];
	if(pSkillexp3->skillType)
	{
		if(g_year > pSkillexp3->year)
		{//�ذ� ������ ������ 
			bSendFlag = true;
		}
		if(g_mon*30 + g_day > pSkillexp3->day)
		{//���� ������ ������ 
			bSendFlag = true;			
		}
		if(bSendFlag)
		{
			bSendFlag = false;
			memset(pSkillexp3,0,sizeof(DWORD));//��������
			SendSkillExp(SKILL_MASTER, ch->skillexp[SKILL_MASTER], ch->GetServerID());
		}
	}
	return;
}

bool CSkillMgr::CheckSkillNo( const int iSkillNo )const 
{	
//MAX_SKILL_NO		MAX_SKILL_NO �� �� ���� MAX_SKILLMAIN���� ������ �ȵȴ�
//	if(	MAX_SKILL_NO <= skillno ||	0 >= skillno)//0�� ��ų�� ����
	if(	MAX_SKILLMAIN >= iSkillNo && 0 < iSkillNo)//0�� ��ų�� ����
	{
		return true;	
	}
	return false;
}

bool CSkillMgr::CheckIsYourSkill( LPCHARLIST ch, const int iSkillNo )const
{
	if(!CheckSkillNo(iSkillNo))	{	return false;	}
	if( ch->Skill[iSkillNo] )	{	return true;	}
	return false;
}

int CSkillMgr::CorrectLearnItem(CHARLIST *ch)
{
	int i = 0;
	int j = 0;
	for( ;i < MAX_LEARN_ITEM; i++)
	{
		for(;j < MAX_LEARN_ITEM_CONVERT; j++)
		{
			if(ch->Item[i] == LIC[j].iBeforeItemNo)
			{
				ch->Item[i] = LIC[j].iAfterItemNo;
	//	memcpy(p->u.change_map_protocol.mapdata6.ch_data.Item,(UCHAR *)ch->Item,SIZE_OF_ITEMINDEX );
				break;
			}
		}
	}
	return 0;
}

int CSkillMgr::FindLearnItem( CHARLIST *ch, const int item_no )		// ���� ������ -1�� ����, ������ item_max�� ����
{
	// 020620 YGI
	if( !ch ) return -1;

	for( int i=0; i<ITEM_JOIN_MAX; i++ )		// �⺻������ ��� �ʿ䰡 ���� ����..
	{
		if( item_join[i].item_id == item_no )
		{
			if( !item_join[i].need_know )
			{
				return -1;		// ��� �ʿ䰡 ����.
			}
			else break;
		}
	}

//	CorrectLearnItem(ch);//???

	const int iItemMax = ch->ItemMax;
	int ct;
	for( ct=0 ; ct < iItemMax ; ct++ )
	{
		if( ch->Item[ct] )
		{
			if( ch->Item[ct] == item_no )		// �̹� ���� ����...
				return -1;
		}	
	}
	return ct;
}

extern int TotalItemJoinTableCount;
int CSkillMgr::CheckYouCanSkillIt( const int cn, CItem_Join &join, const int skillno, const int x, const int y, const int target_id )
{
	int iCheck = 0;
	bool bJoinFlag = false;
	CHARLIST *ch = ::CheckServerId(cn);

	// �װ� �˰� �ִ� Item�ΰ�?
	iCheck = FindLearnItem( ch, target_id );		// ���� ������ -1�� ����, ������ item_max�� ����
	if( iCheck != -1 ){ return ITEMMAKE_UNKNOWN; }	// ���� �𸣴� ������...

	for(int i = 0 ; i < TotalItemJoinTableCount ; i ++)
	{		
		if( item_join[i].item_id == target_id && item_join[i].need_skill1 == skillno )
		{		
			join = item_join[i];
			bJoinFlag = true;
			break;
		}	
	}
	
	if(!bJoinFlag){ return ITEM_MAKE_NO_JOIN;}

	// ������ �����Ѱ�?
	iCheck = CheckSkillTool( ch, join, skillno, target_id );
	if( iCheck ){ return iCheck;}//���⼭ �޼����� �ִ��� �ؾ� �� ��
	
	return ITEMMAKE_YOUCAN;
}

/*
48				���� ������
49				���� �߼���
50				���� ä����
51				���� ���ÿ�
52				���� ���ÿ�
53				���� �����
54				���� ���簡����
55				���� �����
56				���� ����ä����
57				���� ���縸����
58				���� ������
59				���� �����
60				���� ö����
61				���� ���¿丮��(��Ŀ)
62				���� ��¿丮��
63				���� �ٴ÷�
64				���� Ȱ���ۿ�
65				���� �Ҵ�
66				���� �������ۿ�
67				���� �г�������
68				���� ���ֿ̹�
69				���� �׹���
70				���� �ν˵���
71				�۾��� �뱤��
72				�۾��� ��Ʋ
73				�۾��� ����
74				�۾��� ���
75				�۾��� ȭ��
76				�۾��� �������
77				�۾��� ����
78				�۾��� �Ҵ� 
*/
// ��� : ������ ������ �䱸�ϴ�  ������ ��ġ�ϴ°��� Ȯ���ϰ� 
//        ������ �����Ǿ� ������� �����Ų��. 
//	������ ������ 0 �� �����Ѵ�
int CSkillMgr::CheckSkillTool( LPCHARLIST ch, CItem_Join &join, const int iSkillNo, const int target_id )
{	
	const int iToolNo = ch->equip[WT_WEAPON].item_no;;
	CItem *ii  = ItemUnit(iToolNo);
	int iNowToolKind = 0;
	int iNeedToolKind = 0;

	// SkillMain.Tool == -1  :  ������ �������̺��� �����Ѵ�. 
	// SkillMain.Tool ==  0  :  ������ �ʿ�� ���� �ʴ´�. 
	// SkillMain.Tool >   0  :  SkillMain.Tool�� Ÿ�԰� ������ �������� Ÿ�԰� ����Ѵ�. 
	switch( SkillTbl[ iSkillNo].Tool )
	{
	case 0	:{  return 0;	}break;// Tool�� �ʿ�� ���� �ʴ´�. 
	case -1 :// ItemJoin ���̺��� �����Ѵ�. 
		{
			if( !join.Tool_kind )  {return 0;}	// ������ �ʿ������..
			
			if( !ii ){return join.Tool_kind;}
			iNowToolKind = ii->GetItemKind();
			iNeedToolKind = join.Tool_kind;
			if( iNeedToolKind == iNowToolKind )
			{	//�⺻ ������ ��� �ϸ� ������ ����
				DecreaseEquipDuration( ch, rand()%5+3, WT_WEAPON, 0 ,true); 
				return 0; // ��Ȯ�� �����̴�. 
			}
			else
			{
				return iNeedToolKind;
			}
		}break; 
	
	default :	
		{
			if( !ii ){return 1;}
			iNowToolKind = ii->GetItemKind();
			iNeedToolKind = SkillTbl[iSkillNo].Tool;//üũ �ؾ� �ϳ�
			if( iNeedToolKind == iNowToolKind )
			{	//�⺻ ������ ��� �ϸ� ������ ����
				DecreaseEquipDuration( ch, rand()%5+3, WT_WEAPON, 0 ,true);
				return 0;
			}
			else
			{
				return iNeedToolKind;
			}
		}break;
	}
	return 0;	// ������ ���� �ʴ´�.   // ������ �ʿ�� ���� �ʴ´�. 
}

bool CSkillMgr::CanLearnItem( CHARLIST *ch, const int join_num )const
{
	if(	 item_join[join_num].need_skill1? (ch->Skill[item_join[join_num].need_skill1]&& ch->skillexp[SkillTbl[item_join[join_num].need_skill1].inclusive].skillexp/10000 >= ( unsigned int )item_join[join_num].skill1_minimum ):true )
	{
		if(	 item_join[join_num].need_skill2? (ch->Skill[item_join[join_num].need_skill1]&& ch->skillexp[SkillTbl[item_join[join_num].need_skill2].inclusive].skillexp/10000 >= ( unsigned int )item_join[join_num].skill2_minimum ):true ) 
		{
			return true;
		}
	}
	return false;
}

void CSkillMgr::CharacterToAlive(CHARLIST *ch, const int iType)//����ִ� �Լ�
{
	if( iType )//������ Ǯ��
	{
		ch->Hp		= ch->HpMax;
		ch->Hungry  = ch->HungryMax;
		ch->Mana	= ch->ManaMax;
		ch->Cp = 0;
	}
	else//������ ����Ʈ��
	{
		ch->Hp		= ch->HpMax		* 20 / 100;
		ch->Hungry  = ch->HungryMax * 30 / 100;
		ch->Mana	= 0;
		ch->Cp = 0;
	}

	ch->MoveGox = ch->X;//030102 lsw
	ch->MoveGoy = ch->Y;
	
	aliveCharacter( ch );
	t_packet packet;
	packet.h.header.type = CMD_PLEASE_GIVE_LIFE;
	packet.h.header.size = sizeof( t_please_give_life );
	packet.u.please_give_life.id		= ch->GetServerID();
	packet.u.please_give_life.who		= -1;
	packet.u.please_give_life.nLife	= ch->Hp;
	packet.u.please_give_life.nMana	= ch->Mana;
	packet.u.please_give_life.nHungry	= ch->Hungry;
	QueuePacket( connections, ch->GetServerID(), &packet,1 );
	CastMe2Other( ch->GetServerID(), &packet );
	CheckAndSendChangeEquip(ch,true); // CSD-021216
}

void CSkillMgr::CharacterToGhost(CHARLIST *ch, const int iType)//�������� ����� �ִ� �Լ�
{
	ch->deadcount	= 0;
	if (ch->Hp <= 0)  ch->Hp = 0;
	if (ch->Cp <= 0)  ch->Cp = 0;
	ch->viewtype = VIEWTYPE_GHOST_;
	ch->InitCurse();  
	SendViewTypeChange( ch->GetServerID(), VIEWTYPE_GHOST_ );
	ch->MoveGox = ch->X;//030102 lsw
	ch->MoveGoy = ch->Y;
}

bool CSkillMgr::SetCanViewGhost(CHARLIST *ch, const int iTime)//�ʴ���
{
	if(iTime)
	{
		const DWORD dwTargetTime = (global_time + (iTime*1000));
		if( ch->dwCanViewGhostTime > dwTargetTime )
		{
			return false;
		}

		ch->bCanViewGhost = true;
		ch->dwCanViewGhostTime = dwTargetTime;//�ʴ���
		ch->SendCharInfoBasic(  VIEWGHOST, iTime);//020704 lsw
	}
	else
	{
		ch->bCanViewGhost = false;
		ch->SendCharInfoBasic( VIEWGHOST, iTime);//020704 lsw
	}
	return true;
}

bool CSkillMgr::SetCanViewBomb(CHARLIST *ch, const int iTime)//�ʴ���
{
	if(iTime)
	{
		const DWORD dwTargetTime = (global_time + (iTime*1000));
		if( ch->dwCanViewBombTime > dwTargetTime )
		{
			return false;
		}

		ch->bCanViewBomb = true;
		ch->dwCanViewBombTime = dwTargetTime;//�ʴ���
		ch->SendCharInfoBasic(  VIEWBOMB, iTime);//020704 lsw
	}
	else
	{
		ch->bCanViewBomb = false;
		ch->SendCharInfoBasic( VIEWBOMB, iTime);//020704 lsw
	}
	return true;
}

bool CSkillMgr::SetCanViewTrap(CHARLIST *ch, const int iTime)//�ʴ���
{
	if(iTime)
	{
		const DWORD dwTargetTime = (global_time + (iTime*1000));
		if( ch->dwCanViewTrapTime > dwTargetTime )
		{
			return false;
		}

		ch->bCanViewTrap = true;
		ch->dwCanViewTrapTime = dwTargetTime;//�ʴ���
		ch->SendCharInfoBasic(  VIEWTRAP, iTime);//020704 lsw
	}
	else
	{
		ch->bCanViewTrap = false;
		ch->SendCharInfoBasic( VIEWTRAP, iTime);//020704 lsw
	}
	return true;
}

bool CSkillMgr::IncSkillCount(CHARLIST *ch, const int iType, const int iKind, const int iPoint)//020815 lsw
{

	if (!ch->IsPayedUser())
	{
		return false;
	}

	short nCount=0, nBeforeCount = 0;
	switch(iType)
	{
	case	SADONIX_NO:
		{
			nBeforeCount = ch->nSkillSadCount;
			(ch->nSkillSadCount) += iPoint;
			if( ch->nSkillSadCount > MAX_ITEM_SKILL_COUNT)
			{
				ch->nSkillSadCount = MAX_ITEM_SKILL_COUNT;
			}
			nCount = (ch->nSkillSadCount);
		}break;
	case	BLESSLEAF_NO:
		{
			nBeforeCount = ch->nSkillBlessLeaf;
			(ch->nSkillBlessLeaf) += iPoint;
			if( ch->nSkillBlessLeaf > MAX_ITEM_SKILL_COUNT)
			{
				ch->nSkillBlessLeaf = MAX_ITEM_SKILL_COUNT;
			}
			nCount = (ch->nSkillBlessLeaf);
		}break;
	default:
		{
			return false;
		}break;
	}
	
	if( (nCount/100) != (nBeforeCount/100) )
	{
		t_packet p;
		p.h.header.type = CMD_NOW_SKILL_POINT;
		p.h.header.size = sizeof(t_NowSkillPoint);

		p.u.NowSkillPoint.item.item_no	=	iType;
		p.u.NowSkillPoint.iNowPoint		=	nCount;
				
		::QueuePacket(connections,ch->GetServerID(),&p,1);
	}

	if(MAX_ITEM_SKILL_COUNT <= nCount)
	{
		const int r = rand()%1000;//1%�� �ؾ� �ð��� ���缭 ������ �Ǵ°� ó�� ���̴°� ���� �� �ִ�
		if(10 > r)//Ư�� �������� ���� �� �ִ�
		{
			SendCMD_CONFORM_SADONIX(ch->GetServerID(), iType, iKind, ch->X, ch->Y);//���� ���� ��� 
		}
	}
	return true;
}

void CSkillMgr::SendSkillExp( const int iType, const t_skillexp exp, const int cn)
{
	if( !::CheckServerId( cn ) ) return;
	
	t_packet packet;
	packet.h.header.type = CMD_SKILL_EXP;
	packet.h.header.size = sizeof( k_server_skill_exp );
	
	packet.u.kein.server_skill_exp.type			= iType;
	packet.u.kein.server_skill_exp.exp.skillexp = exp.skillexp;
	packet.u.kein.server_skill_exp.exp.rare		= exp.rare;
	
	::QueuePacket( connections, cn, &packet, 1 );		// Ŭ���̾�Ʈ ����	������ ����� �����ش�.
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CSkillMgr::Skill_Detect_ghost					(const int iCn, const int iSkillNo, const int iX, const int iY, const int targettype, const int iTargetID ){return 0;}
int CSkillMgr::Skill_Talk_with_ghost				(const int iCn, const int iSkillNo, const int iX, const int iY, const int targettype, const int iTargetID ){return 0;}
int CSkillMgr::Skill_Poisoning						(const int iCn, const int iSkillNo, const int iX, const int iY, const int targettype, const int iTargetID ){return 0;}
int CSkillMgr::Skill_Detect_poison					(const int iCn, const int iSkillNo, const int iX, const int iY, const int targettype, const int iTargetID ){return 0;}
int CSkillMgr::Skill_Find_traps						(const int iCn, const int iSkillNo, const int iX, const int iY, const int targettype, const int iTargetID ){return 0;}
int CSkillMgr::Skill_Remove_traps					(const int iCn, const int iSkillNo, const int iX, const int iY, const int targettype, const int iTargetID ){return 0;}
int CSkillMgr::Skill_Entrapment						(const int iCn, const int iSkillNo, const int iX, const int iY, const int targettype, const int iTargetID ){return 0;}
int CSkillMgr::Skill_Detect_bomb					(const int iCn, const int iSkillNo, const int iX, const int iY, const int targettype, const int iTargetID ){return 0;}
int CSkillMgr::Skill_Bomb_in_box					(const int iCn, const int iSkillNo, const int iX, const int iY, const int targettype, const int iTargetID ){return 0;}
int CSkillMgr::Skill_Remove_bomb					(const int iCn, const int iSkillNo, const int iX, const int iY, const int targettype, const int iTargetID ){return 0;}
int CSkillMgr::Skill_Stealing						(const int iCn, const int iSkillNo, const int iX, const int iY, const int targettype, const int iTargetID ){return 0;}
int CSkillMgr::Skill_Reclaim						(const int iCn, const int iSkillNo, const int iX, const int iY, const int targettype, const int iTargetID ){return 0;}
int CSkillMgr::Skill_Sow							(const int iCn, const int iSkillNo, const int iX, const int iY, const int targettype, const int iTargetID ){return 0;}
int CSkillMgr::Skill_Havest							(const int iCn, const int iSkillNo, const int iX, const int iY, const int targettype, const int iTargetID ){return 0;}
int CSkillMgr::Skill_Mining							(const int iCn, const int iSkillNo, const int iX, const int iY, const int targettype, const int iTargetID ){return 0;}
int CSkillMgr::Skill_Refine							(const int iCn, const int iSkillNo, const int iX, const int iY, const int targettype, const int iTargetID ){return 0;}
int CSkillMgr::Skill_Fish_hooking					(const int iCn, const int iSkillNo, const int iX, const int iY, const int targettype, const int iTargetID ){return 0;}
int CSkillMgr::Skill_Netting						(const int iCn, const int iSkillNo, const int iX, const int iY, const int targettype, const int iTargetID ){return 0;}
int CSkillMgr::Skill_Cutting						(const int iCn, const int iSkillNo, const int iX, const int iY, const int targettype, const int iTargetID ){return 0;}
int CSkillMgr::Skill_Lumbering						(const int iCn, const int iSkillNo, const int iX, const int iY, const int targettype, const int iTargetID ){return 0;}
int CSkillMgr::Skill_Butchering						(const int iCn, const int iSkillNo, const int iX, const int iY, const int targettype, const int iTargetID ){return 0;}
int CSkillMgr::Skill_Digging						(const int iCn, const int iSkillNo, const int iX, const int iY, const int targettype, const int iTargetID ){return 0;}
int CSkillMgr::Skill_Makepowder						(const int iCn, const int iSkillNo, const int iX, const int iY, const int targettype, const int iTargetID ){return 0;}
int CSkillMgr::Skill_Shear							(const int iCn, const int iSkillNo, const int iX, const int iY, const int targettype, const int iTargetID ){return 0;}	
int CSkillMgr::Skill_Carpentry						(const int iCn, const int iSkillNo, const int iX, const int iY, const int targettype, const int iTargetID ){return 0;}	
int CSkillMgr::Skill_Blacksmithy					(const int iCn, const int iSkillNo, const int iX, const int iY, const int targettype, const int iTargetID ){return 0;}	
int CSkillMgr::Skill_Cooking						(const int iCn, const int iSkillNo, const int iX, const int iY, const int targettype, const int iTargetID ){return 0;}	
int CSkillMgr::Skill_Clothing						(const int iCn, const int iSkillNo, const int iX, const int iY, const int targettype, const int iTargetID ){return 0;}	
int CSkillMgr::Skill_Tailoring						(const int iCn, const int iSkillNo, const int iX, const int iY, const int targettype, const int iTargetID ){return 0;}	
int CSkillMgr::Skill_Bowcrafting					(const int iCn, const int iSkillNo, const int iX, const int iY, const int targettype, const int iTargetID ){return 0;}	
int CSkillMgr::Skill_Fletching						(const int iCn, const int iSkillNo, const int iX, const int iY, const int targettype, const int iTargetID ){return 0;}	
int CSkillMgr::Skill_Makepotion						(const int iCn, const int iSkillNo, const int iX, const int iY, const int targettype, const int iTargetID ){return 0;}
int CSkillMgr::Skill_Makemetal						(const int iCn, const int iSkillNo, const int iX, const int iY, const int targettype, const int iTargetID ){return 0;}	
int CSkillMgr::Skill_Makebottle						(const int iCn, const int iSkillNo, const int iX, const int iY, const int targettype, const int iTargetID ){return 0;}
int CSkillMgr::Skill_Candlemaking					(const int iCn, const int iSkillNo, const int iX, const int iY, const int targettype, const int iTargetID ){return 0;}	
int CSkillMgr::Skill_Snooping						(const int iCn, const int iSkillNo, const int iX, const int iY, const int targettype, const int iTargetID ){return 0;}	
int CSkillMgr::Skill_Makefire						(const int iCn, const int iSkillNo, const int iX, const int iY, const int targettype, const int iTargetID ){return 0;}	
int CSkillMgr::Skill_Taming							(const int iCn, const int iSkillNo, const int iX, const int iY, const int targettype, const int iTargetID ){return 0;}
int CSkillMgr::Skill_Evaluate_inteligence			(const int iCn, const int iSkillNo, const int iX, const int iY, const int targettype, const int iTargetID ){return 0;}	
int CSkillMgr::Skill_Evaluate_wisdom				(const int iCn, const int iSkillNo, const int iX, const int iY, const int targettype, const int iTargetID ){return 0;}	
int CSkillMgr::Skill_Anatomy_judgement				(const int iCn, const int iSkillNo, const int iX, const int iY, const int targettype, const int iTargetID ){return 0;}	
int CSkillMgr::Skill_Item_identification			(const int iCn, const int iSkillNo, const int iX, const int iY, const int targettype, const int iTargetID ){return 0;}	
int CSkillMgr::Tactics_crapple						(const int iCn, const int iSkillNo, const int iX, const int iY, const int targettype, const int iTargetID ){return 0;}
int CSkillMgr::Tactics_swordmanship					(const int iCn, const int iSkillNo, const int iX, const int iY, const int targettype, const int iTargetID ){return 0;}
int CSkillMgr::Tactics_archery						(const int iCn, const int iSkillNo, const int iX, const int iY, const int targettype, const int iTargetID ){return 0;}
int CSkillMgr::Tactics_fencing						(const int iCn, const int iSkillNo, const int iX, const int iY, const int targettype, const int iTargetID ){return 0;}
int CSkillMgr::Tactics_mace_fighting				(const int iCn, const int iSkillNo, const int iX, const int iY, const int targettype, const int iTargetID ){return 0;}
int CSkillMgr::Tactics_pierce						(const int iCn, const int iSkillNo, const int iX, const int iY, const int targettype, const int iTargetID ){return 0;}
int CSkillMgr::Tactics_whirl						(const int iCn, const int iSkillNo, const int iX, const int iY, const int targettype, const int iTargetID ){return 0;}
int CSkillMgr::Tactics_hurl							(const int iCn, const int iSkillNo, const int iX, const int iY, const int targettype, const int iTargetID ){return 0;}
int CSkillMgr::Tactics_parrying						(const int iCn, const int iSkillNo, const int iX, const int iY, const int targettype, const int iTargetID ){return 0;}
int CSkillMgr::Tactics_double_swordmanship			(const int iCn, const int iSkillNo, const int iX, const int iY, const int targettype, const int iTargetID ){return 0;}
int CSkillMgr::Tactics_double_macefighting			(const int iCn, const int iSkillNo, const int iX, const int iY, const int targettype, const int iTargetID ){return 0;}
int CSkillMgr::Tactics_magery						(const int iCn, const int iSkillNo, const int iX, const int iY, const int targettype, const int iTargetID ){return 0;}
int CSkillMgr::Tactics_orison						(const int iCn, const int iSkillNo, const int iX, const int iY, const int targettype, const int iTargetID ){return 0;}
int CSkillMgr::Skill_Ghost_resurrection				(const int iCn, const int iSkillNo, const int iX, const int iY, const int targettype, const int iTargetID ){return 0;}	
int CSkillMgr::Skill_Gm_carpenter					(const int iCn, const int iSkillNo, const int iX, const int iY, const int targettype, const int iTargetID ){return 0;}		
int CSkillMgr::Skill_Gm_blacksmith					(const int iCn, const int iSkillNo, const int iX, const int iY, const int targettype, const int iTargetID ){return 0;}		
int CSkillMgr::Skill_Gm_cook						(const int iCn, const int iSkillNo, const int iX, const int iY, const int targettype, const int iTargetID ){return 0;}		
int CSkillMgr::Skill_Gm_tailor						(const int iCn, const int iSkillNo, const int iX, const int iY, const int targettype, const int iTargetID ){return 0;}
int CSkillMgr::Skill_Gm_bowcraft					(const int iCn, const int iSkillNo, const int iX, const int iY, const int targettype, const int iTargetID ){return 0;}
int CSkillMgr::Skill_Gm_alchemist					(const int iCn, const int iSkillNo, const int iX, const int iY, const int targettype, const int iTargetID ){return 0;}
int CSkillMgr::Skill_Gm_candlemaker					(const int iCn, const int iSkillNo, const int iX, const int iY, const int targettype, const int iTargetID ){return 0;}

void CSkillMgr::SkillMapper( const int cn, const int iSkillNo, const int x, const int y, const int targettype, const int target_id, const POS s )
{
	//acer7
	CHARLIST *ch = CheckServerId(cn);
	if( !ch ) return;


	if( target_id < 0 ) return;

	switch(iSkillNo)
	{
	case	SKILL_DETECT_GHOST						:
	case	SKILL_TALK_WITH_GHOST					:
	case	SKILL_POISONING							:
	case	SKILL_DETECT_POISON						:
	case	SKILL_FIND_TRAPS						:
	case	SKILL_REMOVE_TRAPS						:
	case	SKILL_ENTRAPMENT						:
	case	SKILL_DETECT_BOMB						:
	case	SKILL_BOMB_IN_BOX						:
	case	SKILL_REMOVE_BOMB						:
	case	SKILL_STEALING							:
	case	SKILL_RECLAIM							:
	case	SKILL_SOW								:
	case	SKILL_HAVEST							:
	case	SKILL_MINING							:
	case	SKILL_REFINE							:
	case	SKILL_FISH_HOOKING						:
	case	SKILL_NETTING							:
	case	SKILL_CUTTING							:
	case	SKILL_LUMBERING							:
	case	SKILL_BUTCHERING						:
	case	SKILL_DIGGING							:
	case	SKILL_MAKEPOWDER						:
	case	SKILL_SHEAR								:
	case	SKILL_CARPENTRY							:
	case	SKILL_BLACKSMITHY						:
	case	SKILL_COOKING							:
	case	SKILL_CLOTHING							:
	case	SKILL_TAILORING							:
	case	SKILL_BOWCRAFTING						:
	case	SKILL_FLETCHING							:
	case	SKILL_MAKEPOTION						:
	case	SKILL_MAKEMETAL							:
	case	SKILL_MAKEBOTTLE						:
	case	SKILL_CANDLEMAKING						:
	case	SKILL_SNOOPING							:
	case	SKILL_MAKEFIRE							:
	case	SKILL_TAMING							:
	case	SKILL_EVALUATE_INTELIGENCE				:
	case	SKILL_EVALUATE_WISDOM					:
	case	SKILL_ANATOMY_JUDGEMENT					:
	case	SKILL_ITEM_IDENTIFICATION				:
	case	TACTICS_Crapple							:
	case	TACTICS_swordmanship					:
	case	TACTICS_Archery							:
	case	TACTICS_Fencing							:
	case	TACTICS_Mace_fighting					:
	case	TACTICS_Pierce							:
	case	TACTICS_Whirl							:
	case	TACTICS_Hurl							:
	case	TACTICS_Parrying						:
	case	TACTICS_Double_Swordmanship				:
	case	TACTICS_Double_MaceFighting				:
	case	TACTICS_Magery							:
	case	TACTICS_Orison							:
	case	SKILL_GHOST_RESURRECTION				:
	case	SKILL_GM_CARPENTER						:
	case	SKILL_GM_BLACKSMITH						:
	case	SKILL_GM_COOK							:
	case	SKILL_GM_TAILOR							:
	case	SKILL_GM_BOWCRAFT						:
	case	SKILL_GM_ALCHEMIST						:
	case	SKILL_GM_CANDLEMAKER					:
	default:break;
	}
	return;
}

//npc�� ������ ���� �������� ����� �ش�.  ���� : 0   ���� : ���� ������ ���� ��ȯ
//Npc_Item_Table
//NPC_ITEM_TABLE_TOTAL_ITEM			60							//���� �ȿ� �ִ� �������� ��// 
//NPC_ITEM_TABLE_START_ITEM_POS		2							//�迭���� ��ġ
//NPC_ITEM_TABLE_PROBABILITY_POS	62							//�迭���� ��ġ.�����δ� 50��° ��ġ.
//NPC_ITEM_TABLE_BETWEEN_POS		63							//�迭���� ��ġ.
//NPC_ITEM_TABLE_LEVEL_POS			1							//�迭���� ��ġ.
int CSkillMgr::GetItemFallPercentPos(const int iFallNo)
{
	const int iTotalPer = ItemFallPercent[iFallNo].per[0];
	
	if( !iTotalPer ){ return 0;}
	const int iNowPer =rand()%iTotalPer;
	
	int iDummy = 0;
	int i = 1;
	for(; i < 21 ; i++)
	{
		iDummy += ItemFallPercent[iFallNo].per[i];
		if( iNowPer < iDummy )
		{
			break;
		}
	}
	return i;
}

int CSkillMgr::SetNPCItem(const int iBagNo, const bool bIsBasic, ItemAttr *pItem,const int iCallType)
{
	int iFallNo = 0;
	if(bIsBasic)
	{
		iFallNo = BasicItemBag[iBagNo].iBasicItemPercentTableNo;
	}
	else
	{
		iFallNo = RareItemBag[iBagNo].iRareItemBagPercentTableNo;
	}
	
	const int i = GetItemFallPercentPos(iFallNo);

	if(bIsBasic)
	{
		if(iCallType)//1�̸� ������ ������
		{
			*pItem = ItemMgr.GiveLegendItem(BasicItemBag[iBagNo].ItemNo[i]);
		}
		else // 0�̸� �븻 ������
		{
			*pItem = ItemMgr.GenerateItem(BasicItemBag[iBagNo].ItemNo[i]);
		}
		return 1;
	}
	else
	{
		RareMain rare;
		t_RareItemBag RareBag = RareItemBag[iBagNo];
		const int iR = rand()%100;
		if( 20 > iR )//20���ϴ� ��� ���´�
		{
			*pItem = ItemMgr.GiveRareItem(RareBag.ItemNo[i],0,RARE_ABLE,1,RareBag.iGradeMin,RareBag.iGradeMax,rare,H_LV_NO_HIGH_ITEM);//020725 lsw
		}
		else
		{
			*pItem = ItemMgr.GiveRareItem(RareBag.ItemNo[i],0,RARE_ABLE,1,RareBag.iGradeMin,RareBag.iGradeMax,rare,H_LV_HIGH_ITEM);//020725 lsw
		}
		return 1;
	}
	return 0;
}

int CSkillMgr::SetNPCInventory(CHARLIST *NPC, const int iItemControlNo,ItemAttr* pItem, const int iCallType)
{
	int a = 0, b = 0, c = 0;
	
	const int iFallNo	= ItemControl[iItemControlNo].ItemControlPercentNo;
	const int iTotalPer = ItemControlPercent[iFallNo].per[0];
	
	if( !iTotalPer ){ return 0;}
	const int iNowPer =rand()%iTotalPer;
	
	int iDummy = 0;
	int i = 1;
	for(; i < 8 ; i++)
	{
		iDummy += ItemControlPercent[iFallNo].per[i];
		if( iNowPer < iDummy )
		{
			break;
		}
	}

	switch(i)
	{
	case 1://�Ϲ� ������ ��
	case 2:
	case 3:
	case 4:
	case 5:
		{
			return SetNPCItem(ItemControl[iItemControlNo].ItemGroup[i], true,pItem,iCallType);
		}break;
	case 6://���� ������ ��
	case 7:
		{
			return SetNPCItem(ItemControl[iItemControlNo].ItemGroup[i], false,pItem);
		}break;
	default:
		{
		}break;
	}
	return 0;
}

int CSkillMgr::GenerateNPCItem2(CHARLIST *NPC, NPC_Generation *NPCGenerate)
{	
	//���Ǿ��� ���� �ɶ� ��Ȳ�� �Ǵ� �Ѵ�
	//����, Ư����, ��¥ , �̺�Ʈ ���� 
	int iItemControlNo = 0;

	if(rand()%2)//����
	{
		iItemControlNo = NPCGenerate->Inventory[0];//0~14 �����ϱ�
	}
	else
	{
		iItemControlNo = NPCGenerate->Inventory[1];//0~14 �����ϱ�
	}

	const int iMyMapGroup = MapInfo[MapNumber].group;

	if(NPCGenerate->Inventory[2] && iMyMapGroup == NPCGenerate->Inventory[2] )//�ʱ׷�
	{//���� �׷��� ���̸�
		iItemControlNo = NPCGenerate->Inventory[3];
	}

	if(NPCGenerate->Inventory[4] && iMyMapGroup == NPCGenerate->Inventory[4] )//�ʱ׷�
	{//���� �׷��� ���̸�
		iItemControlNo = NPCGenerate->Inventory[5];
	}

	if(NPCGenerate->Inventory[6] && iMyMapGroup == NPCGenerate->Inventory[6] )//�ʱ׷�
	{//���� �׷��� ���̸�
		iItemControlNo = NPCGenerate->Inventory[7];
	}

	//��¥
	if(NPCGenerate->Inventory[8]/10000 && NPCGenerate->Inventory[8]%10000)
	{
		if(IsEquipAbleCheckAbleDay(NPCGenerate->Inventory[8]/10000,NPCGenerate->Inventory[8]%10000))
		{
			iItemControlNo = NPCGenerate->Inventory[9];//0~14 �����ϱ�
		}
	}
	
	if(GT_EVENT_MONSTER == NPC->generationtype)
	{
		switch(NPC->GainedFame)	//1�̸� ������ 0�� �ʸ���
		{
		case 0:
			{
				if( NPCGenerate->Inventory[10])
				{
					iItemControlNo = NPCGenerate->Inventory[10];//0~14 �����ϱ�
				}
			}break;
		default:
			{
				if( NPCGenerate->Inventory[11])
				{
					 iItemControlNo = NPCGenerate->Inventory[11];//0~14 �����ϱ�
				}
			}
		}
	}

	int iMax = ItemControl[iItemControlNo].iFallItemCount;
    const int dropPercent = (NPC->generationtype == GT_SKB_FILE || NPC->generationtype == GT_SCRIPT) ? CurrentOperationsRates().dropPercent : 100;
    if (dropPercent != 100 && !ScaleOperationsDraws(iMax, dropPercent, rand()%100, iMax)) {
        MyLog(LOG_FATAL, "NPC drop multiplier rejected: draw count overflow");
        iMax = 0;
    }

	for(int iCount = 0; iMax > iCount ; iCount++)
	{
		ItemAttr *pItem = SearchInv(NPC);
		if(pItem)
		{
			SetNPCInventory(NPC,iItemControlNo,pItem);
		}
	}

	//021113 lsw
	const int iMyMapClass = MapInfo[MapNumber].Class;
	if(NPCGenerate->Inventory[12] && iMyMapClass == NPCGenerate->Inventory[12] )//��Ŭ������ ����
	{	//���� Ŭ������ ���̸�
		iItemControlNo = NPCGenerate->Inventory[13];

		int iMax = ItemControl[iItemControlNo].iFallItemCount;
    const int dropPercent = (NPC->generationtype == GT_SKB_FILE || NPC->generationtype == GT_SCRIPT) ? CurrentOperationsRates().dropPercent : 100;
    if (dropPercent != 100 && !ScaleOperationsDraws(iMax, dropPercent, rand()%100, iMax)) {
        MyLog(LOG_FATAL, "NPC drop multiplier rejected: draw count overflow");
        iMax = 0;
    }

		for(int iCount = 0; iMax > iCount ; iCount++)
		{
			ItemAttr *pItem = SearchInv(NPC);
			if(pItem)
			{
				SetNPCInventory(NPC,iItemControlNo,pItem,1);
			}
		}
	}
	//�� �ֱ�
	if( (rand()%100) < 30 ) 
	{
		int iRandMoney = NPCGenerate->Money_Max - NPCGenerate->Money_min;
		if( iRandMoney > 0 ) 
		{
			iRandMoney = rand()%iRandMoney;
		}
		else 
		{
			iRandMoney = 0;
		}
		const int iMoney= NPCGenerate->Money_min + iRandMoney;
		ItemAttr item;
		if( iMoney> 0 )
		{
			if(LocalMgr.IsChangeMoney())//030102 lsw
			{
				if( iMoney > 5 ) 
				{
					item = GenerateItem( IN_NEW_COINS, IATTR_MUCH,iMoney);			// 010204 YGI
				}
				else// if( iMoney <= 5 ) 
				{
					item = GenerateItem( IN_NEW_COIN, IATTR_MUCH, iMoney);//020505 lsw
				}
			}
			else
			{
				if( iMoney > 5 ) 
				{
					item = GenerateItem( IN_COINS, IATTR_MUCH,iMoney);			// 010204 YGI
				}
				else// if( iMoney <= 5 ) 
				{
					item = GenerateItem( IN_COIN, IATTR_MUCH,iMoney);//020505 lsw
				}
			}
			ItemAttr* pItem = SearchInv(NPC);
			if(pItem)
			{
				*pItem = item;
			}
		}
	}

	return 0;
}

int CSkillMgr::FallItemWhenNPCDie(CHARLIST *NPC)//������ �޴����� �Űܾ� �� �� ����..
{
	for( int k = 0 ; k < 3 ; k ++)
	{
		for( int j = 0 ; j < 3 ; j ++)
		{
			for( int i = 0 ; i < 8 ; i ++)
			{	 	
				ItemAttr item = NPC->inv[k][j][i];
				if( item.item_no )
				{	
					DropItem( NPC->X + (rand()%100)-50, NPC->Y +(rand()%100)-50, &item);	
				}	
			}
		}
	}
	int Npcno = NPC->SprNo + NPC->mutant * 100;
	if( !(Npcno<0 || Npcno>400) )
	{
		GiveSpecialItem(Npcno,NPC->X,NPC->Y,MapNumber,0);
	}
	return 0;
}

void CSkillMgr::SendCMD_CONFORM_SADONIX(const int cn, const int iItemNo, const int kind,const int x , const int y)
{
	CHARLIST *ch = ::CheckServerId(cn);
	if(ch)
	{
		t_packet p;
		p.h.header.type = CMD_CONFORM_SADONIX;
		p.h.header.size = sizeof(t_ConformSadonix);

		p.u.ConformSadonix.cn = cn;
		
		memcpy (p.u.ConformSadonix.name,ch->Name,20);
		
		p.u.ConformSadonix.item.item_no= iItemNo;
		p.u.ConformSadonix.kind= kind;
		
		p.u.ConformSadonix.x = x;
		p.u.ConformSadonix.y = y;

		QueuePacket(connections,DB_DEMON,&p,1);
	}
}

void CSkillMgr::RecvCMD_CONFORM_SADONIX(const t_packet &p)
{
	const int cn		=	p.u.ConformSadonix.cn;
	const int item_no	=	p.u.ConformSadonix.item.item_no;
	const int kind		=	MapInfo[MapNumber].port;
	const int x			=	p.u.ConformSadonix.x;
	const int y			=	p.u.ConformSadonix.y;

	ItemAttr item		=	GenerateItem(item_no);
	CHARLIST *ch =	::CheckServerId(cn);
	if(!ch)
	{
		MyLog(2,"RecvCMD_CONFORM_SADONIX Receiver Is Fault!.");
		return;
	}

	switch(item_no)
	{
	case	SADONIX_NO:		{	ch->nSkillSadCount	= 0;	}break;//�̰� ������ �ҵ�..
	case	BLESSLEAF_NO:	{	ch->nSkillBlessLeaf = 0;	}break;
	default:
		{	
			::MyLog(2,"RecvCMD_CONFORM_SADONIX Item No Is Wrong");
			return;
		}break;
	}

	::OutSound(ch,0,51);
	::OutMessage(ch,2,950);

	DropItem(x+rand()%50-25, y+rand()%50-25,&item);
	Send_RareItemMakeLog(	cn,	item_no,
							0,0,0,0,0,
							0,0,0,0,
							item.attr[IATTR_LIMIT],
							item.attr[IATTR_MUCH],
							0,3000,kind,//��Ʈ//Ÿ�Ե� 3000
							0,0,0,0,
							3000,0
							);
}

// �������� ĳ������ ������ ���� ���¸� �ʱ�ȭ��.
// �ʱ�ȭ�ϸ鼭 ������ Packet�� Ŭ���̾�Ʈ�� �����Ѵ�. 
inline void Init_CharacterCondition(CHARLIST* ch)
{
  if (ch->GetState() != CON_NORMAL)//0x00 ���°� ������ �ƴ� ���
  { 
		ch->SetState(CON_NORMAL);
		SendCharacterCondition(ch, 0);
	}

  ch->Clear();
}

inline void aliveCharacter(CHARLIST *d)
{	
	d->bAlive = ALIVE_;	
	d->InitCurse();
	d->deadcount = 0;
	d->Peacests = 0;
	d->viewtype = VIEWTYPE_NORMAL_;
	Init_CharacterCondition( d );
}
