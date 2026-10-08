
#include "../stdafx.h"
//#include "MySql.h"
#include "Dr_network.h"
#include "QuestFunction.h"
#include "Scrp_exe.h"
#include <assert.h>
#include "RegenManager.h" // 031028 kyo 
#include "SealStone.h"		// BBD 040318
#include "CItem.h"			// BBD 040329

CQuestInMap g_QuestInMap;
extern HDBC hDBC;
extern int  GetAliveNPCList(void);		//npc_list.h
extern void KillMonster(WORD wNumber); //op_magic.h
extern void CastMe2Other( int cn, t_packet *packet );	//Area.cpp
extern int SetRareItemInvPc( const int cn, const int iItemNo, const int iGradeMin, const int iGradeMax, const int iTryMulti, const int iDur, const int iHigh);
extern void FameUp2(const int cn, const int Type, const int Value);
extern SORT_DATA PC_TABLE[DRAGON_MAX_CONNECTIONS_+1];
extern void QuestTeleport( const int nCn, const int nX, const int nY );

//<! BBD 040401		// 惫啊规绢仿俊 蝶弗 搬拌籍 酒捞袍 焊惑侩
int g_RequitalItemRate[8] =
{
	100, 70, 45, 25, 15, 5, 3, 1,

};

int DecideRequitalByNationDefensePoint(CHARLIST * ch);
unsigned short	g_DefensePoint[NW_NATION_COUNT];	// BY,ZY,YL LocalWar Point

#define SEALSTONE_ITEMGIVERATE	2

//> BBD 040401		// 惫啊规绢仿俊 蝶弗 搬拌籍 酒捞袍 焊惑侩


struct IsIDHereInTimer : binary_function<CScriptTimer, int, bool>
{
public:
	bool operator()(const CScriptTimer& rTimer, const int nID) const //, const char* szName) const
	{
		return (rTimer.GetConnectionID() == nID) ? true:false;
	}
};

struct IsEndInTimer : binary_function<CScriptTimer, int, bool>
{
public:
	bool operator()(const CScriptTimer& rTimer, int z) const
	{
		return ( !rTimer.IsStart() );
	}
};

struct IsIDHereInCounter : binary_function<CScriptCounter, int, bool>
{
public:
	bool operator()(const CScriptCounter& rCounter, const int nID) const 
	{
		return (rCounter.GetConnectionID() == nID) ? true:false;
	}
};

struct IsEndInCounter : binary_function<CScriptCounter, int, bool>
{
public:
	bool operator()(const CScriptCounter& rCounter, int z) const //, int z) const
	{
		return ( !rCounter.IsStart() );
	}
};

struct IsNpcIndexHere : binary_function<table_requital_list, int, bool >
{
public:
	bool operator()( const table_requital_list& rRequital, int index) const
	{
		return ( rRequital.Npc_index == index )? true: false;
	}
};
//<! BBD 040329
struct IsItemIndex : binary_function<table_requital_Item, int, bool >
{
public:
	bool operator()( const table_requital_Item& rRequital, int index) const
	{
		return ( rRequital.index== index )? true: false;
	}
};
//> BBD 040329
/*
template <class T>
struct IsCnHere : binary_function<T, int,bool>
{
	bool operator()( const T& rT, int iId ) const
	{
		return ( rT.GetId( ) == iId )? true: false;
	}
};
*/
struct IsCnHere : binary_function<CSpellMove, int,bool>
{
	bool operator()(const CSpellMove &rT, int iId ) const
	{
		return ( rT.GetId() == iId )? true: false;
	}
};
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//CQuestInMap
CQuestInMap::CQuestInMap()
{
}
	
CQuestInMap::~CQuestInMap()
{	// 040305 kyo
	SAFE_DELETE(m_cRequital);
}	

int CQuestInMap::LoadQuestInMap()
{
	char szQuery[0xff];
	HSTMT hStmt= NULL ;
	RETCODE ret ;
	//SWORD	nCols ;
	SQLLEN cbValue ;
	
    SQLAllocStmt(hDBC, &hStmt);
	
	strcpy( szQuery, "select * from quest_in_map order by quest_num" );

	ret= SQLExecDirect(hStmt, (UCHAR *)szQuery, SQL_NTS) ;
	
	if(ret == SQL_SUCCESS_WITH_INFO || ret == SQL_SUCCESS)
	{
		ret = SQLFetch( hStmt );
		while( ret == SQL_SUCCESS )
		{
			table_quest_in_map table;
			//quest_num, map_num
			ret = SQLGetData( hStmt, 1, SQL_C_LONG, &table.quest_num, 0, &cbValue );
			ret = SQLGetData( hStmt, 2, SQL_C_LONG, &table.map_num, 0, &cbValue );

			if(ret != SQL_SUCCESS_WITH_INFO && ret != SQL_SUCCESS) 
			{
				MyLog( LOG_FATAL, "Table : quest_in_map: Error!!! (%d)", ret) ;
				return -1;
			}
			SetQuestInMap( table );

			ret = SQLFetch( hStmt );
		}
		return 1;
	}
	return 0;
}	

int CQuestInMap::LoadQuestInfoByStep()
{
	char szQuery[0xff];
	HSTMT hStmt= NULL ;
	RETCODE ret ;
	//SWORD	nCols ;
	SQLLEN cbValue ;
	
    SQLAllocStmt(hDBC, &hStmt);
	
	strcpy( szQuery, "select * from quest_info_byStep order by quest_num" );

	ret= SQLExecDirect(hStmt, (UCHAR *)szQuery, SQL_NTS) ;
	
	if(ret == SQL_SUCCESS_WITH_INFO || ret == SQL_SUCCESS)
	{
		ret = SQLFetch( hStmt );
		while( ret == SQL_SUCCESS )
		{
			table_quest_info_bystep table;
			//quest_num, begin_step, end_step, quest_info
			ret = SQLGetData( hStmt, 1, SQL_C_LONG, &table.quest_num, 0, &cbValue );
			ret = SQLGetData( hStmt, 2, SQL_C_LONG, &table.begin_step, 0, &cbValue );
			ret = SQLGetData( hStmt, 3, SQL_C_LONG, &table.end_step, 0, &cbValue );
			ret = SQLGetData( hStmt, 4, SQL_C_LONG, &table.quest_info, 0, &cbValue );

			if(ret != SQL_SUCCESS_WITH_INFO && ret != SQL_SUCCESS) 
			{
				MyLog( LOG_FATAL, "Table : quest_info_bystep: Error!!! (%d)", ret) ;
				return -1;
			}
			SetQuestInfoByMap( table );

			ret = SQLFetch( hStmt );
		}
		return 1;
	}
	return 0;
}

list<int> CQuestInMap::GetQuestInMap( int iMapnumber)
{
	list<int>	retQuest;
	list<table_quest_in_map>::iterator	it;

	for( it = m_tQuestInMap.begin(); it != m_tQuestInMap.end(); it++ )
		if( (*it).map_num == iMapnumber )
			retQuest.push_back( (*it).quest_num );

	return retQuest;
}

int CQuestInMap::GetQuestInfo( int iQuestNum, int iStep)
{//窜 窍唱狼 搬苞父捞 乐阑 挥捞促.  搬苞 绝栏搁 -1
	list<table_quest_info_bystep>::iterator it;
	for( it = m_tQuestInfo.begin(); it != m_tQuestInfo.end(); it++)
		if( (*it).quest_num == iQuestNum )
			if( (*it).begin_step < iStep && iStep <= (*it).end_step )
				return (*it).quest_info;
	return -1;
}

void CQuestInMap::SendQuestInfo( t_quest_in_map *tp, int cn)
{
	if( tp == NULL ) return;

	list<int> lQuestNum;
	lQuestNum = GetQuestInMap( tp->iMapnumber);
	if( lQuestNum.size() <=0 ) return;

	list<int>::iterator it;
	for( it = lQuestNum.begin(); it != lQuestNum.end(); it++)
	{
		//int z= GetQuestInfo( (*it), var[cn][(*it)]);
		if( 0 > (tp->iQuestInfo = GetQuestInfo( (*it), var[cn][(*it)])) ) continue;	// 021126 kyo
		else
			SendShowQuestInfo( tp, cn );
	}

}

void CQuestInMap::SendShowQuestInfo( t_quest_in_map *tp, int cn )
{
	if( tp==NULL ) return;
	t_packet packet;
	packet.h.header.type	= CMD_WHAT_QUEST_IN_MAP;
	packet.h.header.size = sizeof( t_quest_in_map);
	memset( (void**)&packet.u.quest_in_map, 0, sizeof( t_quest_in_map ) );
	memcpy( (void**)&packet.u.quest_in_map, tp, sizeof( t_quest_in_map ) );
	packet.u.quest_in_map.name[20] = '\0';
	
	QueuePacket( connections, cn, &packet, 1 );
}

