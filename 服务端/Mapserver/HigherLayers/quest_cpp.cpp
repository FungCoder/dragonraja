#include "..\stdafx.h"
#include "DefaultHeader.h"
#include "MenuDefine.h"
#include "ItemList.h"
#include "CItem.h"
#include "Scrp_exe.h"
#include "quest_cpp.h"


void SendBBS2( char *text, short int cn )
{
	t_packet p;
	p.h.header.type = CMD_BBS_QUEST;
		strcpy( p.u.server_bbs.bbs, text );
	p.h.header.size = sizeof( t_server_bbs ) - MAX_PATH + strlen( text );
	QueuePacket( connections, cn , &p, 1 );
}