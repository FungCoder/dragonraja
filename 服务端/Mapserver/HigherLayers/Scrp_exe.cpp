#include "..\stdafx.h"
#include "..\LowerLayers\mylog.h"
#include "..\LowerLayers\servertable.h"

#include "Map.h"
#include "MenuDefine.h"
#include "Chatting.h"
#include "Scrp_int.h"
#include "quest_cpp.h"
#include "OP_magic.h" 
#include "citem.h"
#include "SealStoneGameServer.h"
#include "Scrp_exe.h"
#include "NationSys.h"	// 010904 LTS
#include "teambattle.h"
#include "LTSSupport.h"
#include "QuestFunction.h"	// 021015 kyo
#include "eventmgr.h"
#include "SymbolItemMgr.h"	// 030419 kyo
#include "ArenaManager.h"
#include "BattleManager.h"
#include "UserManager.h"
#include "KickOffAll.h"		// BBD 040329

int var[ DRAGON_MAX_CONNECTIONS_][ _SCRIPT_VARABLE_POINT_ ] = {0,};
char String[ MAX_STRING_]  = {0,};

int EventPC, EventNPC[ DRAGON_MAX_CONNECTIONS_] = {0,};

extern void UpdateCharacterData(t_connection c[], int cn, bool bDirect = false);
extern void SendCMD_MAGIC_POSITION( t_server_magic_position *p);
extern void SendCharInfoSkill( t_connection c[], int cn );
extern void makemyjobexpsetting( int skillno , CHARLIST *ch );
extern void SendLearnSkillOk( int skill_no, t_connection c[], int cn );
extern void RecvCMD_CLIENT_CHARACTER_DEMAND( int cn, t_client_character_demand *p ); 

extern CKickOffAll	g_KickMgr;	// BBD 040329

void LogInFile( string szFilePath,  char *szFormat, ... ) //kyo
{
	FILE *fp;
	if(!(fp=fopen(szFilePath.c_str(), "a+")))
	{
		return;
	}

	char szMessage[512];//
	va_list val;
	va_start(val, szFormat);
	vsprintf(szMessage, szFormat, val);
	va_end(val);

	time_t nowTime;
	struct tm *now;
	time( &nowTime );
	now = localtime(&nowTime);
	int year = 0, mon = 0, day = 0;
	int hour = 0, min = 0, sec = 0;
	// Make it usable.
	
	year = now->tm_year + 1900;
	mon  = now->tm_mon + 1;
	day  = now->tm_mday;
	hour = now->tm_hour;
	min  = now->tm_min;
	sec  = now->tm_sec;
	char szLog[512];
	sprintf( szLog, "%d %d/%d %d:%d:%d\t%s\n", year, mon, day, hour, min, sec, szMessage);

	if( !fputs( szLog, fp ) )
	{
		fclose(fp);
		return;
	}

	fclose(fp);			
}

void ReCallScript( void )
{	//< CSD-CN-031213
	CUserManager::HASH_USER mpUser = g_pUserManager->GetUserSet();

	for (CUserManager::ITOR_USER i = mpUser.begin(); i != mpUser.end(); ++i)
	{
		const int idUser = i->second;

		if (!var[idUser][1])
		{
			continue;
		}

		CHARLIST* pUser = CheckServerId(idUser);
		
		if (pUser != NULL)
		{
			if (var[idUser][3] < g_curr_time )
			{
				EventPC = idUser;
				Script(var[idUser][1]);
			}
		}
	}
}	//> CSD-CN-031213

int SearchItemByInv( int item_no, CHARLIST *ch )
{
	
	for( int a=0; a<3; a++ )
		for( int b=0; b<3; b++ )
			for( int c=0; c<8; c++ )
			{
				if( ch->inv[a][b][c].item_no == item_no ) 
				{
					return 1;
				}
			}
			
			return 0;
}

int SearchItemByInv( int item_no, CHARLIST *ch, int &a, int &b, int &c )
{
	for( a=0; a<3; a++ )
		for( b=0; b<3; b++ )
			for( c=0; c<8; c++ )
			{
				if( ch->inv[a][b][c].item_no == item_no ) 
				{
					return 1;
				}
			}
			
			return 0;
}


int SearchItemByEquip( int item_no, CHARLIST *ch )
{
	for( int a=0; a<8; a++ )
	{
		if( ch->equip[a].item_no == item_no ) return 1;
	}
	
	return 0;
}

int SearchItemByQuick( int item_no, CHARLIST *ch )
{
	for( int a=0; a<6; a++ )
	{
		if( ch->quick[a].item_no == item_no ) return 1;
	}
	
	return 0;
}

// 021107 YGI
int SendDial( short int cn, int npc_index, char *msg, int length )
{
	if( !CheckServerIndex( cn ) ) return 0;
	
	t_packet packet;
	packet.h.header.type = CMD_CHAT_DATA;
	{
		packet.u.server_chat_data.server_id = npc_index+10000;
		packet.u.server_chat_data.type		= (unsigned char)CHATTYPE_NORMAL;
		packet.u.server_chat_data.length = (unsigned char)length;
		memset(packet.u.server_chat_data.text, 0, TEXT_LENGTH );
		memcpy(packet.u.server_chat_data.text, msg, length);
	}
	packet.h.header.size = sizeof(t_server_chat_data)-TEXT_LENGTH+length;
	QueuePacket(connections, cn, &packet, 1);
	return 1;
}


/*??????????????????????????????????????
?Լ??? : void sDialogD( void )
??  ?? : ??ȭ?? ?ϴµ? ??ȭ?ϴ? ???? ?????? ???ΰ??? ???Ѵ?.
??  ?? :
??  ?? :
???????????????????????????????????????*/
void sDialOnlyPC( void )// 0
{		
	int length;
	
	// Client?? ??ȭ?? ?????ش?....
	GetString( String );
	length = strlen( String );
	SendDial( EventPC, EventNPC[EventPC], String, strlen( String ) );
	
	
	
	return;
}		

// Added by chan78 at 2000/11/28
void sSendMessageBox()			// 001212 YGI
{
	int length;
	t_packet packet;
	
	// Client?? ??ȭ?? ?????ش?....
	GetString( String );
	
	length = strlen( String );
	packet.h.header.type = CMD_SCRIPT_MESSAGE_BOX;
	{
		memcpy(packet.u.kein.default_msg, String, length);
	}
	packet.h.header.size = length+1;
	
	if(connections[EventPC].dwAgentConnectionIndex && connections[ EventPC].state >= CONNECT_JOIN)
		QueuePacket(connections, EventPC, &packet, 1);
	
	return;
}

void sSendSound()			// 001222 YGI
{
	t_packet packet;
	// Client?? ??ȭ?? ?????ش?....
	int type = GetInt();
	int sound = GetInt();
	
	CHARLIST *ch = CheckServerId( EventPC );
	if( !ch ) return;
	
	packet.h.header.type = CMD_SEND_SOUND;
	{
		packet.u.kein.default_short_int = sound;
	}
	packet.h.header.size = sizeof( short int );
	
	switch (type)
	{	//< CSD-CN-031213
	case 0:	
		{	// ?ڱ??ڽ????׸?
			QueuePacket(connections, EventPC, &packet, 1); 
			break;		
		}
	case 1:		
		{	// ?ֺ?????????
			CastMe2Other(EventPC, &packet);
			QueuePacket(connections, EventPC, &packet, 1); 
			break;
		}
	case 2:
		{	// ???? ??
			g_pUserManager->SendPacket(&packet);
			break; 
		}
	case 3:
		{
			g_pUserManager->SendPacket(&packet);
			SendNationPacket(&packet, ch->name_status.nation);	
			break;
		}
	case 4:
		{	// ????
			g_pUserManager->SendPacket(&packet);
			g_pServerTable->BroadCastToEveryServer((char*)&packet, (sizeof(t_header)+packet.h.header.size), SERVER_TYPE_MAP);
			break;
		}
	}	//> CSD-CN-031213
}

void sSendEffect()			// 010522 YGI
{
	int type = GetInt();
	int effect = GetInt();
	CHARLIST *ch = CheckServerId( EventPC );
	if( !ch ) return;
	
	t_packet p;
	p.h.header.type = CMD_SEND_EFFECT;
	p.u.kein.send_effect.effect_num = effect;
	p.u.kein.send_effect.target_id = EventPC;
	// 040105 YGI ????
	p.u.kein.send_effect.x = 0;
	p.u.kein.send_effect.y = 0;
	p.h.header.size = sizeof( k_send_effect );
	
	QueuePacket(connections, EventPC, &p, 1);
	
	if( type == 1 )	// ?ֺ???????Ե?
	{
		CastMe2Other( EventPC, &p );
	}
}


void sDialInAreaPC( void )
{
	GetString( String );
}

void sDialEveryone( void )
{
	GetString( String );
}


void sAddItem( void )// 1
{
	int x, y;
	ItemAttr i;
	
	
	i.item_no = GetInt();
	i.attr[0] = 1;
	i.attr[1] = 1;
	i.attr[2] = 0;
	i.attr[3] = 1;
	i.attr[4] = 1;
	i.attr[5] = 1;
	
	x =		 GetInt();
	y =		 GetInt();
	
	AddItemList(i.item_no, i.attr, 0, x, y, 0,0);
}


void sAddNpc( void)// 2
{
	int npcindex, x, y;
	
	npcindex =	GetInt();
	x =			GetInt();
	y =			GetInt();		  
	
	AddNPCList( npcindex, x, y );
}

void sSetQuestNumStep(void)
{		
	int quest_no=GetInt();
	int quest_value=GetInt();
	//	EventPC		// ??ũ??Ʈ?? ?????ϰ? ?ִ? ĳ???? ???̵? ?????? ?ִ?.
	//	connections	// ???? ?????? ???????ִ? Ŭ???̾?Ʈ ????ü ( ĳ???? ?????? ?????? ?ִ?. )
	
	//	CHARLIST *ch = &connections[EventPC].chrlst;
	//	if( !ch ) return;
	
	var[EventPC][quest_no]=quest_value;
	//return ch->var[quest_no] = quest_value;
	
}

void sSetQuestNumStep(int quest_no,int quest_value)
{	
	// EventPC		// ??ũ??Ʈ?? ?????ϰ? ?ִ? ĳ???? ???̵? ?????? ?ִ?.
	// connections	// ???? ?????? ???????ִ? Ŭ???̾?Ʈ ????ü ( ĳ???? ?????? ?????? ?ִ?. )
	// CHARLIST *ch = &connections[EventPC].chrlst;
	// if( !ch ) return;
	var[EventPC][quest_no] = quest_value;
}

void sCmpQuestNumStep( int quest_no, int quest_value)
{
	if(var[EventPC][quest_no] - quest_value > 0)
	{
		sYES=2;
	}
	if(var[EventPC][quest_no] - quest_value == 0)
	{
		sYES=1;
	}
	if(var[EventPC][quest_no] - quest_value < 0)
	{
		sYES=0;
	}
}

void sCmpQuestNumStep( void )
{
	int quest_no=GetInt();
	int quest_value=GetInt();
	if(var[EventPC][quest_no] - quest_value > 0)
	{
		sYES=2;
	}
	if(var[EventPC][quest_no] - quest_value == 0)
	{
		sYES=1;
	}
	if(var[EventPC][quest_no] - quest_value < 0)
	{
		sYES=0;
	}
}

void sMapName( void )// 3
{
	GetString( MapName );
	CharUpper( MapName );
	
	strcpy( MAPNAME, MapName );
	
	
	
	int i;
	for( i = 0 ; i < MAX_MAP_  ; i ++)
	{
		if( strcmp( MapInfo[i].mapfile, MAPNAME ) ==0 ) 
		{
			MapNumber = i;
			return;
		}
	}
	
	if( i == MAX_MAP_ ) Error( "*** Error : Can NOT find the MapNumber of '%s'", MapName );
}

extern void DeleteSquadMember(const int cn); // 010904 LTS//020903 lsw
void DecLocalWarfieldMemberCount(const int cn);	// LTS LOCALWAR
extern void CheckMapMove(int ID);			// LTS DRAGONLORDWAR
// Modified by chan78 at 2000/11/27

