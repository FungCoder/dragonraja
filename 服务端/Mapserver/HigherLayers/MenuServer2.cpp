#include "..\stdafx.h"
#include "..\LowerLayers\servertable.h"
#include "..\LowerLayers\recvmsg.h"
#include "..\LowerLayers\mylog.h"
#include "DefaultHeader.h"
#include "MenuDefine.h"
#include "ItemList.h"
#include "CItem.h"
#include "Scrp_exe.h"
#include "SealStone.h"
#include "SealStonegameserver.h"
#include "op_magic.h"
#include "map.h"
#include "teambattle.h"
#include "LTSSupport.h"
#include "dungeon.h"
#include "eventmgr.h" 
#include "LogManager.h"
#include "ArenaManager.h"
#include "UserManager.h"

void SendSaveEventItem( CHARLIST *ch, int item_no, int count )
{
	t_packet packet;
	packet.h.header.type = CMD_ADD_EVENT_ITEM;
	packet.u.kein.save_event_item.item_no = item_no;
	packet.u.kein.save_event_item.count = count;
	strcpy( packet.u.kein.save_event_item.name, ch->Name );
	packet.h.header.size = sizeof( k_save_event_item ) - 20 + strlen( ch->Name )+1 ;
	QueuePacket( connections, DB_DEMON, &packet, 1 );
}

void RecvOXSelect( t_packet *p, short int cn )
{
	CHARLIST *ch = CheckServerId( cn );
	if( !ch ) return;

	SetOXSelect( ch,p->u.kein.default_char ); 
	return;
}

void SetOXSelect( CHARLIST *ch, int type )
{
	ch->ox_select = type;
}

void RecvOXAnser( t_packet *p, short int cn )
{	//< CSD-CN-031213
	const int type = p->u.kein.default_char;

	CUserManager::HASH_USER mpUser = g_pUserManager->GetUserSet();

	for (CUserManager::ITOR_USER i = mpUser.begin(); i != mpUser.end(); ++i)
	{
		const int idUser = i->second;

		if (idUser == cn)
		{
			continue;
		}

		CHARLIST* pUser = CheckServerId(idUser);
		
		if (pUser == NULL)
		{
			continue;
		}

		if (pUser->IsCounselor() || pUser->IsReporter())
		{
			continue;
		}

		if (pUser->ox_select == type) 
		{
			continue;
		}

		MapMove(idUser, "G_BATTLE", 140, 100);	// 030929 kyo
	}
}	//> CSD-CN-031213

// �� �ʿ� �ִ� ��� ������ ������ ������ ���̵� ��Ų��.
// type == 1 �̸� ���ڳ�, ��ڴ� �����.
int UserAllMapMove( int type, char *map_name, int x, int y )
{	//< CSD-CN-031213
	if (strcmp(map_name, MapName) == 0)
	{
		return 0;
	}

	if (GetMapMoveType(map_name) == MMT_FAIL)
	{	// ���� ���̴�.
		return 0;
	}

	CUserManager::HASH_USER mpUser = g_pUserManager->GetUserSet();

	for (CUserManager::ITOR_USER i = mpUser.begin(); i != mpUser.end(); ++i)
	{
		const int idUser = i->second;
		CHARLIST* pUser = CheckServerId(idUser);
		
		if (pUser != NULL)
		{
			if (type == 1)
			{
				if (pUser->IsCounselor() || pUser->IsReporter()) 
				{
					continue;
				}
			}

			MapMove(idUser, map_name, x, y);
		}
	}

	return 1;
}	//> CSD-CN-031213

void RecvCheckSkill( short int cn )
{
	CHARLIST *ch= CheckServerId( cn );
	if( !ch ) return;

	// ���� ����� ������� ���� �ش�.
	if( ch->Skill[46] )
	{
		ch->IncBankMoney(50000, BMCT_SKILL_RESET); // CSD-030723
		ch->Skill[46] = 0;
		SendPutMenuString( KM_BBS, 121, cn ); // �����ϱ� ��ų�� �����߽��ϴ�.
	}	
}


void DeleteMenuData()
{

	// ����� ������Ű�� ��..

}

void RecvPutGuildItem( t_packet *p, short int cn )
{	//< CSD-030324
	CHARLIST *ch = CheckServerId( cn );
	if( !ch ) return;

	POS pos = p->u.kein.default_pos;
	ItemAttr *pItem = GetItemByPOS( ch, pos );
	if( !pItem ) return;
	if( !pItem->item_no ) return;
	if( !ch->GetGuildCode() ) return;		// ��尡 ����.
	//if( ch->name_status.guild_master ) return;		// ��峻 ��� ������ �Ϸ��� ���⼭

	t_packet packet;
	packet.h.header.type = CMD_PUT_GUILD_ITEM;
		packet.u.kein.put_guild_item.item = *pItem;
		packet.u.kein.put_guild_item.guild_code = ch->GetGuildCode();
		strcpy( packet.u.kein.put_guild_item.name, ch->Name );
	packet.h.header.size = sizeof( k_put_guild_item );
	QueuePacket( connections, DB_DEMON, &packet, 1 );	// DB�� �����Ѵ�.

	//021030 YGI
	SendItemEventLog( pItem, cn, SN_NOT_USER, SILT_PUT_GUILD_BOX, 1 ); // 021018 YGI

	// �������� �����
	memset( pItem, 0, sizeof( ItemAttr ) );
	// ������ ����� Ŭ���̾�Ʈ�� �����ش�.
	SendPutMenuString( KM_OK, 132, cn );		// �����Ǿ����ϴ�.
}	//> CSD-030324

void RecvGetGuildItemClient( t_packet *p, short int cn )
{	//< CSD-030324
	CHARLIST *ch = CheckServerId( cn );
	if( !ch ) return;

//	if( !ch->GetGuildCode() ) return;
//	int degree = ch->name_status.guild_master;
//	if( !degree || degree > 2 ) return;		// 1�� 2�� ��� ����
	if( !CheckGuildPower( GDP_GUILD_BOX, ch ) ) return;

	DWORD index = p->u.kein.default_dword;
	t_packet packet;
	packet.h.header.type = CMD_GET_GUILD_ITEM_MAP;

		packet.u.kein.get_guild_item_map.server_id = cn;
		packet.u.kein.get_guild_item_map.guild_code = ch->GetGuildCode();
		packet.u.kein.get_guild_item_map.index = index;

	packet.h.header.size = sizeof( k_get_guild_item_map );
	QueuePacket( connections, DB_DEMON, &packet, 1 );	// DB���� �ذ�
}	//> CSD-030324

// ��� ���󿡼� �������� ���� �Դ�.
void RecvGetGuildItemMap( t_packet *p )
{
	int server_id = p->u.kein.get_guild_item_db.server_id;
	ItemAttr *pItem = &p->u.kein.get_guild_item_db.item;

	CHARLIST *ch = CheckServerId( server_id );
	if( !ch ) return;

	//021030 YGI
	SendItemEventLog( pItem, server_id, SN_NOT_USER, SILT_GET_GUILD_BOX, 3 ); // 021018 YGI
	MoveEmptyInv( pItem, ch );
}

void RecvChangeGuildMaster(short int cn)
{	//< CSD-030326
	CHARLIST* ch = CheckServerId(cn);
	if (!ch)  return;

	int guild_code = ch->GetGuildCode();
	const int guild_master = ch->name_status.guild_master;
	
	if (guild_code <= 0 || guild_master != 1) 
	{
		return;		// ��� �����Ͱ� �ƴϴ�.
	}

	SendPacketDefault(CMD_CHANGE_GUILD_MASTER, &guild_code, sizeof(short int), DB_DEMON);
	ch->name_status.guild_master = 0;		// ��� ������
	SendChangeGuildDegree(cn, guild_code, 0, cn);
	SendPutMenuString(KM_OK, 178, cn);
}	//> CSD-030326

// ���� ������ ����
void SendChangeGuildDegree( int server_id, int guild_code, int guild_degree, short int cn )
{
	t_packet packet;

	packet.h.header.type = CMD_CHANGE_GUILD_MASTER;
		packet.u.kein.change_guild_degree.server_id = server_id;
		packet.u.kein.change_guild_degree.guild_code = guild_code;
		packet.u.kein.change_guild_degree.degree = guild_degree;
	packet.h.header.size = sizeof( k_change_guild_degree );
	QueuePacket( connections, cn, &packet, 1 );
	CastMe2Other( cn, &packet );
}

// �渶�� �ٲ���.
void RecvChangeGuildMasterAllMap( t_packet *p )
{	//< CSD-030324
	char *name = p->u.kein.default_name;
	int cn = ExistHe( name );
	if( cn <= 0 ) return;

	CHARLIST *ch = CheckServerId( cn );
	if( !ch ) return;

	ch->name_status.guild_master = 1;
	SendChangeGuildDegree( cn, ch->GetGuildCode(), ch->name_status.guild_master, cn );
}	//> CSD-030324

int GetDegree2Param( int degree )		// degree ���� �Ķ���ͷ� �����Ѵ�.
{
	static int degree2param[] = { 4, 0, 1, 2, 3 };
	return degree2param[degree];
}

