#include "..\stdafx.h"
#include "../../Library/Shared/LegacySkillMap.h"
#include "DefaultHeader.h"
#include "Citem.h"
#include "Kein_item.h"
#include "Sealstone.h"
#include "..\HigherLayers\WeatherControl.h"//020610 lsw
#include "Scrp_exe.h"
#include "..\LowerLayers\servertable.h"	// BBD 040311

extern void SaveBallancingSkill( LPCHARLIST a, int skillno );

extern int CanLocalWarAttack(CHARLIST* pAttacker,CHARLIST* pDefencer);		// 020115 LTS 弥己悼 ,烙惑快傍悼 //020205 lsw
extern int CheckEventAttack();
extern bool isLevelUp(CHARLIST*);

t_ItemRare ItemRare[MAX_ITEM_RARE];
t_ItemGrade ItemGrade[MAX_ITEM_GRADE];
t_ItemMutantKind ItemMutantKind[MAX_ITEM_MUTANT_KIND];

static lpMAPSKILLTABLE	g_Header[8][8];
CItem_Join	item_join[ ITEM_JOIN_MAX];
t_earthpower EarthPower;
int TotalItemJoinTableCount;
int Npc_Item_Table[NPC_ITEM_TABLE_X][NPC_ITEM_TABLE_Y];	//npc啊 磷阑锭 唱棵 酒捞袍甸捞 葛咯乐绰 抛捞喉
int SkillLevelExp[ SKILL_LEVEL_EXP_TABLE_MAX_ + 1 ][ SKILL_LEVEL_EXP_TABLE_MAX_ +1 ];
int SkillExp[ SKILL_EXP_TABLE_MAX_ +1 ][SKILL_EXP_TABLE_MAX_ +1 ];
int MAX_JOB_LEVEL = 100; // CSD-040223

//<! BBD 040311
unsigned short	g_UsercountInServerSet = 1501;		// 弥檬 1501悼立捞扼绊 焊绊 矫累茄促.
float resource_rate[6] =
{
	// 悼立俊 蝶弗 磊盔 硅啦
	1.0f, 1.2f, 1.4f, 1.6f, 1.8f, 2.0f
};

//<! BBD 040322 俊捞辑侩
float resource_rate2[6] =
{
	// 悼立俊 蝶弗 磊盔 硅啦
	1.4f, 1.55f, 1.7f, 1.85f, 2.0f, 2.15f
};
//> BBD 040322

float GetResourceRate(int nCount);
//> BBD 040311

extern HDBC hDBC;
extern void SendOtherInv( int type, int other_id, t_connection c[], int cn );
extern void SendNK( int id, int nk );
extern inline int	IsHeNK(  LPCHARLIST ch, int nation  );
extern inline int	OutNK( LPCHARLIST ch, int nation );
extern void			InNK( LPCHARLIST ch, int nation, short int add_nk );
extern inline bool	IsWarfare( int anation, int bnation );
extern int			GetCaseOfNK( int cur_nation, int a_nation, int d_nation, int a_nk, int d_nk ) ;
extern void			CheckNK( int a_id, int d_id, int type  );
extern bool ExtraExp(void);
extern int	addEXP(CHARLIST *a, CHARLIST *d, int criteria, int damage );

extern void RequestUsercountToProxy();		// BBD 040311

// 030307 soto眠啊 吝赋瘤开牢瘤 魄窜茄促.
bool IsNeutralZone(int nMapNumber)
{	//< CSD-030314   
	return (MapInfo[nMapNumber].nation == 0) ? true:false; //吝赋瘤开俊辑父 档迪龙捞 啊瓷窍促.
}	//> CSD-030314
// soto End. 

int skillProb_k1( const int skillno, CItem_Join *join )
{	
	int k1 = 65;
	
	if( join ) // ItemJoinTable俊辑 k1阑 啊廉柯促.
	{
		k1 = join->k1;
	}
	else
	{
		k1 = SkillTbl[skillno].k1;
	}
	if( k1 < 0 ) k1 = 0;
	if( k1 > 100 ) k1 = 100;
	return k1;
}	
//-----------------------------------------------------
// 扁  瓷 : 扁贱 己傍涪狼 弥家蔼阑 府畔茄促. 
// 牢  磊 : 扁贱锅龋.
// 府畔蔼 : 0 < n < 100 
//-----------------------------------------------------
int skillProb_k2( const int skillno, CItem_Join *join )
{	
	int k2 = 65;
	if( join ) // ItemJoinTable俊辑 k2阑 啊廉柯促.
	{
		k2 = join->k2;
	}
	else
	{
		k2 = SkillTbl[skillno].k2;
	}
	if( k2 < 0 ) k2 = 0;
	if( k2 > 100 ) k2 = 100;
	return k2;
}	

int skillProb_needmax( const int skillno, const int target_id, CItem_Join *join )
{	
	int ret = 0;

	switch( SkillTbl[skillno].MotherSkillExp )
	{
		// 公炼扒 100%
	case 0 : ret = 100; 
		break;

		// 弊成 1 - 100
	case 1 : ret = 100;							
		break;	

		// 秦寸 酒摹袍狼 搬钦抛捞喉曼炼..
	case 2 : if( join == NULL ) ret = 0;
		else 
		{
			ret = join->skill1_maximum;	
			return ret;
		}
		break;

		// 秦寸 胶懦狼 抛捞喉曼炼.
	case 3 : ret = SkillTbl[ skillno].Ability_max;
		break;

		// 胶懦狼 怕腐磐 单捞磐 曼炼..( 抛捞怪 )
	case 4 : if( target_id >= 10000 )
			 {
				ret = NPC_Gen_Ref[NPCList[ target_id - 10000].SprNo].Taming_max;
			 }
			 else
			 {
				ret = 100;
			 }break;
		
		// 秦寸 胶懦狼 某腐磐 单捞磐 曼炼..
	case 5 : if( target_id >= 10000 )
			 {
				 ret = NPC_Gen_Ref[NPCList[ target_id-10000].SprNo].Talkanimal_max;
			 }
			 else
			 {
				ret = 100;
			 }break;
		
		// 酒捞袍狼 漂己 曼炼..版累...
	case 6 :  ret = 100;							
		break;

		// 拱绊扁 抛捞喉 曼炼.. 超矫.
	case 7 :  ret = 100;

		// 秦寸 胶懦 某腐磐 单捞磐 曼炼 ( 澜厩胶懦 ) 
	case 8 :
	case 9 :
	case 10:
	case 11:
	case 12:
	case 13:
	case 14:
	case 15:
	default :	ret = 100;
		break;
	}
	if( ret < 0 ) ret = 0;
	if( ret > 100 ) ret = 100;
	return ret;
}

int skillProb_needmin( const int skillno, const int target_id, CItem_Join *join )
{	
	int ret = 25;
	switch( SkillTbl[skillno].MotherSkillExp )
	{
		// 公炼扒 100%
	case 0 : ret = 1; 
		break;

		// 弊成 1 - 100
	case 1 : ret = 1;							
		break;	

		// 秦寸 酒摹袍狼 搬钦抛捞喉曼炼..
	case 2 : if( join == NULL ) ret = 0;
			 else ret = join->skill1_minimum;		
		break;

		// 秦寸 胶懦狼 抛捞喉曼炼.
	case 3 : ret = SkillTbl[ skillno].Ability_min;
		break;

		// 胶懦狼 怕腐磐 单捞磐 曼炼..( 抛捞怪 )
	case 4 :	if( target_id >= 10000 )
				{
					ret = NPC_Gen_Ref[NPCList[ target_id - 10000].SprNo].Taming_min;
				}
				else
				{
					ret = 1;
				}
		break;
		
		// 秦寸 胶懦狼 某腐磐 单捞磐 曼炼..
	case 5 : if( target_id >= 10000 )
			 {
				ret = NPC_Gen_Ref[NPCList[ target_id-10000].SprNo].Talkanimal_min;
			 }
			 else
			 {
				 ret = 1;
			 }
		 
		break;
		
		// 酒捞袍狼 漂己 曼炼..版累...
	case 6 :  ret = 1;							
		break;

		// 拱绊扁 抛捞喉 曼炼.. 超矫.
	case 7 :  ret = 1;

		// 秦寸 胶懦 某腐磐 单捞磐 曼炼 ( 澜厩胶懦 ) 
	case 8 :
	case 9 :
	case 10:
	case 11:
	case 12:
	case 13:
	case 14:
	case 15:
	default :	ret = 1;
		break;
	}

	if( ret < 0 ) ret = 0;
	if( ret > 100 ) ret = 100;
	return ret;
}

int ReturnSkillProbablity(	const int cn, const int skillno, const int x, const int y, 
							const int targettype, const int target_id,CItem_Join *joinitem = NULL )
{	
	if( cn >= 10000 ) return 0;
	
	const int needmin = skillProb_needmin( skillno, target_id, joinitem );//聪靛固聪肛 罐酒坷绰镑! //this2 lsw
	const int needmax = skillProb_needmax( skillno, target_id, joinitem );
	int base = needmax - needmin;
	
	if( base < 0 ) 
	{
		return 0; 
	}
	if( base <= 0 ) 
	{
		base = 1;
	}

	int mybase = connections[ cn].chrlst.skillexp[ SkillTbl[skillno].MotherSkillType].skillexp / 10000 -  needmin;

	double k = (double)mybase / (double)base;

	if(k>1) k=1;
	
	const int k1 =  skillProb_k1( skillno, joinitem );
	const int k2 =  skillProb_k2( skillno, joinitem );	
	
	int hitrate =  (int)(k * k1 + k2);
	
	if( hitrate < 0 )	hitrate = 0;
	if( hitrate > 100 ) hitrate = 100;

	AddSkillExp( cn, skillno, hitrate );

	return hitrate;
}	

void SendSkillResultFail( const int cn, const int iSkillNo, const int iWhy = 0 )
{	
	t_packet p;
	p.h.header.type = CMD_SKILL_RESULT_FAIL;
	p.u.server_skill_result_fail.skillno	= iSkillNo;
	p.u.server_skill_result_fail.why		= iWhy;
	p.h.header.size = sizeof( t_server_skill_result_fail );
	QueuePacket( connections, cn, &p, 1);
}	

void SendViewCharInfo(const int cn, int target_id, int type, DWORD data, char *killer)
{
	t_packet p;
	int len;

	p.h.header.type = CMD_VIEW_CHAR_INFO;
	p.u.view_char_info.server_id= target_id;
	p.u.view_char_info.type		= type;
	p.u.view_char_info.data		= data;
	if( killer )
	{
		if( killer[0] )	EatRearWhiteChar( killer );
		len = strlen( killer );
	}
	else len = 0;

	if( killer != NULL ) strcpy( p.u.view_char_info.killer, killer );
	else	p.u.view_char_info.killer[0] = 0;

	p.h.header.size = sizeof( t_view_char_info ) - ( 31 - len );

	QueuePacket( connections, cn, &p, 1);
}

//1013 zhh 烹唱公客 格犁俊父 阂捞 嘿绰 巴阑 官槽促
int skill_Makefire_preCheck	(const int cn, const int skillno, const int x, const int y, const int targettype, const int target_id )
{	
	
	int ret = ReturnSkillProbablity( cn, skillno, x, y, targettype, target_id );
	if( ret == 0 ) { SendSkillResultFail(cn, skillno); return 0; }

	switch( ItemList[target_id].item_no  )
	{
	case 1011	:	//	<Log>	<烹唱公>
	case 1012	:	//	<Timber>	<格犁>	
	case 1023	:	//	<Nutmeg Log>	<厚磊唱公>
	case 1024	:	//	<唱公>	<府扁促唱公>
	case 1025	:	//	<Fir Log>	<备惑唱公>
	case 1026	:	//	<Oak Log>	<惑荐府唱公>
	case 1027	:	//	<唱公>	<芭力荐唱公>
	case 1028	:	//	<Beech Log>	<呈档广唱公>
	case 1029	:	//	<Whitebirch Log>	<磊累唱公 >
	case 1030	:	//	<Nutmeg Timber>	<厚磊唱公 格犁>
	case 1031	:	//	<唱公>	<府扁促唱公 格犁>
	case 1032	:	//	<Fir Timber>	<备惑唱公 格犁>
	case 1033	:	//	<Oak Timber>	<惑荐府唱公 格犁>
	case 1034	:	//	<唱公>	<芭力荐唱公 格犁>
	case 1035	:	//	<Beech Timber>	<呈档广唱公 格犁>
	case 1036	:	//	<Whitebirch Timber>	<磊累唱公 格犁>
		break;
		
	default :
				SendSkillResultFail(cn, skillno);
				return 0;		
	} 
	
	if( ( ItemList[ target_id].attr[ IATTR_ATTR] & IA2_FIRE ) == 0 )
	{
		ItemList[ target_id].attr[ IATTR_ATTR] |= IA2_NOTMOVE;
		SetArea( CHANGE_IA2_ATTR_ITEM_AREA, target_id );
	}
				
	return 1; 
}	

void PrintSkillItemJoin( void  )
{			
	FILE *fp;
	CItem_Join *s;
	int i;
	int no;
			
	fp = fopen( "./OutPut/WhatNeededToMakeItem.txt", "wt");
	if( fp == NULL ) return;
			
	for( i = 0 ; i < TotalItemJoinTableCount ; i ++)
	{		
		s = &item_join[i];
			
		no = s->item_id;
		CItem *ii = ItemUnit( no/1000, no%1000 );
			
		if( ii == NULL ) 
		{	
			fprintf( fp, "\n %d俊 乐绰 Item篮 荤侩登瘤 臼绰 Item捞啊夸? \n\n", no ); 
			continue;
		}	
			
		fprintf( fp, "'%s(%d)'阑 掘阑妨搁 '%s(%d)扁贱捞 鞘夸.\n", ii->GetHanName(), no, 
						SkillTbl[s->need_skill1].Skill_Minute, s->need_skill1 );
		
			
		fprintf( fp, "        鞘夸茄 扁贱篮 ");
		if( s->need_skill1 )	fprintf( fp,"'%30s(%7d)'( %d - %d )\n", SkillTbl[s->need_skill1].Skill_Minute, s->need_skill1, s->skill1_minimum, s->skill1_maximum );
		if( s->need_skill2 )	fprintf( fp, "                        '%30s(%7d)'( %d - %d )\n", SkillTbl[s->need_skill2].Skill_Minute, s->need_skill2,  s->skill2_minimum, s->skill2_maximum );

		if( s->need_skill1 == 0 && s->need_skill2 == 0 ) 
		{
		    fprintf( fp, "绝嚼聪促. \n" );
		}
				
		if( s->resource1 == 0 && s->resource2 == 0 && s->resource3 == 0 && s->resource4 == 0)
		{		
			fprintf( fp, " 捞扁贱篮 酒捞袍捞 鞘夸啊 绝嚼聪促. \n" );
		}		
		else	
		{		
//			fprintf( fp, "        鞘夸茄 Item 篮  " );

			if( s->resource1 )
				if( s->resource1_amount ) 
				{
					ii = ItemUnit( s->resource1/1000, s->resource1%1000 );
					if( ii ) fprintf( fp, "        '%15s(%7d)'  %d俺 鞘夸\n", ii->GetHanName(), s->resource1, s->resource1_amount );
				}
			if( s->resource2 )
				if( s->resource2_amount ) 
				{
					ii = ItemUnit( s->resource2/1000, s->resource2%1000 );
					if( ii ) fprintf( fp, "        '%15s(%7d)'  %d俺 鞘夸\n", ii->GetHanName(), s->resource2, s->resource2_amount );
				}
			if( s->resource3 )
				if( s->resource3_amount ) 
				{
					ii = ItemUnit( s->resource3/1000, s->resource3%1000 );
					if( ii ) fprintf( fp, "        '%15s(%7d)'  %d俺 鞘夸\n", ii->GetHanName(), s->resource3, s->resource3_amount );
				}
			if( s->resource4 )
				if( s->resource4_amount ) 
				{
					ii = ItemUnit( s->resource4/1000, s->resource4%1000 );
					if( ii ) fprintf( fp, "        '%15s(%7d)'  %d俺 鞘夸\n", ii->GetHanName(), s->resource4, s->resource4_amount );
				}
		}		
		
		switch( s->Tool_kind )
		{
			case 48	:		fprintf( fp, "        鞘夸肺窍绰 窍绰 Tool 辆幅绰 : 档备 俺埃侩(48)\n" );		break;
			case 49	:		fprintf( fp, "        鞘夸肺窍绰 窍绰 Tool 辆幅绰 : 档备 眠荐侩(49)\n");		break;
			case 50	:		fprintf( fp, "        鞘夸肺窍绰 窍绰 Tool 辆幅绰 : 档备 盲堡侩(50)\n");		break;
			case 51	:		fprintf( fp, "        鞘夸肺窍绰 窍绰 Tool 辆幅绰 : 档备 力访侩(51)\n");		break;
			case 52	:		fprintf( fp, "        鞘夸肺窍绰 窍绰 Tool 辆幅绰 : 档备 超矫侩(52)\n");		break;
			case 53	:		fprintf( fp, "        鞘夸肺窍绰 窍绰 Tool 辆幅绰 : 档备 国格侩(53)\n");		break;
			case 54	:		fprintf( fp, "        鞘夸肺窍绰 窍绰 Tool 辆幅绰 : 档备 格犁啊傍侩(54)\n");	break;
			case 55	:		fprintf( fp, "        鞘夸肺窍绰 窍绰 Tool 辆幅绰 : 档备 档绵侩(55)\n");		break;
			case 56	:		fprintf( fp, "        鞘夸肺窍绰 窍绰 Tool 辆幅绰 : 档备 距檬盲笼侩(56)\n");	break;
			case 57	:		fprintf( fp, "        鞘夸肺窍绰 窍绰 Tool 辆幅绰 : 档备 距犁父甸扁侩(57)\n");	break;
			case 58	:		fprintf( fp, "        鞘夸肺窍绰 窍绰 Tool 辆幅绰 : 档备 啊困幅(58)\n");		break;
			case 59	:		fprintf( fp, "        鞘夸肺窍绰 窍绰 Tool 辆幅绰 : 档备 格傍侩(59)\n"); break;
			case 60	:		fprintf( fp, "        鞘夸肺窍绰 窍绰 Tool 辆幅绰 : 档备 枚傍侩(60)\n"); break;
			case 61	:		fprintf( fp, "        鞘夸肺窍绰 窍绰 Tool 辆幅绰 : 档备 措快绰夸府侩(捻目)(61)\n"); break;
			case 62	:		fprintf( fp, "        鞘夸肺窍绰 窍绰 Tool 辆幅绰 : 档备 结绰夸府侩(62)\n"); break;
			case 63	:		fprintf( fp, "        鞘夸肺窍绰 窍绰 Tool 辆幅绰 : 档备 官疵幅(63)\n"); break;
			case 64	:		fprintf( fp, "        鞘夸肺窍绰 窍绰 Tool 辆幅绰 : 档备 劝力累侩(64)\n"); break;
			case 65	:		fprintf( fp, "        鞘夸肺窍绰 窍绰 Tool 辆幅绰 : 档备 阂措(65)\n"); break;
			case 66	:		fprintf( fp, "        鞘夸肺窍绰 窍绰 Tool 辆幅绰 : 档备 剧檬力累侩(66)\n"); break;
			case 67	:		fprintf( fp, "        鞘夸肺窍绰 窍绰 Tool 辆幅绰 : 档备 剐吵啊傍侩(67)\n"); break;
			case 68	:		fprintf( fp, "        鞘夸肺窍绰 窍绰 Tool 辆幅绰 : 档备 抛捞怪侩(68)\n"); break;
			case 69	:		fprintf( fp, "        鞘夸肺窍绰 窍绰 Tool 辆幅绰 : 档备 弊拱幅(69)\n"); break;
			case 70	:		fprintf( fp, "        鞘夸肺窍绰 窍绰 Tool 辆幅绰 : 档备 何剿倒幅(70)\n"); break;

			default : 		fprintf( fp, "        鞘夸肺窍绰 窍绰 Tool 辆幅绰 : 绝澜..\n" ); break;
		}

		fprintf( fp, "        鞘夸茄 厘家绰  " );
		switch( s->Flatform_kind )
		{
			case 0 :  fprintf( fp, "'酒公单唱' 捞促. \n");			break;

			case 71	:		fprintf( fp, " 累诀措 侩堡肺(71)\n"); break;
			case 72	:		fprintf( fp, " 累诀措 硅撇(72)\n"); break;
			case 73	:		fprintf( fp, " 累诀措 急馆(73)\n"); break;
			case 74	:		fprintf( fp, " 累诀措 葛风(74)\n"); break;
			case 75	:		fprintf( fp, " 累诀措 拳傣(75)\n"); break;
			case 76	:		fprintf( fp, " 累诀措 矫氰包幅(76)\n"); break;
			case 77	:		fprintf( fp, " 累诀措 拱饭(77)\n"); break;
			case 78	:		fprintf( fp, " 累诀措 阂措 (78)\n"); break;

			default : fprintf( fp, "'抛捞喉蔼捞 肋给等巴鞍促.\n" );	break;
		}

		fprintf( fp, "---------------------------------------------------------------------------------------------\n" );
	}		
	fclose(fp);
}

void CItem_Join::ReadItemJoin( char *buf )
{	
	char *token;
	int *target;
	token = strtok(buf,"\t\n\r");
	if( token == NULL ) return;
	no = atoi(token);
	
	token = strtok(NULL,"\t\n\r");
	if( token == NULL ) return;
	item_id = atoi(token);
	
	token= strtok(NULL, "\t\n");
	if( token == NULL ) return;
	strcpy(item_name, token);

	token = strtok(NULL,"\t\n\r");
	if( token == NULL ) return;
	item_amount = atoi(token);
	
	token= strtok(NULL, "\t\n");
	if( token == NULL ) return;
	strcpy(han_name, token);
	
	target = &need_know;
	int ct=0;
	while( ( token= strtok(NULL, "\t\n\r")) != NULL )
	{
		*(target+ct) = atoi(token);
		ct++;
	}
}