int CQuestInMap::CheckSpell( const int cn, const char* szSpell)
{	//荤恩捞 决波唱 林巩捞 撇府搁 0,己傍窍搁 1
	//林巩阑 己傍利栏肺 鞠扁窍搁 扁粮 林巩阑 檬扁拳茄促.

	if( m_szClientSpell.size() <=0 ) return 0;
	string spell = GetSpellWord( cn );
	if( spell.size() == NULL ) return 0;

	if( strcmp( spell.c_str(), szSpell ) == 0 )
	{
		DeleteSpellUser( cn);
		return 1;
	}

	return 0;
}

list<CSpellMove>::iterator CQuestInMap::FindSpellUser( const int cn )
{
	SPELLITER itFind = find_if(m_szClientSpell.begin(), m_szClientSpell.end(), bind2nd( IsCnHere(), cn ) );

	if( itFind != m_szClientSpell.end() )
		return itFind;

	return m_szClientSpell.end();
}

void CQuestInMap::DeleteSpellUser( const int cn )
{
	if( m_szClientSpell.size() <=0 ) return;

	SPELLITER itFind = find_if(m_szClientSpell.begin(), m_szClientSpell.end(), bind2nd( IsCnHere(), cn ) );

	if( itFind != m_szClientSpell.end() )
		m_szClientSpell.erase(itFind);
}

string CQuestInMap::GetSpellWord( const int cn )
{
	SPELLITER itFind = find_if(m_szClientSpell.begin(), m_szClientSpell.end(), bind2nd( IsCnHere(), cn ) );

	if( itFind != m_szClientSpell.end() )
		return (*itFind).m_szSpell;
  return ""; // 021114 kyo
}

void CQuestInMap::SendSpellMapMove( const int&cn, const char* szSpell, const char* szMap, const int&  x, const int& y)
{// 021128 kyo
	t_packet packet;
	packet.h.header.type	= CMD_SCRIPT_SPELL_MAPMOVE;
	packet.h.header.size	= sizeof( t_script_spellmapmove );
	packet.u.spell_mapmove.cn = cn;
	packet.u.spell_mapmove.iX = x;
	packet.u.spell_mapmove.iY = y;
	strcpy( packet.u.spell_mapmove.szMap, szMap );
	//林巩篮 蜡聪内靛啊 瞪 啊瓷己捞 乐栏聪瘪 皋葛府 墨乔肺 窍磊.
	//strcpy( packet.u.spell_mapmove.szSpell, szSpell );
	memset( (void**)&packet.u.spell_mapmove.szSpell, 0x00, sizeof( packet.u.spell_mapmove.szSpell ) );
	memcpy( (void**)&packet.u.spell_mapmove.szSpell, szSpell, sizeof( packet.u.spell_mapmove.szSpell ) );
	
	QueuePacket( connections, cn, &packet, 1 );	
}

void CQuestInMap::RecvSpellMapMove( const t_script_spellmapmove *spell, const int& cn )
{// 021128 kyo
	if( spell->cn != cn ) return;

	if( !spell->isSuc )
	{
		SendSpellMapMove_Fail(cn);
		return;
	}

	SendSpellMapMove_Suc(spell->szMap, cn);
	::MapMove( cn, spell->szMap, spell->iX, spell->iY );
}

void CQuestInMap::SendSpellMapMove_Suc(const char* szMap, const int& cn)
{// 021128 kyo
	t_packet packet;
	packet.h.header.type	= CMD_SCRIPT_SPELL_MAPMOVE_SUC;
	packet.h.header.size	= sizeof( t_script_spellmapmove);
	memset( packet.u.spell_mapmove.szMap, 0, sizeof( packet.u.spell_mapmove.szMap));
	strcpy( packet.u.spell_mapmove.szMap, szMap );//, strlen( packet.u.spell_mapmove.szMap));
	//memcpy( packet.u.spell_mapmove.szMap, szMap, strlen( packet.u.spell_mapmove.szMap));

	QueuePacket( connections, cn, &packet, 1 );
}

void CQuestInMap::SendSpellMapMove_Fail(const int& cn)
{// 021128 kyo
	t_packet packet;
	packet.h.header.type	= CMD_SCRIPT_SPELL_MAPMOVE_FAIL;
	packet.h.header.size	= 0;
	QueuePacket( connections, cn, &packet, 1 );
}

void CQuestInMap::ShowStateMsg( const int& cn, const char* szMsg, const int& R, const int& G, const int& B )
{
	CHARLIST *ch = CheckServerId( cn );
	if( !ch ) return;
	t_packet packet;
	packet.h.header.type	= CMD_SCRIPT_SHOW_COLOR_MSG;
	packet.h.header.size	= sizeof( t_show_msg);
	packet.u.show_msg.R = R;
	packet.u.show_msg.G = G;
	packet.u.show_msg.B = B;
	//strcpy( packet.u.show_msg.szMsg , szMsg );
	memset( packet.u.show_msg.szMsg, 0, sizeof( packet.u.show_msg.szMsg ) );
	memcpy( packet.u.show_msg.szMsg, szMsg, strlen( szMsg ) );

	QueuePacket( connections, cn, &packet, 1 );	
}

void CQuestInMap::ShowStateMsg( const int& cn, const int& iMsgNum, const int& R, const int& G, const int& B )
{
	t_packet packet;
	packet.h.header.type	= CMD_SCRIPT_SHOW_COLOR_MSG;
	packet.h.header.size	= sizeof( t_show_msg);
	packet.u.show_msg.R = R;
	packet.u.show_msg.G = G;
	packet.u.show_msg.B = B;
	packet.u.show_msg.iMsgNum = iMsgNum;

	QueuePacket( connections, cn, &packet, 1 );
}

inline void RunNothing(){return;};

inline void SendSynchPacket1()
{
	t_packet packet;
	packet.h.header.type	= CMD_SCRIPT_SHOW_COLOR_MSG;
	packet.h.header.size	= sizeof( t_show_msg);
	char tmp[10];
	sprintf( tmp, "%d", EventPC );
	strcpy( packet.u.show_msg.szMsg, tmp);
	packet.u.show_msg.R = 200;
	packet.u.show_msg.G= 200;
	packet.u.show_msg.B= 200;
	
	QueuePacket( connections, EventPC, &packet, 1 );
};

inline void SendSynchPacket2(DWORD dwTmp)
{
	t_packet packet;
	packet.h.header.type	= CMD_SCRIPT_SHOW_COLOR_MSG;
	packet.h.header.size	= sizeof( t_show_msg);
	char tmp[10];
	sprintf( tmp, "%d", (dwTmp/1000) );
	strcpy( packet.u.show_msg.szMsg, tmp);
	packet.u.show_msg.R = 200;
	packet.u.show_msg.G= 0;
	packet.u.show_msg.B= 0;
	
	QueuePacket( connections, EventPC, &packet, 1 );
};


bool CQuestInMap::StartScriptTimer(const int iServerID, const char* szName, const DWORD &dwTimer, const DWORD &dwSync, const char* mapfile, const int& x, const int& y)
{
	//矫埃捞 0捞搁 秦寸 cn苞 name甫 啊柳 鸥捞赣甫 秦力茄促.
	//矫埃捞 剧荐搁 甸绢稠 蔼栏肺 鸥捞赣甫 push茄促.

	_ASSERTE( szName != NULL );
	if( szName == NULL ) return false;
	
	if( dwTimer == 0 )	//0捞搁 秦寸 鸥捞赣 昏力
	{
		//秦寸锅龋肺等 鸥捞赣甫 秦力茄促.
		if( DeleteScriptTimer( iServerID, szName ) ) 
			return true;
		else
			return false;
		//父距 立加捞 盔劝窍瘤 臼酒 iServerID尔 szName啊 老摹窍瘤 臼酒 鸥捞赣甫 秦力且荐 绝绰 版快扼档
		//RunScirtpTimer俊辑 老摹窍瘤 臼绰巴阑 very_long_term俊 蝶扼 犬牢窍扁锭巩俊 惑包绝促.
	}
	else if( dwTimer < 0 )	//澜荐搁 俊矾
		return false;

	//秦寸 蜡历啊 鸥捞赣甫 捞固 角青窍备 乐栏搁 货肺 鸥捞赣甫 倒府瘤 臼绰促. 
	if( FindUserInScriptTimer( iServerID, szName ) != m_cTimer.end() ) 
		return false;
	
	//CScriptTimer CScriptTimer( iServerID, szName, dwTimer, dwSync );	
	CScriptTimer cTimer;//(iServerID, dwTimer, );
	cTimer.SetID( iServerID);
	cTimer.SetName( szName );
	cTimer.SetTimerTime( dwTimer );
	cTimer.SetSynchroneTime( DEFAULT_TIMER_SYNC_TIME ); //5檬俊 茄锅
	cTimer.SetFunction( RunNothing, SendSynchPacket1 );	//泅犁绰 捞 窃荐甫 荤侩窍瘤 臼绰促. 
	cTimer.SetAfterMap( mapfile, x, y);//盔贰 窃荐甫 技泼秦具窍瘤父 

	//cTimer.SendSCRIPT_TIEMER( dwTimer, CMD_SCRIPT_TIMER_START );
	cTimer.StartTimer();
	m_cTimer.push_back( cTimer );
	return true;
}