// 020620 YGI
bool CheckGuildPower( int type, CHARLIST *ch )
{	//< CSD-030324
	if( !ch ) return 0;
	if( !ch->GetGuildCode() ) return 0;
	if( type < 0 || type > GDP_MAX ) return 0;
	
	static int power_table[][5] = 
	{
		{ 1, 1, 0, 0, 0 },		// ��� ����				// GDP_JOINING
		{ 1, 1, 0, 0, 0 },		// ��� ��ü �޽���			// GDP_BBS
		{ 1, 0, 0, 0, 0 },		// ���� ��å����			// GDP_CHANGE_DEGREE  // 020620 YGI
		{ 1, 1, 0, 0, 0 },		// ��� â�� �̿�			// GDP_GUILD_BOX
		{ 1, 1, 0, 0, 0 },		// ���� ��� Ż�� ��Ű��	// GDP_EXIT_GUILD_HIM
		{ 1, 0, 0, 0, 0 },		// ��� ��ũ ����			// GDP_CHANGE_MARK
		{ 1, 0, 0, 0, 0 },		// ��� ��å �� ����		// GDP_CHANGE_DEGREE_INFO
		{ 1, 0, 0, 0, 0 },		// ��� ������ ����			// GDP_GUILD_CHANGE_MASTER
		{ 1, 0, 0, 0, 0 },		// ��� ����				// GDP_GUILD_BREAK
		{ 1, 0, 0, 0, 0 },		// �α渶 ���				// GDP_REGIST_SUB_MASTER
		{ 0, 1, 1, 1, 1 },		// ������ ��� Ż��			// GDP_EXIT_GUILD_SELF
		{ 1, 0, 0, 0, 0 },		// �������� ����			// GDP_BUY_GUILD_HOUSE
	};

	int para = GetDegree2Param( ch->name_status.guild_master );
	return ((power_table[type][para])?true:false);
}	//> CSD-030324

void RecvGetNationItem( short int cn )
{
	CHARLIST *ch = CheckServerId( cn );
	if( !ch ) return;

	t_packet packet;
	packet.h.header.type = CMD_GET_NATION_ITEM;
		packet.u.kein.nation_item.item_no = g_nation_item.item_no;
		if( g_nation_item.money > 0 )
		{
			packet.u.kein.nation_item.give_type = 0;		// money_type
			packet.u.kein.nation_item.give_much = g_nation_item.money;
		}
		else
		{
			packet.u.kein.nation_item.give_type = 1;		// money_type
			packet.u.kein.nation_item.give_much = g_nation_item.fame;
		}
	packet.h.header.size = sizeof( k_nation_item );
	QueuePacket( connections, cn, &packet, 1 );
}

extern void SaveChangeNationItem(CHARLIST* pTarget, int type, ItemAttr* item); // CSD-040224

void RecvBuyNationItem( short int cn )
{
	CHARLIST *ch = CheckServerId( cn );
	if( !ch ) return;

	int ret = 0;
	if( g_nation_item.money > 0 )
	{
		if( ch->Money< g_nation_item.money )
		{
			// ���� �����ϴ�. 
			ret = 2;
		}
		else
		{
			// 021209 YGI
			if( SubtractMoney( g_nation_item.money, ch ) == 1 )
			{
				ItemAttr item = GenerateItem( g_nation_item.item_no);
				SendItemEventLog( &item, cn, SN_NOT_USER, SILT_BUY_NATION_ITEM_MONEY, 100 );
				SaveChangeNationItem(ch, 0, &item);	// CSD-040224
				MoveEmptyInv( &item, ch );
				SendNationMoney( NM_ADD, ch->name_status.nation, g_nation_item.money); // ������ ��ȭ��Ų��.
				ret = 1;
			}
			else
			{
				ret = 2;
			}
		}
	}
	else
	{
		if( ch->fame - g_nation_item.fame < 1000 )
		{
			// ����ġ�� �����ϴ�.
			ret = 3;
		}
		else
		{
			// 021209 YGI
			ret = 1;
			SaveChangeFame(ch,ch->fame,ch->fame-g_nation_item.fame,LF_ITEMBUY);
			ch->fame -= g_nation_item.fame;
			ItemAttr item = GenerateItem( g_nation_item.item_no);
			SendItemEventLog( &item, cn, SN_NOT_USER, SILT_BUY_NATION_ITEM_FAME, 100 );
			SaveChangeNationItem(ch, 1, &item);	// CSD-040224
			MoveEmptyInv( &item, ch );
		}
	}

	t_packet packet;
	packet.h.header.type = CMD_BUY_NATION_ITEM_OK;
		packet.u.kein.default_char = ret;
	packet.h.header.size = sizeof( char );
	QueuePacket( connections, cn, &packet, 1 );	
}

void RecvCheckColossusMap(t_packet *p, short int cn)
{	//< CSD-TW-030701
	CHARLIST* pTarget = CheckServerId(cn);
		
	if (pTarget == NULL)
	{
		return;
	}

	CArenaInfo* pArenaInfo = g_pArenaManager->GetArenaInfo(g_MapPort);

	if (pArenaInfo == NULL)
	{
		return;
	}

	CArenaManager::HASH_GAME mpArena = g_pArenaManager->GetGameSet();

	for (CArenaManager::ITOR_GAME i = mpArena.begin(); i != mpArena.end(); ++i)
	{
		CBaseArena* pGame = i->second;
		pGame->SendGameInfo(pTarget, g_MapPort);
	}
}	//> CSD-TW-030701

// 021107 YGI
void SendPacket2Map( t_packet *p, int port )
{
	if( port != ST_SEND_MAP_ALL	)
	{
		if( port == GetOwnPort() )	// ���� ������ ���� ���
		{
			HandleCommand( CN_FROM_OTHER_MAPSERVER_MSG, p );
		}
		else
		{
			g_pServerTable->SendRajaPacketToOtherMapServer( port, (char *)p, p->h.header.size+sizeof(t_header) );
		}
	}
	else // all
		g_pServerTable->BroadCastToEveryServer((char *)p, p->h.header.size+sizeof(t_header),SERVER_TYPE_MAP);		
}

void RecvInviteColossus( t_packet *p, short int cn )
{	//< CSD-030509
	CHARLIST* ch = CheckServerId(cn);
	
	if (ch == NULL)
	{
		return;
	}

	CBaseArena* pGame = ch->GetJoinArenaGame();

	if (pGame == NULL)
	{
		return;
	}
	// ���常 ����� �� �ִ� ���ɾ�
	if (pGame->GetLeader() != ch) 
	{
		return;		
	}
	
	t_packet packet;
	packet.h.header.type = CMD_INVITE_COLOSSUS_REPLY;
	packet.h.header.size = sizeof(k_invite_colossus_map) - 20 + strlen(ch->Name) + 1;
	packet.u.kein.invite_colossus_map.port = g_MapPort;
	strcpy(packet.u.kein.invite_colossus_map.name, ch->Name);
	SendPacket2Map(&packet, ST_SEND_MAP_ALL);
	RecvInviteColossusReply(&packet);
}	//> CSD-030509

void RecvInviteColossusReply( t_packet *p )
{	//< CSD-CN-031213
	g_pUserManager->SendPacket(p);
}	//> CSD-CN-031213

void RecvJoinColossusMap( t_packet *p, short int cn )
{	//< CSD-030519
	CHARLIST* ch = CheckServerId(cn);
	
	if (ch == NULL)
	{
		return;
	}

	const int nPort = p->u.kein.default_short_int;

	if (nPort == g_MapPort)
	{
		return;
	}

	CArenaInfo* pArena = g_pArenaManager->GetArenaInfo(nPort);

	if (pArena == NULL)
	{
		return;
	}
	
	t_MapInfo* pMapInfo = GetMapInfoByMapPort(nPort);

	if (pMapInfo == NULL)
	{
		return;
	}

	int nX = 0, nY = 0;
	pArena->GetWaitPosition(nX, nY);
	MapMove(cn, pMapInfo->mapfile, nX, nY);
}	//> CSD-030519

void RecvAttackColossusItem( t_packet *p, short int cn )
{	//< CSD-030509
	CHARLIST* ch = CheckServerId(cn);
	
	if (ch == NULL) 
	{
		return;
	}

	CBaseArena* pArena = ch->GetJoinArenaGame();

	if (pArena == NULL)
	{
		return;
	}

	if (!pArena->IsPlayGame())
	{
		return;
	}

	CArenaTeam* pTeam = ch->GetJoinArenaTeam();

	if (pTeam == NULL)
	{
		return;
	}

	const int nItem = p->u.kein.default_short_int;
	
	if (nItem < 0 || nItem >= MAX_ITEM_LIST) 
	{
		return;
	}

	SetStoneByTeam(&ItemList[nItem], pTeam->GetTeamNumber());

	t_packet packet;
	packet.h.header.type = CMD_ATTACK_COLOSSUS_STONE_RESULT;
	packet.h.header.size = sizeof(k_attack_colossus_stone_result);
	packet.u.kein.attack_colossus_stone_result.server_id = cn;
	packet.u.kein.attack_colossus_stone_result.item_id = nItem;
	packet.u.kein.attack_colossus_stone_result.attr = ItemList[nItem].attr[IATTR_ATTR];
	QueuePacket(connections, cn, &packet, 1);
	CastMe2Other(cn, &packet);
	// ���� ���� ���� ī��Ʈ �ؼ� �����ش�.
	int team_stone[4] = {0,};
	g_ColossusStone.GetTeamCount( team_stone );
	
	int i;
	for (i = 0; i < 2; ++i)
	{	//< CSD-030520
		CBaseArena* pGame = g_pArenaManager->GetGameInfo(i);

		if (pGame == NULL)
		{
			continue;
		}

		pGame->SendGameMessage(i, 2, team_stone[i]);
	}	//> CSD-030520
	// ��ü���� ���� ������ ���� �ߴ��� �˷��ش�.
	CBaseArena* pGame = ch->GetJoinArenaGame();

	if (pGame != NULL)
	{	//< CSD-030520
		pGame->SendGameMessage(ch, 5, ItemList[nItem].dumno+1);
	}	//> CSD-030520

	// ���� ���� �ߴ��� �˾ƺ���.
	int time = 0;
	int got_stone_team = 0;		// ���� ������ ��
	for( i=0; i<2; i++ )
	{
		if( team_stone[i] == g_ColossusStone.GetMaxStone() )
		{
			got_stone_team = i;
			time = g_curr_time;
			break;
		}
	}
	g_ColossusStone.SetWinCondition( time, got_stone_team );
}	//> CSD-030509