bool LoadItemJoinTable( void )
{			
	TotalItemJoinTableCount=0;

	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	SQLLEN		cbValue;
	char		szQuerry[256];
			
	sprintf(szQuerry, "Select * From itemjoin order by `no#`" );
			
	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{		
		int i = 0;
		retCode = SQLFetch(hStmt);
		while( retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
		{	
			retCode = SQLGetData(hStmt, 1, SQL_C_LONG, &item_join[i].no,		0, &cbValue);
			retCode = SQLGetData(hStmt, 2, SQL_C_LONG, &item_join[i].item_id,	0, &cbValue);
			item_join[i].item_id/=100;
			retCode = SQLGetData(hStmt, 3, SQL_C_CHAR, &item_join[i].item_name	,31, &cbValue);
			retCode = SQLGetData(hStmt, 4, SQL_C_LONG, &item_join[i].item_amount	,0, &cbValue);
			retCode = SQLGetData(hStmt, 5, SQL_C_CHAR, &item_join[i].han_name	,31, &cbValue);
			retCode = SQLGetData(hStmt, 6, SQL_C_LONG, &item_join[i].need_know	,0, &cbValue);
			retCode = SQLGetData(hStmt, 7, SQL_C_LONG, &item_join[i].teach_place	,0, &cbValue);
			retCode = SQLGetData(hStmt, 8, SQL_C_LONG, &item_join[i].need_skill1	,0, &cbValue);
			retCode = SQLGetData(hStmt, 9, SQL_C_LONG, &item_join[i].skill1_minimum	,0, &cbValue);
			retCode = SQLGetData(hStmt,10, SQL_C_LONG, &item_join[i].skill1_maximum	,0, &cbValue);
			retCode = SQLGetData(hStmt,11, SQL_C_LONG, &item_join[i].need_skill2	,0, &cbValue);
			retCode = SQLGetData(hStmt,12, SQL_C_LONG, &item_join[i].skill2_minimum	,0, &cbValue);
			retCode = SQLGetData(hStmt,13, SQL_C_LONG, &item_join[i].skill2_maximum	,0, &cbValue);
			retCode = SQLGetData(hStmt,14, SQL_C_LONG, &item_join[i].tech_price	,0, &cbValue);
			retCode = SQLGetData(hStmt,15, SQL_C_LONG, &item_join[i].Tool_kind	,0, &cbValue);
			retCode = SQLGetData(hStmt,16, SQL_C_LONG, &item_join[i].Flatform_kind	,0, &cbValue);
			retCode = SQLGetData(hStmt,17, SQL_C_LONG, &item_join[i].resource1	,0, &cbValue);item_join[i].resource1/=100;
			retCode = SQLGetData(hStmt,18, SQL_C_LONG, &item_join[i].resource1_amount	,0, &cbValue);
			retCode = SQLGetData(hStmt,19, SQL_C_LONG, &item_join[i].resource2	,0, &cbValue);item_join[i].resource2/=100;
			retCode = SQLGetData(hStmt,20, SQL_C_LONG, &item_join[i].resource2_amount	,0, &cbValue);
			retCode = SQLGetData(hStmt,21, SQL_C_LONG, &item_join[i].resource3	,0, &cbValue);item_join[i].resource3/=100;
			retCode = SQLGetData(hStmt,22, SQL_C_LONG, &item_join[i].resource3_amount	,0, &cbValue);
			retCode = SQLGetData(hStmt,23, SQL_C_LONG, &item_join[i].resource4	,0, &cbValue);item_join[i].resource4/=100;
			retCode = SQLGetData(hStmt,24, SQL_C_LONG, &item_join[i].resource4_amount	,0, &cbValue);
			retCode = SQLGetData(hStmt,25, SQL_C_LONG, &item_join[i].Result_resource1	,0, &cbValue);
			retCode = SQLGetData(hStmt,26, SQL_C_LONG, &item_join[i].Result_resource1_amount	,0, &cbValue);
			retCode = SQLGetData(hStmt,27, SQL_C_LONG, &item_join[i].Result_resource2	,0, &cbValue);
			retCode = SQLGetData(hStmt,28, SQL_C_LONG, &item_join[i].Result_resource2_amount	,0, &cbValue);
			retCode = SQLGetData(hStmt,29, SQL_C_LONG, &item_join[i].reutilizerble	,0, &cbValue);
			retCode = SQLGetData(hStmt,30, SQL_C_LONG, &item_join[i].k1	,0, &cbValue);
			retCode = SQLGetData(hStmt,31, SQL_C_LONG, &item_join[i].k2	,0, &cbValue);
			retCode = SQLGetData(hStmt,32, SQL_C_CHAR, &item_join[i].txt	,220, &cbValue);
			TotalItemJoinTableCount= i;
			i++;
			
		retCode = SQLFetch(hStmt);
		if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
			{
			}
			
		else if( retCode == SQL_NO_DATA ) 
			{
				break;
			}
		
		else
			{
				SQLFreeStmt(hStmt, SQL_DROP);
				return 0;
			}

		}		
	TotalItemJoinTableCount= i; //010618 lsw 困俊辑 捞悼
	}			
				
	SQLFreeStmt(hStmt, SQL_DROP);
			
	PrintSkillItemJoin();
				
	return 1;	
}

bool LoadItemMutantKind()
{
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	SQLLEN		cbValue;
	char		szQuerry[256];
			
	sprintf(szQuerry, "Select * From Item_mutant_kind order by item_kind" );
			
	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{		
		int i = 0;
		retCode = SQLFetch(hStmt);
		while( retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
		{
			if(i >= MAX_ITEM_MUTANT_KIND) return false;
			int column = 0;
			retCode = SQLGetData(hStmt, ++column, SQL_C_LONG,	&ItemMutantKind[i].item_kind,	0, &cbValue);
			//< CSD-030324
			//retCode = SQLGetData(hStmt, ++column, SQL_C_CHAR,	&ItemMutantKind[i].exp_kind,	100, &cbValue);
			++column;
			//> CSD-030324
			retCode = SQLGetData(hStmt, ++column, SQL_C_LONG,	&ItemMutantKind[i].upgrade_type,	0, &cbValue);
			int count = 0;
			for(count = 0; count <4; count ++)
			{
				int iDummy = 0;
				retCode = SQLGetData(hStmt, ++column, SQL_C_LONG,	&iDummy,	0, &cbValue);
				ItemMutantKind[i].AddItem[count]= iDummy;
			}
			retCode = SQLGetData(hStmt, ++column, SQL_C_LONG,	&ItemMutantKind[i].iBuffer1,	0, &cbValue);
			retCode = SQLGetData(hStmt, ++column, SQL_C_LONG,	&ItemMutantKind[i].iBuffer2,	0, &cbValue);
			retCode = SQLGetData(hStmt, ++column, SQL_C_LONG,	&ItemMutantKind[i].iBuffer3,	0, &cbValue);
			retCode = SQLGetData(hStmt, ++column, SQL_C_LONG,	&ItemMutantKind[i].iBuffer4,	0, &cbValue);

			i++;
			retCode = SQLFetch(hStmt);
			if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
			{
			}
			else if( retCode == SQL_NO_DATA ) 
			{
				break;
			}
			else
			{
				SQLFreeStmt(hStmt, SQL_DROP);
				return false;
			}
		}	
	}		
	else 
	{
		SQLFreeStmt(hStmt, SQL_DROP);
		return 0;
	}
	SQLFreeStmt(hStmt, SQL_DROP);
	return true;	
}

bool LoadItemGrade()
{
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	SQLLEN		cbValue;
			
	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)"Select * From Item_grade order by grade", SQL_NTS);
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{		
		int i = 0;
		retCode = SQLFetch(hStmt);
		while( retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
		{
			if(i >= MAX_ITEM_GRADE) return false;
/*
		typedef struct ITEMGRADE
		{
			int iGrade;
			int iAble;
			int ResourceAmount[4];
			int iBuffer1;
			int iBuffer2;
			int iBuffer3;
			int iBuffer4;
		}t_ItemGrade,*LPITEM_GRADE;
*/
			int column = 0;
			retCode = SQLGetData(hStmt, ++column, SQL_C_LONG,	&ItemGrade[i].iGrade,	0, &cbValue);
			retCode = SQLGetData(hStmt, ++column, SQL_C_LONG,	&ItemGrade[i].iAble,	0, &cbValue);
			int count = 0;
			for(count = 0; count <4; count ++)
			{
				int iDummy = 0;
				retCode = SQLGetData(hStmt, ++column, SQL_C_LONG,	&iDummy,	0, &cbValue);
				ItemGrade[i].ResourceAmount[count]= iDummy;
			}
			retCode = SQLGetData(hStmt, ++column, SQL_C_LONG,	&ItemGrade[i].iBuffer1,	0, &cbValue);
			retCode = SQLGetData(hStmt, ++column, SQL_C_LONG,	&ItemGrade[i].iBuffer2,	0, &cbValue);
			retCode = SQLGetData(hStmt, ++column, SQL_C_LONG,	&ItemGrade[i].iBuffer3,	0, &cbValue);
			retCode = SQLGetData(hStmt, ++column, SQL_C_LONG,	&ItemGrade[i].iBuffer4,	0, &cbValue);

			i++;
			retCode = SQLFetch(hStmt);
			if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
			{
			}
			else if( retCode == SQL_NO_DATA ) 
			{
				break;
			}
			else
			{
				SQLFreeStmt(hStmt, SQL_DROP);
				return false;
			}
		}	
	}
	else 
	{
		SQLFreeStmt(hStmt, SQL_DROP);
		return 0;
	}
	SQLFreeStmt(hStmt, SQL_DROP);
	return true;	
}

bool LoadItemRare( void )
{
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	SQLLEN		cbValue;
			
	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)"Select * From Item_rare order by `NO`", SQL_NTS);
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{		
		int i = 0;
		retCode = SQLFetch(hStmt);
		while( retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
		{
			if(i >= MAX_ITEM_RARE) return false;
/*
			typedef struct ITEM_MUTANT
			{
				int		iNo		;
				int		iType	;
				char	EngName[100];
				char	LocalName[100];
				char	Explain[100];
				char	ExplainAdd[100];
				int		iExpMark;// + - 扁龋 钎矫
				int		iMakeAble;//父甸绢 瘤绰啊?
				int		RaisePro[8];// 0锅篮 DefaultRaisePro
				int		iUpgradeAble;
				int		iAbleNation;//
				int		GRADE[16];//0锅篮 MAX 唱赣瘤绰 弊饭捞靛喊 蔼
				int		Buffer1;
				int		Buffer2;
				int		Buffer3;
				int		Buffer4;
				int		Buffer5;
				int		Buffer6;
			}t_Item_Mutant,*LPITEM_MUTANT;
*/
			int column = 0;
			retCode = SQLGetData(hStmt, ++column, SQL_C_LONG,	&ItemRare[i].iNo,	0, &cbValue);
			retCode = SQLGetData(hStmt, ++column, SQL_C_LONG,	&ItemRare[i].iType,	0, &cbValue);
			retCode = SQLGetData(hStmt, ++column, SQL_C_CHAR,	&ItemRare[i].EngName,	100, &cbValue);
			retCode = SQLGetData(hStmt, ++column, SQL_C_CHAR,	&ItemRare[i].LocalName,	100, &cbValue);
			retCode = SQLGetData(hStmt, ++column, SQL_C_CHAR,	&ItemRare[i].Explain,	100, &cbValue);
			retCode = SQLGetData(hStmt, ++column, SQL_C_CHAR,	&ItemRare[i].ExplainAdd,	100, &cbValue);
			retCode = SQLGetData(hStmt, ++column, SQL_C_LONG,	&ItemRare[i].iExpMark,	0, &cbValue);
			retCode = SQLGetData(hStmt, ++column, SQL_C_LONG,	&ItemRare[i].iMakeAble,	0, &cbValue);
			int count = 0;
			for(count = 0; count <8; count ++)
			{
				int iDummy = 0;
				retCode = SQLGetData(hStmt, ++column, SQL_C_LONG,	&iDummy,	0, &cbValue);
				ItemRare[i].RaisePro[count]= iDummy;
			}
			retCode = SQLGetData(hStmt, ++column, SQL_C_LONG,	&ItemRare[i].iUpgradeAble,	0, &cbValue);
			retCode = SQLGetData(hStmt, ++column, SQL_C_LONG,	&ItemRare[i].iAbleNation,	0, &cbValue);
							
			for(count= 0; count <16; count ++)
			{
				int iDummy = 0;
				retCode = SQLGetData(hStmt, ++column, SQL_C_LONG,	&iDummy,	0, &cbValue);
				ItemRare[i].GRADE[count]= iDummy;
			}

			i++;
			retCode = SQLFetch(hStmt);
			if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
			{
			}
			else if( retCode == SQL_NO_DATA ) 
			{
				break;
			}
			else
			{
				SQLFreeStmt(hStmt, SQL_DROP);
				return false;
			}
		}	
	}		
	else 
	{
		SQLFreeStmt(hStmt, SQL_DROP);
		return 0;
	}
	return true;	
}

bool LoadItemTable( void )//020815 lsw
{	//< CSD-038006
	if( !LoadItemRare() )			{return false;}
	if( !LoadItemMutantKind()	)	{return false;}
	if( !LoadItemGrade()		)	{return false;}
	if( !LoadRaregroup()		)	{return false;}
	if( !LoadItemFunctionGroup())	{return false;}
	if( !LoadRareProbability()	)	{return false;}
	if( !LoadItemFunction()		)	{return false;}
	if( !LoadLearnItemConvetrer())	{return false;}
	if( !LoadRareItemBag())			{return false;}//020505 lsw
	if( !LoadItemControlPercent())	{return false;}
	if( !LoadItemFallPercent())		{return false;}
	if( !LoadBasicItemBag())		{return false;}
	if( !LoadItemControl())			{return false;}
	if( !RareEM.LoadItem_Multi_Rare_Table()){return false;}//021111 lsw 
	if( !RareEM.LoadItem_Legend_Table()){return false;}//021111 lsw 
	if( !RareEM.LoadItem_Legend_Bonus_Item()){return false;}//021111 lsw 
	
	return true;
}	//> CSD-038006

bool youvegotitInventory(const int cn, ItemAttr *item )
{
	CHARLIST *ch = GetCharListPtr(cn);
	int  i,j,m;
	if( item == NULL ) return false;
	CItem *ii = ItemUnit( item->item_no/1000, item->item_no % 1000 );	// 弊繁 Item捞 Table俊 乐绰啊?
	if( ii == NULL ) return false;

	ItemAttr *tempitem;

	for( i = 0 ; i < 3 ; i ++)
		for( j = 0 ; j < 3 ; j ++)
			for( m = 0 ; m < 8 ; m ++ )
			{
				tempitem = &ch->inv[i][j][m];
				if( tempitem->item_no  == item->item_no	&& 
					tempitem->attr[ 0] == item->attr[0] &&
					tempitem->attr[ 1] == item->attr[1] &&
					tempitem->attr[ 2] == item->attr[2] &&
					tempitem->attr[ 3] == item->attr[3] &&
					tempitem->attr[ 4] == item->attr[4] &&
					tempitem->attr[ 5] == item->attr[5]  )
				{
					// amount ..
					return true;
				}
			}

	for( i = 0 ; i < 8 ; i ++)
	{
		tempitem = &ch->equip[i];
		if( tempitem->item_no  == item->item_no	&& 
			tempitem->attr[ 0] == item->attr[0] &&
			tempitem->attr[ 1] == item->attr[1] &&
			tempitem->attr[ 2] == item->attr[2] &&
			tempitem->attr[ 3] == item->attr[3] &&
			tempitem->attr[ 4] == item->attr[4] &&
			tempitem->attr[ 5] == item->attr[5]  )
		{
					// amount ..
				return true;
		}
	}

	for( i = 0 ;  i < 7 ; i ++)
	{
		tempitem = &ch->quick[i];
		if( tempitem->item_no  == item->item_no	&& 
			tempitem->attr[ 0] == item->attr[0] &&
			tempitem->attr[ 1] == item->attr[1] &&
			tempitem->attr[ 2] == item->attr[2] &&
			tempitem->attr[ 3] == item->attr[3] &&
			tempitem->attr[ 4] == item->attr[4] &&
			tempitem->attr[ 5] == item->attr[5]  )
		{
					// amount ..
				return true;
		}
	}

	tempitem = &ch->handheld;
	if( tempitem->item_no  == item->item_no	&& 
		tempitem->attr[ 0] == item->attr[0] &&
		tempitem->attr[ 1] == item->attr[1] &&
		tempitem->attr[ 2] == item->attr[2] &&
		tempitem->attr[ 3] == item->attr[3] &&
		tempitem->attr[ 4] == item->attr[4] &&
		tempitem->attr[ 5] == item->attr[5]  )
	{
				// amount ..
			return true;
	}
	
	return false;
}

bool youvegotitInventory(const int cn, int target_id )
{
	CHARLIST *ch = GetCharListPtr(cn);
	if( !ch ) return false;	// 020620 YGI
	int  i,j,m;

	for( i = 0 ; i < 3 ; i ++)
		for( j = 0 ; j < 3 ; j ++)
			for( m = 0 ; m < 8 ; m ++ )
				if( ch->inv[i][j][m].item_no == target_id ) 
				{
					return true;
				}

	for( i = 0 ; i < 8 ; i ++)
		if( ch->equip[i].item_no == target_id )	
				{
					return true;
				}
	for( i = 0 ;  i < 7 ; i ++)
		if( ch->quick[i].item_no == target_id ) 
				{
					return true;
				}

	if( ch->handheld.item_no == target_id ) 
				{
					return true;
				}
	
	return false;
}
			
CItem_Join *ReturnItemJoinTable(const int cn, int skillno, int target_id )
{			
	int i;	
	for( i = 0 ; i < TotalItemJoinTableCount ; i ++)
	{		
		if( item_join[i].item_id == target_id && item_join[i].need_skill1 == skillno )
		{		
			return 	&item_join[i];
		}	
	}		
			
	return NULL;
}			
	
int AddSkillExp2Exp(const int cn, int skillno, int hitrate )
{										
	int fix;					
	int inc;						
	int lev;					
	CHARLIST *ch;				
								
	ch = &connections[ cn].chrlst;
								
	int kind = SkillTbl[skillno].MotherSkillType;		// 0527 YGI
	lev = ch->skillexp[kind].skillexp / 10000;
	if( lev < 1 )	lev = 1;	
	if( lev > 100 ) lev = 100;	
								
	//1010 zhh	版氰摹 抛捞喉俊	辑 佬绢 甸咯辑 贸府茄促.捞固 抛捞喉阑 佬绢 甸捞绊 乐瘤父 老窜 捞吧肺 绊媚敌促. 货肺款 抛捞喉捞扼绊
	//积阿等促.					
	inc = SkillExp[hitrate][lev];

								
	fix = SkillTbl[ skillno].Ability_Fix;  // 0 <= fix <= SKILL_ABILITY_FIX_
	inc = inc * fix / SKILL_EXP_FIX_;
  
	ch->ChangeUpAddExp(inc); // CSD-TW-030624
	isLevelUp(ch);
	return 1;					
}								
								
//-------------------------------------------------------------------------------------------------------------------------
// 扁瓷 :  
// 牢  磊 : 0 <= hitrate <= 100
// 府畔蔼 : 
//-------------------------------------------------------------------------------------------------------------------------
int AddSkillExp(const int cn, int skillno, int hitrate )
{	
	int expadd;
	int fix;
	bool myjob =false;

	switch( connections[ cn].chrlst.Job )
	{
	case J_FAMER		:	 if( SkillTbl[skillno].Select_Farmer		== 1 )		 myjob = true;		break;
	case J_MINNER		:	 if( SkillTbl[skillno].Select_miner			== 1 )		 myjob = true;		break;
	case J_FISHER		:	 if( SkillTbl[skillno].Select_fisher		== 1 )		 myjob = true;		break;
	case J_LUMBERJACK	:	 if( SkillTbl[skillno].Select_Lumberjack	== 1 )		 myjob = true;		break;
	case J_BUTCHER		:	 if( SkillTbl[skillno].Select_Butcher		== 1 )		 myjob = true;		break;
	case J_HERB_DIGGER	:	 if( SkillTbl[skillno].Select_Herbdigger	== 1 )		 myjob = true;		break;
	case J_HERDSMAN		:	 if( SkillTbl[skillno].Select_herdman		== 1 )		 myjob = true;		break;
	case J_CARPENTER	:	 if( SkillTbl[skillno].Select_Carpenter		== 1 )		 myjob = true;		break;
	case J_BLACKSMITH	:	 if( SkillTbl[skillno].Select_Blacksmith	== 1 )		 myjob = true;		break;
	case J_COOKER		:	 if( SkillTbl[skillno].Select_Cooking		== 1 )		 myjob = true;		break;
	case J_TAILOR		:	 if( SkillTbl[skillno].Select_Tailoring		== 1 )		 myjob = true;		break;
	case J_BOWCRAFT		:	 if( SkillTbl[skillno].Select_Bowcraft_Fletcher == 1 )	 myjob = true;		break;
	case J_ALCHEMIST	:	 if( SkillTbl[skillno].Select_Alchemy		== 1 )		 myjob = true;		break;
	case J_CANDLEMAKER	:	 if( SkillTbl[skillno].Select_Candlemaker	== 1 )		 myjob = true;		break;
	case J_MERCHANT		:	 if( SkillTbl[skillno].Select_Merchant		== 1 )		 myjob = true;		break;
	}

	if( myjob == true ) // 瘤陛 荤侩窍绰 扁贱捞...
	{}
	
	fix = SkillTbl[ skillno].Ability_Fix;  // 0 <= fix <= SKILL_ABILITY_FIX_
	
	//1010 zhh		胶懦饭骇苞 犬伏俊 蝶扼辑 掘绢瘤绰 版氰摹啊 促福促.
	int skilllevel=connections[ cn].chrlst.skillexp[SkillTbl[skillno].MotherSkillType].skillexp/10000;
	if(skilllevel<0)	skilllevel=0;		//救傈厘摹甸
	if(skilllevel>100)	skilllevel=100;		//救傈厘摹甸
	if(hitrate<0)		hitrate=0;			//救傈厘摹甸
	if(hitrate>100)		hitrate=100;		//救傈厘摹甸
	expadd = fix * SkillLevelExp[hitrate][skilllevel];	
//	expadd = fix * ( 1 +(100 - hitrate)*(100 - hitrate) / 30 );  // 0607 
	//1010 zhh		胶懦饭骇苞 犬伏俊 蝶扼辑 掘绢瘤绰 版氰摹啊 促福促.咯扁鳖瘤

	expadd /= SKILL_ABILITY_FIX_;
	
	int inc = SkillTbl[skillno].MotherSkillType;										// 0527 YGI
	//001214 zhh	//酒贰俊 乐带 版氰摹 林绰 风凭阑 困肺 缠绢 棵啡促.胶懦 版氰摹绰 救棵扼档 版氰摹绰 坷弗档废 荐沥.
	AddSkillExp2Exp( cn, skillno, hitrate );

	if (connections[cn].chrlst.skillexp[inc].skillexp >= MAX_JOB_LEVEL*10000)
	{	//< CSD-040223
		return 1;
	}	//> CSD-040223

	//001214 zhh	//gm老 版快绰 版氰摹甫 憋栏搁 救等促.
	//011012 lsw >
	if( connections[ cn].chrlst.skillexp[ inc].skillexp > 999999 ) 
	{
	}
	else
	{
		connections[ cn].chrlst.skillexp[ inc].skillexp += expadd;
		if( connections[ cn].chrlst.skillexp[ inc].skillexp > 999999 ) 
		{
			connections[ cn].chrlst.skillexp[ inc].skillexp = 999999;
			return 1;
		}
	}

	//011022 lsw >
	SkillMgr.SendSkillExp( inc, connections[ cn].chrlst.skillexp[inc], cn);
	//011022 lsw <
	
	return 1;
}	
/**************************************************************************************************
	
  SkillTbl[skillno].MotherSkillExp : 己傍伏 拌魂 狼 屈怕 
	
(2000.6.3 16:25)
	
			(唱狼 瓷仿(mybase/10000) - 弥家摹(needmin))
 己傍伏 = ----------------------------------------------   *  k1  +  K2
			 (弥措摹(needmax)    -    弥家摹(needmin))
	
	mybase = connections[ cn].chrlst.skillexp[ 45 ];   ( 0 =< n< 1,000,000 ) 
			-->版氰摹肺 拌魂窍绊 Level阑 免仿且锭,  / 10000栏肺 茄促.


***************************************************************************************************/

int ReturnTamingPoint(const int cn , int skillno, int target_id, CItem_Join *joinitem = NULL )
{	
	int tame_add;
	int base, mybase;
	int needmin, needmax;
	int k1,k2;
	
	if( cn >= 10000 ) return 0;
	
	needmin = skillProb_needmin( skillno, target_id, joinitem );
	needmax = skillProb_needmax( skillno, target_id, joinitem );
	base = needmax - needmin;
	
	if( base < 0 ) return 0;	// 俊矾八免风凭..
	if( base <= 0 ) base = 1;	// min == max牢版快 ?

	mybase = connections[ cn].chrlst.skillexp[ SkillTbl[skillno].MotherSkillType].skillexp / 10000 -  needmin;

	double k = (double)mybase / (double)base;
	
	k1 =  skillProb_k1( skillno, joinitem );
	k2 =  skillProb_k2( skillno, joinitem );	
	
	int t = k * k1 + k2;
	if( t > 100 ) t = 100;

	tame_add=  SkillTbl[ skillno].Produce_Fix * t / 10000;
	return tame_add;
}	

int ReturnSkillProbablityNoExpAdd(const int cn, const int skillno, const int x, const int y, const int targettype, const int target_id, CItem_Join *joinitem = NULL )
{	
	int hitrate;
	int base, mybase;
	int needmin, needmax;
	int k1,k2;
	
	if( cn >= 10000 ) return 0;
	
	needmin = skillProb_needmin( skillno, target_id, joinitem );
	needmax = skillProb_needmax( skillno, target_id, joinitem );
	base = needmax - needmin;
	
	if( base < 0 ) return 0; 
	if( base <= 0 ) base = 1;

	mybase = connections[ cn].chrlst.skillexp[ SkillTbl[skillno].MotherSkillType].skillexp / 10000 -  needmin;

	double k = (double)mybase / (double)base;

	if(k>1)	k=1;
	
	k1 =  skillProb_k1( skillno, joinitem );
	k2 =  skillProb_k2( skillno, joinitem );


	hitrate =  (int)(k * k1 + k2);
	
	if( hitrate < 0 )	hitrate = 0;
	if( hitrate > 100 ) hitrate = 400;//测试分享经验100以后就没经验
	
	return hitrate;
}	
	