int MapMove(int idUser, const char* pMapName, int nX, int nY)
{	//< CSD-HK-030829

	//<! BBD 040329		?????????? ?ʹ??? ?????Ѵ?
	if(g_KickMgr.IsKicking())
	{
		return 0;
	}
	//> BBD 040329		?????????? ?ʹ??? ?????Ѵ?

	CHARLIST* pUser = CheckServerId(idUser);
	
	if (pUser == NULL)
	{
		return 0;
	}
	// ???̵??? ĳ???? ????Ÿ ????
	UpdateCharacterData(connections, idUser);

	LPMAPINFO pMapInfo = GetMapInfoByMapName(pMapName);
	
	if (pMapInfo == NULL)
	{
		return 0;
	}

	MAP_MOVE_INFO infMapMove;
	infMapMove.idUser = idUser;
	infMapMove.nMapPort = pMapInfo->port;
	infMapMove.nX = nX;
	infMapMove.nY = nY;
	g_pUserManager->Push(infMapMove);
	return 1;
}	//> CSD-HK-030829

int GotoMap(int cn, int nMapPort, int nMoveX, int nMoveY)
{	//< CSD-HK-030829
	CHARLIST *ch = ::CheckServerId(cn);
	if( !ch ) return 0;
	
	LPMAPINFO pMapInfo = GetMapInfoByMapPort(nMapPort);
	
	if (!pMapInfo)
	{
		return 0;
	}
	
	const int iLimitLvMin = pMapInfo->LvMin;//030116 lsw
	const int iLimitLvMax = pMapInfo->LvMax;
	if (!ch->IsCounselor() &&
		((iLimitLvMin && iLimitLvMin > ch->GetLevel()) || (iLimitLvMax && iLimitLvMax < ch->GetLevel())))
	{
		t_packet p;
		p.h.header.type = CMD_MAP_MOVE_LV_LIMIT;
		p.h.header.size = sizeof(MAPMOVELVLIMIT);
		
		p.u.Hwoa.rang.MapMoveLvLimit.iLvMin = iLimitLvMin;
		p.u.Hwoa.rang.MapMoveLvLimit.iLvMax = iLimitLvMax;
		
		::QueuePacket(connections,cn,&p,1);
		return 0;
	}
	
	if( !g_LightVersion.IsAbleMapMove( ch, pMapInfo->mapfile ) )
	{	// ????Ʈ ?????̶? ????
		ch->Message( MK_WARNING, 5, 28 );
		ch->Message( MK_WARNING, 5, 29 );
		return 0;		// ????Ʈ ?????̶? ????..
	}
	
	if (!ch->IsCounselor() && !ch->IsReporter()) // ??ڳ? ?????, ???ڴ? ???? ?ִ?.
	{
		const int type = pMapInfo->m_MoveType; //020903 lsw

		switch (type)
		{
		case MMT_NORMAL: 
			{
				break;		// ?̵??ϴµ? ?ƹ??? ?????? ????.
			}
		case MMT_FAIL: 		// ???? ???̴?.
		case MMT_NOT_MOVE: 
			{
				CallClient(cn, CMD_CHANGE_MAP_FAIL);
				return 0;		// ?̵??? ?? ????.
			}		
		}
	}
	
	if (ch->IsDead())
	{
		::SendViewTypeChange( ch->GetServerID(), VIEWTYPE_GHOST_ );//030102 lsw
	}
	
	t_packet dummy_packet;
	strcpy(connections[cn].mapname, pMapInfo->mapfile);
	// ??ü???? ???л?Ȳ?? ???? ???.
	ch->MapChange_MoveGox = ch->MoveGox;
	ch->MapChange_MoveGoy = ch->MoveGoy;
	ch->MapChange_X = ch->X;
	ch->MapChange_Y = ch->Y;
	ch->MapChange_MoveSx = ch->MoveSx;
	ch->MapChange_MoveSy = ch->MoveSy;
	ch->MapChange_MoveLength = ch->MoveLength;
	ch->MapChange_MovePathCount = ch->MovePathCount;
	// ?? ???? ???? ????.
	ch->MovePathCount	= 0;
	ch->MoveLength		= 0;
	ch->MoveGox = ch->X = nMoveX * TILE_SIZE + 16;
	ch->MoveGoy = ch->Y = nMoveY * TILE_SIZE + 16;
	ch->MoveSx = nMoveX;
	ch->MoveSy = nMoveY;
	strcpy( ch->MapName, pMapInfo->mapfile );
	// Packet Setting
	memcpy(dummy_packet.u.server_change_map.mapname, pMapInfo->mapfile, 20);
	dummy_packet.u.server_change_map.x = nMoveX;
	dummy_packet.u.server_change_map.y = nMoveY;
	// Build Packet by Querying.
	if(GetChangeMapPort(connections, cn, &dummy_packet) != 1) 
	{
		MyLog( LOG_FATAL, "sMapMove() :: Failed To Query" );
		goto NO_PORT_;
	}	
	// Copy it connections[]
	memcpy( &connections[cn].server_change_map, &dummy_packet.u.server_change_map, sizeof(t_server_change_map));
	// Get My Own Agent ServerID
	WORD wMyAgentServerID;
	if( g_pServerTable->GetServerData( connections[cn].dwAgentConnectionIndex ) )
	{	
		wMyAgentServerID = g_pServerTable->GetServerData( connections[cn].dwAgentConnectionIndex )->wPort;
	}	
	else
	{	
		wMyAgentServerID = 0;
	}	
	
	if (NewSendGameToLoginChangeMapData(cn, wMyAgentServerID, WORD(dummy_packet.u.server_change_map.port)))
	{
		DeleteSquadMember(cn);
		DecLocalWarfieldMemberCount(cn);
		connections[cn].state = CONNECT_CHANGING_MAP;
		CheckMapMove(cn);								// LTS DRAGONLORDWAR
		return 1;
	}

NO_PORT_:	
	// Ŀ?ؼ? ?? ????
	connections[cn].state = CONNECT_JOIN;
	strcpy(connections[cn].mapname, MapName);
	// ????س??Ҵ? ?????? ?ǵ?????.
	ch->MoveGox = ch->MapChange_MoveGox;
	ch->MoveGoy = ch->MapChange_MoveGoy;
	ch->X = ch->MapChange_X;
	ch->Y = ch->MapChange_Y;
	ch->MoveSx = ch->MapChange_MoveSx;
	ch->MoveSy = ch->MapChange_MoveSy;
	ch->MoveGox = ch->X;
	ch->MoveGoy = ch->Y;
	ch->MoveLength = ch->MapChange_MoveLength;
	ch->MovePathCount = ch->MapChange_MovePathCount;
	
	CallClient( cn, CMD_CHANGE_MAP_FAIL);
	return 0;
}	//> CSD-HK-030829

// 021117 YGI
void sMapMove( void )// 4
{
	k_map_move data;
	//char mapname[ 20];
	//short int	 x, y;
	
	GetString( data.mapname );				
	strupr( data.mapname );					
	data.x = GetInt();
	data.y = GetInt();
	
	//SendPacketDefault( CMD_MOVEMAP_FOR_LIGHT_VERSION, &data, sizeof( k_map_move )-20+strlen( data.mapname ), EventPC );
	//sYES = 1;
	
	sYES = MapMove( EventPC, data.mapname, data.x, data.y );
}

void sIsLevel( void )
{	//< CSD-030806
	const int lev = GetInt();
	// EventPC?? ?????? lev???? ?????? 1?? sYES?? ?ְ? ?׷??? ?????? 0?? ?ִ´?. 
	if (connections[EventPC].chrlst.GetLevel() >= lev)
	{
		sYES = 1;
	}
	else 
	{
		sYES = 0;
	}
	
}	//> CSD-030806

void sIsJobPC( void )
{
	int job = GetInt()-1+20;//?????? 0?? ???ϵ?... 1?????ͷ? ?????... 
	// EventPC?? ?????? job?̸?  1??  sYES?? ?ְ? ?׷??? ?????? 0?? ?ִ´?. 
	
	if( connections[ EventPC].chrlst.Job == job )
	{
		sYES = 1;
	}
	else 
	{
		sYES = 0;
	}
	
}

void sGetGender( void )			// 010210 YGI
{
	// 030930 kyo <<
	CHARLIST *ch = CheckServerId(EventPC);
	if( NULL == ch)
	{
		return;
	}
	// 030930 kyo >>

	sYES = connections[ EventPC].chrlst.Gender;		// 0 : ????, 1 : ????
}

void sQuestComplete(int magic_no)
{	//< CSD-TW-030606
	CHARLIST* pCaster = CheckServerId(EventPC);
	
	if (pCaster == NULL)
	{
		return;
	}

	t_packet packet;
	packet.h.header.type = CMD_MAGIC_EXECUTE;
	packet.h.header.size = sizeof(t_server_magic_execute);
	packet.u.magic.server_magic_execute.nMagic	= BYTE(magic_no);
	packet.u.magic.server_magic_execute.idCaster = pCaster->GetServerID();
	packet.u.magic.server_magic_execute.idTarget = pCaster->GetServerID();
	packet.u.magic.server_magic_execute.nX = pCaster->X ;
	packet.u.magic.server_magic_execute.nY = pCaster->Y ;
	QueuePacket(connections, EventPC, &packet, 1);
	CastMe2Other( EventPC, &packet);
}	//> CSD-TW-030606

void sQuestComplete(void)
{
	short int command_type = GetInt();
	t_client_character_demand p;
	p.target_id=connections[EventPC].chrlst.GetServerID();
	p.command_type = command_type;
	RecvCMD_CLIENT_CHARACTER_DEMAND( EventPC, &p);
	
}
void sQuestSound(void)
{
	//15001
	//
	short int command_type = GetInt();
	t_client_character_demand p;
	p.target_id=connections[EventPC].chrlst.GetServerID();
	p.command_type = command_type;
	RecvCMD_CLIENT_CHARACTER_DEMAND( EventPC, &p);
	
}
void sQuestTeleport(void)
{
	int commandNum = GetInt();
	int tile_X=GetInt();
	int tile_Y=GetInt();
	//	int count=0;
	//	t_packet packet;
	//	CHARLIST	*caster;
	t_server_magic_position	smp;
	
	//Ŭ???̾?Ʈ?? ???? ??Ŷ ????
	smp.nType =	PT_TELEPORT;
	smp.idTarget = EventPC;
	smp.nX = tile_X;
	smp.nY = tile_Y;
	SendCMD_MAGIC_POSITION(&smp);
}

void QuestTeleport( const int nCn, const int nX, const int nY ) 
{// 031030 kyo
	
	CHARLIST *ch = CheckServerId( nCn );
	if( !ch ) return;

	t_server_magic_position	smp;
	
	//Ŭ???̾?Ʈ?? ???? ??Ŷ ????
	smp.nType =	PT_TELEPORT;
	smp.idTarget = nCn;
	smp.nX = nX;
	smp.nY = nY;
	SendCMD_MAGIC_POSITION(&smp);
}

void sIsInvHavePC(void)
{		
	int ItemNum = GetInt();
	int ret;
	CHARLIST	*caster;
	caster=&connections[EventPC].chrlst;
	ret = SearchItemByInv(ItemNum,caster);
	if( ret==1 )
	{
		sYES=1;
	}
	else
	{
		ret = SearchItemByEquip(ItemNum,caster);
		if( ret==1 )
		{
			sYES=2;
		}
		else
		{
			ret = SearchItemByQuick(ItemNum,caster);
			if( ret==1 )
			{
				sYES=3;
			}
			else
			{	
				sYES=0;
			}
		}
	}
}