void CallMenu( short int menu, short int cn )
{
	SendPacketDefault( CMD_CALL_MAIN_MENU, &menu, sizeof( short int ), cn );
}

void RecvRevivalColossus(t_packet *p, short int cn)
{	//< CSD-030509
	CHARLIST* ch = CheckServerId(cn);
	
	if (ch == NULL)
	{
		return;
	}

	CBaseArena* pGame = ch->GetJoinArenaGame();

	if (pGame == NULL)
	{
		return;
	}

	if (!pGame->IsPlayGame())
	{
		return;
	}

	CArenaTeam* pTeam = ch->GetJoinArenaTeam();

	if (pTeam == NULL)
	{
		return;
	}

	switch (pGame->GetArenaType())
	{
	case AT_STONE:
	case AT_MONSTER:
		{
			if (ch->viewtype != VIEWTYPE_GHOST_ && ch->Hp > 0) 
			{	// ������ �ƴϴ�.
				return;
			}

			int nX = 0, nY = 0;
			pTeam->GetBasePosition(nX, nY);
			MovePc(cn, nX, nY);
			SkillMgr.CharacterToAlive(ch);
			// ��ü ���� �޽���
			pGame->SendGameMessage(ch, 4, 0); // CSD-030520
			break;
		}
	}
}	//> CSD-030509

void RecvGuildHouseInfo( t_packet *p, short int cn )
{	//< CSD-030324
	CHARLIST *ch = CheckServerId( cn ); 
	if( !ch ) return;

	//if( !ch->GetGuildCode() ) return;
	int nation = ch->name_status.nation;

	t_packet packet;
	packet.h.header.type = CMD_GUILD_HOUSE_INFO;
	k_guild_house_info *t = (k_guild_house_info *)packet.u.data;
	CGuildHouseUnit *pData;
	t->count=0;
	for( int i=0; i<g_GuildHouse.m_Max; i++ )
	{
		pData = &g_GuildHouse.m_aGuildHouse[i];
		if( !pData->id ) continue;
		if( pData->nation != nation ) continue;
		t->data[t->count].id = pData->id;
		t->data[t->count].guild_code = pData->guild_code;
		t->data[t->count].blue = pData->blue;
		t->data[t->count].red = pData->red;
		t->data[t->count].lv = pData->level;

		t->data[t->count].time = 0;
		// 031009 CI YGI
		int time = 0;
		if( pData->year >= 1900 )
		{
			time += (pData->year-g_year )*365;
			time += (pData->month-g_mon-1)*30;
			time += pData->day-g_day;
			if( time < 0 ) t->data[t->count].time = 0;
			else if( time > 125 ) t->data[t->count].time = 60;
			else
			{
				t->data[t->count].time = time;
			}
		}
		
		strcpy( t->data[t->count].map_name, pData->map_name );
		t->count ++;
	}
	packet.h.header.size = sizeof( k_guild_house_info_unit )*t->count+sizeof( char );
	QueuePacket( connections, cn, &packet, 1 );
	static int check_day = g_day;
	//if( check_day != g_day )
	{
		check_day = g_day;
		CallClient( DB_DEMON, CMD_CHECK_GUILD_HOUSE );		// �������� �� ��尡 �ִ��� üũ�Ѵ�. // ���� ������ �� �ʿ� �ѷ��ش�.
	}
}	//> CSD-030324

void RecvBuyGuildHouse( t_packet *p, short int cn )
{	//< CSD-030324
	CHARLIST *ch = CheckServerId( cn );
	if( !ch ) return;

	int house_id = p->u.kein.default_char;
	int guild_code = ch->GetGuildCode();

	if( !CheckGuildPower( GDP_BUY_GUILD_HOUSE, ch ) ) return;		// �渶�� ����� �� �ִ�.

	CGuildHouseUnit	*pHouse = g_GuildHouse.GetHouseByIndex( house_id );		// ���� ��尡 �ִ�.
	if( !pHouse  || !pHouse->id ) return;
	if( pHouse->nation != ch->name_status.nation ) return;


	if( GetItemMuchByMuchItem( ch, IN_BLUE_SIGNET ) < pHouse->blue ) return;		// �ñ׳��� �����ϴ�.
	if( GetItemMuchByMuchItem( ch, IN_RED_SIGNET ) < pHouse->red ) return;		// �ñ׳��� �����ϴ�.

	k_change_guild_house_info data;
	data.guild_code = guild_code;
	data.id = pHouse->id;		// �������� �ε���
	if( pHouse->guild_code )
	{
		if( pHouse->guild_code != guild_code ) return;

		int time = 0;
		if( pHouse->year >= 1900 )
		{
			time += (pHouse->year-g_year )*365;
			time += (pHouse->month-g_mon-1)*30;
			time += pHouse->day-g_day;
		}
		if( time < 0 || time > 5 )
		{
			// �ѹ��� ���� �� �� �ִ�.
			SendPutMenuString( KM_FAIL, 174, cn );
			return;
		}
		// �̰�쿣 ��� ������ ��缭���� �����ְ� ����..
		data.day_type = 2;		// ����
	}
	else
	{
		CGuildHouseUnit	*pData = g_GuildHouse.GetHouseByGuildCode( guild_code );
		if( pData ) return;		// �̹� ������ �ִ� �������ư� �ִ�.
		// ���� ��û
		data.day_type = 1;		// �ű� ��û
	}

	// ���� // ��� ���� ��� ���� �ǹǷ� DB-demon���� ��� �Ǹ� �ȵȴ�.
	SendDeleteMuchItemQuantity( ch, IN_BLUE_SIGNET, pHouse->blue );
	SendDeleteMuchItemQuantity( ch, IN_RED_SIGNET, pHouse->red );

	SendPacketDefault( CMD_CHANGE_GUILDHOUSE_INFO, &data, sizeof( k_change_guild_house_info ), DB_DEMON );
}	//> CSD-030324

void RecvChangeGuildHouse( t_packet *p, short int cn )
{
	// �������� ���� ����
	int house_id = p->u.kein.change_guild_house_info_db2map.house_id;
	CGuildHouseUnit	*pHouse = g_GuildHouse.GetHouseByIndex( house_id );
	if( !pHouse  || !pHouse->id ) return;

	pHouse->guild_code		=	p->u.kein.change_guild_house_info_db2map.guild_code;
	pHouse->year			=	p->u.kein.change_guild_house_info_db2map.year;
	pHouse->month			=	p->u.kein.change_guild_house_info_db2map.month;
	pHouse->day				=	p->u.kein.change_guild_house_info_db2map.day;
}

void RecvCheckGuildHouse( t_packet *p, short int cn )
{
	int count = p->u.kein.check_guild_house.count;
	char *pHouseId = p->u.kein.check_guild_house.pHouseId;
	if( !count ) return;

	for( int i=0; i<count; i++ )
	{
		CGuildHouseUnit	*pHouse = g_GuildHouse.GetHouseByIndex( pHouseId[i] );
		if( !pHouse ) continue;
		pHouse->guild_code = 0 ;
	}
}

/////////////////////////////////////////////////////////////////////////////////////
// CPotionBox member fuctions
int CPotionBox::PutItem( ItemAttr *item )
{
	//if( !box_type ) return 0;
	CItem *t= ::ItemUnit( item->item_no );
	if( !t ) return 0;

	switch( t->GetItemKind())
	{
	case IK_POTION_BAG:		// BBD 040213	���ǹ鵵 ���ǹڽ��� ���� �ʵȴ�
	case IK_POTION_BOX:
	case IK_CANDLE:
	case IK_GOOD_CANDLE:
		{
			return 0;
		}break;
	}
	if( t->GetRbutton() != USE_ITEM ) {return 0;}

	if(item->attr[IATTR_RARE_MAIN])//��� �������� ���Ծ�
	{
		return 0;
	}

	if( much )
	{
		if( item_no != item->item_no ) {return 0;}
		if( 50<= much){return 0;}//020520 lsw 50���� ����
	}
	else
	{
		item_no = item->item_no ;
	}
 
	much++;
	SaveData();
	::DeleteItem(item);
	
	return 1;
}

int CPotionBox::DeleteItem()
{
	if( !much ) return 0;		// ���� ����
	much--;
	SaveData();
	return 1;
}

int CPotionBox::UseItem()
{
	if( !DeleteItem() ) return 0;		// ����.
	CItem *t = ::ItemUnit( item_no );
	if( !t ) 
	{
		return 0;
	}
	
	switch( t->GetItemKind())//���ʳ� ���� ������ ��� ���� ������
	{
	case IK_POTION_BAG:		// BBD 040213	���ǹ鵵 ���ǹڽ��� ���� �ʵȴ�, ��� ��뵵 �ʵ�
	case IK_POTION_BOX:
	case IK_CANDLE:
	case IK_GOOD_CANDLE:
		{
			item_no = 0;
			much = 0;
			return 0;
		}break;
	}
	const int add_hp = t->EatItem( hero );
	return add_hp;
}