void SkillTargetTypeItemMake(const int cn, t_client_skill_info *sp)
{	
	t_packet p;
	ItemAttr item = {0,};
	CItem_Join join;

	CHARLIST *ch= GetCharListPtr(cn);
	if( !ch ) return;	// 020620 YGI

	unsigned int iAttr3		= 0;
	int RareType		= 0;
	int SuccessMuch		= 0;//己傍 荐樊
	int iTryRare		= sp->s.p1;
	int iRareGroup		= sp->s.p2;
	int iTryMakeCount	= sp->s.p3;

	if( iTryMakeCount > 10 )
	{
		return;
	}

	const int  skillno		= sp->skillno;
	const int  x			= sp->x;
	const int  y			= sp->y;
	const int  targettype	= sp->targettype;
	const int  target_id	= sp->target_id;

	const int check = SkillMgr.CheckYouCanSkillIt( cn, join, skillno, x, y, target_id );//鉴辑 林狼.. //咯扁辑 Join阑 函版 矫难霖促
	const int hitrate = ReturnSkillProbablityNoExpAdd( cn, skillno, x, y, targettype, target_id, &join );

	switch(iTryRare)
	{
	case 1:
		{
			iTryMakeCount = 1;//饭绢 矫档甫 窍搁 矫档肮荐绰 1捞 等促
		}break;
	case 21:
	case 31:
	case 41:
		{
		}break;
	default://饭绢 矫档啊 酒聪搁 弊缝俊 蔼甸绢客档 瘤况 滚赴促
		{
			iRareGroup =0;
		}break;
	}

	switch( check )// 扁贱阑 镜荐 乐绰 惑炔牢瘤甫 眉农茄促. 
	{
	case ITEMMAKE_YOUCAN:{break;}//唱绰 捞 酒捞袍阑 父甸 荐 乐促!
	default://父甸 荐 绝促 
		{
			SendSkillResultFail( cn, skillno, check );
			return;
		}
		break;
	}

//	DecreaseEquipDuration(ch,(rand()%6+1)*iTryMakeCount,WT_WEAPON,0,false);//胶懦砒 郴备仿 皑家 //矫档冉荐 父怒

	//矫埃 眉农 1俺寸 1.5 檬 掉饭捞甫 冈绰促
	if( global_time < ch->dwSkillMakeitemTime )	{return;}
	else{	ch->dwSkillMakeitemTime = global_time+iTryMakeCount*1500;}

	for(int count = 0  ; count < iTryMakeCount ; count++)
	{
		int r = (rand()%100)+1;	// 己傍犬伏阑 拌魂茄促. 
		if( r < hitrate ) 	{ 	SuccessMuch++;	}//己傍茄 肮荐 备茄促
	}
	
	int	amount[4]	={	join.resource1_amount,join.resource2_amount,join.resource3_amount,join.resource4_amount	};
	int	resource[4]	={	join.resource1,join.resource2,join.resource3,join.resource4	};
	int	resultresource[2]	={	join.Result_resource1,	join.Result_resource2	};

	// ItemJoin抛捞喉俊 乐绰 搬苞蔼措肺 贸府
	for ( int cou = 0 ; cou	< iTryMakeCount; cou++ ) //府家胶 瘤况林绊 捍 鞍篮芭 冻迸 林扁
	{//府家胶绰 矫档茄 冉荐父怒 瘤款促
		HungryMuchAdd( ch , HT_SKILL );//020214 lsw
		int roll = 0; 
		for(; roll < 4; roll++)
		{
			for( int i = 0 ; i < amount[roll] ; i ++)
			{
				if (!DeleteMakeItemResource(ch, resource[roll]))
				{
					SendSkillResultFail( cn, skillno, ITEMMAKE_NOT_RESOURCE);
					return;
				}
				else//1 2 父 乐绰单
				{
					if( 0 < roll && 2 > roll ) //炼牢 1 2 父 搬苞 府家胶啊 粮犁 
					{
						if( resultresource[roll])
						{
						item = GenerateItem( resultresource[roll] );
						DropItem(	ch->X + (rand()%40) - 40, ch->Y + (rand()%40) - 40, &item);
						}
					}
				}
			}
		}
		AddSkillExp( cn, skillno, hitrate );//版氰摹 咯矾锅 林扁
	}
	
	//钙胶 锭巩俊 给父甸搁 酒捞袍 救 荤扼瘤绊,
	RareType = TryRareItem(cn,skillno,join,iTryRare,iRareGroup,iTryMakeCount,SuccessMuch);//饭绢 鸥涝 罐酒坷扁

	if( !SuccessMuch )//角菩 皋技瘤
	{	
		RareType = 0;
		SendSkillResultFail( cn, skillno ); 
		if(iTryRare)
		{
			OutMessage(cn,2,259);//酒捞袍捞 角菩窍咯 父甸荐 绝澜
			return;
		}
		else
		{
			OutMessage(cn,2,260);//酒捞袍捞 角菩窍咯 父甸荐 绝澜
		}
	}

	if(iTryRare && RareType && SuccessMuch)
	{	
		if(iTryRare/10)
		{
			ItemMgr.MakeRareAttr(iAttr3,1,RareType,0,0,0,true);	
		}
		else
		{
			ItemMgr.MakeRareAttr(iAttr3,1,RareType,0,0,0,false);	
		}
	}

	if(sp->s.p1)
	{
		if(!RareType)
		{
			SendSkillResultFail( cn, skillno );
	 		return;
		}
	}

	for ( int iResultCount = 0 ; iResultCount< SuccessMuch; iResultCount++ ) 			
	{	// 扁贱狼 搬苞拱阑 积己矫挪促. 

		CItem *ii = ItemUnit( target_id );
		if( !ii ){return;}
		int iAmount = 0;
		if( ii->GetRbutton() == DIVIDE_ITEM )
		{	iAmount  = 1;	}
		else 
		{	iAmount = join.item_amount;	}

		for(int c = 0; c< iAmount;c++)
		{
			item = GenerateItem( target_id );//010618 lsw

			//011013 lsw <
			if(iAttr3)
			{
				ItemAttr *inv = SearchInv(ch);
				item.attr[IATTR_RARE_MAIN] = iAttr3;
				if(ii->GetRbutton() == DIVIDE_ITEM )
				{//酒捞袍 肺弊 巢扁扁 
					Send_RareItemMakeLog( cn, join.item_id, 
					((t_skillexp2*)&ch->skillexp[SkillTbl[skillno].MotherSkillType-14])->makecount
					,1, RareType,0,0,0,0,0,0,item.attr[IATTR_LIMIT],
					item.attr[IATTR_DURATION],0,100);
				}
				else
				{
					Send_RareItemMakeLog( cn, join.item_id, 
					((t_skillexp2*)&ch->skillexp[SkillTbl[skillno].MotherSkillType-14])->makecount
					,1, RareType,0,0,0,0,0,0,item.attr[IATTR_LIMIT],
					LOWORD(item.attr[IATTR_DURATION]),HIWORD(item.attr[IATTR_DURATION]),100);
				}
				
				if(inv)//牢亥配府啊 厚菌栏搁 
				{
					*inv = item;
					SendItemEventLog( inv, cn, SN_NOT_USER, SILT_MAKE_BY_SKILL, 1 ); //YGI acer
					SendServerEachItem( inv,ch);
					OutMessage(cn,2,524);
					continue;//酒捞袍阑 持绢 玲栏聪 官蹿俊绰 酒捞袍阑 冻绢哆府瘤 臼绰促 
				}
			}
			SendItemEventLog( &item, cn, SN_NOT_USER, SILT_MAKE_TO_GROUND_BY_SKILL, 1 ); //YGI acer	// 蝶肺 肺弊 鸥涝阑 沥狼茄促.
			DropItem(	ch->X + (rand()%100) - 50, ch->Y + (rand()%100) - 50, &item);
		}
	}

	if(SuccessMuch)//己傍 肮荐 绝栏搁 救焊晨//020603 lsw
	{
		p.h.header.type = CMD_SKILL_ITEMMAKE_RESULT;
		{
			p.u.server_skill_itemmake_result.skillno = skillno;
			p.u.server_skill_itemmake_result.item = item;
			p.u.server_skill_itemmake_result.MakeItemCount  = SuccessMuch;
			p.u.server_skill_itemmake_result.TryItemCount	= iTryMakeCount;
		}							
		p.h.header.size = sizeof( t_server_skill_itemmake_result );
		QueuePacket( connections, cn, &p, 1);	
	}	
}
lpMAPSKILLTABLE	skill_ReturnProbablityFromSkillTable( int *probablity,  int skillno, int x, int y )
{			
			
	lpMAPSKILLTABLE	 s;
	int range = 0;	
	int pro;		
	int dx, dy;		
			
	x = x / TILE_SIZE;
	y = y / TILE_SIZE;
			
	dx = (g_Map.file.wWidth +7 ) / 8;
	dy = (g_Map.file.wHeight+7 ) / 8;
			
	dx = x / dx;	
	dy = y / dy;	
			
//	for( i = 0 ; i < 8 ; i ++ )
	{				
//		for( j = 0 ; j < 8 ; j ++ )
		{			
			s = g_Header[dx][dy];
					
			while( s != NULL )
			{		
				if( s->skillno == skillno )
				{	
					range = s->tile_Range +1;
					if( s->x == x && s->y == y ) 
					{
						*probablity = s->probability;
						return s;
					}
					else if( BoxAndDotCrash( s->x - range, s->y - range, range*2, range*2, x, y ) )
					{
						pro =  abs(range - abs(s->x-x));
						pro = s->probability * pro / range;
						*probablity = pro;
						return s;
					}
				}	
				s = s->next;
			}		
		}			
	}				
	return NULL;	
}					
void SendSkillGauge(const int cn, int skillno, int x, int y )
{
	t_packet p;
	lpMAPSKILLTABLE	 ms; 
	int probablity;
	t_earthpower *ep = &EarthPower;

	switch(skillno )
	{
			// Reclain..
		case 21 : 
			// mining
		case 26 : 
			// cutting
		case 32 : 
			// digging..
		case 36 : 
			// Tamming..
		case 75 :	
		case 28:	//fish
		case 30:	//netting

			break;

		default : return;
	}

	p.h.header.type = CMD_SKILL_GAUGE;
	p.h.header.size = sizeof( t_server_skill_gauge );
				
	p.u.server_skill_gauge.skillno =  skillno;
	p.u.server_skill_gauge.x		= x;
	p.u.server_skill_gauge.y		= y;

	
	//	mining/cutting俊 10栏肺 唱传捞蜡绰 蔼捞 焊烹 0 - 1000狼 蔼捞绊 霸捞瘤狼 钎泅裹困绰 1/100捞扁 锭巩..
	switch( skillno )
	{				
		// 俺埃...	
	case 21 :	ms = skill_ReturnProbablityFromSkillTable( &probablity,  TOOL_FARMING, x, y );
				if( ms )
				{	

					p.u.server_skill_gauge.min = ep->reclaim[ ms->type_Num ]/100;
					QueuePacket( connections, cn, &p, 1 );
				}
		break;
		// mining
	case 26 :	ms = skill_ReturnProbablityFromSkillTable( &probablity,  TOOL_MINING, x, y );
				if( ms )
				{
					
					p.u.server_skill_gauge.min = ep->mining[ ms->type_Num ]/10;
					QueuePacket( connections, cn, &p, 1 );
				}
				//1018 zhh
				break;

		// cutting
	case 32 : ms = skill_ReturnProbablityFromSkillTable( &probablity,  TOOL_CHOPPING, x, y );
				if( ms )
				{
					p.u.server_skill_gauge.min = ep->cutting[ ms->type_Num ]/10;
					QueuePacket( connections, cn, &p, 1 );
				}
				//1018 zhh
				break;

		// digging..
	case 36 : ms = skill_ReturnProbablityFromSkillTable( &probablity,  TOOL_HUB, x, y );
				if( ms )
				{
					p.u.server_skill_gauge.min = ep->digging[ ms->type_Num ]/10;
					QueuePacket( connections, cn, &p, 1 );		
				}
				//1018 zhh
				break;

	//1018 zhh
	case 28 :
	case 30 : ms = skill_ReturnProbablityFromSkillTable( &probablity,  TOOL_FISHING, x, y );
				if( ms )
				{
					p.u.server_skill_gauge.min = ep->fishing[ ms->type_Num ]/10;
					QueuePacket( connections, cn, &p, 1 );
				}				
				break;

	}
}
void RecvSkillInfo(const int cn, t_client_skill_info *sp)//020531 lsw 
{
	const int	skillno			= sp->skillno;
	const int	x				= sp->x;
	const int	y				= sp->y;
	const int	targettype		= sp->targettype;
	const int	target_id		= sp->target_id;
	
	char TargetName[31]={0,};
	memcpy(TargetName,sp->TargetName,30);

	int temp	= 0;
	int hitrate = 0;

	const int iTryRare		= sp->s.p1;
	const int iRareGroup	= sp->s.p2;
	int iTryMakeCount	= sp->s.p3;

	// 020428 YGI acer
	if( iTryMakeCount > 10 ) iTryMakeCount = 10;
	
	// acer5		// 阁胶磐 酒捞叼啊 逞绢 坷搁 绢痘霸 登扼绊
	CHARLIST *ch= CheckServerId(cn);
	if( !ch )					{return;}
	if( ch->cur_skillno )		{return;} // 泅犁 矫傈吝牢 扁贱捞 乐促搁 罐瘤 臼绰促. 
	if( ch->bAlive != ALIVE_ )	{return;}

	SaveBallancingSkill( ch, skillno );
		
	if( !ch->Hungry ) 
	{
		SendSkillResultFail( cn, skillno, ITEMMAKE_TIRED ); 
		return;
	}

	if(  timeGetTime() > ch->dwSkillTime)
	{
		ch->dwSkillTime = timeGetTime()+2000;
	}
	else
	{
		return;
	}

	//	郴啊 啊柳 扁贱捞 嘎促搁 逞绢啊绊 酒聪搁 府畔茄促.
	if(!SkillMgr.CheckIsYourSkill( ch, skillno )) 
	{
//		MyLog(LOG_NORMAL,"RecvSkillInfo() Recved Error SKillNo\n");
//		MyLog(LOG_NORMAL,"Name : %s Skill NO : %d\n",ch->Name, skillno);
		return;
	}
	HungryMuchAdd( ch , HT_SKILL );
	
	switch( sp->targettype )// 角菩啊 酒聪扼 肋给等 Data甫	吧矾辰促. 
	{		
	case SKILLTARGETTYPE_MAP_ :	
		{
			if( x < 0 ) goto FAIL_;
			if( y < 0 ) goto FAIL_;
			if( x / TILE_SIZE >= g_Map.file.wWidth )	goto FAIL_;
			if( y / TILE_SIZE >= g_Map.file.wHeight )	goto FAIL_;
		//	DecreaseEquipDuration(ch,rand()%6+1,WT_WEAPON,0,true);//020720 lsw
		}
		break;										
	case SKILLTARGETTYPE_TABLE_ :	//剧檬 父甸扁唱 捍父甸扁甫 困茄 何盒
		{
			if( target_id/1000 < 0 || target_id/1000 >ETC ) goto FAIL_;
		}
		break;
														
	case SKILLTARGETTYPE_ITEM_ :	//鸥百捞 酒捞袍牢巴
		{
			if( target_id < 0 || target_id >= MAX_ITEM_LIST ) goto FAIL_;
			if( ItemList[ target_id].bAlive == REMOVE_ ) goto FAIL_;

			temp = ItemList[ target_id].item_no;
			// 趣矫 扁贱阑 荤侩且荐 乐绰 Item寇狼 锅龋啊 坷绰巴阑 阜绰促.
			if( temp / 1000 < 0 || temp /1000 > 10 ) goto FAIL_;
		}
		break;
			     
	case SKILLTARGETTYPE_CHAR_ :	
		{
			if( target_id >= 10000 )	
			{
				if( target_id -10000 < NPC_LIST_START || target_id -10000 >= MAX_NPC_LIST ) goto FAIL_;
			}
			else
			{
				if( target_id < DRAGON_CONNECTIONS_START || target_id >= DRAGON_MAX_CONNECTIONS ) goto FAIL_;
			}
			
			CHARLIST *targetch = CheckServerId(target_id);
			if(targetch)
			{
				if(strcmp(TargetName,targetch->Name))//捞抚捞 促福促搁//020531 lsw
				{
					goto FAIL_;
				}
			}
		}
		break;
	}	
	
	hitrate = ReturnSkillProbablityNoExpAdd( cn, skillno, x, y, targettype, target_id );

	switch( targettype )
	{												
		case SKILLTARGETTYPE_TABLE_ : //SkillTargetTypeItemMake
			{
				SkillTargetTypeItemMake( cn, sp);
			}
			break;
			
		default : 
		{	
			t_packet p;
			p.h.header.type = CMD_SKILL_INFO;
			{
				p.u.server_skill_info.server_id = cn;
				p.u.server_skill_info.skillno	= skillno;
				p.u.server_skill_info.x			= x;
				p.u.server_skill_info.y			= y;
				p.u.server_skill_info.targettype= targettype;
				p.u.server_skill_info.target_id = target_id;
			}
						
			p.h.header.size = sizeof( t_server_skill_info );
			CastMe2Other( cn, &p );	
			QueuePacket( connections, cn, &p, 1);
		}			
				
		// 秦寸窍绰 扁贱老版快 努扼捞攫飘俊辑 霸捞瘤甫 免仿窍霸 茄促. 
		SendSkillGauge( cn, skillno, x, y );
		// Pre Check....//恐 刚历 眉农 窍绰单??
		switch( skillno )
		{	// MakeFire
			case  SKILL_MAKEFIRE :	
				{
					if( skill_Makefire_preCheck	( cn, skillno, x, y, targettype, target_id ) == 0 )	return;
				}break;
		}	
					
		ch->cur_skillno				= sp->skillno;				
		ch->cur_skillx				= sp->x;					
		ch->cur_skilly				= sp->y;							
		ch->cur_skilltargettype		= sp->targettype;				
		ch->cur_skilltarget_id		= sp->target_id;
		ch->cur_skillKey			= sp->iKey;//030102 lsw
		ch->cur_skillPOS			= sp->s;
		//020531 lsw		>
		memcpy(ch->cur_skillTargetName	,sp->TargetName,30);
		ch->cur_skillTargetName[30] = 0;//strcmp且锭 NtDll俊辑 欢唱绰芭 抗规
		//020531 lsw		<
		ch->exetime_skill			= g_curr_time + SkillTbl[ skillno].Time_judge / 10;
		break;						
	}
	return;
FAIL_:	
	SendSkillResultFail( cn, skillno );		
	return ;
}		

void ReduceTame()		//老沥 矫埃捞 儒福搁 tame阑 憋绰促.void CheckNPCTammingTimeElapsed( CHARLIST *ch )阑 汗荤秦辑 荤侩.
{	
	static int min;				
	
	if( g_curr_time - min > 900 )
	{		
		min = g_curr_time;
		for(int i = NPC_LIST_START ; i < MAX_NPC_LIST; i ++)
		{	
			if( NPCList[i].bAlive == ALIVE_ )
			if( NPCList[i].tame > 0 )
			{					
				CHARLIST *n = &NPCList[i];

				if (n->IsTamedNpc() == true)
				{	
					n->tame -= NPC_Gen_Ref[n->SprNo].ValueSP * 15;
					if(n->tame<0)	n->tame=0;
				}	
				else
				{	
					n->tame -= (NPC_Gen_Ref[n->SprNo].ValueHP * 15);
					if(n->tame<0)	n->tame=0;
				}				
								
				if (n->IsTamedNpc() && n->tame <= NPC_Gen_Ref[n->SprNo].Taming_min)
				{				
					n->SetTamed(false);		// LTS TAME
					n->HostName[0] = 0;
					n->tame_cmd = LC_TAME_STOP;
					n->patterntype = n->oldpatterntype;
								
					t_packet p;	
					p.h.header.type= CMD_TAME_END;
					p.u.tame_end.id = n->GetServerID() + 10000;
					p.h.header.size = sizeof( t_tame_host_name );
					CastNPC2Other( n->GetServerID(), &p );
								
					SendNPC_Parameter( n->GetServerID(), NPC_PARAMETER_TAME_CMD,	n->tame_cmd );
					SendNPC_Parameter( n->GetServerID(), NPC_PARAMETER_PATTERNTYPE,	n->patterntype );
				}				
			}					
		}						
	}							
}								

void RefreshSkillMapTable( void )
{	
//<! BBD 040311
/*	static	int		ttime;

	int g_y, g_mo, g_d, g_h, g_mi, g_se;		// 霸烙矫埃
	CurrentGameDate( g_curr_time*6, &g_y, &g_mo, &g_d, &g_h, &g_mi, &g_se );

	int sub = g_h-ttime;
	if( sub < 0 ) sub=sub+24;
	if( sub < 6 ) return;	//1矫埃 付促 refresh

	ttime = g_h;

	t_earthpower *ep;				
	ep = &EarthPower;
	int		i;

	static  int iRefreshCount=0;	
	if(iRefreshCount>3)	iRefreshCount=0;
	for( i = iRefreshCount ; i < ep->miningcount   ; i=i+4)	ep->mining[i]  =ep->mining_old[i];
	for( i = iRefreshCount ; i < ep->fishingcount  ; i=i+4)	ep->fishing[i] =ep->fishing_old[i];
	for( i = iRefreshCount ; i < ep->cuttingcount  ; i=i+4)	ep->cutting[i] =ep->cutting_old[i];
	for( i = iRefreshCount ; i < ep->diggingcount  ; i=i+4)	ep->digging[i] =ep->digging_old[i];
	iRefreshCount++;*/

	// 捞傈狼 鸥捞赣甫 脚汾且荐 绝菌促. 弊贰辑 窃荐郴何甫 狙荤府 促矫 备己窃

	static int old_hour = g_hour;
	static int old_min = g_min;
	static bool bReq = false;	// 悼立夸没阑 1锅父 窍扁困茄 敲饭弊

	int sub = g_hour-old_hour;
	if( sub < 0 ) sub=sub+24;

	if( sub != 4 || old_min != g_min)	// 4 矫埃 汝矾具瘤 关率栏肺 柳涝 4矫埃付促 refresh	// BBD 040322
	{
		int min = old_min - 1;
		if(min < 0) min += 60;

		if(!bReq && sub == 4 && min == g_min)	// 捞傈矫埃栏肺何磐 4矫埃 儒弗 矫埃狼 1盒傈 悼立阑 夸没茄促	// BBD 040322
		{
			RequestUsercountToProxy();		// 悼立夸没
			bReq = true;
		}
		return;	// 6矫埃捞 臼夌栏搁 府畔
	}

	bReq = false;
	old_hour = g_hour;
	old_min = g_min;

	t_earthpower *ep;				
	ep = &EarthPower;

	float Rate = GetResourceRate(g_UsercountInServerSet);	// 磊盔硅啦阑 备秦柯促

	MyLog(0, "Resetting ResourceRate = %.1f, cause UsercountInServerSet = %d", Rate, g_UsercountInServerSet);
	MyLog(0, "mine[0] orign:%d\tbefore:%d\tcurrent:%d", ep->mining_old[0], ep->mining[0], (int)(ep->mining_old[0] * Rate));
	MyLog(0, "fishing[0] orign:%d\tbefore:%d\tcurrent:%d", ep->fishing_old[0], ep->fishing[0], (int)(ep->fishing_old[0] * Rate));
	MyLog(0, "cutting[0] orign:%d\tbefore:%d\tcurrent:%d", ep->cutting_old[0], ep->cutting[0], (int)(ep->cutting_old[0] * Rate));
	MyLog(0, "digging[0] orign:%d\tbefore:%d\tcurrent:%d", ep->digging_old[0], ep->digging[0], (int)(ep->digging_old[0] * Rate));

	int		i;
	static  int iRefreshCount=0;	
	if(iRefreshCount>3)	iRefreshCount=0;
	for( i = iRefreshCount ; i < ep->miningcount   ; i=i+4)
	{
		ep->mining[i]  = (int)(ep->mining_old[i] * Rate);
	}
	for( i = iRefreshCount ; i < ep->fishingcount  ; i=i+4)
	{
		ep->fishing[i] = (int)(ep->fishing_old[i] * Rate);
	}
	for( i = iRefreshCount ; i < ep->cuttingcount  ; i=i+4)
	{
		ep->cutting[i] = (int)(ep->cutting_old[i] * Rate);
	}
	for( i = iRefreshCount ; i < ep->diggingcount  ; i=i+4)
	{
		ep->digging[i] = (int)(ep->digging_old[i] * Rate);
	}

	iRefreshCount++;
//> BBD 040311
}

