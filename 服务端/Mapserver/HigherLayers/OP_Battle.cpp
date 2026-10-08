#include "..\stdafx.h"
#include "DefaultHeader.h"

#include "../LowerLayers/mylog.h" //!!
#include "CItem.h"
#include "WeatherControl.h"
#include "Map.h"
#include "Path.h"
#include "OP_Magic.h"
#include "SealStoneGameServer.h"
#include "SealStone.h"
#include "scrp_exe.h"
#include "teambattle.h"
#include "QuestFunction.h"	// 021022 kyo 
#include "LogManager.h"
#include "AIManager.h"
#include "ArenaManager.h"
#include "RegenManager.h"
#include "UserManager.h"
#include "GameplaySafety.h"

extern bool UsesRegularMonsterRespawn(CHARLIST* npc);

extern void CountNationDeath(CHARLIST* pCaster,CHARLIST* pTarget);
extern void CheckLocalWarDeath(CHARLIST* pCaster,CHARLIST* pTarget);	// LTS LOCALWAR
extern void CheckEventLocalWarDeath(CHARLIST* Attacker,CHARLIST* Defencer);		// 020115 LTS
extern bool isNationWarfieldServer();
extern bool IsWar();
extern void UpdateGuardStoneStatus(LPCHARLIST pCaster,LPCHARLIST pTarget);
extern bool isNewWarfieldServer();
extern void CountNewWarfieldDeath(LPCHARLIST Attacker,LPCHARLIST Defencer);


// 030506 YGI
extern char g_LogoutPath[80];
extern char g_ChangeLogPath[80];

// tactics skill definition
#define FIST	0
#define SWORD	1
#define AXE		2
#define MACE	3
#define STAFF	4
#define SPEAR	5
#define THR_SW	6
#define BOW		7
#define TWO_SW	8
#define TWO_AX	9
#define SHIELD	10
#define FENCING	11


#define PLAYER	0
#define NPC		1

#define MELEE			0	// ???.
#define RANGE_BOW		1	// ???..
#define RANGE_HURL		2	// ???.
#define RANGE_DEFAULT	3	// ???..
#define MAGIC			4	// ??.

extern NPCName_by_Gender	NPC_Name_Ref[Num_Of_NPC_Name];
extern NPCLev_to_Exp		NPC_Lev_Ref[Num_Of_NPC_Lev] ;
extern NPC_Generation		NPC_Gen_Ref[Num_Of_NPC_Generation] ;

// 010129 
extern NPC_Generation		NPC_Gen_Ref[Num_Of_NPC_Generation];
extern MAGICTABLE Magic_Ref[] ;

extern int GetEnemyEnemy( CHARLIST *a, CHARLIST *d );
extern bool isLevelUp(CHARLIST *chr);
extern void getItemIndex(int, int &, int &) ;


extern void SendServerKillPc( short int cn );			//
///////////////////// 0811 yjs ?? ////////////////////////

extern bool IsWar();	// 010904 LTS								// ??? ???? ??? ??? ?? ??? ??
extern bool CanDestroyTarget(CHARLIST* pCaster,CHARLIST* pTarget);  // 010904 LTS	// ???? ?? ??? ??
extern void CheckBossAndDeActiveGroupNo(CHARLIST* Npc);				// LTS AI2

void rollDice(int base_hd, int &dice_res);
int  getDistance(CHARLIST *a, CHARLIST *d);
int  calcClassWeaponCorrection( CHARLIST * a, int itemtype );	// ???? ?? Item? ?????? 20%????. 
short int getWeatherCorrection( void );
int  calcRangeCorrection(CHARLIST *a, CHARLIST *d );
int  AttackMapper( int a_id, int d_id,  int *damage_val);
void SendNK( int id, int nk );
void SendNK( int id, int nk3, int nk4, int nk6 );
inline int  IsHeNK(  LPCHARLIST ch, int nation  );
inline int	OutNK( LPCHARLIST ch, int nation );
void		InNK( LPCHARLIST ch, int nation, short int add_nk );
inline bool IsWarfare( int anation, int bnation );
int			GetCaseOfNK( int cur_nation, int a_nation, int d_nation, int a_nk, int d_nk ) ;
void		CheckNK( int a_id, int d_id, int type  );
extern void CheckDragonLordWarTeam(CHARLIST *a,CHARLIST *d);			// LTS DRAGONLORD



// 020428 YGI acer
extern int SaveLogDefault( char *file_name, char *str, int is_first );
extern void SaveCharacterBin( char *filename, CHARLIST *ch );

///////////////////////////////////////////////////////////////////////////////////////
///
///		User Function Definitions....
///

inline int ran( int r )
{
	if( r == 0 ) return 0;
	return rand()%r;
}

void rollDice(int base_hd, int &dice_res)		// 0712 YGI
{	
	int dice_type, dice_repeat;
	int i ;
	
	int plus = 0;
	if( base_hd > 1000000 ) 
	{
		plus = base_hd % 1000;
		base_hd /= 1000;
	}
	
	dice_type	= base_hd % 1000 ;
	dice_repeat	= base_hd / 1000 ;
	
	dice_res= 0 ;
	for(i= 0 ; i < dice_repeat ; i++) {
		dice_res+= (ran(dice_type) + 1);
	}
	dice_res += plus;
}	

int getDistance(CHARLIST *a, CHARLIST *d)
{
	double dist ;
	dist= pow((a->X - d->X), 2) + pow((a->Y - d->Y), 2) ;
	dist= sqrt(dist) ;
	
	//return -1 ;
	return (int)dist / TILE_SIZE ;
}	

int calcClassWeaponCorrection( CHARLIST * a, int itemtype )	
{				
	int item_no= a->equip[ itemtype].item_no ;
	int type, no;
				
	getItemIndex( item_no, type, no );
	if( item_no <= 0 ) return 80;
				
	CItem *ii = ItemUnit( type, no );
	if( ii == NULL ) return 80;
	else		
	{			
		if ( itemtype == WT_WEAPON || itemtype == WT_SHIELD ) 
		{		
			switch( a->Class )
			{	
			case  WARRIOR	:	return CWeapon[no].Class_Warrior * 20 + 80;
			case  THIEF		:	return CWeapon[no].Class_Archer  * 20 + 80;
			case  ARCHER	:	return CWeapon[no].Class_Thief   * 20 + 80;
			case  WIZARD	:	return CWeapon[no].Class_Wizard  * 20 + 80;
			case  PRIEST	:	return CWeapon[no].Class_Cleric  * 20 + 80;
			default : return 80;
			}	
		}		
	}				
				
	return 80;
}					

static bool	bAttackChangeCondition_slow_i = false;
static bool	bAttackChangeCondition_slow_you = false;
static bool	bAttackChangeCondition_daze_i  =false;
static bool	bAttackChangeCondition_daze_you  =false;
// ???-??? ??: 1.??(???:100/???:100/???:90) 2.??(???:90/???:100/???:100) 3.??(???:90/???:100/???:100)
//                     4.???(???:100/???:100/???:100) 5.???(???:100/???:100/???:100)
static	int rate_by_class[5][3] = { { 100, 90, 100 }, { 90, 100, 100}, { 90, 100, 100}, { 100, 100, 100 }, { 100, 100, 100 } };

extern ItemAttr *GetItemByPOS( int cn, POS item ); // itemtable.cpp

void SendItemAttrDurationChange( t_connection c[], int cn, int pos )
{	
	t_packet p;
	
	ItemAttr *i = &c[cn].chrlst.equip[pos];
	if( i == NULL ) return;
	if( i->item_no == 0 ) return;
	
	p.h.header.type = CMD_ITEM_DURATION_CHANGE;
	{
		p.u.item_duration_change.pos = pos;
		p.u.item_duration_change.dur = LOWORD( i->attr[ IATTR_DURATION] );
	}
	p.h.header.size = sizeof( t_item_duration_change );
	
	QueuePacket( c, cn, &p, 1 );
}	


extern void GetItemDuration( ItemAttr item, WORD &d_curr, WORD &d_max );	// ??? ??? ?? ??
// 020428 YGI acer
extern int BreakItemByDuration( ItemAttr *item, CHARLIST *ch, POS pos );

/*	??? ???? ??? ??..
// type : 0 ????.
//        1 ??? 

  
	d_curr = LOWORD( item.attr[IATTR_DURATION] );
	d_max = HIWORD( item.attr[IATTR_DURATION] );
	
	  ?? ????  1000??? ????. 
	  
		??? 1000? ??? ??? Display? /10? ?? ????. 
		
*/
int getEquipRandomPos( void )//??? ??? ???? ???? ??
{	
	const int pos = rand()%100;
	
	if( pos < 25 )  {return	WT_ARMOR;}
	if( pos < 50 )  {return  WT_SHIELD;}
	if( pos < 75 )  {return  WT_HELMET;}
	if( pos < 100 ) {return  WT_SHOES;}
	
	return WT_ARMOR; 
}	

// ??? ??? ??? ?? ????? ??? ???? ??..
////////////////////// 0804 lkh ?? ////////////////////////
int calcRangeCorrection(CHARLIST *a, CHARLIST *d )
{	
	int range_corr= 100;
	int min = 0, mid = 0, max = 0;
	
	const int dist= getDistance(a, d) ;
	if(dist < 2 )			return 100;		// ?? ??? 64????..
	
	const int ino = a->equip[ WT_WEAPON].item_no;		// Item? ??.
	if( ino == 0 )			return 100;
	
	CItem *ii = ItemUnit( ino/1000, ino%1000 );	// Item? ??.
	if( ii == NULL )		return 100;
	
	
	// ??? ????..
	/*
	int range = ii->Getrange();
	if( range < 2 )			return 100;
	*/
	
	switch( ii->GetItemKind() ) 
	{
	case IK_HURL: // general throwing weapon
		min = 3;
		mid = 4;
		max = 5;
		break ;
		
	case IK_LONG_BOW:	// 010724 LSW
	case IK_DUAL_BOW://030102 lsw Dual BOW
		min = 13;
		mid = 19;
		max = 22;
		break ;
		
	case IK_SHORT_BOW : // short BOW
		min = 4;
		mid = 8;
		max = 12;
		break ;
		
	case IK_TWO_ARROW :	//??.??..
	case IK_DUAL_ARROW:
		min = 5;
		mid = 10;
		max = 15;
		break ;
		
	default :
		min = 3;
		mid = 5;
		max = 7;
		break;
	}
	
	if		( dist < min )		range_corr= 100;
	else if	( dist < mid )		range_corr=  90;
	else if	( dist < max )		range_corr=  85;
	
	return 	range_corr;
}	

