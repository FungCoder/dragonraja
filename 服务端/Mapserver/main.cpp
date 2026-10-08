#include "../Library/Shared/OperationsRates.h"
#include "stdafx.h"
#include <mmsystem.h>
#include <direct.h>
#include "LowerLayers\monitor.h"
#include "LowerLayers\GameTimer.h"
#include "mapserver.h"
#include "HigherLayers\DefaultHeader.h"
#include "HigherLayers\Rajasystem.h"
#include "LowerLayers\mylog.h"
#include "HigherLayers\Npclist.h"
#include "HigherLayers\MenuServer.h"
#include "HigherLayers\Winzs.h"
#include "HigherLayers\Scrp_int.h"
#include "HigherLayers\WeatherControl.h"
#include "HigherLayers\Op_Magic.h"
#include "HigherLayers\SealStone.h" // 001209 KHS 
#include "HigherLayers\AREA.h"		// 010205 KHS
#include "LowerLayers\servertable.h"	// 001215 KHS
#include "HigherLayers\TeamBattle.h"		// 010205 KHS
#include "HigherLayers\ChrLog.h"
#include "HigherLayers\UserManager.h"
#include "HigherLayers\SymbolItemMgr.h"//soto-030512
#include "HigherLayers\EventTreasureBoxMgr.h"//soto-030711

extern CSymbolItemMgr g_CSymbolMgr;//soto-030512
extern void InitLimitedTimeAndAge(); // 030929 kyo
// 011130 YGI
int DRAGON_MAX_CONNECTIONS;

static unsigned int random_next;
inline int ran()
{
	random_next = random_next * 1103515245 + 12345;
	return (random_next / 65536);
}

void sran( DWORD cnt )
{
	random_next = cnt;
}

// 020707 YGI
int srand_()
{
	sran( (unsigned)time( NULL ) );
	return 1;
}

int rand_()
{
	static int a = srand_();
	return ran();
}

// ----------------------------------------------------------------------------------------------
// Global Variables Begin
// ----------------------------------------------------------------------------------------------
int g_year				= 0;
int g_mon				= 0;	// 0 - 11
int g_day				= 0;
int g_yday				= 0;
int g_wday				= 0;
int g_hour				= 0;
int g_min				= 0;
int g_sec				= 0;

int g_count_ok;

DWORD global_time		= 0;	// 1ms ????..
DWORD g_alive_time		= 0;
DWORD g_curr_time		= 0;	// 1?????..
DWORD g_curr_time_with_out_year = 0;		// ??, ??, ?ð??? ??????? ???, ?? ?? ??????? ??´?.	// 031009 CI YGI
int   g_weatherflag		= 0;	// 1 ??? ?????? ????? BroadCast???? ?????. 


char bbsString[ MAX_PATH];
int  BBSBroadcast; 

int	 DRAGON_PORT	= 0;

HANDLE hIn;
// ----------------------------------------------------------------------------------------------
// Global Variables End
// ----------------------------------------------------------------------------------------------


// ----------------------------------------------------------------------------------------------
// Externs Begin
// ----------------------------------------------------------------------------------------------
// for Item Init
extern struct CItem_List Item_Ref ;
extern short int nNPC_Type ;


int LoadSkillLevelTable(void); // skill.cpp
int initItem(void) ;
int initNPCTable(void) ;
int InitEffectTbl();
int InitMagicTbl();
void CleanUpConnectionList(void);

// 010904 LTS
extern bool InitNationSystem();
extern bool InitLocalWarSystem();		// LTS LOCALWAR
extern bool InitEventLocalWarSystem();		// 020115 LTS
extern int InitMapServerConfigINI();		// LTS NEW LOCALWAR
extern int  LoadGeneration();			// LTS AI2
extern int	InitAIData();				// LTS AI2

// 020430 YGI acer 2
extern int InitChrLogDB( int port );

//021011 KYO 
extern int InitLoadQuestTable();		//????T???? ????? ???


// ----------------------------------------------------------------------------------------------
// Externs End
// ----------------------------------------------------------------------------------------------