//YGI acer
void sSetItemInvPC(void)
{
	int change_no = GetInt();
	int item_no = GetInt();
	int item_dur = GetInt();
	//	int item_durmax = GetInt();
	
	CHARLIST *ch =&connections[EventPC].chrlst;
	for( int a=0; a<3; a++ )
		for( int b=0; b<3; b++ )
			for( int c=0; c<8; c++ )
				if( ch->inv[a][b][c].item_no == change_no ) 
				{
					SendItemEventLog( &ch->inv[a][b][c], EventPC, SN_NOT_USER, SILT_DELETE_BY_SCRIPT, 2 );//?ֱ???.

					ItemAttr target = GenerateItem( item_no, IATTR_MUCH,item_dur);//020509 lsw
					
					ch->inv[a][b][c] = target;
					if(target.item_no)
					{
						SendItemEventLog( &ch->inv[a][b][c], EventPC, SN_NOT_USER, SILT_MAKE_BY_SCRIPT, 2 );//??????
					}
					
					POS pos;
					SetItemPos( INV, a, b, c, &pos );
					SendServerEachItem(  &pos, &target, EventPC );		// 001028 YGI
					
					sYES=1;
					return;
				} 
				
				
				sYES=0;
}

void sSetSkillPC(void)
{
	int SkillNum = GetInt();
	int SkillLearn = GetInt();
	CHARLIST *ch =&connections[EventPC].chrlst;
	if( !ch ) return;
	
	if( SkillLearn ) 
	{
		ch->Skill[SkillNum ] = true;
		makemyjobexpsetting( SkillNum, ch );
		SendLearnSkillOk(SkillNum , connections, EventPC );	
	}
	return;
	
}

void Recv_CMD_QUEST_JOB_FINISH( int cn, int type  )
{	
	sCmpQuestNumStep(52,1997);
	if(sYES==1)
	{
		sSetQuestNumStep(52,1998);
	}
	
}



void sIsItemHavePC( void )
{		
	int ItemNum = GetInt();
	int ItemPos = GetInt();
	//?ش??ϴ? ???????? ?ش? ???  ?????ϰ? ??????.. ?????ƴϸ? ??
	if(connections[ EventPC].chrlst.equip[ItemPos].item_no == ItemNum)
	{
		sYES = 1;
	}
	else 
	{
		sYES = 0;
	}
	return;
	
	
}	
void sIsClassPC( void )
{		
	int ClassNum = GetInt();
	
	//?ش??ϴ? Ŭ??????.. ?????ƴϸ? ??
	if(connections[EventPC].chrlst.Class == ClassNum)
	{
		sYES = 1;
	}
	else 
	{
		sYES = 0;
	}
	return;
	
	
}

void sIsSkillHavePC(void)
{
	int SkillNum = GetInt();
	//?ش? ????? ?????? ???? ?ƴϸ? ?? 
	if(connections[EventPC].chrlst.Skill[SkillNum] == 1)
	{
		sYES = 1;
	}
	else
	{
		sYES = 0;
	}
	return;
}

void sCheckNation(void)
{
	int NationNum = GetInt();
	// ???? ?????? ????  
	
	if(connections[EventPC].chrlst.name_status.nation == (DWORD )NationNum)
	{
		sYES = 1;
	}
	else
	{
		sYES = 0;
	}
	return;
}	


// Monster?? ?߻??? ??ҿ? Sprite??ȣ?? ?????Ѵ?. 
void sInitMonsterGeneratedPosition( void )
{
	int x = GetInt();
	int y = GetInt();
	int npcindex = GetInt();
	
	return;//
	
	NPCGeneratePosition( npcindex, x, y, 0, 0 );
}

void sIsPlayArenaGame()		// ?????? ???????? ?ƴ???
{	//< CSD-030517
	const int nGame = GetInt();
	CBaseArena* pGame = g_pArenaManager->GetGameInfo(nGame);

	if (pGame == NULL)
	{
		sYES = 0;
		return;
	}

	if (!pGame->IsPlayGame())
	{
		sYES = 0;
	}
	else
	{
		sYES = 1;
	}
}	//> CSD-030517

void sAddFightMap( void )
{	//< CSD-RESERVE

}	//> CSD-RESERVE

void sDeleteFightMap( void )				// 914 YGI
{	//< CSD-RESERVE

}	//> CSD-RESERVE

void sIsFullFightMapTeam( void )		// 0915 YGI
{	//< CSD-RESERVE

}	//> CSD-RESERVE

int callmenudef[]= 
{ 
	MN_QUEST			,	// ?ӽ? 5?? YGI  0923
	MN_FAME				,
	MN_QUEST_INDEX		,
	MN_RELATION			,
	MN_MAN_TO_MAN		,
	
	MN_SKILLGUILD_BUY	,		
	MN_SKILLGUILD_SELL	,	
	MN_SKILLGUILD_SKILL	,
	MN_SKILLGUILD_ITEM	,	
	MN_SKILLGUILD_REPAIR,		// 9
	MN_BANK_DEPOSIT,
	MN_BANK_DEFRAYAL,
	MN_BANK_CUSTODY,
	MN_BANK_MAIN,
	MN_BANK_LOAN,
	MN_BANK_REPAYMENT,
	MN_BANK_AUCTION,
	MN_BANK_AUCTION_SALE,
	MN_BLACKSMITH			,
	MN_FARM		,			// 19
	MN_RESTAURANT,
	MN_HERBSHOP,	
	MN_SHAMBLES	,			// 22
	MN_CANDLE	,			// 23
	MN_SLOPSHOP	,
	MN_ALCHEMY	,
	MN_WOODWORKING,			// 26
	MN_BOWCRAFT	,
	MN_SHIPYARD	,
	MN_TOWNHALL ,		// 29
	MN_TOWNHALL_PUBLICNOTICE ,
	MN_TOWNHALL_BUILDING,
	MN_TOWNHALL_COMMERCE1,
	MN_TOWNHALL_COMMERCE2,
	MN_TOWNHALL_COMMERCE3,
	MN_TOWNHALL_HOUSEREPAIR,
	MN_TOWNHALL_GUILD,
	MN_TOWNHALL_FORUM,
	MN_TOWNHALL_EVENT,
	MN_TOWNHALL_RANK1,		// 39
	MN_TOWNHALL_RANK2,
	MN_TOWNHALL_NOTICE4,
	MN_TOWNHALL_NOTICE3,
	MN_TOWNHALL_NOTICE2,
	MN_TOWNHALL_NOTICE1,
	MN_TOWNHALL_COMMERCE4,
	MN_TOWNHALL_CONSTRUCTION,
	MN_MY_HOME,
	MN_MY_HOME_EXPLAN,
	MN_HOUSE,				// 49
	MN_MY_HOME_IN,
	MN_HOME_PERSON,
	MN_GOD1,
	MN_GOD2,
	MN_GOD3,
	MN_MAGICBOOK,
	MN_FUNCTION,
	MN_DRINK_MAIN,
	MN_DRINK_RUMOR	,
	MN_DRINK_BUY	,		// 59
	MN_SHOP_MAIN,			//60
	MN_SHOP_SELL,
	MN_SHOP_BUY	,
	MN_HEAL_MAIN,			//63
	MN_HEAL_HEALING,
	MN_HEAL_PREVENTING,
	MN_HEAL_LEARN,
	MN_INN_MAIN,			// 67
	MN_INN_REST,
	MN_INN_SLEEP,			// 69
	MN_SYSTEM_LOGOUT,		// 70
	MN_MAGIC_GUILD,			// 71 ?????? ???		// 0208_2 YGI
	MN_ACADEMY,				// 72 ??ī????			// 0208_2 YGI
	
	MN_GOD_EDELBLHOY,		// 73	????????? ????		// 0410 YGI
	MN_GOD_TEFFERY,			// 74	???丮 ????			// 0410 YGI
	MN_GOD_LETTY,			// 75	??Ƽ ????			// 0410 YGI
	
	MN_GUILD_ADMIN,			// 76	GM ??? ?޴? 		// 1027 YGI
	MN_SALVATION2,			// 77	?????? ?޴?			//  YGI				// 010522 YGI
	MN_CHANGE_MONEY,		// 78						// 010204 YGI
	
	MN_MERCHANT_MAIN,		// 79	// ???? ?޴? ????		// 010410 YGI
	MN_ARENA_LOBBY,			// 80	// ??Ʋ?޴? // CSD-030509
	
	MN_LADDER_RANK,			// 81	// 010511 YGI
	MN_GOD_OREM,			// 82		// 010525 YGI
	
	MN_NATION_MAIN_VA,		// 83
	MN_NATION_MAIN_ZY,		// 84
	MN_NATION_MAIN_ILL,		// 85

	MN_MERCHANT_OLDMAIN,	// 86	// ???ε?ϸ޴? ??ã?? ?޴? // BBD 040303 
	
};		

// 010530 YGI
extern void SendCallSmallMenu( int cn, int storelistindex, int menu );
void sCallMenu( void  )
{
	int menu = GetInt();
	SendCallSmallMenu( EventPC, 0, menu );
}

void sGetNationByMap( void )	// Ư?? ???? ?????˾? ????
{
	char	mapname[30];
	GetString( mapname );
	
	s_Nation = GetNationByMapName( mapname );
}

void sCalcFriendShip( void )	// Ư?? ?????? ?????ġ ?????ϱ?	// ???? ???????? -???? ?־? ?ش?.
{
	int nation = GetInt();
	int friend_ship = GetInt();
	
	CHARLIST *ch = CheckServerId( EventPC );
	if( !ch ) return;
	
	sYES = 1;
	
	short int *ch_friend_ship;
	switch( nation )
	{
	case N_VYSEUS :	ch_friend_ship = &ch->nk[ N_VYSEUS]; break;
	case N_ZYPERN : ch_friend_ship = &ch->nk[ N_ZYPERN]; break;
	case N_YILSE : ch_friend_ship =  &ch->nk[ N_YILSE]; break;
		
	default : sYES = 0; return;
	}
	
	*ch_friend_ship += friend_ship;
	return;
}

void sCompFriendShip( void )	// ?????ġ ???ϱ?
{
	int nation = GetInt();
	int comp_data = GetInt();
	
	CHARLIST *ch = CheckServerId( EventPC );
	if( !ch ) return;
	
	short int *ch_friend_ship;
	switch( nation )
	{
	case N_VYSEUS :	ch_friend_ship = &ch->nk[ N_VYSEUS]; break;
	case N_ZYPERN : ch_friend_ship = &ch->nk[ N_ZYPERN]; break;
	case N_YILSE  : ch_friend_ship = &ch->nk[ N_YILSE]; break;
		
	default : sYES = -2; return;
	}
	
	if( *ch_friend_ship > comp_data ) sYES = 1;
	else if( *ch_friend_ship == comp_data ) sYES = 0;
	else sYES = -1;
}

// 010531			YGI
void sCompLadderScore( void )
{
	int score = GetInt();
	
	CHARLIST *ch = CheckServerId( EventPC );
	if( !ch ) return;
	
	sYES = ch->LadderScore - score;
}


///////////////////////////////////////////////////////////////////////////x

void sCallSmallMenu( void )
{
	int index = GetInt();	// ???? List Index..
	int menu = GetInt();	// SmallMenu Index..

	
//<soto-030430
	if(256 == menu || 257 == menu || 258 == menu)
	{
		CallSmallMenu( EventPC, index, menu );
		return;
	}
//>soto-030430

	
	//020707 YGI
	if( menu < 1000 )
	{
		CallSmallMenu( EventPC, index, callmenudef[ menu] );
	}
	else
	{
		SendCallSmallMenu( EventPC, index, menu-1000 );
	}
}

void sWeatherOn( void )
{
	g_weatherflag = 1;
}

void sWeatherOff( void )
{
	g_weatherflag = 0;
}

void sRecallScriptStart( void )
{
	int aftertime;
	
	sRECALL_SCRIPT_COUNT	= GetInt();	
	aftertime				= GetInt();
	
	sRECALL_SCRP_NO  = NScriptNo; //    (var[EventPC][1])
	sRECALL_CURTIME = g_curr_time ;
	sRECALL_AFTTIME = g_curr_time + aftertime;
}
void sRecallScriptEnd( void )
{
	sRECALL_SCRIPT_COUNT = 0;
	sRECALL_SCRP_NO = 0;
}