bool CQuestInMap::DeleteScriptTimer( const int iServerID , const char* szName )	
{//秦寸 cn苞 捞抚捞 乐绰巴篮 瘤款促.
//菩哦阑 罐芭唱 胶农赋飘老锭促..
	TIMERITER itFind = find_if( m_cTimer.begin(), m_cTimer.end(), bind2nd( IsIDHereInTimer(), iServerID) );

	if (itFind != m_cTimer.end())
		if( strcmp( szName, (*itFind).GetName() ) == 0 )
		{		
			(*itFind).SendSCRIPT_TIEMER( 0, CMD_SCRIPT_TIMER_END_FAL);
			DeleteScriptCounter( iServerID, szName );	//Timmer啊 瘤况龙锭 Counter档 瘤况柳促. 
			m_cTimer.erase(itFind);
			return true;
		}
	return false;
}

void CQuestInMap::DeleteScriptTimer()	
{//鸥捞赣啊 场抄巴瘤款促. 

	int tmp = 0;	//抄吝俊 瘤矿矫促.
	TIMERITER itFind = find_if(m_cTimer.begin(), m_cTimer.end(), bind2nd( IsEndInTimer(),  tmp) );

	if (itFind != m_cTimer.end())
	{
		(*itFind).SendSCRIPT_TIEMER( 0, CMD_SCRIPT_TIMER_END_SUC);
		DeleteScriptCounter( (*itFind).GetConnectionID(), (*itFind).GetName());//iServerID, szName );	//Timmer啊 瘤况龙锭 Counter档 瘤况柳促. 
		m_cTimer.erase(itFind);
	}
}


void CQuestInMap::RunScriptTimer()
{
	if( m_cTimer.size() <= 0 ) return;	//窍唱滴 绝静搁 扁成 府畔茄促.

	list<CScriptTimer>::iterator it;
	//for_each( m_cTimer.begin(), m_cTimer.end(), m_cTimer.RunTimer() );
	//抄吝俊 己悼揪俊霸 拱绢杭帛... for_each俊辑it俊 甸绢啊绰 窃荐 结档 登绰啊...
	for( it = m_cTimer.begin(); it != m_cTimer.end(); it++)
	{
		//int z = (*it).GetConnectionID();
		(*it).RunTimer();

		//捞抚捞尔 cn捞 撇府绰 版快绰 鸥捞赣甫 肛冕促.(酵
		if( !CheckID2Name( (*it).GetConnectionID(), (*it).GetName() ) )
			(*it).EndTimer( SCRIPT_END_FAL );
	}


	//捞霸 [ if(!ch) return; ] 咯扁乐绰 捞蜡 .. 蜡历啊 立加秦辑 涅胶飘甫 柳青窍促 肺弊酒眶沁促. 葛电 捞甸捞 悼矫俊 富具..
	// m_cTimer.size()绰 咯矾俺瘤父 cn捞 绝扁 锭巩俊 捞霸 涝备俊 乐促搁 官肺 府畔秦滚矾寂
	// cn,name街捞 撇府绰 仇阑 瘤匡荐啊 绝促.
	// 弊矾促啊 酒鳖 涅胶飘 柳青窍带 逞捞 甸绢吭绰单. 犁荐肺 傈狼 cn阑 罐阑 啊瓷己捞 乐波耽..  弊扼搁 
	// 酒鳖 墨款磐啊 拌加 倒绊( 教农包访篮 持瘤 救酒静骨肺 ) 捞蜡绝捞 甘捞悼阑 且荐啊 乐蝶.. 弊烦 弊芭 绢骂 瞒廉? 凉唱 绢妨款 滚弊啊 瞪瘤滴 钢弗窜富具.
	CHARLIST *ch = CheckServerId( EventPC );
	if( !ch ) return;

	//鸥捞赣啊 场抄巴捞 乐栏搁 辆丰矫挪促.
	DeleteScriptTimer();
}

void CQuestInMap::EndScriptTimer( const int iID )
{
	CHARLIST *ch = CheckServerId( iID );
	if( !ch ) return;

	DeleteScriptTimer( iID , ch->Name );
}

bool CQuestInMap::CheckID2Name( const int& iID, const char* szName )
{//酒第尔 捞抚捞 泅 府胶飘尔 老摹窍瘤 臼绰促.
	if( !szName ) return false;
	CHARLIST *ch = CheckServerId( iID );
	if( !ch) return false;//蜡历啊 捞固 唱艾促.

	//蜡历啊 官布促.
	if( strcmp( ch->Name, szName ) != 0  )
		return false;

	return true;
}

void CQuestInMap::RevcScriptTimerPacket( t_script_timer *timer, int iType, int cn )
{
	//秦寸 cn狼 某腐磐啊 鸥捞赣甫 荤侩窍绊 乐阑锭
	CHARLIST *ch = CheckServerId( cn);
	if( !ch) return;

	TIMERITER it;
	it= FindUserInScriptTimer(cn, ch->Name);
	if( it == m_cTimer.end() ) return;

	switch( iType )
	{
	case CMD_SCRIPT_TIMER_START:
		{}break;
	case CMD_SCRIPT_TIMER_SYNC:
		{
			//教农啊 救 嘎栏搁 甘鸥捞赣尔 努扼捞 鸥捞赣 辆丰茄促..
			if( !(*it).ConfirmSyncTimer( timer->dwTime ) )
			{
				(*it).SendSCRIPT_TIEMER( 0, CMD_SCRIPT_TIMER_END_FAL);
				m_cTimer.erase( it );
			}

		}break;
	case CMD_SCRIPT_TIMER_END_SUC:
		{
			(*it).EndTimer( SCRIPT_END_SUC );	

			DeleteScriptCounter( cn, ch->Name );		//鸥捞赣啊 辆丰且锭 墨款磐档 鞍捞 辆丰矫挪促. 
			
			//(*it).SendSCRIPT_TIEMER( 0, CMD_SCRIPT_TIMER_END_SUC);
			//m_cTimer.erase( it );
		}break;
	case CMD_SCRIPT_TIMER_END_FAL:
		{
			(*it).EndTimer( SCRIPT_END_FAL ); //m_cTimer.erase( it );
			DeleteScriptCounter( cn, ch->Name );		//鸥捞赣啊 辆丰且锭 墨款磐档 鞍捞 辆丰矫挪促. 
			
		}break;
	default:
		break;
	}
	
}

list<CScriptTimer>::iterator CQuestInMap::FindUserInScriptTimer( const int& iId, const char* name)
{
	TIMERITER itFind = find_if(m_cTimer.begin(), m_cTimer.end(), bind2nd( IsIDHereInTimer(), iId ) );
	if( itFind != m_cTimer.end() ) 
		if( strcmp( (*itFind).GetName(), name) == 0)
			return itFind;

	return itFind;
}

bool CQuestInMap::StartScriptCounter( const int iServerID, const char* szName, const int iNum, const int iType, const int iMuch, const int iFlag)
{	//辆幅: 0=ncp,1=酒捞袍,2=固沥. 敲矾弊: 0=肛勉,1=矫累,2=肮荐尔 老摹

	_ASSERTE( szName != NULL );
	if( szName == NULL ) return false;

	switch( iFlag) 
	{
		case SCRIPT_COUNT_STOP :
		{	//墨款磐甫 肛苗扼(昏力秦扼)
			return DeleteScriptCounter( iServerID, szName );			
		}break;

		case SCRIPT_COUNT_CHECK :
		{	//墨款磐 蔼阑 犬牢秦扼
			return CheckScriptCounter( iServerID, szName, iNum, iType, iMuch );			
		}break;

		case SCRIPT_COUNT_START:
		{
			if( FindUserInScriptCounter( iServerID, szName, iType ) != m_cCounter.end() ) return false;

			CScriptCounter cCounter;
			cCounter.SetConnectionID( iServerID);
			cCounter.SetName( szName );
			cCounter.SetSpeciesNum( iNum );
			cCounter.SetSpeciesType( iType );
			cCounter.SetSpeciesMuch( iMuch );
			cCounter.StartCounter();
			m_cCounter.push_back( cCounter );

		}break;
	}

	return true;
	
}