// ----------------------------------------------------------------------------------------------
// Functions Begin
// ----------------------------------------------------------------------------------------------
int InitialScript( void )
{	
	int ret;
	
	InitTextScript();
	ret = LoadScript( MapName );
	
	if( !ret )
	{
		MyLog( LOG_NORMAL, " *** Error : Script Syntax " );
		return ret;
	}
	
	Script( 0 );
	MyLog( LOG_NORMAL, "   Script(0) Executed !");
	
	return ret;
};
// ----------------------------------------------------------------------------------------------
void RecvBBSLogin( char *msg )
{
	strcpy( bbsString, msg );
	
	BBSBroadcast = 30; 
}
// ----------------------------------------------------------------------------------------------
void StartingBBS( void )
{
	static DWORD time;
	char temp[ MAX_PATH];
	char *s;
	
	FILE *fp;
	
	wsprintf( temp, "%s/data/??????????????.txt", GameServerDataPath );
	
	fp = fopen( temp, "rt" );
	if( fp == NULL ) return;
	
	fgets( temp, MAX_PATH-1, fp );
	s = EatRearWhiteChar( temp );
	strcpy( bbsString, s );
	fclose(fp);
	
	BBSBroadcast = 30; 
}

void BroadCastBBS_Sub( char *msg, int len )
{	//< CSD-031213
	if (msg == NULL)
	{
		return;
	}

	if (len == 0)
	{
		return;
	}

	t_packet p;
	p.h.header.type = CMD_BBS;
	p.h.header.size = sizeof(t_server_bbs) - MAX_PATH + len;
	strcpy(p.u.server_bbs.bbs, msg);
	g_pUserManager->SendPacket(&p);
}	//> CSD-031213

void RecvTotalMapConnections( int cn )
{
	t_packet p;
	int no;
	no = PC_COUNT;
	if( no < 0 ) no = 0;
	p.h.header.type = CMD_TOTAL_MAP_CONNECTIONS;
	p.u.total_map_connections.no = no;
	p.h.header.size = sizeof( t_total_map_connections );
	QueuePacket( connections, cn, &p, 1 );
}

void RecvTotalConnections(int cn)
{
	int no = TotalConnections();
	
	if (no < 0) 
	{
		no = 0;
	}

	t_packet p;
	p.h.header.type = CMD_TOTAL_CONNECTIONS;
	p.h.header.size = sizeof(t_total_connections);
	p.u.total_connections.no = no;
	QueuePacket(connections, cn, &p, 1);
}

void RecvAbsLogOut2GameServer(char* id)
{	//< CSD-HK-030829
	for (int i = DRAGON_CONNECTIONS_START; i < DRAGON_MAX_CONNECTIONS; ++i)
	{
		if (connections[i].dwAgentConnectionIndex)
		{
			if (strcmp(connections[i].id, id) == 0)
			{
				closeconnection(connections, i, -203);
				break;
			}
		}
	}
}	//> CSD-HK-030829

void prepare(t_connection c[])
{
	global_time = ::timeGetTime();
	
	time_t lTime = {0,};
	time(&lTime);
	struct tm *today = localtime(&lTime);
	
	g_year = today->tm_year + 1900;
	g_mon  = today->tm_mon;
	g_yday = today->tm_yday;
	g_wday = today->tm_wday;
	g_day  = today->tm_mday;
	g_hour = today->tm_hour;
	g_min  = today->tm_min;
	g_sec  = today->tm_sec;
	
	// 031009 CI YGI
	g_curr_time_with_out_year = 
		(g_yday* 86400) 
		+ (g_hour* 3600) 
		+ (g_min * 60) 
		+ g_sec;
	
	g_curr_time = ( (g_year-1999 ) * 31536000) + g_curr_time_with_out_year;
}

// ----------------------------------------------------------------------------------------------
// 020808 YGI 
struct ID_PASS
{
	char m_szID1[30];
	char m_szPASS1[30];
	char m_szID2[30];
	char m_szPASS2[30];
	char m_szID3[30];
	char m_szPASS3[30];
	
	ID_PASS()
	{
		m_szID1[0] = 0;
		m_szPASS1[0] = 0;
		m_szID2[0] = 0;
		m_szPASS2[0] = 0;
		m_szID3[0] = 0;
		m_szPASS3[0] = 0;
	}
};