void sMaxMonsterGeneration( void )
{
	Max_NPC_Generation = GetInt();	
	
	if( Max_NPC_Generation > MAX_NPC_LIST ) Max_NPC_Generation = MAX_NPC_LIST;
}

void sLoadItemValueOneMaul( void )
{
	int maulno = GetInt();
}

void srandom( void )		// 0131 YGI
{
	int start, end;
	start = GetInt();
	end = GetInt();
	var[EventPC][5] = (rand()%( end - start )) + start;
}

void sEventMsg( void )// 0
{		
	// Client?? ??ȭ?? ?????ش?....	
	GetString( String );
	
	if(connections[ EventPC].dwAgentConnectionIndex && connections[ EventPC].state >= CONNECT_JOIN)
	{
		SendBBS2( String, (short)EventPC );
		//SendBBS2??? ?̺?Ʈ ?޼??? ?߻? ?Լ??? ?????Ѵ?.
		//QueuePacket(connections, EventPC, &packet, 1);
	}	
	
	return;
}		

void sReturnSealStoneBreakable( void )
{		
	switch( ReturnSealStoneBreakable() )
	{	
	case CMD_STEALSTONE_BREAKABLE	 :			sYES = 1;	break;
	case CMD_STEALSTONE_NON_BREAKABLE_PEACE	:	sYES = 2;	break;
	case CMD_STEALSTONE_NON_BREAKABLE:			sYES = 0;	break;
	default :									sYES = 0;	break;
	}	
}		

// 020428 YGI acer
void sSaveItemAtDb( void )
{
	int a, b, c;
	POS pos;
	ItemAttr *pItem;
	
	int item_no = GetInt();
	CHARLIST *ch = CheckServerId( EventPC );
	if( !ch ) goto ERROR__;
	
	if( !SearchItemByInv( item_no, ch, a, b, c ) ) goto ERROR__;
	
	// ???? ???????? ????? ???? ??????.
	
	SetItemPos( INV, a, b, c, &pos );
	pItem = GetItemByPOS( EventPC, pos );
	if( !pItem ) goto ERROR__;
	
	// 020428 YGI acer
	SendItemEventLog( pItem, EventPC, 0, SILT_DELETE_BY_SCRIPT, 7);	 //YGI acer
	SendDeleteItem( pItem, &pos, ch );
	SendSaveEventItem( ch, item_no, 1);		// ?̺?Ʈ ?????? ??? ????
	
	sYES = 1;
	return;
	
ERROR__ :
	sYES = 0;
	return;
}

void sCheckMoney( void )	// ?ǽ??? ???? ?˾ƿ???
{	
	int money = GetInt();
	CHARLIST *ch = CheckServerId( EventPC );
	if( !ch ) return;
	
	DWORD pc_money;
	
	s_PcMoney = pc_money = GetMoneyByItem( ch );
	
	//sYes = pc_money - money;		// ?????? ?????? ??????쿣 ???? ????
	if( pc_money > money ) sYES = 1;		// ?????? ?????? ???????
	else if( pc_money == money ) sYES = 0;
	else sYES = -1;
}

// ?ǽÿ??Լ? ?? ???
void sSubtractMoney( void )
{
	int money = GetInt();
	CHARLIST *ch = CheckServerId( EventPC );
	if( !ch ) return;
	
	DWORD pc_money;
	pc_money = GetMoneyByItem( ch );
	if( pc_money < money ) money = pc_money;
	
	SubtractMoney( money, ch );
}

// 010129 KHS		
// ???Ͱ? ?߻? ?????? ?о???δ?. 
// ?????? NPC?? ?߻??? .SKBȭ?Ͽ? ??? ?ִ? Data?? ?̿??ϴ°??? ??Ģ???? ??????
// Quest???ǿ? ?´? NPC?? ?߻???Ű?? ???? ?߰? ?Ͽ???. 
// ?̴? 10?и??? ?????? ?˻??Ͽ? ???ǿ? ?????? ?߻??Ѵ?. 
void sGenMonSpec( void )
{	
	
/*
int x, y;
int monsprno;
int lev;
int rate;
int time;
int iftime;
int turn;

  x			= GetInt();	// ???? ??ǥ.(????: Tile )
  y			= GetInt();
  monsprno	= GetInt();	// ?????? ??ȣ.
  lev			= GetInt();	// ?߻??? Monster?? Level
  rate		= GetInt();	// ?߻?Ȱ?? ( 10000 = 100% )
  
	genyear		= GetInt();	// ?߻????ǽð??ð?( ?? ?ð????? ???? ?Ʒ? ?ð??? ???? ?߻??Ѵ?. )
	genmon		= GetInt();	
	genday		= GetInt();	
	genhour		= GetInt();	
	genmin		= GetInt();	
	
	  iftime		= GetInt();	// ???? gen*?? ?ð??? ???̸? ?߻????????? ?Ѵ?. 
	  
		turn		= GetInt();	// ???ϸ??? ?߻??ϴ°?.( 1?̸? 10?и??? ?߻??Ѵ?. )
	*/
}

void sGetGuild()		// 010511 YGI
{	//< CSD-030324
	CHARLIST *ch = CheckServerId( EventPC );
	if( !ch ) return;
	
	sYES = ch->GetGuildCode();
	return;
}	//> CSD-030324

extern int g_pLoadingPoint[NW_NATION_COUNT][NW_SQUAD_MAX]; // 010904 LTS

void sCheckLoadingPoint() // 010904 LTS	
{
	CHARLIST* ch=CheckServerId(EventPC);   //?????? ????ڸ? ?и??ؾ? ?Ѵ?... ??.?? 
	if (!ch) return;
	sYES=g_pLoadingPoint[ch->NWCharacter.WarfieldNo][ch->NWCharacter.SquadNo];	// 010915 LTS
}

extern bool CheckDefence(int cn); // 010904 LTS
extern bool CanGoBonus();			// 010915 LTS

void sCheckDefence()			// ????? ???? Ȯ???Ѵ?. 
{
	if (CheckDefence(EventPC)) sYES=1;
	else sYES=0;
}

void sCanGoBonus()
{
	if (CanGoBonus()) sYES=1;
	else sYES=0;
}

void sFameDown()
{
	int tempValue=GetInt();
	SaveChangeFame(&connections[EventPC].chrlst,connections[EventPC].chrlst.fame,connections[EventPC].chrlst.fame-tempValue,LF_SCRIPT);		// 020909 LTS
	connections[EventPC].chrlst.fame-=tempValue;
}

void sFameUp()
{
	int tempValue=GetInt();
	SaveChangeFame(&connections[EventPC].chrlst,connections[EventPC].chrlst.fame,connections[EventPC].chrlst.fame+tempValue,LF_SCRIPT);		// 020909 LTS
	connections[EventPC].chrlst.fame+=tempValue;
}

void sFameCheck()
{
	int tempValue=GetInt();
	if (connections[EventPC].chrlst.fame>=tempValue) sYES=1;
	else sYES=0;
}
//_________________________________________________________________	// 010915 LTS

// CSB AddCode
// CSB AddCode
// CSB AddCode
extern void (*SFunction[_SCRIPT_FUNCTION_POINT_ ])(void);

QT QTickData[DRAGON_MAX_CONNECTIONS_];
DWORD QTemp, QTick = 1;

// ?ʴ? ?ѹ??? ???Ե?
void QuestSecond()	// 1000 ms
{	
	QTick++;
	
	for (QTemp = 0; QTemp < DRAGON_MAX_CONNECTIONS; QTemp++) 
	{	
		if (QTick == QTickData[QTemp].Tick) 
		{		
			if (!QTickData[QTemp].Func) 
			{
				continue;
			}
			
			QTickData[QTemp].Func();
		}
	}
}		

void TimerFirstFunc( void ) 
{	
	// CSB ?ڵ??ʿ?
	
	// ???߿? ?̰??? ?ش? ????( QTemp )?? ????Ʈ ??????(??й????)
	// ?? ?ִ??? üũ?Ͽ? ?????ְ? ???????? ?ʾ??ٴ? ?޽????? ???? ?־???Ѵ?.
	// ????Ʈ ????(????Ʈ ??ȣ ????)?? Ư?? ????(????--;)?? ???????־?? ?Ѵ?.
	// 2~3?? ?ڵ? ??? ( ???? ?????? ?? )
	
	// ????(????)
	// if( ??й????? ?ִ°?? ) {
	//		????Ʈ ???? ????
	//		2?? ?????? ???????? ?˸?
	//		??й??? ????
	// } 
	
	int change_no = 0;
	if( SearchItemByInv(10176,&connections[QTemp].chrlst) ) change_no = 10176;
	if( SearchItemByInv(10177,&connections[QTemp].chrlst) ) change_no = 10177;
	if( SearchItemByInv(10178,&connections[QTemp].chrlst) ) change_no = 10178;
	if( change_no ) {
		
		var[QTemp][ 56 ] = 10;
		
		
		int item_no = 0;
		int item_dur = 0;
		
		CHARLIST *ch =&connections[QTemp].chrlst;
		for( int a=0; a<3; a++ )
			for( int b=0; b<3; b++ )
				for( int c=0; c<8; c++ )
					if( ch->inv[a][b][c].item_no == change_no ) 
					{
						ItemAttr target = GenerateItem( item_no, IATTR_MUCH, item_dur);//020509 lsw
						ch->inv[a][b][c] = target;
						POS pos;
						SetItemPos( INV, a, b, c, &pos );
						SendServerEachItem(  &pos, &target, QTemp );
						sYES=1;
						return;
					} 
					
	}
	
	int length;
	t_packet packet;
	
	// Client?? ??ȭ?? ?????ش?....
	strcpy( String, "2???? ???" );
	
	length = strlen( String );
	packet.h.header.type = CMD_SCRIPT_MESSAGE_BOX;
	{
		memcpy(packet.u.kein.default_msg, String, length);
	}
	packet.h.header.size = length+1;
	
	if(connections[QTemp].dwAgentConnectionIndex && connections[ QTemp].state >= CONNECT_JOIN)
		QueuePacket(connections, QTemp, &packet, 1);
	
	return;
	
}

void TimerSecondFunc( void )
{
	
	// ?̰??? ?ڵ?.
	
}

// Ư?? ?ð??? ????ɼ? ?ִ? ?Լ? ????Ʈ
void (*TimerFunc[_SCRIPT_FUNCTION_POINT_])(void) =
{	
	TimerFirstFunc,	 // 0 ?? Ÿ?̸? ?Լ?
	TimerSecondFunc, // 1 ?? Ÿ?̸? ?Լ?
};