bool CQuestInMap::DeleteScriptCounter( const int iServerId, const char* szName)
{	
	COUNTERITER itFind = find_if(m_cCounter.begin(), m_cCounter.end(), bind2nd( IsIDHereInCounter(), iServerId ) );

	if( itFind != m_cCounter.end() ) 
		if( strcmp( (*itFind).GetName(), szName) == 0 )
		{
			(*itFind).EndCounter();
			m_cCounter.erase( itFind);
			return true;
		}
	return false;
}

void CQuestInMap::DeleteScriptCounter()
{
	int z=0;	//抄吝俊 bind1st甫 肋 舅霸登搁 绊摹磊.
	COUNTERITER itFind = find_if( m_cCounter.begin(), m_cCounter.end(), bind2nd( IsEndInCounter(), z ) );
	if( itFind != m_cCounter.end() ) 
	{
		(*itFind).EndCounter();
		m_cCounter.erase( itFind);
	}

}

void CQuestInMap::RunScriptCounter( const int cn, const char* szName, int iNum, int iType)//CHARLIST *ch )//, const char* szName)
{

	if( m_cCounter.size() <=0 ) return;

	COUNTERITER	it = FindUserInScriptCounter( cn, szName, iType );	//秦寸 蜡历啊 秦寸鸥涝狼 墨款磐甫 角青窍搁
	if( it == m_cCounter.end() ) return;

	{
		(*it).RunCounter( iNum );
	}
	
	DeleteScriptCounter();	//墨款磐啊 场捞 唱搁 墨款磐甫 瘤匡鳖?( 辑滚鸥捞赣父 磷绰促.) 努扼捞攫飘绰 RunCounter俊 乐促.
}

list<CScriptCounter>::iterator CQuestInMap::FindUserInScriptCounter( const int& iId, const char* name, const int iType)
{
	COUNTERITER itFind = find_if(m_cCounter.begin(), m_cCounter.end(), bind2nd( IsIDHereInCounter(), iId ) );
	if( itFind != m_cCounter.end() ) 
		if( strcmp( (*itFind).GetName(), name) == 0)
			if( (*itFind).GetSpeciesType() == iType )
				return itFind;

	return m_cCounter.end();
}

bool CQuestInMap::CheckScriptCounter( const int& iServerId, const char* szName, const int iNum, const int iType, const int iCounter )
{
	COUNTERITER itFind = find_if(m_cCounter.begin(), m_cCounter.end(), bind2nd( IsIDHereInCounter(), iServerId ) );
	if( itFind == m_cCounter.end() ) 
	{
		MyLog(0,"CheckScriptCounter Returned 1");
		return false;
	}

	if( ::strcmp( (*itFind).GetName(), szName)) 
	{
		MyLog(0,"CheckScriptCounter Returned 2");
		return false;
	}

	if( (*itFind).GetSpeciesType() != iType )
	{
		MyLog(0,"CheckScriptCounter Returned 3");
		return false;
	}

	if( (*itFind).GetSpeciesNum() != iNum )					
	{
		MyLog(0,"CheckScriptCounter Returned 4 user's %d zs file %d",(*itFind).GetSpeciesNum(),iNum);
		return false;
	}

	if( (*itFind).GetCounter() < iCounter) 				
	{
		MyLog(0,"CheckScriptCounter Returned 5 user's user's %d, %d",(*itFind).GetCounter(),iCounter);
		return false;
	}
	return true;
}

void CQuestInMap::RevcScriptCounterSync( t_script_counter *counter,  int cn )
{// 030110 
	CHARLIST *ch = CheckServerId( cn);
	if( ch == NULL ) return;

	COUNTERITER itFind = find_if(m_cCounter.begin(), m_cCounter.end(), bind2nd( IsIDHereInCounter(), cn ) );	
	if( itFind != m_cCounter.end() ) 
		if( strcmp( (*itFind).GetName(), ch->Name) == 0)
		{
			(*itFind).SetCounter( counter->iCounter );
		}
	
}

void CQuestInMap::RunQuestTimer()
{
	RunScriptTimer();	//胶农赋飘 鸥捞赣
	RunBossTimer();		//boss 鸥捞赣
}

DWORD CQuestInMap::StartBossTimer(const int& iCn, const int& iBoss, const int& iX, const int& iY, const DWORD& dwTimer, const int iMaxEntree, const int nFlag) 
{	//boss鸥捞赣甫 积己茄促. 焊胶档 积己茄促.
	CHARLIST *ch = ::CheckServerId( iCn );
	if( !ch ) return 0;

	//力距牢盔俊 吧赴促. 馆券 0
	//鸥捞赣啊 倒绊乐促  泅犁矫埃 馆券
	//鸥捞赣啊 绝促.     汲沥鸥捞赣 矫埃 馆券窍绊 焊胶积己
	int nKey = iX*1000 + iY; 
	if( m_mBossTimer.find( nKey ) != m_mBossTimer.end() )	//捞 鸥捞赣啊 乐促.
	{
		if( iMaxEntree<= m_mBossTimer[nKey]->GetUserSize() )
		{
			return 0;
		}
		else
		{
			m_mBossTimer[nKey]->AddUser( iCn );
			return (m_mBossTimer[nKey]->m_cBossTimer.GetCurretTimerTime() - GetCurrentBossTimer( nKey));
		}

	}
	else
	{//秦寸 鸥捞赣啊 绝栏福葛 积己茄促. 
		
		CBossTimer *cBTimer;
		cBTimer= new CBossTimer;
		cBTimer->m_cBossTimer.SetTimerTime( dwTimer );		
		cBTimer->m_cBossTimer.SetSynchroneTime( DEFAULT_TIMER_SYNC_TIME ); //5檬俊 茄锅
		cBTimer->m_cBossTimer.StartTimer( true );	//辑滚父 档绰 鸥捞赣
		cBTimer->m_nFlag	=nFlag; 
		
		DeleteBossNpc( nKey );
		if( CBossTimer::TYPE_BOSS_ONLY == nFlag )
		{			
			cBTimer->m_nBossID = GetAliveNPCList(); //积己且荐 乐绰 listid甫 何咯罐绰促.
			cBTimer->m_iBossSprNo = iBoss;
			if( ::NPC_Create( cBTimer->m_nBossID , iBoss, iX, iY,-1, 0, GT_SCRIPT ) )//021230 lsw 付过阑 救静扁 锭巩俊 GT_SCRIPT肺 官层辑 抛胶飘
				
			{	// npc甫 积己茄肺弊甫 巢变促.
				MyLog( 1, "BossTimerStart :StartBossTimer:BossListID=%d,BossSprNo=%d,OpenCN=%s,Timer=%d",
							cBTimer->m_nBossID, iBoss, ch->Name, dwTimer);
			}
			
		}
		else if( CBossTimer::TYPE_BOSS_GROUP == nFlag )
		{
			cBTimer->m_nBossID = iBoss;
			g_pRegenManager->Ready(iBoss);
		}

		cBTimer->AddUser( iCn);
		m_mBossTimer.insert( PAIR_BOSS(nKey, cBTimer) );
	}

	return dwTimer;
}

void CQuestInMap::RunBossTimer()
{
	if( m_mBossTimer.empty() ) return;

BEGIN_LOOP:
	for( map<int, CBossTimer*>::iterator it = m_mBossTimer.begin() ; it != m_mBossTimer.end(); it++)
	{
		(*it).second->m_cBossTimer.RunTimer();
		if( !(*it).second->m_cBossTimer.IsStart() )
		{
			if( DeleteBossTimer( (*it).first ) )
			{
				goto BEGIN_LOOP;
			}
		}
	}
}

void CQuestInMap::DecreaseBossTimerUser(const int nKey, const int nCn)
{
	//return;
	ITOR_BOSS it = m_mBossTimer.find( nKey );
	if( it == m_mBossTimer.end() )
	{
		return;
	}
	m_mBossTimer[nKey]->DeleteUser(nCn);
}

void CQuestInMap::CheckBossTimerBoss( const int nKey)
{
	ITOR_BOSS it = m_mBossTimer.find( nKey );
	if( it == m_mBossTimer.end() )
	{
		return;
	}
	if( 0 >= m_mBossTimer[nKey]->GetUserSize() )
	{
		DeleteBossTimer( nKey );
	}
}