bool DecoadFile( char *filename, void *pData, int size )
{
	CHSEL_STREAM m_hsel;
	FILE *fp = fopen( filename, "rb" );
	if( !fp ) return false;
	
	int nVersion = 0;
	fread( (void*)(&nVersion), sizeof(int), 1, fp ); 
	if (m_hsel.GetVersion() != nVersion)  return false;
	HselInit deinit;
	fread( (void*)(&deinit), sizeof(HselInit), 1, fp );
	if (!m_hsel.Initial(deinit))  return false;
	fread( pData, 1, size, fp );
	m_hsel.Decrypt((char *)pData, size);
	fclose( fp );
	return true;
}
bool IncordFile( char *filename, void *pData, int size )
{
	CHSEL_STREAM m_hsel;
	FILE *fp = fopen( filename, "wb" );
	if( !fp ) return false;
	
	HselInit eninit;
	eninit.iEncryptType	=	HSEL_ENCRYPTTYPE_RAND;
	eninit.iDesCount	=	HSEL_DES_TRIPLE;
	eninit.iCustomize	=	HSEL_KEY_TYPE_DEFAULT;
	eninit.iSwapFlag	=	HSEL_SWAP_FLAG_ON;
	
	if (!m_hsel.Initial(eninit)) 
	{
		return false;
	}
	
	HselInit deinit;
	deinit = m_hsel.GetHSELCustomizeOption();
	const int nVersion = m_hsel.GetVersion();
	fwrite((void *)(&nVersion), sizeof(nVersion), 1, fp ); 
	fwrite((void *)(&deinit), sizeof(HselInit), 1, fp ); 
	m_hsel.Encrypt((char*)pData, size);
	fwrite( pData, 1, size, fp );
	fclose( fp );
	return true;
}

void MakeServerNeedFolder()
{	//< CSD-TW-030622
	::mkdir("./Output");
}	//> CSD-TW-030622