short nSadTime = -1;
void UpdateSadonixCount()
{
	if( MapInfo[MapNumber].port == NATION_MANAGE_SERVER )
	{
		if( nSadTime == -1 )
		{
			nSadTime = g_wday*100 + g_hour;
		}
	
		const short tempnSadTme = nSadTime;
		nSadTime = g_wday*100 + g_hour;
		if( tempnSadTme != nSadTime)//函沁促搁
		{
			t_packet p;
			p.h.header.type = CMD_SADONIX_COUNT_UPDATE;
			p.h.header.size = sizeof(t_SadonixCountUpdate);
			p.u.SadonixCountUpdate.nSadTime = nSadTime;
			QueuePacket( connections, DB_DEMON, &p, 1 );
		}
	}
}
// 扒 扁贱俊 措秦 矫傈矫埃捞 瘤唱搁 角青矫挪促. 
void CheckSkillTimeElapsed( void )
{	//< CSD-HK-030829
	static DWORD time;

	RefreshSkillMapTable();
	ReduceTame();

	if (g_curr_time != time) 
	{ 
		time = g_curr_time; 
	}
	else
	{
		return;
	}

	CheckWeatherSystem();
	WeatherControl.CheckDayLight();
	
	for (int i = DRAGON_CONNECTIONS_START; i < DRAGON_MAX_CONNECTIONS ; ++i)
	{
		if (connections[i].dwAgentConnectionIndex && connections[i].state >= CONNECT_JOIN)
		{	
			CHARLIST* pTarget = &connections[i].chrlst;

			if (pTarget->GetServerID() != i)//020530 lsw
			{ 
				MyLog(LOG_FATAL,"Wrong ServerID User Have [%d] But [%d]", pTarget->GetServerID(), i);
				pTarget->SetServerID(i);
			}

			CrackMgr.AutoCheckAccelator(pTarget);
			WeatherControl.AutoSetDayLight(pTarget);

			if (pTarget->bAlive == ALIVE_)
			{
				SkillMgr.AutoRecoverHp(pTarget);
				SkillMgr.AutoRecoverMp(pTarget);
				SkillMgr.AutoChangeHungry(pTarget);
				SkillMgr.AutoRecoverCp(pTarget);
				SkillMgr.AutoRefreshRareMakeCount(pTarget);
				SkillMgr.AutoSetCanView(pTarget); //蜡飞 , 气藕 , 付过飘乏 殿阑 杭 荐 乐绰瘤 眉农
				RareEM.AutoCheckDynamicRare(pTarget->DynamicRare);

				if (pTarget->cur_skillno)
				{
					if (pTarget->exetime_skill < g_curr_time)
					{
						SkillMapper(i, pTarget->cur_skillno, pTarget->cur_skillx, pTarget->cur_skilly, pTarget->cur_skilltargettype, pTarget->cur_skilltarget_id, pTarget->cur_skillPOS);
						pTarget->cur_skillno = 0;
					}
				}
			}
		}
	}
	// 阁胶磐 磊悼 眉仿 雀汗
	for (int j = NPC_LIST_START; j < MAX_NPC_LIST; ++j)
	{
		CHARLIST* ch = CheckNpcId(j);
		if (ch == NULL)  continue;

		if (ch->bAlive == ALIVE_ && ch->IsLifeUp() && global_time >= ch->dwHpRecoverTime)
		{
			ch->dwHpRecoverTime = global_time + DEFAULT_RECOVER_HP_TIME;

			const int nLife = ch->GetLifeUp();
			const int nReal = ch->IncLife(nLife);

			if (nReal > 0)
			{	// 眉仿 雀汗贸烦 荤侩 				
				t_packet packet;
				packet.h.header.type = CMD_MAGIC_RESULT_H;
				packet.h.header.size = sizeof(t_server_magic_result_h);
				packet.u.magic.server_magic_result_h.nMagic = LIFE_UP;
				packet.u.magic.server_magic_result_h.idTarget = j + 10000;
				packet.u.magic.server_magic_result_h.nLife = nReal;
				packet.u.magic.server_magic_result_h.nState = ch->GetState();
				packet.u.magic.server_magic_result_h.nResult = HIT_AND_NOTDEAD;
				::CastNPC2Other(j, &packet);
			}
		}
	}

	UpdateSadonixCount();
}	//> CSD-HK-030829

//瘤况具 且 巴 鞍篮 窃荐 涝聪促.
void RecvSkillCursorView(const int cn, t_client_skill_cursor_view  *p)
{	
	t_packet packet;
	
	packet.h.header.type = CMD_SKILL_CURSOR_VIEW;
	{
		packet.u.client_skill_cursor_view.skillno = p->skillno;
		packet.u.client_skill_cursor_view.x = p->x;
		packet.u.client_skill_cursor_view.y = p->y;
	}
	packet.h.header.size = sizeof( t_client_skill_cursor_view );

	//	QueuePacket( connection, cn, &packet );
	CastMe2Other( cn, &packet );
}	
	
//	扁贱 搬苞甫 促弗捞俊霸 舅赴促.
//  漂洒 档迪扁贱俊 角菩窍搁 鞘洒 舅妨具 窍瘤... 鞠....
//	cn : 扁贱阑 敬荤恩.
//	target_id : 扁贱阑 寸茄 荤恩..
void SendSkillResultToOther(const int cn, const int skillno, const int target_id, const int result )
{	
	t_packet p;
	p.h.header.type = CMD_SKILL_RESULT_TOOTHER;
	
		p.u.server_skill_result_toother.id		= cn; // 扁贱阑 敬荤恩..
		p.u.server_skill_result_toother.skillno = skillno;
		p.u.server_skill_result_toother.result  = result;
	
	p.h.header.size = sizeof( t_server_skill_result_fail ); // 
	
	QueuePacket( connections, target_id, &p, 1);
}	
		
void SendSkillResult(const int cn, const int skillno, const int target_id, const int x, const int y, const int result )
	{	
	t_packet p;
	p.h.header.type = CMD_SKILL_RESULT;
	p.u.server_skill_result.skillno = skillno;
	p.u.server_skill_result.x		= x;
	p.u.server_skill_result.y		= y;
	p.u.server_skill_result.result	= result;
	p.h.header.size = sizeof( t_server_skill_result );
	QueuePacket( connections, cn, &p, 1);
}	
	
void SendViewTypeChange(const int cn, int viewtype )
{	
	t_packet p;
	
	p.h.header.type		= CMD_VIEWTYPE_CHANGE;
	p.h.header.size		= sizeof( t_viewtype_change );
	p.u.viewtype_change.server_id = cn;
	p.u.viewtype_change.viewtype = viewtype;
	if( cn < 10000 )		//PC牢 版快父 焊晨
	{
		QueuePacket( connections, cn, &p, 1);
		CastMe2Other( cn, &p );
	}
	else
	{
		CastNPC2Other( cn-10000, &p);
	}
}
		
int skill_Detect_Ghost	( const int cn, const int skillno, const int x, const int y, const int targettype, const int target_id )
{	
	CHARLIST *ch = CheckServerId(cn);//020531 lsw
	if( !ch ) return false;	// 020620 YGI

	const int iHitrate	= ReturnSkillProbablity( cn, skillno, x, y, targettype, target_id );
	const int r = rand()%100;
	const int iTime = (SkillTbl[skillno].Produce_Fix*iHitrate)/1000000;//杭 荐 乐绰 檬 窜困
	
	if(SkillMgr.CanViewGhost(ch)){goto FAIL_; }
	if( r > iHitrate )	{ goto FAIL_; }
	if(!iTime)			{ goto FAIL_; }
	
	SkillMgr.SetCanViewGhost(ch,iTime);
	SendSkillResult( cn, skillno, target_id, x, y, iTime );
	return 1; 

FAIL_:
	SendSkillResultFail(cn, skillno); 
	return 0;
}	

int skill_Poisoning			(const int cn, const int skillno, const int x, const int y, const int targettype, const int target_id )
{
	SendSkillResult( cn, skillno, target_id, x, y, -999 ); 
	return 1; 
}

int skill_Detect_Poison		(const int cn, const int skillno, const int x, const int y, const int targettype, const int target_id )
{ 
	SendSkillResult( cn, skillno, target_id, x, y, -999 ); 
	return 1; 
}

int skill_Find_Traps		(const int cn, const int skillno, const int x, const int y, const int targettype, const int target_id )
{	
	CHARLIST *ch = GetCharListPtr(cn);
	if(!ch) return 0;
	if( ch->bCanViewTrap ){ return 1;}
	
	const int iHitrate	= ReturnSkillProbablity( cn, skillno, x, y, targettype, target_id );
	const int r = rand()%100;
	if( r > iHitrate ) {  SendSkillResultFail(cn, skillno); return 0; }

	const int lv = ch->skillexp[ SkillTbl[skillno].MotherSkillType].skillexp / 10000;
	int iTime = 30 + (lv*3);//	扁夯 30 檬 + 1 寸 3		
	const int MAX = 300;
	
	if( iTime > MAX ) {iTime = MAX;}
	SkillMgr.SetCanViewTrap(ch,iTime);//020510 lsw
	SendSkillResult( cn, skillno, target_id, x, y, iTime );
	return 1; 
}

int skill_GhostResurrection(const int cn, const int skillno, const int x, const int y, const int targettype, const int target_id )
{
	const int r = rand()%100;;
	int hitrate;
	LPCHARLIST Caster=	CheckServerId(cn);
	LPCHARLIST Target=	CheckServerId(target_id);

	if( ! Caster ) {goto FAIL_;}
	if( ! Target ) {goto FAIL_;}
	
	if(!SkillMgr.CanViewGhost(Caster))			{	goto FAIL_;	}//蜡飞阑 杭 荐 绝促//020701 lsw
	if( Target->viewtype != VIEWTYPE_GHOST_ )	{	goto FAIL_;	}

	hitrate = ReturnSkillProbablity( cn, skillno, x, y, targettype, target_id );
	
	if( r >= hitrate ) {goto FAIL_; }

	// 捞扁贱篮 努贰胶啊 己流磊父 荤侩且荐 乐促. 
	switch(Caster->Class)
	{
		case WARRIOR : 
		case THIEF	 : 
		case ARCHER	 : 
		case WIZARD	 : goto FAIL_;
		case PRIEST	 : break;
	}

//< soto-030326 吝赋瘤开捞 酒匆锭绰...促弗 惫啊 荤恩阑 混副荐啊 绝促.  1.02滚怜
	if(!IsNeutralZone(MapNumber))
	{
		if(Caster->name_status.nation != Target->name_status.nation)
			goto FAIL_;
	}
//> soto-030326

	


	// 扁贱捞 甸绢艾栏唱 穿啊 刚历 混啡阑 锭...
//	if(CON_NORMAL == Target->GetState() )	{goto FAIL_;}
	if( CheckEventAttack())					{goto FAIL_;}//020205 lsw
	if( CanLocalWarAttack(Caster,Target) )	{goto FAIL_;}//020205 lsw

	SkillMgr.CharacterToAlive(Target);//荤恩 混府扁
	
	addEXP( Caster, Target, HIT_GHOSTRESURRECTION, Target->HpMax / 10 );

	SendSkillResult( cn, skillno, target_id, x, y, target_id );
	return 1; 

FAIL_:

	SendSkillResultFail(cn, skillno); 
	return 0;
}
//01214 lsw
int skill_Remove_Traps		(const int cn, const int skillno, const int x, const int y, const int targettype, const int target_id )
{
	CItem_Etc *t;
	int UserSkillLevel = 0;
	const int ret = ReturnSkillProbablity( cn, skillno, x, y, targettype, target_id );
	if( ret == 0 ) goto FAIL_;

	t  =(CItem_Etc *)ItemUnit( ItemList[ target_id].item_no);

	if(!t){goto FAIL_;}

	if(t->GetItemKind() != IK_TRAP){goto FAIL_;}
	
	UserSkillLevel = connections[ cn].chrlst.skillexp[ SkillTbl[skillno].MotherSkillType].skillexp / 10000;
	if( t->Repair_max > UserSkillLevel ) {goto FAIL_;}
	
	if( ItemList[ target_id].attr[ IATTR_ATTR] & IA2_TRAPED1 )
	{
		ItemList[ target_id].attr[ IATTR_ATTR] &= ( ~(IA2_NOTMOVE) );
		ItemList[ target_id].attr[ IATTR_ATTR] &= ( ~(IA2_TRAPED1) );

		//1024 zhh
		ItemList[ target_id].attr[IATTR_DURATION] = 10;

		SetArea( CHANGE_IA2_ATTR_ITEM_AREA, target_id );

		//1020 zhh
		RemoveItemList(target_id);
		AddItemList( ItemList[target_id].item_no  , ItemList[target_id].attr  , 0, ItemList[target_id].x, ItemList[target_id].y, 0, 0);
	//	AddItemList( c[cn].chrlst.handheld.item_no, c[cn].chrlst.handheld.attr, 0, pi->x, pi->y, 0, 0 );

		SendSkillResult( cn, skillno, target_id, x, y, 1 );
		return 1; 
	}
	
FAIL_ :
	 SendSkillResultFail(cn, skillno); return 0; 
}
//011215 lsw 
int skill_Entrapment		(const int cn, const int skillno, const int x, const int y, const int targettype, const int target_id )
{
	const int ret = ReturnSkillProbablity( cn, skillno, x, y, targettype, target_id );
	CItem *t;
//	if( g_block_steal ) {	goto FAIL_;	}//020821 lsw
	if( ret == 0 ) goto FAIL_;

	// 气藕狼 傍拜.. 付过 公矫 窍绊 埃促.
	t  = ItemUnit( ItemList[ target_id].item_no);

	if(!t){goto FAIL_;}

	if(t->GetItemKind() != IK_TRAP){goto FAIL_;}//if( ItemList[ target_id].item_no == 10033 ) 措眉
	
	//repairmin 单固瘤 ,...
	//repairmax 啊 瓷仿摹 力茄
	if( ( ItemList[ target_id].attr[ IATTR_ATTR] & IA2_TRAPED1 ) == 0 )
	{
		//1009 zhh	//单固2瘤甫 固府 拌魂茄促
		CItem_Etc *t = (CItem_Etc *)ItemUnit(ItemList[ target_id].item_no/1000, ItemList[ target_id].item_no%1000);
			
		if(!t){goto FAIL_;}

		int UserSkillLevel = connections[ cn].chrlst.skillexp[ SkillTbl[skillno].MotherSkillType].skillexp / 10000;
		if( t->Repair_max > UserSkillLevel ) 
		{
			OutMessage(cn,2,522);
			goto FAIL_;
		}
		
		if( !SkillTbl[skillno].Produce_Fix){goto FAIL_;}
		if(	!abs(ret) ){goto FAIL_;}
		if(!t->Repair_min){goto FAIL_;}

		DWORD dwDmg = ((SkillTbl[skillno].Produce_Fix/10000) * (abs(ret)/10) * ((t->Repair_min)/10));//020205 lsw

		if(!dwDmg) {goto FAIL_;}

		ItemList[ target_id].attr[ IATTR_ATTR] |= IA2_NOTMOVE;
		ItemList[ target_id].attr[ IATTR_ATTR] |= IA2_TRAPED1;
		
		ItemList[ target_id].birthday = g_curr_time;//020215 lsw
		
		ItemList[ target_id].attr[IATTR_DURATION] = dwDmg + (dwDmg*(rand()%20))/100;
				
		SetArea( CHANGE_IA2_ATTR_ITEM_AREA, target_id );
		SetArea( CHANGE_IA2_DURATION_ITEM_AREA, target_id );//021107 lsw
		
		SendSkillResult( cn, skillno, target_id, x, y, 1 );
		return 1; 
	}
	
FAIL_ :
	 SendSkillResultFail(cn, skillno); return 0; 
	return 0; 
}






int skill_Detect_Bomb		(const int cn, const int skillno, const int x, const int y, const int targettype, const int target_id )//011215 lsw
{	
	CHARLIST *ch = GetCharListPtr(cn);
	if(!ch) return 0;
	
	const int iHitrate	= ReturnSkillProbablity( cn, skillno, x, y, targettype, target_id );
	const int r = rand()%100;
	if( r > iHitrate ) {  SendSkillResultFail(cn, skillno); return 0; }

	const int lv = ch->skillexp[ SkillTbl[skillno].MotherSkillType].skillexp / 10000;
	int iTime = 30 + (lv*3);//	扁夯 30 檬 + 1 寸 3		
	const int MAX = 300;
	
	if( ch->bCanViewBomb ){ return 1;}
	
	if( iTime > MAX ) {iTime = MAX;}
	SkillMgr.SetCanViewBomb(ch,iTime);//020510 lsw
	SendSkillResult( cn, skillno, target_id, x, y, iTime );
	return 1; 
}
//011215 lsw
int skill_Bomb_in			(const int cn, const int skillno, const int x, const int y, const int targettype, const int target_id )
{
	int ret = ReturnSkillProbablity( cn, skillno, x, y, targettype, target_id );
	CItem *t;
//	if( g_block_steal ) {	goto FAIL_;	}//020821 lsw
	if( ret == 0 ) goto FAIL_;

	// 气藕狼 傍拜.. 付过 公矫 窍绊 埃促.
	t  = ItemUnit( ItemList[ target_id].item_no);

	if(!t){goto FAIL_;}

	if(t->GetItemKind() != IK_BOMB){goto FAIL_;}//if( ItemList[ target_id].item_no == 10033 ) 措眉
	
	//repairmin 单固瘤 ,...
	//repairmax 啊 瓷仿摹 力茄
	if( ( ItemList[ target_id].attr[ IATTR_ATTR] & IA2_TRAPED0 ) == 0 )
	{
		//1009 zhh	//单固2瘤甫 固府 拌魂茄促
		CItem_Etc *t = (CItem_Etc *)ItemUnit(ItemList[ target_id].item_no/1000, ItemList[ target_id].item_no%1000);
			
		if(!t){goto FAIL_;}

		int UserSkillLevel = connections[ cn].chrlst.skillexp[ SkillTbl[skillno].MotherSkillType].skillexp / 10000;
		if( t->Repair_max > UserSkillLevel ) 
		{	OutMessage(cn,2,522);
			goto FAIL_;
		}

		if( !SkillTbl[skillno].Produce_Fix){goto FAIL_;}
		if(	!abs(ret) ){goto FAIL_;}
		if(!t->Repair_min){goto FAIL_;}

		DWORD dwDmg = ((SkillTbl[skillno].Produce_Fix/10000) * (abs(ret)/10) * ((t->Repair_min)/10));//020205 lsw

		if(!dwDmg) {goto FAIL_;}

		ItemList[ target_id].attr[ IATTR_ATTR] |= IA2_NOTMOVE;
		ItemList[ target_id].attr[ IATTR_ATTR] |= IA2_TRAPED0;

		ItemList[ target_id].attr[IATTR_DURATION] = dwDmg+ (dwDmg*(rand()%20))/100;
		ItemList[ target_id].birthday = g_curr_time;//020215 lsw

		SetArea( CHANGE_IA2_ATTR_ITEM_AREA, target_id );
		SetArea( CHANGE_IA2_DURATION_ITEM_AREA, target_id );//021107 lsw
		
		SendSkillResult( cn, skillno, target_id, x, y, 1 );
		return 1; 
	}
	
FAIL_ :
	 SendSkillResultFail(cn, skillno); 
	 return 0; 
}
//1027 zhh
int BombExplosionCheck(const int cn,int id)
{
	
	
	if( id < 0 ) return 0;
	if( id >= MAX_ITEM_LIST ) return 0;
	
	if( connections[cn].chrlst.bAlive == ALIVE_ )
	if( ItemList[ id].bAlive == ALIVE_ )
	{
		if( ItemList[ id].attr[ IATTR_ATTR] & IA2_TRAPED0 )
		{
			if(rand()%10) return 0;
			
			t_packet packet;

      connections[cn].chrlst.DecLife(ItemList[ id ].attr[IATTR_DURATION]);

			if (connections[cn].chrlst.IsDead() == true)
			{
				killCharacter( NULL, GetCharListPtr(cn) );
			}

      packet.h.header.type = CMD_FOOT_ON_TRAP_0_ITEM;
      packet.h.header.size = sizeof(t_server_foot_on_trap0);
			packet.u.server_foot_on_trap0.idServer = WORD(cn);
			packet.u.server_foot_on_trap0.idItem = WORD(id);
			packet.u.server_foot_on_trap0.nLife = connections[cn].chrlst.Hp;

			QueuePacket(connections, cn, &packet, 1);
			CastMe2Other( cn, &packet );
			
			RemoveItemList( id );
			return 1;
		}
	}
	return 0;
}

//011214 lsw
int skill_Remove_Bomb		(const int cn, const int skillno, const int x, const int y, const int targettype, const int target_id )
{	
	CItem_Etc *t;
	int UserSkillLevel = 0;
	int ret = ReturnSkillProbablity( cn, skillno, x, y, targettype, target_id );
	if( ret == 0 ) goto FAIL_;

	t  = (CItem_Etc *)ItemUnit( ItemList[ target_id].item_no);

	if(!t){goto FAIL_;}

	if(t->GetItemKind() != IK_BOMB){goto FAIL_;}//if( ItemList[ target_id].item_no == 10033 ) 措眉

	UserSkillLevel = connections[ cn].chrlst.skillexp[ SkillTbl[skillno].MotherSkillType].skillexp / 10000;
	if( t->Repair_max > UserSkillLevel ) {goto FAIL_;}
	
	if( ItemList[ target_id].attr[ IATTR_ATTR] & IA2_TRAPED0 ) // Bomb
	{
		//1019 zhh
	//	ItemList[ target_id].birthday=g_curr_time;

//		if(BombExplosionCheck(const int cn,int id)==1)		goto fail_;
		//1024 zhh
		ItemList[ target_id].attr[IATTR_DURATION] = 10;

		ItemList[ target_id].attr[ IATTR_ATTR] &= ( ~(IA2_NOTMOVE) );
		ItemList[ target_id].attr[ IATTR_ATTR] &= ( ~(IA2_TRAPED0) );
		SetArea( CHANGE_IA2_ATTR_ITEM_AREA, target_id );
		//1020 zhh
		RemoveItemList(target_id);
		AddItemList( ItemList[target_id].item_no  , ItemList[target_id].attr  , 0, ItemList[target_id].x, ItemList[target_id].y, 0, 0);
	//	AddItemList( c[cn].chrlst.handheld.item_no, c[cn].chrlst.handheld.attr, 0, pi->x, pi->y, 0, 0 );
		
		SendSkillResult( cn, skillno, target_id, x, y, 1 );
		return 1; 
	}
FAIL_:
	SendSkillResultFail(cn, skillno); 
	return 0; 
}

//<soto-steal眠啊
int		SerchItemNPCINV(CHARLIST* ch,int nItemNumber)
{
	int count = 0;
	for(int a=0;a<3;a++)
		for(int  b=0;b<3;b++)
			for(int c=0;c<8;c++)
				if(ch->inv[a][b][c].item_no == nItemNumber)
				{
					++count;
				}

	return count;
}

int		TotalItemCountNPCINV(CHARLIST* ch)
{
	int count = 0;
	for(int a=0;a<3;a++)
		for(int  b=0;b<3;b++)
			for(int c=0;c<8;c++)
				if(ch->inv[a][b][c].item_no)
				{
					++count;
				}

	return count;
}
//>soto-steal眠啊