bool CQuestInMap::DeleteBossTimer(int nKey)
{
	ITOR_BOSS it = m_mBossTimer.find( nKey );
	if( it == m_mBossTimer.end() )
	{
		return false;
	}

	DeleteBossNpc(nKey);
	SAFE_DELETE( (*it).second );
	m_mBossTimer.erase( it );	

	return true;
}


void CQuestInMap::DeleteBossNpc(int nKey)
{
	ITOR_BOSS it = m_mBossTimer.find( nKey );
	if( it == m_mBossTimer.end() )
	{
		return;
	}

	(*it).second->RemoveBossNpc();
}

DWORD CQuestInMap::GetCurrentBossTimer(int nKey)
{
	if( m_mBossTimer.find(nKey) == m_mBossTimer.end() )
	{
		return 0;
	}
	return m_mBossTimer[nKey]->m_cBossTimer.GetSpendTime();
}

int CQuestInMap::GetBossTimerEntree(int nKey)
{
	if( m_mBossTimer.find(nKey) == m_mBossTimer.end() )
	{
		return 0;
	}
	return m_mBossTimer[nKey]->GetUserSize();
}

int	CQuestInMap::LoadRequitalTable()
{
	if( m_cRequital != NULL )
		DeleteRequeitalTable();
	m_cRequital =  new CRequital;

	return m_cRequital->LoadRequitalTable();
}
//<! BBD 040329
int	CQuestInMap::LoadRequitalItemTable()
{
	if( m_cRequital == NULL )
		m_cRequital =  new CRequital;

	return m_cRequital->LoadRequitalItemTable();
}
//> BBD 040329
void CQuestInMap::DeleteRequeitalTable()
{
	SAFE_DELETE(m_cRequital);
}


void CQuestInMap::GiveRequitalAfterKillNpc( const CHARLIST *a, const CHARLIST *d)
{
	if( !d ) return;

	//d啊 阁胶磐啊 酒聪搁 府畔
	if( d->IsPlayer() ) return;

	//npc刚历 茫绊 瘤档啊 嘎绰瘤 茫绰促. 
	REQUITALLISTITER it = FindRequitalNpc( d->npc_index );
	if( it == m_cRequital->RequitalEnd() ) return;

//<! BBD 040318	// 局矫寸檬 甘逞滚俊 秦寸窍绰 饭内靛父 掘绢吭栏骨肺 厚背啊 公狼固
	//甘锅龋 犬牢
//	if( (*it).Map_num != MapNumber ) return;
//> BBD 040318

	if( (*it).Tile_area > 0 )
	{
		RequitalRoundUser( d->GetServerID(), (*it) );
	}
	else if( (*it).Tile_area  == 0 ) 
	{
		RequitalKillUser( a->GetServerID(), (*it) );
	}
}

list<table_requital_list >::iterator CQuestInMap::FindRequitalNpc( const int iNpcIndex)
{
	if( m_cRequital == NULL ) return list<table_requital_list>::iterator();
	return m_cRequital->GetRequital_KillNpc( iNpcIndex );
}

void CQuestInMap::RequitalRoundUser( const int &cn, table_requital_list& table)
{
	list<int> ListCn;
	ListCn = GetRoundUser( cn );
	if( ListCn.size() <= 0 ) return;
	list<int>::iterator it;

	int i = 0;		// BBD 040401
	int rate = rand()%SEALSTONE_ITEMGIVERATE;	// BBD 040401

	for( it = ListCn.begin(); it != ListCn.end(); it++ )
	{
		//<! BBD 040401		搬拌籍篮 傈眉狼 1/2父 瘤鞭
		if(table.WarStatus)
		{
			if((i%SEALSTONE_ITEMGIVERATE) == rate)
			{
				GiveRequitalByCn( (*it), table );
			}
			i++;
		}
		else
		{
			GiveRequitalByCn( (*it), table );	
		}
		//> BBD 040401		搬拌籍篮 傈眉狼 1/2父 瘤鞭
	}
}

void CQuestInMap::RequitalKillUser( int cn, table_requital_list& table)
{
	GiveRequitalByCn( cn, table );
}

void CQuestInMap::GiveRequitalByCn( const int cn, table_requital_list& table)
{
	CHARLIST *ch = CheckServerId( cn);
	if( !ch) return;
//<! BBD 040318	 惫瘤傈俊 曼啊吝牢瘤狼 眉农
	if(table.WarStatus)	// 惫瘤傈 曼啊老锭父 焊惑阑 临巴烙
	{
		//<!BBD 040401	 惫啊 规绢仿俊 蝶扼 酒捞袍 瘤鞭阑 搬沥
		int nResult = DecideRequitalByNationDefensePoint(ch);

		if( nResult == -1)	// 俊矾牢版快
		{
			MyLog( LOG_NORMAL, "Character have wrong nation code name : %s", ch->Name );			
		}
		else if(nResult == 0)	// 焊惑犬伏郴肺 给甸绢吭促.
		{
			return;
		}
		//>BBD 040401	 惫啊 规绢仿俊 蝶扼 酒捞袍 瘤鞭阑 搬沥
		
		// Attack茄 仇捞 惫瘤傈俊 曼啊吝牢瘤 犬牢且 鞘夸 乐澜
		if(!ch->JoinLocalWar)
		{
			return;
		}
		// 规绢惫 敲饭捞绢搁 林瘤 富磊
		switch(table.Npc_index)
		{
		case SEALSTONE_YLLSE_NO:
			{
				if(ch->name_status.nation == N_YILSE)
				{
					return;
				}
				break;
			}
		case SEALSTONE_VYSEUS_NO:
			{
				if(ch->name_status.nation == N_VYSEUS)
				{
					return;
				}
				break;
			}
		case SEALSTONE_ZYPERN_NO:
			{
				if(ch->name_status.nation == N_ZYPERN)
				{
					return;
				}
				break;
			}
		default:
			break;
		}
		//<! BBD 040329 惫瘤傈 傈侩狼 酒捞袍阑 霖饶 府畔窍磊
		if(!Requital_About_SealStone(cn, table.Sardonyx, table.LeafOfBlessed, table.ref_index))
		{
			::OutMessage(ch,2,13);//"牢亥配府 傍埃捞 面盒摹 臼嚼聪促."
		}
		else
		{
			::OutMessage(ch,4,120);//"酒捞袍捞 牢亥配府肺 瘤鞭登菌嚼聪促.
			return;
		}
		//> BBD 040329 惫瘤傈 傈侩狼 酒捞袍阑 霖饶 府畔窍磊
	}
//> BBD 040318	 惫瘤傈俊 曼啊吝牢瘤狼 眉农

	if(table.Item_no != 0)	//item
	{
		//<! BBD 040322
		if(SetRareItemInvPc( cn, table.Item_no, table.Item_Min, table.Item_Max, table.Item_rare_count, 0, table.Item_rare_type ))
		{
			// 己傍
			::OutMessage(ch,4,120);//"酒捞袍捞 牢亥配府肺 瘤鞭登菌嚼聪促.
		}
		else
		{
			// 角菩
			::OutMessage(ch,2,13);//"牢亥配府 傍埃捞 面盒摹 臼嚼聪促."
		}
		//> BBD 040322
	}

	if( table.Quest_no !=0)	//qust_step
	{
		var[ cn ][ table.Quest_no ] = table.Quest_step;
	}

	if( table.Real_Fame !=0 )	//real_fame
	{
		FameUp2( cn, 1, table.Real_Fame );
	}

	{//dual fame蔼篮 quest蔼狼 10锅蔼捞促.
		const int iOldDF = var[ch->GetServerID()][DUAL_FAME_FIELD];
		const int iNewDF = iOldDF+table.Dual_Fame;
		var[ch->GetServerID()][DUAL_FAME_FIELD] = iNewDF;
		
		::SaveChangeDualFame( ch, iOldDF, iNewDF, LDF_QUEST);
		::MyLog(0,"Increase User Spy Game Point, User : %s, OldPoint %d, NewPoint %d",ch->Name,iOldDF,iNewDF);
	}
}

list<int> CQuestInMap::GetRoundUser( const int cn)
{
	list<int> RtList;
	for( int i = 0; i < PC_COUNT; i++ )
		for(int j = 0; j < MAX_AREA_BUFFER; j ++)
		if( connections[PC_TABLE[i].ID	].area_tbl[j].bAlive == TRUE )
		if( connections[PC_TABLE[i].ID	].area_tbl[j].Type == 2 )
		if( connections[	PC_TABLE[i].ID	].area_tbl[j].ID == cn)
		if( connections[	PC_TABLE[i].ID	].dwAgentConnectionIndex) 
		{ 
			RtList.push_back(  PC_TABLE[i].ID );
		}
	return RtList;
}