int addEXP(CHARLIST *a, CHARLIST *d, int criteria, int damage )
{	//< CSD-030509
	if (a == NULL || d == NULL) return 0;
	if (a->GetLevel() < 0 || a->GetLevel() >= Num_Of_NPC_Lev ||
		d->GetLevel() < 0 || d->GetLevel() >= Num_Of_NPC_Lev ||
		(d->IsNpc() && (d->npc_index < 0 || d->npc_index >= Num_Of_NPC_Generation)))
		return 0;
	if (g_pArenaManager->IsColossusArena())
	{
		return 0;
	}
	//> CSD-030509 
	if (a->IsNpc())                     return 0;
	if (a->IsPlayer() && d->IsPlayer()) return 0;
			
	int exp = 0;
	float base_point = 0;
	DWORD partyexp;
	int tdamage;
	
	int t = 1, i;
	int partexpflag = 0;											
	bool bCanGetExpPcPc = false;
	
	damage = abs(damage);
	
	switch (d->Race)
	{ // ???? ?? ?? NPC
    case GUARD:
    case SEALSTONE: return 0; 
	}
	
	if( d->SprNo == 96 ) return 0;
	if (d->IsTamedNpc())  return 0;	// ??? ????? ??? ??? ??.
	
#ifdef _NATION_APPLY_
	if( d->SprType != SPRITETYPE_NPC ) 
		if( a->name_status.nation != d->name_status.nation ) bCanGetExpPcPc = true;	// 0927 YGI
#endif
		
		switch(criteria)					
		{
		case HIT_FAILED : return 0;											
		case HIT_AND_DEAD:
		case HIT_AND_NOTDEAD:	
			{
				if (bCanGetExpPcPc == false)
					if (a->IsPlayer() && d->IsPlayer())
					{// PK? ????. 
						return 0;
					}		
					
					if( d ) // ???? ??? ?? NPC?..					
						if( d->SprType == SPRITETYPE_NPC )
						{
							switch( d->SprNo )
							{
							case 92 :	// ??..
							case 93 :	// ??..
							case 94 :	// ??..
							case 95 :	// ????.
							case 96 :	// ????.
								{
									if (a->GetLevel() > ENABLE_DUMMY_LEVEL)
									{	//< CSD-030806
										return 0;
									}	//> CSD-030806
								}
							}
							// NPC? ??? ????...????..
							if( d->Race == 1 ) return 0;
						}
						
						const int nTotal = (d->IsPlayer()) ? NPC_Lev_Ref[d->GetLevel()].nTrainingExp:
															 NPC_Gen_Ref[d->npc_index].nEpMax; // CSD-030806 
						if (a->GetLevel() == d->GetLevel())
						{	//< CSD-030806 
							t = 1;
						}	//> CSD-030806 
						
						if (a->GetLevel() < d->GetLevel())
						{	//< CSD-030806 
							t = 1;
							if( NPC_Lev_Ref[a->GetLevel()].nGreatRate )
							{
								t = (d->GetLevel() - a->GetLevel())/NPC_Lev_Ref[a->GetLevel()].nGreatRate;
							}
							else
							{
								SaveCharacterBin( "error_level_check", a);
							}
						}	//> CSD-030806 
						
						if (a->GetLevel() > d->GetLevel())
						{	//< CSD-030806 
							t = 1;
							if( NPC_Lev_Ref[a->GetLevel()].nLessRate )
							{
								t = (a->GetLevel() - d->GetLevel())/NPC_Lev_Ref[a->GetLevel()].nLessRate;
							}
							else
							{
								SaveCharacterBin( "error_level_check", a);
							}
						}	//> CSD-030806 
						
						base_point = (t < 1) ? nTotal:nTotal/t;
						break;
			}
		case HIT_GHOSTRESURRECTION:
			{
				base_point = damage;
				break;
			}
		}
		
		
		// base_point? ??? ??? ?? ????. 
		switch(criteria)
		{
		case HIT_FAILED:		exp = 0; partexpflag = 0;	break;
		case HIT_AND_DEAD:	
			{
				exp = base_point*0.4;
				partexpflag = 1; // ??? ???.	
				
				if (d)
				{
					switch (d->SprNo) 	
					{ // ??? ?????? ??? ?? ??? ??? ???. 
					case 92:
					case 93:
					case 94:
					case 95:
					case 96: partexpflag = 0;	break;
					}		
				}
				
				break; 
			}
		case HIT_AND_NOTDEAD:
			{
				if (damage > d->HpMax)
				{
					exp =	base_point;
				}
				else
				{
						if (d->HpMax <= 0) return 0;
						tdamage = static_cast<int>(static_cast<__int64>(damage)*100/ATTACKPOWER_PRO_);
					exp =	base_point*tdamage/d->HpMax;
				}
				
				partexpflag = 1; // ??? ???.
				break; 
			}
		case HIT_AND_ALIVE:	        
			{
				exp = (DWORD)(base_point*0.02);	
				partexpflag = 1; // ??? ???.  
				break; 
			}
		case HIT_GHOSTRESURRECTION:	
			{
				exp = (DWORD)base_point;  
				partexpflag = 0;	 
				break;
			}
		default :  
			{
				return 0;
			}
		}
		
		if( bCanGetExpPcPc == true )
		{	
			exp /= GET_EXP_PC_AND_PC_;	// PC??? ???? ??? ??. 
		}	
		
		if( exp == 0 ) return 0; 	
		
		if (a->GetLevel() < 5)
		{	//< CSD-030806
			exp += 2; // ?? 2??? ??? ??.   ????? ??? ???..
		}	//> CSD-030806
		
		CHARLIST *ttt[MAX_PARTY_MEMBER] ={0,};
		int tc = 0;

		if( partexpflag == 1  ) // ???? ???????, ??? ??? ???. 
		{
			AREA *aa = connections[a->GetServerID()].area_tbl;

			for( i = 0; i < MAX_AREA_BUFFER; i ++)
			{			
				if( aa[i].bAlive == TRUE )
				{		
					switch( aa[i].Type )
					{						
					case 1:
						{
							CHARLIST *tt = ::CheckServerId(aa[i].ID);

							if (tt != NULL && !tt->IsDead())
							{
								if( BoxAndDotCrash( a->MoveSx - 15,  a->MoveSy - 15, 30, 30, tt->MoveSx, tt->MoveSy ) )
								{	
									for (int i = 0; i < MAX_PARTY_MEMBER; i++)
									{
										if (a->party[i].On && !stricmp(a->party[i].Name, tt->Name))
										{   // ????? ?? ??
											if (abs(a->GetLevel() - tt->GetLevel()) <= ENABLE_PARTY_LEVEL)
											{   //< CSD-030806 : ????? 35???? ??? ??
												goto YES_FIND_PARTY_;
											}	//> CSD-030806
										}
									}
									continue;
	YES_FIND_PARTY_:// ??? ???  tc != 0 ? ??. 
									bool alreadyAdded = false;
									for (int member = 0; member < tc; ++member)
										if (ttt[member] == tt) alreadyAdded = true;
									if (!alreadyAdded && tc < MAX_PARTY_MEMBER)
										ttt[tc++] = tt;
								}		
							}

							break;
						}
					default:	continue;
					}		
				}		
			}	
		}

		a->ChangeUpAddExp(exp, d->IsNpc()); // CSD-TW-030624		
		isLevelUp(a);

		if( tc )// ???? ?? ???? ????. 
		{
			switch( tc )
			{						  	
			case 1:	partyexp = exp*65/100; break; 
			case 2:	partyexp = exp*60/100; break;
			case 3:	partyexp = exp*55/100; break;
			case 4:	partyexp = exp*50/100; break;
			case 5:	partyexp = exp*45/100; break;
			case 6:	partyexp = exp*40/100; break;
			default: goto END_;    
			}	
			
			for( i = 0 ; i < tc ; i ++)	
			{	
				switch (ttt[i]->party_select)
				{	//< CSD-TW-030624
				case 0:
				case PS_EXP:
					{ 
						ttt[i]->ChangeUpAddExp(partyexp, d->IsNpc());
						isLevelUp(ttt[i]);
						break;
					} 
				case PS_TACTIC:
					{ 
						ttt[i]->ChangeUpTacExp(5, partyexp*3);
						break;
					} 
				case PS_MAGIC_TAC:
					{
						ttt[i]->ChangeUpTacExp(3, partyexp*3);
						break;
					} 
				} //> CSD-TW-030624
			}
		}
		
END_:		
		
		return exp;
}			
//1018 zhh

extern void MakeSealStoneItem(CHARLIST* ach,CHARLIST* dch);	// 011213 LTS
extern void AddNPCControlData(int cn,int NPC_ID);		// LTS NPC BUG
extern void RemoveNPCControlData(int cn,int NPC_ID);	// LTS NPC BUG
extern void CheckNPCControlData(int cn,int NPC_ID);	// LTS NPC BUG
// YGI acer 3
extern int g_LocalWarBegin;			// 011215 LTS


