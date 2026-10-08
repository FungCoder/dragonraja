#include "..\stdafx.h"
#include "..\LowerLayers\servertable.h"
#include "..\LowerLayers\recvmsg.h"
#include "..\LowerLayers\mylog.h"
#include "DefaultHeader.h"
#include "MenuDefine.h"
#include "ItemList.h"
#include "CItem.h"
#include "map.h"

tagNationInfo	NationInfo[MAX_NATION];
bool			g_MainNationMap[MAX_NATION];			// ����ϴ� ���� ��ȣ

void CheckMainMap()
{	//< CSD-030506
	for (int i = 0; i < MAX_NATION; ++i)
	{
		if (NationInfo[i].main_map_port == g_MapPort)
		{
			g_MainNationMap[i] = true;
			LoadNationInfoOfMainMap(i);	// ���� �޴����� ����� ������� �ε��ؿ´�.
		}
	}
}	//> CSD-030506

void SendNationMoney( int type, int nation, double money )
{
	t_packet packet;

	packet.h.header.type = CMD_NATION_INFO_MAP_TO_MAP;

		packet.u.kein.nation_info_money.type = type;
		packet.u.kein.nation_info_money.nation = nation;
		packet.u.kein.nation_info_money.money = money;

	packet.h.header.size = sizeof( k_nation_info_money );

	if( type != N_NATION_ALL && g_MainNationMap[nation] )		// �̰����� ����ϴ� ���̶��
	{
		RecvNationInfo( &packet );		// ���� ó���ϰ�
	}
	else								// �ƴϸ�
	{									// ��� ������ ������.
		g_pServerTable->SendRajaPacketToOtherMapServer( (WORD)NationInfo[nation].main_map_port, (char *)&packet, packet.h.header.size+sizeof(t_header) );
	}
}

void SaveNationInfo( int nation )		// ��� ���� �����Ѵ�.
{
	static int save_time = g_curr_time;
	if( (g_curr_time - save_time) < 600 )	return;	// 10�п� �ѹ���
	save_time = g_curr_time;

	if( !g_MainNationMap[nation] ) // ���� ���� �ƴϴ�.	
		return;

	t_packet packet;
	packet.h.header.type = CMD_SAVE_NATION_INFO;

		packet.u.kein.save_nation_info.nation		=	nation;
		packet.u.kein.save_nation_info.nation_money	=	NationInfo[nation].nation_money;
		packet.u.kein.save_nation_info.king_support	=	NationInfo[nation].king_support;
		packet.u.kein.save_nation_info.friend_va	=	NationInfo[nation].friend_va	;
		packet.u.kein.save_nation_info.friend_zy	=	NationInfo[nation].friend_zy;
		packet.u.kein.save_nation_info.friend_ill	=	NationInfo[nation].friend_ill;
		memcpy( packet.u.kein.save_nation_info.nation_item, NationInfo[nation].nation_item, sizeof( int ) * 20 );

	packet.h.header.size = sizeof( k_save_nation_info );
	QueuePacket( connections, DB_DEMON, &packet,1 );


	// ��� �ʿ� �ѷ� �ش�.
	packet.h.header.type = CMD_NATION_INFO_MAP_TO_MAP;

		packet.u.kein.nation_info_money.type = N_NATION_ALL;
		packet.u.kein.nation_info_money.nation = nation;
		packet.u.kein.nation_info_money.money = NationInfo[nation].nation_money;

	packet.h.header.size = sizeof( k_nation_info_money );
	g_pServerTable->BroadCastToEveryServer( (char*)&packet, (sizeof(t_header)+packet.h.header.size), SERVER_TYPE_MAP );
}

void RecvNationInfo( t_packet *p )
{
	int type = p->u.kein.nation_type;
	switch( type )
	{
		case NM_ADD	:		NationInfo[p->u.kein.nation_info_money.nation].nation_money += p->u.kein.nation_info_money.money;
			SaveNationInfo(p->u.kein.nation_info_money.nation);
			break;
		case NM_SUBTRACT :	
			{
				int nation = p->u.kein.nation_info_money.nation;
				int money = p->u.kein.nation_info_money.money;
				if( NationInfo[nation].nation_money > money )
				{
					NationInfo[p->u.kein.nation_info_money.nation].nation_money -= p->u.kein.nation_info_money.money;
				}
				else NationInfo[nation].nation_money = 0;
				SaveNationInfo(p->u.kein.nation_info_money.nation);
				break;
			}
		case N_NATION_ALL : 
			{
				int nation = p->u.kein.nation_info_money.nation;
				NationInfo[nation].nation_money = p->u.kein.nation_info_money.money;
				break;
			}
		default : return;
	}
}