void CPotionBox::SaveData( )		// �������� �������  �����Ѵ�.
{
	DWORD temp;
	temp = MAKELONG( item_no, much );
	hero->Sight = temp;
	hero->SendCharInfoBasic( CIB_POTION_BOX, GetData() );		// ���� ������ �˷��ش�.////020704 lsw
}

void RecvPkOnOff( t_packet *p, short int cn )
{
	CHARLIST *ch = CheckServerId( cn );
	if( !ch ) return;

	int pk_on_off= p->u.kein.default_char;
	if( ch->pk_on_off != pk_on_off )
	{
		ch->pk_on_off = pk_on_off;
		QueuePacket( connections, cn, p, 1 );
	}
}

void RecvCMD_GET_PK_ON_OFF( t_packet *p, short int cn )
{
	CHARLIST *ch = CheckServerId( cn );
	if( !ch ) return;

	SendPacketDefault( CMD_GET_PK_ON_OFF, &ch->pk_on_off, sizeof( char ), cn );
}

// 020430 YGI acer 3
//////////////////////////////////////////////////////////////////////
// CItemLog member functions
CItemLog	g_item_log;
//acer4
void CItemLog::Load( int port )
{
	int temp = 0;
	char MapServerConfigFileName[MAX_PATH];
	sprintf(MapServerConfigFileName,"%s/data/MapServerConfig.ini", GameServerDataPath);
	//if( GetOwnPort() == GetManagementMapPort( port ) )
	{
		temp = 0; temp = (int)GetPrivateProfileInt("ItemLog", "DeleteDay",0,MapServerConfigFileName);
		if( !temp ) return;
		m_nDeleteDay = temp;

		temp = 0; temp = (int)GetPrivateProfileInt("ItemLog", "SaveLevel",0,MapServerConfigFileName);
		if( temp ) m_nSaveLv = temp;

		temp = 0; temp = (int)GetPrivateProfileInt("ItemLog", "ItemLevel",0,MapServerConfigFileName);
		if( temp ) m_nItemLv = temp;

		MyLog( LOG_NORMAL, "Item Log config, DeleteDay = %d, SaveLevel = %d, ItemLevel = %d", m_nDeleteDay, m_nSaveLv, m_nItemLv );

		temp = 0; temp = (int)GetPrivateProfileInt("ItemLog", "trade",0,MapServerConfigFileName);
		if( temp ) 
		{
			m_aType2Lv[SILT_PUT_OTHER_CH] = temp;
			m_aType2Lv[SILT_GET_OTHER_CH] = temp;
		}
		temp = 0; temp = (int)GetPrivateProfileInt("ItemLog", "store",0,MapServerConfigFileName);
		if( temp ) 
		{
			m_aType2Lv[SILT_GET_STORE] = temp;
			m_aType2Lv[SILT_PUT_STORE] = temp;
		}
		temp = 0; temp = (int)GetPrivateProfileInt("ItemLog", "ground",0,MapServerConfigFileName);
		if( temp ) 
		{
			m_aType2Lv[SILT_PUT_GROUND] = temp;
			m_aType2Lv[SILT_GET_GROUND] = temp;
			m_aType2Lv[SILT_PUT_GROUND_MAX_INVEN] = temp;
			m_aType2Lv[SILT_DROP_BY_PK] = temp;
		}
		temp = 0; temp = (int)GetPrivateProfileInt("ItemLog", "duration",0,MapServerConfigFileName);
		if( temp ) 
		{
			m_aType2Lv[SILT_DELETE_BY_DURATION] = temp;
		}
		temp = 0; temp = (int)GetPrivateProfileInt("ItemLog", "stolen",0,MapServerConfigFileName);
		if( temp ) 
		{
			m_aType2Lv[SILT_GET_OTHER_CH_INVEN] = temp;
			m_aType2Lv[SILT_GET_STOLEN] = temp;
			m_aType2Lv[SILT_PUT_STOLEN] = temp;
		}
		temp = 0; temp = (int)GetPrivateProfileInt("ItemLog", "make_by_skill",0,MapServerConfigFileName);
		if( temp ) 
		{
			m_aType2Lv[SILT_MAKE_BY_SKILL] = temp;
		}
		temp = 0; temp = (int)GetPrivateProfileInt("ItemLog", "make_by_event",0,MapServerConfigFileName);
		if( temp ) 
		{
			m_aType2Lv[SILT_MAKE_BY_DB] = temp;
			m_aType2Lv[SILT_MAKE_BY_SCRIPT] = temp;
			m_aType2Lv[SILT_GET_GAMBLE_ITEM] = temp;
			m_aType2Lv[SILT_PUT_GAMBLE_ITEM] = temp;
			m_aType2Lv[SILT_DELETE_BY_SCRIPT] = temp;
		}
		temp = 0; temp = (int)GetPrivateProfileInt("ItemLog", "resource",0,MapServerConfigFileName);
		if( temp ) 
		{
			m_aType2Lv[SILT_RESOURCE_BY_SKILL] = temp;
			m_aType2Lv[SILT_NEED_ITEM] = temp;
		}
		temp = 0; temp = (int)GetPrivateProfileInt("ItemLog", "man2man",0,MapServerConfigFileName);
		if( temp ) 
		{
			m_aType2Lv[SILT_GET_MEN_TO_MEN] = temp;
			m_aType2Lv[SILT_PUT_MEN_TO_MEN] = temp;
		}
	}
	// 020620 YGI
	sprintf(MapServerConfigFileName,"%s/data/NoItemLog.txt", GameServerDataPath);
	m_NoItemList.Load( MapServerConfigFileName );
}

bool CItemLog::IsSave( ItemAttr *item, int event_type, int lv )
{
	if( lv == 100 ) return true;		// ������ ���� // 030506 YGI

	if( !item ) return false;
	if( item->attr[IATTR_RARE_MAIN] ) return true;			// ���� ����
	
	if( m_NoItemList.Search( item->item_no ) ) return false;		// �������� ���ƶ� ��Ͽ� ������ �������� �ʴ´�.
	if( item->item_no/1000 == ACCESSORY ) return true;		// ����ũ�� ����
	
	CItem *t = ::ItemUnit( item->item_no );
	if( !t ) return false;

	if( t->GetLevel() < m_nItemLv ) return false;			// ������ ������ ������ �������� �ʴ´�.
	if( !m_aType2Lv[event_type] ) return false;				// 0 �ϰ�� �������� �ʴ´�.
	if( m_nSaveLv && m_aType2Lv[event_type] > m_nSaveLv ) return false;	// �̺�Ʈ Ÿ���� ������ �������� �ʴ´�.
	
	return true;		// ������ ������ Ÿ���� ���ƾ����� �����Ѵ�.
}

// 020620 YGI
////////////////////////////////////////////////////////////////////////////
// class C2JinSearch

int C2JinSearch::Load( char *filename )
{
	Remove();

	FILE *fp = fopen( filename, "rt" );
	if( !fp ) return 0;	

	char temp[256];
	int count = 0;
	while( fgets( temp, 256, fp ) )
	{
		if( temp[0] == ' ' || temp[0] == ';' || temp[0] == '\n' ) continue;
		if( !atoi( temp ) ) continue;
		count++;
	}
	fseek( fp, 0, SEEK_SET );
	max = count;
	item = new int[max];
	count = 0;
	while( fgets( temp, 256, fp ) )
	{
		if( temp[0] == ' ' || temp[0] == ';' || temp[0] == '\n' ) continue;
		int value = atoi( temp );
		if( !value ) continue;
		item[count] = value;
		count++;
	}
	Sort();
	fclose( fp );
	return 1;
}

int C2JinSearch::Sort()
{
	if( !max ) return 0;
	for( int i=0; i<max-1; i++ )
	{
		for( int j=i+1; j<max; j++ )
		{
			if( item[i] > item[j] )
			{
				int temp = item[i];
				item[i] = item[j];
				item[j] = temp;
			}
		}
	}
	return 1;
}

int C2JinSearch::Search( int no )
{
	if( !max ) return 0;
	int low=0, high = max-1;
	int mid = 0;
	while( low <= high )
	{
		mid = (low+high)/2;
		if( item[mid] == no ) return 1;
		if( item[mid] < no ) low = mid+1;
		else high = mid-1;
	}

	return 0;
}

int RecvSaveItemLogByEvent( t_packet *p )
{	//< CSD-030806
	if( g_MapPort == GetManagementMapPort( MM_SAVE_LOG_FILE_MAP ) ) 
	{
		k_item_log *pData = &p->u.kein.item_log;
		if( g_item_log.GetSaveLv() )
		{
			//�޴µ������� �� ��������.
			//if( pData->lv > g_item_log.GetSaveLv()  ) return 0;
		}
		return SaveItemLog( pData );
	}
	return 0;
}	//> CSD-030806