int skill_Stealing	(const int cn, const int skillno, int x, int y, const int targettype, const int target_id )//020815 lsw
{	
	t_packet p;	
	
	int count =0;
	int i=0,j=0,k=0;
	int ex=0, ey=0;
	int sx=0, sy=0;
	int iTrapdamage=0;
	int iTrapResultdamage=0;
	int a1=0,a2=0,a3=0;		
	int why = 0;
	int ret = 0,r = 0;
	int iTraped = 0;
	int iSteal  = 0;
	int warning = 40;
	int iForceItemNo = 0;

	ItemAttr StealItem;
	ItemAttr ResultItem;
	POS pos;
	CItem *ii = NULL;
	
	ItemAttr *pStealItem = NULL;

	LPCHARLIST target = ::CheckNpcId(target_id - 10000);
//	LPCHARLIST target = ::CheckServerId(target_id);
	LPCHARLIST caster = ::CheckServerId(cn);

	int nItemCount = SerchItemNPCINV(target,10123);
	int nTotalCount = TotalItemCountNPCINV(target);
	
	POS ItemPos[72] = {0,};

	if( !target ) {goto FAIL__;}
	if( !caster ) {goto FAIL__;}
	if( caster == target ) {goto FAIL__;}
	
	if( g_block_steal ) {goto FAIL__;}//020821 lsw

	if(	targettype != SKILLTARGETTYPE_CHAR_ ){	goto FAIL__;	}
	if( MapInfo[MapNumber].forrookie == 1 
	||	MapInfo[MapNumber].forrookie == 2)
	{
		goto FAIL__;
	} //PK啊 救登绰 瘤开俊辑绰 壬磨 荐 绝促.
	

	if(::stricmp(target->Name,caster->cur_skillTargetName))//020531 lsw 
	{
		goto FAIL__;
	}
	if(caster->GetLevel() < 31) {goto FAIL__;} // CSD-030806
	if(target->IsCounselor()){goto FAIL__;}//款康磊绰 给壬媚//020730 lsw
	if(CanLocalWarAttack(caster,target)){goto FAIL__;}
	if( target->viewtype == VIEWTYPE_GHOST_ ) {goto FAIL__;}
	if(	target->Hp <=0 ) {goto FAIL__;}

	x = target->MoveSx;
	y = target->MoveSy;

	if(LocalMgr.IsAbleNation(KOREA))
	{
		if( TileMap[x][y].attr_no_battle || !IsNeutralZone(MapNumber) ) // soto 030307 " || !IsEnableSkill_Steal(MapNumber) " 何盒 眠啊.
		{
			OutMessage(caster->GetServerID(),2,525);//011218 lsw
			::SendSkillResultFail(cn, skillno); 
			return 0;
		}

		if(!LocalMgr.IsFreeBeta())
		{
			if (!caster->IsPayedUser())
			{
				::SendSkillResultFail(cn, skillno); 
				return 0;
			}
		}
	}
	else
	{
		if( TileMap[x][y].attr_no_battle )
		{
			OutMessage(caster->GetServerID(),2,525);//011218 lsw
			SendSkillResultFail(cn, skillno); 
			return 0;
		}
	}

	ex = target->MoveSx;	
	ey = target->MoveSy;
	sx = caster->MoveSx;	
	sy = caster->MoveSy;

	if(BoxAndDotCrash(sx-3,sy-3,7,7,ex,ey))//	档迪龙篮 距 馆版 3鸥老救俊 乐绢具 4-8鸥老观俊 乐栏搁 20%肺 兑冻绢柳促. 
	{

	}	
	else if( BoxAndDotCrash( sx - 6, sy-6, 13, 13, ex, ey ) ) 
	{
		if( (rand()%100) < 80 )
		{
			goto FAIL__;
		}
	}
	else 
	{
		goto FAIL__;
	}

	ret = ReturnSkillProbablity( cn, skillno, x, y, targettype, target_id );
	r  = rand()%100;
	
	if( r > ret )
	{
		goto FAIL__;
	}

	count= 0;
	for(i = 0;	i<3	;	i++)
	{
		for(j= 0;	j<3	;j++)
		{
			for(k= 0;	k<8	;k++)
			{
				StealItem = target->inv[i][j][k];
				if(StealItem.item_no)
				{		
					if(StealItem.item_no == IN_STEAL_RESULT)//刘芭 酒袍捞搁 归厘固捞促.
					{
						continue;
					}
					ii = ItemUnit( StealItem.item_no );
					if( ii == NULL ) {continue;}
					
					if(ii->GetItemKind() == IK_LOCK_STEAL)
					{
						iForceItemNo = StealItem.item_no;
						iTraped =1;
					}

					//if( ii->GetLevel() > caster->skillexp[SkillTbl[skillno].MotherSkillType].skillexp / 10000 )
					if( ii->GetLevel() > 98)//98鳖瘤父 壬磨荐 乐促.
					{
						continue;
					}
					SetItemPos(INV, i,j,k, &ItemPos[count++]);
				}
			}
		}
	}
	if(!count)
	{
		goto FAIL__;
	}

	iSteal= rand()%count;

	if(!iTraped )//飘乏捞 酒丛 
	{
		pStealItem = GetItemByPOS( target, ItemPos[iSteal]);
		i =ItemPos[iSteal].p1;
		j =ItemPos[iSteal].p2;
		k =ItemPos[iSteal].p3;
	}
	else
	{
		if(SearchItemByInv(iForceItemNo,target,i,j,k))//飘乏捞搁 飘乏锅龋肺 茫酒辰促
		{
			POS pos;
			pos.type = INV;
			pos.p1 = i; pos.p2 = j; pos.p3 = k;
			pStealItem = GetItemByPOS( target, pos);
		}
		else
		{
			goto FAIL__;
		}
	}
	
	if( !pStealItem ) goto FAIL__;
	StealItem = *pStealItem;//酒捞袍阑 壬摹聪鳖 蔼阑 烙矫傍埃俊 历厘 秦 滴绊
	ii = ItemUnit( StealItem.item_no );

	if(!StealItem.item_no)
	{
		goto FAIL__;
	}
	///////////////////////////////
	// 壬摹扁 寸茄磊狼 贸府(构啊 灯电 鸥百登绰 磊府狼 酒捞袍篮 厘固啊 登带瘤 绝绢柳促)
	if(!iTraped )//飘乏捞 酒丛 
	{
		ResultItem = ::GenerateItem( IN_STEAL_RESULT );//归厘固
	}
	else//飘乏烙
	{
		ZeroMemory(&ResultItem	,sizeof(ItemAttr));//酒捞袍 绝绢咙
		ZeroMemory(&StealItem   ,sizeof(ItemAttr));//壬媚稠 酒捞袍 绝绢咙//飘乏阑 壬磨 荐绰 绝垒酒.
////////担 贸府
// soto 030307 担俊 吧府搁...官肺 磷绢具 等促.		
		if(LocalMgr.IsAbleNation(KOREA))
		{
			caster->Hp = 0;
		}
		else
		{
			iTrapdamage  = ii->Getbody();
			iTrapResultdamage = (iTrapdamage / 10000) ;
			iTrapResultdamage += rand()%((iTrapdamage%10000)+1);
			caster->Hp -= iTrapResultdamage;
			if( 0 > caster->Hp){caster->Hp=0;}			
		}
// soto End.
		p.h.header.type = CMD_STEAL_ON_TRAP;
		p.h.header.size = sizeof(t_server_steal_on_trap);
		p.u.server_steal_on_trap.idServer = WORD(cn);
		p.u.server_steal_on_trap.nLife = caster->Hp;//矫傈磊狼 Hp甫 弊措肺 焊郴淋

		QueuePacket(connections, cn, &p, 1);
		CastMe2Other( cn, &p );

		if (caster->IsDead() == true)
		{
			killCharacter( NULL, caster );
			SendCharacterCondition(caster, 0); // soto 030307 眠啊. 惑怕甫 焊郴 林绢具 磷歹扼. ぱぱ;
		}
		
		OutMessage(target->GetServerID(),2,523);
		OutMessage(caster->GetServerID(),2,523);

// soto 030307 眠啊.
////////飘肪阑 绝矩促.
		SetItemPos( INV, i,j,k, &pos );
//		SendDeleteItem(&target->inv[i][j][k],&pos,target,false);
		DeleteItem(&target->inv[i][j][k]);//阁胶磐 捞骨肺.
		return 0; 		
// soto End.
	}
	SendItemEventLog( pStealItem, target_id, cn, SILT_PUT_STOLEN, 1 );
	*pStealItem = ResultItem;//寸茄 拱扒阑 持绢林磊	//壬模 酒捞袍 牢亥配府 器牢磐俊 蔼阑 持绢霖促

// soto 030307 lsw 丛狼 夸没栏肺 荐沥.
//	p.h.header.type = CMD_GIVE_ITEM; 
//	{
//		p.u.server_give_item.p1 = i;
//		p.u.server_give_item.p2 = j;
//		p.u.server_give_item.p3 = k;
//		p.u.server_give_item.item = ResultItem;
//	}
//	p.h.header.size = sizeof( t_server_give_item );
//	QueuePacket( connections, target_id, &p, 1);//寸茄荤恩 酒捞袍 扁废 巢辫

//	SetItemPos( INV, i,j,k, &pos );
//	SendServerEachItem( &pos, &target->inv[i][j][k], target_id);
// soto End.

	memset(&p, 0, sizeof(t_packet));
	// 咯扁鳖瘤 坷搁 壬摹扁 己傍捞扁 锭巩俊 
	if( SearchInv( caster->inv, a1, a2, a3 ) )  //后沫捞 乐促搁
	{		
		caster->inv[a1][a2][a3] = StealItem;	// 
		SetItemPos( INV, a1,a2,a3, &pos );
		SendServerEachItem( &pos, &caster->inv[a1][a2][a3], cn );
	}				
	else//顶俊 代绢哆覆....
	{				
		AddItemList( StealItem.item_no,StealItem.attr, 0, target->X+rand()%40, target->Y+rand()%60, 0, 0,  0,0,0,0 );
	}
	
	if( caster->IsUltraNK(MapInfo[MapNumber].nation) )	
	{	
		warning = 100;
	}


//	if( rand()%100 < warning ) // 5%狼 犬伏肺 寸茄 娟茄单 搬苞甫 舅妨霖促.  ( 甸淖促 )
//	{
//		SendSkillResult( cn, skillno, target_id, x, y, 2 ); // 甸淖促. 
//		SendSkillResultToOther( cn, skillno, target_id, 1 );
//		CheckNK( cn, target_id, NK_TYPE_STEAL_ );
//	}
//	else 
	{
		SendSkillResult( cn, skillno, target_id, x, y, 1 );
	}


	return 0;
FAIL__:
	{
		CheckNK( cn, target_id, NK_TYPE_NO_KILL_);//NK 眠啊
		SendSkillResultFail(cn, skillno); 
		return 0;
	}
	return 0;
}
		

int skill_Reclaim			(const int cn, const int skillno, const int x, const int y, const int targettype, const int target_id )
{	
	CHARLIST *ch = CheckServerId(cn);//020720 lsw
	if(!ch){return 0;}

	int probablity;
	int why = 0; 
	int how;
	lpMAPSKILLTABLE	mt;
	int ret;
	t_earthpower *ep = &EarthPower;
	DecreaseEquipDuration(ch,rand()%8+1,WT_WEAPON,0,true);//020720 lsw

	mt = skill_ReturnProbablityFromSkillTable( &probablity,  1, x, y );
	if( mt == NULL ) { why = SKILLFAIL_NOT_PROPER_PLACE; goto FAIL_; }

	if( mt->type_Num && mt->type_Num < MAX_EARTHSKILL ) 
	{
 		if( ep->reclaim[ mt->type_Num ] >= 10000 ) // 促 俺埃捞 登菌栏搁..
		{
			why = SKILLFAIL_NO_MORE_RACLAIM;
			goto FAIL_;
		}

		ret = ReturnSkillProbablity( cn, skillno, x, y, targettype, target_id );
		how = ret * SkillTbl[skillno].Produce_Fix;
		how /= 10000;

		ep->reclaim[ mt->type_Num ] += how;
		if( ep->reclaim[ mt->type_Num ] > 10000 ) ep->reclaim[ mt->type_Num ] = 10000;
		SendSkillResult( cn, skillno, target_id, x, y, ep->reclaim[ mt->type_Num ]/ 100 );
		return 1; 
	}
	
FAIL_:
	
	SendSkillResultFail( cn, skillno, why );
	return 0;
}	
/*int skill_Sow				(const int cn, const int skillno, const int x, const int y, const int targettype, const int target_id )
{	
	DWORD attr = ItemList[target_id].attr[2];
	
	if( attr & IA2_FARM0 ) { SendSkillResultFail(cn, skillno); return 0; }
	if( attr & IA2_FARM1 ) { SendSkillResultFail(cn, skillno); return 0; }
	if( attr & IA2_FARM2 ) { SendSkillResultFail(cn, skillno); return 0; }
	if( attr & IA2_FARM3 ) { SendSkillResultFail(cn, skillno); return 0; }
	if( attr & IA2_FARM4 ) { SendSkillResultFail(cn, skillno); return 0; }
	
	int ret = ReturnSkillProbablity( cn, skillno, x, y, targettype, target_id );
	if( ret == 0 ) { SendSkillResultFail(cn, skillno); return 0; }
	
	int probablity;
	lpMAPSKILLTABLE	mt = skill_ReturnProbablityFromSkillTable( &probablity,  1, x, y );
	
	ItemList[target_id].attr[2] |= IA2_FARM0;
	ItemList[target_id].attr[2] |= IA2_NOTMOVE;
	ItemList[target_id].attr[IATTR_DURATION] = g_curr_time + 6;
	
	SetArea( CHANGE_IA2_ATTR_ITEM_AREA, target_id );
	SetArea( CHANGE_IA2_DURATION_ITEM_AREA, target_id );
	
	SendSkillResult( cn, skillno, target_id, x, y, mt != NULL );
	
	return 1; 
}*/	
	

int skill_Sow				(const int cn, const int skillno, const int x, const int y, const int targettype, const int target_id, POS s )
{
	ItemAttr *item, ditem;
	CItem *ii;	
	lpMAPSKILLTABLE	mt;
	int probablity;
	int why = 0;
	DWORD har;	
	t_earthpower *ep;
	int hitrate;
	int tep;
	int iSendValue = 0;		// acer
	
	CHARLIST *pCh = CheckServerId(cn);//030102 lsw
	if(!pCh){return 0;}

	switch(s.type) 
	{			
		case INV: 	item = &pCh->inv[s.p1][s.p2][s.p3];	break;
		case HAND:	item = &pCh->handheld;				break;
		case EQUIP:	item = &pCh->equip[s.p3];			break;
		case QUICK:	item = &pCh->quick[s.p3];			break;
		default : goto ERROR_;
	}			
	if( !item->attr[IATTR_MUCH])//荐樊捞 葛磊扼促搁 扁贱捞 救等促
	{
		goto ERROR_;
	}
	if( item->item_no != target_id )
	{
		goto ERROR_;
	}
	
	ii = ItemUnit( item->item_no );
	if( ii == NULL ) 
	{
		goto ERROR_;
	}
	
	if( ii->GetItemKind() != IK_SEED ) 
	{
		goto ERROR_;
	}

	mt = skill_ReturnProbablityFromSkillTable( &probablity,  1, x, y );

	if( NULL == mt)
	{
		why = SKILLFAIL_NOT_PROPER_PLACE; 
		goto FAIL_; 
	}

	hitrate = ReturnSkillProbablity( cn, skillno, x, y, targettype, target_id );
	if( 0 == hitrate ) 
	{
		goto ERROR_; 
	}
	
	memcpy( &ditem, item, sizeof( ItemAttr ) );
	iSendValue = ItemMgr.DecItemDur(*item,1);
	
	SendServerEachItem(&s,item,cn);		// acer

	ditem.attr[ IATTR_ATTR]		|=	IA2_FARM0;
	ditem.attr[ IATTR_ATTR]		|=	IA2_NOTMOVE;
	ditem.attr[IATTR_DURATION]	 =  g_curr_time + ITEM_FARM_GROW_TIME;
			
//			
//          俺埃档/100 * Produce_Fix/100 * 扁夯己傍伏 * ( 101 - 累拱狼 Lv ) 
//            100 *   100      *     100%   *       100       = 100,000,000
//			
//          窍唱狼 备开俊辑 掘阑荐 乐绰 荐犬拱篮 困狼 拌魂俊辑 唱柯蔼 / 1,000,000 捞促. 
// 牢 磊 :	
//	

	ep = &EarthPower;

	tep = ep->reclaim[ mt->type_Num ] / 3;
	if( tep < 0 ) tep = 0;
	if( tep > 10000 ) tep = 10000;

	har = tep;
	har /= 100;
	har *= ( SkillTbl[ skillno].Produce_Fix / 100 );
	har *= hitrate;
	har *= ( 101 - ii->GetLevel() );
	har /= 1000000;

	ep->reclaim[ mt->type_Num ] -= tep;

	ditem.attr[ IATTR_RESERVED0] = har;
			
	AddItemList( ditem.item_no, ditem.attr, 0, x, y, 0,0 );
			
	SendSkillResult( cn, skillno, target_id, x, y, mt != NULL );	
			
	return 1; 
FAIL_ :
ERROR_:

	SendSkillResultFail( cn, skillno, why );
	return 0;
}	
	
int skill_Havest			(  int cn, const int skillno, const int x, const int y, const int targettype, const int target_id )
{		
	ItemAttr item = {0,};
	int why = 0;
	CItem *ii;
	int ret;
	LPITEMLIST target;
	int getitemno;
	int amount;
	int hitrate;
	int i;
	CHARLIST *ch = GetCharListPtr(cn);
	int sx = ch->X;
	int sy = ch->Y;

	
	if( target_id < 0 || target_id >= MAX_ITEM_LIST ) goto FAIL_;

	target = &ItemList[ target_id];
	ii = ItemUnit( target->item_no );
	if( ii == NULL ) { why = SKILLFAIL_UNKNOWN_ITEM; goto FAIL_; }

	if( ii->GetItemKind() != IK_SEED ){	why = SKILLFAIL_NO_SEED;	goto FAIL_; }

	ret = SkillMgr.FindLearnItem( ch, target->item_no );
	if( ret == false )  { why = SKILLFAIL_UNKNOWN_ITEM; goto FAIL_; }

	if( target->attr[ IATTR_ATTR ] & IA2_FARM7 ) 
	{	
		hitrate = ReturnSkillProbablity(cn, skillno, x, y, targettype, target_id );

		getitemno = ii->Getmouse_MRD();
		getitemno /= 100;
		if( getitemno == 0 ) { why = SKILLFAIL_NO_SEED; goto FAIL_; }
		
		item = GenerateItem( getitemno );//020509 lsw
		if( item.item_no == 0 ) { why = SKILLFAIL_NO_SEED; goto FAIL_; }

		// 茄锅 撤龙俊 荐犬且荐 乐绰 樊阑 沥茄促. 
		amount = 5;
		if( target->attr[ IATTR_RESERVED0] < amount ) amount = target->attr[ IATTR_RESERVED0];
		target->attr[ IATTR_RESERVED0] -= amount;
		
		for( i = 0 ; i < amount ; i ++)
		{
			DropItem( sx-50+rand()%100, sy-50+rand()%100, &item);
		}
		
		//1002 zhh
		if( i == 0 ) { why = 0; goto FAIL_; }
		
		// 葛滴 荐犬沁栏搁..
		if( target->attr[ IATTR_RESERVED0] == 0 ) RemoveItemList( target_id);

		//胶懦 瓷仿摹 80 捞惑捞绊 //犬伏篮 1/500
/*
		if((connections[cn].chrlst.skillexp[ SkillTbl[skillno].MotherSkillType].skillexp / 10000)  > 80)
		{
			const int r = rand()%10000;
			if(TAKE_SADONIX_PERCENT > r)
			{
				GiveSadonixToGround(cn, SADONIX_NO,skillno,MapInfo[MapNumber].port,sx, sy);//021010 lsw
			}
		}
*/
		SkillMgr.IncSkillCount(ch,BLESSLEAF_NO,skillno,ItemMgr.GetSkillPointByItem(item));//胶懦 墨款飘 刘啊
		SendSkillResult( cn, skillno, target_id, x, y, 1 );
	}	
	else {  why = SKILLFAIL_NOT_YET_HARVEST; goto FAIL_; }
		
	return 1; 
		
FAIL_:	
		
	SendSkillResultFail( cn, skillno, why );
	return 0;	
		
}		

//010604 lsw
int skill_Mining	(const int cn, const int skillno, const int x, const int y, const int targettype, const int target_id )
{
	CHARLIST *ch = CheckServerId(cn);//020720 lsw
	if(!ch){return 0;}

	ItemAttr item = {0,};
	int probablity;
	lpMAPSKILLTABLE mt;
	short int subType;
	int why = 0;
	int i;
	int r;
	int hitrate;
	int sx, sy;
	int itemno;
	int amount;
	t_earthpower *ep;
	
	int checkitem  = 0;//011014 lsw
	
	DecreaseEquipDuration(ch,rand()%8+1,WT_WEAPON,0,true);//020720 lsw

	mt = skill_ReturnProbablityFromSkillTable( &probablity,  TOOL_MINING , x, y );
	if( mt == NULL ) { why = SKILLFAIL_NOT_PROPER_PLACE; goto FAIL_; }

	r = rand()%100;

//	hitrate = ReturnSkillProbablityNoExpAdd( cn, skillno, x, y, targettype, target_id );
	hitrate = ReturnSkillProbablity( cn, skillno, x, y, targettype, target_id );//010612 lsw 抗傈栏肺
	if( r >= hitrate ) { goto FAIL_; }

	ep = &EarthPower;
	
	if( ep->mining[ mt->type_Num ] > 0 )//	概厘樊捞 乐栏搁 
	{	
//		AddSkillExp( cn, skillno, hitrate );//010607 lsw
		subType = mt->subType;
		itemno = ReturnFromSkillMapTable(cn,TOOL_MINING,subType,hitrate - r,RETURN_SMT_ITEMNO);//020214 lsw
		//011014 lsw >
		checkitem = SkillMgr.FindLearnItem(GetCharListPtr(cn),itemno);
		if ( checkitem != -1 ) {goto FAIL_;}
		//011014 lsw <

		if( itemno == 0 )  { goto FAIL_; }
		
		//1013 zhh
		int demand=ReturnFromSkillMapTable(cn,TOOL_MINING,subType,hitrate - r,RETURN_SMT_DEMAND);//020214 lsw
		if(demand!=-9999)
			amount = SkillTbl[ skillno].Produce_Fix * ( abs(hitrate - demand)/15 ) / 10000 +1;
		else
		{
			amount=1;
		}
		
		ep->mining[ mt->type_Num ] -= amount;
		if( ep->mining[ mt->type_Num ] < 0 ) ep->mining[ mt->type_Num ]  = 0;
	}	
	else
	{	
		why = SKILLFAIL_NO_MINING;
		goto FAIL_; 
	}	
		
	sx = ch->X;
	sy = ch->Y;
	item = GenerateItem( itemno);//020509 lsw 
	for( i = 0 ; i < amount;  i ++)
	{
		DropItem( sx-50+rand()%100, sy-50+rand()%100, &item);
	}
	SkillMgr.IncSkillCount(ch,SADONIX_NO,skillno,ItemMgr.GetSkillPointByItem(item));//胶懦 墨款飘 刘啊
	SendSkillResult( cn, skillno, target_id, x, y, 1 );
	return 1; 
		
FAIL_ :		

	//1013 zhh
	if(why==0 || why==SKILLFAIL_NO_MINING)
		GiveBasicItem(cn,TOOL_MINING,1);

	SendSkillResultFail( cn, skillno, why );
	return 0;
}		
		
		
int skill_Refine			(const int cn, const int skillno, const int x, const int y, const int targettype, const int target_id )
{
	SendSkillResult( cn, skillno, target_id, x, y, -999 );
		
	return 1; 
}
//010604 lsw
int skill_Fish				(const int cn, const int skillno, const int x, const int y, const int targettype, const int target_id )
{ 
	CHARLIST *ch = CheckServerId(cn);//020720 lsw
	if(!ch){return 0;}

	ItemAttr item = {0,};
	int probablity;
	lpMAPSKILLTABLE mt;
	int why = 0; 
	int r;
	int sx, sy;
	int hitrate;
	int checkitem  = 0;//011014 lsw
	
	// 超矫.....
	mt = skill_ReturnProbablityFromSkillTable( &probablity,  TOOL_FISHING, x, y );
	if( mt == NULL ) { why = SKILLFAIL_NOT_PROPER_PLACE; goto FAIL_; }

	DecreaseEquipDuration(ch,rand()%8+1,WT_WEAPON,0,true);//020720 lsw

	r = rand()%100;
//	hitrate = ReturnSkillProbablityNoExpAdd( cn, skillno, x, y, targettype, target_id );
	hitrate = ReturnSkillProbablity( cn, skillno, x, y, targettype, target_id );//010612 lsw 抗傈栏肺
	if( r >= hitrate ) { goto FAIL_; }

	//1009 zhh
	int i;
	int itemno;
	int amount;
	t_earthpower *ep;
	short int subType;
	ep = &EarthPower;
	//	概厘樊捞 乐栏搁 
	if( ep->fishing[ mt->type_Num ] > 0 )
	{	
//		AddSkillExp( cn, skillno, hitrate );//010607 lsw
		subType = mt->subType;
		itemno = ReturnFromSkillMapTable(cn,TOOL_FISHING,subType,hitrate - r,RETURN_SMT_ITEMNO);//020214 lsw
		if( itemno == 0 )  { goto FAIL_; }
	
		//1013 zhh
		int demand=ReturnFromSkillMapTable(cn,TOOL_FISHING,subType,hitrate - r,RETURN_SMT_DEMAND);//020214 lsw
		if(demand!=-9999)
			amount = SkillTbl[ skillno].Produce_Fix * ( abs(hitrate - demand)/15 ) / 10000 +1;
		else
		{
			amount=1;
		}
		
		ep->fishing[ mt->type_Num ] -= amount;
		if( ep->fishing[ mt->type_Num ] < 0 ) ep->fishing[ mt->type_Num ]  = 0;
	}	
	else
	{	
		//1017 zhh
		why = SKILLFAIL_NO_FISHING;
		goto FAIL_; 
	}	
		
	sx = ch->X;
	sy = ch->Y;
	item = GenerateItem( itemno);//020509 lsw 

	//011014 lsw >
	checkitem = SkillMgr.FindLearnItem(GetCharListPtr(cn),itemno);
	if ( checkitem != -1 ) {goto FAIL_;}
	//011014 lsw <

	for( i = 0 ; i < amount;  i ++)
	{
		DropItem( sx-50+rand()%100, sy-50+rand()%100, &item);
	}

	SkillMgr.IncSkillCount(ch,BLESSLEAF_NO,skillno,ItemMgr.GetSkillPointByItem(item));//胶懦 墨款飘 刘啊
	SendSkillResult( cn, skillno, target_id, x, y, 1 );
	return 1;

FAIL_:

	//1013 zhh //1017 zhh
	if(why==0 || why==SKILLFAIL_NO_FISHING)
		GiveBasicItem(cn,TOOL_FISHING,1);

	SendSkillResultFail( cn, skillno, why );
	return 0; 
}