//////////////////// 0812 lkh ?? //////////////////////
void killCharacter( CHARLIST *a,CHARLIST *d )
{	// ???? NULL? ?? ??
	//	if( a == NULL ) return;
	if( d == NULL ) return;
	if( d->GetServerID() == LOGIN_SERVER_CONNECT || d->GetServerID() == DB_DEMON ) return;
	
	if( d->bAlive >= DEAD_ ) return;
	
	d->bAlive		= DEAD_;	
	
	if( d->SprType != SPRITETYPE_NPC )
	{	
		d->deadcount = DEAD_PC_COUNT_;				
	}	
	else						
	{	//< CSD-040407
		switch (d->Race)
		{						
		case HUMAN:
			{
				d->deadcount = DEAD_PC_COUNT_;
				break;
			}
		case ANIMAL:
			{
				d->deadcount = DEAD_NPC_ANIMAL_COUNT_;
				break;
			}
		case SEALSTONE:
			{
				d->deadcount = DEAD_SEALSTONE_COUNT_;
				break;
			}
		default:
			{	// ??? ???? ??? ????? ??
				d->deadcount =  (d->Meat_amount > 0) ? DEAD_NPC_MONSTER_COUNT_:DEAD_NPC_HUNT_MONSTER_COUNT_;
				break;
			}
		}						
	}	//> CSD-040407
	
	if (UsesRegularMonsterRespawn(d))
	{
		d->deadcount = RegularMonsterDeathCount(
			d->Meat_amount > 0 || d->Skin_amount > 0, DEAD_NPC_COUNT_BOTTOM_);
	}

	d->SetState(CON_DEATH);
	
	if (d->IsNpc() && !d->IsTamedNpc())
	{	
		switch (d->generationtype)
		{	//< CSD-030417
		case GT_EVENT_MONSTER:
			{
				g_pAIManager->CheckDeath(d); // LTS DRAGON MODIFY
				CheckBossAndDeActiveGroupNo(d);		// LTS AI2
				break;
			}
		case GT_HUNT_MONSTER:
			{
				const int nGroup = d->JoinLocalWar;
				CRegenInfo* pRegen = g_pRegenManager->GetRegenInfo(nGroup);

				if (pRegen != NULL)
				{
					switch (d->GainedFame)
					{
					case 0: 
					case 2: pRegen->DelMember(d); break;
					case 1: pRegen->DelMaster(d); break;
					}

					pRegen->CalcRegenTime();
				}
								
				if (g_pArenaManager->IsColossusArena())
				{	//< CSD-030522
					if (a != NULL && a->IsPlayer())
					{	
						CArenaTeam* pTeam = a->GetJoinArenaTeam();

						if (pTeam == NULL)
						{
							break;
						}
							
						pTeam->IncKillCount();

						CBaseArena* pGame = a->GetJoinArenaGame();

						if (pGame == NULL)
						{
							break;
						}

						const int nTeam = pTeam->GetTeamNumber();
						const int nCount = pTeam->GetKillCount();
						pGame->SendKillCount(nTeam, nCount);
					}	
				}	//> CSD-030522
				break;
			}	
		}	//> CSD-030417
		
		if( d->Race == SEALSTONE ) 
		{
			if (a->IsPlayer()) a->GainedFame+=2;					// ???? ?? ???? ?? ???? +2???.
			Send_CMD_SEALSTONE_STATUS( d, 0 );
		} 
		else
		{
			if (d->Race==GUARDSTONE)						// ??? ??
			{
				UpdateGuardStoneStatus(a,d);						
			}
			SkillMgr.FallItemWhenNPCDie(d);
		}
		
		if (!d->ChairNum)							// ?? AI? ?? NPC? ??? ?? ??? ???? ????.		// LTS DRAGON MODIFY
		{
			int cn=d->ctrlpc;
			RemoveNPCControlData(cn,d->GetServerID());	// NPC AI ??? ???? ????.
		}
		//1020 zhh		//??? ??? ????? ??? ??? ??
	}			
	
	if (d->IsPlayer())
	{	
		ExpDown(a, d, true); // CSD-030314
		// ?? ?? ?? ??? ?? ? ??
		if (isNationWarfieldServer() && IsWar())
		{
			CountNationDeath(a, d);
		}
		if (isNewWarfieldServer() && IsWar())
		{
			CountNewWarfieldDeath(a,d);
		}
		
		CheckLocalWarDeath(a,d);			// LTS LOCALWAR
		CheckEventLocalWarDeath(a,d);		// 020115 LTS
		CheckDragonLordWarTeam(a,d);		// LTS DRAGONLORD
		
		// 020428 YGI acer 3
		if( a )
		{
			if( a->IsPlayer() || a->IsTamedNpc() )	// ??? ???? ???? IsTamedNpc() ? true ??.
			{	// ???? ???? ??? ???...
				CHARLIST *ch = 0;
				if( a->IsPlayer() )
				{
					ch = a;
				}
				else
				{ 
					if (a->IsTamedNpc())
					{
						// 020620 YGI
						CHARLIST *master = GetCharListPtr(a->GetMaster() );
						if( master && master->IsPlayer() ) ch = master;
					}
				}
				
				if( ch )
				{
					bool bRecord = true;
					
					if (ch->fight_flag == 1 &&	d->fight_flag == 1 && ch->fight_id == d->GetServerID())
					{
						bRecord = false;
					}
					
					if (isNationWarfieldServer() &&	IsWar()) 
					{
						bRecord = false;
					}
					
					if (IsArenaFight(ch))
					{	//< CSD-030506
						bRecord = false;
					}	//> CSD-030506

					if (g_LocalWarBegin &&	ch->JoinLocalWar &&	d->JoinLocalWar)
					{
						bRecord = false;
					}
					
					if( bRecord )
					{
						char pk_msg[2048] = {0,};
						sprintf( pk_msg, "pk : %s(%3d,%3d) [%s](NK:%d,%d,%d)->[%s](NK:%d,%d,%d) ( %02d.%02d %02d:%02d:%02d ) ",
							MapName, ch->MoveSx, ch->MoveSy, ch->Name,OutNK(ch,3), OutNK(ch, 4), OutNK(ch, 6), 
							d->Name,OutNK(d,3), OutNK(d, 4), OutNK(d, 6), g_mon+1, g_day,g_hour, g_min, g_sec );
						// 030506 YGI
						string strPath;

						if (g_pLogManager->GetLogPath(strPath))
						{	//< CSD-TW-030701
							char temp_path[80];
							sprintf( temp_path, "%s/pk.txt", strPath.c_str());
							SaveLogDefault(temp_path, pk_msg, 0);
						}	//> CSD-TW-030701
					}
				}
			}
		}
	}
	
	d->Hp = 0;
	d->Mana = 0;
	d->Hungry= 0;
	d->MoveLength = d->MovePathCount = 0;				
	d->cur_skillno = 0;  // ?? ??? ???????....
	
	
	//021030 YGI
	extern void CheckEventWhenKillMonster( CHARLIST *pKiller, CHARLIST *pNpc );
	if( d->IsNpc() )
	{
		CheckEventWhenKillMonster( a, d );
	}
	
	
	if( a == NULL ) return;
	if( a->GetServerID() == LOGIN_SERVER_CONNECT || a->GetServerID() == DB_DEMON ) return;
	
	// 021023 kyo npc?? ???? ?? ??? 
	if (a && d->IsNpc())
	{
		//???? ???? ???? ???? ????.
		CHARLIST *ch = a;

		if (a->IsSummonMonster())
		{
			const WORD idMaster = a->GetMaster();
			ch  = GetCharListPtr(idMaster);
		}
		
		g_QuestInMap.RunScriptCounter( ch->GetServerID(), ch->Name, d->npc_index, SCRIPT_COUNTER_TYPE__NPCKILL );
		g_QuestInMap.GiveRequitalAfterKillNpc( ch, d ); // 021106 kyo
	}
	// ?? ???? ?? ??. 
	if (a->IsSummonMonster())
	{
		char szBuffer[40];
		const WORD idMaster = a->GetMaster();
		// 020620 YGI
		LPCHARLIST pMaster = GetCharListPtr(idMaster);
		if( pMaster )
		{
			sprintf(szBuffer, "%s(%s)", a->Name, pMaster->Name);
			strcpy(d->KillerName, szBuffer);
		}
	}
	else
	{
		strcpy(d->KillerName, a->Name);
	}
	
	switch (NPC_Gen_Ref[d->SprNo].nNpcRace)
	{	//< CSD-031013					
	case HUMAN:
		{
			if (a->name_status.nation == d->name_status.nation)
			{	
				++a->killpc;
				SendServerKillPc(a->GetServerID());
			}

			break;	
		}
	case ANIMAL:
		{
			++a->killanimal;
			break;		
		}
	case NORM_MON:
		{
			if (a->SprType  != SPRITETYPE_NPC)
			{
				++a->killmon;
				SendServerKillMon(a->GetServerID()); //0209 YGI
			}
			
			break;		
		}
	}	//> CSD-031013
	// ???? ?? ??? ? ???? ???? ??? ???
	if (a && a->IsSummonMonster() && a->ctrlpc < 10000)
	{ 
		const WORD idMaster = a->GetMaster();
		LPCHARLIST pMaster = GetCharListPtr(idMaster);
		
		if (pMaster != NULL)
		{
			pMaster->SetMaster(idMaster);
			pMaster->m_xSummon.Command(FollowSummon);
		}
	} 
	// ???? ???? ??? ????? ??
	if (d && d->IsSummonMonster())
	{ 
		const WORD idMaster = d->GetMaster();
		LPCHARLIST pMaster = GetCharListPtr(idMaster);
		
		if (pMaster != NULL)
		{
			pMaster->m_xSummon.Decrease(d->GetServerID());
		}
	} 
	
	if( d && d->fight_flag )
	{
		EndManToManFight( a, d );
	}
}								

#define PAI   3.141592

BOOL ChangeDirection( DIRECTION* direction, int sx, int sy, int dx, int dy )
//int ReturnDirection( float tx1, float ty1, float tx2, float ty2 )
{			
	double t;
	
	if( dy == sy && sx == dx ) return 0;
	
	t = atan2( (float)(dy - sy) , (float)(dx - sx) );
	if(    PAI*7/8 < t && t <=  PAI + 1   )	{*direction = DIRECTION_LEFT;		return 1; }
	if(   -PAI -1    <=t && t <= -PAI*7/8 )	{*direction = DIRECTION_LEFT;		return 1; }
	
	if(   -PAI*7/8 < t && t <= -PAI*5/8 )	{*direction = DIRECTION_LEFTUP;		return 1; }
	if(   -PAI*5/8 < t && t <= -PAI*3/8 )	{*direction = DIRECTION_UP;			return 1; }
	if(   -PAI*3/8 < t && t <= -PAI  /8 )	{*direction = DIRECTION_RIGHTUP;	return 1; }
	if(   -PAI  /8 < t && t <=  PAI  /8 )	{*direction = DIRECTION_RIGHT;		return 1; }
	if(    PAI  /8 < t && t <=  PAI*3/8 )	{*direction = DIRECTION_RIGHTDOWN; 	return 1; }
	if(    PAI*3/8 < t && t <=  PAI*5/8 )	{*direction = DIRECTION_DOWN; 		return 1; }
	if(    PAI*5/8 < t && t <=  PAI*7/8 )	{*direction = DIRECTION_LEFTDOWN;	return 1; }
	
	return 0;
}		