void SendItemEventLog( ItemAttr *item, short int hero_cn, short int target, int event_type, int lv )
{
	if( !g_item_log.IsSave( item, event_type, lv ) ) return;		// ���� ����� �ȵǸ� ��������.. // 030506 YGI

	CHARLIST *hero = CheckServerId( hero_cn );
	if( !hero ) return;

	t_packet packet;
	k_item_log *pData = &packet.u.kein.item_log;
	packet.h.header.type = CMD_SAVE_ITEM_LOG;
	packet.h.header.size = sizeof( k_item_log );

	if( lv == 0 ) lv = 1;
	pData->item = *item;
	pData->lv = g_item_log.GetTypeLv(event_type)?g_item_log.GetTypeLv(event_type):1;	// 030506 YGI
	pData->port = GetOwnPort();	
	pData->type = event_type;
	strcpy( pData->name1, hero->Name );
	if( target > 0 )
	{
		CHARLIST *target_ch;
		if( target > 10000 )	target_ch = CheckNpcId( target-10000 );
		else					target_ch = CheckServerId( target );

		if( target_ch )	
		{
			if( target_ch->IsNpc() )
			{
				strcpy( pData->name2, "<_NPC_>");
			}
			else
			{
				memcpy( pData->name2, target_ch->Name, 20 );//020510 lsw //Ȥ�� �� Garbage Value
			}
		}
		else
		{
			strcpy( pData->name2, "<_NULL_>" );
		}
	}
	else
	{
		switch( event_type )
		{
			default :	//acer7
			case SILT_GET_STOLEN					:
			case SILT_PUT_STOLEN					:
			case SILT_GET_OTHER_CH_INVEN	:	
			case SILT_PUT_OTHER_CH				:
			case SILT_GET_OTHER_CH				:
				strcpy( pData->name2, "<_NULL_>" );
				break;
			case SILT_PUT_GROUND					:
			case SILT_GET_GROUND					:
				strcpy( pData->name2, "<_GROUND_>" );
				break;
			case SILT_PUT_BANK						:
			case SILT_GET_BANK						:
				strcpy( pData->name2, "<_BANK_>" );
				break;

			case SILT_MAKE_BY_DB					:	
			case SILT_MAKE_BY_SCRIPT			:	
			case SILT_MAKE_BY_SKILL				:
			case SILT_RESOURCE_BY_SKILL		:
				strcpy( pData->name2, "<_MAKE_>" );
				break;
			case SILT_PUT_STORE						:
			case SILT_GET_STORE						:
				strcpy( pData->name2, "<_STORE_>" );
				break;

			case SILT_GET_GAMBLE_ITEM			:
			case SILT_GET_TODAY_ITEM			:	
			case SILT_GET_GUILD_WAR				:
			case SILT_USE									:
			case SILT_DELETE_BY_DURATION	:
			case SILT_DELETE_BY_ATTR			:
			case SILT_DELETE_BY_RARE_ATTR	:
				strcpy( pData->name2, "<_SERVER_>" );
				break;
			//<! BBD 040226	���� �ŷ��� �α׶�°� �˼� �ֵ��� �߰�
			case SILT_REGISTER_AUCTION_ITEM		:	// ���� ��Ͻ�
			case SILT_CANCEL_AUCTION_SELL_ITEM	:	// ��� ��ҽ�
			case SILT_TAKE_AUCTION_RESULT_ITEM	:	// ������ ���� ã����
				strcpy( pData->name2, "<_AUCTION_>" );
				break;
			//> BBD 040226	���� �ŷ��� �α׶�°� �˼� �ֵ��� �߰�

		}
	}
	SendDirectMap( &packet, GetManagementMapPort( MM_SAVE_LOG_FILE_MAP ) );
}

int GetServerSetNum()
{
	return g_pServerTable->GetServerSetNum();
}

// GM ���, �� ���� �̵�
//acer7
void RecvDirectMapMove( t_packet *p, short int cn )
{
	if( !CheckServerId( cn ) ) return;

	int index = p->u.kein.direct_map_move.map_number;
	int x = p->u.kein.direct_map_move.x;
	int y = p->u.kein.direct_map_move.y;
	char *name = p->u.kein.direct_map_move.name;
	int target_id = cn;

	if( name[0] )
	{
		int temp = ExistHe( name );		// cn ���� �ִٸ�...
		if( temp == -1 ) 
		{
			SendPutMenuString( KM_FAIL, 181, cn );		// ���� �ʿ� ���� ĳ���� �Դϴ�.
			return;		// ����� �̸��� �� ���ƴ�.
		}
		target_id = temp;
	}
	CrackMgr.SendHackingUser( cn, USE_GM_TOOL, " Map Move " );
	GotoUser( MapInfo[index].mapfile, x, y, target_id );			// 020705 YGI
}


// acer4
void RecvSearchChar( t_packet *p, short int cn )
{
	CHARLIST *ch = CheckServerId( cn );
	if( !ch ) return;

	t_packet packet;
	char *name = p->u.kein.default_name;
	int he = ExistHe( name );
	if( he > 0 )
	{
		CHARLIST *him = CheckServerId( he );
		if( !him ) return;

		PutHimInfo( him, &packet.u.kein.char_info );
		packet.h.header.type = CMD_SEARCH_OTHER_CHAR;
		packet.h.header.size = sizeof( k_char_info );
		QueuePacket( connections, cn, &packet, 1 );
		return;
	}
	else		// �ٸ� ������ ��û�Ѵ�.
	{
		packet.h.header.type = CMD_SEARCH_OTHER_CHAR_MAP;
		packet.u.kein.char_info_map.port = GetOwnPort(); 
		packet.u.kein.char_info_map.cn = cn;
		strcpy( packet.u.kein.char_info_map.name, name );
		packet.h.header.size = sizeof( k_char_info_map );
		SendPacket2Maps( &packet );
	}
}

// acer4
int PutHimInfo( CHARLIST *him, k_char_info *him_info )
{	//< CSD-030324
	him_info->map_number = MapNumber;
	him_info->curr_tactic_kind = him->GetTacticsKind();
	him_info->curr_tactic_lv = him->GetWeaponSkill();
	memcpy( &him_info->name_status, &him->name_status, sizeof( DWORD ) );
	him_info->btLevel = him->GetLevel(); // CSD-030806
	him_info->nk = him->GetNK( him->name_status.nation );
	him_info->x = him->MoveSx;
	him_info->y = him->MoveSy;
	him_info->guild_code = him->GetGuildCode();
	return 1;
}	//> CSD-030324

void RecvSearchCharMap( t_packet *p )
{
	int server_id = p->u.kein.char_info_map.cn;
	int port = p->u.kein.char_info_map.port;
	char *name = p->u.kein.char_info_map.name;

	int he = ExistHe( name );
	if( he > 0 )
	{
		CHARLIST *him = CheckServerId( he );
		if( !him ) return;

		k_char_info data;
		PutHimInfo( him, &data );
		SendDirectClientOtherMap( CMD_SEARCH_OTHER_CHAR, server_id, port, &data, sizeof( k_char_info ) );
	}
}

//acer7
void RecvUseGmTool_SaveLog( t_packet *p, short int cn )
{
	CHARLIST *ch = CheckServerId( cn );
	if( !ch ) return;

	char *type = p->u.kein.default_msg;
	CrackMgr.SendHackingUser( cn, USE_GM_TOOL, type );
}

void RecvEventNumber( t_packet *p, short int cn )
{
	CHARLIST *ch = CheckServerId( cn );
	if( !ch ) return;

	int event_no = p->u.kein.default_int;
}

void MapServerFree_kein()
{
	SAFE_DELETE(g_pDungeonMap);
	SAFE_DELETE(g_pDungeonMgr);
	
}

void SaveChangeLadder(CHARLIST* pTarget, int old_ladder, int new_ladder, int type)
{	//< CSD-040224
	string strPath;

	if (!g_pLogManager->GetLogPath(LT_CHANGE, strPath))
	{
		return;
	}
	
	FILE* fp = fopen(VA("%s/%s.txt", strPath.c_str(), pTarget->Name), "at+");

	if (fp == NULL)
	{
		fp = fopen(VA("%s/%s.txt", strPath.c_str(), connections[pTarget->GetServerID()].id), "at+");

		if (fp == NULL)
		{
			return;
		}
	}
	
	char why[MAX_PATH];

	switch (type)
	{
		case CLT_CHANGE_DUAL: 
			{
				strcpy(why, "upgrade dual");
				break;
			}
		case CLT_WIN_COLOSSUS:
			{
				strcpy(why, "win colossus");
				break;
			}
		case CLT_LOSE_COLOSSUS:
			{
				strcpy(why, "loss colossus");
				break;
			}
		default:
			{
				strcpy(why, "default");
				break;
			}
	}

	fprintf(fp,
		    "@ChangeLadderScore	%02d/%02d/%02d/%02d/%02d/%02d	(old ladder)%d(==>)	(new ladder)%d	(why)%s",
			g_year - 2000, g_mon + 1, g_day, g_hour, g_min, g_sec,
			old_ladder, new_ladder, why);
	fprintf(fp, "\n");
	fclose(fp);
}	//> CSD-040224