//1016 zhh 葛滴 货肺 备泅等 窃荐
int skill_Netting			(const int cn, const int skillno, const int x, const int y, const int targettype, const int target_id )
{ 
	CHARLIST *ch = CheckServerId(cn);//020720 lsw
	if(!ch){return 0;}

	ItemAttr item = {0,};
	int probablity;
	lpMAPSKILLTABLE mt;
	int why = 0; 
	int r;
	int sx, sy;
	int hitrate;
	int checkitem =0;//011014 lsw 
	
	DecreaseEquipDuration(ch,rand()%8+1,WT_WEAPON,0,true);//020720 lsw

	// 超矫.....
	mt = skill_ReturnProbablityFromSkillTable( &probablity,  TOOL_FISHING, x, y );
	if( mt == NULL ) { why = SKILLFAIL_NOT_PROPER_PLACE; goto FAIL_; }

	hitrate = ReturnSkillProbablity( cn, skillno, x, y, targettype, target_id );
	r = rand()%100;
	//1011 zhh
	if( r >= hitrate ) { goto FAIL_; }

	//1009 zhh
	int i;
	int itemno;
	int amount;
	t_earthpower *ep;
	short int subType;
	ep = &EarthPower;
	//	概厘樊捞 乐栏搁 
	if( ep->fishing[ mt->type_Num ] > 0 )
	{	
		subType = mt->subType;
		itemno = ReturnFromSkillMapTable(cn,TOOL_FISHING,subType,hitrate - r,RETURN_SMT_ITEMNO);//020214 lsw
		if( itemno == 0 )  { goto FAIL_; }
	
		//1013 zhh
		int demand=ReturnFromSkillMapTable(cn,TOOL_FISHING,subType,hitrate - r,RETURN_SMT_DEMAND);//020214 lsw
		if(demand!=-9999)
			amount = SkillTbl[ skillno].Produce_Fix * ( abs(hitrate - demand)/15 ) / 10000 +1;
		else
			amount=1;
	//	amount = SkillTbl[ skillno].Produce_Fix * (( hitrate ) + 2 ) / 10000;
		
		ep->fishing[ mt->type_Num ] -= amount;
		if( ep->fishing[ mt->type_Num ] < 0 ) ep->fishing[ mt->type_Num ]  = 0;
	}	
	else
	{	
		//1017 zhh
		why = SKILLFAIL_NO_FISHING;
		goto FAIL_; 
	}	
		
	sx = ch->X;
	sy = ch->Y;
	item = GenerateItem( itemno);//020509 lsw 
	//011014 lsw >
	
	checkitem = SkillMgr.FindLearnItem(GetCharListPtr(cn),itemno);
	if ( checkitem != -1 ) {goto FAIL_;}
	//011014 lsw <

	for( i = 0 ; i < amount;  i ++)
	{
		DropItem( sx-50+rand()%100, sy-50+rand()%100, &item);
	}
	SkillMgr.IncSkillCount(ch,BLESSLEAF_NO,skillno,ItemMgr.GetSkillPointByItem(item));//胶懦 墨款飘 刘啊
	SendSkillResult( cn, skillno, target_id, x, y, 1 );
	return 1;

FAIL_:

	//1013 zhh	//1017 zhh
	if(why==0 || why==SKILLFAIL_NO_FISHING)
		GiveBasicItem(cn,TOOL_FISHING,1);

	SendSkillResultFail( cn, skillno, why );
	return 0; 
}




//010604 lsw
int skill_Cutting			(const int cn, const int skillno, const int x, const int y, const int targettype, const int target_id )
{	
	ItemAttr item = {0,};
	int probablity;
	lpMAPSKILLTABLE mt;
	int why = 0; 
	int r;
	int sx, sy;
	int hitrate;
	int amount;
	int subType;
	int itemno;
	int i;
	CHARLIST *ch = CheckServerId(cn);//020720 lsw
	if(!ch){return 0;}
	t_earthpower *ep;	

	//	国格......
	mt = skill_ReturnProbablityFromSkillTable( &probablity,  TOOL_CHOPPING, x, y );
	if( mt == NULL ) { why = SKILLFAIL_NOT_PROPER_PLACE; goto FAIL_; }

	r = rand()%100;
//	hitrate = ReturnSkillProbablityNoExpAdd( cn, skillno, x, y, targettype, target_id );
	hitrate = ReturnSkillProbablity( cn, skillno, x, y, targettype, target_id );//010612 lsw 抗傈栏肺
	DecreaseEquipDuration(ch,rand()%8+1,WT_WEAPON,0,true);//020803 lsw
	if( r >= hitrate ) { goto FAIL_; }

	//1011 zhh

	ep = &EarthPower;
	//	骇 唱公啊 乐栏搁 
	if( ep->cutting[ mt->type_Num ] > 0 )
	{	
//		AddSkillExp( cn, skillno, hitrate );//010607 lsw
		subType = mt->subType;
		itemno = ReturnFromSkillMapTable(cn,TOOL_CHOPPING,subType,hitrate - r,RETURN_SMT_ITEMNO);//020214 lsw
		if( itemno == 0 )  { goto FAIL_; }
	
		//1013 zhh
		int demand=ReturnFromSkillMapTable(cn,TOOL_CHOPPING,subType,hitrate - r,RETURN_SMT_DEMAND);//020214 lsw
		if(demand!=-9999)
			amount = SkillTbl[ skillno].Produce_Fix * ( abs(hitrate - demand)/15 ) / 10000 +1;
		else
		{
			amount=1;
		}
	//	amount = 1 + rand() % 5;

		ep->cutting[ mt->type_Num ] -= amount;
		if( ep->cutting[ mt->type_Num ] < 0 ) ep->cutting[ mt->type_Num ]  = 0;
		SendSkillGauge( cn, skillno, x, y );
	}	
	else
	{	
		why = SKILLFAIL_NO_CUTTING;
		goto FAIL_; 
	}	
		
	sx = ch->X;
	sy = ch->Y;

	item = GenerateItem( itemno );
	for( i = 0 ; i < amount ; i ++)
	{
		DropItem( sx-50+rand()%100, sy-50+rand()%100, &item);
	}
	SkillMgr.IncSkillCount(ch,SADONIX_NO,skillno,ItemMgr.GetSkillPointByItem(item));//胶懦 墨款飘 刘啊
	SendSkillResult( cn, skillno, target_id, x, y, 1 );
	return 1; 
		
FAIL_:	

	//1013 zhh
	if(why==0 || why==SKILLFAIL_NO_CUTTING)
		GiveBasicItem(cn,TOOL_CHOPPING,1);

	SendSkillResultFail( cn, skillno, why );
	return 0; 
}		

// 1005 zhh
int skill_Lumbering			(const int cn, const int skillno, const int x, const int y, const int targettype, const int target_id )
{
	ItemAttr item = {0,};
	int why = 0, r = 0, hitrate = 0;
	
	CItem_Join *join = NULL;
	CItem *ii = NULL;
	int iResultItemNo = 0;
	int iGroundItemNo = 0;
	CHARLIST* pCh = CheckServerId(cn);
	const int iDestItemNo = pCh->cur_skillKey;//Clicked Item
	
	const ITEMLIST *pItemList = &ItemList[target_id];
	iGroundItemNo = pItemList->item_no;
	if( FALSE == pItemList->bAlive
	||	iDestItemNo != iGroundItemNo
	||  !iGroundItemNo) 
	{
		why = ITEMMAKE_NO;
		goto _ERROR;
	}
	
	ii = ItemUnit(iGroundItemNo);
	if( !ii )
	{
		goto _FAIL;
	}

	iResultItemNo = ii->Getmouse_MRD()/100;	// 格利 ItemNo甫 备茄促. 
	if( !iResultItemNo )	
	{  
		why = SKILLFAIL_NOT_RESOURCE; 
		goto _FAIL; 
	}

	join = ReturnItemJoinTable( cn, skillno, iResultItemNo );
	if(!join){goto _ERROR;}

	hitrate = ReturnSkillProbablity( cn, skillno, x, y, targettype, target_id , join );
	r = rand()%100;
	if( r >= hitrate ) 
	{
		goto _FAIL; 
	}
 
	RemoveItemList( target_id );
	item = GenerateItem(iResultItemNo);
	AddItemList( item.item_no, item.attr,0,  x, y, 0,0 );
	
	SendSkillResult( cn, skillno, target_id, x, y, 1 );
	return 1; 

_ERROR:	//Data Error Dest Item No Delete
	{
		SendSkillResultFail( cn, skillno, why );
	}
	return 0; 
_FAIL: //Skill Failed
	{
		RemoveItemList( target_id );
		SendSkillResultFail( cn, skillno, why );
	}
	return 0; 
}	
	
// 010129 KHS
int skill_Butchering		(const int cn, const int skillno, const int x, const int y, const int targettype, int target_id )
{	
	ItemAttr item = {0,};
	CHARLIST *n;
	CHARLIST *butcher = GetCharListPtr(cn);//010302 lsw// 档绵磊狼 某腐磐 罐酒咳 
	int itemno;
	int sx, sy;
	int why = 0;
	int ret,r;
	int checkitem;
	CHARLIST *ch = CheckServerId(cn);//020720 lsw
	if(!ch){return 0;}

	if( target_id < 10000 )
	{	
		//010204 lsw			// 010210 YGI
		why = -84;//010302 lsw
		goto FAIL_;
	}
		
	target_id -= 10000;
	if( target_id < NPC_LIST_START || target_id >= MAX_NPC_LIST )
	{
		//010204 lsw			// 010210 YGI
		why = -84;//010302 lsw
		goto FAIL_;
	}
		
	n = &NPCList[ target_id];

	if( n->bAlive != DEAD_ )  
	{
		why = -82;	
		goto FAIL_;
	}
		
	//1101 zhh
	switch( n->SprNo )
	{	
	case 18: case 19 : case 20 : case 21: case 22: case 23: case 24: case 56: case 57: case 63: case 64: case 65: case 66: case 67: case 69: case 71: case 72: case 73: case 74: case 75: case 76: case 77: case 78: case 79: case 80:
		goto FAIL_;
	}	

	ret = ReturnSkillProbablity( cn, skillno, x, y, targettype, target_id );
	r = rand()%100;
	//1011 zhh
	if( r > ret ) goto FAIL_;
				
	// Meat..
	itemno = n->Get_meat;

	checkitem = SkillMgr.FindLearnItem(butcher,itemno);
	if(checkitem== -1)//010302  lsw
	{
	if( n->Meat_amount > 0 )
		{
			n->Meat_amount --;
			sx = connections[cn].chrlst.X - 30 + rand()%60;
			sy = connections[cn].chrlst.Y - 30 + rand()%60;
			item = GenerateItem(itemno);	
			DropItem( sx-50+rand()%100, sy-50+rand()%100, &item);
			//1002 zhh
			SendSkillResult( cn, skillno, target_id, x, y, 1 );
		}
	}	
	else
	{
		why = -10;
		goto FAIL_;
	}

	// Leather..
	
	itemno = n->Get_skin;
	checkitem = SkillMgr.FindLearnItem(butcher,itemno);
	if(checkitem== -1)
	{
		if( n->Skin_amount > 0 )
		{
			n->Skin_amount --;
			sx = connections[cn].chrlst.X - 30 + rand()%60;
			sy = connections[cn].chrlst.Y - 30 + rand()%60;
			item = GenerateItem( itemno);//020509 lsw 
			DropItem( sx-50+rand()%100, sy-50+rand()%100, &item);
			//1002 zhh
			SendSkillResult( cn, skillno, target_id, x, y, 1 );
/*
			//011014 lsw >
			if( n->Level >= 36 )
			{
				const int r = rand()%10000;
				if(TAKE_SADONIX_PERCENT > r)
				{
					GiveSadonixToGround(cn,SADONIX_NO,skillno,MapInfo[MapNumber].port,sx, sy);
				}
			}
			//011014 lsw <
*/
		}
	}
	else
	{
		why = -10;
		goto FAIL_;
	}

	if( n->Skin_amount == 0 &&  n->Meat_amount == 0 )
	{
		why = SKILLFAIL_NO_MEET;
		goto FAIL_;
	}
	SkillMgr.IncSkillCount(ch,SADONIX_NO,skillno,ItemMgr.GetSkillPointByItem(item));//胶懦 墨款飘 刘啊
	SendSkillResult( cn, skillno, target_id, x, y, 1 );

	return 1; 
		
FAIL_:	
		
		
	SendSkillResultFail(cn, skillno, why ); 
	
		
	return 0;
}		
	

//010604 lsw
int skill_Digging			(const int cn, const int skillno, const int x, const int y, const int targettype, const int target_id )
{
	CHARLIST *ch = CheckServerId(cn);//020720 lsw
	if(!ch){return 0;}

	ItemAttr item = {0,};
	int probablity;
	lpMAPSKILLTABLE mt;
	short int subType;
	int why = 0; 
	int r;
	int hitrate;
	int amount;
	int itemno;
	int i;
	t_earthpower *ep;
	int checkitem = 0;
	
	mt = skill_ReturnProbablityFromSkillTable( &probablity,  TOOL_HUB, x, y );
	if( mt == NULL ) { why = SKILLFAIL_NOT_PROPER_PLACE; goto FAIL_; }
	
	DecreaseEquipDuration(ch,rand()%8+1,WT_WEAPON,0,true);//020720 lsw

	r = rand()%100;
//	hitrate = ReturnSkillProbablityNoExpAdd( cn, skillno, x, y, targettype, target_id );
	hitrate = ReturnSkillProbablity( cn, skillno, x, y, targettype, target_id );//010612 lsw 抗傈栏肺
	if( r >= hitrate ) { goto FAIL_; }

	ep = &EarthPower;
	//	纳尘 距檬啊 乐栏搁 
	if( ep->digging[ mt->type_Num ] > 0 )
	{	
//		AddSkillExp( cn, skillno, hitrate );//010607 lsw
		subType = mt->subType;
		itemno = ReturnFromSkillMapTable(cn,TOOL_HUB,subType,hitrate - r,RETURN_SMT_ITEMNO);//020214 lsw
		if( itemno == 0 )  { goto FAIL_; }
		
		//1013 zhh
		int demand=ReturnFromSkillMapTable(cn,TOOL_HUB,subType,hitrate - r,RETURN_SMT_DEMAND);//020214 lsw
		if(demand!=-9999)
		{
			amount = SkillTbl[ skillno].Produce_Fix * ( abs(hitrate - demand)/15 ) / 10000 +1;
		}
		else
		{
			amount=1;//唱棵 荐樊捞肺绊..
		}
		ep->digging[ mt->type_Num ] -= amount;
		if( ep->digging[ mt->type_Num ] < 0 ) ep->digging[ mt->type_Num ]  = 0;
		SendSkillGauge( cn, skillno, x, y );
	}	
	else
	{	
		why = SKILLFAIL_NO_DIGGING;
		goto FAIL_; 
	}	
		
	int sx, sy;
	sx = connections[cn].chrlst.X;
	sy = connections[cn].chrlst.Y;
		
	item = GenerateItem( itemno);//020509 lsw 
	//011014 lsw >
	checkitem = SkillMgr.FindLearnItem(GetCharListPtr(cn),itemno);
	if ( checkitem != -1 ) {goto FAIL_;}
	//011014 lsw <

	for( i = 0 ; i < amount ; i ++)
	{
		DropItem( sx-50+rand()%100, sy-50+rand()%100, &item);
	}

	SkillMgr.IncSkillCount(ch,BLESSLEAF_NO,skillno,ItemMgr.GetSkillPointByItem(item));//胶懦 墨款飘 刘啊
	SendSkillResult( cn, skillno, target_id, x, y, item.item_no );
	
	return 1; 
FAIL_ :	

	//1013 zhh
	if(why==0 || why==SKILLFAIL_NO_DIGGING)
		GiveBasicItem(cn,TOOL_HUB,1);
		
	SendSkillResultFail( cn, skillno, why );
	return 0;
}
int skill_Makepowder		(const int cn, const int skillno, const int x, const int y, const int targettype, const int target_id ){ SendSkillResult( cn, skillno, target_id, x, y, -999 );return 1; }



//010604 lsw 
int skill_Shear				(const int cn, const int skillno, const int x, const int y, const int targettype, int target_id )
{ 
	ItemAttr item = {0,};
	CHARLIST *n;
	int itemno;
	int sx, sy;
	int hitrate;
	int why = 0;
	int r;
//荤恩 茄抛 借绰单 皋技瘤 捞惑窍绊, 版氰摹 悼矫俊 救棵扼皑?

	if( target_id < 10000 ) 
	{
		//010204 lsw			// 010210 YGI
		why = -83;	//010302 lsw
		goto FAIL_;	// PC俊霸 扁贱阑 洤栏搁 俊矾.
	}

	target_id -= 10000;
	n = &NPCList[ target_id];
	
	if( n->bAlive != ALIVE_ ) 
	{	
		//010204 lsw			// 010210 YGI
		why = -85; //this2 lsw lsw
		goto FAIL_;
	}
	// Hair..
	itemno = n->Get_Wool;
	if( n->Wool_amount > 0 )
	{
		hitrate = ReturnSkillProbablity( cn, skillno, x, y, targettype, target_id );//010604 lsw
		r = rand()%100;
		if( r > hitrate )  goto FAIL_;

		n->Wool_amount--;
		sx = connections[cn].chrlst.X + 30 + rand()%60;
		sy = connections[cn].chrlst.Y + 30 + rand()%60;
		item = GenerateItem( itemno);//020509 lsw 
		
		DropItem( sx-50+rand()%100, sy-50+rand()%100, &item);

		//1002 zhh
		SendSkillResult( cn, skillno, target_id, x, y, 1 );
		return 1; 
	}
	else why = SKILLFAIL_NO_SHEAR;
	
FAIL_:
	SendSkillResultFail(cn, skillno, why ); 
	return 0;
}


int skill_Manufacturing(const int cn, const int skillno, const int x, const int y, const int targettype, const int target_id )
{ 
	return 1; 
}

int skill_Cooking			(const int cn, const int skillno, const int x, const int y, const int targettype, const int target_id ){ SendSkillResult( cn, skillno, target_id, x, y, -999 );return 1; }
int skill_Clothing			(const int cn, const int skillno, const int x, const int y, const int targettype, const int target_id ){ SendSkillResult( cn, skillno, target_id, x, y, -999 );return 1; }

int skill_Tailoring			(const int cn, const int skillno, const int x, const int y, const int targettype, const int target_id ){ SendSkillResult( cn, skillno, target_id, x, y, -999 );return 1; }
int skill_Bowcrafting		(const int cn, const int skillno, const int x, const int y, const int targettype, const int target_id ){ SendSkillResult( cn, skillno, target_id, x, y, -999 );return 1; }
int skill_Fletching			(const int cn, const int skillno, const int x, const int y, const int targettype, const int target_id ){ SendSkillResult( cn, skillno, target_id, x, y, -999 );return 1; }
int skill_MakePotion		(const int cn, const int skillno, const int x, const int y, const int targettype, const int target_id ){ SendSkillResult( cn, skillno, target_id, x, y, -999 );return 1; }
int skill_MakeMetal			(const int cn, const int skillno, const int x, const int y, const int targettype, const int target_id ){ SendSkillResult( cn, skillno, target_id, x, y, -999 ); return 1; }
int skill_MakeBottle		(const int cn, const int skillno, const int x, const int y, const int targettype, const int target_id ){ SendSkillResult( cn, skillno, target_id, x, y, -999 );return 1; }
int skill_Candlemaking		(const int cn, const int skillno, const int x, const int y, const int targettype, const int target_id ){ SendSkillResult( cn, skillno, target_id, x, y, -999 );return 1; }
int skill_Oiling			(const int cn, const int skillno, const int x, const int y, const int targettype, int target_id )
{ 
	ItemAttr item = {0,};
	CHARLIST *n;
	int itemno;
	int sx, sy;

	int ret = ReturnSkillProbablity( cn, skillno, x, y, targettype, target_id );
	if( ret == 0 ) { SendSkillResultFail(cn, skillno); return 0; }

	if( target_id < 10000 )
	{
		// 荤扼捵ぉ Butchering且鉴  绝瘤...
		SendSkillResultFail( cn, skillno );
		return 0;
	}

	target_id -= 10000;
	n = &NPCList[ target_id];
	if( n->bAlive != DEAD_ )  { SendSkillResultFail(cn, skillno); return 0; }

	// Wax..
	itemno = n->Get_beewax;
	if( n->BeeWax_amount > 0 )
	{
		n->BeeWax_amount--;
		sx = connections[cn].chrlst.X - 30 + rand()%60;
		sy = connections[cn].chrlst.Y - 30 + rand()%60;

		item = GenerateItem( itemno);//020509 lsw 
		AddItemList( item.item_no, item.attr,0,  sx, sy, 0,0 );
		//1002 zhh
		SendSkillResult( cn, skillno, target_id, x, y, 1 );
	}

	return 1; 
}

int skill_Snooping			(const int cn, const int skillno, const int x, const int y, const int targettype, const int target_id )
{
	
	int ret,r;

	LPCHARLIST target;//020530 lsw 鸥百
	LPCHARLIST caster;

	if( cn == target_id )	goto FAIL_;
	if( target_id >= 10000 )	goto FAIL_;

	if( MapNumber == 30 ) goto FAIL_; // Scholium俊辑绰 壬摹扁 且 荐 绝促.

	target = CheckServerId(target_id);
	caster = CheckServerId(cn);
	
	if( !target ) {goto FAIL_;}
	if( !caster ) {goto FAIL_;}
	
	if(strcmp(target->Name,caster->cur_skillTargetName))//020531 lsw 
	{
		goto FAIL_;
	}
	if(target->IsCounselor()){goto FAIL_;}//款康磊绰 给壬媚//020730 lsw

	ret = ReturnSkillProbablity( cn, skillno, x, y, targettype, target_id );

	r = rand()%100;
	if( ret > r )
	{
		if( target->bAlive == DEAD_ && 	target->viewtype != VIEWTYPE_GHOST_ )
		{
			SendOtherInv( CMD_OTHER_CH_INV, target_id, connections, cn );
			SendSkillResult( cn, skillno, target_id, x, y, 1 );

			CheckNK( cn, target_id,NK_TYPE_STEAL_ );
		}
		else goto FAIL_;
	}

	return 1; 

FAIL_:

	SendSkillResultFail(cn, skillno); return 0;
}

int skill_Makefire			(const int cn, const int skillno, const int x, const int y, const int targettype, const int target_id )
{	
//	int ret = ReturnSkillProbablity( cn, skillno, x, y, targettype, target_id );
//	if( ret == 0 ) { SendSkillResultFail(cn, skillno); return 0; }
	int ret = 1;
	switch( ItemList[target_id].item_no  )
	{
	case 1011	: 
	case 1012	:	
	case 1023	:	//	<Nutmeg Log>	<厚磊唱公>
	case 1024	:	//	<唱公>	<府扁促唱公>
	case 1025	:	//	<Fir Log>	<备惑唱公>
	case 1026	:	//	<Oak Log>	<惑荐府唱公>
	case 1027	:	//	<唱公>	<芭力荐唱公>
	case 1028	:	//	<Beech Log>	<呈档广唱公>
	case 1029	:	//	<Whitebirch Log>	<磊累唱公 >
	case 1030	:	//	<Nutmeg Timber>	<厚磊唱公 格犁>
	case 1031	:	//	<唱公>	<府扁促唱公 格犁>
	case 1032	:	//	<Fir Timber>	<备惑唱公 格犁>
	case 1033	:	//	<Oak Timber>	<惑荐府唱公 格犁>
	case 1034	:	//	<唱公>	<芭力荐唱公 格犁>
	case 1035	:	//	<Beech Timber>	<呈档广唱公 格犁>
	case 1036	:	//	<Whitebirch Timber>	<磊累唱公 格犁>
		break;
	default :		SendSkillResultFail(cn, skillno);		return 0;
	}
	
	if( ( ItemList[ target_id].attr[ IATTR_ATTR] & IA2_FIRE ) == 0 )
	{
		ItemList[ target_id].attr[ IATTR_ATTR] |= IA2_FIRE;
		ItemList[ target_id].attr[ IATTR_ATTR] |= IA2_NOTMOVE;
	
		// 盔贰绰 父甸绢龙锭 矫埃阑 啊扁绊 父甸绢柳促. 
		ItemList[ target_id].attr[IATTR_DURATION ] = g_curr_time + ITEM_FIRE_GROW_TIME;
			
		SetArea( CHANGE_IA2_ATTR_ITEM_AREA, target_id );
		SendSkillResult( cn, skillno, target_id, x, y, 1 );
	}		
					
	return 1; 
}			