extern int InitMapInfo(t_MapInfo T[]); // CSD-030516
// 020808 YGI ??????? ????
int InitDRMapServerDatas(void)
{
    if (!InitializeOperationsRates()) {
        MyLog(LOG_FATAL, "Invalid Operations rates in MapServer.ini (valid percent: 10..1000)");
        return 0;
    }
    MyLog(LOG_NORMAL, "Operations rates: combat=%d%% npc-draws=%d%%",
          CurrentOperationsRates().experiencePercent, CurrentOperationsRates().dropPercent);
    if (!CurrentOperationsRates().reported) MyLog(LOG_FATAL, "Could not write operations-loaded.json; manager cannot confirm loaded rates");
	srand( (unsigned)time( NULL ) );
	
	MakeServerNeedFolder();
	
	prepare(connections);		//021030 YGI
	
	if (InitGameMakeModeSetting( MAP_SERVER_INI_ ) < 0 )
	{
		MyLog( LOG_FATAL, "File '%s' not FOUND",MAP_SERVER_INI_ );
		return (0);
	}
	
	
	char path[MAX_PATH];
	ID_PASS id_password;
	int bIdPassword = 0;		// ????? ????? ??? ??°??
	{
		sprintf( path, "%s/data/IdPassword.bin", GameServerDataPath );
		bIdPassword = DecoadFile( path, (char *)&id_password, sizeof( ID_PASS ) );
	}
	
	ID_PASS IdPassWord;
	::strcpy( IdPassWord.m_szID1,		LocalMgr.GetDBAccount(DRAGON_DB,ID));//030102 lsw
	::strcpy( IdPassWord.m_szPASS1,	LocalMgr.GetDBAccount(DRAGON_DB,PASS) );//030102 lsw
	
	if (bIdPassword && GetPrivateProfileInt("mysql_conn", "use_legacy_id_password",
		0, MAP_SERVER_INI_) != 0)
	{
		IdPassWord = id_password;
		MyLog(LOG_NORMAL, "Using legacy IdPassword.bin credentials by configuration.");
	}
	
	if( (Init_SQL("DragonRajaDB", IdPassWord.m_szID1, IdPassWord.m_szPASS1) ) == 0)
	{
		MyLog( LOG_FATAL, "SQL server connect fail !");
		return(0);
	}
	MyLog( LOG_NORMAL, "SQL server connect OK!");
	
	// 020430 YGI acer 2
	if( !InitChrLogDB( GetManagementMapPort( MM_SAVE_LOG_FILE_MAP ) ) ) return 0;
	//< CSD-031229
	/*
	// 031009 CI YGI
	if( !InitItemLimitCountFromDB() )		// ????T ??? ?????? ???? ???.
	{
		JustMsg( "Error!!! Check please ItemLimitMgrTable table" );
		return 0;
	}
	*/
	//> CSD-031229
	InitNPCList();
	if(initNPCTable() < 0) { Error ("  NPC Table Loading Failed.\n");	} 
	else
	{
		MyLog( LOG_NORMAL, "'NPC_NamebyGender' Table initializing  ..........  OK !");
		MyLog( LOG_NORMAL, "'NPC_Lv2Exp'       Table initializing  ..........  OK !");
		MyLog( LOG_NORMAL, "'NPC_Generation_SP'Table initializing  ..........  OK !\n");
	}
	
	g_MapPort = GetOwnPort(); // CSD-030506	
	InitMoveDelayTable();
	
	// 0527 YGI ??????
	MyLog( LOG_NORMAL, "  Reading './Data/BadWord.TXT'      count = %d ", LoadFilterWord() );
	
	InitItemList();
	
	if(initItem() < 0) 	{	Error ("  Item loading Fail !\n");	return(0);	}
	else MyLog( LOG_NORMAL, "Item Table initializing    ...................... OK !\n");
	
	MakeStoreList();
	MyLog( LOG_NORMAL, "Store Item List  initializing    ................ OK !\n");
	
	if( InitSkillTable( ) < 0 ) 	{	Error ("  'skillmain' loading Fail !\n");	return(0);	}
	MyLog( LOG_NORMAL, "'SkillMain' Table initializing   ................ OK !");
	
	if( InitGmQuest( ) < 0 ) {	Error ("  'GMquest' loading Fail !\n");	return(0);	}
	else MyLog( LOG_NORMAL, "'GMquest' Table initializing    ................. OK !");

	if( InitMapInfo( MapInfo ) < 0 ) { Error( " MapInfo Loading Fail !! \n" ); return(0); }
	else	MyLog( LOG_NORMAL, "'Map_Info' Table loading        ................. OK !");
	if (LoadArenaInfo())
	{	//< CSD-030517
		if (LoadArenaGameInfo())
		{
			MyLog(LOG_NORMAL, "'arean_game_info' Table loading        ........... OK !");
		}

		if (LoadArenaTeamInfo())
		{
			MyLog(LOG_NORMAL, "'arean_Team_info' Table loading        ........... OK !");
		}	

		if (LoadArenaBonusInfo())
		{
			MyLog(LOG_NORMAL, "'arean_bonus_info' Table loading       ........... OK !");
		}	
	}	//> CSD-030517

	if( g_AliveMap.LoadAliveMapZone() < 0 )	Error ("  AliveMapZone Table loading Fail !\n");	// 010502 YGI
	else	MyLog( LOG_NORMAL, "'alive_zone' Table loading        ..............  OK !");
	
	// 010522 YGI
	if( LoadNationInfo(NationInfo ) != 1 )	Error( "  Nation_Info Table loading fail!!!!! \n" );
	else	MyLog( LOG_NORMAL, "'Nation_Info' Table loading         ............  OK !");
	
	if( LoadGuildListForMapServer( ) != 1 )	{ Error( "  Guild_List Table loading fail!!!!! ... new! " ); return(0); }
	else	MyLog( LOG_NORMAL, "'Guild_List' Table loading         ............  OK");
	
	
	// 011130 YGI
	extern void LoadDRAGON_MAX_CONNECTIONS();
	LoadDRAGON_MAX_CONNECTIONS();
	if( !DRAGON_MAX_CONNECTIONS ) DRAGON_MAX_CONNECTIONS = DRAGON_MAX_CONNECTIONS_; 
	
	
	
	//< CSD-011126
	if (InitEffectTbl() < 0) 	
	{	
		Error("  Effect loading Fail ! ... new! \n");	
		return(0);	
	}
	else 
	{
		MyLog(LOG_NORMAL, "'Effect' Table initializing     ...............   OK !");
	}
	
	if (InitMagicTbl() < 0) 	
	{	
		Error ("  Magic loading Fail ! ... new! \n");	
		return(0);	
	}
	else 
	{
		MyLog(LOG_NORMAL, "'Magic' Table initializing	     ...............   OK !");
	}
	//> CSD-011126
	if( InitSkillMapTable() < 0) 	{	Error ("  SkillMapTable loading Fail !");	return(0);	}
	else MyLog( LOG_NORMAL, "'SkillMapTable' initializing   ................   OK !");
	
	if( InitialScript() <= 0  ) {	MyLog( LOG_NORMAL, " Script Loading Warning (non-fatal) ");	}
	
	// ????? ?????? Item?? ???? Table..
	if( LoadItemsInBoxTableSQL() < 0  ){	Error ("  'ItemsBox_new' Table loading Fail !");	return(0);	}
	else MyLog( LOG_NORMAL, " 'ItemsBox_new' Loading  ......................   OK !" );
	
	
	
	if (!MapBuild(&g_Map, MapName)) { Error("Map terrain loading failed"); return 0; }
	MyLog( LOG_NORMAL, "<%11s.map>  loading                        OK !", MapName );
	
	if (LoadTOI(MapName)) MyLog(LOG_NORMAL, "<%11s.toi2> loading OK !", MapName);
	else MyLog(LOG_NORMAL, "[MAP] Optional object resource missing for %s; static doors/boxes are unavailable", MapName);
	
	if (LoadSkillMapTable())
		MyLog(LOG_NORMAL, "<%11s.skb> loading OK", MapName);
	else
	{
		// ?????????? Event_Monster/????,??? SKB?
		MyLog(LOG_NORMAL, "[SKB] Map resource unavailable; only event/script generation remains enabled");
	}
	
	if (!LoadGeneration()) MyLog( LOG_NORMAL, "****** FAILED :: Event_Monster Table loading, Not Terminate......");		// LTS AI2
	else MyLog( LOG_NORMAL,"Event_Monster Table load Complete.........    OK !");
	
	
	if( LoadItemJoinTable()	) MyLog( LOG_NORMAL, "'ItemJoin' Table initializing  ................   OK !");
	else { Error ("******* FAILED !! :: Item Join Table initializing"); return 0; }
	
	if( LoadWeatherTable() ) MyLog( LOG_NORMAL, "WeatherTable(data/rain.tbl, data/Temprature.tbl) initializing .... OK !");
	else { Error ("***** FAILED :: Weather Table  initializing"); return 0; }
	
	//this2 lsw soksung 
	//011025 lsw >
	if( LoadItemTable()	) 
	{
		MyLog( LOG_NORMAL, "Item Attr. initializing  ");
		MyLog( LOG_NORMAL, 
			"     (Item_mutant/ Item_grade/ Item_mutant_kind ) OK !");
	}
	else { Error ("******* FAILED !! :: Item mutant, rare Table  initializing"); return 0; }
	//011025 lsw <
	
	int t = LoadSkillLevelTable();
	if(	t > 0  ) MyLog( LOG_NORMAL, " skill_lvexp/ skill_exp/ mon_bag Table Loading    OK !");
	else{ Error ("******* FAILED !! :: skill_lvexp/ skill_exp / mon_bag ( %d )", t ); return 0; }
	MyLog(LOG_NORMAL, "InitDRMapServerDatas: step 1 - after LoadSkillLevelTable");
	
	if(LoadGenerateSpecialItem() < 0) {	Error ("  Special_Item initializing Fail !\n");	return(0);	}
	else MyLog( LOG_NORMAL, "'Special_Item' Table initializing .............   OK !" );
	MyLog(LOG_NORMAL, "InitDRMapServerDatas: step 2 - after LoadGenerateSpecialItem");
	
	if (LoadAbilityLimit())
	{
		MyLog(LOG_NORMAL, "'ability_limit' Table initializing .............   OK !");
	}
	else
	{
		Error("  ability_max initializing Fail!\n");	
		return 0;
	}
	
	if (LoadAbilityDivide())
	{
		MyLog(LOG_NORMAL, "'ability_divide' Table initializing ............   OK !");
	}
	else
	{
		Error("  ability_divide initializing Fail!\n");
		return 0;
	}
	
	if (LoadDualInfo())
	{
		MyLog(LOG_NORMAL, "'dual_info' Table initializing .................   OK !");
	}
	else
	{
		Error("  dual_info initializing Fail!\n");
		return 0;
	}
	
	if( InitGameItem() == 1 )
		MyLog(LOG_NORMAL, "'gamble_item' Table initializing ...............   OK !");
	else
	{
		Error("  gamble_item initializing Fail!\n");
		return 0;
	}
	
	LoadNationItem( g_wday );
	
	SaveMoveDelayTable();// 1Y?? ?????µ? ????½ð??? Text???? ??????. ( ????? )
	LoadConditionTable();
	
	if (!InitMapServerConfigINI())	// LTS NEW LOCALWAR
	{
		Error("******* FAILED !! :: Load MapServerConfig.ini\n");
		return 0;
	}	// LTS LOCALWAR
	MyLog(LOG_NORMAL, "InitDRMapServerDatas: step 3 - after InitMapServerConfigINI");
	
	if (!InitNationSystem()) // 010904 LTS
	{
		Error("******* FAILED !! :: Nation System Initializing Fault!\n");
		return 0;
	}
	MyLog(LOG_NORMAL, "InitDRMapServerDatas: step 4 - after InitNationSystem");
	
	if (!InitLocalWarSystem())		// LTS LOCALWAR
	{
		MyLog( LOG_NORMAL, " WARNING: LocalWar System Initializing Fault (non-fatal) ");
	}
	
	if (!InitEventLocalWarSystem())		// 020115 LTS
	{
		MyLog( LOG_NORMAL, " WARNING: Event LocalWar System Initializing Fault (non-fatal) ");
	}
	
	if (!InitAIData())		// LTS AI2
	{
		Error("******* FAILED !! :: AI Data Initializing Fault!\n");
		return 0;
	}

	if (LoadWarStartInfo())
	{	//< CSD-030723
		MyLog(LOG_NORMAL, "'WarStartTBL' Table initializing ...............   OK !");
	}	//> CSD-030723
	
	InitRefreshMenu();		// 020620 YGI
	InitNationRelation();	// 001209 KHS ???????? ???? ????.
	LoadGetGodTable();		// 0605 YGI	???? ????? ????????
	LoadGameServerStatus();	
	CheckingChrLogAvailable();	// 010301 KHS
	
	InitOnlyStart_kein();		// 020818 YGI
	MyLog(LOG_NORMAL, "InitDRMapServerDatas: step 5 - after InitOnlyStart_kein");
	
	int tflag = InitLoadQuestTable(); // 021106 kyo
	MyLog(LOG_NORMAL, "InitDRMapServerDatas: step 6 - InitLoadQuestTable returned %d", tflag);
	if( tflag == 1 )
	{
		MyLog(LOG_NORMAL, "'quest_in_map'	Table initializing ..............   OK ! ");
		MyLog(LOG_NORMAL, "'quest_info_map' Table initializing ..............   OK ! ");
		MyLog(LOG_NORMAL, "'requital_list'  Table initializing ..............   OK ! ");
		MyLog(LOG_NORMAL, "'requital_item'  Table initializing ..............   OK ! ");	// BBD 040329
	}
	else
	{
		if( tflag == -1 )
			Error("******* FAILED !! 'quest_in_map' QuestTable!\n");
		else if( tflag == -2 )
			Error("******* FAILED !! 'quest_info_map' QuestTable!\n");
		else if( tflag == -3 )
			Error("******* FAILED !! 'requital_list' QuestTable!\n");
		//<! BBD 040329		
		else if( tflag == -4 )
			Error("******* FAILED !! 'requital_item' QuestTable!\n");
		//> BBD 040329		
		
		return 0;
	}
	
	//< soto-030331 ??W?? ????? ???.
	int nRet = 0;
	if(nRet = LoadGuardianGuildList())
	{
		MyLog(LOG_NORMAL,"GuardianGuildList Table initializing ..............   OK ! ");
	}
	else
	{
		Error("******* FAILED !!  GuardianGuildList Table initializing!");
	}
	//> soto-030331
	if (LoadHuntInfo())
	{	//< CSD-030509
		if (LoadHuntGroupInfo())
		{	
			LoadHuntMonsterInfo();
			LoadHuntPlaceInfo();
		}	
	}	//> CSD-030509
	
	if( LocalMgr.IsAbleNation(THAI) )// 030929 kyo
	{
		::InitLimitedTimeAndAge();
	}

	tflag  = GetPrivateProfileInt( "Option", "DisconnectAccelatorUser",	0, MAP_SERVER_INI_ );
	if( tflag )
	{
		g_accelator_user_closeconnection = true;
		MyLog( LOG_NORMAL, "  ** Accelator Users will be disconnected and logged in!");
	}
	else 
	{
		MyLog( LOG_NORMAL, "  ** Accelator Users will be ** NOT ** disconnected and logged in!");
	}
	
	g_alive_time = timeGetTime();
	MyLog( LOG_NORMAL, "All of 'GameServerData' Loaded, successfuly ............^ ^\n\n");
	
	MyLog( LOG_NORMAL, " F1 : Server Status");
	MyLog( LOG_NORMAL, " F5 : Re-Load a part of 'GameServerData'" );
	MyLog( LOG_NORMAL, "ESC : Exit" );
	
	return(1);
}