void SaveObtainCombat(CHARLIST* pTarget, int nOld, int nNew)
{	//< CSD-040224
	string strPath;

	if (!g_pLogManager->GetLogPath(LT_CHANGE, strPath))
	{
		return;
	}
	
	FILE* fp = fopen(VA("%s/%s.txt", strPath.c_str(), pTarget->Name), "at+");

	if (fp == NULL)
	{
		fp = fopen(VA("%s/%s.txt", strPath.c_str(), connections[pTarget->GetServerID()].id), "at+");

		if (fp == NULL)
		{
			return;
		}
	}
	
	fprintf(fp ,
		    "@ObtainCombatPoint	%02d/%02d/%02d/%02d/%02d/%02d	(old point)%d(==>)	(new point)%d\n",
		    g_year - 2000, g_mon + 1, g_day, g_hour, g_min, g_sec, nOld, nNew);
	fprintf(fp, ";Lightning Boom    %3d  Thunder Blow      %3d\n", pTarget->GetCombatLevel(LIGHTNING_BOOM), pTarget->GetCombatLevel(THUNDER_BLOW));
	fprintf(fp, ";Lightning Shock   %3d  Thunder Strike    %3d\n", pTarget->GetCombatLevel(LIGHTNING_SHOCK), pTarget->GetCombatLevel(THUNDER_STRIKE));
	fprintf(fp, ";Lightning Shield  %3d  Lightning Extreme %3d\n", pTarget->GetCombatLevel(LIGHTNING_SHIELD), pTarget->GetCombatLevel(LIGHTNING_EXTREME));
	fprintf(fp, ";Blood Earth       %3d  Ground Attack     %3d\n", pTarget->GetCombatLevel(BLOOD_EARTH), pTarget->GetCombatLevel(GROUND_ATTACK));
	fprintf(fp, ";Blood Will        %3d  Stone Armor       %3d\n", pTarget->GetCombatLevel(BLOOD_WILL), pTarget->GetCombatLevel(STONE_ARMOR));
	fprintf(fp, ";Simunian Bosom    %3d  Earth Extreme     %3d\n", pTarget->GetCombatLevel(SIMUNIAN_BOSOM), pTarget->GetCombatLevel(EARTH_EXTREME));
	fprintf(fp, ";Sword N'Roses     %3d  Multiple Fire     %3d\n", pTarget->GetCombatLevel(SWORD_N_ROSES), pTarget->GetCombatLevel(MULTIPLE_FIRE));
	fprintf(fp, ";Fleet Missile     %3d  Converting Armor  %3d\n", pTarget->GetCombatLevel(FLEET_MISSILE), pTarget->GetCombatLevel(CONVERTING_ARMOR));
	fprintf(fp, ";Burning Bloody	%3d  Fire Extreme      %3d\n", pTarget->GetCombatLevel(BURNING_BLOODY), pTarget->GetCombatLevel(FIRE_EXTREME)); // 030415 kyo 
	fprintf(fp, ";Poisoning Nova    %3d  Cherroy Shade     %3d\n", pTarget->GetCombatLevel(POISONING_NOVA), pTarget->GetCombatLevel(CHERROY_SHADE));
	fprintf(fp, ";Dark Blade        %3d  Tybern Gift       %3d\n", pTarget->GetCombatLevel(DARK_BLADE), pTarget->GetCombatLevel(TYBERN_GIFT));
	fprintf(fp, ";AcuquireDark      %3d  Dark Extreme      %3d\n", pTarget->GetCombatLevel(ACUQUIRE_DARK), pTarget->GetCombatLevel(DARK_EXTREME));// 030415 kyo 
	fprintf(fp, ";Icing Blast       %3d  Critical Hiding   %3d\n", pTarget->GetCombatLevel(ICING_BLAST), pTarget->GetCombatLevel(CRITICAL_HIDING));
	fprintf(fp, ";Shark Missile     %3d  Horn Of Iceberg   %3d\n", pTarget->GetCombatLevel(SHARK_MISSILE), pTarget->GetCombatLevel(HORN_OF_ICEBERG));
	fprintf(fp, ";Ice Shield        %3d  Ice Extreme       %3d\n", pTarget->GetCombatLevel(ICE_SHIELD), pTarget->GetCombatLevel(ICE_EXTREME));
	fprintf(fp, ";Charging          %3d  Double Attack     %3d\n", pTarget->GetCombatLevel(CHARGING), pTarget->GetCombatLevel(DOUBLE_ATTACK));
	fprintf(fp, ";Whilwind          %3d  Twister           %3d\n", pTarget->GetCombatLevel(WHILWIND), pTarget->GetCombatLevel(TWISTER));
	fprintf(fp, ";Gust              %3d  Wind Extreme      %3d\n", pTarget->GetCombatLevel(GUST), pTarget->GetCombatLevel(WIND_EXTREME));
	fprintf(fp, ";Reserved Combat Skill Point %3d \n", pTarget->GetCombatPoint());
	
	fclose(fp);
}	//> CSD-040224

void SaveChangeCombat(CHARLIST* pTarget, int nCombat, int nOld, int nNew)
{	//< CSD-040224
	string strPath;
	
	if (!g_pLogManager->GetLogPath(LT_CHANGE, strPath))
	{
		return;
	}
	
	FILE* fp = fopen(VA("%s/%s.txt", strPath.c_str(), pTarget->Name), "at+");

	if (fp == NULL)
	{
		fp = fopen(VA("%s/%s.txt", strPath.c_str(), connections[pTarget->GetServerID()].id), "at+");

		if (fp == NULL)
		{
			return;
		}
	}
	
	fprintf(fp,
		    "@ChangeCombatPoint	%02d/%02d/%02d/%02d/%02d/%02d	(old level)%d(==>)	(new level)%d	(combat skill)%d\n",
		    g_year - 2000, g_mon + 1, g_day, g_hour, g_min, g_sec, nOld, nNew, nCombat);
	fprintf(fp, ";Lightning Boom    %3d  Thunder Blow      %3d\n", pTarget->GetCombatLevel(LIGHTNING_BOOM), pTarget->GetCombatLevel(THUNDER_BLOW));
	fprintf(fp, ";Lightning Shock   %3d  Thunder Strike    %3d\n", pTarget->GetCombatLevel(LIGHTNING_SHOCK), pTarget->GetCombatLevel(THUNDER_STRIKE));
	fprintf(fp, ";Lightning Shield  %3d  Lightning Extreme %3d\n", pTarget->GetCombatLevel(LIGHTNING_SHIELD), pTarget->GetCombatLevel(LIGHTNING_EXTREME));
	fprintf(fp, ";Blood Earth       %3d  Ground Attack     %3d\n", pTarget->GetCombatLevel(BLOOD_EARTH), pTarget->GetCombatLevel(GROUND_ATTACK));
	fprintf(fp, ";Blood Will        %3d  Stone Armor       %3d\n", pTarget->GetCombatLevel(BLOOD_WILL), pTarget->GetCombatLevel(STONE_ARMOR));
	fprintf(fp, ";Simunian Bosom    %3d  Earth Extreme     %3d\n", pTarget->GetCombatLevel(SIMUNIAN_BOSOM), pTarget->GetCombatLevel(EARTH_EXTREME));
	fprintf(fp, ";Sword N'Roses     %3d  Multiple Fire     %3d\n", pTarget->GetCombatLevel(SWORD_N_ROSES), pTarget->GetCombatLevel(MULTIPLE_FIRE));
	fprintf(fp, ";Fleet Missile     %3d  Converting Armor  %3d\n", pTarget->GetCombatLevel(FLEET_MISSILE), pTarget->GetCombatLevel(CONVERTING_ARMOR));
	fprintf(fp, ";Burning Bloody	  %3d  Fire Extreme      %3d\n", pTarget->GetCombatLevel(BURNING_BLOODY), pTarget->GetCombatLevel(FIRE_EXTREME)); // 030415 kyo 
	fprintf(fp, ";Poisoning Nova    %3d  Cherroy Shade     %3d\n", pTarget->GetCombatLevel(POISONING_NOVA), pTarget->GetCombatLevel(CHERROY_SHADE));
	fprintf(fp, ";Dark Blade        %3d  Tybern Gift       %3d\n", pTarget->GetCombatLevel(DARK_BLADE), pTarget->GetCombatLevel(TYBERN_GIFT));
	fprintf(fp, ";AcuquireDark      %3d  Dark Extreme      %3d\n", pTarget->GetCombatLevel(ACUQUIRE_DARK), pTarget->GetCombatLevel(DARK_EXTREME)); // 030415 kyo 
	fprintf(fp, ";Icing Blast       %3d  Critical Hiding   %3d\n", pTarget->GetCombatLevel(ICING_BLAST), pTarget->GetCombatLevel(CRITICAL_HIDING));
	fprintf(fp, ";Shark Missile     %3d  Horn Of Iceberg   %3d\n", pTarget->GetCombatLevel(SHARK_MISSILE), pTarget->GetCombatLevel(HORN_OF_ICEBERG));
	fprintf(fp, ";Ice Shield        %3d  Ice Extreme       %3d\n", pTarget->GetCombatLevel(ICE_SHIELD), pTarget->GetCombatLevel(ICE_EXTREME));
	fprintf(fp, ";Charging          %3d  Double Attack     %3d\n", pTarget->GetCombatLevel(CHARGING), pTarget->GetCombatLevel(DOUBLE_ATTACK));
	fprintf(fp, ";Whilwind          %3d  Twister           %3d\n", pTarget->GetCombatLevel(WHILWIND), pTarget->GetCombatLevel(TWISTER));
	fprintf(fp, ";Gust              %3d  Wind Extreme      %3d\n", pTarget->GetCombatLevel(GUST), pTarget->GetCombatLevel(WIND_EXTREME));
	fprintf(fp, ";Reserved Combat Skill Point %3d \n", pTarget->GetCombatPoint());
	
	fclose(fp);
}	//> CSD-040224

// 021117 YGI
void RecvSetInstallVersion ( t_packet *p, short int cn )
{
	CHARLIST *ch = CheckServerId( cn );
	if( !ch ) return;
	ch->installType = IT_LIGHT_VERSION;
}