// 1. 某腐磐狼 n->tame
// 2. 矫傈磊狼 taming skil (SkillMotherExp 鸥涝) Level捞   
//  	NPC_Gen_Ref[n->SprNo].Taming_min焊促 臭酒具 Taming且荐 乐促.
// 3. 
//1018 zhh 葛滴 官诧 窃荐
//曼绊 磊丰
//#define SKILLFAIL_LOW_LEVEL_TO_TAMING		-110	//taming且 荐 绝绰 瓷仿摹甫 蜡历啊 啊瘤绊 乐促
//#define SKILLFAIL_NO_TAMING_ALLOWED		-111	//taming且 荐 绝绰 措惑
//#define SKILLFAIL_TARGET_IS_NOT_NPC		-112	//taming且 胶 乐绰 NPC啊 酒聪促
int skill_Taming			(const int cn, const int skillno, const int x, const int y, const int targettype, const int target_id )
{		
	int why=0, hitrate=0, r=0;
	int needed_max_tame;

	if( target_id - 10000 < NPC_LIST_START || target_id - 10000 >= MAX_NPC_LIST ) 
	{	
		//1018 zhh
		why=SKILLFAIL_TARGET_IS_NOT_NPC;
		SendSkillResultFail(cn, skillno,why); return 0;
	}
	//咯扁鳖瘤 坷搁 NPC啊 酒聪绊 弥措 NPC锅龋 焊促 利促
	
	if( target_id < 10000 ) { why=SKILLFAIL_TARGET_IS_NOT_NPC;	SendSkillResultFail(cn, skillno,why); return 0; }
	//咯扁鳖瘤 坷搁 老窜 NPC促

	CHARLIST *n = &NPCList[ target_id - 10000];			
	if( n->tamingable == 0 )
	{				
		why=SKILLFAIL_NO_TAMING_ALLOWED;
		SendSkillResultFail(cn, skillno,why); return 0;
	}			
	//咯扁鳖瘤 坷搁 抛捞怪 且 荐 乐绰 NPC扼绰 捞具扁
	if( NPC_Gen_Ref[n->SprNo].Taming_min >= 100 )	
	{		
		why=SKILLFAIL_NO_TAMING_ALLOWED;
		SendSkillResultFail(cn, skillno,why); return 0;
	}
	// 咯扁鳖瘤 坷搁 抛捞怪 且 荐 乐绰 NPC扼绰 捞具扁
	if (n->IsTamedNpc() || n->IsSummonMonster())
	{
		SendSkillResultFail(cn, skillno ); return 0;
	}

	int MyLevel = connections[ cn].chrlst.skillexp[ SkillTbl[skillno].MotherSkillType].skillexp / 10000;
	int MonLevel = n->GetLevel(); // CSD-030806

	if(		MonLevel>MyLevel)	hitrate = 70 - (MonLevel-MyLevel)*1.2;	
	else if(MonLevel==MyLevel)	hitrate = 70;	
	else if(MonLevel<MyLevel)	hitrate = 70 + ((MyLevel-MonLevel)*5)/8;	

	if(hitrate>95)	hitrate=95;
	if(hitrate<0)	hitrate=0;

	needed_max_tame = NPC_Gen_Ref[n->SprNo].Taming_max;

	AddSkillExp( cn, skillno, hitrate );

	// 捞力何磐 林牢捞 绝绊, 捞仇阑 怕怪窍绰 荤恩档 林困俊 绝促. 

	r=rand()%100;
	if( hitrate > r )
	{
		int add=ReturnTamingPoint( cn ,skillno, target_id );
		
		n->tame += add;

		if(n->tame < 0)		n->tame=0;	// 俊矾八免风凭.. 
		if( n->tame > needed_max_tame ) n->tame = needed_max_tame;
	}

	// 怕怪俊 己傍窍看促. 
	if( n->tame >= needed_max_tame )
	{	//< CSD-031106
		n->SetTamed(true);
		strcpy(n->HostName, connections[cn].chrlst.Name);
		n->SetMaster(cn);
		
		if (n->patterntype != NPC_PATTERN_TAME_)
		{
			n->oldpatterntype	= n->patterntype;
		}

		n->patterntype = NPC_PATTERN_TAME_;
		n->tame_cmd	= LC_TAME_STOP;

		SendNPC_Parameter(n->GetServerID(), NPC_PARAMETER_TAME_CMD,	n->tame_cmd);
		SendNPC_Parameter(n->GetServerID(), NPC_PARAMETER_OLDPATTERNTYPE, n->oldpatterntype);
		SendNPC_Parameter(n->GetServerID(), NPC_PARAMETER_PATTERNTYPE, n->patterntype);
		SendNPC_Parameter(n->GetServerID(), NPC_PARAMETER_ATTRIBUTE, AT_TAMING);

		n->tamingcountdown	=  60;
		SendSkillResult(cn, skillno, target_id, x, y, 200);
		SendToAllHostName(cn, n);
		return 1; 
	}	//> CSD-031106

	SendSkillResult( cn, skillno, target_id, x, y, n->tame );
	return 1; 
}


int skill_Evaluate_Inteligence	(const int cn, const int skillno, const int x, const int y, const int targettype, const int target_id )
{
	LPCHARLIST ch;
	int ret = ReturnSkillProbablity( cn, skillno, x, y, targettype, target_id );
	if( ret == 0 ) { goto FAIL_; }

	if( target_id >= 10000 )	
	{
		if( target_id -10000 < NPC_LIST_START || target_id -10000 >= MAX_NPC_LIST ) goto FAIL_;
		ch = &NPCList[ target_id - 10000];

	}
	else
	{
		if( target_id < DRAGON_CONNECTIONS_START || target_id >= DRAGON_MAX_CONNECTIONS ) goto FAIL_;
		ch = &connections[ target_id].chrlst;
	}
	if( ch->bAlive == DEAD_ ) goto FAIL_;
	SendViewCharInfo( cn,  target_id, INT_, ch->Int, NULL );

	
	return 1; 

FAIL_:

	SendSkillResultFail(cn, skillno); 
	return 0; 
}

int skill_Evaluate_Wisdom		(const int cn, const int skillno, const int x, const int y, const int targettype, const int target_id )
{
	LPCHARLIST ch;
	int ret = ReturnSkillProbablity( cn, skillno, x, y, targettype, target_id );
	if( ret == 0 ) { goto FAIL_; }

	if( target_id >= 10000 )	
	{
		if( target_id -10000 < NPC_LIST_START || target_id -10000 >= MAX_NPC_LIST ) goto FAIL_;
		ch = &NPCList[ target_id - 10000];
	}
	else
	{
		if( target_id < DRAGON_CONNECTIONS_START || target_id >= DRAGON_MAX_CONNECTIONS ) goto FAIL_;
		ch = &connections[ target_id].chrlst;
	}

	if( ch->bAlive == DEAD_ ) goto FAIL_;
	SendViewCharInfo( cn,  target_id, WIS, ch->Wis, NULL );
	
	return 1; 

FAIL_:

	SendSkillResultFail(cn, skillno); 
	return 0; 
}

int skill_Anatomy_Judgement		(const int cn, const int skillno, const int x, const int y, const int targettype, const int target_id )
{
	LPCHARLIST ch;
	int ret = ReturnSkillProbablity( cn, skillno, x, y, targettype, target_id );
	if( ret == 0 ) { goto FAIL_; }

	if( target_id >= 10000 )	
	{
		if( target_id - 10000 < NPC_LIST_START || target_id - 10000 >=MAX_NPC_LIST ) goto FAIL_;
		ch = &NPCList[ target_id - 10000];
	}
	else
	{
		if( target_id < DRAGON_CONNECTIONS_START || target_id >= DRAGON_MAX_CONNECTIONS ) goto FAIL_;
		ch = &connections[ target_id].chrlst;
	}

	if( ch->bAlive != DEAD_ ) goto FAIL_;

	SendViewCharInfo( cn, target_id,  9999, 0, ch->KillerName );
	
	return 1; 

FAIL_:

	SendSkillResultFail(cn, skillno); 
	return 0; 
}

int skill_Item_Identification	(const int cn, const int skillno, const int x, const int y, const int targettype, const int target_id ){ SendSkillResult( cn, skillno, target_id, x, y, -999 );return 1; }
int skill_Crapple				(const int cn, const int skillno, const int x, const int y, const int targettype, const int target_id ){ SendSkillResult( cn, skillno, target_id, x, y, -999 );return 1; }
int skill_Swordmanship			(const int cn, const int skillno, const int x, const int y, const int targettype, const int target_id ){ SendSkillResult( cn, skillno, target_id, x, y, -999 );return 1; }
int skill_Archery				(const int cn, const int skillno, const int x, const int y, const int targettype, const int target_id ){ SendSkillResult( cn, skillno, target_id, x, y, -999 ); return 1; }
int skill_Fencing				(const int cn, const int skillno, const int x, const int y, const int targettype, const int target_id ){ SendSkillResult( cn, skillno, target_id, x, y, -999 );return 1; }
int skill_Mace_Fighting			(const int cn, const int skillno, const int x, const int y, const int targettype, const int target_id ){ SendSkillResult( cn, skillno, target_id, x, y, -999 );return 1; }
int skill_Pierce				(const int cn, const int skillno, const int x, const int y, const int targettype, const int target_id ){ SendSkillResult( cn, skillno, target_id, x, y, -999 );return 1; }
int skill_Whirl					(const int cn, const int skillno, const int x, const int y, const int targettype, const int target_id ){ SendSkillResult( cn, skillno, target_id, x, y, -999 );return 1; }
int skill_Hurl					(const int cn, const int skillno, const int x, const int y, const int targettype, const int target_id ){ SendSkillResult( cn, skillno, target_id, x, y, -999 );return 1; }
int skill_Parrying				(const int cn, const int skillno, const int x, const int y, const int targettype, const int target_id ){ SendSkillResult( cn, skillno, target_id, x, y, -999 );return 1; }
int skill_Double_Swordmanship	(const int cn, const int skillno, const int x, const int y, const int targettype, const int target_id ){ SendSkillResult( cn, skillno, target_id, x, y, -999 );return 1; }
int skill_Double_Macefighting	(const int cn, const int skillno, const int x, const int y, const int targettype, const int target_id ){ SendSkillResult( cn, skillno, target_id, x, y, -999 );return 1; }
int skill_Magery				(const int cn, const int skillno, const int x, const int y, const int targettype, const int target_id ){ SendSkillResult( cn, skillno, target_id, x, y, -999 );return 1; }
int skill_Orison				(const int cn, const int skillno, const int x, const int y, const int targettype, const int target_id ){ SendSkillResult( cn, skillno, target_id, x, y, -999 );return 1; }
	
void AddSkill( lpMAPSKILLTABLE *Header, lpMAPSKILLTABLE	lpST )		//傅农靛 府胶飘狼 赣府俊 嘿咯 持扁
{		
	lpMAPSKILLTABLE t, temp; 
		
	if( *Header == NULL )		//傅农靛 府胶飘 弥檬 积己
	{	
		MemAlloc( *Header, sizeof( MAPSKILLTABLE ));

		(*Header)->type			= lpST->type;
		(*Header)->x			= lpST->x; 
		(*Header)->y			= lpST->y; 
		(*Header)->skillno		= lpST->skillno;
		(*Header)->tile_Range	= lpST->tile_Range;
		(*Header)->subType		= lpST->subType;
		(*Header)->probability	= lpST->probability;
		(*Header)->type_Num		= lpST->type_Num;

		(*Header)->prev			= NULL; 
		(*Header)->next			= NULL; 
	}	
	else						//捞固 积己等 府胶飘俊 梅啊 楷搬
	{	
		t = NULL;
		MemAlloc( t, sizeof( MAPSKILLTABLE ));

		t->type			= lpST->type;
		t->x			= lpST->x; 
		t->y			= lpST->y; 
		t->skillno		= lpST->skillno;
		t->tile_Range	= lpST->tile_Range;
	 	t->subType		= lpST->subType;
		t->probability	= lpST->probability;
		t->type_Num		= lpST->type_Num;
		
		temp = *Header ;
		temp->prev = t;
		t->next = temp;
		t->prev = NULL;
		*Header = t;
	}	
}

void DeleteSkillLink( lpMAPSKILLTABLE lpHeader )
{	
	if( !lpHeader ) return;
	if( lpHeader->next ) 
	{
		DeleteSkillLink( lpHeader->next );	
	}
	MemFree( lpHeader );
}

void DeleteSkillAll()
{
	for( int i=0; i<8; i++ )
	{
		for( int j=0; j<8; j++ )
		{
			DeleteSkillLink( g_Header[i][j] );
		}
	}
}

void DeleteSkill( lpMAPSKILLTABLE *Header, lpMAPSKILLTABLE f)
{
	lpMAPSKILLTABLE  t = *Header;//g_lpMapSkillTable;

	while( t != NULL )
	{
		if( t == f )
		{
			if( f == *Header )		//header啊 力芭登绰 版快
			{
				t = (*Header)->next;
		
				if(*Header!=NULL)
					MemFree( *Header );
		
				if(t!=NULL)
				{
					*Header = t;
					(*Header)->prev = NULL;
				}
				return;
			}
			else 
			{
				if ( t->prev != NULL )
				{
					t->prev->next = t->next;
				}
				if( t->next != NULL )
				{
					t->next->prev = t->prev;
				}
				MemFree( t );
				return;
			}
		}
		t= t->next;
	}	
}
		
//	甘篮 例措谅钎...
lpMAPSKILLTABLE FindSkill( lpMAPSKILLTABLE *H, int x, int y )
{	   	
	lpMAPSKILLTABLE t;
	t = *H;
	while( t != NULL )
	{	
		if( t->x == x && t->y == y )
		{
			return t;
		}
		t = t->next;
	}			
	return NULL;
}			
	

//1017 zhh	//惑寸 何盒 荐沥 登菌嚼聪促
//--------------------------------------------------------------------------------------------
//	扁  瓷 : 1矫埃(角力 矫埃. 扼磊俊辑绰 6h)付促 磊楷俊辑 掘绢尘荐 乐绰 樊捞  檬扁拳等促. 
//	牢  磊 : 
//	搬  苞 : 
//--------------------------------------------------------------------------------------------
/*------------------------------------------------------------------------------------------------------------
  扁 瓷 : SkillMapTable俊辑 弊瘤开俊辑 惯积且荐 乐绰 利寸茄 酒捞袍锅龋甫 府畔秦霖促. 
  牢 磊 : type  :   TOOL_FARMING			1
				 	 TOOL_MINING			2
					 TOOL_FISHING			3
					 TOOL_CHOPPING			4
					 TOOL_HUB				5
					 TOOL_NPC_GENER			6
					 TOOL_DONTSKILL			7
	    tableno :  0 - 10 
	    hitrate :  0 - 100
  搬 苞 : 酒捞袍锅龋甫 府畔茄促. 
------------------------------------------------------------------------------------------------------*/
int ReturnFromSkillMapTable(const int cn, const int iSkillToolType, const int iTableno, const int iHitrate ,int iReturnType)
{				
	int i = 0;
	int iReturnValue = 0;
	t_SkillMapTable *smt;
	int maxdemand = -9999;
	t_SkillMapTable smtsad = SkillMapTable[0];
	
	if( 10 < iTableno || 0 > iTableno) {return 0;}
	CHARLIST *ch = GetCharListPtr(cn);
	if(!ch){ return 0;}

	for( i = 0 ; i < SkillMapTableCount ; i ++)
	{	
		smt = &SkillMapTable[ i];

		if( smt->Skill_type == iSkillToolType && smt->table[iTableno] )
		{
			if( smt->demand <= iHitrate )
			{
				if( maxdemand < smt->demand )
				{
					switch(iReturnType)
					{
					case RETURN_SMT_DEMAND:
						iReturnValue  = smt->demand;
						break;
					case RETURN_SMT_ITEMNO:
						iReturnValue = smt->Item_id;//020507 lsw
						smtsad = *smt;//020401 lsw
						break;
					}
				}
			}
		}
	}	
	if(iReturnType	== RETURN_SMT_ITEMNO)
	{
		if(smtsad.new_item[0] && smtsad.new_item_pro[0] )
		{
			if(iReturnValue)
			{
				const int r = rand()%10000;

				if( smtsad.new_item_pro[0] > r )
				{
					if( -1 == SkillMgr.FindLearnItem(ch,iReturnValue))
					{
//						GiveSadonixToGround(cn, smtsad.new_item[0],iSkillToolType,MapInfo[MapNumber].port,ch->X, ch->Y);//020110 lsw
					}
				}
			}
		}

		if(smtsad.new_item[1] && smtsad.new_item_pro[1] )
		{
			if(iReturnValue)
			{
				const int r = rand()%10000;

				if( smtsad.new_item_pro[1] > r )
				{
					if( -1 == SkillMgr.FindLearnItem(ch,iReturnValue))
					{
//						GiveSadonixToGround(cn, smtsad.new_item[1],iSkillToolType,MapInfo[MapNumber].port,ch->X, ch->Y);//020110 lsw
					}
				}
			}
		}
	}

	return iReturnValue;
}		

//-------------------------------------------------------------------------------------------------------		
//	扁 瓷 : 扁贱阑 佬绢甸捞搁辑  NPC 积己困摹甫 棱绰促. 	
//	牢 磊 :		
//	搬 苞 :		
//-------------------------------------------------------------------------------------------------------
BOOL LoadSkillMapTable(void)
{
	int		i=0;
	char	temp[FILENAME_MAX];
	FILE *fp ;	
	MAPSKILLTABLE st;
	int t;
				
	t_earthpower *ep;
				
	sprintf( temp, "%s/map/%s.skb", GameServerDataPath, MapName );
	fp = fopen( temp, "rb" );
	if(fp == NULL)
	{
		MyLog(LOG_NORMAL, "[SKB] Missing map resource: %s", temp);
		return FALSE;
	}

	// 旧 SKB 保存的是 32 位进程结构；磁盘上的指针仅为占位，不能读入运行时。
	if (fseek(fp, 0, SEEK_END) != 0)
	{
		fclose(fp);
		return FALSE;
	}
	const long fileSize = ftell(fp);
	if (fileSize < 0 || fileSize % sizeof(LegacySkillMapRecord) != 0 ||
		fseek(fp, 0, SEEK_SET) != 0)
	{
		MyLog(LOG_NORMAL, "[SKB] Invalid file length: %s (%ld)", temp, fileSize);
		fclose(fp);
		return FALSE;
	}
				
	int dx = (g_Map.file.wWidth+ 7)/8;
	int dy = (g_Map.file.wHeight+7)/8;
	if (dx <= 0 || dy <= 0)
	{
		fclose(fp);
		return FALSE;
	}
				
	ep = &EarthPower;
				
	ep->reclaimcount = 0;
	ep->miningcount  = 0;
	ep->fishingcount = 0;
	ep->cuttingcount = 0;
	ep->diggingcount = 0;

	//<! BBD 040311	
	float Rate = GetResourceRate(g_UsercountInServerSet);	// 磊盔硅啦阑 备秦柯促
	MyLog(0, "Starting ResourceRate = %.1f, cause UsercountInServerSet = %d", Rate, g_UsercountInServerSet);
	//> BBD 040311	
	
	int recordCount = 0;
	int npcCount = 0;
	const long expectedRecords = fileSize / sizeof(LegacySkillMapRecord);
	while (recordCount < expectedRecords)
	{
		LegacySkillMapRecord disk;
		if (fread(&disk, sizeof(disk), 1, fp) != 1 ||
			!IsValidLegacySkillMapRecord(disk, g_Map.file.wWidth, g_Map.file.wHeight) ||
			(disk.skillno == 6 && (disk.typeNum < 0 || disk.typeNum >= Num_Of_NPC_Generation)))
		{
			MyLog(LOG_NORMAL, "[SKB] Invalid or incomplete record %d in %s", recordCount, temp);
			fclose(fp);
			return FALSE;
		}
		DecodeLegacySkillMapRecord(disk, st);
		++recordCount;
				
		int x = (int)(st.x / dx);
		int y = (int)(st.y / dy);
		if (x < 0 || x >= 8 || y < 0 || y >= 8)
		{
			fclose(fp);
			return FALSE;
		}
		TileMap[ st.x ][ st.y ].attr_skill = 1;
							
		// 版累..		
		// EarthPower[]	绰 0 - 10000狼 荐摹甫 啊柳促. 弥檬 距 0-2000狼 荐摹甫 啊瘤绊 矫累茄促. 
		t = st.probability <= 0 ? 0 : (st.probability >= 100 ? 1000 : st.probability * 10);
		//1018 zhh
		if( t > 1000 ) t = 1000;
		if( t < 0    ) t = 0;

		t = (int)(t * Rate);		// BBD 040311	

		switch( st.skillno )
		{				
			// 版累..	
		case 1 :	if( ep->reclaimcount < MAX_EARTHSKILL )	
					{												
						ep->reclaim[ ep->reclaimcount] = 700 + (rand()%1300);
						st.type_Num = ep->reclaimcount ++;	
					}	
			break;		
			// 盲堡.	
		case 2 :	if( ep->miningcount < MAX_EARTHSKILL )	
					{												
						ep->mining_old[ ep->miningcount] = ep->mining[ ep->miningcount] = t;
						st.type_Num = ep->miningcount ++;	
					}	
			break;		
			// 超矫.	
		case 3 :	if( ep->fishingcount < MAX_EARTHSKILL )	
					{												
						ep->fishing_old[ ep->fishingcount] = ep->fishing[ ep->fishingcount] = t;
						st.type_Num = ep->fishingcount ++;	
					}			
			break;		
			// 国格.	
		case 4 :	if( ep->cuttingcount < MAX_EARTHSKILL )	
					{												
						ep->cutting_old[ ep->cuttingcount] = ep->cutting[ ep->cuttingcount] = t;
						st.type_Num = ep->cuttingcount ++;	
					}				
			break;		
			// 距檬.	
		case 5 :	if( ep->diggingcount < MAX_EARTHSKILL )	
					{												
						ep->digging_old[ ep->diggingcount] = ep->digging[ ep->diggingcount] = t;
						st.type_Num = ep->diggingcount ++;	
					}			
			break;		
			// NPC积己. 
		case 6 : 	//	tile_Range	-> 弥措 惯积荐.
					//	probability	-> 胶农赋飘锅龋( 0 - 100 ).
					//	type_Num	-> NPC 绊蜡锅龋.
					NPCGeneratePosition( st.type_Num, st.x, st.y,  st.probability, st.tile_Range );
					++npcCount;
			break;
		}
		
		if (st.skillno>0) AddSkill( &g_Header[ x][y], &st );	// LTS DRAGON MODIFY
	}	
	fclose(fp);
	MyLog(LOG_NORMAL, "[SKB] Loaded %d records, %d NPC generation positions: %s",
		recordCount, npcCount, temp);
		
	return TRUE;	
}		
	
DWORD ReturnHowLongTimeGrow( LPITEMLIST i )
{	
	CItem *ii = ItemUnit( i->item_no/1000, i->item_no % 1000 );

	if( ii )	
	{
//		return ii->Getmouse_MR_Effect();
		return ii->Getmouse_Mr_Wave();
	}

	return 0;
}			
			

	
void RecvCMD_FOOT_ON_TRAP_0_ITEM(const int cn, t_client_foot_on_trap0 *p )
{
  if( !CheckServerId( cn ) ) return;
	int id = p->idServer;
	if( id < 0 ) return ;
	if( id >= MAX_ITEM_LIST ) return;
	
	if( connections[cn].chrlst.bAlive == ALIVE_ )
	if( ItemList[ id].bAlive == ALIVE_ )
	{
		if( ItemList[ id].attr[ IATTR_ATTR] & IA2_TRAPED0 )
		{
//			SetArea( CHANGE_FOOT_ON_TRAP0_AREA, id );
			SendItemFoot_On_Trap0( connections, cn, id ); // ####			
			RemoveItemList( id );
		}
	}
}	
	