///CQuestInMap
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///CBossTimer

CBossTimer::CBossTimer()
{

}

CBossTimer::~CBossTimer()
{
	m_lUserCn.clear();
	int zz = m_lUserCn.size();
	RemoveBossNpc();
}

bool CBossTimer::AddUser(const int nCn)
{
	ITOR_INT it =find( m_lUserCn.begin(), m_lUserCn.end(), nCn );
	if( it == m_lUserCn.end() )
	{
		m_lUserCn.push_back(nCn);
		return true;
	}
	return false;
}

bool CBossTimer::DeleteUser(const int nCn)
{
	ITOR_INT it =find( m_lUserCn.begin(), m_lUserCn.end(), nCn );
	if( it != m_lUserCn.end() )
	{
		m_lUserCn.erase( it);
		return true;
	}
	return false;
	
}

void CBossTimer::RemoveBossNpc()
{
	if( TYPE_BOSS_ONLY == m_nFlag)
	{
		CHARLIST* pNpc = &NPCList[m_nBossID];
		if( !pNpc )	return; //焊胶啊 磷菌促绰 具扁. 
		
		if( pNpc->SprNo == m_iBossSprNo ) 
		{
			::KillMonster( m_nBossID);
		}
		else
		{
			MyLog( 1, "BossTimerStart :DeleteBossNpc: Boss is alive. but sprNo is not accorded" );	
		}
	}
	else if( TYPE_BOSS_GROUP == m_nFlag)
	{
		if( g_pRegenManager->IsExistHunt(m_nBossID) )
		{
			g_pRegenManager->Remove(m_nBossID);
		}
	}
}

//CBossTimer
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//CScriptTimer

bool CScriptTimer::StartTimer( bool bOnlyServer )
{
	//矫埃 例沥 盲农窃荐 持阑巴
	//矫埃捞 巢酒乐绰啊 窍绰芭?

	if( m_bStart == true) return false;

	//server only啊 酒聪搁 努扼捞攫飘档 矫累矫挪促. 
	if( bOnlyServer == false )	
		SendSCRIPT_TIEMER( m_dwTimerTime, CMD_SCRIPT_TIMER_START);

	m_bStart = true;
	return m_bStart;
}

void CScriptTimer::EndTimer( int iFlag)
{
	m_bStart = false; 
	if( iFlag == SCRIPT_END_SUC )
	{
		SendSCRIPT_TIEMER( 0, CMD_SCRIPT_TIMER_END_SUC);
		AfterTimer();
	}
	else if( iFlag == SCRIPT_END_FAL )
	{
		SendSCRIPT_TIEMER( 0, CMD_SCRIPT_TIMER_END_FAL);
	}
}

void CScriptTimer::ClearTimer()
{
	m_dwTimerTime = 0;
	m_dwSynchTime = 0;
	m_dwSpendTime = 0;
	m_ID		  = 0;
	//m_szName	  = NULL;
	m_dwOld		  = 0;
	m_dwSynch     = 0;
	m_bStart	  = false;
}

bool CScriptTimer::IsRunning()
{
	if( m_bStart== true && m_dwTimerTime > 0 && m_dwSpendTime > 0 )
		return true;
	return false;
}
void CScriptTimer::SetName( const char* name )
{
	if( !name  ) return;
	strcpy( m_szName, name );
	m_szName[30] = '\0';
}

void CScriptTimer::SetTimerTime( DWORD dwTime )
{
	//_ASSERTE( dwTime < MAX_TIMER_TIME );
	//_ASSERTE( dwTime > 0 );
	if( dwTime > MAX_TIMER_TIME || dwTime < 0 )
		m_dwTimerTime = MAX_TIMER_TIME;
	else
		m_dwTimerTime = dwTime;
}

void CScriptTimer::SetSynchroneTime( DWORD dwSynchTime )
{
	//_ASSERTE( dwSynchTime < MAX_TIMER_TIME );
	//_ASSERTE( dwSynchTime > MIN_SYNCH_TIME );

	if( dwSynchTime < MIN_SYNCH_TIME || dwSynchTime > MAX_TIMER_TIME )
		m_dwSynchTime = MIN_SYNCH_TIME;
	else
		m_dwSynchTime = dwSynchTime;
}


void CScriptTimer::RunTimer( )	//捞 窃荐甫 very long捞聪瘪 .75檬付促 角青等促绰吧 蜡狼窍档废
{
	//if( !IsRunning() ) return;
	//static DWORD dwOld= 0;		//贸澜 矫累矫埃
	//static DWORD dwSynch = 0;	//倔付付促 焊尘鳖?
	if( m_dwOld == 0 ) 
		m_dwOld = GetTickCount();

	DWORD dwCurrent = GetTickCount();
	m_dwSpendTime = dwCurrent - m_dwOld;		//倔付唱 矫埃捞 瘤抄绰啊?

	if( m_dwSpendTime > m_dwTimerTime )		//矫埃捞 促 馋唱搁 馋郴扼绰 疙飞阑 焊辰促. 
	{		
		m_dwOld = 0;
		m_dwSynch = 0;
		EndTimer( SCRIPT_END_SUC );	//矫埃 促 登辑 场唱搁 己傍利牢 辆丰
		return;
	}

	//汲沥茄 矫埃付促 (*sync)窃荐 角青
	if( (m_dwSpendTime - m_dwSynch) > m_dwSynchTime )
	{
		m_dwSynch = m_dwSpendTime;

		//for test
		SendSCRIPT_TIEMER( m_dwSpendTime, CMD_SCRIPT_TIMER_SYNC);
		//(*m_syncFunc)();
	}
	//else
	//亲惑 (*func)窃荐 角青
	//(*m_func)();
}

void CScriptTimer::SetFunction( void (*func)(), void (*syncFunc)() )
{
	m_func = func;
	m_syncFunc = syncFunc;
}

void CScriptTimer::SendSCRIPT_TIEMER( DWORD dwSpendTime , int iType)
{
	if( m_ID == 0) return;

	t_packet packet;
	packet.h.header.type	= iType;
	packet.h.header.size	= sizeof( t_script_timer);
	packet.u.script_timer.dwTime = dwSpendTime;	
	
	QueuePacket( connections, m_ID, &packet, 1 );
}

void CScriptTimer::SetAfterMap( const char* mapfile, const int& x, const int& y)
{	//< CSD-030509
	strcpy(m_szMapFile, mapfile); 
	m_nPosX	= x;
	m_nPosY = y;
}	//> CSD-030509

void CScriptTimer::AfterTimer()
{	//< CSD-030509
	//鸥捞赣啊 场捞 抄 饶俊 甘捞悼 且荐档 乐绊 促弗 老阑 且荐档 乐蝶. 
	if( m_ID == 0 )	return;// id啊 汲沥登瘤 臼疽阑锭绰 捞悼窍瘤 臼绰促. bosstimer俊辑 荤侩等促.
	
	//Log_UserInfo( m_ID); // 030617 kyo
	::CharUpper(m_szMapFile);
	::CharUpper(MapName);
	if( strcmp( m_szMapFile, MapName ) == 0 )
	{//鞍篮 甘捞搁 涅胶飘 炮饭器飘  肺 捞悼
		::QuestTeleport( m_ID, m_nPosX, m_nPosY);
	}
	else
	{
		::MapMove( m_ID, m_szMapFile, m_nPosX, m_nPosY);
	}
}	//> CSD-030509

bool CScriptTimer::ConfirmSyncTimer( DWORD dwTimer)
{
	//dwRecvTime捞尔 泅犁矫埃捞尔 +-X檬 捞惑 瞒捞唱搁 悼扁拳 角菩促..
	return true;
}

extern void LogInFile( string szFilePath,  char *szFormat, ... ); //kyo
void CScriptTimer::Log_UserInfo( const int nCn)//kyo
{	
	CHARLIST *ch = CheckServerId( nCn );
	if( !ch ) return;
	char szFilename[32];
	string szItem;
	sprintf( szFilename, "./QuestLog/%s.txt", ch->Name );
	char buff[8];
	for( int a=0; a<3; a++ )
	{
		for( int b=0; b<3; b++ )
		{
			for( int c=0; c<8; c++ )
			{
				itoa( ch->inv[a][b][c].item_no, buff, 8);
				szItem += buff;
				szItem += " ";
			}
			
		}
	}
	::LogInFile( szFilename, "CScriptTime-Mapmove :: Name:%s, InvItem: %s", ch->Name, szItem.c_str() );
}
//CScriptTimer
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//CScriptCounter
void CScriptCounter::ClearCounter()
{
	m_iSpeciesNum = 0;
	m_iSpeciesType =0;
	m_iSpeciesMuch =0;
	m_iCounter	= 0;
	m_bStart	= false;
	m_ID =0;
	m_szName[0] ='\0';
}