// ?????? Ÿ?̸? ????
void sSetTimer()
{
	QTickData[EventPC].Tick = QTick + GetInt();	   // ?? ???? ???? (?ִ? ???? ???? 1??)
	GetInt();									   // ????
	QTickData[EventPC].Func = TimerFunc[GetInt()]; // ???? ????? ?Լ?
	QTickData[EventPC].ExtInt = GetInt();		   // ?????? ???? Ȯ?尪(Integer)
	GetString(QTickData[EventPC].ExtChar);		   // ?????? ???? Ȯ?尪(String)
	
}
// ?????? Ÿ?̸? ????
void sStopTimer()
{
	QTickData[EventPC].Func = NULL;
}
// ???? ???????? - yes ?? ???? ????
void sGetLevel()	
{	//< CSD-030806
	sYES = connections[EventPC].chrlst.GetLevel();
}	//> CSD-030806
// Ŭ???? ???????? - yes ?? Ŭ?????? ????
void sGetClass()
{
	sYES = connections[EventPC].chrlst.Class;
}
// ?????Ƽ ???????? - yes ?? ?????Ƽ ??ȣ ????
void sGetAbil()
{
	int abil = GetInt();
	sYES = 0;
	
	switch( abil ) 
	{
	case 1:  sYES = connections[EventPC].chrlst.Str;   break;
	case 2:  sYES = connections[EventPC].chrlst.Con;   break;
	case 3:  sYES = connections[EventPC].chrlst.Dex;   break;
	case 4:  sYES = connections[EventPC].chrlst.MoveP; break;
	case 5:  sYES = connections[EventPC].chrlst.Wis;   break;
	case 6:  sYES = connections[EventPC].chrlst.Int;   break;
	case 7:  sYES = connections[EventPC].chrlst.wsps;  break;
	case 8:  sYES = connections[EventPC].chrlst.Char;  break;
	case 9:  sYES = connections[EventPC].chrlst.Endu;  break;
	case 10: sYES = connections[EventPC].chrlst.Moral; break;
	case 11: sYES = connections[EventPC].chrlst.Luck;  break;
	}
}
// Ư?? ?????? ??Ƽ?? ?ξ????? Ȯ?? - yes?? ?????? 1 ?????? 0
void sCheckParty()
{
	char id[0xff];
	int ti;
	
	GetString( id );
	
	sYES = 0;
	for( ti = 0; ti < MAX_PARTY_MEMBER; ti++ ) 
	{
		if( connections[ EventPC ].chrlst.party[ti].On
			&& !strcmp( connections[ EventPC ].chrlst.party[ti].Name, id ) ) sYES = 1;
	}
	
	
}
// Ư?? ?????? ?????? ?????? - yes ?? ???? ??ȣ
void sGetNat()
{
	sYES = connections[ EventPC ].chrlst.name_status.nation;
}
// ???? ???? ?????? - yes ?? ???? ???? ??ȣ ??ȯ
void sGetSpell()
{
	sYES = connections[ EventPC ].chrlst.Spell;
}
// ??Ƽ ?? ??ȯ - yes ?? ???? ?ΰ? ?ִ? ??Ƽ?? ??ȯ
void sCheckPartyCount()
{
	int ti;
	
	sYES = 0;
	for( ti = 0; ti < MAX_PARTY_MEMBER; ti++ ) {
		if( connections[ EventPC ].chrlst.party[ti].On ) sYES++;
	}
}
// ?ֱ? ????? ???? - yes ?? ?ֱٿ?(????????) ????? ??????ȣ ??ȯ
void sCheckLastMagic()
{
	sYES = connections[ EventPC ].chrlst.GetMagic();
}
// ??ƽ ???????? - yes ?? ???ڰ????? ???? ??ƽ?? ??ȯ
void sGetTactic()
{
	int tac = GetInt();	
	sYES = connections[ EventPC ].chrlst.Skill[ 87+tac ];
}
// ??? ??ƽ?? Ư?? ??ġ?̻??? ??ƽ?? ?ִ??? üũ - yes ?? ?????? 0 ?????? 1 ??ȯ
void sCheckTacticPoint()
{
	int mode = GetInt();
	int point = GetInt();
	int ti;
	
	sYES = 0;
	for( ti = 0; ti < 13; ti++ ) {
		
		if( ( mode == 0 && point > connections[ EventPC ].chrlst.Skill[ 88+ti ] )
			|| ( mode == 1 && point == connections[ EventPC ].chrlst.Skill[ 88+ti ] )
			|| ( mode == 2 && point < connections[ EventPC ].chrlst.Skill[ 88+ti ] ) ) {
			
			sYES = 1;
			break;
			
		}
		
	}
	
}


// ??? ???? ?ƴ??? - yes ?? ???ڰ????? 0?? ?ƴ? 1?? ??? ??ȯ
void sIsDual()
{
	sYES = connections[ EventPC ].chrlst.IsDual();
}


// ??? ??û
void sCallCustomSmallMenu( void )
{
	
	int iMenuNo = GetInt();
	int iPicNo = GetInt();
	int iWidth = GetInt();
	int iHeight = GetInt();
	int iExt = GetInt();
	
	t_packet p;
	p.h.header.type = CMD_CALL_SMALLMENUCUSTOM;
	{
		p.u.server_call_smallmenucustom.iMenuNo = iMenuNo;
		p.u.server_call_smallmenucustom.iPicNo = iPicNo;
		p.u.server_call_smallmenucustom.iWidth = iWidth;
		p.u.server_call_smallmenucustom.iHeight = iHeight;
		p.u.server_call_smallmenucustom.iExt = iExt;
	}
	p.h.header.size = sizeof( t_server_call_smallmenucustom );
	
	QueuePacket(connections, EventPC, &p, 1);
	
}

// 021117 YGI
void sSetRareItemInvPc(void)
{
	const int iItemNo	= GetInt();
	const int iTryMulti	= GetInt();
	const int iGradeMin	= GetInt();
	const int iGradeMax	= GetInt();
	const int iMutant	= GetInt();
	
	CHARLIST *ch = ::CheckServerId( EventPC );
	if( !ch ) return;
	
	ItemAttr item;//021213 lsw 0???? ???????? ???? ????
	const int iGradeGab = iGradeMax-iGradeMin;
	const int iResultGrade = iGradeMin + ((0 < iGradeGab)?rand()%iGradeGab:0);
	const int ret = ItemMgr.MakeDefaultRareItem( &item, iItemNo, iMutant, iResultGrade, iTryMulti );
	if( ret == 1 )
	{
		::SendItemEventLog( &item, EventPC, SN_NOT_USER, SILT_MAKE_BY_SCRIPT, 2);
		::MoveEmptyInv( &item, ch );
		sYES = 1;
		return;
	}
	sYES=0;
}

// 021106 kyo
int SetRareItemInvPc( const int cn, const int iItemNo, const int iGradeMin, const int iGradeMax, const int iTryMulti, const int iDur, const int iHigh)
{
	if( iHigh != 1 && iHigh != 0 )
		return 0;
	
	int a,b,c;
	CHARLIST *ch =CheckServerId(cn);//&connections[cn].chrlst;
	if( !ch ) return -1;
	
	RareMain RareAttr;
	ItemAttr  Item = ItemMgr.GiveRareItem(iItemNo, iDur ,RARE_ABLE,iTryMulti, iGradeMin, iGradeMax,RareAttr,iHigh);//020725 lsw
	
	if( Item.item_no )
	{
		if( SearchInv( ch->inv, a, b, c ))
		{
			ch->inv[a][b][c] = Item;
			POS pos;
			SetItemPos( INV, a, b, c, &pos );
			SendServerEachItem(  &pos, &Item, cn);		// 001028 YGI
			return 1;
		}
	}
	return 0;
}

extern int g_LocalWarBegin;										// LTS LOCALWAR	
extern cNation* g_pNation;
extern cLocalWarfield* g_pLocalWarfield;						// LTS NEW LOCALWAR

extern bool isNationManageServer();	
extern void SendPacket2NWManager(t_packet* p);

void sLocalWarMode()											
{
	MyLog(0,"LocalWarMode Call LocalWar Status : %d, User Name : %s, JoinLocalWar : %d",g_LocalWarBegin,connections[EventPC].name,connections[EventPC].chrlst.JoinLocalWar);
	if (g_LocalWarBegin&&connections[EventPC].chrlst.JoinLocalWar) sYES=1;
	else sYES=0;
}

void sFameCheck2()
{
	int Type=GetInt();
	int Value=GetInt();
	
	switch(Type)
	{
	case 1 : 
		if (connections[EventPC].chrlst.fame>=Value) sYES=1;
		else sYES=0;
		break;
	case 2 :
		if (connections[EventPC].chrlst.GainedFame>=Value) sYES=1;
		else sYES=0;
		break;
	}
}

void sFameUp2()
{
	int Type=GetInt();
	int Value=GetInt();
	
	switch (Type)
	{
	case 1 : 
		SaveChangeFame(&connections[EventPC].chrlst,connections[EventPC].chrlst.fame,connections[EventPC].chrlst.fame+Value,LF_SCRIPT);		// 020909 LTS
		connections[EventPC].chrlst.fame+=Value;		break;
	case 2 : 
		connections[EventPC].chrlst.GainedFame+=Value;	break;
	}
}

void sNationBelong()
{
	int Nation=connections[EventPC].chrlst.name_status.nation;
	if (Nation==NW_YL)	
		Nation=connections[EventPC].chrlst.NWCharacter.YL_JoinNation;
	
	switch (Nation)
	{
	case NW_BY : sYES=1; break;
	case NW_ZY : sYES=2; break;
	default : sYES=3; break;
	}
}

void FameUp2(const int cn, const int Type, const int Value)	// 021107 kyo
{
	switch (Type)
	{
	case 1 : 
		SaveChangeFame(&connections[cn].chrlst,connections[cn].chrlst.fame,connections[cn].chrlst.fame+Value,LF_SCRIPT);		// 020909 LTS
		connections[cn].chrlst.fame+=Value;		break;
	case 2 : 
		connections[cn].chrlst.GainedFame+=Value;	break;
	}
}

void sFameDown2()
{
	int Type=GetInt();
	int Value=GetInt();
	
	switch (Type)
	{
	case 1 : 
		SaveChangeFame(&connections[EventPC].chrlst,connections[EventPC].chrlst.fame,connections[EventPC].chrlst.fame-Value,LF_SCRIPT);		// 020909 LTS
		connections[EventPC].chrlst.fame-=Value;		break;
	case 2 : connections[EventPC].chrlst.GainedFame-=Value;	break;
	}
}


void sLocalWarPointUp()
{
	t_packet packet;
	
	int NationIndex=GetInt();
	int Value=GetInt();
	
	switch (NationIndex)
	{
	case 3 : NationIndex=0; break;
	case 4 : NationIndex=1; break;
	case 6 : NationIndex=2;	break;
	}
	
	if (isNationManageServer())
	{
		g_pNation->IncLocalWarPoint(NationIndex,Value);
	}
	else
	{
		packet.h.header.type=CMD_LOCALWAR_NATION_POINT_CHANGED;
		switch(NationIndex)
		{
		case 0 :
			packet.u.LocalWar.NationPointChanged.NationPoint[0]=Value;
			packet.u.LocalWar.NationPointChanged.NationPoint[1]=0;
			packet.u.LocalWar.NationPointChanged.NationPoint[2]=0;
			break;
		case 1 :
			packet.u.LocalWar.NationPointChanged.NationPoint[0]=0;
			packet.u.LocalWar.NationPointChanged.NationPoint[1]=Value;
			packet.u.LocalWar.NationPointChanged.NationPoint[2]=0;
			break;
		case 2 :
			packet.u.LocalWar.NationPointChanged.NationPoint[0]=0;
			packet.u.LocalWar.NationPointChanged.NationPoint[1]=0;
			packet.u.LocalWar.NationPointChanged.NationPoint[2]=Value;
			break;
		}
		packet.h.header.size=sizeof(t_NationPointChanged);
		SendPacket2NWManager(&packet);
	}
}

void sLocalWarPointDown()
{
	t_packet packet;
	
	int NationIndex=GetInt();
	int Value=GetInt();
	
	Value*=-1;	//?????? ????? ?ش?.
	
	switch (NationIndex)
	{
	case 3 : NationIndex=0; break;
	case 4 : NationIndex=1; break;
	case 6 : NationIndex=2;	break;
	}
	
	if (isNationManageServer())
	{
		g_pNation->IncLocalWarPoint(NationIndex,Value);
	}
	else
	{
		packet.h.header.type=CMD_LOCALWAR_NATION_POINT_CHANGED;
		switch(NationIndex)
		{
		case 0 :
			packet.u.LocalWar.NationPointChanged.NationPoint[0]=Value;
			packet.u.LocalWar.NationPointChanged.NationPoint[1]=0;
			packet.u.LocalWar.NationPointChanged.NationPoint[2]=0;
		case 1 :
			packet.u.LocalWar.NationPointChanged.NationPoint[0]=0;
			packet.u.LocalWar.NationPointChanged.NationPoint[1]=Value;
			packet.u.LocalWar.NationPointChanged.NationPoint[2]=0;
		case 2 :
			packet.u.LocalWar.NationPointChanged.NationPoint[0]=0;
			packet.u.LocalWar.NationPointChanged.NationPoint[1]=0;
			packet.u.LocalWar.NationPointChanged.NationPoint[2]=Value;
		}
		packet.h.header.size=sizeof(t_NationPointChanged);
		SendPacket2NWManager(&packet);
	}
}
/*
extern t_Warfield	g_pWarfieldStatus[NW_NATION_COUNT];		
extern cWarfield*	g_pWarfield;
extern bool			isNationWarfieldServer();

  
	void sWarfieldMany()				// 011213 LTS		// ????
	{
	int PoseNo=GetInt();
	int BYPose=0,ZYPose=0;
	int Nation;
	
	  if (!isNationWarfieldServer())  {sYES=0; return; }
	  
		if (g_pWarfieldStatus[0].Possession==NW_BY) BYPose++;
		else ZYPose++;
		if (g_pWarfieldStatus[1].Possession==NW_BY) BYPose++;
		else ZYPose++;
		if (g_pWarfieldStatus[2].Possession==NW_BY) BYPose++;
		else ZYPose++;
		
		  Nation=g_pWarfieldStatus[g_pWarfield->GetWarfieldCode()].Possession;
		  if (Nation==NW_BY)
		  {
		  if (BYPose>=PoseNo) sYES=1;
		  else sYES=0; 
		  }
		  else
		  {
		  if (ZYPose>=PoseNo) sYES=1;
		  else sYES=0; 
		  }
		  }
		  
			void sWarfieldOwner()
			{
			int WarfieldNo=GetInt();
			int Nation=GetInt();
			
			  if (Nation==g_pWarfieldStatus[WarfieldNo].Possession) sYES=1;
			  else sYES=0;
			  }
*/
void sSetOXSelect()
{
	int type = GetInt();
	
	CHARLIST *ch = CheckServerId( EventPC );
	
	if( !ch ) return;
	SetOXSelect( ch , type );
}