//021030 YGI
void RecvResendItem( short int cn )
{
	if( !CheckServerIndex( cn ) ) return;
	SendItemInventory( 0, connections, cn );
	SendItemInventory( 1, connections, cn );
	SendItemInventory( 2, connections, cn );
	SendItemInventory( 3, connections, cn );
	SendItemInventory( 4, connections, cn );
	SendItemInventory( 5, connections, cn );
	SendItemInventory( 6, connections, cn );
	SendItemInventory( 7, connections, cn );
	SendItemInventory( 8, connections, cn );
}

void RecvScriptItem( t_packet *p, short int cn )
{
	k_event_script_item	*pData = (k_event_script_item *)p->u.data;

	CEventNpcScript *pEvent = g_EventMgr.GetEventNpcScriptByEventNo( pData->event_no);
	if( !pEvent ) return;

	CHARLIST *ch = CheckServerId( pData->server_id );
	if( !ch ) return;

	if( strcmp( ch->Name, pData->name ) != 0 ) return ;
	
	if( pData->result )
	{
		SendEventNpcScriptItem( pData->server_id, pData->npc_index, pEvent->m_nEventNpcItemBag, 
			pEvent->m_lpszMessage, pEvent->m_nMessageSize );
	}
	else		// �������� ������ ���� ������ �̾߱⸦ �������
	{
		if( pEvent->m_lpszMessage2 && pEvent->m_nMessageSize2 )		// npc�� ��ȭ�� �����ش�.
			SendDial( pData->server_id, pData->npc_index, pEvent->m_lpszMessage2, pEvent->m_nMessageSize2 );

	}
}

void RecvEventMoveMap( t_packet *p, short int cn )
{
	CHARLIST *ch = CheckServerId( cn );
	if( !ch ) return;

	int event_no = p->u.kein.default_int;
	CEventMoveMap *pEvent = g_EventMgr.GetEventMoveMapPoint( event_no );
	if( !pEvent ) return;

	// �̺�Ʈ ���� ���� Ȯ�� --------------------
	if( !pEvent->m_bAll )
	{
		static int sex2sex[] = { 2, 1 };
		static int class2class[] = { 1, 3, 2, 5, 4 };
		static int nation2nation[] = { 0, 0, 0, 1, 2, 0, 3 };

		if( pEvent->m_nSex )
		{
			if( pEvent->m_nSex != sex2sex[ch->SprNo] ) return;
		}
		if( pEvent->m_nClass )
		{
			if( pEvent->m_nClass != class2class[ch->Class] ) return;
		}
		if( pEvent->m_nNation )
		{
			if( pEvent->m_nNation != nation2nation[ch->name_status.nation] ) return;
		}
	}
	//----------------------------

	t_packet packet;
	packet.h.header.type = CMD_EVENT_MOVE_MAP_CHECK_MAP;
		packet.u.kein.event_move_map_check_map.server_id = cn;
		packet.u.kein.event_move_map_check_map.server_port = GetOwnPort();
		packet.u.kein.event_move_map_check_map.event_no = event_no;
	packet.h.header.size =  sizeof( k_event_move_map_check_map );

	int port = GetMapPortByName( pEvent->m_szMap );
	if( port == GetOwnPort() ) return;
	if( !port )	return;
	SendPacket2Map( &packet, port );
}

void RecvEventMoveMapCheckMap( t_packet *p, short int cn )		// �ٸ� �ʿ��� �ð��
{
	k_event_move_map_check_map *pData = (k_event_move_map_check_map *)p->u.data;
	CEventMoveMap *pEvent = g_EventMgr.GetEventMoveMapPoint( pData->event_no );
	if( !pEvent ) return;
	if( _stricmp( pEvent->m_szMap, MapName ) != 0 ) return;		// �̺�Ʈ ��ȣ�� ������. 

	int user_count = g_pServerTable->GetNumOfUsers();
	if( user_count >= pEvent->m_nMax )
	{
		// ���̻� ������ �� ����.
		k_put_menu_string msg;
		msg.type = KM_FAIL;
		msg.str_num = 212;

		SendDirectClientOtherMap( CMD_PUT_MENU_STRING, pData->server_id, pData->server_port, &msg, sizeof( k_put_menu_string ) );
	}
	else
	{
		t_packet packet;
		packet.h.header.type = CMD_EVENT_MOVE_MAP_OK;
			strcpy( packet.u.kein.event_move_map_ok.map_name, MapName );
			packet.u.kein.event_move_map_ok.server_id = pData->server_id;
			packet.u.kein.event_move_map_ok.map_x = pEvent->m_nMapX;
			packet.u.kein.event_move_map_ok.map_y = pEvent->m_nMapY;

		packet.h.header.size = sizeof( k_event_move_map_ok );

		SendPacket2Map( &packet, pData->server_port );

		// ������ �����Ѵ�.
		k_save_event_move_map save_data;
		save_data.event_no = pData->event_no;
		save_data.max = pEvent->m_nMax;	// 021128 YGI
		save_data.now = user_count;
		SendPacketDefault( CMD_SAVE_EVENT_MAPMOVE_NOW, &save_data, sizeof( k_save_event_move_map ), DB_DEMON );
	}
}

void RecvEventMoveMapOk( t_packet *p, short int cn )
{
	k_event_move_map_ok *pData = (k_event_move_map_ok *)p->u.data;
	MapMove( pData->server_id, pData->map_name, pData->map_x, pData->map_y );
}

void CheckDoubleName( CHARLIST *ch, short int cn )
{
	char name[20]={0,};
	::strcpy( name, ch->Name );
	int target_id = ExistHe( ch->Name );
	if( target_id > 0 )		// ������ ���� �̸��� ������ �ִ�.
	{
		MyLog(1,"CheckDoubleName() name = %s", ch->Name );
		closeconnection( connections, target_id, -101 );		// �ϴ� �ٸ� ���� ���� --> ����
		ch->updatable = 0; // CSD-TW-030620
		closeconnection( connections, cn, -102 );				// ���� ���� �� ���� �������� ���鼭 ����...
	}
	else
	{
		target_id = ExistConnectingHe( ch->Name );
		if( target_id > 0 )
		{
			connections[target_id].chrlst.updatable = 1; // CSD-TW-030620
			closeconnection( connections, target_id, -103 );		// �ϴ� �ٸ� ���� ���� --> ����
			ch->updatable = 0; // CSD-TW-030620
			closeconnection( connections, cn, -104 );				// ���� ���� �� ���� �������� ���鼭 ����...
		}
	}

	// �ٸ� ���� �ִ� �� ����..
	t_packet packet;
	packet.h.header.type = CMD_CHECK_DOUBLE_NAME;
		strcpy( packet.u.kein.invite_colossus_map.name, name );
		packet.u.kein.invite_colossus_map.port = GetOwnPort();
	packet.h.header.size = sizeof( k_invite_colossus_map );
	SendPacket2Map( &packet, ST_SEND_MAP_ALL );
}

// �ش� �̸��� �ִ��� Ȯ���Ѵ�.
void RecvCheckDoubleName( t_packet *p, short int cn )
{
	char *name = p->u.kein.invite_colossus_map.name;
	int port = p->u.kein.invite_colossus_map.port;

	int target_id = ExistHe( name );
	if( target_id > 0 )
	{
		closeconnection( connections, target_id, -105 );		// �ϴ� �ٸ� ���� ���� --> ����
		// �� ����� �˷��ش�.
		t_packet packet;
		packet.h.header.type = CMD_CHECK_DOUBLE_NAME_OK;
			strcpy( packet.u.kein.default_name, name );
		packet.h.header.size = strlen(name);
		SendPacket2Map( &packet, port );
	}
	else
	{
		target_id = ExistConnectingHe( name );
		if( target_id > 0 )
		{
			connections[target_id].chrlst.updatable = 1; // CSD-TW-030620
			closeconnection( connections, target_id, -106 );		// �ϴ� �ٸ� ���� ���� --> ����

			// �� ����� �˷��ش�.
			t_packet packet;
			packet.h.header.type = CMD_CHECK_DOUBLE_NAME_OK;
				strcpy( packet.u.kein.default_name, name );
			packet.h.header.size = strlen(name);
			SendPacket2Map( &packet, port );
		}
	}
}

void RecvCheckDoubleNameOk( t_packet *p, short int cn )
{
	char *name = p->u.kein.default_name;
	int target_id = ExistHe(name );
	if( target_id > 0 )
	{	// �ٸ� �ʿ� ���� ���� �ִٴ� �ҽ��� �Ա� ������ �������� ���� ��������
		connections[target_id].chrlst.updatable = 0; // CSD-TW-030620
		closeconnection( connections, target_id, -102 );
	}
	else
	{
		target_id = ExistConnectingHe( name );

		if( target_id > 0 )
		{
			connections[target_id].chrlst.updatable = 0; // CSD-TW-030620
			closeconnection( connections, target_id, -102 );		// �ϴ� �ٸ� ���� ���� --> ����
		}
	}
}

void RecvCheckDoubleNameDB2Map( t_packet *p, short int cn )
{
	return;
	int target_id;
	int bClose;
	char *pName;  
	for( int i=0; i<4; i++ )
	{
		pName = p->u.kein.login_check_double_name.name[i];
		if( pName[0] )
		{
			EatRearWhiteChar( pName );
			bClose = 0;
			target_id = ExistHe( pName );
			if( target_id > 0 )
			{
				bClose = 1;
			}
			else
			{
				target_id = ExistConnectingHe( pName );
				if( target_id > 0 )
				{
					bClose = 1;
				}
			}

			if( bClose )
			{
				connections[target_id].chrlst.updatable = 1; // CSD-TW-030620
				closeconnection( connections, target_id, -101 );
			}
		}
	}
}