///////////////////////// 0525 lkh ?? ///////////////////////////////
bool	YouCanBackdraw( CHARLIST *a, CHARLIST *d, int how )
{		
	DIRECTION dir;
	int x, y;
	int	t_X=0, t_Y=0;
	
	ChangeDirection( &dir, d->MoveSx, d->MoveSy, a->MoveSx, a->MoveSy );
	
	x = d->MoveSx;
	y = d->MoveSy;
	
	switch( dir )
	{	
	case DIRECTION_UP		:				t_Y=how;		break;
	case DIRECTION_RIGHTUP	:	t_X=-how;	t_Y=how;		break;
	case DIRECTION_LEFTUP	:	t_X=how;	t_Y=how;		break;
	case DIRECTION_RIGHT	:	t_X=-how;      				break;
	case DIRECTION_LEFT		:	t_X=how;      				break;	
	case DIRECTION_RIGHTDOWN:	t_X=-how;	t_Y=-how;		break;
	case DIRECTION_LEFTDOWN	:	t_X=how;	t_Y=-how;		break;
	case DIRECTION_DOWN		:				t_Y=-how;		break;
	}	
	x += t_X;
	y += t_Y;
	
	if( FreeTile( d, x, y, x, y ) )			//?? ?? ?? ?????
	{	
		d->MoveLength = d->MovePathCount = 0;
		d->MoveSx	= x;
		d->MoveSy	= y;
		d->X		= x*TILE_SIZE;
		d->Y		= y*TILE_SIZE;
		d->Tox		= t_X;
		d->Toy		= t_Y;
		
		if( d->SprType == SPRITETYPE_NPC )
		{
			SetArea( MOVE_NPC_AREA, d->GetServerID());
		}
		else
		{
			SetArea( MOVE_PC_AREA, d->GetServerID());
		}
		
		return true;
	}	
	
	d->Tox = 0;
	d->Toy = 0;
	return false;
}

inline int IsNKNation(LPCHARLIST ch)
{		
	switch (MapInfo[MapNumber].nation)
	{	
	case N_VYSEUS:
	case N_YILSE:
	case N_ZYPERN:
		{
			return MapInfo[ MapNumber].nation;
		}
	default:
		{
			switch( ch->name_status.nation )
			{
			case N_VYSEUS:
			case N_YILSE:
			case N_ZYPERN:
				{
					return ch->name_status.nation;
				}
			}

			break;
		}
	}	
	
	return 0;
}		

inline int IsHeNK( LPCHARLIST ch, int nation )
{	// ?? ?? ??? PK??? ??.
	switch (nation)
	{
	case N_VYSEUS:
	case N_ZYPERN:
	case N_YILSE:
		{
			return ch->nk[ nation];
		} 
	}

	return 0;
}	

void CheckElapsedTime_NK_Down( void )
{	//< CSD-CN-031213
	static DWORD time = 0;
	
	if( g_curr_time - time > 1000  )
	{
		time = g_curr_time;
	}	
	else 
	{
		return;
	}	

	CUserManager::HASH_USER mpUser = g_pUserManager->GetUserSet();

	for (CUserManager::ITOR_USER i = mpUser.begin(); i != mpUser.end(); ++i)
	{
		const int idUser = i->second;
		CHARLIST* pUser = CheckServerId(idUser);
		
		if (pUser != NULL)
		{
			if (pUser->nk[N_VYSEUS] > 0)
			{
				InNK(pUser, N_VYSEUS, -1);
			}

			if (pUser->nk[N_ZYPERN] > 0)
			{
				InNK(pUser, N_ZYPERN, -1);
			}

			if (pUser->nk[N_YILSE ] > 0)
			{
				InNK(pUser, N_YILSE , -1);
			}
		}
	}
}	//> CSD-CN-031213

#define MAX_NK_		30000
#define MAX_ULTRA_NK_	100
#define PK_START_LIMIT_		5
static int nCaseOfNK[3][3] = {{0,1,1},{ 4,3,3},{4,2,2}};

inline int OutNK( LPCHARLIST ch, int nation )
{	
	switch( nation )
	{
	case N_VYSEUS :	
	case N_ZYPERN : 
	case N_YILSE :  return ch->nk[ nation];  
		
	default : return ch->nk[ ch->name_status.nation ];	
	}

	return 0;
}	

void InNK( LPCHARLIST ch, int nation, short int add_nk )
{	
	if( add_nk == 0 ) return;
	if( ch->SprType == SPRITETYPE_NPC ) return;
	
	short int oldnk;
	short int *ch_nk;
	switch( nation )
	{
	case N_VYSEUS	:
	case N_ZYPERN	:
	case N_YILSE	:   oldnk = ch->nk[ nation];
		ch_nk = &ch->nk[ nation];  break;
	default			:  return;
	}
	*ch_nk += add_nk;
	
	if( *ch_nk > MAX_NK_ ) *ch_nk = MAX_NK_;
	if( *ch_nk < 0       ) *ch_nk = 0;
	
	if( *ch_nk == 0 )
	{
		switch( nation )
		{
		case N_VYSEUS :	ch->name_status.ultra_nk3 = 0;	break;
		case N_ZYPERN : ch->name_status.ultra_nk4 = 0;	break;
		case N_YILSE :  ch->name_status.ultra_nk6 = 0;	break;
		default : return;
		}
	}
	else if( *ch_nk >= MAX_ULTRA_NK_ )
	{
		switch( nation )
		{
		case N_VYSEUS :	ch->name_status.ultra_nk3 = 1;	break;
		case N_ZYPERN : ch->name_status.ultra_nk4 = 1;	break;
		case N_YILSE :  ch->name_status.ultra_nk6 = 1;	break;
		default : return;
		}
	}
	
	SendNK( ch->GetServerID(), ch->nk[ N_VYSEUS], ch->nk[ N_ZYPERN], ch->nk[ N_YILSE] );
}	


// ? ??? ?????? true? ????. 
inline bool IsWarfare( int anation, int bnation )
{
	if( anation < 0 || anation >= MAX_NATION )	return false;
	if( bnation < 0 || bnation >= MAX_NATION )	return false;
	
	switch( NationRelationStatus[ anation][ bnation] )
	{
	case NATION_RELATION_ENEMY_	: return true;
	default : return false;
	}
	
	return false;
}


int GetCaseOfNK( int cur_nation, int a_nation, int d_nation, int a_nk, int d_nk ) 
{
	bool warfare;
	int step2flag = 0, step3flag = 0;
	
	warfare = IsWarfare( cur_nation, d_nation );
	
	//step 2
	if( warfare )
	{
		step2flag = 0;
	}
	else if( d_nk )
	{
		step2flag = 2;
	}
	else
	{
		step2flag = 1;
	}
	
	//step3
	warfare = IsWarfare( cur_nation, a_nation );
	
	if( warfare )
	{
		step3flag = 0;
	}
	else if( d_nk )
	{	
		step3flag = 2;
	}	
	else
	{	
		step3flag = 1;
	}	
	
	int tcase = nCaseOfNK[ step2flag][ step3flag];
	
	return tcase;
}

#define NK_TYPE_STEAL_		0
#define NK_TYPE_KILL_		1
#define NK_TYPE_NO_KILL_	2

static int NationNkNum[5][3][3] = 
{	
	{{0, 0, 0},	{ 0, 0, 0},	{0, 0, 0}},
	{{0, 0, 0},	{ 0, -5,10},{0, 0, 0}},
	{{0, 0, 0},	{ 0, 0, 0},	{0, 0, 0}},
	{{0, 5, 5},	{ 0,20,20}, {0, 1, 1}},
	{{0, 0, 0},	{-5, 0,10},	{0, 0, 0}}
};

extern int g_isLocalWarServer;
extern int g_LocalWarBegin;

void CheckNK( int a_id, int d_id, int type )
{	//< CSD-030509
	switch (MapInfo[MapNumber].forrookie)
	{
	case 0:
	case 3: return;
	}

	LPCHARLIST attacker = GetCharListPtr(a_id);
	
	if (attacker == NULL) 
	{
		return;
	}

	LPCHARLIST defender = GetCharListPtr(d_id);

	if (defender == NULL)
	{
		return;
	}
	// ???? ??
	if (defender->GetWanted()) 
	{
		return;
	}
	// ???? ??
	if (g_LocalWarBegin && g_isLocalWarServer)
	{
		if (attacker->JoinLocalWar && defender->JoinLocalWar)
		{	
			return;
		}
	}

	if (g_pArenaManager->IsColossusArena())
	{
		if (attacker->IsJoinArenaGame() && defender->IsJoinArenaGame())
		{
			if (attacker->IsJoinArenaTeam() && defender->IsJoinArenaTeam())
			{
				return;
			}
		}
	}
	
	switch (type)
	{
	case NK_TYPE_KILL_:
	case NK_TYPE_NO_KILL_:
		{
			if( IsManToManFight( attacker, defender ) ) return;
			break;
		}
	case NK_TYPE_STEAL_:
		break;
	}
	
	int down = 0;	
	short int ano = 0, dno = 0;
	int a_nk, d_nk;	
	int cur_nation = MapInfo[ MapNumber].nation;
	int a_nation, d_nation;		
	
	a_nation =	attacker->name_status.nation;
	d_nation =	defender->name_status.nation;
	
	// Monster? ?? ??.. 
	if( d_nation == N_NONSTER ) return;
	if( defender->SprType == SPRITETYPE_NPC && type == NK_TYPE_NO_KILL_ ) return;  // 010613 KHS
	
	// ?? ??? ?? ?? NK???..
	a_nk = OutNK( attacker, cur_nation );
	d_nk = OutNK( defender, cur_nation );
	int tcase = GetCaseOfNK( cur_nation, a_nation, d_nation, a_nk, d_nk );
	int *t = NationNkNum[tcase][ type];
	
	InNK( attacker, a_nation, t[0] );
	InNK( attacker, cur_nation, t[1] );
	
	if( tcase == 3)	
	{
		if( cur_nation != d_nation)	InNK( attacker, d_nation, t[2] ); // ??? ?????? ???? ?? PK? ???? ?? ??.
	}
	else 
	{
		InNK( attacker, d_nation, t[2] );	
	}
	// ??? ????... ??? ?? NK??? ???
	if (d_nk && type == NK_TYPE_KILL_)	
	{
		InNK(defender, cur_nation, -10);
	}	
}	//> CSD-030509

//------------------------------------------------------------------------------

// ??? ?? ?? ???? 
// ??? ?? ????? ??...
// 0810 KHS
bool CanBattleArea( LPCHARLIST a, LPCHARLIST  d)
{
	// 0810 KHS  CMD_MOVE ??? ?????? d->attackedblock = 0;?? Setting??. 
	if( d->attackedblock ) return false; // ??? ??? ???? ???????? ??? ?? ???. 
	
	
	if( a->Hp <= 0 ) return false; //  ???? ???? ??. 
	if( d->Hp <= 0 ) return false; //  ???? ? ???? ??. 
	
	// ??PK // 1012 KHS
	//	if( d->name_status.ultra_pk == 1 ) return true;
	//	if( a->name_status.ultra_pk == 1 ) return false; //  ?? PK? ???? ??.  ???? ??  Check ??...
	
	if( d->IsCounselor() ) return false;
	if( d->accessory[ WT_WEAPON] == 114 ) return false; // ?? ?? ???. 
	// 030106 kyo ????
	if (a->Race != GUARD) 
	{	//< CSD-030226 : ???? ???? ??
		if (TileMap[a->MoveSx][a->MoveSy].attr_no_battle) 
		{
			return false;  // ?? no battle ??? ??? ???? ??. 
		}
		
		if (TileMap[d->MoveSx][d->MoveSy].attr_no_battle) 
		{
			return false;  // ?? no battle ??? ??? ???? ??. 
		}
	}	//> CSD-030226
	
#ifdef _NATION_APPLY_
	if( a->name_status.nation != d->name_status.nation ) return true;	// 0927 YGI
																		/*	else
																		{
																		if( d->SprType == SPRITETYPE_NPC )
																		if( d->Race == HUMAN )
																		{
																		// ?? ?? ??npc? ??? ?? ?? ??. 
																		return false;																}
																		}
	*/
	
#endif
	
	if (a->Race == GUARD)	return true;
	if (IsHeNK(d, MapInfo[MapNumber].nation) > 5)	return true;
	if (g_block_attack)	return false; // ???? ??. ?? Event??? ???? ?? ??? ??? ????? ..
	if (a->dwDontAttack > 0)	return false; //?? ???? ????
	if (a->Hungry <= 0)	return false;
	
	return true;
}