// 020620 YGI-------------------------
extern void SendCheckGotoScenarioDungeon( short int cn, char *map_name, int x, int y );
void sMoveDungeon()
{
	char map_name[20];
	GetString( map_name);
	int x = GetInt();
	int y = GetInt();
	SendCheckGotoScenarioDungeon( EventPC, map_name, x, y );
}
//-------------------------------------

// 020707 YGI
void sDualCheck()
{
	int type = GetInt();
	
	CHARLIST *ch = CheckServerId( EventPC );
	if( !ch ) return;
	
	sYES = (type == ch->GetClassStep()) ? 1:0;
}

// ???????Ʒ? ?̵?
void sGotoGuildHouse()					// ?迵??
{	//< CSD-030324
	CHARLIST *ch = CheckServerId( EventPC );
	if( !ch ) return;
	
	sYES = g_GuildHouse.GoToHouse( ch );
	
	if (sYES)
	{
		t_packet packet;
		packet.h.header.type=CMD_GOTO_GUILD_HOUSE_RESULT;
		packet.u.LocalWar.CommonDataDw.Data=ch->GetGuildCode();
		packet.h.header.size=sizeof(t_CommonDataDw);
		QueuePacket(connections,ch->GetServerID(),&packet,1);
	}
}	//> CSD-030324

// ?ڱ? ??? ?????? Ȯ??
void sIsMyGuildHouse()			//?迵??
{
	int event_no = GetInt();
	CHARLIST *ch = CheckServerId( EventPC );
	if( !ch ) return;
	
	sYES = g_GuildHouse.IsMyGuildHouse( ch, event_no );
	
	CGuildHouseUnit *house = g_GuildHouse.GetPointByEventNo( MapName, event_no );
	if (!sYES)
	{
		if (house) house->GoToHouse(ch->GetServerID());
	}
	
	t_packet packet;
	packet.h.header.type=CMD_CHECK_GUILD_HOUSE_RESULT;
	if (house) packet.u.LocalWar.CommonDataDw.Data=house->guild_code;
	else packet.u.LocalWar.CommonDataDw.Data=0;
	packet.h.header.size=sizeof(t_CommonDataDw);
	QueuePacket(connections,ch->GetServerID(),&packet,1);
}

extern int g_EventLocalWarStatus;
extern int g_EventLocalWarStep;

void sCheckEventLocalWarStatus()			// 020115 LTS
{
	sYES=g_EventLocalWarStatus;
}

void sCheckEventLocalWarStep()			// 020115 LTS
{
	sYES=g_EventLocalWarStep;
}

void sCheckEventJoin()					// 020115 LTS	//?̺?Ʈ ??ȣ?? ?????? ?????? ?ƴ????? ????
{
	int EventNo=GetInt();
	switch (EventNo)
	{
	case 1	: sYES=connections[EventPC].chrlst.EventJoin.Event1; break;
	case 2	: sYES=connections[EventPC].chrlst.EventJoin.Event2; break;
	case 3	: sYES=connections[EventPC].chrlst.EventJoin.Event3; break;
	case 4	: sYES=connections[EventPC].chrlst.EventJoin.Event4; break;
	case 5	: sYES=connections[EventPC].chrlst.EventJoin.Event5; break;
	case 6	: sYES=connections[EventPC].chrlst.EventJoin.Event6; break;
	case 7	: sYES=connections[EventPC].chrlst.EventJoin.Event7; break;
	default : sYES=connections[EventPC].chrlst.EventJoin.Event8; break;
	}
}
void sClearEventJoin()
{
	int EventNo=GetInt();
	t_packet packet;
	packet.h.header.type=CMD_DELETE_EVENT_JOIN;
	strcpy(packet.u.LocalWar.DeleteEventJoin.UserName,connections[EventPC].name);
	packet.u.LocalWar.DeleteEventJoin.EventNo=EventNo;
	packet.h.header.size=sizeof(t_DeleteEventJoin);
	QueuePacket(connections, DB_DEMON, &packet, 1);
	
	switch (EventNo)
	{
	case 1	:	connections[EventPC].chrlst.EventJoin.Event1=0;	break;
	case 2	:	connections[EventPC].chrlst.EventJoin.Event2=0; break;
	case 3	:	connections[EventPC].chrlst.EventJoin.Event3=0; break;
	case 4	:	connections[EventPC].chrlst.EventJoin.Event4=0; break;
	case 5	:	connections[EventPC].chrlst.EventJoin.Event5=0; break;
	case 6	:	connections[EventPC].chrlst.EventJoin.Event6=0; break;
	case 7	:	connections[EventPC].chrlst.EventJoin.Event7=0; break;
	case 8	:	connections[EventPC].chrlst.EventJoin.Event8=0; break;
	}
}

extern int	 GetNationIndex(int Nation);			// LTS NEW LOCALWAR

void sCheckAllSealStoneBroked()						// LTS NEW LOCALWAR
{
	int Nation=GetInt();
	
	if (!g_pLocalWarfield)
	{
		MyLog(0," Scrip Call Error!! CheckAllSealStoneBroked()");
		sYES=0;
	}
	sYES=g_pLocalWarfield->CheckAllSealStonebroked(GetNationIndex(Nation));
}

void sSetFame()										// LTS NEW LOCALWAR
{
	CHARLIST *ch = CheckServerId( EventPC );
	int sFame=GetInt();
	if( !ch ) return;
	MyLog(0,"User Fame Setting Changed By Script......");
	MyLog(0,"UserName : %s, OldFame : %d, NewFame : %d",ch->Name,ch->fame,sFame);
	ch->fame=sFame;
}

void sCheckQuestMode()								// LTS DRAGON MODIFY
{
	CHARLIST *ch = CheckServerId( EventPC );
	if( !ch ) 
	{
		sYES=0;
		return;
	}
	sYES=ch->QuestMode;
	MyLog(0,"%d User Set Quest Mode : %d",ch->GetServerID(),sYES);
}


void sResetCombatSkill()	//89 // 021010	kyo
{
	g_pBattleManager->RecvCombatReset( EventPC );
}

void sSpellMapMove()	//90 // 021014 kyo
{
	char szSpell[128];
	char szMapName[20];
	GetString( szSpell);
	GetString( szMapName );
	int iMap_x = GetInt();
	int iMap_y = GetInt();
	
	CHARLIST *ch = CheckServerId( EventPC );
	if( !ch ) return;
	
	/* 021128 kyo ?ʼ????? ???ϸ? ?شܴ?.
	sYES =  g_QuestInMap.CheckSpell( EventPC, szSpell );
	//int port = GetMapPortByName( dungeon_name );
	if( sYES == 1)
	{//???̵??Ѵ?.
	sYES = MapMove( EventPC, szMapName, iMap_x, iMap_y );			
	}
	*/
	
	g_QuestInMap.SendSpellMapMove( EventPC, szSpell, szMapName, iMap_x, iMap_y );  // 021128 kyo
}

void sShowStateMessage()		//91 //021015 kyo
{
	GetString( String );
	int iR = GetInt();
	int iG = GetInt();
	int iB =  GetInt();
	
	CHARLIST *ch = CheckServerId( EventPC );
	if( !ch ) return;
	
	g_QuestInMap.ShowStateMsg( EventPC, String, iR, iG, iB );
}


int SearchItemMuch( const int& iItemNum, const int& cn)
{
	CHARLIST *ch = CheckServerId( cn );
	if( !ch ) return 0;
	
	int iRet = 0;
	//inv
	for( int a=0; a<3; a++ )
		for( int b=0; b<3; b++ )
			for( int c=0; c<8; c++ )
				if( ch->inv[a][b][c].item_no == iItemNum ) 
					iRet++;
				
					/*
					//equip
					for( a=0; a<8; a++ )
					if( ch->equip[a].item_no == iItemNum ) 
					iRet++;
					
					  //quick
					  for( a=0; a<6; a++ )
					  if( ch->quick[a].item_no == iItemNum ) 
					  iRet++;
				*/
				
				return iRet;
}

bool DeleteItemMuch( const int iItemNum, const int iNumMuch, const int cn)
{
	CHARLIST *ch = CheckServerId( cn );
	if( !ch ) return false;
	
	int iCheckNum = 0;
	
	//??????? ???????? ?????? ?????ź??? ???????
	if( iNumMuch > SearchItemMuch( iItemNum, cn ) ) return false;
	
	//?????? ?????? ?????? setiteminvPc ?״??
	//inv
	for( int a=0; a<3; a++ )
		for( int b=0; b<3; b++ )
			for( int c=0; c<8; c++ )
				if( ch->inv[a][b][c].item_no == iItemNum ) 
				{					
					ItemAttr target = GenerateItem( iItemNum, 0,0);
					SendItemEventLog( &target, cn, SN_NOT_USER, SILT_MAKE_BY_SCRIPT, 2 );
					ch->inv[a][b][c] = target;
					POS pos;
					SetItemPos( INV, a, b, c, &pos );
					SendServerEachItem(  &pos, &target, cn );		// 001028 YGI
					iCheckNum++;					
					if( iCheckNum >= iNumMuch )
						return true;
				}
				/*
				//equip
				for( a=0; a<8; a++ )
				if( ch->equip[a].item_no == iItemNum ) 
				{
				ItemAttr target = GenerateItem( iItemNum, 0,0);
				SendItemEventLog( &target, cn, SN_NOT_USER, SILT_MAKE_BY_SCRIPT, 2 );
				ch->equip[a] = target;
				POS pos;
				SetItemPos( EQUIP, a, &pos );
				SendServerEachItem(  &pos, &target, cn );		// 001028 YGI
				iCheckNum++;					
				if( iCheckNum >= iNumMuch )
				return true;
				}
				
				  //quick
				  for( a=0; a<6; a++ )
				  if( ch->quick[a].item_no == iItemNum ) 
				  {
				  ItemAttr target = GenerateItem( iItemNum, 0,0);
				  SendItemEventLog( &target, cn, SN_NOT_USER, SILT_MAKE_BY_SCRIPT, 2 );
				  ch->quick[a] = target;
				  POS pos;
				  SetItemPos( QUICK, a, &pos );
				  SendServerEachItem(  &pos, &target, cn );		// 001028 YGI
				  iCheckNum++;					
				  if( iCheckNum >= iNumMuch )
				  return true;
				  }
				*/
				if( iCheckNum >= iNumMuch)
					return true;
				else
					return false;
}