void RecvCheckDoubleNameWhenFirst( t_packet *p, short int cn )
{
	char *pName = p->u.kein.check_double_when_first.name;
	int server_id = p->u.kein.check_double_when_first.server_id;
	int port = p->u.kein.check_double_when_first.port;

	int bClose = 0;
	int target_id = ExistHe( pName );
	if( target_id > 0 )
	{
		bClose = 1;
	}
	else
	{
		target_id = ExistConnectingHe( pName );
		if( target_id > 0 )
		{
			bClose = 1;
		}
	}

	if( bClose )
	{
		connections[target_id].chrlst.updatable = 1; // CSD-TW-030620
		closeconnection( connections, target_id, -101 );

		t_packet packet;
		packet.h.header.type = CMD_CHECK_DOUBLE_NAME_WHERE_OK;
			packet.u.kein.default_short_int = server_id;
		packet.h.header.size = sizeof( short int );
		SendPacket2Map( &packet, port );
	}
}

void RecvCheckDoubleNameWhenFirstOk( t_packet *p, short int cn )
{
	int server_id = p->u.kein.default_short_int;
	if( CheckServerIndex( server_id ) )
	{
		connections[server_id].chrlst.updatable = 0; // CSD-TW-030620
		closeconnection( connections, server_id, -102 );
	}
}

void CheckDoubleNameWhenFirst( char *pName, short int server_id )
{
	int bClose = 0;
	int target_id = ExistHe( pName );
	if( target_id > 0 )
	{
		bClose = 1;
	}
	else
	{
		target_id = ExistConnectingHe( pName );
		if( target_id > 0 )
		{
			bClose = 1;
		}
	}
	if( bClose )
	{
		connections[target_id].chrlst.updatable = 1; // CSD-TW-030620
		closeconnection( connections, target_id, -101 );

		connections[server_id].chrlst.updatable = 0; // CSD-TW-030620	
		closeconnection( connections, server_id, -102 );
	}

	t_packet packet;
	packet.h.header.type = CMD_CHECK_DOUBLE_NAME_WHERE_FIRST;
		strcpy( packet.u.kein.check_double_when_first.name, connections[server_id].name );
		packet.u.kein.check_double_when_first.server_id = server_id;
		packet.u.kein.check_double_when_first.port = GetOwnPort();
	packet.h.header.size = sizeof( k_check_double_when_first );
	SendPacket2Map( &packet, ST_SEND_MAP_ALL );
}

// 030923 HK YGI
void SaveLoginLogoutLogByKein(CHARLIST* pTarget, int type, int call_function_id )
{
	if (!g_EventFlagMgr.IsSaveLogWhenSaveUserData())
	{
		return;
	}

	char type2str[2][20] = {"LOGOUT", "LOGIN"};
	
	string strPath;

	if (g_pLogManager->GetLogPath(LT_CHANGE, strPath))
	{
		char temp[FILENAME_MAX] = {0,};
		char log[FILENAME_MAX] = {0,};
		::sprintf(temp, "%s/savelog_%s.txt", strPath.c_str(), pTarget->Name);
		::sprintf(log, "%s(%d): [%s][%s] lv=%d, exp=(%d)%d, class=%d",
			      type2str[type], call_function_id, MapName,
				  pTarget->Name, pTarget->GetLevel(), pTarget->GetExperienceStep(), pTarget->Exp, pTarget->Class);
		SaveLogDefault2(temp, log);
	}
}

// 031009 CI YGI
int InitItemLimitCountFromDB( )
{
	char query[512];
	WORD port = GetOwnPort();
	if( !port )
	{
		return 0;
	}
	DWORD limit_number;
	int ret = GetItemLimitCount_SQL( port, &limit_number );
	if( ret < 1 )
	{
		limit_number = 1;
		sprintf( query, "INSERT INTO ItemLimitMgrTable ( port, limit_number ) VALUES ( %d, %d )", port, limit_number );
		Querry_SQL( query );
	}

	ItemMgr.SetLimitCount( limit_number );

	ret = ItemMgr.LoadHaveLimitItem();
	if( !ret )		// ����
	{
		JustMsg( "Error!!! check please ItemList_HaveLimit table" );
		return 0;
	}
	return 1;
}
// 040105 YGI ���� ----------------
int GetTreasureMining( int lv )
{
	switch( lv )
	{
		case 1: return 30;
		case 2: return 50;
		case 3: return 70;
		case 4: return 99;
	}
	return 101;
}
void RecvCheckTreasureXY( t_packet *p, short int cn )
{
	CHARLIST *ch = CheckServerId( cn );
	if( !ch ) return;
	int event_index = -1;

	k_check_treasure_xy *pData = (k_check_treasure_xy *)p->u.data;
	int tile_x = pData->x/TILE_SIZE;
	int tile_y = pData->y/TILE_SIZE;
	int lv = 0;
	bool mining = false;		// ����� �����ϴ�

	for( int a=0; a<3; a++ )
	{
		for( int b=0; b<3; b++ )
		{
			for( int c=0; c<8; c++ )
			{
				ItemAttr *pItem = &ch->inv[a][b][c];
				lv = IsTreasureMapItem( pItem->item_no );
				if( !lv ) continue;		// ���� ������ �ƴϴ�.
				CTreasureMapItem *pAttr = (CTreasureMapItem *)&pItem->attr[IATTR_TREASURE_MAP];
				if( !pAttr->isChecked || pAttr->mapnumber != MapNumber ) continue;
				if( ch->skillexp[MINING].skillexp/10000 < GetTreasureMining( lv ) )
				{
					mining = false;
					continue;
				}
				if( pAttr->x != tile_x || pAttr->y != tile_y ) continue;
				event_index = g_EventFindTreasure.CheckXY( tile_x, tile_y );
				if( event_index == -1 ) continue;
				// ���̳� ����
				mining = true;
				
				// ����Ѱ��� �ٽ� ������� ���ϰ� ���� �Ѵ�.
				//pAttr->use = 1;
				POS pos;
				SetItemPos( INV, a, b, c, &pos );
				SendDeleteItem( pItem, &pos, ch );
				//SendServerEachItem( &pos, pItem, cn );
				goto find_stop;
			}
		}
	}

find_stop : 
	if( event_index == -1 ) return;
	if( lv == 0 ) return;
	if( !mining ) 
	{
		SendPutMenuString( KM_FAIL, 215, cn );
		return;
	}

	if( g_TreasureGuardMgr.IsEvent( event_index ) ) return;	// �̹� �����߾����� Ȯ��
	int treasure_class = lv-1;		// Ƣ�� ���;� �� ���� ���߿� ���� ������ ���
	int ret = g_TreasureGuardMgr.CreateGuard( event_index, treasure_class, tile_x, tile_y, cn );
	if( ret )
	{
		MyLog( 0, "Treasure Guard Event Start <event_index:%d>, <treasure_class:%d>, (%d,%d) <%s>",
			event_index, treasure_class, tile_x, tile_y, ch->Name );
	}
} //////////////////////////

void RecvGetTreasureMapDetail( t_packet *p, short int cn )
{
	CHARLIST *ch = CheckServerId( cn );
	if( !ch ) return;

	POS *pPos = &p->u.kein.default_pos;
	ItemAttr *item = GetItemByPOS( ch, *pPos );
	if( !item ) return;

	int lv = IsTreasureMapItem( item->item_no );
	if( !lv ) return;		// ���� ������ �ƴϴ�.

	CTreasureMapItem *pAttr = (CTreasureMapItem *)&item->attr[IATTR_TREASURE_MAP];
	if( pAttr->isChecked) return;

	t_packet packet;
	packet.h.header.type = CMD_GET_TREASURE_MAP_DETAIL_DB;
		packet.u.kein.get_treasure_map_detail_to_db.cn = cn;
		packet.u.kein.get_treasure_map_detail_to_db.pos = *pPos;
	packet.h.header.size = sizeof( k_get_treasure_map_detail_to_db );
	QueuePacket( connections, DB_DEMON, &packet, 1 );
}

void RecvGetTreasureMapDetailDB( t_packet *p, short int cn )
{
	k_get_treasure_map_detail_to_map *pData = &p->u.kein.get_treasure_map_detail_to_map;
	CHARLIST *ch = CheckServerId( pData->cn );
	if( !ch ) return;

	ItemAttr *item = GetItemByPOS( ch, pData->pos );
	if( !item ) return;
	int lv = IsTreasureMapItem( item->item_no );
	if( !lv ) return;		// ���� ������ �ƴϴ�.
	CTreasureMapItem *pAttr = (CTreasureMapItem *)&item->attr[IATTR_TREASURE_MAP];
	if( pAttr->isChecked ) return;

	pAttr->isChecked = 1;
	pAttr->mapnumber = pData->map_number;
	//pAttr->use = 0;
	// lv�� ���� �����ϰ�...
	pAttr->x = pData->x;
	pAttr->y = pData->y;
	switch( lv )
	{
		case 1 : pAttr->offset2 = 0; pAttr->offset = 0; break;
		case 2 : pAttr->offset2 = rand()%4; pAttr->offset = rand()%2; break;
		case 3 : pAttr->offset2 = rand()%4; pAttr->offset = rand()%3; break;
		case 4 : pAttr->offset2 = rand()%4; pAttr->offset = rand()%4; break;
	}

	SendServerEachItem( &pData->pos, item, pData->cn );
}