inline int BojungDamage( LPCHARLIST d, int damage )
{
	return damage;
}
//............................
void SendYouDie( int cn )
{
	t_packet p;
	
	p.h.header.type = CMD_YOU_DIE;
	p.u.you_die.id		= cn;
	p.u.you_die.t		= 10;
	p.h.header.size = sizeof( t_you_die );
	
	QueuePacket( connections, cn, &p, 1 );
	CastMe2Other( cn, &p );     
}

void RecvDebugReqCharInfo( int cn, int target )
{
	t_packet p;
	LPCHARLIST n;
	
	p.h.header.type = CMD_DEBUG_REQ_CHAR_INFO;
	p.u.debug_server_req_char_info.id = target;
	
	if( target >= 10000 )
	{
		n = &NPCList[target-10000];
	}
	else 
	{
		n = &connections[ target].chrlst;
	}
	
	p.u.debug_server_req_char_info.id = target;
	
	p.u.debug_server_req_char_info.lv = n->GetLevel(); // CSD-030806
	p.u.debug_server_req_char_info.exp   = n->Exp; 
	p.u.debug_server_req_char_info.ac    = n->Ac;
	p.u.debug_server_req_char_info.Str   = n->Str;					//  ?
	p.u.debug_server_req_char_info.Con   = n->Con;				//  ??
	p.u.debug_server_req_char_info.Dex   = n->Dex;				//	???
	p.u.debug_server_req_char_info.Wis   = n->Wis;				//	??
	p.u.debug_server_req_char_info.Int   = n->Int;				//	??
	p.u.debug_server_req_char_info.MoveP = n->MoveP;				//	???
	p.u.debug_server_req_char_info.Char  = n->Char;				//	??
	p.u.debug_server_req_char_info.Endu  = n->Endu;				//  ???
	p.u.debug_server_req_char_info.Moral = n->Moral;				//	??
	p.u.debug_server_req_char_info.Luck  = n->Luck;				//	??
	p.u.debug_server_req_char_info.wsps  = n->wsps;				//  ???->?? ?????
	p.u.debug_server_req_char_info.Hp    = n->Hp;    	
	p.u.debug_server_req_char_info.HpMax = n->HpMax;
	p.u.debug_server_req_char_info.Mp    = n->Mana;     
	p.u.debug_server_req_char_info.MpMax = n->ManaMax;
	p.u.debug_server_req_char_info.Hungry= n->Hungry; 
	p.u.debug_server_req_char_info.HungryMax = n->HungryMax;
	p.u.debug_server_req_char_info.event_no = n->eventno;			// 0131 YGI
	
	p.h.header.size = sizeof( t_debug_server_req_char_info );
	QueuePacket( connections, cn, &p, 1 );
}

//acer7
char skill_name42[][30] = 
{
	"SPRITUALITY",
		"POISONING",
		"ENTRAPMENT",
		"SCROLLING",
		"CUNNING",
		"BOMBPLAY",
		"STEALING",
		"FARMING",
		"MINING",
		"FISHING",
		"CUTDOWN",
		"BUTCHERING",
		"HERBDIGGING",
		"HERDING",
		"CARPENTRY",
		"BLACKSMITHY",
		"COOKING",
		"TAILORING",
		"BOWCRAFTING",
		"ALCHEMING",
		"CANDLEMAKING",
		"REPARING",
		"REUTILIZING",
		"MUSICIANSHIP",
		"DETECT",
		"HIDE",
		"CAMPING",
		"TAIMING",
		"COMMUNICATION",
		"ANIMAL_LORE",
		"ARMS_LORE",
		"EVALUATE_INTELIGENCE",
		"EVALUATE_WISDOM",
		"ANATOMY_JUDGEMENT",
		"ITEM_IDENTIFICATION",
		"TASTE_IDENTIFICATION",
		"APPRAISAL",
		"HEALING",
		"JUMP",
		"RIDING",
};

// acer7
void SaveBallancingSkill( LPCHARLIST a, int skillno )
{							
	FILE *fp;																							
	char temp[ FILENAME_MAX];
	
	a->balancingtest --;
	if( a->balancingtest > 0 )	return;
	a->balancingtest = 50;
	
	sprintf( temp, "./skill_log/%s.txt", a->Name );
	
	fp = fopen( temp, "at+");
	if( fp == NULL )
	{
		sprintf( temp, "./skill_log/ID_%s.txt", connections[a->GetServerID()].id );
		fp = fopen( temp, "at+" );
		if( fp == NULL ) return;
	}
	
	int mother = SkillTbl[skillno].MotherSkillType;
	fprintf( fp, "skill< %s[%d(%d)](lv%d:exp%d) >\n", skill_name42[skillno], skillno, mother, a->skillexp[mother].skillexp/10000, a->skillexp[mother].skillexp%10000 );
	fprintf( fp, "---%s(%3d,%3d) %02d.%02d %02d:%02d:%02d ---\n",  MapName, a->MoveSx, a->MoveSy, g_mon+1, g_day,g_hour, g_min, g_sec );
	fprintf( fp, "WORKER(Lv:%3d/Exp:%9d/ladder:%3d/fame:%3d)\n", a->GetLevel(), a->Exp, a->LadderScore, a->fame ); // CSD-030806
	for(int iCount = 0; iCount<45; iCount++)//?? ???
	{
		if( a->skillexp[iCount].skillexp/10000 )
		{
			fprintf(fp,"SkillExp%d	Exp	%d/%d	RareRate	%d		month	%d	day	%d	makecount	%d\n",
				iCount,a->skillexp[iCount].skillexp/10000, a->skillexp[iCount].skillexp%10000,
				a->skillexp[iCount].rare,
				((LPSKILLEXP2)(&a->skillexp[iCount]))->month,
				((LPSKILLEXP2)(&a->skillexp[iCount]))->day,
				((LPSKILLEXP2)(&a->skillexp[iCount]))->makecount) ;
		}
	}
	fprintf( fp, "\n" );
	fclose(fp);
}	