// 021117 YGI
void sCheckItemMulti()							//92 ???????? ???????? ?ִ°??
{
	int iItemNumber = GetInt();
	int iNumMuch	= GetInt();
	
	CHARLIST *ch = CheckServerId( EventPC );
	if( !ch ) return;
	
	const int iCurNum = SearchItemMuch( iItemNumber, EventPC );
	
	if( iNumMuch >  iCurNum)		sYES = 0;
	else if( iNumMuch <=  iCurNum)	sYES = 1; // 021121 kyo
	else if( iNumMuch <  iCurNum)	sYES = 2;
}

void sDeleteItemMulti()							//93 ???????? ???????? ?????
{
	int iItemNumber = GetInt();
	int iDelMuch	= GetInt();
	
	CHARLIST *ch = CheckServerId( EventPC );
	if( !ch ) return;
	
	if( DeleteItemMuch( iItemNumber, iDelMuch, EventPC ) == true )
		sYES = 1;
	else
		sYES = 0;
}

void sScriptTimer()								//94 // 021020 kyo //?ʷ? ?ð??? ????Ѵ?. 
{//0?̸? Ÿ?̸Ӹ? ?????
	//?? Ŭ???̾?Ʈ?? ?ϳ????? Ÿ?̸Ӱ? ?־?????.
	DWORD dwTime;
	
	int iTime = GetInt();
	char mapfile[30];
	GetString( mapfile );
	int x= GetInt();
	int y= GetInt();
	
	CHARLIST *ch = CheckServerId( EventPC );
	if( !ch ) return;
	
	if( iTime > 0 )
		dwTime = iTime* 1000;	//1000?? 1??
	else
		dwTime = 0;
	
	//Ÿ?̸??? ???? Ÿ?̸Ӹ? ?ߵ???ų???? cn?? name?? ??Ͻ?Ų??. 
	if( g_QuestInMap.StartScriptTimer( EventPC, ch->Name, dwTime , MIN_SYNCH_TIME, mapfile, x, y) )
		sYES = 1;
	else
		sYES = 0;
	
}

void sScriptCounter()				//95 // 021023 kyo 
{	//(???ڼ??? ??ȣ, ????, ????, ?÷???)
	//????: 0=ncp,1=??????,2=????. 
	//?÷???: 0=????,1=????,2=?????? ??ġ
	int iNumber = GetInt();
	int iType = GetInt();
	int iMuch = GetInt();
	int iFlag = GetInt();
	
	CHARLIST *ch = CheckServerId( EventPC );
	if( !ch ) return;
	
	if( g_QuestInMap.StartScriptCounter( EventPC, ch->Name, iNumber, iType, iMuch, iFlag ) )
		sYES = 1;
	else
		sYES = 0;
}

// 021117 YGI
void sUpRareInEquip()			//96 // 021028 kyo
{	// iii
	const int iEquipPos = GetInt();
	const int iMulti = GetInt();
	const int iMin = GetInt();
	const int iMax	= GetInt();
	const int iMutant	= GetInt();
	
	CHARLIST *ch = CheckServerId( EventPC);	
	if( !ch) return;
	
	//????
	// ???? ?Ǿ? ?ִ? ?????? ??ȣ?? ??? ?´?.
	// ?? ?????? ??ȣ?? ???ʷ? ???? ???????? ?????.
	// ?κ??丮?? ?־? ?ش?.
	
	if( WT_WEAPON > iEquipPos || iEquipPos > WT_UNIQUE2 )
	{//??ġ ????
		sYES = 0; return;
	}
	int item_no = ch->equip[iEquipPos].item_no;
	if( !item_no ) 
	{// ???????? ?? ??? ?ִ?.
		sYES = 0; return;
	}

	int grade_gab = iMax-iMin+1;
	
	if(0 >= grade_gab)
	{
		sYES = 0; return;
	}

	ItemAttr item = {0,};

	int ret = ItemMgr.MakeDefaultRareItem( &item, item_no, iMutant, rand()%grade_gab+iMin, iMulti );

	if( ret == 1 )		// ????
	{
		SendItemEventLog( &item, EventPC, SN_NOT_USER, SILT_MAKE_BY_SCRIPT, 2 );
		MoveEmptyInv( &item, ch );
	}
	else
	{
		sYES = 2; return;
	}
	
	sYES = 1;
}

void sBossTimerCheck()		//97 // 021106 kyo
{//iiiiii (npc??ȣ,x,y, ?ο?????, Ÿ?̸ӽð?, ????ð?)
	const int iNPC = GetInt();
	const int iNpc_x = GetInt();
	const int iNpc_y = GetInt();
	const int iMaxEntree = GetInt();//Entree limit 
	const int iTime = 1000*GetInt();
	const int iLimiteTime = 1000*GetInt();	// 030123 kyo
	
	const int nFlag = GetInt();	// 031028 kyo

	CHARLIST *ch = CheckServerId(EventPC);
	if( !ch) return;
	
	int nKey = iNpc_x*1000 + iNpc_y;

	//flag 0 -> ???? ?Ѹ? ????
	//flag 1 -> ???? ???? ????
	//flag 2 -> ?????濡?? ?????? ?? ?????? ?? ????
	if( nFlag == CBossTimer::TYPE_BOSS_OUT )
	{
		g_QuestInMap.DecreaseBossTimerUser( nKey, EventPC );
		g_QuestInMap.CheckBossTimerBoss( nKey);
		sYES= -1;
		return;
	}

	if( g_QuestInMap.GetBossTimerEntree(nKey) > iMaxEntree)
	{
		sYES = 0;
		return;
	}

	if( g_QuestInMap.GetCurrentBossTimer(nKey) <= (iTime - iLimiteTime) )	//Ÿ?̸? ?????? 0, ?????? Ÿ?̸ӽð?(dwtime)
	{
		sYES = ( g_QuestInMap.StartBossTimer( EventPC, iNPC, iNpc_x, iNpc_y, iTime, iMaxEntree, nFlag) )/1000; //031028 kyo
	}
	else //????ð??ȿ? ?ִ?.
	{
		sYES = 1;
	}
}

void sIsGetMagic()
{// 030415 kyo 
	const int nMagic = GetInt();
	CHARLIST *ch = CheckServerId(EventPC);
	if( !ch) return;
	switch (ch->Spell)
	{
    case WIZARD_SPELL:
		{
			if (ch->Ws[nMagic])
			{
				sYES = 1;
			}
			break;
		}
    case PRIEST_SPELL:
		{
			if (ch->Ws[nMagic])
			{
				sYES = 1;
			}
			break;
		}
	default:
		{
			sYES = 0;	
			break;
		}
	}
}

void sDualClassCheck()
{ //100 ???? ?ش? ???Ŭ?????ΰ? ?????? 1 Ʋ???? 0
	const int n1Class = GetInt();
	const int n2Class = GetInt();
	CHARLIST *ch = CheckServerId(EventPC);
	if( !ch) return;
	sYES = 0; // 030513 kyo
	if(ch->Class == n1Class || ch->Class >= 5)
	{
		if(ch->GetDualClass()== n2Class)
		{
			sYES =1;
		}
	}
	else
		sYES =0;
}

void sUpDualClassItem()
{ //101 ???Ŭ???????????? ?Ѵܰ?÷??ش?.
	
	const int nFlag = GetInt();
	CHARLIST *ch = CheckServerId(EventPC);
	if( !ch) return;

	if(0 == nFlag)
	{
		sYES = g_CSymbolMgr.GetSymbolGrade(ch);
	}
	else if(1==nFlag)
	{
	
		if( !g_CSymbolMgr.DoHaveSymbol(ch) )
		{
			g_CSymbolMgr.MakeSymbolItem(ch);
			sYES=0;
		}
		else
		{
			int nDualStep = g_CSymbolMgr.GetSymbolGrade(ch);
			sYES = nDualStep;
			g_CSymbolMgr.UpGradeSymbol( ch, ++nDualStep);
		}
	}

}

void sShowLearnDualMagic()
{// 102 ??󸶹??? ???? ?ִ? â?? ????.
	::SendCallDualMagicLearnMenu(EventPC);
}

void sLogDualQuest()
{// 103 iisi ???????Ʈ?? ?α׸? ?????.
	const int nQnum = GetInt();	//????Ʈ ??ȣ
	const int nQCount = GetInt();	//????Ƚ??.
	char szMsg[128];
	GetString( szMsg);	//?޽??? ????
	const int nStart = GetInt();	//???????? ?ƴ???

	CHARLIST *ch = CheckServerId( EventPC);
	if( !ch ) return;
	//< CSD-TW-030618
	//::LogInFile("./changelog/DualQuestLog.txt", "chName=%s, QuestNum=%d, Qcount=%d, Msg=%s, Start=%d", ch->Name, nQnum, nQCount, szMsg, nStart);
	SaveChangeClassQuest(ch, szMsg, nQnum, nQCount, nStart);
}	//> CSD-TW-030618

void sLogForScript()
{// 104 is ??ũ??Ʈ?? ?????? ?α׸? ?????.  // 030826 kyo
	const int nQnum = GetInt();
	char szMsg[128];
	GetString(szMsg);

	if( 0>nQnum|| _SCRIPT_VARABLE_POINT_<=nQnum ) 
	{
		return;
	}
	CHARLIST *ch = CheckServerId( EventPC);
	if( !ch ) return;

	SaveScriptLog(ch, nQnum, var[EventPC][nQnum], szMsg);//kyo
}


//<! BBD 040308	?̺?Ʈ ?????? ?????? ???ִ? ?Լ?
void sEventItemInsert()
{
	GetString( String );
	
	CHARLIST *ch = CheckServerId( EventPC );

	if(connections[ EventPC].dwAgentConnectionIndex && connections[ EventPC].state >= CONNECT_JOIN)
	{
		//SendBBS2??? ?̺?Ʈ ?޼??? ?߻? ?Լ??? ?????Ѵ?.
		SendBBS2( String, (short)EventPC );
		
		if(!ch->IsEventRecv())
		{
			// ?κ??? ??ĭ ????
			int blankcount = 0;
			for(int a=0; a<3; a++)
			{
				for(int b=0; b<3; b++)
				{
					for(int c=0; c<8; c++)
					{
						if( !ch->inv[a][b][c].item_no )
						{
							blankcount++;
						}
					}
				}
			}
			
			if(blankcount < 10)	// 10ĭ ?̻??ΰ??
			{
				::OutMessage(ch,2,13);//?ӽ?,,?κ??丮 ?????? ???ġ ?ʽ??ϴ?.
				return;
			}


			t_packet	p;
			p.h.header.type = CMD_EVENTRAREITEM_REQ;
			p.h.header.size = sizeof(t_EVENT_RARE_ITEM_LIST);
			
			p.u.Event_Item_List.nCn = EventPC;
			strcpy(p.u.Event_Item_List.szChrName, ch->Name);
			strcpy(p.u.Event_Item_List.szLoginID, connections[ EventPC].id);
			
			QueuePacket(connections,DB_DEMON,&p,1);

			ch->SetEventRecv(true);
		}
		
	}	
}
//> BBD 040308	?̺?Ʈ ?????? ?????? ???ִ? ?Լ?

//<! kjy 040406  ĳ???? ?ɷ?ġ ???????? ?Լ?
void sGetCharacterStatus()
{
	int type = GetInt();
	int type2 = GetInt();
	CHARLIST *ch = CheckServerId( EventPC );
	if ( !ch ) return;

	sYES = 0;
	switch( type )
	{
		case STR:	sYES = 	ch->Str;		break;
		case CON:	sYES =	ch->Con;		break;
		case DEX:	sYES =	ch->Dex;		break;
		case WIS:	sYES =	ch->Wis;		break;
		case INT_:	sYES =	ch->Int;		break;
		case MOVP:	sYES =	ch->MoveP;		break;
		case CHA:	sYES =	ch->Char;		break;
		case ENDU:	sYES =	ch->Endu;		break;
		case MOR:	sYES =	ch->Moral;		break;
		case LUCK:	sYES =	ch->Luck;		break;
		case WSPS:	sYES =	ch->wsps;		break;
	}

	if ( sYES < type2 )
	{
		sYES = 0;
	}
	else if ( sYES >= type2 )
	{
		sYES = 1;
	}
} 
//> kjy 040406  ĳ???? ?ɷ?ġ ???????? ?Լ?