void RecvCMD_FOOT_ON_TRAP_1_ITEM(const int cn, t_client_foot_on_trap1 *p )
{
  if( !CheckServerId( cn ) ) return;
	int id = p->idServer;
	if( id < 0 ) return ;
	if( id >= MAX_ITEM_LIST ) return;

	if( connections[cn].chrlst.bAlive == ALIVE_ )
	if( ItemList[ id].bAlive == ALIVE_ )
	{
		if( ItemList[ id].attr[ IATTR_ATTR] & IA2_TRAPED1 )
		{
//			SetArea( CHANGE_FOOT_ON_TRAP1_AREA, id );
			SendItemFoot_On_Trap1( connections, cn, id ); 
			RemoveItemList( id );
		}
	}
}

void RecvSkillGauge(const int cn, t_client_skill_gauge *p )
{					
	t_packet packet;	
	int probablity;	
	lpMAPSKILLTABLE	 ms;
	t_earthpower *ep;
					
	packet.h.header.type = CMD_SKILL_GAUGE;
	packet.h.header.size = sizeof( t_server_skill_gauge );
					
	packet.u.server_skill_gauge.skillno = p->skillno;
	packet.u.server_skill_gauge.x		= p->x;
	packet.u.server_skill_gauge.y		= p->y;
					
	ep = &EarthPower;

	switch( p->skillno )
	{				
		// 俺埃...	
	case 21 :	ms = skill_ReturnProbablityFromSkillTable( &probablity,  1, p->x, p->y );
				if( ms )
				{	
					packet.u.server_skill_gauge.min = ep->reclaim[ ms->type_Num ]/100;
					QueuePacket( connections, cn, &packet, 1 );
					return;
				}	
		break;		
	}				
}					

////////////////////////////////////////////////////////////////////////////////////////
//                                    版  力
////////////////////////////////////////////////////////////////////////////////////////
// Item啊拜 炼沥 
const char itemtypename[ 11][20] = {	"Plant",		
								"Mineral",		
								"Herb",		
								"Cook",		
								"Potion",		
								"Tool",		
								"Weapon",		
								"Disposable",	
								"Armor",		
								"Accessory",	
								"Etc"};

void RecvDebugChangeSkillExp( t_debug_change_skill_exp *tp )
{
	LPCHARLIST ch;
	int i;

	if( connections[tp->id].dwAgentConnectionIndex && connections[tp->id].state >= CONNECT_JOIN)
	{
		ch = &connections[tp->id].chrlst;
	}
	else return;

	for( i = 0 ; i < 45 ; i ++)
	{
		ch->skillexp[i].skillexp = tp->skillexp[i];
	}
}

int SkillLevelExpLoad()
{
	HSTMT hStmt= NULL ;
	RETCODE ret ;
	SWORD nCols ;
	int c=0;
	
	SQLAllocStmt(hDBC, &hStmt);
	ret= SQLExecDirect(hStmt, (UCHAR *)"select * from skill_lvexp order by probability", SQL_NTS) ;
	if(ret != SQL_SUCCESS_WITH_INFO && ret != SQL_SUCCESS) 
	{
		MyLog(LOG_NORMAL, "skill_lvexpl: ExecDirect Error ") ;
		return -1 ;
	}

	SQLNumResultCols(hStmt, &nCols) ;
	
	ret= SQLFetch(hStmt) ;
	if(ret != SQL_SUCCESS_WITH_INFO && ret != SQL_SUCCESS) 
	{
		MyLog(LOG_NORMAL, "skill_lvexpl: Fetch Error ") ;
		return -1 ;
	}
	
	while( ret == SQL_SUCCESS)
	{
		// Accquire the DATA		
		SQLLEN  cbCustID , cbexp;
		SQLINTEGER  sCustID, exp;
		ret= SQLGetData(hStmt, 1, SQL_C_SLONG, &sCustID, 0, &cbCustID);	
		if(ret != SQL_SUCCESS_WITH_INFO && ret != SQL_SUCCESS) 
		{
			MyLog(LOG_NORMAL, "skill_lvexpl: Error at 1 %d ", ret) ;
			return -1 ;
		}

		if( sCustID < 0 || sCustID > SKILL_LEVEL_EXP_TABLE_MAX_ ) 
		{
			MyLog(LOG_NORMAL, "skill_lvexpl:  Data is Invalid...( sCusID = %d )", sCustID ) ;
			return -1;
		}

		for(int i=2;i<103;i++)
		{
			ret= SQLGetData(hStmt, i, SQL_C_SLONG, &exp, 0, &cbexp) ;
			if(ret != SQL_SUCCESS_WITH_INFO && ret != SQL_SUCCESS) 
			{
				MyLog(LOG_NORMAL, "skill_lvexpl:  Error at %d ", c ) ;
				return -1 ;
			}
			SkillLevelExp[sCustID][i-2]=exp;
		}
		c++;
		ret= SQLFetch(hStmt) ;
	}
	SQLFreeStmt(hStmt, SQL_DROP) ;

	return c ;
}

int SkillExpLoad()
{
	HSTMT hStmt= NULL ;
	RETCODE ret ;
	SWORD nCols ;
	char query_stmt[80]= {0, } ;
	int c=0;
	
	SQLAllocStmt(hDBC, &hStmt);
	
	strcpy(query_stmt, "select * from skill_exp order by probability") ;
	ret= SQLExecDirect(hStmt, (UCHAR *)query_stmt, SQL_NTS) ;
	if(ret != SQL_SUCCESS_WITH_INFO && ret != SQL_SUCCESS) 
	{
		MyLog(LOG_NORMAL,"skill_exp: ExecDirect Error ") ;
		return -1 ;
	}

	SQLNumResultCols(hStmt, &nCols) ;
	
	ret= SQLFetch(hStmt) ;
	if(ret != SQL_SUCCESS_WITH_INFO && ret != SQL_SUCCESS) 
	{
		MyLog(LOG_NORMAL, "skill_exp: Fetch Error ") ;
		return -1 ;
	}
	
	while( ret == SQL_SUCCESS)
	{
		SQLLEN  cbCustID , cbexp;
		SQLINTEGER  sCustID, exp;
		ret= SQLGetData(hStmt, 1, SQL_C_SLONG, &sCustID, 0, &cbCustID);	
		if(ret != SQL_SUCCESS_WITH_INFO && ret != SQL_SUCCESS) {
			MyLog(LOG_NORMAL, "skill_exp: Error at 1" ) ;
			return -1 ;
		}

		if( sCustID < 0 || sCustID > SKILL_EXP_TABLE_MAX_ ) 
		{
			MyLog(LOG_NORMAL, "skill_exp: Data is Invalid...( sCusID = %d )", sCustID ) ;
			return -1;
		}

		for(int i=2;i<103;i++)
		{
			ret= SQLGetData(hStmt, i, SQL_C_SLONG, &exp, 0, &cbexp) ;
			if(ret != SQL_SUCCESS_WITH_INFO && ret != SQL_SUCCESS) 
			{
				MyLog(LOG_NORMAL, "skill_exp: Error at %d", c ) ;
				return -1 ;
			}
			SkillExp[sCustID][i-2]=exp;
		}
		c++;
		ret= SQLFetch(hStmt) ;
	}
	SQLFreeStmt(hStmt, SQL_DROP) ;
	return c ;
}

//63(BETWEEN_POS) = 60(NPC_ITEM_TABLE_TOTAL_ITEM) + 2(NPC_ITEM_TABLE_START_ITEM_POS) + 1(促澜捞聪鳖)
int Load_NPC_ItemGenerateTable()									//DB俊辑 抛捞喉阑 佬绢 柯促
{
	HSTMT hStmt= NULL ;
	RETCODE ret ;
	SWORD nCols ;
	int c=0;
	
	SQLAllocStmt(hDBC, &hStmt);
	ret= SQLExecDirect(hStmt, (UCHAR *)"select * from mon_bag order by bagno", SQL_NTS) ;
	if(ret != SQL_SUCCESS_WITH_INFO && ret != SQL_SUCCESS) 
	{
		MyLog(LOG_NORMAL, "mon_bag : ExecDirect Error ") ;
		return -1 ;
	}

	SQLNumResultCols(hStmt, &nCols) ;
	
	ret= SQLFetch(hStmt) ;
	if(ret != SQL_SUCCESS_WITH_INFO && ret != SQL_SUCCESS) 
	{
		MyLog(LOG_NORMAL, "mon_bag: Fetch Error ") ;
		return -1 ;
	}
	
	for(int x=0;x<NPC_ITEM_TABLE_X;x++)
	{
		SQLLEN  cbitemno;
		SQLINTEGER  itemno;
		for(int i=1;i<NPC_ITEM_TABLE_Y+1;i++)
		{
			ret= SQLGetData(hStmt, i, SQL_C_SLONG, &itemno, 0, &cbitemno) ;
			if(ret != SQL_SUCCESS_WITH_INFO && ret != SQL_SUCCESS) 
			{
				MyLog(LOG_NORMAL, "mon_bag: Error at %d", i ) ;
				return -1 ;
			}
			Npc_Item_Table[x][i-1]=itemno;
		}
		c++;
		ret= SQLFetch(hStmt) ;
	}
	SQLFreeStmt(hStmt, SQL_DROP) ;
	return c ;
}

int LoadSkillLevelTable(void)
{
	if(SkillLevelExpLoad()<0)			return -1;
	if(SkillExpLoad()<0)				return -2;
	if(Load_NPC_ItemGenerateTable()<0)	return -3;
	return 1;
}

/*------------------------------------------------------------------------------------------------------------
  扁 瓷 : SkillMapTable俊辑 弊瘤开俊辑 惯积且荐 乐绰 利寸茄 demand甫 府畔秦霖促. 
  牢 磊 : type  :   TOOL_FARMING			1
			 		TOOL_MINING				2
					TOOL_FISHING			3
					TOOL_CHOPPING			4
					TOOL_HUB				5
					TOOL_NPC_GENER			6
					TOOL_DONTSKILL			7
	    tableno :  0 - 10 
	    hitrate :  0 - 100
  搬 苞 : demand甫 府畔茄促. 
------------------------------------------------------------------------------------------------------*/
int GiveBasicItem(const int cn,const int iSkillTooltype,const int subType)//020214 胶懦 矫傈矫 角菩秦档 砒俊 蝶弗 扁夯 酒捞袍 力傍 1/15
{
	CHARLIST *ch = GetCharListPtr(cn);
	if(!ch){return 0;}
	if(!(rand()%15))
	{
		const int sx = ch->X;
		const int sy = ch->Y;
		const int itemno =ReturnFromSkillMapTable(cn,iSkillTooltype,subType,14,RETURN_SMT_ITEMNO);//020214 lsw
		ItemAttr item = GenerateItem(itemno);
		DropItem( sx-50+rand()%100, sy-50+rand()%100, &item);
		return 1;
	}
	return 0;
}

void DecreaseEquipDuration( CHARLIST *ch, const DWORD dwDecValue, const int iItemPos, const int iTargetSprNo, const bool bIsAttacker,bool bIsNoCount)
{	
	POS p;
	ItemAttr *item		=	&ch->equip[iItemPos];
	CItem *t			=	ItemUnit( item->item_no );	
	int iSendValue		= 0;
	const int iValue	= abs((int)dwDecValue);
	const int cn		= ch->GetServerID();
	
	if( NULL == t){return;}
	if( SPRITETYPE_NPC 	==  ch->SprType){return;}
	if( 96 	==  iTargetSprNo){return;}

	if( DIVIDE_ITEM == t->GetRbutton() )//郴备档侩 酒捞袍
	{	// DIVIDE_ITEM老版快 attack_pow父怒 猾促. 
		if(bIsAttacker)
		{
			iSendValue = ItemMgr.DecItemDur(*item,iValue);
		}
		else
		{
			iSendValue = ItemMgr.DecItemDur(*item,1);
		}
	}
	else	//郴备档啊 酒囱 酒捞袍
	{	
		int t = 0;
		if(!bIsNoCount)
		{
			ch->dec_duration++ ;
			if( ch->dec_duration > 30 ) ch->dec_duration = 0;
			else return;
			t = (int) log( abs(iValue)+1);
			if( t > 14 ) t = 14;
			if( t < 1 ) t = 1;
		}
		else//墨款飘 棺 log 绝捞 官肺 盔窍绰 父怒 皑家
		{	
			t = iValue;
		}
		iSendValue = ItemMgr.DecItemDur(*item,t);
	}
	
	switch(iSendValue)
	{
	case 1:
		{	
			::SetItemPos( EQUIP, iItemPos, &p );
			::BreakItemByDuration( item, ch, p );
		}
		break;
	case 2:
		{
			::SendItemAttrDurationChange( connections, cn, iItemPos );
		}break;
	default:
		break;
	}
}				

int OutMessage(const int cn, const DWORD para1, const DWORD para2)
{
	return OutMessage( ::CheckServerId(cn), para1, para2);
}

int OutMessage( LPCHARLIST ch, const DWORD para1, const DWORD para2)
{
	if(!ch){return 0;}

	if( !ch->IsPlayer() ) {return 0;}
	
	t_packet p;	
	p.h.header.type = CMD_OUTPUT_MESSAGE;
	{
		p.u.server_output_message.para1 = para1;
		p.u.server_output_message.para2 = para2;
	}
	p.h.header.size = sizeof( t_server_output_message );
	QueuePacket( connections, ch->GetServerID(), &p, 1);
	return 1;
}

int OutSound(LPCHARLIST ch, const int iFileType, const int iNo)
{
	if(!ch){return 0;}
	if(!ch->IsPlayer() ){return 0;}
	
	t_packet p;	
	p.h.header.type = CMD_SOUND_OUT;
	{
		p.u.server_output_message.para1 = iFileType;
		p.u.server_output_message.para2 = iNo;
	}
	p.h.header.size = sizeof( t_server_output_message );
	::QueuePacket( connections, ch->GetServerID(), &p, 1);
	return 1;
}

void SkillMapper( const int cn, const int iSkillNo, const int x, const int y, const int targettype, const int target_id, const POS s )
{	//< CSD-030821
	CHARLIST *ch = CheckServerId(cn);
	
	if (ch == NULL)
	{
		return;
	}

	if (target_id < 0)
	{
		return;
	}

	switch (iSkillNo)
	{
	case SKILL_DETECT_GHOST:
		{
			skill_Detect_Ghost(cn, iSkillNo, x, y, targettype, target_id);
			break;
		}
	case SKILL_POISONING:
		{
			skill_Poisoning(cn, iSkillNo, x, y, targettype, target_id);
			break;
		}
	case SKILL_DETECT_POISON:
		{
			skill_Detect_Poison(cn, iSkillNo, x, y, targettype, target_id);
			break;
		}
	case SKILL_FIND_TRAPS:
		{
			skill_Find_Traps(cn, iSkillNo, x, y, targettype, target_id);
			break;
		}
	case SKILL_REMOVE_TRAPS:
		{
			skill_Remove_Traps(cn, iSkillNo, x, y, targettype, target_id);
			break;
		}
	case SKILL_ENTRAPMENT:
		{
			skill_Entrapment(cn, iSkillNo, x, y, targettype, target_id);
			break;
		}

	case SKILL_DETECT_BOMB:
		{
			skill_Detect_Bomb(cn, iSkillNo, x, y, targettype, target_id);
			break;
		}
	case SKILL_BOMB_IN_BOX:
		{
			skill_Bomb_in(cn, iSkillNo, x, y, targettype, target_id);
			break;
		}
	case SKILL_REMOVE_BOMB:
		{
			skill_Remove_Bomb(cn, iSkillNo, x, y, targettype, target_id);
			break;
		}
	case SKILL_STEALING:
		{
			skill_Stealing(cn, iSkillNo, x, y, targettype, target_id);
			break;
		}
	case SKILL_RECLAIM:
		{
			skill_Reclaim(cn, iSkillNo, x, y, targettype, target_id);
			break;
		}
	case SKILL_SOW:
		{
			skill_Sow(cn, iSkillNo, x, y, targettype, target_id, s);
			break;
		}
	case SKILL_HAVEST:
		{
			skill_Havest(cn, iSkillNo, x, y, targettype, target_id);
			break;
		}
	case SKILL_MINING:
		{
			skill_Mining(cn, iSkillNo, x, y, targettype, target_id);
			break;
		}
	case SKILL_REFINE:
		{
			skill_Refine(cn, iSkillNo, x, y, targettype, target_id);
			break;
		}
	case SKILL_FISH_HOOKING					:
		{
			skill_Fish(cn, iSkillNo, x, y, targettype, target_id);
			break;
		}
	case SKILL_NETTING:
		{
			skill_Netting(cn, iSkillNo, x, y, targettype, target_id);			
			break;
		}
	case SKILL_CUTTING:
		{
			skill_Cutting(cn, iSkillNo, x, y, targettype, target_id);
			break;
		}
	case SKILL_LUMBERING:
		{
			skill_Lumbering(cn, iSkillNo, x, y, targettype, target_id);
			break;
		}
	case SKILL_BUTCHERING:
		{
			skill_Butchering(cn, iSkillNo, x, y, targettype, target_id);			
			break;
		}
	case SKILL_DIGGING:
		{
			skill_Digging(cn, iSkillNo, x, y, targettype, target_id);
			break;
		}
	case SKILL_MAKEPOWDER:
		{
			skill_Makepowder(cn, iSkillNo, x, y, targettype, target_id);
			break;
		}
	case SKILL_SHEAR:
		{
			skill_Shear(cn, iSkillNo, x, y, targettype, target_id);
			break;
		}
	case SKILL_CARPENTRY:
		{
			skill_Manufacturing(cn, iSkillNo, x, y, targettype, target_id);
			break;
		}
	case SKILL_BLACKSMITHY:
		{
			skill_Manufacturing(cn, iSkillNo, x, y, targettype, target_id);
			break;
		}
	case SKILL_COOKING:
		{
			skill_Cooking(cn, iSkillNo, x, y, targettype, target_id);
			break;
		}
	case SKILL_CLOTHING:
		{
			skill_Clothing(cn, iSkillNo, x, y, targettype, target_id);
			break;
		}
	case SKILL_TAILORING:
		{
			skill_Tailoring(cn, iSkillNo, x, y, targettype, target_id);
			break;
		}
	case SKILL_BOWCRAFTING:
		{
			skill_Bowcrafting(cn, iSkillNo, x, y, targettype, target_id);
			break;
		}
	case SKILL_FLETCHING:
		{
			skill_Fletching(cn, iSkillNo, x, y, targettype, target_id);
			break;
		}
	case SKILL_MAKEPOTION:
		{
			skill_MakePotion(cn, iSkillNo, x, y, targettype, target_id);
			break;
		}
	case SKILL_MAKEMETAL:
		{
			skill_MakeMetal(cn, iSkillNo, x, y, targettype, target_id);
			break;
		}
	case SKILL_MAKEBOTTLE:
		{
			skill_MakeBottle(cn, iSkillNo, x, y, targettype, target_id);
			break;
		}
	case SKILL_CANDLEMAKING:
		{
			skill_Candlemaking(cn, iSkillNo, x, y, targettype, target_id);
			break;
		}
	case SKILL_SNOOPING:
		{
			skill_Snooping(cn, iSkillNo, x, y, targettype, target_id);
			break;
		}
	case SKILL_MAKEFIRE:
		{
			skill_Makefire(cn, iSkillNo, x, y, targettype, target_id);
			break;
		}
	case SKILL_TAMING:
		{
			skill_Taming(cn, iSkillNo, x, y, targettype, target_id);
			break;
		}
	case SKILL_EVALUATE_INTELIGENCE:
		{
			skill_Evaluate_Inteligence(cn, iSkillNo, x, y, targettype, target_id);
			break;
		}
	case SKILL_EVALUATE_WISDOM:
		{
			skill_Evaluate_Wisdom(cn, iSkillNo, x, y, targettype, target_id);
			break;
		}
	case SKILL_ANATOMY_JUDGEMENT:
		{
			skill_Anatomy_Judgement(cn, iSkillNo, x, y, targettype, target_id);
			break;
		}
	case SKILL_ITEM_IDENTIFICATION:
		{
			skill_Item_Identification(cn, iSkillNo, x, y, targettype, target_id);
			break;
		}
	case TACTICS_Crapple:
		{
			skill_Crapple(cn, iSkillNo, x, y, targettype, target_id);
			break;
		}
	case TACTICS_swordmanship:
		{
			skill_Swordmanship(cn, iSkillNo, x, y, targettype, target_id);			
			break;
		}
	case TACTICS_Archery:
		{
			skill_Archery(cn, iSkillNo, x, y, targettype, target_id);
			break;
		}
	case TACTICS_Fencing:
		{
			skill_Fencing(cn, iSkillNo, x, y, targettype, target_id);
			break;
		}
	case TACTICS_Mace_fighting:
		{
			skill_Mace_Fighting(cn, iSkillNo, x, y, targettype, target_id);
			break;
		}
	case TACTICS_Pierce:
		{
			skill_Pierce(cn, iSkillNo, x, y, targettype, target_id);
			break;
		}
	case TACTICS_Whirl:
		{
			skill_Whirl(cn, iSkillNo, x, y, targettype, target_id);
			break;
		}
	case TACTICS_Hurl:
		{
			skill_Hurl(cn, iSkillNo, x, y, targettype, target_id);
			break;
		}
	case TACTICS_Parrying:
		{
			skill_Parrying(cn, iSkillNo, x, y, targettype, target_id);
			break;
		}
	case TACTICS_Double_Swordmanship:
		{
			skill_Double_Swordmanship(cn, iSkillNo, x, y, targettype, target_id);
			break;
		}
	case TACTICS_Double_MaceFighting:
		{
			skill_Double_Macefighting(cn, iSkillNo, x, y, targettype, target_id);
			break;
		}
	case TACTICS_Magery:
		{
			skill_Magery(cn, iSkillNo, x, y, targettype, target_id);
			break;
		}
	case TACTICS_Orison:
		{
			skill_Orison(cn, iSkillNo, x, y, targettype, target_id);
			break;
		}
	case SKILL_GHOST_RESURRECTION:
		{
			skill_GhostResurrection(cn, iSkillNo, x, y, targettype, target_id);
			break;
		}
	case SKILL_GM_CARPENTER:
	case SKILL_GM_BLACKSMITH:
	case SKILL_GM_COOK:
	case SKILL_GM_TAILOR:
	case SKILL_GM_BOWCRAFT:
	case SKILL_GM_ALCHEMIST:
	case SKILL_GM_CANDLEMAKER:
		{
			break;
		}
	}
}	//> CSD-030821

//<! BBD 040311	橇废矫肺 悼立荐 夸没窍绰 窃荐
void RequestUsercountToProxy()
{
	char Msg = 0;
	Msg = BYTE(PTCL_SERVERSET_USERNUM_REQUEST);

	g_pServerTable->SendToProxyServer(&Msg, sizeof(Msg));

}
//> BBD 040311	橇废矫肺 悼立荐 夸没窍绰 窃荐

//<! BBD 040311	悼立荐肺 利侩瞪 磊盔硅啦阑 掘绢坷绰 窃荐
float GetResourceRate(int nCount)
{
	int nIndex = 0;
	if( nCount <= 1000)		// 1000 捞窍
	{
		nIndex =  0;
	}
	else if( nCount > 1000 && nCount <= 1500)	// 1000 ~ 1500
	{
		nIndex = 1;
	}
	else if( nCount > 1500 && nCount <= 2000)	// 1500 ~ 2000
	{
		nIndex = 2;
	}
	else if( nCount > 2000 && nCount <= 2500)	// 2000 ~ 2500
	{
		nIndex = 3;
	}
	else if( nCount > 2500 && nCount < 3000)	// 2500 ~ 3000
	{
		nIndex = 4;
	}
	else		// 3000 捞惑老锭
	{
		nIndex = 5;
	}

	//<! BBD 040322	俊捞辑俊 包窍咯 蝶肺贸府
	if(LocalMgr.IsAbleNation( CHINA | TAIWAN | HONGKONG ))
	{
		return resource_rate2[nIndex];
	}
	else
	{
		return resource_rate[nIndex];
	}
	//> BBD 040322	俊捞辑俊 包窍咯 蝶肺贸府
}
//> BBD 040311	悼立荐肺 利侩瞪 磊盔硅啦阑 掘绢坷绰 窃荐