void SaveBallancing2( LPCHARLIST a, LPCHARLIST d, int magicno, int damage, int mana, int attack, int attacktype, int tac  )
{   //< CSD-030324
	if(a->IsNpc()){return;}
	if( a->balancingtest/20  == (a->balancingtest-1)/20)//20 ?? ??
	{
		return;
	}
	
	char temp[ FILENAME_MAX] = {0,};
	::sprintf( temp, "./SimpleLog/%s.txt", a->Name );
	FILE *fp = fopen( temp, "at+");
	if( NULL == fp )
	{
		sprintf( temp, "./SimpleLog/%s.txt", connections[a->GetServerID()].id );
		fp = fopen( temp, "at+" );
		if( fp == NULL ) return;
		fprintf( fp, " %s \n", a->Name );
	}
	
	if( magicno )
	{
		if (magicno >= 215)
		{
			fprintf( fp, "Combat Skill< %3d:%30s >  --> %20s   \n", magicno, Magic_Ref[magicno].Han_Name, d->Name );
		}
		else
		{
			fprintf( fp, "Magic< %3d:%30s >  --> %20s   \n", magicno, Magic_Ref[magicno].Han_Name, d->Name );
		}
		
		fprintf( fp, "---%s(%3d,%3d) %02d.%02d %02d:%02d:%02d ---\n",  MapName, a->MoveSx, a->MoveSy, g_mon+1, g_day,g_hour, g_min, g_sec );
		
		fprintf( fp, " Attack(Lv:%3d/Exp:%9d/ladder:%3d/fame:%3d)\n", a->GetLevel(), a->Exp, a->LadderScore, a->fame); // CSD-030806
		fprintf( fp, " Defence(Lv:%3d/Exp:%9d)\n", d->GetLevel(), d->Exp); // CSD-030806

		fprintf( fp, " Hp %4d/%4d ->  Hp %4d/%4d \n", a->Hp, a->HpMax, d->Hp, d->HpMax );
		fprintf( fp, " Mana %4d/%4d -> Mana %4d/%4d \n", a->Mana, a->ManaMax, d->Mana, d->ManaMax );  
		fprintf( fp, " Hungry %4d/%4d -> Hngy %4d/%4d \n", a->Hungry, a->HungryMax, d->Hungry, d->HungryMax );
		fprintf( fp, " Damage : %4d        \n", damage );
		fprintf( fp, " Used Mana: %4d        \n",   mana );

		switch(a->Spell)
		{
		case WIZARD_SPELL:
			{
				::fprintf(fp, " Magery               Tactics(%3d:%d)\n", 
					a->Skill[TACTICS_Magery], a->tac_skillEXP[11]);
			}break;
		case PRIEST_SPELL:
			{
				::fprintf(fp, " Orison               Tactics(%3d:%d)\n", 
					a->Skill[TACTICS_Orison], a->tac_skillEXP[12]);
			}break;
		}
	}	
	else
	{
		fprintf(fp, "Weapon-Attack< type=%d, id=%d > --> %s\n", attacktype, magicno, d->Name);
		fprintf( fp, "---%s(%3d,%3d) %02d.%02d %02d:%02d:%02d ---\n",  MapName, a->MoveSx, a->MoveSy, g_mon+1, g_day,g_hour, g_min, g_sec );
		
		fprintf(fp, " Attack(Lv:%3d/Exp:%9d/ladder:%3d/fame:%3d)\n", a->GetLevel(), a->Exp, a->LadderScore, a->fame); // CSD-030806
		fprintf(fp, " Defence(Lv:%3d/Exp:%9d)\n", d->GetLevel(), d->Exp); // CSD-030806
				
		const int ino = a->equip[ WT_WEAPON].item_no;
		CItem *ii = ItemUnit(ino);
		
		int itemkind = IK_NONE;

		if( ii )	{itemkind = ii->GetItemKind();}
		else		{itemkind = IK_NONE;}
		
		char *at = NULL;

		switch( itemkind )
		{			
			//	??? ??....
		case IK_DUAL_BOW:		at = "DUAL_BOW";	break;//030102 lsw Dual BOW
		case IK_SHORT_BOW	:	at = "SHORT_BOW";	break;
		case IK_TWO_ARROW	:	at = "TWO_ARROW";	break;
		case IK_DUAL_ARROW	:	at = "DUAL_ARROW";	break;//030102 lsw
		case IK_LONG_BOW	:	at = "LONG_BOW";	break;
		case IK_ARROW_POINT	:	at = "ARROW_POINT";	break;
		case IK_FEATHER		:	at = "FEATHER";	break;
			// ????.. 
		case IK_HURL		:	at = "THROWING WEAPON"; break;
			//	?????..
		default             :   at = "MELEE WEAPON";    break;
		}			
		
		char temp[ FILENAME_MAX];
		
		if( ii ) 
		{
			strcpy( temp, ii->GetHanName() );
		}
		else 
		{
			strcpy( temp, " NO WEAPON " );
		}
		fprintf( fp, " Hp %4d/%4d ->  Hp %4d/%4d \n", a->Hp, a->HpMax, d->Hp, d->HpMax );
		fprintf( fp, " Mana %4d/%4d ->Mana %4d/%4d \n", a->Mana, a->ManaMax, d->Mana, d->ManaMax );
		fprintf( fp, " Hungry %4d/%4d ->Hngy %4d/%4d \n", a->Hungry, a->HungryMax, d->Hungry, d->HungryMax );
		fprintf( fp, " Ac  %3d    ->       %3d   \n", 	a->Ac, d->Ac );
		fprintf( fp, " Damage  :    %4d     \n",   damage );
		fprintf( fp, " WEAPON :  %s( %d )  \n",    temp, a->equip[ WT_WEAPON].item_no );
		fprintf( fp, " AttackType :    %s      \n",    at );
	}
//	
	const int iTacKind = a->GetTacticsKind();// == SKILL_UNKNOWN ?? ?? ?? ???.
	switch(iTacKind)
	{
	case TACTICS_Crapple						:
		{
			::fprintf(fp, " Crapple              Tactics(%3d:%d)\n", 
				a->Skill[iTacKind], a->tac_skillEXP[0]);
		}break;
	case TACTICS_swordmanship					:
		{
			::fprintf(fp, " Swordmanship         Tactics(%3d:%d)\n", 
				a->Skill[iTacKind], a->tac_skillEXP[1]);
		}break;
	case TACTICS_Archery						:
		{
			::fprintf(fp, " Archery              Tactics(%3d:%d)\n", 
				a->Skill[iTacKind], a->tac_skillEXP[2]);
		}break;
	case TACTICS_Fencing						:
		{
			::fprintf(fp, " Fencing              Tactics(%3d:%d)\n", 
				a->Skill[iTacKind], a->tac_skillEXP[3]);
		}break;
	case TACTICS_Mace_fighting					:
		{
			::fprintf(fp, " Mace Fighting        Tactics(%3d:%d)\n", 
				a->Skill[iTacKind], a->tac_skillEXP[4]);
		}break;
	case TACTICS_Pierce							:
		{
			::fprintf(fp, " Pierce               Tactics(%3d:%d)\n", 
				a->Skill[iTacKind], a->tac_skillEXP[5]);
		}break;
	case TACTICS_Whirl							:
		{
			::fprintf(fp, " Whirl                Tactics(%3d:%d)\n", 
				a->Skill[iTacKind], a->tac_skillEXP[6]);
		}break;
	case TACTICS_Hurl							:
		{
			::fprintf(fp, " Hurl                 Tactics(%3d:%d)\n", 
				a->Skill[iTacKind], a->tac_skillEXP[7]);
		}break;
	case TACTICS_Parrying						:
		{
			::fprintf(fp, " Parrying             Tactics(%3d:%d)\n", 
				a->Skill[iTacKind], a->tac_skillEXP[8]);
		}break;
	case TACTICS_Double_Swordmanship			:
		{
			::fprintf(fp, " Double Swordmanship  Tactics(%3d:%d)\n", 
				a->Skill[iTacKind], a->tac_skillEXP[9]);
		}break;
	case TACTICS_Double_MaceFighting			:
		{
			::fprintf(fp, " Double Mace Fighting Tactics(%3d:%d)\n", 
				a->Skill[iTacKind], a->tac_skillEXP[10]);
		}break;
	case TACTICS_Magery							:
		{
			::fprintf(fp, " Magery               Tactics(%3d:%d)\n", 
				a->Skill[iTacKind], a->tac_skillEXP[11]);
		}break;
	case TACTICS_Orison							:
		{
			::fprintf(fp, " Orison               Tactics(%3d:%d)\n", 
				a->Skill[iTacKind], a->tac_skillEXP[12]);
		}break;
	default:
		{
			::fprintf(fp, " TACTIC UNKNOWN \n");
		}break;
	}
	::fprintf(fp, "\n");
	fclose(fp);
}