//<! kjy 040406  ???? ?????? ???? Ȯ?? ?Լ?
void sSearchItemMuchForRare()
{
	int item_number = GetInt();
	int rare_type	= GetInt();
	int grade		= GetInt();
	int tt			= GetInt();

	CHARLIST *ch = CheckServerId( EventPC );
	if( !ch ) return;

	int iRet = 0;
	
	// ???? ???? ??? ?????? ?˻?..
	for( int a=0; a<3; a++ )
		for( int b=0; b<3; b++ )
			for( int c=0; c<8; c++ )
			{
				ItemAttr *pItem = &ch->inv[a][b][c];
				
				if( pItem->item_no == item_number ) 
				{
					RareMain *pRare = (RareMain *)&pItem->attr[IATTR_RARE_MAIN];
	
					if ( pRare->soksung1 == rare_type && pRare->grade == grade )
					{
						iRet++;
					}
				}
			}

	if ( iRet < tt )		sYES = 0;
	else if ( iRet >= tt )	sYES = 1;
}
//> kjy 040406  ???? ?????? ???? Ȯ?? ?Լ?

void (*SFunction[_SCRIPT_FUNCTION_POINT_])(void) =
{	
	sDialOnlyPC,			//  0
	sDialInAreaPC,
	sDialEveryone,
	sAddItem,				//  4 //Ư?? ??ġ?? ?????? ???? ???? (x??ǥ * 32, y??ǥ *32)
	sAddNpc,				//  5 //Ư?? ??ġ?? NPC ???? ????	(x??ǥ , y??ǥ)
	sMapName,
	sMapMove,				//  7 
	sIsLevel,
	sInitMonsterGeneratedPosition,
	sCallSmallMenu, 
	sWeatherOn,				// 10 // ?????? ???? BroadCast?Ѵ?. 
	sWeatherOff,			// 11 // ?????? ???? BroadCast???? ?ʴ´?.  
	sRecallScriptStart,		// 12
	sRecallScriptEnd,		// 13
	sMaxMonsterGeneration,	// 14
	sLoadItemValueOneMaul,  // 15 //031116 lsw ???ŵ? ?Լ?.
	srandom,				// 16 // 0131 YGI
	sEventMsg,				// 17 //?̺?Ʈ ?޼????? ?ѷ??ش?.
	sIsItemHavePC,			// 18 // 0720 LHS Ư?? ???????? ?????? ?ִ??? Ȯ???Ѵ?. ?????? ???ؼ?
	sIsClassPC,				// 19 // 0721 LHS Ư?? Ŭ?????ΰ?? i
	sSetQuestNumStep,		// 20 // 0721 LHS ????Ʈ ??ȣ?? ?????Ѵ?. ii
	sCmpQuestNumStep,		// 21 // 0722 LHS ???? ????Ʈ ??ȣ???? ???? ?ܰ踦 Ȯ???Ѵ?. 0 ?????? 1 ?????? 2 ?????? ii
	sQuestComplete,			// 22 // 0722 LHS ????Ʈ ?Ϸ?? ????????~~ i
	sQuestTeleport,			// 23 // 0726 LHS ????Ʈ?ڷ???Ʈ iii
	sIsJobPC,				// 24 // 0728 LHS ?ش? ?????ΰ?? i	
	sIsInvHavePC,			// 25 // 0728 LHS ???κ???Ţ?? ?ش? ???????? ?ִ°?? i
	sSetItemInvPC,			// 26 // 0729 LHS ?κ??丮â?? ?ش? ???????? ?ٸ? ?????????? ?????Ѵ?.(?ٲ????,?ٲܾ???,???۵?) iii
	sIsSkillHavePC,			// 27 // 0729 LHS ??ų?? ????°?? i
	sSetSkillPC,			// 28 // 0729 LHS ??ų?? ???? ?Ѵ?. ?ȹ??ŷ? ?Ѵ?. ii(??ų,???/?ȹ??)	
	sQuestSound,			// 29 // 0809 LHS ????Ʈ?? ???? i
	sAddFightMap,			// 30 // 0905 YGI ?????ʿ??? ?? ??? i
	sIsPlayArenaGame,		// 31 // CSD-030517 : ?Ʒ??? ??? ?????? ???? ?˻?
	sIsFullFightMapTeam,	// 32 // 0907 YGI ?????ʿ??? ???? ??? ????? ???????? ?ƴ??? i( ?? ??ȣ )
	sDeleteFightMap,		// 33 // 0915 YGI ?????ʿ??? ?? ???? ?ϱ? 
	sCheckNation,
	sReturnSealStoneBreakable,	// 35 // 001026 KHS sYES == 1 ?̵?????, sYES == 0 ?̵??Ұ?.
	// Added by chan78
	sSendMessageBox,			// 36 // 001124 YGI  // ??ǳ???? ?߰? ?Ѵ?.		// 001212 YGI
	sSendSound,					// 37 // 001205 YGI		// Ư?? ???? ??ȣ?? ???带 ????ϰ? ?Ѵ?. 001212 YGI
	sGenMonSpec,				// 38 // 010129 KHS
	sGetGender,					// 39 // 010205 YGI				// 010210 YGI
	
	// 010303 YGI
	sSaveItemAtDb,				// 40 // Ư?? ???????? ??? ?????Ѵ?.
	sCheckMoney,				// 41 // ???? ?󸶳? ?ִ??? ?˾ƺ???.
	sSubtractMoney,				// 42 // pc???׼? ???? ??????.
	sGetGuild,					// 43 // ??? ?ڵ带 ?????´?. sYES ?? ????ȴ?.		// 010511 YGI
	sSendEffect,				// 44	// effect ?????ֱ? 010522 YGI
	
	sCallMenu,					// 45	???? ?????ε? ??ȣ?? ?޴??? ȣ???Ѵ?.		// 010528 YGI
	sGetNationByMap,			// 46	Ư?? ???? ?????˾? ????
	sCalcFriendShip,			// 47	Ư?? ?????? ?????ġ ?????ֱ?
	sCompFriendShip,			// 48	?????ġ ???ϱ?
	sCompLadderScore,			// 49	???? ???? ??					// 010531			YGI
	sCheckLoadingPoint,			// 50	?δ??? ???ݰ????? Ȯ??			// 010904 LTS
	sCheckDefence,				// 51	????? ?????ڸ? ?̵????? ?? ??Ż..		// 010904 LTS
	sFameDown,					// 52   ĳ?????? ?????? ?????			// 010915 LTS
	sFameUp,					// 53	ĳ?????? ?????? ???δ?
	sFameCheck,					// 54	ĳ?????? ?????? ?־??? ???̻????? ?Ǻ??Ǵ?
	sCanGoBonus,				// 55	// ?????? ???ʽ? ?????? ???? ?ִ? ?Ⱓ???? Ȯ???Ѵ?.
	// CSB AddCode
	sGetLevel,					// 56
	sGetClass,					// 57
	sGetAbil,					// 58
	sCheckParty,				// 59	
	sGetNat,					// 60
	sGetSpell,					// 61
	sCheckPartyCount,			// 62
	sCheckLastMagic,			// 63
	sSetTimer,					// 64
	sGetTactic,					// 65
	sCheckTacticPoint,			// 66
	sIsDual,					// 67
	sStopTimer,					// 68
	sCallCustomSmallMenu,		// 69
	
	sSetRareItemInvPc,			// 70
	sLocalWarMode,				// 71		// LTS LOCALWAR
	sFameCheck2,				// 72
	sFameUp2,					// 73
	sFameDown2,					// 74
	sLocalWarPointUp,			// 75
	sLocalWarPointDown,			// 76
	// CSB AddCode End
	// ?߰??? ?Ʒ? ?ƽ? ???? ?÷??ش?.
	sCheckEventLocalWarStatus,	// 77		// 020115 LTS
	sCheckEventLocalWarStep,	// 78		// 020115 LTS
	sCheckEventJoin,			// 79		// 020115 LTS
	sClearEventJoin,			// 80		// 020115 LTS
	
	sSetOXSelect,				// 81		// YGI
	sGotoGuildHouse,			// 82	
	sIsMyGuildHouse,			// 83		// i ( ?̺?Ʈ ??ȣ )
	sCheckAllSealStoneBroked,	// 84		// LTS NEW LOCALWAR
	sSetFame,					// 85		// LTS NEW LOCALWAR
	sCheckQuestMode,			// 86		// LTS DRAGON MODIFY
	// 020620 YGI
	sMoveDungeon,				// 87		// sii
	sDualCheck,					// 88		// i		// 020707 YGI
	sResetCombatSkill,    // 89    //   // 021008 KYO
	sSpellMapMove,				//90		// ssii (?ֹ?, ????,????x,????y) // 021014 kyo ?ֹ??? ?ܸ? ?̵??Ѵ?. 
	sShowStateMessage,			//91		// siii	(?޽???, R,G,B )	//???? ?´? ???ڸ? ?????ش?.
	sCheckItemMulti,			//92 		// ii   ?ټ??? ?????? Ȯ?? (?????۹?ȣ, ????)  yes?? ???? =0, ???? =1		
	sDeleteItemMulti,			//93		// ii	?ټ??? ?????? ?輼 (?????۹?ȣ, ????)  yes?? ???? =0, ???? =1
	sScriptTimer,				//94		// isii	(Ÿ?̸? ???? ?ð?, ???̸?,??x,??y) //???? ?ð?: 0=????, 0!=?ð?
	sScriptCounter,				//95		// iiii (???ڼ??? ??ȣ, ????, ????, ?÷???) ????: 0=ncp,1=??????,2=?????ص?. ?÷???: 0=????,1=????,2=?????? ??ġ?
	sUpRareInEquip,				//96		// iii	?????븻???????? ???? ???????? ?ٲ۴?.: ?Ӽ??? ?ø? ( ??????ġ, ???, ?Ӽ?) yes?? 0=???۾???, 1=????, 2=?븻???۾ƴ?
	sBossTimerCheck,			//97		// iiiiii (npc??ȣ,x,y, ?ο?????, Ÿ?̸ӽð?, ????ð?)
	// yes 0=?ο??ʰ?, 1=????ð??ȿ? ?ִ?. ?׿?= ???? Ÿ?̸??? ?ð?
	sNationBelong,        // 98   yes 1 : ???̼??? 2:?????? 3 : error
	sIsGetMagic,				//99		// i (??????ȣ) ??????  ??????? 1, ?ƴϸ? 0 // 030415 kyo 
	sDualClassCheck,			//100		//ii ????Ŭ???? ???? Ŭ???? ????Ŭ?????? 5?̻??̸? ????Ŭ?????? ?????Ѵ?.
								//0.???? 1,???? 2. ?? ?? 3. ?????? 4.  ??????
	sUpDualClassItem,				//101		//i ?ξ???????? ???? ?縲?? ?ְ? ?ִ? ????? ?ܰ踦 ?ø???. syes???? ???? ?????? ???¸? ????
	sShowLearnDualMagic,		//102		//null ĳ???Ͱ? ???? ?մ? ????å?? ????????.
	sLogDualQuest,				//103		//iisi	qcount, ????, ????(0:????, 1:??) 
	sLogForScript,				//104		//is	????Ʈ??ȣ	msg	 //?????? ???¸? ?α??Ѵ?. 
	sEventItemInsert,			//105		// BBD 040226	?̺?Ʈ?????? ?????? ?ϴ? ?Լ?
	sGetCharacterStatus,		//106		// kjy 040406   ĳ?????? ?ɷ?ġ?? ?????ͼ? ??????  i = 0 : str, 1 : con, 2 : dex..., 10 : wsps, tt = ???? ?? (?????? ?ɷ?ġ?? tt???? ?????? syes???? 0, ũ?? 1, ?????? 2)
	sSearchItemMuchForRare		//107		// kjy 040406   Ư?? Rare ???????? ?????? ???ͼ? ???? ?? 
	//MAX_SCRIPT_FUNCTION ???? ???? ?ƽ??? Ȯ???Ҳ?. 
}; // ????? ???? ?Լ????? ??????