void CScriptCounter::SendSCRIPT_COUNTER( int iType)
{
	t_packet packet;
	packet.h.header.type	= iType;
	packet.h.header.size	= sizeof( t_script_counter);
	packet.u.script_counter.iNum = m_iSpeciesNum;	
	packet.u.script_counter.iType = m_iSpeciesType;	
	packet.u.script_counter.iMuch = m_iSpeciesMuch;	
	packet.u.script_counter.iCounter = m_iCounter;	

	QueuePacket( connections, m_ID, &packet, 1 );
}

void CScriptCounter::RecvSCRIPT_COUNTER()
{
}

bool CScriptCounter::StartCounter()
{
	if( m_bStart == true ) return false;
	
	SendSCRIPT_COUNTER( CMD_SCRIPT_COUNTER_START);
	m_bStart = true;
	m_iCounter =0;
	return true;
}

void CScriptCounter::RunCounter( int iNum )
{
	if( iNum != GetSpeciesNum() ) return;

	AddCounter();
	if( GetCounter() == GetSpeciesMuch() )
	{
		//墨款磐甫 倒促 蔼捞 瘤唱搁 肛眠绰单 瘤陛篮 肛眠瘤 救纳 父惦
		//EndCounter();	//俊辑 // 021129

	}
	
}

void CScriptCounter::AddCounter()
{
	m_iCounter++;
	SendSCRIPT_COUNTER( CMD_SCRIPT_COUNTER_SYNC );
}

void CScriptCounter::EndCounter()
{
	SendSCRIPT_COUNTER( CMD_SCRIPT_COUNTER_END );
	//ClearCounter();	//救肛眠霸窃.
}

//CScriptCounter
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//CRequital

int CRequital::LoadRequitalTable()
{
	char szQuery[0xff];
	HSTMT hStmt= NULL ;
	RETCODE ret ;
	SQLLEN cbValue ;	
    SQLAllocStmt(hDBC, &hStmt);
	
//	strcpy( szQuery, "select * from requital_list order by npc_index" );	// BBD 040318
	sprintf( szQuery, "select * from requital_list where map_num = %d  order by npc_index ", MapNumber );	// BBD 040318

	ret= SQLExecDirect(hStmt, (UCHAR *)szQuery, SQL_NTS) ;
	
	if(ret == SQL_SUCCESS_WITH_INFO || ret == SQL_SUCCESS)
	{
		ret = SQLFetch( hStmt );
		while( ret == SQL_SUCCESS )
		{
			table_requital_list table;
			//Map_num,Npc_index,Quest_no, Quest_step,Item_no, Item_rare_type, ,Item_rare_count, Real_Fame,Dual_Fame,	Tile_area;
			ret = SQLGetData( hStmt, 1, SQL_C_LONG, &table.Map_num, 0, &cbValue );
			ret = SQLGetData( hStmt, 2, SQL_C_LONG, &table.Npc_index, 0, &cbValue );
			ret = SQLGetData( hStmt, 3, SQL_C_LONG, &table.Quest_no, 0, &cbValue );
			ret = SQLGetData( hStmt, 4, SQL_C_LONG, &table.Quest_step, 0, &cbValue );
			ret = SQLGetData( hStmt, 5, SQL_C_LONG, &table.Item_no, 0, &cbValue );
			ret = SQLGetData( hStmt, 6, SQL_C_LONG, &table.Item_rare_type, 0, &cbValue );
			ret = SQLGetData( hStmt, 7, SQL_C_LONG, &table.Item_Max, 0, &cbValue );
			ret = SQLGetData( hStmt, 8, SQL_C_LONG, &table.Item_Min, 0, &cbValue );
			ret = SQLGetData( hStmt, 9, SQL_C_LONG, &table.Item_rare_count, 0, &cbValue );
			ret = SQLGetData( hStmt, 10,SQL_C_LONG, &table.Real_Fame, 0, &cbValue );
			ret = SQLGetData( hStmt, 11,SQL_C_LONG, &table.Dual_Fame, 0, &cbValue );
			ret = SQLGetData( hStmt, 12,SQL_C_LONG, &table.Tile_area, 0, &cbValue );
			ret = SQLGetData( hStmt, 13,SQL_C_LONG, &table.WarStatus, 0, &cbValue );	// BBD 040318
			ret = SQLGetData( hStmt, 14,SQL_C_LONG, &table.Sardonyx, 0, &cbValue );			// BBD 040329
			ret = SQLGetData( hStmt, 15,SQL_C_LONG, &table.LeafOfBlessed, 0, &cbValue );	// BBD 040329
			ret = SQLGetData( hStmt, 16,SQL_C_LONG, &table.ref_index, 0, &cbValue );		// BBD 040329
			if(ret != SQL_SUCCESS_WITH_INFO && ret != SQL_SUCCESS) 
			{
				MyLog( LOG_FATAL, "Table : requital_list : Error!!! (%d)", ret) ;
				SQLFreeStmt(hStmt, SQL_DROP);	// BBD 040329
				return -1;
			}
			SetRequitalTable( table );

			ret = SQLFetch( hStmt );
		}
		SQLFreeStmt(hStmt, SQL_DROP);	// BBD 040329
		return 1;
	}
	SQLFreeStmt(hStmt, SQL_DROP);	// BBD 040329
	return 0;
}

list<table_requital_list >::iterator CRequital::GetRequital_KillNpc( const int index )			//npc甫 磷看阑锭 林绰 秦琶狼 辆幅
{
	REQUITALLISTITER itFind = find_if(m_tRequital.begin(), m_tRequital.end(), bind2nd( IsNpcIndexHere(), index ) );
	if( itFind != m_tRequital.end() )
	{
		return itFind;	
	}

	return m_tRequital.end();
}

//<! BBD 040329
list<table_requital_Item >::iterator CRequital::GetRequital_ItemIttr( const int index)
{
	REQUITALITEMITER itFind = find_if(m_tRequitalItem.begin(), m_tRequitalItem.end(), bind2nd( IsItemIndex(), index ) );
	if( itFind != m_tRequitalItem.end() )
	{
		return itFind;	
	}

	return m_tRequitalItem.end();
}
//> BBD 040329
//CRequital
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int InitLoadQuestTable()		//涅胶飘俊 包访等 抛捞喉狼 肺靛
{
	g_QuestInMap.IniteAllTable();
	if( 1 != g_QuestInMap.LoadQuestInMap() ) return -1;
	if( 1 != g_QuestInMap.LoadQuestInfoByStep() ) return -2;
	//焊胶磷咯 秦琶林绰 抛捞喉 肺靛
	if( 1 != g_QuestInMap.LoadRequitalTable() ) return -3;	// 021105 kyo
	if( 1 != g_QuestInMap.LoadRequitalItemTable() ) return -4;	// BBD 040329

	return 1;
}

//<! BBD 040329
int CRequital::LoadRequitalItemTable()
{
	char szQuery[0xff];
	HSTMT hStmt= NULL ;
	RETCODE ret ;
	SQLLEN cbValue ;	
    SQLAllocStmt(hDBC, &hStmt);
	
	strcpy( szQuery, "select * from requital_item order by `Index`" );


	ret= SQLExecDirect(hStmt, (UCHAR *)szQuery, SQL_NTS) ;
	
	if(ret == SQL_SUCCESS_WITH_INFO || ret == SQL_SUCCESS)
	{
		ret = SQLFetch( hStmt );
		while( ret == SQL_SUCCESS )
		{
			table_requital_Item table;
			//Map_num,Npc_index,Quest_no, Quest_step,Item_no, Item_rare_type, ,Item_rare_count, Real_Fame,Dual_Fame,	Tile_area;
			ret = SQLGetData( hStmt, 1, SQL_C_LONG, &table.index, 0, &cbValue );
			ret = SQLGetData( hStmt, 2, SQL_C_LONG, &table.count, 0, &cbValue );
			ret = SQLGetData( hStmt, 3, SQL_C_LONG, &table.rate, 0, &cbValue );
			ret = SQLGetData( hStmt, 4, SQL_C_LONG, &table.item_no[0], 0, &cbValue );
			ret = SQLGetData( hStmt, 5, SQL_C_LONG, &table.item_no[1], 0, &cbValue );
			ret = SQLGetData( hStmt, 6, SQL_C_LONG, &table.item_no[2], 0, &cbValue );
			ret = SQLGetData( hStmt, 7, SQL_C_LONG, &table.item_no[3], 0, &cbValue );
			ret = SQLGetData( hStmt, 8, SQL_C_LONG, &table.item_no[4], 0, &cbValue );
			ret = SQLGetData( hStmt, 9, SQL_C_LONG, &table.grade, 0, &cbValue );
			ret = SQLGetData( hStmt, 10,SQL_C_LONG, &table.kind1, 0, &cbValue );
			ret = SQLGetData( hStmt, 11,SQL_C_LONG, &table.kind2, 0, &cbValue );
			ret = SQLGetData( hStmt, 12,SQL_C_LONG, &table.kind3, 0, &cbValue );
			ret = SQLGetData( hStmt, 13,SQL_C_LONG, &table.IsDynamic, 0, &cbValue );
			ret = SQLGetData( hStmt, 14,SQL_C_LONG, &table.HighRare, 0, &cbValue );
			if(ret != SQL_SUCCESS_WITH_INFO && ret != SQL_SUCCESS) 
			{
				MyLog( LOG_FATAL, "Table : requital_list : Error!!! (%d)", ret) ;
				SQLFreeStmt(hStmt, SQL_DROP);
				return -1;
			}
			SetRequitalItemTable( table );

			ret = SQLFetch( hStmt );
		}
		SQLFreeStmt(hStmt, SQL_DROP);
		return 1;
	}
	SQLFreeStmt(hStmt, SQL_DROP);
	return 0;
}
//> BBD 040329