void SaveBallancing( LPCHARLIST a, LPCHARLIST d, int magicno, int damage, int mana, int attack, int attacktype, int tac  )
{   //< CSD-TW-030622
	if (a->IsNpc())
	{
		return;
	}

	a->balancingtest --;
	if( a->balancingtest > 0 )
	{
		::SaveBallancing2( a, d, magicno, damage, mana, attack, attacktype, tac  );
		return;
	}
	a->balancingtest = 100;
	
	string strPath;

	if (!g_pLogManager->GetLogPath(LT_BATTLE, strPath))
	{
		return;
	}
	//< CSD-040224
	FILE* fp = fopen(VA("%s/%s.txt", strPath.c_str(), a->Name), "at+");

	if (fp == NULL)
	{
		fp = fopen(VA("%s/%s.txt", strPath.c_str(), connections[a->GetServerID()].id), "at+");

		if (fp == NULL)
		{
			return;
		}
	}
	//> CSD-040224
	if( magicno )
	{
		if (magicno >= 215)
		{
			fprintf( fp, "Combat Skill< %3d:%30s >  --> %20s   \n", magicno, Magic_Ref[magicno].Han_Name, d->Name );
		}
		else
		{
			fprintf( fp, "Magic< %3d:%30s >  --> %20s   \n", magicno, Magic_Ref[magicno].Han_Name, d->Name );
		}
		
		fprintf( fp, "---%s(%3d,%3d) %02d.%02d %02d:%02d:%02d ---\n",  MapName, a->MoveSx, a->MoveSy, g_mon+1, g_day,g_hour, g_min, g_sec );
		
		fprintf(fp, " Attack(Lv:%3d/Exp:%9d/ladder:%3d/fame:%3d)\n", a->GetLevel(), a->Exp, a->LadderScore, a->fame); // CSD-030806
		fprintf(fp, " Defence(Lv:%3d/Exp:%9d)\n", d->GetLevel(), d->Exp); // CSD-030806
		fprintf( fp, " Reserved Point %3d -> Reserved Point %3d\n", a->GetReservedPoint(), d->GetReservedPoint());
		fprintf( fp, " Str   %3d    ->  Str   %3d                Hp %4d/%4d ->  Hp %4d/%4d \n", a->Str, d->Str, a->Hp, a->HpMax, d->Hp, d->HpMax );
		fprintf( fp, " Con   %3d    ->  Con   %3d              Mana %4d/%4d ->Mana %4d/%4d \n", a->Con, d->Con, a->Mana, a->ManaMax, d->Mana, d->ManaMax );  
		fprintf( fp, " Dex   %3d    ->  Dex   %3d            Hungry %4d/%4d ->Hngy %4d/%4d \n", a->Dex, d->Dex, a->Hungry, a->HungryMax, d->Hungry, d->HungryMax );
		fprintf( fp, " Wis   %3d    ->  Wis   %3d                                   \n", 	a->Wis, d->Wis );
		fprintf( fp, " Int   %3d    ->  Int   %3d                 Damage : %4d        \n", 	a->Int, d->Int, damage );
		fprintf( fp, " MoveP %3d    ->  MoveP %3d             Used Mana: %4d        \n",    a->MoveP, d->MoveP, mana );
		fprintf( fp, " Char  %3d    ->  Char  %3d                                   \n",    a->Char , d->Char );
		fprintf( fp, " Endu  %3d    ->  Endu  %3d                                   \n",    a->Endu , d->Endu ); 
		fprintf( fp, " Moral %3d    ->  Moral %3d                                   \n",    a->Moral, d->Moral); 
		fprintf( fp, " Luck  %3d    ->  Luck  %3d                                   \n",    a->Luck , d->Luck ); 
		fprintf( fp, " wsps  %3d    ->  wsps  %3d                                   \n",    a->wsps , d->wsps ); 
		
		fprintf( fp, "???? ????? \n" );
		fprintf(fp, "Resist Poison  %3d\n", d->GetBasicResist(RT_POISON));//030227 lsw
		fprintf(fp, "Resist Curse   %3d\n", d->GetBasicResist(RT_CURSE));
		fprintf(fp, "Resist Fire    %3d\n", d->GetBasicResist(RT_FIRE));
		fprintf(fp, "Resist Ice     %3d\n", d->GetBasicResist(RT_ICE));
		fprintf(fp, "Resist Elect   %3d\n", d->GetBasicResist(RT_ELECT));
		fprintf(fp, "Resist Holy    %3d\n", d->GetBasicResist(RT_HOLY));
	}	
	else
	{	
		fprintf(fp, "Weapon-Attack< type=%d, id=%d > --> %s\n", attacktype, magicno, d->Name);
		fprintf( fp, "---%s(%3d,%3d) %02d.%02d %02d:%02d:%02d ---\n",  MapName, a->MoveSx, a->MoveSy, g_mon+1, g_day,g_hour, g_min, g_sec );
		
		fprintf(fp, " Attack(Lv:%3d/Exp:%9d/ladder:%3d/fame:%3d)\n", a->GetLevel(), a->Exp, a->LadderScore, a->fame); // CSD-030806
		fprintf(fp, " Defence(Lv:%3d/Exp:%9d)\n", d->GetLevel(), d->Exp); // CSD-030806
		
		int itemkind;
		char *at;
		const int ino = a->equip[ WT_WEAPON].item_no;
		CItem *ii = ItemUnit( ino/1000, ino%1000);
		
		if( ii )	itemkind = ii->GetItemKind();
		else		itemkind = IK_NONE;
		
		switch( itemkind )
		{			
			//	??? ??....
		case IK_DUAL_BOW:		at = "DUAL_BOW";	break;//030102 lsw Dual BOW
		case IK_SHORT_BOW	:	at = "SHORT_BOW";	break;
		case IK_TWO_ARROW	:	at = "TWO_ARROW";	break;
		case IK_DUAL_ARROW	:	at = "DUAL_ARROW";	break;//030102 lsw
		case IK_LONG_BOW	:	at = "LONG_BOW";	break;
		case IK_ARROW_POINT	:	at = "ARROW_POINT";	break;
		case IK_FEATHER		:	at = "FEATHER";	break;
			// ????.. 
		case IK_HURL		:	at = "THROWING WEAPON"; break;
			//	?????..
		default             :   at = "MELEE WEAPON";    break;
		}			
		
		char temp[ FILENAME_MAX];
		
		if( ii ) 
		{
			strcpy( temp, ii->GetHanName() );
		}
		else 
		{
			strcpy( temp, " NO WEAPON " );
		}
		
		fprintf( fp, " Reserved Point %3d -> Reserved Point %3d\n", a->GetReservedPoint(), d->GetReservedPoint());
		fprintf( fp, " Str   %3d	->  Str   %3d                Hp %4d/%4d ->  Hp %4d/%4d \n", a->Str, d->Str, a->Hp, a->HpMax, d->Hp, d->HpMax );
		fprintf( fp, " Con   %3d	->  Con   %3d              Mana %4d/%4d ->Mana %4d/%4d \n", a->Con, d->Con, a->Mana, a->ManaMax, d->Mana, d->ManaMax );
		fprintf( fp, " Dex   %3d	->  Dex   %3d            Hungry %4d/%4d ->Hngy %4d/%4d \n", a->Dex, d->Dex, a->Hungry, a->HungryMax, d->Hungry, d->HungryMax );
		fprintf( fp, " Wis   %3d	->  Wis   %3d                Ac  %3d    ->       %3d   \n", 	a->Wis, d->Wis, a->Ac, d->Ac );
		fprintf( fp, " Int   %3d	->  Int   %3d                                   \n",    a->Int, d->Int);
		fprintf( fp, " MoveP %3d	->  MoveP %3d              Damage  :    %4d     \n",    a->MoveP, d->MoveP, damage );
		fprintf( fp, " Char  %3d	->  Char  %3d             WEAPON :  %s( %d )  \n",    a->Char , d->Char,  temp, a->equip[ WT_WEAPON].item_no );
		fprintf( fp, " Endu  %3d	->  Endu  %3d           AttackType :    %s      \n",    a->Endu , d->Endu,  at );
		fprintf( fp, " Moral %3d	->  Moral %3d                                   \n",    a->Moral, d->Moral );
		fprintf( fp, " Luck  %3d	->  Luck  %3d                                   \n",    a->Luck , d->Luck );
		fprintf( fp, " wsps  %3d	->  wsps  %3d                                   \n",    a->wsps , d->wsps );
	}
	
	fprintf(fp, " [Tactic Info]\n");
	fprintf(fp, " Crapple              Tactics(%3d:%d)\n", a->Skill[TACTICS_Crapple], a->tac_skillEXP[0]);
	fprintf(fp, " Swordmanship         Tactics(%3d:%d)\n", a->Skill[TACTICS_swordmanship], a->tac_skillEXP[1]);
	fprintf(fp, " Archery              Tactics(%3d:%d)\n", a->Skill[TACTICS_Archery], a->tac_skillEXP[2]);
	fprintf(fp, " Fencing              Tactics(%3d:%d)\n", a->Skill[TACTICS_Fencing], a->tac_skillEXP[3]);
	fprintf(fp, " Mace Fighting        Tactics(%3d:%d)\n", a->Skill[TACTICS_Mace_fighting], a->tac_skillEXP[4]);
	fprintf(fp, " Pierce               Tactics(%3d:%d)\n", a->Skill[TACTICS_Pierce], a->tac_skillEXP[5]);
	fprintf(fp, " Whirl                Tactics(%3d:%d)\n", a->Skill[TACTICS_Whirl], a->tac_skillEXP[6]);
	fprintf(fp, " Hurl                 Tactics(%3d:%d)\n", a->Skill[TACTICS_Hurl], a->tac_skillEXP[7]);
	fprintf(fp, " Parrying             Tactics(%3d:%d)\n", a->Skill[TACTICS_Parrying], a->tac_skillEXP[8]);
	fprintf(fp, " Double Swordmanship  Tactics(%3d:%d)\n", a->Skill[TACTICS_Double_Swordmanship], a->tac_skillEXP[9]);
	fprintf(fp, " Double Mace Fighting Tactics(%3d:%d)\n", a->Skill[TACTICS_Double_MaceFighting], a->tac_skillEXP[10]);
	fprintf(fp, " Magery               Tactics(%3d:%d)\n", a->Skill[TACTICS_Magery], a->tac_skillEXP[11]);
	fprintf(fp, " Orison               Tactics(%3d:%d)\n", a->Skill[TACTICS_Orison], a->tac_skillEXP[12]);

	fprintf(fp, " [NK Info]\n");
	fprintf(fp, "N_NEUTRAL  = %d	N_NONSTER	= %d	N_VYSEUS = %d\n", a->nk[N_NEUTRAL], a->nk[N_NONSTER], a->nk[N_VYSEUS]);
	fprintf(fp, "N_ZYPERN	= %d	N_HEGEMONIA	= %d	N_YILSE	 =%d\n", a->nk[N_ZYPERN	], a->nk[N_HEGEMONIA], a->nk[N_YILSE]);
	
	fprintf(fp, " [Party Info]\n");
	
	for (int i = 0; i < MAX_PARTY_MEMBER; ++i)
	{
		if (a->party[i].On && a->party[i].Name)
		{ 
			fprintf(fp, " [%d]%20s%s", i, a->party[i].Name, (i==2)?"\n":"  ");
		}
	}

	fprintf(fp, "\n");
	fclose(fp);
}	//> CSD-TW-030622