// 020430 acer 2
/////////////////////////////////////////////////////////////////////////////
extern void InitItemLog( int port );
int InitChrLogDB( int port )
{
	if( GetOwnPort() == port )		// ??? ???? ????
	{
		if(!Init_ChrLogDB_SQL( "ChrLogDB", 
			LocalMgr.GetDBAccount(CHRLOG_DB,ID), 
			LocalMgr.GetDBAccount(CHRLOG_DB,PASS)))
		{
			MyLog( LOG_FATAL, "'ChrLogDB' Table Initializing Fail !" );
			JustMsg("'ChrLogDB' Table Initializing Fail !"  );
			return(-1);
			
		}
		MyLog( LOG_NORMAL, "SQL server( ChrLogDB ) connect OK!");
	}
	// item_log ?? ???? chrlogdb odbc ????
	InitItemLog( port );
	//MyLog( LOG_FATAL, "Error! init ChrLogDB : This Surver port = [%d], ManagementServer port = [%d]", GetOwnPort(), port );
	return(1);
}
/////////////////////////////////////////////////////////////////////////////



// Rewrote by chan78 at 2000/11/27
// 020620 YGI 
void SaveAllUserDatas(void)
{
	static int ct = 0;
	DWORD counter = 0;
	int i;
	
	MyLog( LOG_IMPORTANT, "Saving Player Datas on DATABASE");
	for( i = DRAGON_CONNECTIONS_START ; i < DRAGON_MAX_CONNECTIONS ; i ++)
	{
		if( i && !(i % ((DRAGON_MAX_CONNECTIONS/5)?(DRAGON_MAX_CONNECTIONS/5):1)) ) 
		{
			MyLog( 0, "-- Now %d connections are saved (%d/%d)", counter, i, DRAGON_MAX_CONNECTIONS );
		}
		if( connections[i].dwAgentConnectionIndex && connections[i].state >= CONNECT_JOIN )
		{
			int ret1 = updateCharacterStatus(connections, i);
			int ret2 = UpdateCharStatusByKein( connections, i);		// 0410 YGI	// ??? ????? ???? ????
			if( ret1 != 1 || ret2 != 1 )
			{
				ct++;
				CHARLIST *ch = &connections[i].chrlst;
				MyLog( 0, "%03d - Name(%-20s): ERROR_CODE[%d/%d], ID(%d), ServerId(%d), EXP(%u)\n", ct, ch->Name, ret1, ret2, i, ch->GetServerID(), ch->Exp );
			}
			else counter++;
		}
	}
	MyLog( 0, "-- Total %d connections are saved (%d:%d)", counter, i, DRAGON_MAX_CONNECTIONS );
	//SaveGameServerStatus();
};	