//<! BBD 040329	// 搬拌籍俊 包茄 焊惑窃荐 
bool CQuestInMap::Requital_About_SealStone( const int cn, int nSardAmount, int nLeafAmount, int nRef_index)
{
	CHARLIST *ch = CheckServerId( cn);
	if( !ch )		return false;

	int a,b,c;
	RareMain RareAttr;
	ItemAttr  Item;
	POS pos;
	//<! BBD 040401
	int rate = rand()%10;
	switch(rate)
	{
	// 荤靛 林扁////////////////////////////////////////////////////////////////////////
	case 0:
	case 1:
	case 2:
		{
			for(int i = 0; i < nSardAmount; i++)		// 荤靛甫林磊
			{
				Item = ::GenerateItem(SARD_ID);	// BBD 040331
				if( SearchInv( ch->inv, a, b, c ))
				{
					ch->inv[a][b][c] = Item;
					SetItemPos( INV, a, b, c, &pos );
					::SendItemEventLog( &Item, ch->GetServerID(), 0, SILT_MAKE_BY_DB, 100 );
					SendServerEachItem(  &pos, &Item, cn);	// 捞逞捞 DB单阁栏肺 酒袍肺弊 巢扁扼绊 茄促
/*					Send_RareItemMakeLog(	cn,	Item.item_no,
							0,0,0,0,0,
							0,0,0,0,
							Item.attr[IATTR_LIMIT],
							Item.attr[IATTR_MUCH],
							0,3000,MapInfo[MapNumber].port,
							0,0,0,0,
							3000,0
							);				*/
				}
				else	// 牢亥俊 磊府绝澜
				{
					return false;
				}
			}
			break;
		}
	// 绵蕾 林扁////////////////////////////////////////////////////////////////////////
	case 3:
	case 4:
	case 5:
		{
			for(int i = 0; i < nLeafAmount; i++)		// 绵汗狼 蕾荤蓖甫林磊
			{
				Item = ::GenerateItem(LEAFBLESS_ID);	// BBD 040331
				if( SearchInv( ch->inv, a, b, c ))
				{
					ch->inv[a][b][c] = Item;
					SetItemPos( INV, a, b, c, &pos );
					::SendItemEventLog( &Item, ch->GetServerID(), 0, SILT_MAKE_BY_DB, 100 );
					SendServerEachItem(  &pos, &Item, cn);	// 捞逞捞 DB单阁栏肺 酒袍肺弊 巢扁扼绊 茄促
/*					Send_RareItemMakeLog(	cn,	Item.item_no,
							0,0,0,0,0,
							0,0,0,0,
							Item.attr[IATTR_LIMIT],
							Item.attr[IATTR_MUCH],
							0,3000,MapInfo[MapNumber].port,
							0,0,0,0,
							3000,0
							);				*/
				}
				else	// 牢亥俊 磊府绝澜
				{
					return false;
				}
			}
			break;
		}
	// 酒捞袍 林扁////////////////////////////////////////////////////////////////////////
	case 6:
	case 7:
	case 8:
	case 9:
		{
			if(nRef_index)	// 曼炼牢郸胶啊 乐栏搁 酒袍阑 父甸绢林磊
			{
				if(m_cRequital)
				{
					REQUITALITEMITER it = m_cRequital->GetRequital_ItemIttr(nRef_index);
					if(it != m_cRequital->RequitalItemEnd())	// 秦寸 酒捞袍狼 牢郸胶甫 茫疽衬?
					{
						int rate = rand()%100;
						if(rate < (*it).rate) // 犬伏郴肺 甸绢吭衬?
						{
							if(!SearchInv( ch->inv, a, b, c ))	// 牢亥俊 磊府绝澜					{
							{
								return false;
							}
							// 秦寸等 仇捞 割俺狼 酒捞袍阑 焊蜡沁绰瘤 犬牢
							int amount = 0;
							for(int i = 0; i < 5; i++)
							{
								if((*it).item_no[i])
								{
									amount++;
								}
							}

							if(!amount)	// 卿~ 茄俺档 绝促
							{
								return true;
							}

							int rate2 = rand()%amount;	// 乐绰肮荐 吝(item_no1 ~ 5) 茄仇阑 榜扼霖促
							// 酒捞袍阑 父电促
							ItemAttr item = ::GenerateItem((*it).item_no[rate2]);	// BBD 040331
							ItemMgr.MakeRareAttr(item.attr[IATTR_RARE_MAIN], (*it).grade, (*it).kind1, (*it).kind2, (*it).kind3,
								(*it).IsDynamic, (*it).HighRare);

							ch->inv[a][b][c] = item;	// 酒捞袍阑 持绢霖促.
							SetItemPos( INV, a, b, c, &pos );
							::SendItemEventLog( &item, ch->GetServerID(), 0, SILT_MAKE_BY_DB, 100 );
							SendServerEachItem(  &pos, &item, cn);	// 捞逞捞 DB单阁栏肺 酒袍肺弊 巢扁扼绊 茄促
						}
					}
				}

			}
			break;
		}
	default:
		{
			break;
		}
	}
	//> BBD 040401
	return true;
}
//> BBD 040329	// 搬拌籍俊 包茄 焊惑窃荐

//<! BBD 040401	 某腐磐狼 惫啊俊 蝶扼 酒捞袍 焊惑咯何甫 魄窜
int DecideRequitalByNationDefensePoint(CHARLIST *ch)
{
	int NationIndex = 0;

	switch(ch->name_status.nation)	// 惫啊 犬牢
	{
	case N_VYSEUS:
		{
			NationIndex = 0;
			break;
		}
	case N_ZYPERN:
		{
			NationIndex = 1;
			break;
		}
	case N_YILSE:
		{
			NationIndex = 2;
			break;
		}

	default:
		{
			return -1;		// 卿 惩 绢蠢唱扼 仇捞衬?
		}
	}

	int DefensePoint = g_DefensePoint[NationIndex];
	int DefIndex = 0;
	// 硅凯狼 秦寸 牢郸胶甫 茫澜
	if(DefensePoint >= 100)
	{
		DefIndex = 0;
	}
	else if(DefensePoint >= 80 && DefensePoint < 100)
	{
		DefIndex = 1;
	}
	else if(DefensePoint >= 70 && DefensePoint < 80)
	{
		DefIndex = 2;
	}
	else if(DefensePoint >= 60 && DefensePoint < 70)
	{
		DefIndex = 3;
	}
	else if(DefensePoint >= 50 && DefensePoint < 60)
	{
		DefIndex = 4;
	}
	else if(DefensePoint >= 40 && DefensePoint < 50)
	{
		DefIndex = 5;
	}
	else if(DefensePoint >= 30 && DefensePoint < 40)
	{
		DefIndex = 6;
	}
	else
	{
		DefIndex = 7;
	}

	int RequitalRate = g_RequitalItemRate[DefIndex];

	int rate = rand()%100;

	if(rate < RequitalRate)
	{
		return 1;	// 犬伏郴肺 甸绢吭促.
	}
	else
	{
		return 0;	// 犬伏郴肺 臼甸绢吭促.
	}
}
//> BBD 040401	 某腐磐狼 惫啊俊 蝶扼 酒捞袍 焊惑咯何甫 魄窜