void SaveLevelUp(LPCHARLIST pTarget)
{	//< CSD-TW-030622
	string strPath;

	if (!g_pLogManager->GetLogPath(LT_LEVEL_UP, strPath))
	{
		return;
	}
	//< CSD-040224
	FILE* fp = fopen(VA("%s/%s.txt", strPath.c_str(), pTarget->Name), "at+");

	if (fp == NULL)
	{
		fp = fopen(VA("%s/%s.txt", strPath.c_str(), connections[pTarget->GetServerID()].id), "at+");

		if (fp == NULL)
		{
			return;
		}
	}
	//> CSD-040224
	fprintf(fp, " %04d.%02d.%02d %02d:%02d:%02d\n", g_year, g_mon + 1, g_day, g_hour, g_min, g_sec); // CSD-040202

	fprintf(fp, " [Ability Info]\n");
	fprintf(fp, " Level %3d     Exp   %3d     Step  %3d\n", pTarget->GetLevel(), pTarget->Exp, pTarget->GetExperienceStep()); // CSD-030806
	fprintf(fp, " Class %3d     Step  %3d     Dual  %3d\n", pTarget->Class, pTarget->GetClassStep(), pTarget->GetDualClass());
	fprintf(fp, " Str   %3d     Con   %3d     Dex   %3d\n", pTarget->Str, pTarget->Con, pTarget->Dex);
	fprintf(fp, " Wis   %3d     Int   %3d     MoveP %3d\n", pTarget->Wis, pTarget->Int, pTarget->MoveP);
	fprintf(fp, " Char  %3d     Endu  %3d     Moral %3d\n", pTarget->Char, pTarget->Endu, pTarget->Moral);
	fprintf(fp, " Luck  %3d     wsps  %3d     Reserved Point %3d\n", pTarget->Luck, pTarget->wsps, pTarget->GetReservedPoint());
	
	fprintf(fp, " [Tactic Info]\n");
	fprintf(fp, " Crapple              Tactics(%3d:%d)\n", pTarget->Skill[TACTICS_Crapple], pTarget->tac_skillEXP[0]);
	fprintf(fp, " Swordmanship         Tactics(%3d:%d)\n", pTarget->Skill[TACTICS_swordmanship], pTarget->tac_skillEXP[1]);
	fprintf(fp, " Archery              Tactics(%3d:%d)\n", pTarget->Skill[TACTICS_Archery], pTarget->tac_skillEXP[2]);
	fprintf(fp, " Fencing              Tactics(%3d:%d)\n", pTarget->Skill[TACTICS_Fencing], pTarget->tac_skillEXP[3]);
	fprintf(fp, " Mace Fighting        Tactics(%3d:%d)\n", pTarget->Skill[TACTICS_Mace_fighting], pTarget->tac_skillEXP[4]);
	fprintf(fp, " Pierce               Tactics(%3d:%d)\n", pTarget->Skill[TACTICS_Pierce], pTarget->tac_skillEXP[5]);
	fprintf(fp, " Whirl                Tactics(%3d:%d)\n", pTarget->Skill[TACTICS_Whirl], pTarget->tac_skillEXP[6]);
	fprintf(fp, " Hurl                 Tactics(%3d:%d)\n", pTarget->Skill[TACTICS_Hurl], pTarget->tac_skillEXP[7]);
	fprintf(fp, " Parrying             Tactics(%3d:%d)\n", pTarget->Skill[TACTICS_Parrying], pTarget->tac_skillEXP[8]);
	fprintf(fp, " Double Swordmanship  Tactics(%3d:%d)\n", pTarget->Skill[TACTICS_Double_Swordmanship], pTarget->tac_skillEXP[9]);
	fprintf(fp, " Double Mace Fighting Tactics(%3d:%d)\n", pTarget->Skill[TACTICS_Double_MaceFighting], pTarget->tac_skillEXP[10]);
	fprintf(fp, " Magery               Tactics(%3d:%d)\n", pTarget->Skill[TACTICS_Magery], pTarget->tac_skillEXP[11]);
	fprintf(fp, " Orison               Tactics(%3d:%d)\n", pTarget->Skill[TACTICS_Orison], pTarget->tac_skillEXP[12]);
	
	fprintf(fp, " [Resist Info]\n");
	fprintf(fp, " Resist Poison  %3d\n", pTarget->GetBasicResist(RT_POISON));//030227 lsw
	fprintf(fp, " Resist Curse   %3d\n", pTarget->GetBasicResist(RT_CURSE));
	fprintf(fp, " Resist Fire    %3d\n", pTarget->GetBasicResist(RT_FIRE));
	fprintf(fp, " Resist Ice     %3d\n", pTarget->GetBasicResist(RT_ICE));
	fprintf(fp, " Resist Elect   %3d\n", pTarget->GetBasicResist(RT_ELECT));
	fprintf(fp, " Resist Holy    %3d\n", pTarget->GetBasicResist(RT_HOLY));
	
	fprintf(fp, " [Combat Skill Info]\n");
	fprintf(fp, " Lightning Boom    %3d  Thunder Blow      %3d\n", pTarget->GetCombatLevel(LIGHTNING_BOOM), pTarget->GetCombatLevel(THUNDER_BLOW));
	fprintf(fp, " Lightning Shock   %3d  Thunder Strike    %3d\n", pTarget->GetCombatLevel(LIGHTNING_SHOCK), pTarget->GetCombatLevel(THUNDER_STRIKE));
	fprintf(fp, " Lightning Shield  %3d  Lightning Extreme %3d\n", pTarget->GetCombatLevel(LIGHTNING_SHIELD), pTarget->GetCombatLevel(LIGHTNING_EXTREME));
	fprintf(fp, " Blood Earth       %3d  Ground Attack     %3d\n", pTarget->GetCombatLevel(BLOOD_EARTH), pTarget->GetCombatLevel(GROUND_ATTACK));
	fprintf(fp, " Blood Will        %3d  Stone Armor       %3d\n", pTarget->GetCombatLevel(BLOOD_WILL), pTarget->GetCombatLevel(STONE_ARMOR));
	fprintf(fp, " Simunian Bosom    %3d  Earth Extreme     %3d\n", pTarget->GetCombatLevel(SIMUNIAN_BOSOM), pTarget->GetCombatLevel(EARTH_EXTREME));
	fprintf(fp, " Sword N'Roses     %3d  Multiple Fire     %3d\n", pTarget->GetCombatLevel(SWORD_N_ROSES), pTarget->GetCombatLevel(MULTIPLE_FIRE));
	fprintf(fp, " Fleet Missile     %3d  Converting Armor  %3d\n", pTarget->GetCombatLevel(FLEET_MISSILE), pTarget->GetCombatLevel(CONVERTING_ARMOR));
	fprintf(fp, " Burning Bloody	%3d  Fire Extreme      %3d\n", pTarget->GetCombatLevel(BURNING_BLOODY), pTarget->GetCombatLevel(FIRE_EXTREME)); // 030415 kyo 
	fprintf(fp, " Poisoning Nova    %3d  Cherroy Shade     %3d\n", pTarget->GetCombatLevel(POISONING_NOVA), pTarget->GetCombatLevel(CHERROY_SHADE));
	fprintf(fp, " Dark Blade        %3d  Tybern Gift       %3d\n", pTarget->GetCombatLevel(DARK_BLADE), pTarget->GetCombatLevel(TYBERN_GIFT));
	fprintf(fp, " AcuquireDark      %3d  Dark Extreme      %3d\n", pTarget->GetCombatLevel(ACUQUIRE_DARK), pTarget->GetCombatLevel(DARK_EXTREME)); // 030415 kyo 
	fprintf(fp, " Icing Blast       %3d  Critical Hiding   %3d\n", pTarget->GetCombatLevel(ICING_BLAST), pTarget->GetCombatLevel(CRITICAL_HIDING));
	fprintf(fp, " Shark Missile     %3d  Horn Of Iceberg   %3d\n", pTarget->GetCombatLevel(SHARK_MISSILE), pTarget->GetCombatLevel(HORN_OF_ICEBERG));
	fprintf(fp, " Ice Shield        %3d  Ice Extreme       %3d\n", pTarget->GetCombatLevel(ICE_SHIELD), pTarget->GetCombatLevel(ICE_EXTREME));
	fprintf(fp, " Charging          %3d  Double Attack     %3d\n", pTarget->GetCombatLevel(CHARGING), pTarget->GetCombatLevel(DOUBLE_ATTACK));
	fprintf(fp, " Whilwind          %3d  Twister           %3d\n", pTarget->GetCombatLevel(WHILWIND), pTarget->GetCombatLevel(TWISTER));
	fprintf(fp, " Gust              %3d  Wind Extreme      %3d\n", pTarget->GetCombatLevel(GUST), pTarget->GetCombatLevel(WIND_EXTREME));
	fprintf(fp, " Combat Skill Point %3d \n", pTarget->GetCombatPoint());
	
	fprintf(fp, " [NK Info]\n");
	fprintf(fp, " N_NEUTRAL %d	N_NONSTER   %d	N_VYSEUS %d\n", pTarget->nk[N_NEUTRAL], pTarget->nk[N_NONSTER], pTarget->nk[N_VYSEUS]);
	fprintf(fp, " N_ZYPERN	%d	N_HEGEMONIA	%d	N_YILSE  %d\n", pTarget->nk[N_ZYPERN], pTarget->nk[N_HEGEMONIA], pTarget->nk[N_YILSE]);
	
	fprintf(fp, " [Party Info]\n");
	for (int i = 0; i < MAX_PARTY_MEMBER; ++i)
	{
		if (pTarget->party[i].On && pTarget->party[i].Name)
		{ 
			fprintf(fp, " [%d]%20s%s", i, pTarget->party[i].Name, (i==2)?"\n":"  ");
		}
	}
	
	fprintf(fp, "\n");
	fclose(fp);
}	//> CSD-TW-030622
//----------------------------------------------------------------------------
// 010502 YGI
// ??? ??? ??
// acer5
void Recv_CMD_PLEASE_GIVE_LIFE( int cn, int type, int alive_word_type  )
{	
	LPCHARLIST ch = CheckServerId( cn );
	if( !ch ) return;
	
	bool alive_flag = false;
	
	if( type == 0 )		// ?? ??
	{
		if( ch->viewtype != VIEWTYPE_GHOST_ ) return; //????? ????..
		if( g_stop_please_give_life ) return;		// ??? ??? ??? ??. ( ????? )..
		if( ch->reporter_mode ) return;		// ???????? ????.		// 011128 YGI
		
		int ret = g_AliveMap.CheckAlive( ch->MoveSx, ch->MoveSy, MapNumber, alive_word_type );
		if( ret < 0 ) return;		// ?? ?? ?? ?? ???.
		
		if( ret == 100 )		// ?? ??.
		{
			SkillMgr.CharacterToAlive(ch);// ????.//020501 lsw
		}
		else		// ???? ??? ??? ??? ???.
		{
			char ret_alive_word_type = ret;
			SendPacketDefault( CMD_ALIVE_FAIL_RESULT, (char*)&ret_alive_word_type, sizeof( char), cn );
		}
	}
	else
	{
		//ACER5
		CrackMgr.SendHackingUser( cn, USE_GM_TOOL, "CMD_GIVE_LIFE" );
		SkillMgr.CharacterToAlive(ch, 1);// ????.//020501 lsw
	}
	return;
}

///////////////////////// 0726 lkh ?? ///////////////////////////
void	TameNpcAI( LPCHARLIST master )			//???? ?? ?? ??NPC? ??? ??
{
	LPCHARLIST	tame_Char=NULL, target	= NULL;
	int			distance, distance_Min	= 1000;
	
	for( int i = NPC_LIST_START ; i < MAX_NPC_LIST ; i ++ )	//??? NPC? ???.
	{
		if (NPCList[i].bAlive == ALIVE_ && NPCList[i].IsTamedNpc())
			if( NPCList[i].HostName[0] )
				if( !strcmp(NPCList[i].HostName, master->Name ) )
				{
					tame_Char = &NPCList[i];
					break;
				}
	}
	if(!tame_Char)	return;						//??? ??? NPC? ?? ??
	
	for( int j = NPC_LIST_START ; j < MAX_NPC_LIST ; j ++ )	//??? ??? ???.
	{
		if( NPCList[j].bAlive == ALIVE_ && tame_Char!=&NPCList[j] )
		{
			distance = sqrt( pow( (master->X-NPCList[j].X),2 ) + pow( (master->Y-NPCList[j].Y),2 ) );
			if(distance<distance_Min)
			{
				distance_Min = distance;
				target = &NPCList[j];
			}
		}
	}
	if(!target)		return;						//??? ??? ??.
	
	if( target->SprType == SPRITETYPE_NPC ) 
	{
		tame_Char->targetid = target->GetServerID() + 10000;
		SendNPC_Parameter( tame_Char->GetServerID(), NPC_PARAMETER_TARGETID, tame_Char->targetid );
	}
	else 
	{
		tame_Char->targetid = target->GetServerID();
		SendNPC_Parameter( tame_Char->GetServerID(), NPC_PARAMETER_TARGETID, tame_Char->targetid );
	}
	
	tame_Char->tame_cmd = LC_TAME_ATTACK;
	SendNPC_Parameter( tame_Char->GetServerID(), NPC_PARAMETER_TAME_CMD, LC_TAME_ATTACK );
	strcpy( tame_Char->TargetName, target->Name );
	SendTameResult( master->GetServerID(), 5 );	//TAME_ATTACK
	SendNPC_StringParameter( tame_Char->GetServerID(), NPC_PARAMETER_TARGETNAME, tame_Char->TargetName );
}
// PK? ?? ?? Flag? ???.
//void SendStartGoodBad( int id )
void SendNK( int id, int nk )
{			
	t_packet p;
	
	if( id >= 10000 ) return;
	
	p.h.header.type = CMD_START_GOODBAD;
	p.u.start_goodbad.id		= id;
	p.u.start_goodbad.nk3       = nk;
	p.h.header.size = sizeof( t_start_goodbad );
	
	QueuePacket( connections, id, &p, 1 );
	CastMe2Other( id, &p );
}			

void SendNK( int id, int nk3, int nk4, int nk6 )
{			
	t_packet p;
	
	if( id >= 10000 ) return;
	
	p.h.header.type = CMD_START_GOODBAD;
	p.u.start_goodbad.id		= id;
	p.u.start_goodbad.nk3        = nk3;
	p.u.start_goodbad.nk4        = nk4;
	p.u.start_goodbad.nk6        = nk6;
	p.h.header.size = sizeof( t_start_goodbad );
	
	QueuePacket( connections, id, &p, 1 );
	CastMe2Other( id, &p );
}

void SaveCharacterBin( char *filename, CHARLIST *ch )
{
	FILE *fp;
	char full_name[MAX_PATH];
	sprintf( full_name, "%d_%d_%d_%d_%s.bin", g_mon+1, g_day, g_hour, g_min, filename );
	fp = fopen( full_name, "wb" );
	if( !fp ) return;
	
	fwrite( ch, sizeof(CHARLIST), 1, fp );
	fclose( fp );
}