void OnDestroy()
{
	EndMapServer();
}
// ----------------------------------------------------------------------------------------------
// Functions End
// ----------------------------------------------------------------------------------------------
#include "GameFactory.h"
// ----------------------------------------------------------------------------------------------
// Main()
// ----------------------------------------------------------------------------------------------
int main()
{	//< CSD-HK-030829
	CGameFactory gameFactory;
	CGameSystem gameSystem;
	gameSystem.SetFactory(&gameFactory);
	//> CSD-HK-030829
	DWORD dwResult;
	bool bServerRunning = false;
	
	INPUT_RECORD	irBuffer;
	memset(&irBuffer,0,sizeof(INPUT_RECORD));
	
	// Detach from parent console and create our own
	FreeConsole();
	AllocConsole();
	hIn = GetStdHandle(STD_INPUT_HANDLE);
	g_hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	
	// Set console mode: disable line input, enable processed input
	SetConsoleMode(hIn, ENABLE_PROCESSED_INPUT);
	
	// Bring console window to foreground
	HWND hConWnd = GetConsoleWindow();
	if (hConWnd) {
		SetForegroundWindow(hConWnd);
	}
	
	SetMonitorSize( SCREEN_TEXT_SIZE_X, SCREEN_TEXT_SIZE_Y );
	
	// Initialize LogManager
	InitMyLog();
	
	// 020620 YGI 
	extern void SetMenuFunctionsAll();
	SetMenuFunctionsAll();
	
	// Init Game Timers
	SettingGameTimers();
	
	// Init Lower Layer.
	if( !InitMapServer() )
		goto FinishMapServer;
	
	// MAP Server Datas loading
	if( !InitDRMapServerDatas() )
		goto FinishMapServer;
	
	// Clear Connection List
	CleanUpConnectionList();
	MyLog(LOG_NORMAL, "CleanUpConnectionList OK");

	CLottoSystem::Create();//soto-030505
	MyLog(LOG_NORMAL, "CLottoSystem::Create OK");
	
	g_CSymbolMgr.Init(hDBC,hDBC_ChrLogDB);//soto-030512
	g_CSymbolMgr.LoadTable();//soto-030512
	MyLog(LOG_NORMAL, "CSymbolMgr OK");

	CLottoSystem::Create();//soto-030505
	if(LottoSystem())
	LottoSystem()->LoadTable(hDBC);
	//<soto-030711
	CEventTreasureBoxMgr::Create();
	if(TreasureBoxMgr())
	{
		::TreasureBoxMgr()->LoadTable(hDBC);
	}
	//>soto-030711	
	// Try to connect with PROXY
	MyLog(LOG_NORMAL, "Resuming timers...");
	g_pINet->ResumeTimer(0);
	// Now Start GameTimers
	g_pINet->ResumeTimer(1);
	// Now Start GhostChecker
	g_pINet->ResumeTimer(2);
	MyLog(LOG_NORMAL, "Timers resumed. Entering main loop...");
	
	while ( g_pServerTable->IsServerRunning() )
	{
		// Use WaitForSingleObject with timeout to avoid blocking forever
		DWORD waitResult = WaitForSingleObject(hIn, 100); // 100ms timeout
		
		if (waitResult == WAIT_OBJECT_0) {
			INPUT_RECORD irBuffer;
			DWORD dwResult;
			if (ReadConsoleInput(hIn, &irBuffer, 1, &dwResult) && dwResult > 0) {
				if (irBuffer.EventType == KEY_EVENT && irBuffer.Event.KeyEvent.bKeyDown) {
					switch (irBuffer.Event.KeyEvent.wVirtualKeyCode) {
					case VK_F1:
						MyLog(LOG_NORMAL, "F1 pressed!");
						SetEvent(hKeyEvent[0]);
						break;
					case VK_F5:
						MyLog(LOG_NORMAL, "F5 pressed: Re-Loading GameServerData...");
						SetEvent(hKeyEvent[1]);
						break;
					case VK_F6:
						SetEvent(hKeyEvent[2]);
						break;
					case VK_F7:
						SetEvent(hKeyEvent[3]);
						break;
					case VK_ESCAPE:
						MyLog(LOG_IMPORTANT, "ESC pressed: MapServer shutting down.");
						g_pServerTable->DestroyServer(FINISH_TYPE_NORMAL);
						break;
					}
				}
			}
		}
	}
	
FinishMapServer:
	EndTextScript();//020314 lsw
	MyLog( LOG_NORMAL, "-- MapServer ShutDown :: Now Release SQL" );
	Release_SQL();
	
	MyLog( LOG_NORMAL, "-- MapServer ShutDown :: Now Release INetwork Module, ServerTable" );
	OnDestroy();
	
	MyLog( LOG_NORMAL, "-- MapServer Shutdown :: Now Release Console and Log Resources" );
	FreeConsole();
	FreeMyLog();

	CLottoSystem::Destroy();//soto-030505
	CEventTreasureBoxMgr::Destory();//soto-030711
	return 0;
}
