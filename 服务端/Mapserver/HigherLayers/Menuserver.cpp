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
#include "gamblerate.h"
#include "LTSSupport.h"
#include "Skill_lsw.h"
#include "GmMgr.h"
#include "eventmgr.h" //021030 YGI
#include "LogManager.h"
#include "UserManager.h"
#include "DualManager.h"
#include "GuildManager.h"
#include "ArenaManager.h"

const int SIZE_GR_BS = 50; // CSD-030326
const int SIZE_GR_RS = 100; // CSD-030326

CGambleRateData g_game_rate_data;
CAliveMapZoneAll g_AliveMap;		// �ǻ�� ���� �ִ� �������̺�

extern struct CItem_List Item_Ref ;
extern bool exist(int item_no, int char_id, POS &source);
extern void getItemIndex(int item_no, int &type, int &index);
extern int GetItemValueSell( ItemAttr *item );
extern void CleanUpConnection(short idServer);

void SendStoreItemList( int cn, int storelistindex, int menud);
void SendCallSmallMenu( int cn, int storelistindex, int menu );
void SendCallDualMagicLearnMenu( int cn);//soto-030421
int LoadGuildItem( char *filename, int num );

char g_szPassWordName[20];
int		 storelist_attr[ MAX_STORE_LIST][ MAX_STORE_ITEM_LIST];		// ��� ���� �Ĵ� ���� �Ӽ�
ItemAttr storelist[ MAX_STORE_LIST][ MAX_STORE_ITEM_LIST];
int storelistmax[ MAX_STORE_LIST];
//  ������ Item �Ӽ��� ���� �ٲ��. 
//  ���Լ��� ��.. 1�Ͽ� �ѹ����� ȣ��Ǿ�� �Ѵ�. 
static inline void swap( int  *a, int *b )
{
	int t = *a;
	*a = *b;
	*b = t;	
}			

void MakeStoreList( void )
{
	char temp[ FILENAME_MAX];
	wsprintf( temp, "%s/data/BLACKSMITH.txt", GameServerDataPath );	storelistmax[ BLACKSMITH ] = LoadGuildItem( temp,	BLACKSMITH	);	//���尣		
	MyLog( LOG_NORMAL, " Store 'BLACKSMITH' itemlist number   : %d", storelistmax[ BLACKSMITH ] );
	wsprintf( temp, "%s/data/FARM.txt", GameServerDataPath );		storelistmax[ FARM		 ] = LoadGuildItem( temp,	FARM		);	//����		
	MyLog( LOG_NORMAL, " Store 'FARM'       itemlist number   : %d", storelistmax[ FARM] );
	wsprintf( temp, "%s/data/RESTAURANT.txt", GameServerDataPath );	storelistmax[ RESTAURANT ] = LoadGuildItem( temp,	RESTAURANT	);	//�丮��		
	MyLog( LOG_NORMAL, " Store 'RESTAURANT' itemlist number   : %d", storelistmax[ RESTAURANT] );
	wsprintf( temp, "%s/data/HERBSHOP.txt", GameServerDataPath );	storelistmax[ HERBSHOP	 ] = LoadGuildItem( temp,	HERBSHOP	);	//������		
	MyLog( LOG_NORMAL, " Store 'HERBSHOP'   itemlist number   : %d", storelistmax[ HERBSHOP	 ] );
	wsprintf( temp, "%s/data/SHAMBLES.txt", GameServerDataPath );	storelistmax[ SHAMBLES	 ] = LoadGuildItem( temp,	SHAMBLES	);	//Ǫ�ް�		
	MyLog( LOG_NORMAL, " Store 'SHAMBLES'   itemlist number   : %d", storelistmax[ SHAMBLES	 ] );
	wsprintf( temp, "%s/data/CANDLE.txt", GameServerDataPath );		storelistmax[ CANDLE	 ] = LoadGuildItem( temp,	CANDLE		);	//������		
	MyLog( LOG_NORMAL, " Store 'CANDLE'     itemlist number   : %d", storelistmax[ CANDLE	 ] );
	wsprintf( temp, "%s/data/SLOPSHOP.txt", GameServerDataPath );	storelistmax[ SLOPSHOP	 ] = LoadGuildItem( temp,	SLOPSHOP	);	//�ǻ��		
	MyLog( LOG_NORMAL, " Store 'SLOPSHOP'   itemlist number   : %d", storelistmax[ SLOPSHOP  ] );
	wsprintf( temp, "%s/data/ALCHEMY.txt", GameServerDataPath );	storelistmax[ ALCHEMY	 ] = LoadGuildItem( temp,	ALCHEMY		);	//���ݼ�		
	MyLog( LOG_NORMAL, " Store 'ALCHEMY'    itemlist number   : %d", storelistmax[ ALCHEMY	 ] );
	wsprintf( temp, "%s/data/WOODWORKING.txt", GameServerDataPath );storelistmax[ WOODWORKING] = LoadGuildItem( temp,	WOODWORKING	);	//�����		
	MyLog( LOG_NORMAL, " Store 'WOODWORKING'itemlist number   : %d", storelistmax[WOODWORKING] );
	wsprintf( temp, "%s/data/BOWCRAFT.txt", GameServerDataPath);	storelistmax[ BOWCRAFT	 ] = LoadGuildItem( temp,	BOWCRAFT	);		//Ȱ��ȭ��	
	MyLog( LOG_NORMAL, " Store 'BOWCRAFT'   itemlist number   : %d", storelistmax[ BOWCRAFT	 ] );
	wsprintf( temp, "%s/data/SHIPYARD.txt", GameServerDataPath );	storelistmax[ SHIPYARD	 ] = LoadGuildItem( temp,	SHIPYARD	);	//������		
	MyLog( LOG_NORMAL, " Store 'SHIPYARD'   itemlist number   : %d", storelistmax[ SHIPYARD	 ] );
	wsprintf( temp, "%s/data/PUB.txt", GameServerDataPath );		storelistmax[ PUB		 ] = LoadGuildItem( temp,	PUB			);	//����		
	MyLog( LOG_NORMAL, " Store 'PUB'        itemlist number   : %d", storelistmax[ PUB		 ] );
	wsprintf( temp, "%s/data/STORE.txt", GameServerDataPath );		storelistmax[ STORE		 ] = LoadGuildItem( temp,	STORE		);	//��ȭ��		
	MyLog( LOG_NORMAL, " Store 'STORE'      itemlist number   : %d", storelistmax[ STORE	 ] );
	wsprintf( temp, "%s/data/MAGIC_GUILD.txt", GameServerDataPath );storelistmax[MAGIC_GUILD ] = LoadGuildItem( temp,	MAGIC_GUILD );	//������		
	MyLog( LOG_NORMAL, " Store 'MAGIC_GUILD'itemlist number   : %d", storelistmax[ MAGIC_GUILD ] );
}

void CallSmallMenu(int cn, int storelistindex, int menu)
{	//< CSD-030509
	switch (menu)
	{
	case MN_SKILLGUILD_REPAIR:
	case MN_SHOP_SELL:
	case MN_SHOP_BUY:
		{
			SendStoreItemList(cn, storelistindex, menu);
			break;
		}
	case MN_SKILLGUILD_BUY:	
	case MN_SKILLGUILD_SELL:	
		{
			SendStoreItemList(cn, storelistindex, menu); 
			SendCallSmallMenu(cn, storelistindex, menu);
			break;
		}
	case MN_ARENA_LOBBY:
		{
			CBaseArena* pGame = g_pArenaManager->GetGameInfo(storelistindex);

			if (pGame != NULL)
			{	//< CSD-TW-030627
				if (!pGame->IsPlayGame())
				{
					SendCallSmallMenu(cn, storelistindex, menu);
					return;
				}
				
				CHARLIST* pTarget = CheckServerId(cn);

				if (pTarget != NULL)
				{
					pGame->SendRemainTime(pTarget, g_curr_time);
				}
			}	//> CSD-TW-030627

			break;
		}
	default:
		{
			SendCallSmallMenu(cn, storelistindex, menu);
			break;
		}
	}
}	//> CSD-030509

// 010815 YGI
void RecvItemBuy( int cn, t_client_item_buy *p )
{
	CHARLIST *ch = CheckServerId( cn );
	if( !ch ) return;
//	ch->Money = GetMoneyByItem( ch );

	int store_id = p->store_id;
	int itemno = p->item_no;
	int a,b,c;
	int itempos;
	
	if( store_id >= MAX_STORE_LIST ) return; // Error....
	if( !itemno ) return;

	for( itempos=0; itempos<storelistmax[store_id]; itempos++)
	{
		if( storelist_attr[store_id][itempos] == SIT_EVENT_ITEM ||
			storelist_attr[store_id][itempos] == SIT_NULL ||
			storelist_attr[store_id][itempos] == SIT_ONLY_BUY ) continue;

		if( itemno == storelist[ store_id][itempos].item_no )
		{
			break;
		}
	}
	if( itempos == storelistmax[store_id] ) return;		// �� ������ ���� �������̴�.

	ItemAttr *item = &storelist[ store_id][itempos];	
	CItem *t = ItemUnit( *item );
	if( !t ) return;

	const DWORD price = t->GetValue();
	if( ch->Money < price ) return;			// �ϴ� �� ������ �ȵȴ�.

	//Quick �� �˻��Ͽ� IK_POTION_BOX �ΰ�.. �ؼ�
	const int iItemNo = item->item_no;
	bool bHavePotionBox = false;
	for(int i=0; i < ITEM_QUICK_MAX; i++)
	{
		if( !ch->quick[i].item_no ){continue;}//�������� ������ ��Ƽ��
		CItem *tt = ItemMgr.ConvertItemNoToCItem( ch->quick[i].item_no );
		if( !tt ) continue;
		if(IK_POTION_BOX == tt->GetItemKind())
		{
			bHavePotionBox = true;
			break;
		}
	}
	
	if(bHavePotionBox)
	{
		CPotionBox potion_box( ch );
		if( potion_box.PutItem( item ) ) // ���� ���ڿ� ����.
		{
			if( SubtractMoney( price, ch ) == -1 )		// �����ݿ��� ������ ������
			{
				// �����ϸ� ���� �ٽ� ���´�.
				potion_box.DeleteItem();
			}
			*item = GenerateItem( iItemNo);		// �Ȱ� �� �� ���ο� ������ ���� ( �̰� ������ ��츸���� �ٲٴ��� �ؾ���.. )
			return;	
		}
	}

	int ret = 0;
	if( t->GetRbutton() == DIVIDE_ITEM )	
	{
		ret = SearchItemPosByNumber( item, a, b, c, ch );
	}
	if( !ret )
	{
		if( !SearchInv( ch->inv, a, b, c ) ) return;		// ���ڸ��� ���ٸ�
	}

	// ���� ������ ���� ������. �����ϸ� �� ���.
	if( SubtractMoney( price, ch ) == -1 ) return;	
	if( ch->inv[a][b][c].item_no )
	{
		ch->inv[a][b][c].attr[IATTR_MUCH] += item->attr[IATTR_MUCH];
	}
	else
	{
		ch->inv[a][b][c] = *item;		// ���⼭ generater �Լ��� ȣ������ �ʴ� ������ ���߿� Ư���� �������� �Ȱ�� ����ؼ�..
	}

	POS pos;
	SetItemPos( INV, a, b, c, &pos );
	SendServerEachItem( &pos , &ch->inv[a][b][c], cn );		// ������ �������� �� ��� �����ش�.
	SendItemEventLog( &ch->inv[a][b][c], cn, SN_NOT_USER, SILT_GET_STORE, 3 ); //YGI acer

	*item = GenerateItem( iItemNo);		// �Ȱ� �� �� ���ο� ������ ���� ( �̰� ������ ��츸���� �ٲٴ��� �ؾ���.. )
	return;
}

void RecvItemSell( int cn, t_client_item_sell *p )
{
	CHARLIST *ch = CheckServerId( cn );
	if( !ch ) return;

	int store_id = p->store_id;
	int pos = p->item_position;

	int a = ( pos & 0x0060 ) >> 5;
	int b = ( pos & 0x0018 ) >> 3;
	int c = ( pos & 0x0007 );

	if( a<0 || a>=3 || b<0 || b>=3 || c<0 || c>=8 ) return;
	ItemAttr *item = &ch->inv[a][b][c];

	if( !item->item_no ) return;
	if( store_id > MAX_STORE_LIST) return;
	if( store_id < 0) return;

	int itempos=0;
	for( itempos=0; itempos	<	storelistmax[store_id]; itempos++)
	{
		// �ȵǴ� ����
		if( storelist_attr[store_id][itempos] == SIT_EVENT_ITEM ||
			storelist_attr[store_id][itempos] == SIT_NULL ||
			storelist_attr[store_id][itempos] == SIT_ONLY_SELL ) 
		{
			continue;
		}
		
		if( item->item_no == storelist[ store_id][itempos].item_no )
		{
			break;
		}
	}
	if( itempos == storelistmax[store_id] ) return;		//�ŷ� �Ұ� ����

	if(::GiveMoney(ch,GetItemValueSell( item )))//021126 lsw// ���� �÷� �ش�.
	{//�� ���� �� ���� ��Ȳ�̸� �ǸŰ� �ȵȴ�
		if(item->attr[IATTR_RARE_MAIN])
		{
			const unsigned int uiRare = item->attr[IATTR_RARE_MAIN];
			Send_RareItemMakeLog(cn,item->item_no,-2,
			((LPRareMain)(&uiRare))->grade,
			((LPRareMain)(&uiRare))->soksung1,
			((LPRareMain)(&uiRare))->soksung2,
			((LPRareMain)(&uiRare))->soksung3,
			0,0,0,0,item->attr[IATTR_LIMIT],item->attr[IATTR_MUCH],0,-2);
		}
		::SendItemEventLog( item, cn, SN_NOT_USER, SILT_PUT_STORE, 3 );	 //YGI acer
		::DeleteItem( item );
	}
}


#define MAX_RUMOR_TO_READ_		5 // �д����� ����� �ִ��. 
char RumorText[ MAX_RUMOR_TO_READ_][ TEXT_LENGTH];
void InitRumorText( char *filename )
{
	FILE *fp;
	char  text[ TEXT_LENGTH];
	int   c,cc,i;
	
	fp = fopen( filename, "rt" );
	if( fp == NULL ) return;
	
	c = 0;	
	while( fgets( text, TEXT_LENGTH, fp ))	c++;	
	
	fseek( fp, 0, SEEK_SET );
	for( cc=0, i=0 ; i < c ; i ++ )
	{
		fgets( text, TEXT_LENGTH, fp );
		if( i >= c-5 )
		{
			strcpy( RumorText[cc], text );
			cc++;
		}
	}
	fclose(fp);
}
	
void SendRumor( t_connection c[],  int cn )
{	
	static int count;
	t_packet p;

	p.h.header.type = CMD_MENU_MESSAGE;
	{
		p.u.server_menu_message.type = MM_RUMOR_;	
		strcpy( p.u.server_menu_message.text, RumorText[count] );
	}
	
	p.h.header.size = sizeof( t_server_menu_message ) - TEXT_LENGTH + strlen( RumorText[count] );
	QueuePacket( c, cn, &p, 1 );

	count++;
	count %= MAX_RUMOR_TO_READ_;
}	
	
	
	

//------------------------------------------------------------
/*/����...
	case MN_BLACKSMITH:			
	case MN_FARM	:	
	case MN_RESTAURANT:
	case MN_HERBSHOP:	
	case MN_SHAMBLES	:
	case MN_CANDLE	:
	case MN_SLOPSHOP	:
	case MN_ALCHEMY	:
	case MN_WOODWORKING:
	case MN_BOWCRAFT	:
	case MN_SHIPYARD	:
	

/********************************************************8
��� Menu 
#define CMD_GUILD_INFO					7000
typedef struct server_guild_info
{

}t_server_guild_info;

void SendGuildInfo( t_connection c[], int cn, int guildindex, int storeindex, int repairindex )

  int guildindex, :    
  int storeindex, : 
  int repairindex : 

  . ��ũ��Ʈ���� ����Ǿ�����. 

*********************************************************/
void SendGuildInfo( t_connection c[], int cn  )
{
	t_packet p;

	p.h.header.type = CMD_GUILD_INFO;
	{
	}
	p.h.header.size = sizeof( t_server_guild_info );

	QueuePacket( c, cn, &p, 1 );
}


//////////////////////////////////////////////////////////////////////////////////////
//
//	���� ������ �޼ҵ�
//
//////////////////////////////////////////////////////////////////////////////////////
#define MAX_STR_WRITER		30
#define BBS_STRING_MAX		8192
typedef struct tag_public_notice
{
	int		no;
	int		date;
	char	writer[MAX_STR_WRITER];
	char	title[TEXT_LENGTH];
	char	*text;
}PublicNotice, *lpPublicNotice;


void FreePublicNotice( lpPublicNotice p )
{
	p->no = 0;
	p->date = 0;
	strcpy(p->writer,"");
//	if( p->title )	free(p->title);
//	delete p->text;
}


PublicNotice public_notice[50];
void LoadPublicNotice( void )
{
	for( int i=0; i<50; i++ )		// ������ ���� ����
	{
		if( public_notice[i].no )
		{
			FreePublicNotice( &public_notice[i] );
		}
	}

	char buf[BBS_STRING_MAX];
	
	FILE	*fp;
	char	filename[50];

	sprintf(filename, ".\\data\\��������.txt");

	if( fp= fopen(filename, "rt") )
	{
		int i=0; 
		while( !feof(fp) )
		{
			int len;
			fgets(buf, BBS_STRING_MAX, fp);
			if( *buf == ';' ) continue;

			char *token;
			token = strtok(buf, "\t\n\r");
			if( token == NULL ) break;
			public_notice[i].no = atoi( token );

			token = strtok(NULL, "\t\n\r");
			if( token == NULL ) break;
			public_notice[i].date = atoi( token );

			token = strtok(NULL, "\t\n\r");
			if( token == NULL ) break;
			strcpy( public_notice[i].writer, token );

			token = strtok(NULL, "\t\n\r");
			len = strlen( token );
			if( token == NULL ) break;
//			public_notice[i].title = new char[len];
			strcpy( public_notice[i].title, token );
			
//			token = strtok(NULL, "\t\n\r");
//			len = strlen( token );
//			public_notice[i].text = new char[len];
//			strcpy( public_notice[i].text, token );

			i++;
		}
	}
	else 
	{
		MyLog( LOG_NORMAL, "File open error in Menuserver" );
		return;
	}

	fclose(fp);
}


void SendMessageTitle( int type, int count, t_connection c[], int cn )
{
//	char date[10][ FILENAME_MAX];
	char name[10][ FILENAME_MAX];
	char title[10][ FILENAME_MAX];
	int	 ct[10];
	TIMESTAMP_STRUCT date[10];
	
	t_packet p;
	p.h.header.type = type ;
	{
		switch( type )
		{
			case CMD_PUBLIC_NOTICE_TITLE_REQ:	{
													//PublicNotice title[10];
													int max = GetTitleBBS( count, ct, date, name, title );

													for( int i=0; i<max; i++ )
													{
//														if( !public_notice[i].no ) return;

														p.u.server_menu_message_title.no = ct[i];
														p.u.server_menu_message_title.date = date[i].year*10000 + date[i].month*100+date[i].day;
														strncpy( (char *)p.u.server_menu_message_title.name, name[i], NM_LENGTH-2 );
														strncpy( (char *)p.u.server_menu_message_title.title, title[i], NM_LENGTH-2 );
														p.h.header.size = sizeof( t_server_menu_message_title );
														QueuePacket( c, cn, &p, 1 );
													}
													break;
												}
			case CMD_RANK_TITLE_REQ			:
			case CMD_FORUM_TITLE_REQ		:		
			case CMD_EVENT_TITLE_REQ		:			
			case CMD_MYDIARY_TITLE_REQ		:
			case CMD_BBS_TITLE_REQ			:	
			case CMD_ETC_TITLE_REQ			:		;
		}
	}
}



void SendCharInfomation( int type, t_connection c[], int cn )
{
	t_packet p;
	memset( &p, 0, sizeof( t_server_user_db_data ) );
	p.h.header.type = type ;
	{
		PutPacketCharDB( c, cn, &p );
	}
	p.h.header.size = sizeof(t_server_user_db_data);

	QueuePacket( c, cn, &p, 1 );
}

void SendOtherInv( int type, int other_id, t_connection c[], int cn )		// pk ó����? 
{
	const int GET_ITEM_MAX = 1;

	t_packet p;

	CHARLIST *ch;

	if( other_id < 10000 ) ch = CheckServerId( other_id );
	else ch = CheckNpcId( other_id-10000 );
	if( !ch ) return; 

//soto-tai	for( int i=0; i<15; i++ )	p.u.kein.server_other_ch_inv.item[i] = 0;
	for( int i=0; i<15; i++ )
	{
		memset(&p.u.kein.server_other_ch_inv.item[i],0,sizeof(ItemAttr));
	}

//soto-tai	int item[100];
	ItemAttr	item[100];memset(item,0,sizeof(ItemAttr)*100);


	int ct = 0;
	p.h.header.type = type ;
	{
		for( int a=0; a<3; a++ )
			for( int b=0; b<3; b++ )
				for( int c=0; c<8; c++ )
				{
					if( ch->inv[a][b][c].item_no )
					{
						CItem *t = ItemUnit( ch->inv[a][b][c] );
						if( !t ) continue;
						if( t->GetLevel() >= 100 ) continue;		// ������ ���� �������� �������� ����..

//soto-tai				item[ct++] = ch->inv[a][b][c].item_no;
						item[ct++] = ch->inv[a][b][c];

						if( ct==100 ) goto ok__ ;
					}
				}
	}

	ok__ :

	int count = 0;	// ���������� �� ���ǵ� ī��Ʈ
	if( ct < GET_ITEM_MAX) 
	{
		for( int i=0; i<ct; i++ )
		{
			p.u.kein.server_other_ch_inv.item[i] = item[i];
			count++;
		}		
	}
	else
	{
		int item_count[GET_ITEM_MAX] = {0,};
		while( count < GET_ITEM_MAX )
		{
			int para = rand() % ct;
			int ret = 0;
			for( int i=0; i<count; i++ )
			{
				if( item_count[i] == para )
				{
					ret = 1;
					break;
				}
			}
			if( !ret ) item_count[count++] = para;
		}
		for( int i=0; i<count; i++ )
		{
			p.u.kein.server_other_ch_inv.item[i] = item[item_count[i]];
		}
	}
		p.u.kein.server_other_ch_inv.target_id = other_id;
		p.u.kein.server_other_ch_inv.item_count = count;
//soto-tai	p.h.header.size = sizeof(kein_server_other_ch_inv) - sizeof( short int )*(15-count);
	p.h.header.size = sizeof(kein_server_other_ch_inv) - sizeof( ItemAttr )*(15-count);

	QueuePacket( c, cn, &p, 1 );
}


int RecvDivideItem( kein_item_attr_pos *item, t_connection c[], int cn )
{
	CHARLIST *ch = &c[cn].chrlst;
	if( !ch ) return 0;

	POS &pos_s = item->item_pos;
	int &much = item->much;

	if( !much ) return OK;		// ���� ���� ���� �׳� ����~

	ItemAttr *source = GetItemByPOS( cn, pos_s );
	if( !source ) return NOT;
	ItemAttr temp = *source;			// ��������  ������ ��ȣ�� �Ӽ��� ������ �ִ´�.

	if( source->item_no != item->item_attr.item_no ) return NOT;
	
	if( source->attr[IATTR_MUCH] > MAX_MONEY ) source->attr[IATTR_MUCH] = MAX_MONEY;		// ������ �ִ� �������� 100000�� ���� ���Ѵ�.
	
	if( source->attr[IATTR_MUCH] > (DWORD) much )
	{
		source->attr[IATTR_MUCH] -= much;
	}
	else
	{
		much = source->attr[IATTR_MUCH];
		DeleteItem( source );
//		SendDeleteItem( pos_s, c, cn );		// ������ ���� ���� �Լ� 
	}

	int x, y, z;
	if( !SearchInv( ch->inv, x, y, z ) ) 
	{
		ItemAttr target = temp;
		target.attr[IATTR_MUCH] = much;
		if( target.item_no == IN_COINS && much <= 5 ) target.item_no = IN_COIN;
		else if( target.item_no == IN_COIN && much > 5 ) target.item_no = IN_COINS;

		if( target.item_no == IN_NEW_COINS && much <= 5 ) target.item_no = IN_NEW_COIN;
		else if( target.item_no == IN_NEW_COIN && much > 5 ) target.item_no = IN_NEW_COINS;

		SendItemEventLog( &target, ch->GetServerID(), ch->GetServerID(), SILT_PUT_GROUND_MAX_INVEN, 1 ); //YGI acer
		DropItem( ch->X, ch->Y, &target );			// ���ٴ����� ����߸���.
	}
	else
	{
		ItemAttr &target = ch->inv[x][y][z];
		target = temp;								// ������ ��ȣ�� �Ϲ� �Ӽ� �Ѱ� �ֱ� ����
		target.attr[IATTR_MUCH] = much;
		if( target.item_no == IN_COINS && much <= 5 ) target.item_no = IN_COIN;
		else if( target.item_no == IN_COIN && much > 5 ) target.item_no = IN_COINS;

		if( target.item_no == IN_NEW_COINS && much <= 5 ) target.item_no = IN_NEW_COIN;
		else if( target.item_no == IN_NEW_COIN && much > 5 ) target.item_no = IN_NEW_COINS;


		POS pos;
		SetItemPos( INV, x, y, z, &pos );
		SendServerEachItem( &pos, &target, cn );
	}

	//020303 lsw
/*	if( pos_s.type == BOX )
	{
		SendLoginPutBoxItem( ch->box_id, pos_s.p3, source );
	}
*/


	return 1;
}

void RecvBankInfoDeposit( DWORD dwMoney, t_connection c[], int cn )
{
	CHARLIST *ch = ::CheckServerId( cn );
	if( !ch ) 
	{
		return;
	}

	const DWORD dwBankMoney = ch->GetBankMoney();

	if(MAX_BANK_MONEY <= dwBankMoney)
	{//���ݾ��� 40���� �ִ� �Դϴ�.
		OutMessage(ch,2,930);
	}
	
	if(1==::SubtractMoney( dwMoney, ch )) //���۹��� �Է� ������ ���Ѵ�.
	{
		::SendSubtractMoney( dwMoney, cn );			// ���ŵ� ������ �����ش�.
		ch->IncBankMoney(dwMoney, BMCT_BANK_DEPOSIT); // CSD-030723
	}
}

void RecvBankInfoDefrayal( DWORD dwMoney, t_connection c[], int cn )
{
	CHARLIST	*ch = CheckServerId( cn );
	if( !ch ) return;

	if( ch->GetBankMoney() >= dwMoney )//���࿡ ���� �ִ°�
	{
		if(::GiveMoneyAble(ch,dwMoney))//���� ���� �� �ִ� ��Ȳ�ΰ�
		{
			ch->DecBankMoney(dwMoney, BMCT_BANK_DEFRAYAL); // CSD-030723
			GiveMoney(ch,dwMoney);
		}
	}
}


void SendBankRepayment( t_connection c[], int cn )
{
	CHARLIST	*ch = CheckServerId( cn );
	if( !ch ) return;
	t_packet p;

	p.h.header.type = REP_YOUR_DEPOSIT_INFO;
	{
		p.u.kein.client_bank_repayment.loan_money;
		p.u.kein.client_bank_repayment.last_loan;
		p.u.kein.client_bank_repayment.last_loan_time;
	}
	p.h.header.size = sizeof( k_client_bank_deposit );

	QueuePacket( c, cn, &p, 1 );
}

void RecvCharInfoBasic( t_char_info_basic *p, t_connection c[], int cn )
{
	if( p->kind < 0 ) return;
	if( p->password != 111 ) return; 

	CHARLIST *ch = &c[cn].chrlst;
	if( !ch ) return;

	if( strcmp( g_szPassWordName, ch->Name ) ) return;
	DWORD d = p->data;

	switch (p->kind)
	{	//< CSD-030723
	case STR:
		{
			ch->Str	= short(d);
			break;
		}
    case DEX:
		{
			ch->Dex	= short(d);
			break;
		}
	case CON:
		{
			ch->Con	= short(d);
			break;
		}
	case WIS:
		{
			ch->Wis	= short(d);
			break;
		}
	case INT_:
		{
			ch->Int	= short(d);
			break;
		}
	case MOVP:
		{
			ch->MoveP = short(d);
			break;
		}
	case CHA:
		{
			ch->Char = short(d);
			break;
		}
	case ENDU:
		{
			ch->Endu = short(d);
			break;
		}
	case MOR:
		{
			ch->Moral = short(d);
			break;
		}
	case LUCK:
		{
			ch->Luck = short(d);
			break;
		}
	case WSPS:
		{
			ch->wsps = short(d);
			break;
		}
	case LVPOINT:
		{
			ch->SetReservedPoint(d);
			break;
		}
	case MONEY:
		{
			ch->Money = d;
			break;
		}
	case HP:
		{
			ch->Hp = d;
			break;
		}
	case MAX_HP:
		{
			ch->HpMax = d;
			break;
		}
	case MP:
		{
			ch->Mana = d;
			break;
		}
	case MAX_MP:
		{
			ch->ManaMax	= d;
			break;
		}
	case SP:
        {
			ch->Hungry = d;
			break;	
		}
	case MAX_SP:
		{
			ch->HungryMax = d;
			break;
		}
	case BANKMONEY: 
		{
			ch->SetBankMoney(d, BMCT_RECV_CHAR_INFO_BASIC);
			break;
		}
	case TAC_E1:
	case TAC_E2:
	case TAC_E3:
	case TAC_E4:
	case TAC_E5:
	case TAC_E6:
	case TAC_E7:
	case TAC_E8:
	case TAC_E9:
	case TAC_E10:
	case TAC_E11:
	case TAC_E12:
	case TAC_E13:	  
		{
			ch->ConvertTacticsLevel(p->kind- TAC_E1);
			break;
		}
	}	//> CSD-030723
}

void SendStoreItemList( int cn, int storelistindex, int menu )
{
	if( storelistindex < 0 || storelistindex >= MAX_STORE_LIST ||
		storelistmax[storelistindex] < 0 || storelistmax[storelistindex] > MAX_STORE_ITEM_LIST ) return;
	t_packet p = {};
	int count = 0;
	p.h.header.type = CMD_STORE_ITEMLIST;
	{
		p.u.server_store_itemlist.smallmenu			= menu;
		p.u.server_store_itemlist.storelistindex	= storelistindex;
	
		for( int i = 0 ; i < storelistmax[ storelistindex] ; i ++)
		{
			if( storelist_attr[ storelistindex][ i] == SIT_EVENT_ITEM || 				
				storelist_attr[ storelistindex][ i] == SIT_NULL	) continue;

			switch( menu )
			{
				case MN_SKILLGUILD_REPAIR	:	break;
				case MN_SKILLGUILD_SELL		:	
				case MN_SHOP_SELL			:	if( storelist_attr[ storelistindex][ i] == SIT_ONLY_SELL ) continue;
													break;
				case MN_SKILLGUILD_BUY		:	
				case MN_SHOP_BUY			:	if( storelist_attr[ storelistindex][ i] == SIT_ONLY_BUY ) continue; 
												break;
				default : break;
			}
			p.u.server_store_itemlist.store_item[count++].item	= storelist[ storelistindex][ i].item_no;
		}
	}
	p.u.server_store_itemlist.no = count;
	p.h.header.size = sizeof( t_server_store_itemlist ) - sizeof( k_store_list_item )*(MAX_STORE_ITEM_LIST-count) ;
	QueuePacket(connections, cn, &p, 1);
}

void SendCallSmallMenu( int cn, int storelistindex, int menu )
{
	t_packet p;
	p.h.header.type = CMD_CALL_SMALLMENU;
	{
		p.u.server_call_smallmenu.menu = menu;
		p.u.server_call_smallmenu.storelistindex = storelistindex;
	}
	p.h.header.size = sizeof( t_server_call_smallmenu );
	QueuePacket(connections, cn, &p, 1);
}
//<soto-030421
void SendCallDualMagicLearnMenu( int cn)
{
	t_packet p;
	p.h.header.type = CMD_DUAL_MAGIC_LEARN_BOOK;
	p.h.header.size = 0;
	
	QueuePacket(connections, cn, &p, 1);		
}
//>soto-030421

//<soto-030429
void RecvCallLuckyMenuOpen(int cn)
{
	SendCallLuckyMenuOpen(cn);
}
void	SendCallLuckyMenuOpen(int cn)
{
	t_packet p;
	p.h.header.type = CMD_OPEN_LUCKY_MENU;
	p.h.header.size = 0;
	
	QueuePacket(connections, cn, &p, 1);
}



int LoadGuildItem( char *filename, int num )
{
	FILE *fp;
	fp = fopen( filename, "rt" );
	if( fp == NULL ) return 0;
	
	char buf[MAX_STRING];
	int i=0; 

	while (fgets(buf, sizeof(buf), fp))
	{
		if( *buf == ';' ) continue;

		char *token;

//		token = strtok(buf, "\t\n\r");		// number skip
//		if( token == NULL ) break;

		token = strtok(buf, "\t\n\r");
//		token = strtok(NULL, "\t\n\r");		// ������ ��ȣ
		if( token == NULL ) break;
		storelist[num][i].item_no = atoi( token )/100;
		storelist[num][i] = GenerateItem( storelist[num][i].item_no );		// ������ ����Ʈ �������� ����� ����.

		token = strtok(NULL, "\t\n\r");		// ������ �̸�		skip
		if( token == NULL ) break;

		token = strtok(NULL, "\t\n\r");		// �ȱ� ��� �Ӽ�
		if( token == NULL ) break;
		storelist_attr[num][i] = atoi( token );

		i++;		

		// 010522 KHS
		if( i >= MAX_STORE_ITEM_LIST ) 
		{
			MyLog( LOG_NORMAL, "**** Error :  %s ItemList Number is more than 'MAX_STORE_ITEM_LIST' ", filename );
			break;
		}
	}

	fclose( fp );

	SortingItem( storelist[num], storelist_attr[num], i );

	return i;
}

void swap_kein( void *a, void *b, void *t, int size )
{
	memcpy( t, a, size );
	memcpy( a, b, size );
	memcpy( b, t, size );
}

void SortingItem( ItemAttr *item_list, int *list_int, int &max )
{
	ItemAttr temp;	
	for( int i=0; i<max-1; i++ )
	{
		CItem *item_i = ItemUnit( item_list[i] );
		if( !item_i ) 
		{
			item_list[i] = item_list[max-1];
			list_int[i] = list_int[max-1];
			i=-1;
			max--;
			continue;
		}

		for( int j=i; j<max; j++ )
		{
			int type_i = item_list[i].item_no / 1000;
			int type_j = item_list[j].item_no / 1000;

			CItem *item_i = ItemUnit( item_list[i] );

			if( type_i > type_j ) 
			{
				swap_kein( &item_list[i], &item_list[j], &temp, sizeof( ItemAttr ) );
				swap( &list_int[i], &list_int[j] );
				continue;
			}

			if( type_i == type_j )
			{
				CItem *item_j = ItemUnit( item_list[j] );
				if( !item_j ) continue;
				
				//if( item_i->GetWearAble() > item_j->GetWearAble() )
				if( item_i->GetSkill_Ability() > item_j->GetSkill_Ability() )
				{
					swap_kein( &item_list[i], &item_list[j], &temp, sizeof( ItemAttr ) );
					swap( &list_int[i], &list_int[j] );
					continue;					
				}

				//if( item_i->GetWearAble() == item_j->GetWearAble() )
				if( item_i->GetSkill_Ability() == item_j->GetSkill_Ability() )
				{
					if( item_i->GetSkill_Ability() )
					{
						if( item_i->GetLevel() > item_j->GetLevel() )
						{
							swap_kein( &item_list[i], &item_list[j], &temp, sizeof( ItemAttr ) );
							swap( &list_int[i], &list_int[j] );
							continue;					
						}

						if( item_i->GetLevel() == item_j->GetLevel() )
						{
	//						if( list[i].item_no > list[j].item_no )
	//						{
	//							swap_kein( &list[i], &list[j], &temp, sizeof( ItemAttr ) );
	//							swap( &list_int[i], &list_int[j] );
	//							continue;					
	//						}
						}
					} 
				}
			}
		}
	}
}


void SendDecreaseItem( POS item_pos, DWORD dur, t_connection c[], int cn )	// ������ ���� ������ ���� �Լ�
{
	t_packet p;

	p.h.header.type = CMD_DECREASE_ITEM;
	{
		p.u.kein.server_decrease_item.dur = dur;
		p.u.kein.server_decrease_item.item_pos = item_pos;
	}
	p.h.header.size = sizeof( k_server_decrease_item ); 
	QueuePacket(connections, cn, &p, 1);
}
void RecvLvUpPointEach(unsigned char type, t_connection c[], int cn)
{
	t_packet	packet;
	CHARLIST	*ch;
	ch = &c[cn].chrlst;
	if( !ch ) return;

	if (ch->GetReservedPoint() > 0)
	{
    ch->IncAbility(type);
    ResetAbility(ch);//020303 lsw
	}
	else 
	{	
		packet.h.header.type = CMD_LEVELUP_POINT_NO;
		packet.h.header.size = 0;
		QueuePacket(connections, cn, &packet, 1);
	}
}

void RecvLvDownPointEach( unsigned char type, t_connection c[], int cn )
{
	return;
	t_packet	packet;
	CHARLIST	*ch;
	ch = &c[cn].chrlst;

	if( !ch ) return;

	if (ch->GetReservedPoint() > 0)
	{
		if (ch->GetReservedPoint() >= 10)		// �����ִ� ����������Ʈ�� 10�̻��̸� ����� �ȵȴ�.
		{
			return;
		}
		switch( type )
		{
			case STR:		ch->Str--;		break;
			case CON:		ch->Con--;		break;
			case DEX:		ch->Dex--;		break;
			case WIS:		ch->Wis--;		break;
			case INT_:		ch->Int--;		break;
			case MOVP:		ch->MoveP--;	break;
			case CHA:		ch->Char--;		break;
			case ENDU:		ch->Endu--;		break;
			case MOR:		ch->Moral--;	break;
			case LUCK:		ch->Luck--;		break;
			case WSPS:		ch->wsps--;		break;
			default : return;
		}

		ch->IncReservedPoint();
		ResetAbility(ch);//020303 lsw

		packet.h.header.type = CMD_LEVELDOWN_OK;
		{
			packet.u.kein.client_lvup_point_each.type = type;
		}
		packet.h.header.size = sizeof( client_lvup_point_each );
		QueuePacket(connections, cn, &packet, 1);
	}
	else 
	{	
		packet.h.header.type = CMD_LEVELUP_POINT_NO;
		packet.h.header.size = 0;
		QueuePacket(connections, cn, &packet, 1);
	}	
}

void RecvLearnItem( k_client_learn_item *i, t_connection c[], int cn )
{
	CHARLIST *ch = &c[cn].chrlst;
	if( !ch ) return;

	int type, num;
	getItemIndex( i->item_no, type, num );
	CItem *t = ItemUnit( type, num );
	if( !t ) return;

	int join_num = GetItemJoinNumber( i->item_no );		// join���̺������� �Ķ��Ÿ ��ȣ
	if( !item_join[join_num].item_id ) return;					// ���� ������
	int value = item_join[join_num].tech_price;
	
//	ch->Money = GetMoneyByItem( ch );
	if( ch->Money < (DWORD)value ) return;				// ���� ������.

	int ct = SkillMgr.FindLearnItem( ch, i->item_no ) ;
	if( ct == -1 ) return ;					// ���������� ��� 0�� �����Ѵ�.
	if( ct == MAX_LEARN_ITEM ) return;	// ��� �� ����� ��...

	if( !SkillMgr.CanLearnItem( ch, join_num ) ) return;		// ��� �ɷ��� �����ϴ�.

	ch->Item[ct] = i->item_no;			// ���� ���
	ch->ItemMax++;						// ����� �ִ� ������ �ִ�ġ ++
	SubtractMoney( value, ch );

	t_packet p;
	p.h.header.type = CMD_LEARN_ITEM;		// ok sign
	{
		p.u.kein.item_no.item_no = i->item_no;
	}
	p.h.header.size = sizeof( k_item_no );
	QueuePacket(c, cn, &p, 1);
}

void RecvItemRepair( t_client_item_sell *p, t_connection c[], int cn )
{
	CHARLIST *ch = &c[cn].chrlst;
	if( !ch ) return;
//	ch->Money = GetMoneyByItem( ch );

	int store_id = p->store_id;
	int pos = p->item_position;
	int x,y,z;
	int item_no, type;
	int price;
	int storeitemno;
	int i;

	x = ( pos & 0x0060 ) >> 5;
	y = ( pos & 0x0018 ) >> 3;
	z = ( pos & 0x0007 );
	ItemAttr &item= ch->inv[x][y][z];

	getItemIndex( item.item_no, type, item_no );
	CItem *t = ItemUnit( type, item_no );
	if( !t ) 
	{
		SendServerResult( CM_REPAIR, 3, cn );	// �������� ������ �Ѿ...
		return;
	}

	storeitemno = MAX_STORE_ITEM_LIST;
	for( i = 0 ; i < storeitemno ; i ++)
	{
		if( storelist[store_id][i].item_no == item.item_no ) 
		{
			break;		// ������ �� �������� ����ϴ� ��ǰ�ΰ�
		}
	}

	if( i == storeitemno || !t->GetRepairAble() )
	{
		SendServerResult( CM_REPAIR, 2, cn );	// ��� ��ǰ�� �ƴ�
		return;
	}

	WORD d_curr;
	WORD d_max;
	GetItemDuration( item, d_curr, d_max );

	if(!d_max) 
	{
		SendServerResult( CM_REPAIR, 4, cn );		// �������� �ϳ��� ����...
		return;
	}
	price = (int)( (float)t->GetValue()/d_max * ((d_max - d_curr))*DUR_RATIO +.5); // ���� ��� �� �Ǵ� 
	if(item.attr[IATTR_RARE_MAIN])// ����� ������ ���
	{
		const int iRepairPoint = (int)( (float)((d_max - d_curr))*RARE_DUR_RATIO +.5); // ���� ��� �� �Ǵ� 
		const int iRareGrade  = ((LPRareMain)(&item.attr[IATTR_RARE_MAIN]))->grade;
		price += (iRepairPoint/2) * ((iRareGrade*iRareGrade)+10) * 100; // ���� ��� �� �Ǵ� //020815 lsw
	}

	if( !price ) 
	{
		price = 1;
	}
	if(ch->Money < (DWORD) price ) 
	{
		SendServerResult( CM_REPAIR, 1, cn );		// ���� ����
		return;
	}
	SubtractMoney( price, ch );

	if(item.attr[IATTR_RARE_MAIN])// ����� �������� �ٸ���
	{
		d_max = (WORD)(d_curr+(d_max-d_curr)*RARE_DUR_RATIO);
	}
	else
	{
		d_max = (WORD)(d_curr+(d_max-d_curr)*DUR_RATIO);
	}
	
	DWORD d = MAKELONG(d_max, d_max);
	item.attr[IATTR_DURATION] = d;
	{
		t_packet packet;
		packet.h.header.type = CMD_ITEMREPAIR_OK;
		{
			packet.u.kein.server_item_repair_ok.item_pos=pos ;
			packet.u.kein.server_item_repair_ok.money= price;
			packet.u.kein.server_item_repair_ok.dur= d;			
		}
		packet.h.header.size = sizeof(k_server_item_repair_ok);
		QueuePacket(connections, cn, &packet, 1);
	}
}

void SendServerResult( BYTE type, BYTE result, int cn )		// � ����� �����ٶ�..
{
	t_packet packet;
	packet.h.header.type = CMD_RESULT;
	{
		packet.u.kein.server_result.type= type;
		packet.u.kein.server_result.result= result;
	}
	packet.h.header.size = sizeof(k_server_result);
	QueuePacket(connections, cn, &packet, 1);
}

void GetItemDuration( ItemAttr item, WORD &d_curr, WORD &d_max )
{
	d_curr = LOWORD( item.attr[IATTR_DURATION] );
	d_max = HIWORD( item.attr[IATTR_DURATION] );
}

void SendStatusOpen( t_connection c[], int cn )
{ //< CSD-020821
	CHARLIST *ch = &c[cn].chrlst;
	if (!ch) return;

	t_packet packet;
	packet.h.header.type = CMD_OPEN_STATUS_REQ;
	{
		packet.u.kein.server_status_open.killmon	= ch->killmon;
		packet.u.kein.server_status_open.ac			= ch->GetExtendAc();
		packet.u.kein.server_status_open.fame			= ch->fame;
		packet.u.kein.server_status_open.mov = ch->MoveP;
	}
	packet.h.header.size = sizeof(k_server_status_open);
	QueuePacket(connections, cn, &packet, 1);
} //> CSD-020821

void SendBankInventory( int count, t_connection c[], int cn )		//1225
{
	int i,j;
	
	i=count /3;
	j=count %3;

	t_packet packet;
	packet.h.header.type = CMD_SEND_BANK_INV;
	packet.h.header.size = sizeof(k_server_bank_item);

	packet.u.kein.server_bank_item.first = i;
	packet.u.kein.server_bank_item.second = j;
	memcpy( (unsigned char *)packet.u.kein.server_bank_item.inv, 
			(unsigned char *)c[ cn].chrlst.bank[i][j],	sizeof( ItemAttr ) * 6 );

	QueuePacket( c, cn, &packet, 1);
}


int IsSameCharOfParty( int type, char *name, CHARLIST *ch )		// ���� �̸��� �ִ��� �Ǵ�
{
	if( !ch ) return 0;
	int i;
	switch( type )
	{
		case PARTY :		for( i=0; i<6; i++ ) if( !strcmp( ch->party[i].Name, name ) ) return 1;
							break;
		case RELATION :		for( i=0; i<3; i++ ) if( !strcmp( ch->relation[i].Name, name ) ) return 1;
							break;
		case EMPLOYMENT :	for( i=0; i<6; i++ ) if( !strcmp( ch->employment[i].Name, name ) ) return 1;
							break;
	}
	return 0;
}

// ���踦 �α⿡ ������ ��������..
int IsCheckPartyLevel( int type, CHARLIST *target_ch, CHARLIST *ch )		
{
	if( !ch ) return 0;
	switch( type )
	{
		case PARTY:
		case RELATION:		
		case EMPLOYMENT:	return 1;
							
	}
	return 0;
}

void RecvDoParty( k_client_do_party *p, t_connection c[], int cn, int party_cn, int check_nation )
{
	CHARLIST *ch = &c[cn].chrlst;
	if( !ch ) return;

	if( !party_cn )
		party_cn = ExistHe( p->name );		// cn ���� �ִٸ�...

	if( party_cn == -1 ) return;
	
	CHARLIST *party_ch = &c[party_cn].chrlst;
	int ct = p->ct;

	if( IsSameCharOfParty( p->type, p->name, ch ) ) return;
	if( !IsCheckPartyLevel( p->type, party_ch, ch ) ) 
	{
		t_packet packet;
		packet.h.header.type = CMD_PARTY_NOT_BY_LEVEL;
			packet.u.kein.default_short_int = p->type;
		packet.h.header.size = sizeof(short int);
		QueuePacket( c, ch->GetServerID(), &packet, 1);
		return;
	}
	if( check_nation )
		if( !IsSameNation( party_ch, ch ) ) return;		// ���� �޶� ��Ƽ�� �ξ�� �� ���

	switch( p->type )				// ������ ���� �ٸ� ��Ƽ���� ������ �� ��Ƽ���� �ű�� �����Ѵ�.
	{
		case PARTY :		if( ch->party[ct].On ) 
							{
								for( int i=0; i<6; i++ )
								{
									if( !ch->party[i].On ) 
									{
										ch->party[i] = ch->party[ct];
										ch->party[i].On = true;
										break;
									}
								}
							}
							strcpy( ch->party[ct].Name, p->name ); 
							ch->party[ct].On = true;
							break;

		case RELATION :		if( *ch->relation[ct].Name ) 
							{
								for( int i=0; i<3; i++ )
								{
									if( !*ch->relation[i].Name ) 
									{
										strcpy( ch->relation[i].Name, ch->relation[ct].Name );
										break;
									}
								}
							}
							strcpy( ch->relation[ct].Name, p->name ); break;
		case EMPLOYMENT :	if( *ch->employment[ct].Name ) 
							{
								for( int i=0; i<6; i++ )
								{
									if( !*ch->employment[i].Name ) 
									{
										strcpy( ch->employment[i].Name, ch->employment[ct].Name );
										break;
									}
								}
							}
							strcpy( ch->employment[ct].Name, p->name ); break;
	}
	// �� ĳ������ ���� �����ֱ�
	int ret = SendPartyInfo( p->type, p->ct, c, ch->Name );
	if( ret == -1 )
	{
		switch( p->type )
		{
			case PARTY :		memset( &ch->party[p->ct], 0 , sizeof( CharacterParty ) ); break;
//			case RELATION :		
//			case EMPLOYMENT :		
		}

	}	
	else		// ������� ��Ƽ �ξ����� �˷��ش�.
	{
		t_packet packet;
		packet.h.header.type = CMD_DO_PARTY_OK;
			packet.u.kein.do_party_ok.type= p->type;
			strcpy( packet.u.kein.do_party_ok.name, ch->Name );
		packet.h.header.size = sizeof( k_do_party_ok );
		QueuePacket( c, party_ch->GetServerID(), &packet, 1);
	}
}

inline CharacterParty *GetPartyPointer( CHARLIST *ch, int type, int count )
{
	CharacterParty *target_party;
	switch( type )
	{
		case PARTY :		target_party = &ch->party[count]; break;
		case RELATION :		target_party = &ch->relation[count]; break;
		case EMPLOYMENT :	target_party = &ch->employment[count]; break;
		default : return NULL;
	}
	return target_party;
}

int SendPartyInfo( int type, int ct, t_connection c[], const char* szName )	// ��Ƽ ����, ��Ƽ �迭 ��ȣ		1�κи� ����
{
	CHARLIST *me = CheckServerName(szName);
	if( !me ) return 0;
	
	const int cn = me->GetServerID();	// kyo
	
	if( ct<0 && ct>=6 ) return 0;
	
	CharacterParty *target_party = GetPartyPointer( me, type, ct );
	if( !target_party ||  !target_party->Name[0] ) return 0; // �� ��ȣ�� ���� ��Ƽ���� ����.
	CHARLIST *pa = SearchIdByName(target_party->Name); // CSD-030320
	if( !pa ) return -1; 
	
	t_packet packet;
	memset( &packet, 0, sizeof( t_packet ) );
	//< CSD-030320	
	const int he_exist = ExistHe(pa->Name);
	const int sight = IsMySightInWhoById( he_exist, cn );
	//> CSD-030320
	packet.h.header.type = CMD_SERVER_PARTY;
	{
		packet.u.kein.party_info.type		= (short) type;
		packet.u.kein.party_info.index		= (char) ct;
		
		strcpy( packet.u.kein.party_info.m_Name, target_party->Name);
		packet.u.kein.party_info.m_Gender	= (char) pa->Gender;
		packet.u.kein.party_info.m_ImageNo	= (short) pa->Face;
		packet.u.kein.party_info.m_Class	= (char) pa->Class;
		if( sight && CompPartyBoth( me, pa ) )		// ���þ߿� ������ ���� ������ �� ������ �����ش�. ���� ������ ���� ����..
		{
			packet.u.kein.party_info.m_Lv = pa->GetLevel(); // CSD-030806
			packet.u.kein.party_info.m_Str = pa->Str;
		}
		packet.u.kein.party_info.m_Join		= he_exist?1:0;
	}
	packet.h.header.size = sizeof(k_party_info);
	QueuePacket(connections, cn, &packet, 1);
	
	switch( type )
	{
	case PARTY : {
		for( int j=0; j<6; j++ )
		{
			if( !strcmp( me->Name, pa->party[j].Name ) ) 
			{
				SendPartyJoinOk( target_party->Name, cn );		// ���� �޽����� ���� �𿡰� ok�� �����ְ�
				SendPartyJoinOk( me->Name, pa->GetServerID());	// ������� �׿� ������ ������ ok�� �����ش�.
				//pa->party[j].Server_id = cn;
				break;
			}
		}
		break;
				 }
		
	}
	return 1;
}

void RecvDeleteParty( k_client_delete_party *p, t_connection c[], int cn )
{
	CHARLIST *ch = CheckServerId( cn );
	if( !ch ) return;

	CharacterParty *target;
	switch( p->type )
	{
		case PARTY : target = &ch->party[p->ct]; break;
		case RELATION :	target = &ch->relation[p->ct]; break;
		case EMPLOYMENT : target = &ch->employment[p->ct]; break;
		default : return;
	}
	//CHARLIST *party_ch = CheckServerId( target->Server_id );
	CHARLIST *party_ch = CheckServerName( target->Name );

	if( party_ch && strcmp( party_ch->Name, target->Name )==0 )			// �� ���� ���ӻ� �����ϴ� ���̶��..
	{
		if( IsSameCharOfParty( PARTY, ch->Name, party_ch ) )		// �׳����׵� 
		{
			//SendPacketDefault( CMD_PARTY_JOIN_RELEASE, ch->Name, strlen( ch->Name ), target->Server_id ); 
			CHARLIST *chTar = CheckServerName( target->Name );
			if( !chTar ) return;
			SendPacketDefault( CMD_PARTY_JOIN_RELEASE, ch->Name, strlen( ch->Name ), chTar->GetServerID()); // kyo
		}
	}
	memset( target, 0, sizeof( CharacterParty ));
}

void RecvLearnMagic( short int num, t_connection c[], int cn )
{
	CHARLIST	*ch = &c[cn].chrlst;
	if( !ch ) return;

	if( ch->Spell == WIZARD_SPELL )
	{
		if( ch->Ws[num] )	
		{
			SendServerResult( CM_LEARN_MAGIC, 2, cn );	// �̹� �����.
			return;
		}
	}
	else if( ch->Spell == PRIEST_SPELL )
	{
		if( ch->Ps[num] )	
		{
			SendServerResult( CM_LEARN_MAGIC, 2, cn );	// �̹� �����.
			return;
		}
	}

//	SendServerResult( CM_LEARN_MAGIC, 5, cn );	// ���� ������
	int ret = CanLearnMagic( num, ch );

	switch( ret )
	{
		case 1  :	SendLearnMagicOk( num, cn );	break;
		case 3  :	SendServerResult( CM_LEARN_MAGIC, 3, cn );	return ;// �ɷ��� ������
		case 4  :	SendServerResult( CM_LEARN_MAGIC, 4, cn );	return ;// �׷� ���� ����
		default :	return;
	}

	if( ch->Spell == WIZARD_SPELL )
	{
		SubtractMoney( Magic_Ref[num].Price, ch );
		ch->Ws[num] = true;
	}
	else ch->Ps[num] = true;
}

int	CanLearnMagic( int num, CHARLIST *ch )		// �� ������ ���� �ֳ�?
{
	if( !ch ) return false;
//	return true;			// �ӽ�
	int sptype=0;
	int sp;
	unsigned char *spell;
	if( ch->Spell )			// priest
	{
		sptype = 150;
		spell = ch->Ws;
		sp = TACTICS_Orison;
	}
	else 
	{
		spell = ch->Ps;
		sp = TACTICS_Magery;
	}

	int magic_num = num+sptype;
	if( magic_num == -1 ) return 4;
  int Wis  = ch->GetAbility(WIS);
	int Int  = ch->GetAbility(INT_);
	int wsps = ch->GetAbility(WSPS);
	if( Magic_Ref[magic_num].basic_magery> ch->Skill[sp]+1 )	return 3;			// ��������� ����Ѱ�?
	if( Magic_Ref[magic_num].require_WP > Wis ) return 3;		// �䱸 ����
	if( Magic_Ref[magic_num].require_IT > Int ) return 3;		// �䱸 ����
	if( Magic_Ref[magic_num].exhaust_MS > wsps ) return 3;		// �䱸 WSPS
	if( Magic_Ref[magic_num].point_WS > wsps ) return 3;		// �䱸 WSPS
	if( (DWORD) Magic_Ref[magic_num].Price > ch->Money ) return 3;		// �䱸 WSPS


	return 1;	
}

int GetMagicNumBySerial( int serial )
{
	for( int i=0; i<Num_Of_Magic; i++ )
	{
		if( Magic_Ref[i].serial_Num == serial ) return i;
	}
	
	return -1;
}

void SendLearnMagicOk( int num , int cn)
{
	t_packet packet;
	packet.h.header.type = CMD_LEARN_MAGIC_OK;
	{
		packet.u.kein.learn_magic_ok.magic_num = num;
	}
	packet.h.header.size = sizeof(k_learn_magic_ok);
	QueuePacket(connections, cn, &packet, 1);
}

//
//	���� ����, �������� Ŭ���̾�Ʈ ����
//
void SendPreventingOk( int num , int cn)
{
	t_packet packet;
	packet.h.header.type = CMD_PREVENTING;
	{
		packet.u.kein.client_disease.disease_num = num;
	}
	packet.h.header.size = sizeof(k_client_disease);
	QueuePacket(connections, cn, &packet, 1);
}

void RecvPreventing( short int num, t_connection c[], int cn )
{
	CHARLIST *ch = &c[cn].chrlst;
	if( !ch ) return;
//	ch->Money = GetMoneyByItem( ch );

	int money=100;
	money = disease_tbl[num].price_prevent;	// �����

	if( num < 0 || num > 5 ) return;
	if( ch->Money < (DWORD)money ) 
	{
		SendServerResult( CM_PREVENTING, 2, cn );			// ���� ������...
		ch->SendCharInfoBasic(MONEY, ch->Money );			// ���ŵ� ������ �����ش�.//020704 lsw
		return;
	}
	
// 010531 KHS
//	ch->nut1 = 100;
//	ch->nut2 = 100;
//	ch->nut3 = 100;
	
	SubtractMoney( money, ch );

	SendPreventingOk( num, cn );	// ��������.
}

//
//	���� ġ��
//
void SendCureDiseaseOk( short int num, int cn )
{
	t_packet packet;
	packet.h.header.type = CMD_CURE_DISEASE;
	{
		packet.u.kein.client_disease.disease_num = num;
	}
	packet.h.header.size = sizeof(k_client_disease);
	QueuePacket(connections, cn, &packet, 1);
}

void RecvCureDisease( short int num, t_connection c[], int cn )
{
	CHARLIST *ch = &c[cn].chrlst;
	if( !ch ) return;
//	ch->Money = GetMoneyByItem( ch );

	int money=100;

	if( num < 0 || num > 5 ) return;
	money = disease_tbl[num].price_heal;
	if( ch->Money < (DWORD)money ) 
	{
		SendServerResult( CM_CURE_DISEASE, 2, cn );
		return;
	}
	SubtractMoney( money, ch );
	SendCureDiseaseOk( num, cn );
}

//
// ������ ���� ���� ���� �޼ҵ�� 		
//
void SendItemValue( int item_no, int cn )
{
	int type, num;
	getItemIndex( item_no, type, num );

	CItem *item = ItemUnit( type, num );
	if( !item ) return;

	t_packet packet;
	packet.h.header.type = CMD_REQ_ITEM_VALUE;
	{
		packet.u.kein.server_item_money.item_no = item_no ;
		packet.u.kein.server_item_money.money = item->GetValue();
	}
	packet.h.header.size = sizeof(k_server_item_money);
	QueuePacket(connections, cn, &packet, 1);
}

void SendItemValue_Sell( POS *pos, int cn )
{
	CHARLIST *ch = CheckServerId( cn );
	if( !ch ) return;

	ItemAttr *item = GetItemByPOS( cn, *pos );
	if( !item ) return ;

	DWORD price_sell = GetItemValueSell( item );
	if( price_sell == 0 ) 
		SendDeleteItem( item, pos, ch );

	t_packet packet;
	packet.h.header.type = CMD_REQ_ITEM_VALUE_SELL;
	{
		packet.u.kein.server_item_money.item_no = item->item_no ;
		packet.u.kein.server_item_money.money = price_sell;
	}
	packet.h.header.size = sizeof(k_server_item_money);
	QueuePacket(connections, cn, &packet, 1);
}

void SendItemValue_Repair( int item_no, int cn )
{
	int type, num;
	getItemIndex( item_no, type, num );

	CItem *item = ItemUnit( type, num );
	if( !item ) return;

	t_packet packet;
	packet.h.header.type = CMD_REQ_ITEM_VALUE_REPAIR;
	{
		packet.u.kein.server_item_money.item_no = item_no ;
//		packet.u.kein.server_item_money.money = ;
	}
	packet.h.header.size = sizeof(k_server_item_money);
	QueuePacket(connections, cn, &packet, 1);
}

CHARLIST *SearchIdByName(const char* pName)
{	//< CSD-030320
	return g_pUserManager->GetCharList(pName);
}	//> CSD-030320


void RecvClientInn( char type, int cn )
{
	CHARLIST	*ch = CheckServerId( cn );
	if( !ch ) return;
//	ch->Money = GetMoneyByItem( ch );

	switch( type )
	{
		case 0: return;  // ����..
		
		// ����..
		case INN_SINGLE :	if( ch->Money < 3 ) 
							{
								SendServerResult( CM_INN_REST, 2, cn );		 // ���� �����մϴ�.
								return;
							}
							ch->Status = CS_REST;	
							SubtractMoney( 3, ch );
							break;
							
		case INN_DOUBLE :	ch->Status = CS_REST;	break;
		case INN_MULTI  :	ch->Status = CS_REST;	break;

		// ���ڱ�
		case INN_SINGLE+1 :	ch->Status = CS_SLEEP;	break;
		case INN_DOUBLE+1 :	ch->Status = CS_SLEEP;	break;
		case INN_MULTI+1  :	ch->Status = CS_SLEEP;	break;
	}
}

int GetItemValueSell( ItemAttr *item )//020925 lsw float �� �ٲپ� ������ �ϰԲ� �߽��ϴ�.
{
	int type;
	int num; 
	getItemIndex( item->item_no, type, num );

	CItem *t = ItemUnit( type , num );
	if( t ) 
	{
		int price = 0;
		int base = t->GetValue();
		int much;

		if( type == DISPOSABLE ) 
		{
			much = t->GetItemQuantity();	// dispoable�� ���
			if( !much ) much = 1;
			price = (int)(((float)base * .9+.5)*(((float)item->attr[IATTR_MUCH])/((float)much)));
			return price;
		}
		else if( t->GetRbutton()==DIVIDE_ITEM )		// ������ �ǹ�...
		{
			much = t->GetItemDuration();		// 10�� ����
			if( !much ) much = 1;
			price = (int)(((float)base * .9+.5)*(((float)item->attr[IATTR_MUCH])/((float)much)));
			return price;
		}

		WORD d_curr, d_max;

		GetItemDuration( *item, d_curr, d_max );
		WORD o_d_max = t->GetDuration();
		if( d_max < o_d_max ) d_max = o_d_max;

		if( d_max )
		{	
			if( (item->attr[IATTR_LIMIT]-g_curr_time) > 0 )		// ��������� ���� �־�� �Ѵ�.
			{
				price = (int)( (float)(base*.9)*(float)d_curr/(float)d_max+.5 );
				return price;
			}
			// ��������� �־� �Ѵ�.
			price = (int)( (float)(base*.9)*(float)d_curr/(float)d_max+.5 );		// �ӽ� 
			return price;
		}
	}
	return 0;
}


ItemAttr *GetItemByPOS( int cn, POS item )
{
	CHARLIST *ch = CheckServerId( cn );
	if( !ch ) return NULL;
	return GetItemByPOS( ch, item );
}

ItemAttr *GetItemByPOS( CHARLIST *ch, POS item )
{//021030 lsw
	switch( item.type )
	{
		case	HAND	:	return &ch->handheld;
		case	INV		:	return &ch->inv[item.p1][item.p2][item.p3];
		case	EQUIP	:	return &ch->equip[item.p3];
		case	QUICK	:	return &ch->quick[item.p3];
		case	BANK	:	return &ch->bank[item.p1][item.p2][item.p3];
		case	BOX		:
		case	MAKE	:	
		case	AUCTION_DB :
		case	OTHER_CH:	break;
	}
	return NULL;
}


extern void SendCMD_REQUEST_CHECK_WARNO(const int cn);	// 010915 LTS//020903 lsw

void SendServerWeakUp( int cn )
{
	CHARLIST *ch = CheckServerId( cn );
	if( !ch ) return;
	ch->Status = 0;
	ch->can_memorize_time = g_curr_time;

	t_packet packet;
	packet.h.header.type = CMD_SERVER_WAKE_UP;
	packet.h.header.size = sizeof(0);
	QueuePacket(connections, cn, &packet, 1);
}

void SendJoinGameWho( int cn )		// cn�̶� ���� ���� ������ ��� �α��μ����� �� ��Ƽ���� �ִ����� ��û�ϰ�...
{
	CHARLIST *ch = CheckServerId( cn );
	if( !ch ) return;

	for( int a=0; MAX_PARTY_MEMBER > a ; a++ )
	{
		if(ch->party[a].Name[0])	
		{
			ReqPartyDB( cn, a, ch->party[a].Name );		// ���� �ξ��� ��Ƽ ������ �ϴ� �α��� ������ ��û�Ѵ�.
		}
	}
	SendCMD_REQUEST_CHECK_WARNO(cn);// 010915 LTS
}

CHARLIST *CheckServerIdAndName( int cn, char *name )
{
	CHARLIST *ch = CheckServerId( cn );
	if( ch && strcmp(ch->Name, name )==0 )
		return ch;

	return NULL;
}

void SendExitGameWho( int cn )			// ���� ���� �ΰ� �ִ� ��� ��θ� ã�Ƽ� ������� �ϳ� �ƴ� ���� �ΰ� �ִ� ��� 6������ �����ֳ�?
{	//< CSD-030506
	CHARLIST *ch = &connections[cn].chrlst;
	
	if (EXCHANGE_STATE_READY != ch->GetExchangeState())
	{	// �ŷ����̴��� ��� ��Ų��.
		SendExchangeCancel(ch->GetExchangeTargetId(), cn);
	}
	//< CSD-HK-030828
	g_pUserManager->DelMember(cn); 
	g_pDualManager->DelMember(cn);

	const int nGuildCode = ch->GetGuildCode();

	if (nGuildCode > 0)
	{	
		g_pGuildManager->DelMember(nGuildCode, cn);
	}

	if (g_pArenaManager->IsColossusArena())
	{
		g_pArenaManager->Remove(cn);
	}
	//> CSD-HK-030828
	for( int i=0; i<6; i++ )
	{
		if( !ch->party[i].On ) continue;
		//CHARLIST *gammer = CheckServerIdAndName( ch->party[i].Server_id, ch->party[i].Name ); 
		CHARLIST *gammer = CheckServerName( ch->party[i].Name ); // kyo
		if( gammer )
		{
			for( int j=0; j<6; j++ )
			{
				if( gammer->party[j].On && !strcmp(gammer->party[j].Name, ch->Name) )	// �׻���� ���� ������ ���� ����̶��...
				{
					//gammer->party[j].Server_id = 0;
					t_packet packet;
					packet.h.header.type = CMD_SERVER_EXIT_GAME;
					{
						strcpy( packet.u.kein.client_char_name.name, ch->Name );
					}
					packet.h.header.size = sizeof( k_client_char_name );
					QueuePacket(connections, gammer->GetServerID(), &packet, 1);
					break;
				}
			}
		}
	}
}	//> CSD-030506

void SendMyPartyExist(int cn)			// ���ε��� ĳ���Ϳ��� ��Ƽ���� �ִ� ���� �����ش�.
{	//< CSD-030415
	CHARLIST* ch = CheckServerId(cn);
	
	if (!ch)
	{
		return;
	}

	for	(int i = 0; i < MAX_PARTY_MEMBER; ++i)
	{
		if (ch->party[i].On)
		{
			CHARLIST* pParty = CheckServerName(ch->party[i].Name);

			if (pParty == NULL)
			{
				continue;
			}
				
			t_packet packet;
			packet.h.header.type = CMD_REQ_MY_PARTY;
			packet.h.header.size = sizeof(k_client_char_name);
			strcpy(packet.u.kein.client_char_name.name, ch->party[i].Name);
			QueuePacket(connections, cn, &packet, 1);
		}
	}

	SendPartyJoinOkSet( cn );		// ��Ƽ���� ���� �����ؼ� �����ش�.
}	//> CSD-030415

void SendPartyJoinOk( char *name, int cn )
{
	t_packet p;

	p.h.header.type = CMD_PARTY_JOIN_OK;
	{
		strcpy(p.u.kein.client_char_name.name, name);
	}
	p.h.header.size = sizeof( k_client_char_name );
	QueuePacket( connections, cn, &p, 1 );
}

int ExistHe( char *name )//020903 lsw
{	//< CSD-030314
	return g_pUserManager->GetServerID(name);
}	//> CSD-030314

int ExistConnectingHe( char *name )
{	//< CSD-030314
	return -1;
}	//> CSD-030314


void RecvReleaseParty( char *name, int cn )
{
	CHARLIST *ch = CheckServerId( cn );
	if( !ch ) return;
	int party_id = ExistHe( name);
	if( party_id > 0 ) 
	{
		t_packet p;
		p.h.header.type = CMD_PARTY_JOIN_RELEASE;
		{
			strcpy(p.u.kein.client_char_name.name, ch->Name);
		}
		p.h.header.size = sizeof( k_client_char_name );
		QueuePacket( connections, party_id, &p, 1 );
	}
}
	

void SendPartyJoinOkSet( int cn )		// ������ ĳ������ ��Ƽ�� ��..
{
	CHARLIST *ch = CheckServerId( cn );
	if( !ch ) return;

	for( int i=0; i<6; i++)
	{
		if( !ch->party[i].On ) continue;

		int id = ExistHe( ch->party[i].Name );
		if( id>0 )	// ���� ������ ������
		{
			CHARLIST *pa = &connections[id].chrlst;
			if( !pa ) return;
			
			for( int j=0; j<6; j++ )
			{
				if( !strcmp( ch->Name, pa->party[j].Name ) ) 
				{
					SendPartyJoinOk( ch->party[i].Name, cn );		// ���� �޽����� ���� �𿡰� ok�� �����ְ�
					SendPartyJoinOk( ch->Name, pa->GetServerID());	// ������� �׿� ������ ������ ok�� �����ش�.
					break;
				}
			}
//			if( j==6 ) RecvReleaseParty( ch->party[i], cn );	// �� ���� ���� ������ �ΰ� ���� �ʴ�...
		}
//		else		// ���� ������ ���� �ʴٸ�..
//		{
//			char you_party[6][31];
//			get_BinaryData_Party( (UCHAR **)you_party, ch->party[i] );

//			for( int j=0; j<6; j++ )
//			{
//				if( !strcmp( ch->Name, you_party[j] ) ) 
//				{
//					SendPartyJoinOk( ch->party[i], cn );
//					break;
//				}
//			}
//			if( j== 6) RecvReleaseParty( ch->party[i], cn );
//		}
	}
}

void SendServerEachItem( const ItemAttr *item, const CHARLIST *ch )
{
	if( !item ) return;

	for( int a=0; a<3; a++ )
		for( int b=0; b<3; b++ )
			for( int c=0; c<8; c++ )
			{
				if( &ch->inv[a][b][c] == item )
				{
					POS pos;
					SetItemPos( INV, a, b, c, &pos );
					SendServerEachItem( &pos, item, ch->GetServerID());
					return;
				}
			}
}

void SendServerEachItem( const POS *pos, const ItemAttr *item, const int cn )			// ���ڸ��� �� �� ���� �����ش�.
{
	CHARLIST *ch = CheckServerId( cn );
	if( !ch ) return;

	t_packet packet;
	packet.h.header.type = CMD_SERVER_EACH_ITEM;
	{
		packet.u.kein.server_item.item_pos  = *pos;
		packet.u.kein.server_item.item_attr = *item;
	}
	packet.h.header.size = sizeof( K_ITEM );
	QueuePacket(connections, cn, &packet, 1);
}

void SendAbilityOpen( const int cn )
{
	CHARLIST *ch = ::CheckServerId(cn);
	if( !ch ) 
	{
		return;
	}
	t_packet packet;
	packet.h.header.type = CMD_OPEN_ABILITY;
	{ 
		packet.u.kein.server_resist.naBasicResist[RT_POISON] = ch->GetBasicResist(RT_POISON);
		packet.u.kein.server_resist.naBasicResist[RT_CURSE] = ch->GetBasicResist(RT_CURSE);
		packet.u.kein.server_resist.naBasicResist[RT_HOLY] = ch->GetBasicResist(RT_HOLY);
		packet.u.kein.server_resist.naBasicResist[RT_FIRE] = ch->GetBasicResist(RT_FIRE);
		packet.u.kein.server_resist.naBasicResist[RT_ICE] = ch->GetBasicResist(RT_ICE);
		packet.u.kein.server_resist.naBasicResist[RT_ELECT] = ch->GetBasicResist(RT_ELECT);

		packet.u.kein.server_resist.naExtentionResist[RT_POISON] = ch->GetExtPoisonByItem();//�����ۿ� ���� �������� ��������
		packet.u.kein.server_resist.naExtentionResist[RT_CURSE]	= ch->GetExtCurseByItem() ;
		packet.u.kein.server_resist.naExtentionResist[RT_HOLY]	= ch->GetExtHolyByItem() ;
		packet.u.kein.server_resist.naExtentionResist[RT_FIRE]	= ch->GetExtFireByItem() ;
		packet.u.kein.server_resist.naExtentionResist[RT_ICE]	= ch->GetExtIceByItem() ;
		packet.u.kein.server_resist.naExtentionResist[RT_ELECT]	= ch->GetExtElectByItem(); 
	} 
	packet.h.header.size = sizeof(k_server_resist);
	QueuePacket(connections, cn, &packet, 1);
}

void SendSubtractMoney( DWORD money, int cn )
{
	return;
	t_packet packet;
	packet.h.header.type = CMD_SUBTRACT_MONEY;
	{
		packet.u.kein.server_money.money = money;	
	}
	packet.h.header.size = sizeof(k_server_money);
	QueuePacket(connections, cn, &packet, 1);
}

void SendAddMoney( DWORD money, int cn )
{
	t_packet packet;
	packet.h.header.type = CMD_ADD_MONEY;
	{
		packet.u.kein.server_money.money = money;	
	}
	packet.h.header.size = sizeof(k_server_money);
	QueuePacket(connections, cn, &packet, 1);
}

//////////////////////////////////////////////////////////////
//
// cheat command		  // learn magic, learn skill
//
////////////////////////////////////////////////////////////
void RecvLearnAll( int cn ) 
{
	CHARLIST *ch = CheckServerId( cn );
	if( !ch ) return;
	for( int i=0; i<200; i++ )
	{
		ch->Skill[i] = true;
		ch->Ws[i] = true;
		ch->Ps[i] = true;
	}
}

void RecvLearnAllNot( int cn )
{
	CHARLIST *ch = CheckServerId( cn );
	if( !ch ) return;
	for( int i=0; i<200; i++ )
	{
		ch->Skill[i] = false;
		ch->Ws[i] = false;
		ch->Ps[i] = false;
	}
}

///////////////////////////////////////////////////////////////
//	ĳ���� ó�� ���������� �԰� �ִ� ������...�� �������� ó��
int CheckCurrStats( CHARLIST *ch )		// 010521 YGI
{
	ch->Money = GetMoneyByItem( ch );
	ch->itemweight		= GetTotalItemWeight( ch );	
	ch->Tactics_para	= SetTactics( ch->Spell, ch->Gender, ch->Tactics );

	if (ch->IsDead())
	{	
		ch->bAlive		= DEAD_; 
		ch->SetState(CON_DEATH);
		ch->viewtype	= VIEWTYPE_GHOST_;
	}	
	else 
	{	
		ch->bAlive 		= ALIVE_;
		ch->SetState(CON_NORMAL);
		ch->viewtype	= VIEWTYPE_NORMAL_;
	}

	EquipItem( ch );
	ch->pkid = -1; // ���߿� ��ġ������ �ٲ�...... �ּҿ� PK�� �ϸ� ���� ID�� ������. ...
	ch->can_memorize_time = g_curr_time;		// �޸������ �� �� �ִ� �ð� ����
	return 1;
}

void SendServerItemAttr( short int cn, int type, POS pos, ItemAttr *item )
{	
	short int header_type;
	switch( type )
	{
		case IATTR_DURATION	:		header_type=CMD_IATTR_DURATION	; break;
		case IATTR_LIMIT	:		header_type=CMD_IATTR_LIMIT		; break;
		case IATTR_ATTR		:		header_type=CMD_IATTR_ATTR		; break;
		case IATTR_ID		:		header_type=CMD_IATTR_ID		; break;
		case IATTR_RESERVED0:		header_type=CMD_IATTR_RESERVED0	; break;
		case IATTR_RESERVED1:		header_type=CMD_IATTR_RESERVED1	; break;

		default : return;
	}

	t_packet packet;
	packet.h.header.type = header_type;
	{
		packet.u.kein.server_item_attr.pos = pos;
		packet.u.kein.server_item_attr.attr = item->attr[type];
	}
	packet.h.header.size = sizeof( k_server_item_attr );
	QueuePacket(connections, cn, &packet, 1);
}	

// 020428 YGI acer
// ������ ���� ���� �Լ� // �������� �෹�̼��� ���ο��� �μ������ type�� �ٲ۴�.
int SendDeleteItem( ItemAttr *pItem, POS *pos, CHARLIST *ch, bool broken )
{
	if( pItem )
	{
		if( pos->type == EQUIP )
		{	
			CItem *t = ItemUnit( *pItem );
			if( t ) t->ReleaseItem( ch );
		}
		if( pItem->attr[IATTR_RARE_MAIN])
		{
			const int nRare = pItem->attr[IATTR_RARE_MAIN];
			Send_RareItemMakeLog(ch->GetServerID(), pItem->item_no,-1,
			((LPRareMain)&nRare)->grade,((LPRareMain)&nRare)->soksung1,((LPRareMain)&nRare)->soksung2,
			((LPRareMain)&nRare)->soksung3,0,0,0,pItem->attr[IATTR_RARE_MAIN],pItem->attr[IATTR_LIMIT],0,-1,-1,0,0,0,0,0,0);			//020730 lsw
		}
	}

	t_packet p;
	int type;
	if( broken ) type = CMD_DELETE_ITEM_BY_DURATION;
	else type = CMD_DELETE_ITEM;
	
	p.h.header.type = type;
	{
		p.u.kein.server_delete_item.item_pos = *pos;
	}
	p.h.header.size = sizeof( k_server_delete_item ); 
	QueuePacket(connections, ch->GetServerID(), &p, 1);

	return DeleteItem( pItem );
}

void SendServerAc( int cn )
{ //< CSD-020821
	CHARLIST *ch = CheckServerId( cn );
	if (!ch) return;
  
  t_packet packet;
	packet.h.header.type = CMD_REQ_AC;
  packet.h.header.size = sizeof(k_server_ac);
	packet.u.kein.server_ac.ac = ch->GetExtendAc();
	QueuePacket(connections, cn, &packet, 1);
} //> CSD-020821

void SendServerDmg( int cn )
{
	CHARLIST *ch = CheckServerId( cn );
	if( !ch ) return;

	t_packet packet;
	packet.h.header.type = CMD_REQ_DMG;
  packet.h.header.size = sizeof(k_server_ac);
	packet.u.kein.default_int = ch->CalcPhysicalStrikingPower();
	QueuePacket(connections, cn, &packet, 1);
}

inline void CallClient( short int id, short int type )
{
	t_packet packet;
	packet.h.header.type = type;
	packet.h.header.size = 0;
	QueuePacket(connections, id, &packet, 1);
}

void SendPacketDefault( int type, void *msg, int size, short int cn )
{
	t_packet packet;
	packet.h.header.type = type;
	packet.h.header.size = 0;
	if( msg && size)
	{
		memcpy( packet.u.data, msg, size );
		packet.h.header.size = size;
	}
	QueuePacket(connections, cn, &packet, 1);
}

void SendServerKillMon( short int cn )
{
	CHARLIST *ch = CheckServerId( cn );
	if( !ch ) return;

	t_packet packet;
	packet.h.header.type = CMD_SERVER_KILLMON;
	{
		packet.u.kein.server_killmon.killmon =  ch->killmon;
	}
	packet.h.header.size = sizeof(k_server_killmon);
	QueuePacket(connections, cn, &packet, 1);
}



/// 0212 YGI /////////
int SetTactics( int spell_type, int sex, int tac_type )		// ĳ���� ����ũ�Ұ�� ������ ��ƽ���� ���� �迭÷�ڷ� �ٲٴ� ��ƾ..
{
	int tac_con[2][12] = { 
		{ CRAPPLE_, SWORD_, ARCHERY_, FENCING_, MACE_, PIERCE_, WHIRL_, HURL_, PARRYING_, MAGERY_, },		 // ����
		{ CRAPPLE_, SWORD_, ARCHERY_, PARRYING_, MACE_, PIERCE_, WHIRL_, HURL_, MAGERY_, D_SWORD_, D_MACE_ },	// ����
	};

	int ret = tac_con[sex][tac_type];
	if( spell_type == PRIEST_SPELL && ret == MAGERY_ ) ret = ORISON_;
	return ret;
}


/*
void LearnSkillOfJob( int inc_job, CHARLIST *ch)
{
	int skill_mother[32][2] = {	{1,3},   {3,5},   {5,8},   {8,10},  {10,16}, {16,19}, {19,21}, {21,25}, {25,28}, {28,32},
								{32,34}, {34,35}, {35,38}, {38,42}, {42,43}, {43,44}, {44,45}, {45,48}, {48,50}, {50,53},
								{53,55}, {55,56}, {56,57}, {57,64}, {64,67}, {67,73}, {73,75}, {75,76}, {76,78}, {78,86},
								{86,87}, {87,101}	};

	int min = skill_mother[inc_job][0];
	int max = skill_mother[inc_job][1];

	for( int i=0; i<max; i++ )	ch->Skill[i] = 1;
}
*/
/*
int SendPartyInfo2( short int type, char ct,  short int cn )
{
	CHARLIST *ch = CheckServerId( cn );
	if( !ch ) return -1;

	t_packet packet;
	packet.h.header.type = CMD_SERVER_PARTY;
	{
		packet.u.kein.party_info.type		= (short) type;
		packet.u.kein.party_info.index		= (char) ct;

		strcpy( packet.u.kein.party_info.m_Name, ch->party[ct].Name);
		packet.u.kein.party_info.m_Gender	= (char) ch->party[ct].Gender;
		packet.u.kein.party_info.m_ImageNo	= (short) ch->party[ct].Face;
		packet.u.kein.party_info.m_Lv		= (char) ch->party[ct].Level;
		packet.u.kein.party_info.m_Str		= ch->party[ct].Str;
		packet.u.kein.party_info.m_Class	= (char) ch->party[ct].Class;
		packet.u.kein.party_info.m_Join		= 1;
	}
	packet.h.header.size = sizeof(k_party_info);
	QueuePacket(connections, cn, &packet, 1);
	return 1;
}
*/
//int SendPartyInfo3( short int type, char ct,  short int cn )		// ��ó�� ��Ƽ������ ������ �����ִ� ��		// ����� ��Ƽ��
void SendPartyInfo3( short int type, char ct,  const char* szName)		// ��ó�� ��Ƽ������ ������ �����ִ� ��		// ����� ��Ƽ��
{
	//CHARLIST *ch = CheckServerId( cn );
	CHARLIST *ch = CheckServerName( szName );
	if( !ch ) return;
	int cn = ch->GetServerID();

	t_packet packet;
	memset( &packet, 0, sizeof( t_packet ) );
	packet.h.header.type = CMD_SERVER_PARTY;
	{
		packet.u.kein.party_info.type		= (short) type;
		packet.u.kein.party_info.index		= (char) ct;

		strcpy( packet.u.kein.party_info.m_Name, ch->party[ct].Name);
		packet.u.kein.party_info.m_Gender	= (char) ch->party[ct].Gender;
		packet.u.kein.party_info.m_ImageNo	= (short) ch->party[ct].Face;
		packet.u.kein.party_info.m_Class	= (char) ch->party[ct].Class;
	}
	packet.h.header.size = sizeof(k_party_info);
	QueuePacket(connections, cn, &packet, 1);
	
	//SendPartyInfo( type, ct, connections, cn );				// ���� ���Ͱ��� ���θ� �����ش�. 
	SendPartyInfo( type, ct, connections, szName );				// ���� ���Ͱ��� ���θ� �����ش�. 
	return;
}

void ReqPartyDB( short int client_id, char para, char *name )
{
	t_packet packet;
	packet.h.header.type = CMD_PARTY_ACCESS;
	{
		packet.u.kein.server_req_party_db.server_id = client_id;
		strcpy( packet.u.kein.server_req_party_db.name, name );
		packet.u.kein.server_req_party_db.ct = para;
	}
	packet.h.header.size = sizeof( k_server_req_party_db ) ;
	QueuePacket(connections, DB_DEMON, &packet, 1);
}

void RecvPartyInfoServerToServer( k_server_server_party_info *p )
{
	CHARLIST *ch = CheckServerId( p->Server_id );
	if( !ch ) return;

	if( !(*ch->party[p->ct].Name) ) return;		// ��Ƽ���� ���ٸ�

	ch->party[p->ct].Face =		( int ) p->Face;
	ch->party[p->ct].Level =	( int ) p->Level;
	ch->party[p->ct].Gender =	( int ) p->Gender;
	ch->party[p->ct].Str =		( int ) p->Str;
	ch->party[p->ct].Class =	( int ) p->Class;

	if( !strcmp( ch->party[p->ct].Name, ch->Name ) ) return;		// ���� ��Ƽ���� �ڱ� �ڽ��� ��� ���̻� ������ ���� �ʴ´�.

	//SendPartyInfo3( PARTY, p->ct, p->Server_id );	// ���� ��� �� �𿡰� ��Ƽ������ ������ �����ش�.
	SendPartyInfo3( PARTY, p->ct, ch->Name );	// ���� ��� �� �𿡰� ��Ƽ������ ������ �����ش�.

	// �׳� ���� ��Ƽ�� �ξ������� �˾ƺ���.		// �α��� ���� ��û
	SendReqPartyInfoOfOtherChar( ch->party[p->ct].Name, ch->Name, p->Server_id );
}


int EqualPosByPos( const POS *pos1, const POS *pos2 )
{
	if( pos1->type != pos2->type ) return 0;
	if( pos1->p1 != pos2->p1 ) return 0;
	if( pos1->p2 != pos2->p2 ) return 0;
	if( pos1->p3 != pos2->p3 ) return 0;
	return 1;
}


// �� �þ� �ȿ� �װ� �ִ� �� �������� �Ǵ�.	name->Ư�� �� ����̸�, cn->���� Ŭ���̾�Ʈ�� ���̵�
int IsMySightInWho( char *name, short int cn)		
{
	for(int i = 0; i < MAX_AREA_BUFFER; i ++)
	{
		if(connections[cn].area_tbl[i].bAlive == TRUE && connections[cn].area_tbl[i].Type == 1)
		{
			short int he = connections[cn].area_tbl[i].ID;
			CHARLIST *he_ch = CheckServerId(he);
			if( !he_ch ) continue;
			if( strcmp( he_ch->Name, name )==0 ) return 1;
		}
	}
	return 0;
}

int IsMySightInWhoById( short int check_id, short int cn )
{
	for(int i = 0; i < MAX_AREA_BUFFER; i ++)
	{
		if(connections[cn].area_tbl[i].bAlive && connections[cn].area_tbl[i].Type == 1)
		{
			short int he = connections[cn].area_tbl[i].ID;
			if( he == check_id ) return 1;
		}
	}
	return 0;
}

void SendReqPartyInfoOfOtherChar( char *party_name, char *my_name, short int server_id )
{
	t_packet packet;
	memset( &packet, 0, sizeof( t_packet ) );

	packet.h.header.type = CMD_REQ_PARTY_TOGETHER;
	{
		strcpy( packet.u.kein.server_req_party_together.party_name , party_name );
		strcpy( packet.u.kein.server_req_party_together.my_name, my_name );
		packet.u.kein.server_req_party_together.server_id = server_id;
	}
	packet.h.header.size = sizeof( k_server_req_party_together ) ;
	QueuePacket(connections, DB_DEMON, &packet, 1);
}

void RecvPartyTogetherByLoginServer( short int server_id, char *name )
{
	t_packet packet;
	packet.h.header.type = CMD_REQ_JOIN_GAME;
	{
		strcpy( packet.u.kein.client_char_name.name, name );
	}
	packet.h.header.size = sizeof( k_client_char_name );
	QueuePacket(connections, server_id, &packet, 1);

	CHARLIST *ch = &connections[server_id].chrlst;
	if( !ch ) return;
	for( int i=0; i<6; i++ ) 
		if( !strcmp( ch->party[i].Name, name ) ) 
		{
			//SendPartyInfo( PARTY, i, connections , server_id );	
			SendPartyInfo( PARTY, i, connections , name );
			return;
		}
}		
		


void RecvCharInfoPlus( int type, t_connection c[], int cn )		// ����� ��Ƽ�ϰ�츸...
{
	CHARLIST *ch = &c[cn].chrlst;
	if( !ch ) return;

	if( type == PARTY )
	{
		for( int i=0; i<6; i++ )
			if( ch->party[i].Name[0] )
			{
				ch->party[i].On = true;
				//SendPartyInfo( PARTY, i, c, cn );
				SendPartyInfo( PARTY, i, c, ch->party[i].Name );
			}
	}
}

int RecvNoParty(char *name, short int server_id)	//�� ���� �̸��� ������ ������... �����.
{	//< CSD-HK-030829
	CHARLIST* ch = CheckServerId(server_id);
	
	if (ch == NULL)
	{
		return 0;
	}
		
	for (int i = 0; i < MAX_PARTY_MEMBER; ++i)
	{
		if (strcmp( ch->party[i].Name, name) == 0) 
		{
			memset(&ch->party[i], 0, sizeof(CharacterParty));
			return 1;
		}
	}
		
	return 0;
}	//> CSD-HK-030829

void SendServerMagery( short int cn )			// Ŭ�����̾�Ʈ���� ��û�ϴ� ��ƽ���� �����ش�.
{
	CHARLIST *ch = CheckServerId( cn );
	if( !ch ) return;

	t_packet packet;
	packet.h.header.type = CMD_REQ_MAGERY;
	{
		if( ch->Spell == WIZARD_SPELL )
    {
      packet.u.kein.server_tactics.nStep = ch->GetTacticsStep(MAGERY_);
			packet.u.kein.server_tactics.magery = ch->tac_skillEXP[MAGERY_];
    }
		else if( ch->Spell == PRIEST_SPELL ) 
    {
      packet.u.kein.server_tactics.nStep = ch->GetTacticsStep(ORISON_);
			packet.u.kein.server_tactics.magery = ch->tac_skillEXP[ORISON_];
    }
	}
	packet.h.header.size = sizeof( k_server_tactics );
	QueuePacket(connections, cn, &packet, 1);	
}

void RecvBbsMessageReq( int type, char ct, short int cn )	// �������׵� ���� ����
{
	t_packet p;
	p.h.header.type = CMD_GET_SERVER_TEXT ;
	{
		switch( type )
		{
			case CMD_PUBLIC_NOTICE_BBS_REQ:	{
												;
												char *szTemp;
												szTemp = GetMessageBBS_public( ct );
												if( !szTemp )
												{
													szTemp = new char[100];
													sprintf( szTemp, "������ �����ϴ�." );
												}
												int len=strlen( szTemp );
												int max = (len / 512)+1;
												for( int i=0; i<max; i++ )
												{
													char *temp = szTemp+512*i;
													strncpy( p.u.kein.get_server_text.text, temp ,512 );
													p.u.kein.get_server_text.ct = ct;
													p.u.kein.get_server_text.len = len;
													
													p.h.header.size = sizeof( k_get_server_text );
													QueuePacket(connections, cn, &p, 1);
												}
												SAFE_DELETE_ARRAY(szTemp);
											}
			case CMD_RANK_BBS_REQ:
			case CMD_FORUM_BBS_REQ:		
			case CMD_EVENT_BBS_REQ:			
			case CMD_MYDIARY_BBS_REQ:
			case CMD_BBS_BBS_REQ:	
			case CMD_ETC_BBS_REQ:		;
		}
	}
}
/*	020303 lsw
void SendLoginPutBoxItem( DWORD box_id, int para, ItemAttr *lpItem )
{
	t_packet packet;

	/////////////////////// DB-DEMON ���� DB ���� ////////////////////////
	packet.h.header.type = CMD_PUT_BOX_ITEM;
	{
		packet.u.kein.login_put_box_item.box_id = box_id;
		packet.u.kein.login_put_box_item.para = (char )para;
		packet.u.kein.login_put_box_item.item = *lpItem;
	}
	packet.h.header.size = sizeof( login_put_box_item );
	QueuePacket(connections, DB_DEMON, &packet, 1);		// login_server_connect5`
	////////////////////////////////////////////////////////////////////


	/////////////////////// LOGIN-SERVER ///////////////////////////////
//	packet.h.header.type = CMD_RECALL_BOX_ITEM;
//	{
//		packet.u.kein.game2login_recall_box_item.box_id = box_id;
//	}
//	packet.h.header.size = sizeof( k_game2login_recall_box_item );
//	QueuePacket(connections, LOGIN_SERVER_CONNECT, &packet, 1);
	/////////////////////////////////////////////////////////////////////
}
*/

void SendLoginServerPreachName( k_server_preach_name *p, short int cn )		// �α��� ������ ������.
{
	t_packet packet;
	packet.h.header.type = CMD_LOGIN_PREACH_NAME;
	{
		packet.u.kein.loginserver_preach_name.server_id= cn;
		strcpy( packet.u.kein.loginserver_preach_name.name, p->name );
		packet.u.kein.loginserver_preach_name.believe_god= p->believe_god;
	}
	packet.h.header.size = sizeof( k_loginserver_preach_name );
	QueuePacket(connections, DB_DEMON, &packet, 1);
}

void RecvAndSendPreachName( k_loginserver_preach_name_result *p)	// ����� �޾Ƽ� Ŭ���̾�Ʈ�� ������.
{
	t_packet packet;
	packet.h.header.type = CMD_PREACH_NAME;
	{
		packet.u.kein.server_client_preach_name.believe_god	= p->believe_god;			// �ϴ� ��
		packet.u.kein.server_client_preach_name.result	= p->result;		// ��� 
		sprintf(packet.u.kein.server_client_preach_name.name, p->name);
	}
	packet.h.header.size = sizeof( k_server_client_preach_name );
	QueuePacket(connections, p->server_id, &packet, 1);
}

void RecvGodRegistCancel( short int cn )	// �İ�
{
	CHARLIST *ch = CheckServerId( cn );
	if( !cn || !ch ) return; 

	ch->believe_god = 0;
	ch->faith = 0;

	if( *ch->preach_name )		// �ڽ��� ������ ����� �ִٸ�
	{
		CHARLIST* preach_ch = SearchIdByName(ch->preach_name); // CSD-030320

		if( !preach_ch )		
		{
			// ���ӳ��� ���ٴ� ��, �α��� ������ �žӽ� ������� ����
			t_packet packet;
			packet.h.header.type = CMD_LOGIN_PREACH_NAME_CANCEL;
			{
				strcpy( packet.u.kein.server_preach_name.name, ch->preach_name );
				packet.u.kein.server_preach_name.believe_god= ch->believe_god;
			}
			packet.h.header.size = sizeof( k_server_preach_name );
			QueuePacket(connections, DB_DEMON, &packet, 1);
		}
		else
		{
			if( preach_ch->believe_god == ch->believe_god )		// ���� ������ ��ϵǾ� �־�� �Ѵ�.
			{
				preach_ch->faith -= 10;
				if( preach_ch->faith < 0 ) preach_ch->faith = 0;
				// �� ������� �����ؾ� �ϳ�?
			}
		}
		SendLoginEvangelistDelete( ch->Name );
	}

	ZeroMemory( ch->preach_name, sizeof( char[20] ) );
	return;
}

void RecvGodRegistOk( char believe_god, short int cn )		// ���
{
	CHARLIST *ch = CheckServerId( cn );
	if( !cn || !ch ) return; 

	if( !believe_god ) return;
	ch->believe_god = believe_god;
	ch->faith = 10;		//�ʱ� �ž� ��ġ
}

void SendLoginServePreachNameOk( k_server_preach_name *p, short int cn )		// �� �Լ��� ������ ������� �žӼ�ġ�� �÷��ִ� ��ƾ�̴�.
{
	CHARLIST *ch = CheckServerId( cn );
	if( !ch ) return;
	// �α��� ������ SQL�� ��û
	t_packet packet;
	
	memset(&packet, 0, sizeof( t_packet));
	packet.h.header.type = CMD_LOGIN_PREACH_NAME_OK;
	packet.h.header.size = sizeof( k_loginserver_preach_name );
	packet.u.kein.loginserver_preach_name.server_id = cn ;
	strcpy( packet.u.kein.loginserver_preach_name.name, p->name );
	packet.u.kein.loginserver_preach_name.believe_god= p->believe_god;
	QueuePacket(connections, DB_DEMON, &packet, 1);

	memset(&packet, 0, sizeof(t_packet));
	packet.h.header.type = CMD_REQ_FAITH_UP; // Ȥ�� ������ �ִ��� Ȯ���Ѵ�.
	packet.h.header.size = sizeof( k_req_faith_up );
	strcpy( packet.u.kein.req_faith_up.name,p->name ) ;
	packet.u.kein.req_faith_up.up_point = 10; // 10 �÷� �ش�.
	g_pServerTable->BroadCastToEveryServer((char*)&packet, sizeof(t_header) + packet.h.header.size, SERVER_TYPE_MAP);

	strcpy(ch->preach_name, p->name); // �������� �̸��� �ְ�
	SendLoginEvangelistCreate(ch->Name, p->name);	// ������ ��� �̸� ��� ����
}


void SendLoginEvangelistCreate( char *my_name, char *eva_name )
{
	t_packet packet;
	packet.h.header.type = CMD_LOGIN_EVANGELIST_CREATE;
	{
		strcpy( packet.u.kein.login_sql_evangelist_create.my_name, my_name );
		strcpy( packet.u.kein.login_sql_evangelist_create.eva_name, eva_name );
	}
	packet.h.header.size = sizeof( k_login_sql_evangelist_create );
	QueuePacket(connections, DB_DEMON, &packet, 1);
}

void SendLoginEvangelistDelete( char *my_name )
{
	t_packet packet;
	packet.h.header.type = CMD_LOGIN_EVANGELIST_DELETE;
	{
		strcpy( packet.u.kein.login_sql_evangelist_delete.my_name, my_name );
	}
	packet.h.header.size = sizeof( k_login_sql_evangelist_delete );
	QueuePacket(connections, DB_DEMON, &packet, 1);
}



struct GodTable g_GodTable[MAX_GOD];
char YearMeetingText[MAX_GOD][4][MAX_GOD_TEXT];
char MonthMeetingText[MAX_GOD][12][MAX_GOD_TEXT];


int g_y, g_mo, g_d, g_h, g_mi, g_se;		// ���ӽð�
extern void CurrentGameDate( DWORD t, int *y, int *mo, int *d, int *h, int *mi, int *sec );

void LoadGetGodTableForTest( int type )
{
	CurrentGameDate( g_curr_time*6, &g_y, &g_mo, &g_d, &g_h, &g_mi, &g_se );
	switch( type )
	{
		case YEAR_MEETING:
			{
				for( int i=1; i<MAX_GOD; i++ )			// �׽�Ʈ��		// year
				{
					g_GodTable[i].meet_year_month	= g_mo+1;
				}
			}

		case MONTH_MEETING :
			{
				for( int i=1; i<MAX_GOD; i++ )			// �׽�Ʈ��		// year
				{
					g_GodTable[i].meet_year_day		= g_d+1;
					g_GodTable[i].meet_year_hour	= g_h;
					g_GodTable[i].meet_month_day	= g_d+1;
					g_GodTable[i].meet_month_hour	= g_h;
				}
			}
		case DAY_MEETING :
			{
				for( int i=1; i<MAX_GOD; i++ )			// �׽�Ʈ��
					g_GodTable[i].meet_day_hour		= g_h;
				break;
			}
		case MONTH_MEETING_ALAM :
			{
				for( int i=1; i<MAX_GOD; i++ )			// �׽�Ʈ��		// year
				{
					g_GodTable[i].meet_month_day	= g_d;
					g_GodTable[i].meet_month_hour	= g_h;
				}
				break;
			}
	}
}

void LoadGetGodTable()
{	//< CSD-030506
	TIMESTAMP_STRUCT day, month, year;
	
	for (int i=1; i < MAX_GOD; ++i)
	{
		GetGodMeetingTime( day, month, year, i );

		g_GodTable[i].meet_year_month = (char)year.month;
		g_GodTable[i].meet_year_day = (char)year.day;	
		g_GodTable[i].meet_year_hour = (char)year.hour;
		g_GodTable[i].meet_month_day = (char)month.day;
		g_GodTable[i].meet_month_hour = (char)month.hour;
		g_GodTable[i].meet_day_hour = (char)day.hour;
	}

	LoadMeetingMessage();
}	//> CSD-030506

void LoadMeetingMessage()
{
	for( int a=EDELBLHOY; a<MAX_GOD; a++ )
	{
		for( int i=0; i<4; i++ )
			GetMeetingMessage( a, YEAR_MEETING, i, YearMeetingText[a][i] );

		for( int i=0; i<12; i++ )
			GetMeetingMessage( a, MONTH_MEETING, i, MonthMeetingText[a][i] );
	}
}

char *GetGodMeetMessage( int god_index, int type, int num )
{
	switch( type )
	{
		default : break;
		case YEAR_MEETING  :	return YearMeetingText[god_index][num];
		case MONTH_MEETING : 	return MonthMeetingText[god_index][num];
	}

	return NULL;
}


extern void DugeonMgrProc();// 020620 YGI
extern void CheckDay();

void CheckTimeForMenu_Kein()			//	1�п� �ѹ��� ���´�.
{	//< CSD-CN-031213
	static DWORD time = g_curr_time;
	if( (g_curr_time - time) < 30 ) return;		// 30�� 
	time = g_curr_time;

	CheckDay();
	DugeonMgrProc();   // 020620 YGI
	g_EventMgr.Proc(); // 021030 YGI

	if(LottoSystem())
	{	//<soto - 030517
		LottoSystem()->RunProc();
	}	//>soto - 030517

	CurrentGameDate( g_curr_time*6, &g_y, &g_mo, &g_d, &g_h, &g_mi, &g_se );
	// ���ڱ� ���¿� ���� ó��
	CUserManager::HASH_USER user = g_pUserManager->GetUserSet();

	for (CUserManager::ITOR_USER i = user.begin(); i != user.end(); ++i)
	{
		const int idUser = i->second;
		CHARLIST* pUser = CheckServerId(idUser);
		
		if (pUser != NULL && pUser->Status)
		{	
			if (pUser->IsFullLife() && pUser->IsFullMana())
			{	// ���̻� ���ʿ䰡 ���ٸ�
				SendServerWeakUp(idUser);	// �����
			}
		}
	}
}	//> CSD-CN-031213

void RecvToolPassword( char *password, short int cn )
{
	CHARLIST *ch = CheckServerId( cn );
	if( !ch ) return;
	char *pass = "wkfehlflfk";
	int len = strlen( pass );
	if( !strncmp(password, pass, len) )
	{
		strcpy( g_szPassWordName, ch->Name );
	}
}

void SendCallNoteMeeting( int type, CHARLIST *ch )		// ����ð� �� �ƴٰ� �˸��� ��ƾ
{
	return;		// ������...

	t_packet packet;
	packet.h.header.type = CMD_NOTE_MEETING;
	{
		packet.u.kein.note_meeting.meeting_type = type/DAY_MEETING;
		packet.u.kein.note_meeting.believe_god = ch->believe_god;
	}
	packet.h.header.size = sizeof( k_note_meeting );
	

	QueuePacket(connections, ch->GetServerID(), &packet, 1);
}

int CheckGodMeetingTime( CHARLIST *ch )
{
//	return MONTH_MEETING;

	if( !ch ) return 0;
	if( !ch->believe_god ) return 0;
	
	static bool year_time, month_time, day_time;		// �žӽ� üũ�� �ѹ��� �ϱ� ����

	/////////////// ���� �޴��� ����� ���� �Ǵ��ϴ� ��ƾ
	if( g_GodTable[ch->believe_god].meet_year_month	== g_mo+1 &&
		g_GodTable[ch->believe_god].meet_year_day	== g_d+1 &&
		g_GodTable[ch->believe_god].meet_year_hour	== g_h ) 
	{
		year_time = 0;
		return YEAR_MEETING;
	}

	else if(	!year_time  &&
				g_GodTable[ch->believe_god].meet_year_month	== g_mo+1 &&
				g_GodTable[ch->believe_god].meet_year_day	== g_d+1 &&
				g_GodTable[ch->believe_god].meet_year_hour+1 == g_h ) 
	{
//		year_time =1;
		SendCallNoteMeeting( YEAR_MEETING, ch );
	}

	if( g_GodTable[ch->believe_god].meet_month_day	== g_d+1 &&
		g_GodTable[ch->believe_god].meet_month_hour	== g_h ) 
	{
		month_time = 0;
		return MONTH_MEETING;
	}
	else if( !month_time &&												//		�ð� ���� ����
			g_GodTable[ch->believe_god].meet_month_day	== g_d &&
			g_GodTable[ch->believe_god].meet_month_hour+1	== g_h ) 
	{
//		month_time = 1;
		SendCallNoteMeeting( MONTH_MEETING, ch );
	}

	if( g_GodTable[ch->believe_god].meet_day_hour == g_h )	return DAY_MEETING;
	////////////////////////////////////////////////////////////////////

	/////////////// ���� �ð��� ������ ����� ���� �ð� 1�ð� �Ŀ� �Ǵ��Ͽ� �žӽ��� �÷��ش�.

	if( ch->is_god_meeting == YEAR_MEETING )
	if( g_GodTable[ch->believe_god].meet_year_month	< g_mo+1 ||
		g_GodTable[ch->believe_god].meet_year_day < g_d+1 || 
		(	g_GodTable[ch->believe_god].meet_year_month	== g_mo+1 &&
			g_GodTable[ch->believe_god].meet_year_day	== g_d+1 &&
			g_GodTable[ch->believe_god].meet_year_hour+1 <= g_h ) )
	{

		if( year_time == false )
		{
			if( (g_curr_time - ch->god_meeting_start_time) >= 60 || ch->i_go_year_meeting )
			{
				ch->faith += 100;
				ch->god_cast_level ++;
				CallClient( ch->GetServerID(), CMD_DIVINE_UP );
			}
			else 
			{
				ch->faith -= 30;
				CallClient( ch->GetServerID(), CMD_DIVINE_DOWN );
			}

			ch->SendCharInfoBasic(FAITH, ch->faith );////020704 lsw
		}

		ch->i_go_year_meeting = false;
		year_time = true;
		return 0;
	}

	if( ch->is_god_meeting == MONTH_MEETING )
	if( g_GodTable[ch->believe_god].meet_year_month	< g_mo+1 ||
		g_GodTable[ch->believe_god].meet_year_day < g_d+1 || 
		( g_GodTable[ch->believe_god].meet_month_day == g_d+1 &&
		  g_GodTable[ch->believe_god].meet_month_hour+1 <= g_h ) )
	{
		if( month_time == false )
		{
			if( ch->i_go_month_meeting ) 
			{
				ch->faith += 20;
				ch->god_cast_level ++;		// �ϴ� ���� ������ �ϳ��ۿ� �����Ƿ�...
				CallClient( ch->GetServerID(), CMD_DIVINE_UP );
			}
			else  
			{
				ch->faith -= 10;
				CallClient( ch->GetServerID(), CMD_DIVINE_DOWN );
			}
			ch->SendCharInfoBasic(FAITH, ch->faith );//020704 lsw
		}

		ch->i_go_month_meeting = false;
		month_time = true;
		return 0;
	}

	if( ch->is_god_meeting == DAY_MEETING )
	if( g_GodTable[ch->believe_god].meet_year_month	< g_mo+1 ||
		g_GodTable[ch->believe_god].meet_year_day < g_d+1 || 
		g_GodTable[ch->believe_god].meet_day_hour+1	<= g_h )
	{
		if( day_time == false ) 
			if( ch->i_go_day_meeting ) ch->faith += 5;

		ch->i_go_day_meeting = false;
		day_time = true;
		return 0;
	}
	///////////////////////////////////////////////////////////////////////

//	year_time = false;
//	month_time = false;
//	day_time = false;
	return 0;
}

void SendServerMessage( int type, char *szTemp, short int cn )
{
	if( !szTemp ) return;
	t_packet p;
	int len=strlen( szTemp );
	int max = (len / 512)+1;
	
	p.h.header.type = type;
	for( int i=0; i<max; i++ )
	{
		char *temp = szTemp+512*i;
		strncpy( p.u.kein.server_text_default.text, temp ,512 );
		p.u.kein.server_text_default.len = len;
		
		p.h.header.size = sizeof( k_server_text_default );
		QueuePacket(connections, cn, &p, 1);
	}
}

void SendLoginServerForCharUpdate( short int cn )		// ĳ���� ������ ������Ʈ�Ѵ�.
{
	CHARLIST *ch = &connections[cn].chrlst;
	if( !ch ) return;

	t_packet p;
	p.h.header.type = CMD_CHAR_UPDATE_DATA_BY_KEIN;
	{
		sprintf( p.u.kein.char_update_data.name, "%s", ch->Name );
		p.u.kein.char_update_data.believe_god		= ch->believe_god;
		p.u.kein.char_update_data.faith				= ch->faith;
		p.u.kein.char_update_data.god_cast_level	= ch->god_cast_level;
	}
	p.h.header.size = sizeof( k_char_update_data );
	QueuePacket(connections, DB_DEMON, &p, 1);
}

void  RecvLoginServerGetCharInfoByKein( k_get_char_info *p )
{
	CHARLIST *ch = &connections[p->server_id].chrlst;
	if( strcmp( p->name, connections[p->server_id].name ) != 0 ) return;	// 030923 HK YGI

	ch->believe_god			= p->believe_god;
	ch->faith				= p->faith;
	ch->god_cast_level		= p->god_cast_level;

	t_packet packet;
	packet.h.header.type = CMD_GET_CHARINFO_BY_KEIN;
	{
		packet.u.kein.server_to_client_char_info_plus.believe_god = p->believe_god;
		packet.u.kein.server_to_client_char_info_plus.faith = p->faith;
	}
	packet.h.header.size = sizeof( k_server_to_client_char_info_plus );

	if( p->server_id > DB_DEMON )
		QueuePacket(connections, p->server_id, &packet, 1);
}

void SendReqGetCharInfoEtc( short int server_id, char *name )
{
	t_packet p;

	p.h.header.type = CMD_GET_CHARINFO_BY_KEIN;
	{
		p.u.kein.req_chr_info_etc.server_id	= server_id;
		sprintf( p.u.kein.req_chr_info_etc.name, "%s", name );
	}
	p.h.header.size = sizeof( k_req_chr_info_etc );
	QueuePacket(connections, DB_DEMON, &p, 1);
}

void SendServerKillPc( short int cn )		// 0414 KHS
{
	return;
	if( rand()%3 == 0 )  return;

	CHARLIST *ch = CheckServerId( cn );
	if( !ch ) return;

	t_packet packet;
	packet.h.header.type = CMD_SERVER_KILLPC;
	{
		packet.u.kein.server_killpc.id		=  cn;
		packet.u.kein.server_killpc.killpc  =  ch->killpc;
	}
	packet.h.header.size = sizeof(k_server_killpc);

	QueuePacket(connections, cn, &packet, 1);
}

void RecvCanMemorize( t_connection c[], short int cn )
{
	CHARLIST *ch = &c[cn].chrlst;
	if( !ch ) return;

	if( g_hour >= 6 && g_hour < 13 ) CallClient( cn, CMD_CAN_MEMORIZE_OK );
	else
	{
		if( (g_curr_time - ch->can_memorize_time) <= 1200 ) CallClient( cn, CMD_CAN_MEMORIZE_OK );
		else CallClient( cn, CMD_CAN_MEMORIZE_NOT );
	}	
}

void SendEarMessage( char *name, char *message, short int who, int send_game_server, int send_server_id )		// ���������� �ӼӸ��� ������.
{
	// ����� Ŭ���̾�Ʈ�� �޽����� �����ش�.
	t_packet p;
	p.h.header.type = CMD_SEND_EAR_MESSAGE;
	{
		strcpy( p.u.kein.send_ear_message.name, name );
		strcpy( p.u.kein.send_ear_message.message, message);
		p.u.kein.send_ear_message.send_game_server = send_game_server;		// ���� ����� ���� ���� ���� ��Ʈ��ȣ
		p.u.kein.send_ear_message.send_server_id = send_server_id;			// ���� ����� ���̵�
	}
	p.h.header.size = sizeof( k_send_ear_message )-200+strlen( message );
	QueuePacket(connections, who, &p, 1);
}

void SendEarMessageResult( int type, char *name, short int cn )
{
	t_packet p;
	p.h.header.type = type;
	{
		strcpy( p.u.kein.send_ear_message_ok.name, name );
	}
	p.h.header.size = sizeof( k_send_ear_message_ok )-20+strlen( name )+1;
	QueuePacket(connections, cn, &p, 1);
}

void SendEarMessageResult2( int type, int ret, short int cn )
{
	t_packet p;
	p.h.header.type = type;
	{
		p.u.kein.default_char = (char )ret;
	}
	p.h.header.size = sizeof( char );
	QueuePacket(connections, cn, &p, 1);
}

inline int GetOwnPort( )
{
	SERVER_DATA *pData = g_pServerTable->GetOwnServerData();
	return pData->wPort;
}

void RecvEarMessage( char *name, char *message, short int cn )		// 1�� 
{
	CHARLIST *ch = CheckServerId( cn );
	if( !ch ) return;

	ConvString( message );

	int recv_cn = ExistHe( name );
	if( recv_cn > DB_DEMON )
	{
		CHARLIST *pTarget = CheckServerId( recv_cn );
		if(!pTarget){return;}

		if( ch->name_status.nation != pTarget->name_status.nation ) 
		{
			if(!GMMgr.IsMerchant(ch) 
			&& !GMMgr.IsMerchant(pTarget)
			&& !ch->IsCounselor()
			&& !pTarget->IsCounselor())//030114 lsw
			{
				SendEarMessageResult2( CMD_SEND_EAR_MESSAGE_RESULT_ETC, -1, cn);		// �ٸ� ���� ����̴�.
				return;
			}
		}

		SendEarMessage( ch->Name, message, recv_cn, -1, cn );		// ���������� �ӼӸ��� ������.
		SendEarMessageResult( CMD_SEND_EAR_MESSAGE_OK, name, cn);
		return;
	}
	
	t_packet p;
	p.h.header.type = CMD_SEND_EAR_MESSAGE_OTHER;				// �ٸ� �ʿ� �ִ��� üũ
	{
		p.u.kein.login_to_gameserver_ear_message.game_server = GetOwnPort();
		p.u.kein.login_to_gameserver_ear_message.server_id = cn;
		if(GMMgr.IsMerchant(ch) || ch->IsCounselor())//030129 lsw
		{
			p.u.kein.login_to_gameserver_ear_message.nation = 0;//������ ������ ���� �޼��� ������ �˻絵 ����
		}
		else
		{
			p.u.kein.login_to_gameserver_ear_message.nation = ch->name_status.nation;
		}
		strcpy( p.u.kein.login_to_gameserver_ear_message.recv_name, name );
		strcpy( p.u.kein.login_to_gameserver_ear_message.send_name, ch->Name );
		strcpy( p.u.kein.login_to_gameserver_ear_message.message, message );
	}
	p.h.header.size = sizeof( k_login_to_gameserver_ear_message )-200+strlen( message );


	// Modified by chan78 at 2000/12/01 BroadCast It to MapServers.
	g_pServerTable->BroadCastToEveryServer( (char*)&p, (sizeof(t_header)+p.h.header.size), SERVER_TYPE_MAP );

	CheckEarChatingData( ch->Name, name, message );
	//	SendEarMessageResult( CMD_SEND_EAR_MESSAGE_NOT, name, cn);
}

void SendEarMessageReturnResult( short int gameserver_port, short int server_id, char *name, int ret  )	// ret 1: �� ����, 2: ����, 3:������
{
	t_packet p;
	p.h.header.type = CMD_SEND_EAR_MESSAGE_OK;
	{
		p.u.kein.send_ear_message_ok_to_login.gameserver_port = gameserver_port;
		p.u.kein.send_ear_message_ok_to_login.server_id	= server_id;
		strcpy( p.u.kein.send_ear_message_ok_to_login.name, name );
		p.u.kein.send_ear_message_ok_to_login.ret = ret;
	}
	p.h.header.size = sizeof( k_send_ear_message_ok_to_login );

	// Modified by chan78 at 2000/12/01 BroadCast It to MapServers.
	g_pServerTable->BroadCastToEveryServer( (char*)&p, (sizeof(t_header)+p.h.header.size), SERVER_TYPE_MAP );
}

void FindWhoEarMessageTarget( k_login_to_gameserver_ear_message *p )// �ͼӸ��� ���� ����� �� ���� ������ �����ϴ��� �Ǵ�
{
	const int who = ExistHe( p->recv_name );
	CHARLIST *ch = CheckServerId( who );
	if( !ch ) return;

	if(	!GMMgr.IsMerchant(ch)// he is not Merchant
	&&	!ch->IsCounselor()//He is counselor? Can Recv EarMessage
	&&	p->nation
	&&	ch->name_status.nation 
	&&	p->nation != ch->name_status.nation ) // ������ Ʋ����	// �޽��� ���
	{
		k_send_ear_message_result ret;
		ret.ret = 4;
		ret.send_game_server = p->game_server;
		ret.send_server_id = p->server_id;
		SendLoginEarMessageResult( &ret, -1 );			// �׾� �ִ�
		return;		
	}

	if( ch->bAlive == TRUE )
	{			
		SendEarMessage( p->send_name, p->message, who, p->game_server, p->server_id );		// Ŭ���̾�Ʈ���� ����� �޾ƿ´�. 
	}
	else 
	{
		//k_send_ear_message_result 
		k_send_ear_message_result ret;
		ret.ret = 2;
		ret.send_game_server = p->game_server;
		ret.send_server_id = p->server_id;
		SendLoginEarMessageResult( &ret, -1 );			// �׾� �ִ�
	}
}

void SendLoginEarMessageResult( k_send_ear_message_result *t, short int cn )		
{
	// Ŭ���̾�Ʈ���� ���� �ӼӸ� ��� ó�� �� �ٸ� �������� �� �����( ����� ������ �ȵ������ )

	if( t->send_server_id == -1 )	// 010607 YGI
	{
		t->send_game_server = -1; 
		t->send_server_id = cn;
	}

	if( t->send_game_server == -1 )		// ���� �ʿ� ���� ���
	{
		RecvLoginToGameEarMessageResult( t->send_server_id, t->ret );
		return;		
	}

	t_packet p;
	p.h.header.type = CMD_LOGIN2GAME_EAR_MESSAGE_RESULT;
	{
		p.u.kein.login2game_ear_message_result.send_server_id	= t->send_server_id;
		p.u.kein.login2game_ear_message_result.ret = t->ret;
	}
	int size = p.h.header.size = sizeof( k_login2game_ear_message_result );
	g_pServerTable->SendRajaPacketToOtherMapServer( (WORD)t->send_game_server, (char *)&p, size+sizeof(t_header) );
	//QueuePacket(connections, LOGIN_SERVER_CONNECT, &p, 1);
}

void RecvLoginToGameEarMessageResult( short int server_id, char ret )		// �α��μ������� ������ ������ Ŭ���̾�Ʈ�� �����ش�.
{
	CHARLIST *ch = CheckServerId( server_id );
	if( !ch ) return;

	if( connections[server_id].dwAgentConnectionIndex )
	{
		t_packet p;
		p.h.header.type = CMD_GAME2CLIENT_EAR_RESULT;
		{
			p.u.kein.game2client_ear_result.ret = ret;
		}
		p.h.header.size = sizeof( k_game2client_ear_result );
		QueuePacket(connections, server_id, &p, 1);
	}
}

void SendGameToLoginExistName( char *name, short int cn )
{
	t_packet p;
	p.h.header.type = CMD_GAME2LOGIN_CHECK_NAME;
	{
		strcpy( p.u.kein.loginserver_to_gameserver_ear_message.name, name );
		p.u.kein.loginserver_to_gameserver_ear_message.server_id = cn;
	}
	p.h.header.size = sizeof( k_loginserver_to_gameserver_ear_message);

	// Modified by chan78 at 2000/12/01 BroadCast It to MapServers.
	g_pServerTable->BroadCastToEveryServer( (char*)&p, (sizeof(t_header)+p.h.header.size), SERVER_TYPE_MAP );
}


///////////////////////////////////////////////////////////////////////////
//
//			 ������������ �ؽ�Ʈ�� �����ϴ� �Լ����� ����
//
////////////////////////////////////////////////////////////////////////////
void CheckChatingData( char *name, char *message )		// ä���ϴ� �� ���� �ϱ�
{
	FILE *fp;
	char temp[MAX_PATH];
	sprintf( temp, ".\\logout_chat\\ä��_%s.txt", name);
	fp = fopen( temp, "at+" );
	if( !fp ) return;
	
	fprintf( fp, "%20s>> %s\n", name, message );
	fclose( fp );
}

int ChangePosType( int type )
{
	switch( type )
	{
		case CHARACTER	:	return PT_CHARACTER;
		case GROUND		:	return PT_GROUND;
		case USE		:	return PT_USE;
		default : return type;
	}
}

void CheckBackupItemForCharacter( CHARLIST *ch )
{
	char filename[FILENAME_MAX];
	wsprintf( filename, "./logout_item/%s_item.bin", ch->Name );
	FILE *fp = fopen( filename, "wb+" );
	if( fp )
	{
		fwrite( ch->inv, sizeof( ItemAttr ), 3*3*8, fp );
		fwrite( ch->equip, sizeof( ItemAttr ), 8, fp );
		fclose(fp );
	}
}

void CheckMoveItem( char *name, int source_type, int dest_type, ItemAttr item, DWORD money)
{
	return;

	dest_type = ChangePosType( dest_type );
	source_type = ChangePosType( source_type );

	t_packet p;
	p.h.header.type = CMD_GAME2LOGIN_MOVING_ITEM;
	{
		strcpy( p.u.kein.moving_item.name, name );
		p.u.kein.moving_item.source_type = source_type;
		p.u.kein.moving_item.dest_type = dest_type;
		p.u.kein.moving_item.item = item;
		p.u.kein.moving_item.money = money;
	}
	p.h.header.size = sizeof( k_moving_item );
	QueuePacket(connections, LOGIN_SERVER_CONNECT, &p, 1);	
}

void CheckEarChatingData( char *send_name, char *recv_name, char *message )		// ä���ϴ� �� ���� �ϱ�
{
	FILE *fp;
	char temp[MAX_PATH];
	sprintf( temp, ".\\logout_ear\\�Ӹ�_%s.txt", send_name);
	fp = fopen( temp, "at+" );
	if( !fp ) return;
	
	fprintf( fp, "%20s>>%20s %s\n", send_name, recv_name, message );
	fclose( fp );
}



// �������� Character Data�� �ٸ� MapServer�� �̵���Ű�� �Լ�...
//////////////////////////////////////////////////////////////////////////
//	
//	�ʰ� �̵� �޼ҵ� ( �ޱ� )  
// Rewrote by chan78 at 2000/11/25
//
//////////////////////////////////////////////////////////////////////////
bool NewSendGameToLoginChangeMapData( short int cn, WORD wMyAgentServerID, WORD wTargetServerID )		// ĳ���� ����Ÿ 1
{
	CHARLIST* ch = CheckServerId(cn);
	
	if (!ch)
	{
		return false;
	}
	
	if(!::CanSaveUserData(ch,0)){return false;}

	if (ch->IsPlayer())
	{ // ���̵��� �ϸ� ��ȯ���� ����
		connections[cn].chrlst.m_xSummon.Remove(KillMonster);
	}

	if (connections[cn].dwAgentConnectionIndex == 0)
	{
		return false;
	}

	char szMsg[sizeof(t_header) + sizeof(change_map_protocol) + 1];
	szMsg[0] = BYTE(PTCL_MAP_TO_MAP);
	//ReleaseItem( ch );			// 010521 YGI
	if (EXCHANGE_STATE_READY != ch->GetExchangeState())
	{	// �ŷ����̴��� ��� ��Ų��.
		SendExchangeCancel(ch->GetExchangeTargetId(), cn);
	}

	if (ch->handheld.item_no)
	{
		MoveEmptyInv(&ch->handheld, ch);
		POS pos;
		SetItemPos(HAND, &pos);
		SendServerEachItem(&pos, &ch->handheld, cn);
	}
	
	t_packet* p = (t_packet*)(szMsg + 1);
	// change_map9
	// -------------------------------------------------------------------------------
	{
		p->u.change_map_protocol.mapdata9.game_port = wTargetServerID;
		memcpy(p->u.change_map_protocol.mapdata9.name,	connections[ch->GetServerID()].name, NM_LENGTH);
		p->u.change_map_protocol.mapdata9.Gender = ch->Gender;
		p->u.change_map_protocol.mapdata9.Face = ch->Face;
		p->u.change_map_protocol.mapdata9.Class	= ch->Class;
		p->u.change_map_protocol.mapdata9.Job =	ch->Job;
		p->u.change_map_protocol.mapdata9.Spell = ch->Spell;	
		p->u.change_map_protocol.mapdata9.SprNo = ch->SprNo;	
		p->u.change_map_protocol.mapdata9.Peacests = ch->Peacests;	
		p->u.change_map_protocol.mapdata9.BodyR	= ch->BodyR;	
		p->u.change_map_protocol.mapdata9.BodyG = ch->BodyG;	
		p->u.change_map_protocol.mapdata9.BodyB	= ch->BodyB;	
		p->u.change_map_protocol.mapdata9.ClothR = ch->ClothR;
		p->u.change_map_protocol.mapdata9.ClothG = ch->ClothG;
		p->u.change_map_protocol.mapdata9.ClothB = ch->ClothB;
		p->u.change_map_protocol.mapdata9.total_id = ch->total_id;
		p->u.change_map_protocol.mapdata9.nk3 =	ch->nk[N_VYSEUS];	
		p->u.change_map_protocol.mapdata9.nk4 =	ch->nk[N_ZYPERN];
		p->u.change_map_protocol.mapdata9.nk6 =	ch->nk[N_YILSE];
		// ���� �̵� ��� ���� ���ϱ�
		ch->GetTransMap(p->u.change_map_protocol.mapdata9.nTransPort,
						p->u.change_map_protocol.mapdata9.nTransX,
						p->u.change_map_protocol.mapdata9.nTransY);
		p->u.change_map_protocol.mapdata9.JoinLocalWar = ch->JoinLocalWar;		// LTS LOCALWAR
		p->u.change_map_protocol.mapdata9.GainedFame = ch->GainedFame;
		p->u.change_map_protocol.mapdata9.CurrentPhase = ch->CurrentPhase;
		p->u.change_map_protocol.mapdata9.EventJoin = ch->EventJoin;		// 020115 LTS
		p->u.change_map_protocol.mapdata9.HorseNo = ch->HorseNo;			// LTS HORSERIDER
		p->u.change_map_protocol.mapdata9.HorseGrade = ch->HorseGrade;		// LTS HORSERIDER
		p->u.change_map_protocol.mapdata9.RiderNo = ch->RiderNo;			// LTS HORSERIDER
		p->u.change_map_protocol.mapdata9.RiderGrade = ch->RiderGrade;		// LTS HORSERIDER
		
		p->u.change_map_protocol.mapdata9.bIsTradeOn = ch->IsTradeOn();		//030127 lsw
		p->u.change_map_protocol.mapdata9.nSkillSadCount = ch->nSkillSadCount;		//030127 lsw
		p->u.change_map_protocol.mapdata9.nSkillBlessLeaf = ch->nSkillBlessLeaf;		//030127 lsw

		p->u.change_map_protocol.mapdata9.nBillingType = ch->GetBillingType();		//030127 lsw
		p->u.change_map_protocol.mapdata9.nUserAge	= ch->m_nUserAge;				// 030929 kyo	
	}

	DWORD id = ch->GetServerID();
	// change_map1
	// -------------------------------------------------------------------------------
	{
		p->u.change_map_protocol.mapdata1.game_port = wTargetServerID;

		t_update_char_db* tp = &p->u.change_map_protocol.mapdata1.ch_data;
		strcpy(tp->id, connections[id].id);
		strcpy(tp->name, connections[id].name);

		tp->Level = ch->GetLevel(); // CSD-030806
		tp->nGuildCode = ch->GetGuildCode(); // CSD-030324
		tp->social_status = ch->social_status;
		tp->fame = ch->fame;
		tp->NWCharacter	= ch->NWCharacter; // 010915 LTS	// mapdata9�� �ߺ� 

		tp->nk3 = ch->nk[N_VYSEUS];
		tp->nk4	= ch->nk[N_ZYPERN];
		tp->nk6	= ch->nk[N_YILSE];
		
		tp->Str	= ch->Str;
		tp->Con	= ch->Con;
		tp->Dex	= ch->Dex;
		tp->Wis	= ch->Wis;
		tp->Int	= ch->Int;
		tp->MoveP = ch->MoveP;
		tp->Char = ch->Char;
		tp->Endu = ch->Endu;
		tp->Moral = ch->Moral;
		tp->Luck = ch->Luck;
		tp->wsps = ch->wsps;

		tp->Money = ch->Money;
		tp->Tactics	= ch->Tactics;				// ������ ������� (�����ֱ� ����)
		memcpy(&tp->nation, &ch->name_status, sizeof(DWORD));		// 1004 YGI

		tp->Hp = ch->Hp;
		tp->HpMax = ch->HpMax ;
		tp->Mana = ch->Mana;					// ������
		tp->ManaMax	= ch->ManaMax;
		tp->Hungry = ch->Hungry;	
		tp->HungryMax = ch->HungryMax;

		tp->Condition = ch->GetState();
		memcpy(tp->MapName, ch->MapName, 20);
		tp->Sight = ch->Sight;
		tp->Age = ch->Age;
		tp->bAlive = ch->bAlive;					// ĳ������ ���翩��(	REMOVE_:0 / ALIVE_:1 / DEAD_:2 / BUFE_:3)

		tp->killmon = ch->killmon;						// 1�迭���� ų����
		tp->killanimal = ch->killanimal;				// ���� ų����
		tp->killpc = ch->killpc;						// ���,NPC ų����
		//030227 lsw
		tp->nPoison = ch->GetBasicResist(RT_POISON); // ���ְ迭 ������ ���� ���׷�
		tp->nCurse	= ch->GetBasicResist(RT_CURSE);	 //	���ְ迭 ������ ���� ���׷�
		tp->nHoly	= ch->GetBasicResist(RT_HOLY);    // �ŷ°迭 ���� ������ ���� ���׷�
		tp->nFire	= ch->GetBasicResist(RT_FIRE);     //	�Ұ迭 ���� ������ ���� ���׷�
		tp->nIce	= ch->GetBasicResist(RT_ICE);       //	�����迭 ���� ������ ���� ���׷�
		tp->nElect	= ch->GetBasicResist(RT_ELECT);   //	���ݰ迭 ���� ������ ���� ���׷�

		tp->X = ch->X;
		tp->Y = ch->Y;

		tp->accessory[0] = ch->accessory[0];
		tp->accessory[1] = ch->accessory[1];
		tp->accessory[2] = ch->accessory[2];
		tp->accessory[3] = ch->accessory[3];
		tp->mantle		 = ch->mantle; // 011018 KHS ����
		
		tp->openhouse		= ch->openhouse;
		tp->reserved_point = ch->GetReservedPoint();
		tp->BankMoney		= ch->GetBankMoney();
		
		// 010406 YGI
		memcpy( &tp->win_defeat, &ch->WinLoseScore, sizeof( DWORD ) );
		tp->LadderScore		= ch->LadderScore;
		//tp->LastTotal		= ch->LastTotal;
		tp->LastLoan		= ch->LastLoan;
		//tp->LastLoan_time	= ch->LastLoan_time;

		tp->Exp				= ch->Exp;
		tp->viewtype		= ch->viewtype;
		ch->GetStepInfo(tp->aStepInfo, sizeof(tp->aStepInfo));
		tp->nCombatAmount = ch->Cp;
	}	

	// -------------------------------------------------------------------------------
	{
		t_server_update_binary_data0 *tp = &p->u.change_map_protocol.mapdata2.ch_data;
		p->u.change_map_protocol.mapdata2.game_port = wTargetServerID;
		memcpy(tp->name, connections[id].name, NM_LENGTH);
		memcpy(tp->id, connections[id].id,  ID_LENGTH );
		memcpy(tp->Ws,				(UCHAR *)ch->Ws,			SIZE_OF_WS );
		memcpy(tp->Ps,				(UCHAR *)ch->Ps,			SIZE_OF_PS );
		memcpy(tp->Skill,			(UCHAR *)ch->Skill,			SIZE_OF_SKILL );
		memcpy(tp->skillexp,		(UCHAR *)ch->skillexp,		SIZE_OF_SKILL_EXP );
		memcpy(tp->tac_skillEXP,	(UCHAR *)ch->tac_skillEXP,	SIZE_OF_TAC_SKILL_EXP );
	}
	// -------------------------------------------------------------------------------

	// -------------------------------------------------------------------------------
	{
		p->u.change_map_protocol.mapdata3.game_port = wTargetServerID;
		memcpy(p->u.change_map_protocol.mapdata3.ch_data.name, connections[id].name, NM_LENGTH);
		memcpy(p->u.change_map_protocol.mapdata3.ch_data.id, connections[id].id,  ID_LENGTH );
		memcpy(p->u.change_map_protocol.mapdata3.ch_data.script_var,			(UCHAR *)var[ id],			SIZE_OF_SCRIPT_VAR );
	}
	// -------------------------------------------------------------------------------
	
	// -------------------------------------------------------------------------------
	//////////////////////////// 0218 YGI //////////////////////
	{
		EndsetPartyMenberForUpdate( ch );		
		p->h.header.type = CMD_CHANGE_MAP_DATA4;	
		p->u.change_map_protocol.mapdata4.game_port = wTargetServerID;
		memcpy(p->u.change_map_protocol.mapdata4.ch_data.name, connections[id].name, NM_LENGTH);
		memcpy(p->u.change_map_protocol.mapdata4.ch_data.id, connections[id].id,  ID_LENGTH );
		memcpy(p->u.change_map_protocol.mapdata4.ch_data.equip,			(UCHAR *)ch->equip,			SIZE_OF_EQUIP);
		memcpy(p->u.change_map_protocol.mapdata4.ch_data.quick,			(UCHAR *)ch->quick,			SIZE_OF_QUICK);
		memcpy(p->u.change_map_protocol.mapdata4.ch_data.party,			(UCHAR *)ch->party_str,			SIZE_OF_PARTY );
		memcpy(p->u.change_map_protocol.mapdata4.ch_data.relation,			(UCHAR *)ch->relation_str,		SIZE_OF_RELATION);
		memcpy(p->u.change_map_protocol.mapdata4.ch_data.employment,		(UCHAR *)ch->employment_str,	SIZE_OF_EMPLOYMENT);
	}
	// -------------------------------------------------------------------------------
	

	// -------------------------------------------------------------------------------
	{
		p->u.change_map_protocol.mapdata5.game_port = wTargetServerID;
		memcpy(p->u.change_map_protocol.mapdata5.ch_data.name, connections[id].name, NM_LENGTH);
		memcpy(p->u.change_map_protocol.mapdata5.ch_data.id,   connections[id].id,  ID_LENGTH );
		memcpy(p->u.change_map_protocol.mapdata5.ch_data.inv,			(UCHAR *)ch->inv,				SIZE_OF_INV );
	}
	// -------------------------------------------------------------------------------

	// -------------------------------------------------------------------------------
	{
		p->u.change_map_protocol.mapdata6.game_port = wTargetServerID;
		memcpy(p->u.change_map_protocol.mapdata6.ch_data.name, connections[id].name, NM_LENGTH);
		memcpy(p->u.change_map_protocol.mapdata6.ch_data.id,   connections[id].id,  ID_LENGTH );
		memcpy(p->u.change_map_protocol.mapdata6.ch_data.Item,			(UCHAR *)ch->Item,			SIZE_OF_ITEMINDEX );
	}
	// -------------------------------------------------------------------------------

	// -------------------------------------------------------------------------------
	{
		p->u.change_map_protocol.mapdata7.game_port = wTargetServerID;
		memcpy(p->u.change_map_protocol.mapdata7.ch_data.name,	connections[id].name, NM_LENGTH);
		memcpy(p->u.change_map_protocol.mapdata7.ch_data.id,		connections[id].id,  ID_LENGTH );
		memcpy(p->u.change_map_protocol.mapdata7.ch_data.bankitem,		(UCHAR *)ch->bank,		SIZE_OF_BANKITEM );
	}
	// -------------------------------------------------------------------------------

	// -------------------------------------------------------------------------------
	{
		p->u.change_map_protocol.mapdata8.game_port = wTargetServerID;
		memcpy(p->u.change_map_protocol.mapdata8.ch_data.name,	connections[id].name, NM_LENGTH);

		p->u.change_map_protocol.mapdata8.ch_data.believe_god	= ch->believe_god;
		p->u.change_map_protocol.mapdata8.ch_data.faith			= ch->faith;
		p->u.change_map_protocol.mapdata8.ch_data.god_cast_level = ch->god_cast_level;
	}
	// -------------------------------------------------------------------------------
	// Build MAP_CHANGE_REQUEST packet
	p->h.header.type = CMD_MAP_CHANGE_REQUEST;
	p->h.header.size = sizeof(t_change_map_protocol);
	p->h.header.crc = 0;
	p->u.change_map_protocol.wAgentServerID = wMyAgentServerID;
	p->u.change_map_protocol.dwIDForAgent = connections[cn].dwIDForAgent;
	p->u.change_map_protocol.FromServerCn = cn;

	memcpy(&p->u.change_map_protocol.id, connections[cn].id, ID_LENGTH);
	memcpy(&p->u.change_map_protocol.password, connections[cn].pw, PW_LENGTH );
	memcpy(&p->u.change_map_protocol.name, connections[cn].name, NM_LENGTH );
	memcpy(&p->u.change_map_protocol.mapname, connections[cn].mapname, NM_LENGTH);
	memcpy(&p->u.change_map_protocol.ip, connections[cn].ip_address, NM_LENGTH);
	//EquipItem( ch );		// 010525 YGI
	// Write it.
	if (!g_pServerTable->Send(wTargetServerID, szMsg, sizeof(t_header) + p->h.header.size + 1))
	{
		MyLog(LOG_FATAL, "NewSendGameToLoginChangeMapData() :: Can't send REQUEST to Server(ID:%d)", wTargetServerID);
		return false;
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
//	
//	�ʰ� �̵� �޼ҵ� ( �ޱ� )  
//  2000/11/25, chan78
//
//////////////////////////////////////////////////////////////////////////
void NewRecvChangeMapData1( k_change_map1 *p, DWORD cn )		// ĳ���� ����Ÿ 1
{
	CHARLIST *ch = &connections[cn].chrlst;
	if( !ch ) return;

	t_update_char_db *data1 = &p->ch_data;

	ch->SetLevel(data1->Level); // CSD-030806
	ch->SetGuildCode(data1->nGuildCode); // CSD-030806
	ch->social_status	= data1->social_status;
	ch->fame			= data1->fame;
	ch->NWCharacter		= data1->NWCharacter;		// 010915 LTS

	ch->nk[ N_VYSEUS]	= data1->nk3;
	ch->nk[ N_ZYPERN]	= data1->nk4;
	ch->nk[ N_YILSE]	= data1->nk6;

	ch->Str  				= data1->Str;
	ch->Con  				= data1->Con;
	ch->Dex  				= data1->Dex;
	ch->Wis  				= data1->Wis;
	ch->Int  				= data1->Int;
	ch->MoveP				= data1->MoveP;
	ch->Char 				= data1->Char;
	ch->Endu 				= data1->Endu;
	ch->Moral				= data1->Moral;
	ch->Luck 				= data1->Luck;
	ch->wsps 				= data1->wsps;

	ch->Tactics 		= data1->Tactics;				// ������ ������� (�����ֱ� ����)
	memcpy( &ch->name_status, &data1->nation, sizeof( DWORD ) );		// 1004 YGI
	ch->Money			= data1->Money;
	ch->Hp				= data1->Hp;
	ch->HpMax			= data1->HpMax ;
	ch->Mana			= data1->Mana;					// ������//020214 lsw
	ch->ManaMax			= data1->ManaMax;
    ch->SetState(data1->Condition);
	memcpy( ch->MapName, MapName, 20 );
	ch->Sight = data1->Sight;
	ch->Age = data1->Age;
	ch->bAlive		= data1->bAlive;					// ĳ������ ���翩��(	REMOVE_:0 / ALIVE_:1 / DEAD_:2 / BUFE_:3)

	ch->Hungry = data1->Hungry;	
	ch->HungryMax = data1->HungryMax;
	ch->killmon = data1->killmon;						// 1�迭���� ų����
	ch->killanimal = data1->killanimal;				// ���� ų����
	ch->killpc = data1->killpc;						// ���,NPC ų����

	ch->SetBasicResist(RT_POISON,70); // ���ְ迭 ������ ���� ���׷�//030227 lsw
	ch->SetBasicResist(RT_CURSE,70); //	���ְ迭 ������ ���� ���׷�
	ch->SetBasicResist(RT_HOLY,70); // �ŷ°迭 ���� ������ ���� ���׷�
	ch->SetBasicResist(RT_FIRE,70); //	�Ұ迭 ���� ������ ���� ���׷�
	ch->SetBasicResist(RT_ICE,70); //	�����迭 ���� ������ ���� ���׷�
	ch->SetBasicResist(RT_ELECT,70); //	���ݰ迭 ���� ������ ���� ���׷�

	ch->X = data1->X;
	ch->Y = data1->Y;

	ch->accessory[0] = data1->accessory[0];
	ch->accessory[1] = data1->accessory[1];
	ch->accessory[2] = data1->accessory[2];
	ch->accessory[3] = data1->accessory[3];
	ch->mantle		 = data1->mantle; // 011018 KHS ���� 
		
	ch->openhouse		= data1->openhouse;
	ch->SetReservedPoint(data1->reserved_point);
	ch->SetBankMoney(data1->BankMoney, BMCT_RECV_MAP_CHANGE); // CSD-030723
	// 010406 YGI
	memcpy( &ch->WinLoseScore, &data1->win_defeat, sizeof( DWORD ) );
	ch->LadderScore		= data1->LadderScore;
	//ch->LastTotal		= data1->LastTotal;
	ch->LastLoan		= data1->LastLoan;
	//ch->LastLoan_time	= data1->LastLoan_time;

	ch->Exp				= data1->Exp;
	ch->viewtype		= data1->viewtype;
    ch->SetStepInfo(data1->aStepInfo, sizeof(data1->aStepInfo));
    ch->Cp = data1->nCombatAmount;
	ch->change_map_confirm[0] = 1;		// 1��° ����Ÿ �޾Ҵ�
}

void NewRecvChangeMapData2( k_change_map2 *p, DWORD cn )		// ĳ���� ����Ÿ 2 �ޱ�
{
	CHARLIST *ch = &connections[cn].chrlst;
	if( !ch ) return;

		memcpy(ch->Ws,			p->ch_data.Ws,			SIZE_OF_WS );
		memcpy(ch->Ps,			p->ch_data.Ps,			SIZE_OF_PS );
		memcpy(ch->Skill,		p->ch_data.Skill,		SIZE_OF_SKILL );
		memcpy(ch->skillexp,	p->ch_data.skillexp,	SIZE_OF_SKILL_EXP );
		memcpy(ch->tac_skillEXP,p->ch_data.tac_skillEXP, SIZE_OF_TAC_SKILL_EXP );

	ch->change_map_confirm[1] = 1;		// 2��° ����Ÿ �޾Ҵ�
}

void NewRecvChangeMapData3( k_change_map3 *p, DWORD cn )		// ĳ���� ����Ÿ 3 �ޱ�
{
	CHARLIST *ch = &connections[cn].chrlst;
	if( !ch ) return;

	memcpy(var[cn], p->ch_data.script_var,	SIZE_OF_SCRIPT_VAR );

	ch->change_map_confirm[2] = 1;		// 3��° ����Ÿ �޾Ҵ�
}

void NewRecvChangeMapData4( k_change_map4 *p, DWORD cn )		// ĳ���� ����Ÿ 4 �ޱ�
{
	CHARLIST *ch = &connections[cn].chrlst;
	if( !ch ) return;

		memcpy(ch->equip,			p->ch_data.equip,			SIZE_OF_EQUIP);
		memcpy(ch->quick,			p->ch_data.quick,			SIZE_OF_QUICK);
		memcpy(ch->party_str,		p->ch_data.party,			SIZE_OF_PARTY );
		memcpy(ch->relation_str,	p->ch_data.relation,		SIZE_OF_RELATION);
		memcpy(ch->employment_str,	p->ch_data.employment,		SIZE_OF_EMPLOYMENT);

	// 011030 KHS JJING
	// ���̵������� JJING�� ���� Setting�Ѵ�. 
	ch->jjing = ((RareMain*)&(ch->equip[ WT_ARMOR].attr[ IATTR_RARE_MAIN]))->grade;

	InitPartyMemberServerToServer( ch );

	ch->change_map_confirm[3] = 1;		// 4��° ����Ÿ �޾Ҵ�
}

void NewRecvChangeMapData5( k_change_map5 *p, DWORD cn )		// ĳ���� ����Ÿ 5 �ޱ�
{
	CHARLIST *ch = &connections[cn].chrlst;
	if( !ch ) return;

	memcpy(ch->inv, p->ch_data.inv, SIZE_OF_INV );

	ch->change_map_confirm[4] = 1;		// 5��° ����Ÿ �޾Ҵ�
}

void NewRecvChangeMapData6( k_change_map6 *p, DWORD cn )		// ĳ���� ����Ÿ 6 �ޱ�
{
	CHARLIST *ch = &connections[cn].chrlst;
	if( !ch ) return;

	memcpy(ch->Item, p->ch_data.Item, SIZE_OF_ITEMINDEX );

	ch->change_map_confirm[5] = 1;		// 6��° ����Ÿ �޾Ҵ�
}

void NewRecvChangeMapData7( k_change_map7 *p, DWORD cn )		// ĳ���� ����Ÿ 7 �ޱ�
{
	CHARLIST *ch = &connections[cn].chrlst;
	if( !ch ) return;
	
	memcpy(ch->bank, p->ch_data.bankitem, SIZE_OF_BANKITEM );

	int i;
	for( i = 0 ; i < 1000 ; i ++)
	{
		if( ch->Item[i] == 0) break;
	}
	ch->ItemMax = i;
	//010607 lsw
	//	CheckCurrStats( ch );		// 0206 YGI

	ch->change_map_confirm[6] = 1;		// 7��° ����Ÿ �޾Ҵ�
}

void NewRecvChangeMapData8( k_change_map8 *p, DWORD cn )		// ĳ���� ����Ÿ 8 �ޱ�
{
	CHARLIST *ch = &connections[cn].chrlst;
	if( !ch ) return;

		ch->believe_god = p->ch_data.believe_god;
		ch->faith  = p->ch_data.faith;
		ch->god_cast_level = p->ch_data.god_cast_level;

	ch->change_map_confirm[7] = 1;		// 8��° ����Ÿ �޾Ҵ�
	ch->now_changing_map = 0;

	CalcCharAbilityByChangemap( ch );					// ĳ���� �ʱ� ����
	CallClient( (short int )cn, (short int )CMD_ACCEPT_JOIN_KEIN );				// ��� �޾Ҵ�.
	memset( ch->change_map_confirm, 0, sizeof( char ) * 9 );
}

void NewRecvChangeMapData9( k_change_map9 *p, DWORD cn )		// ĳ���� ����Ÿ 8 �ޱ�
{
	CHARLIST *ch = &connections[cn].chrlst;
	if( !ch ) return;

		ch->Gender			=	p->Gender;
		ch->Face			=	p->Face	;
		ch->Class			=	p->Class;
		ch->Job				=	p->Job	;
		ch->Spell			=	p->Spell;	
		ch->SprNo			=	p->SprNo	;	
		ch->Peacests			=	p->Peacests	;	
		ch->BodyR			=	p->BodyR	;	
		ch->BodyG			=	p->BodyG	;	
		ch->BodyB			=	p->BodyB	;	
		ch->ClothR			=	p->ClothR	;	
		ch->ClothG			=	p->ClothG	;		
		ch->ClothB			=	p->ClothB	;	

		ch->total_id		=	p->total_id	;		

		ch->nk[N_VYSEUS]		=	p->nk3		;	
		ch->nk[N_ZYPERN]		=	p->nk4		;		
		ch->nk[N_YILSE]			=	p->nk6		;		
    ch->SetTransMap(p->nTransPort, p->nTransX, p->nTransY);
	ch->change_map_confirm[8] = 1;		// 9��° ����Ÿ �޾Ҵ�
	ch->JoinLocalWar		=	p->JoinLocalWar;			// LTS LOCALWAR
	ch->GainedFame			=	p->GainedFame;
	ch->CurrentPhase		=	p->CurrentPhase;
	ch->EventJoin			=	p->EventJoin;				// 020115 LTS

	ch->HorseNo				=	p->HorseNo;					// LTS HORSERIDER
	ch->HorseGrade			=	p->HorseGrade;				// LTS HORSERIDER
	ch->RiderNo				=	p->RiderNo;					// LTS HORSERIDER
	ch->RiderGrade			=	p->RiderGrade;				// LTS HORSERIDER

	ch->SetTradeMode(!p->bIsTradeOn);//030127 lsw		
	ch->nSkillSadCount	= 	p->nSkillSadCount;	//030127 lsw
	ch->nSkillBlessLeaf	=	p->nSkillBlessLeaf;	//030127 lsw
	
	ch->SetBillingType(p->nBillingType);
	ch->m_nUserAge	= p->nUserAge;				// 030929 kyo
}
								
void closeconnection_kein(t_connection c[], int cn, int errnum)
{								
	CHARLIST *ch = &connections[cn].chrlst;

	if( cn == LOGIN_SERVER_CONNECT || cn == DB_DEMON)
	{
		return;
	}

	if( cn < DRAGON_CONNECTIONS_START || cn >= DRAGON_MAX_CONNECTIONS ) 
	{
		return ;
	}

	SendExitGameWho( cn );		// ��Ƽ������ �� �����ٰ� �˸���. // 0116 YGI
	SetArea(REMOVE_PC_AREA, cn);

	::SetTileOccupied(ch->MoveSx,ch->MoveSy,false);

	for( int i = 0 ; i < MAX_PC_CONTROL_NPC ; i ++)						// LTS NPC BUG
	{
		if( ch->ctrlnpc[i] != -1 )
		{
			NPCList[ ch->ctrlnpc[i]].ctrlpc = -1;
			SetArea(ADD_NPC_AREA, ch->ctrlnpc[i]);	
		}
	}

	g_pServerTable->SetNumOfUsers( g_pServerTable->GetNumOfUsers()-1,110, cn, errnum, connections[cn].name);
	//< CSD-HK-030829
	CleanUpConnection(cn);
}	//> CSD-HK-030829
								
int CalcCharAbilityByChangemap( CHARLIST *ch )
{			
	int px, py;
	ch->cur_skillno = 0;
	ch->itemweight		= GetTotalItemWeight( ch ); 
		
	if( ch->Peacests < 0 || ch->Peacests > 1 ) ch->Peacests = 0;

	// Ȥ�� ��ġ�� ��ֹ��� ������ ������ �ű��. 
	px = ch->X / TILE_SIZE;
	py = ch->Y / TILE_SIZE;
	NPC_AdjustPosition( ch->SprNo, &px, &py );
	ch->X = px * TILE_SIZE + 16;
	ch->Y = py * TILE_SIZE + 16;
		
	ch->OldMoveSx = ch->MoveSx = px;
	ch->OldMoveSy = ch->MoveSy = py;
	::SetTileOccupied(ch->MoveSx,ch->MoveSy,true);

	sRECALL_SCRP_NO = 0; // �̺�Ʈ�� �߻��ϸ� �̰��� 0�� �ƴѰ����� �ȴ�.  ������ �ٽ� Setting..
		
	ch->timetoupdate	= g_curr_time;
	
	return 1;
}		
//021030 lsw �ڽ� �������� �����		
void RecvChangeMapConfirm( short int cn )
{
	CHARLIST *ch = CheckServerId( cn );
	if( !ch ) return;

	for( int i=0; i<9; i++ )
		if( !ch->change_map_confirm[i] ) 
		{
			CallClient( cn, CMD_CHANGE_MAP_FAIL);	// ���� ������ �־� �� �Ѱ� ���� ���ߴ�.
			return;
		}
	
	CallClient( cn, CMD_ACCEPT_JOIN_KEIN );			// �� �Ѱ� �޾����ϱ� ��� �����϶�
	memset( ch->change_map_confirm, 0, sizeof( char ) * 9 );
}

////////////////////////////////////////
//
//	ä�� �ܾ �ɷ����� �޼ҵ��
//
///////////////////////////////////////
char FILTER[MAX_WORD][20];			// �ܾ�����
int filter_count;				// ��� �ܾ��� ����
int LoadFilterWord( )
{
	filter_count = 0;

	FILE *fp;
	char temp[ MAX_PATH];
	wsprintf( temp, "%s/data/badword.txt", GameServerDataPath );
	fp = fopen( temp, "rt" );
	if( fp == NULL ) return filter_count;

	while( fscanf(fp, "%s", FILTER[filter_count++] ) != EOF );
	
	fclose(fp);

	return filter_count;
}

void ConvString( char *szStr ) // �弳 ����
{
	return;
	char *cut;
	char str_small[255] = {0,};		// ��� �빮�ڷ� �ٲ۰�...

//	int total_len = strlen( szStr );
//	str_small = new char[total_len];		// �������� �ٲ۰����� ���ؼ� ó���ϰԲ��ϱ� ����

	strcpy( str_small, szStr );
	strlwr( str_small );		// �ҹ��ڷ� �ٲ۴�.

	int i=0;
	while( *FILTER[i] && *FILTER[i] != ' ' )		// ��ĭ�� �ƴ� ���ڰ� ������ ������ 
	{
		while( cut = strstr( str_small, FILTER[i] ) )
		{
			char *check_cut = cut;		// �ѱ��� ó���ΰ� üũ ��ƾ
			int count = 0;
			int ret = 0;
			while( 1 )
			{
				if( str_small >= check_cut )
				{
					if( !(count%2) ) ret = 1; // �ѱ��� ó���̴�...
					break;
				}
				if( !(*check_cut & 0x80) )
				{
					if( count%2 ) ret = 1; // �ѱ��� ó���̴�...
					break;
				}
				check_cut --;
				count++;
			}
			if( !ret ) 
			{
				*cut = ' ';
				continue;		// �ѱ��� ó���� �ƴϴ�.
			}

			int len = strlen( FILTER[i] );
			int point_gab = cut - str_small;			// �ּ��� ����
			for( int j=0; j<len; j++ )	
			{
				*(cut+j) = ' ';
				*(szStr+point_gab+j) = ' ';
			}
		}
		i++;
	}
//	delete str_small;
}



int CompPartyBoth( CHARLIST *ch_1, CHARLIST *ch_2 )
{
	for( int i=0; i<6 ; i++ )
	{
		if( !strcmp ( ch_1->party[i].Name, ch_2->Name ) )
		{
			for( int j=0; j<6; j++ )
			{
				if( !strcmp( ch_2->party[j].Name, ch_1->Name) ) return 1;
			}
		}
	}

	return 0;
}

//011022 lsw >
void SendSkillAll( short int cn )
{
	CHARLIST *ch = CheckServerId( cn );
	if( !ch ) return;

	for( int i=0; i<45; i++ ) 
	{
		if( ch->skillexp[i].skillexp )
		{
			SendStartSkillExp( i, ch->skillexp[i], connections, cn );
		}
	}
}
//011022 lsw <

void RecvAndSendFaithUp( char *name, short int up_point )
{
	int cn = ExistHe( name );
	if( cn == -1 ) return ;
	CHARLIST *ch = CheckServerId( cn );
	if( ch )
	{
		ch->faith += up_point;
		ch->SendCharInfoBasic( FAITH, up_point );//020704 lsw
	}
}

void RecvDivineUp( short int cn )			// �⵵���� ���� ���� ����
{
	CHARLIST *ch = CheckServerId( cn );
	if( !ch ) return;

	if( ch->Spell == PRIEST_SPELL && ch->believe_god && ch->faith>20 && ch->god_cast_level )		// 50�� ������ ��
	{
		if( ch->Mana < (ch->ManaMax * 0.8) ) 
		{
			ch->Mana += ch->faith;
			if( ch->Mana > ch->ManaMax ) ch->Mana = ch->ManaMax;
			ch->SendCharInfoBasic(MP, ch->Mana ); //020704 lsw
		//	ch->TempMana = ch->Mana;//020214 lsw
		}
	}
}

int GetMeetMessageRandomNumber( int god_type, int meet_type )		// ���� �޽��� ��ȣ �������� ( �����ִ� ���߿��� �������� )
{
	int num=0;
	char *message = NULL;
	switch( meet_type )
	{
		case YEAR_MEETING  :	num = rand() % 4;
								message = YearMeetingText[god_type][num];
								break;
		case MONTH_MEETING :	num = rand() % 12;
								message = MonthMeetingText[god_type][num];
								break;
		default : return 0;
	}
	return num;	
}

void RecvOpenSupper( short int cn )
{
	CHARLIST *ch = CheckServerId( cn );
	if( !ch ) return;

	if( !ch->in_temple ) 
	{
		SendServerResult( CM_OPEN_SUPPER, 10, cn );	// �������� �ƴϴ�
		return;
	}
	if( !ch->is_god_meeting )
	{
		SendServerResult( CM_OPEN_SUPPER, 20, cn );	// ���� ���� �ð��� �ƴϴ�
		return;
	}
	if( ch->use_supper )
	{
		SendServerResult( CM_OPEN_SUPPER, 30, cn );	// �̹� ���� �޾Ҵ�.
		return;				
	}

	ch->use_supper = true;
	ch->much_supper = 10;		// ���� ���� �� �ִ� ����

	t_packet p;
	p.h.header.type = CMD_OPEN_SUPPER;
	{
		p.u.kein.server_to_client_open_supper.much= ch->much_supper;
	}
	p.h.header.size = sizeof( k_server_to_client_open_supper);
	QueuePacket(connections, cn, &p, 1);
}

void RecvGetItemOfSupper( short int item_no, short int cn )
{
	CHARLIST *ch = CheckServerId( cn );
	if( !ch ) return;
	
	if( ch->much_supper >= 0)
	{

		int item_kind[] = { 3007, };
		int max = sizeof( item_kind ) / sizeof( int );
		int i;
		for( i=0; i<max; i++ )
		{
			if( item_no == item_kind[i] ) break;
		}
		if( i == max ) return;	// ������� �ʴ� �������̴�. ��ŷ����

		ItemAttr item = GenerateItem( item_no );
		if( !item.item_no ) return;
		int a, b, c;
		if( SearchInv( ch->inv, a, b, c ) )
		{
			POS pos;
			SetItemPos( INV, a, b, c, &pos );
			ch->inv[a][b][c] = item;

			SendServerEachItem( &pos, &item, cn );
			ch->much_supper--;
		}
		else return;				// �κ� â�� �� á��.
	}
}

void CheckInitSkill( CHARLIST *ch )		 // Ŭ���� ��ų�� Ȯ���ϰ�...	// ������� ���� ���� �ϴ� ��ƾ...
{
  const int nCombatStart = LIGHTNING_BOOM - 100;
  const int nCombatEnd = WIND_EXTREME - 100;
	int i;
	switch( ch->Class ) 
	{
		case WARRIOR :	for(  i=0; i<MAX_SKILLMAIN;i++ )	
						{
							if( i >= TACTICS_Crapple && i <= TACTICS_Orison ) continue;
              if (i >= nCombatStart && i <= nCombatEnd)  continue;
							if( SkillTbl[i].Class_Warrior == 0 ) ch->Skill[i] = 0;
							else if( SkillTbl[i].Class_Warrior == 1 ) ch->Skill[i] = 1;
						}
						break;

		case THIEF :	for(  i=0; i<MAX_SKILLMAIN;i++ )
						{
							if( i >= TACTICS_Crapple && i <= TACTICS_Orison ) continue;
              if (i >= nCombatStart && i <= nCombatEnd)  continue;
							if( SkillTbl[i].Class_Thief == 1 ) ch->Skill[i] = 1;
							else if( SkillTbl[i].Class_Thief == 0 ) ch->Skill[i] = 0;
						}
						break;
						
		case ARCHER :	for(  i=0; i<MAX_SKILLMAIN;i++ )
						{
							if( i >= TACTICS_Crapple && i <= TACTICS_Orison ) continue;
              if (i >= nCombatStart && i <= nCombatEnd)  continue;
							if( SkillTbl[i].Class_Archery == 1 ) ch->Skill[i] = 1;
							else if( SkillTbl[i].Class_Archery == 0 ) ch->Skill[i] = 0;
						}
						break;
		case WIZARD :	for(  i=0; i<MAX_SKILLMAIN;i++ )
						{
							if( i >= TACTICS_Crapple && i <= TACTICS_Orison ) continue;
              if (i >= nCombatStart && i <= nCombatEnd)  continue;
							if( SkillTbl[i].Class_Wizard == 1 ) ch->Skill[i] = 1;
							else if( SkillTbl[i].Class_Wizard == 0 ) ch->Skill[i] = 0;
						}
						break;
		case PRIEST :	for(  i=0; i<MAX_SKILLMAIN;i++ ) 
						{
							if( i >= TACTICS_Crapple && i <= TACTICS_Orison ) continue;
              if (i >= nCombatStart && i <= nCombatEnd)  continue;
							if( SkillTbl[i].Class_Priest == 1 ) ch->Skill[i] = 1;
							else if( SkillTbl[i].Class_Priest == 0 ) ch->Skill[i] = 0;
						}
						break;
	}

	switch( ch->Job )		
	{
	case J_TAILOR		:	for(  i=0; i<MAX_SKILLMAIN;i++ ) if( SkillTbl[i].Select_Tailoring	== 1 )		{ ch->Skill[i] = 1; makemyjobexpsetting( i, ch ); }break;
	case J_ALCHEMIST	:	for(  i=0; i<MAX_SKILLMAIN;i++ ) 
							{
								if( i >= TACTICS_Crapple && i <= TACTICS_Orison ) continue;
                if (i >= nCombatStart && i <= nCombatEnd)  continue;
								if( SkillTbl[i].Select_Alchemy == 1 )
								{ 
									ch->Skill[i] = 1; 
									makemyjobexpsetting( i, ch ); 
								}
								else if( SkillTbl[i].Select_Alchemy == 0 )
								{
									ch->Skill[i] = 0;
									int kind = SkillTbl[i].MotherSkillType;
									//011012 lsw >
									ch->skillexp[kind].skillexp = 0;
									//011012 lsw <
								}
							}
							break;
	}

//011030 lsw >
	int ok = 0;
	for(  i=0; i<MAX_SKILLMAIN;i++ ) 
	{
		if( SkillTbl[i].Howto == 100 ) 
		{
		switch(i)
			{
			case 106	:	if(ch->skillexp[CUTDOWN].skillexp == 1000000
								&& ch->skillexp[CARPENTRY].skillexp == 1000000) ok = 1;
				break;
			case 107	:	if(ch->skillexp[MINING].skillexp == 1000000
								&& ch->skillexp[BLACKSMITHY].skillexp == 1000000) ok = 1;
				
				break;
			case 108		:	if(ch->skillexp[FARMING].skillexp == 1000000 ||
									ch->skillexp[FISHING].skillexp == 1000000)
									if(ch->skillexp[COOKING].skillexp == 1000000)  ok = 1;
				
				break;
			case 109		:	if(ch->skillexp[BUTCHERING].skillexp == 1000000
									&&ch->skillexp[TAILORING].skillexp == 1000000) ok = 1;
				
				break;
			case 110	:	if(ch->skillexp[CUTDOWN].skillexp == 1000000
								&&ch->skillexp[BOWCRAFTING].skillexp == 1000000) ok = 1;
				
				break;
			case 111	:	if(ch->skillexp[HERBDIGGING].skillexp == 1000000
								&&ch->skillexp[ALCHEMING].skillexp == 1000000) ok = 1;
				
				break;
			case 112	:	if(ch->skillexp[FISHING].skillexp == 1000000
								&&ch->skillexp[CANDLEMAKING].skillexp == 1000000) ok = 1;
				
				break;
			default:
				ok = 0;
				break;
			}
			if(ok)
			{
				ch->Skill[i] = 1;
				ok = 0;
			}
			else 
			{
				ch->Skill[i] = 0;
				ok = 0;
			}
		}
	}
	SendCharInfoSkill( connections, ch->GetServerID());

//011030 lsw <
}

CHARLIST* CheckNpcId(short int cn)
{	//< CSD-031014
	if (cn < NPC_LIST_START || cn > MAX_NPC_LIST)
	{
		return NULL;
	}

	return &NPCList[cn];
}	//> CSD-031014

CHARLIST* CheckNpcName(const char* pName)
{	//< CSD-031014
	for (int i = NPC_LIST_START; i < MAX_NPC_LIST; ++i)
	{
		if (NPCList[i].bAlive == ALIVE_ && strcmp(NPCList[i].Name, pName) == 0)
		{
			return &NPCList[i];
		}
	}

	return NULL;
}	//> CSD-031014

bool CheckServerIndex( short int id )
{
	if (id <= DB_DEMON || id >= DRAGON_MAX_CONNECTIONS)
	{
		return false;
	}

	return true;
}

void RecvReqCharInfo( int cn, int target )
{
	t_packet p;
	LPCHARLIST n;

	p.h.header.type = CMD_REQ_CHAR_INFO;
		p.u.debug_server_req_char_info.id = target;

		if( target >= 10000 )
		{
			n = CheckNpcId( target-10000 );
			if( !n ) return;
		}
		else 
		{
			n = CheckServerId( target );
			if( !n ) return;
		}

		p.u.kein.server_req_char_info.id = target;

//		p.u.kein.server_req_char_info.lv    = n->Level;
//		p.u.kein.server_req_char_info.exp   = n->Exp; 
//		p.u.kein.server_req_char_info.ac    = n->Ac;
//		p.u.kein.server_req_char_info.Str   = n->Str;					//  ��
//		p.u.kein.server_req_char_info.Con   = n->Con;				//  �ǰ�
//		p.u.kein.server_req_char_info.Dex   = n->Dex;				//	��ø��
//		p.u.kein.server_req_char_info.Wis   = n->Wis;				//	����
//		p.u.kein.server_req_char_info.Int   = n->Int;				//	����
//		p.u.kein.server_req_char_info.MoveP = n->MoveP;				//	�̵���
//		p.u.kein.server_req_char_info.Char  = n->Char;				//	�ŷ�
//		p.u.kein.server_req_char_info.Endu  = n->Endu;				//  �����
//		p.u.kein.server_req_char_info.Moral = n->Moral;				//	���
//		p.u.kein.server_req_char_info.Luck  = n->Luck;				//	���
//		p.u.kein.server_req_char_info.wsps  = n->wsps;				//  ������->���� �޸������
		p.u.kein.server_req_char_info.Hp    = n->Hp;    	
		p.u.kein.server_req_char_info.HpMax = n->HpMax;
//		p.u.kein.server_req_char_info.Mp    = n->Mana;     
//		p.u.kein.server_req_char_info.MpMax = n->ManaMax;
//		p.u.kein.server_req_char_info.Hungry= n->Hungry; 
//		p.u.kein.server_req_char_info.HungryMax = n->HungryMax;
//		p.u.kein.server_req_char_info.event_no = n->eventno;			

	p.h.header.size = sizeof( k_server_req_char_info );
	QueuePacket( connections, cn, &p, 1 );
}


void RecvOhMyParty( char *my_name, char *party_name )			// my_name�� ���������� party_name���� �� ����� �������ش�.
{
	int he = ExistHe( party_name );
	if( he == -1 ) return;
	CHARLIST *party_ch = &connections[he].chrlst;
	if( !party_ch ) return;

	t_packet packet;
	packet.h.header.type = CMD_REQ_JOIN_GAME;
	{
		strcpy( packet.u.kein.client_char_name.name, my_name );
	}
	packet.h.header.size = sizeof( k_client_char_name );
	QueuePacket(connections, he, &packet, 1);
}

void SetShape( unsigned char *equip, short int cn )
{
	memset( equip, 0, 4 ); 
	CHARLIST *ch = CheckServerId( cn );
	if( !ch ) return;

	static int shape_index[] = { WT_ARMOR, WT_HELMET, WT_WEAPON, WT_SHIELD };
	for( int i=0; i<4; i++ )
	{
		CItem *tt = ItemUnit( ch->equip[shape_index[i]] );
		if( tt )
		{
			equip[i] = tt->GetItemShow( ch->Gender );
		}
	}
}

void MoveEmptyInv( ItemAttr *item, CHARLIST *ch )		// �������� ��� �ִ� ���� �ű�� �����.
{
	if( !ch ) return;
	int a, b, c;
	int ret = SearchInv( ch->inv, a, b, c );
	if( ret )
	{
		POS pos;
		SetItemPos( INV, a, b, c, &pos );
		ch->inv[a][b][c] = *item;
		SendServerEachItem( &pos, item, ch->GetServerID());
	}
	else		// �賶�� ������ �� á�ٸ� 
	{
		SendItemEventLog( item, ch->GetServerID(), 0, SILT_PUT_GROUND_MAX_INVEN, 1 ); //YGI acer
		DropItem( ch->X, ch->Y, item );	// ���ٴ����� ����߸���.
	}
	memset( item, 0, sizeof( ItemAttr ) );
}

// ������ ���� ���̺����� ������ ��ȣ�� üũ�Ͽ� �� �Ķ���͸� �����Ѵ�.
int GetItemJoinNumber( int item_num )
{
	for( int i=0; i<ITEM_JOIN_MAX; i++ )
	{
		if( item_join[i].item_id == item_num ) return i;
	}
	return 0;
}

void SetTacSkillLevel( CHARLIST *ch )
{
	for( int i=0; i<= TACTICS_Orison - TACTICS_Crapple; i++ )
	{
    ch->ConvertTacticsLevel(i);
	}
}


extern	int GetRider(CHARLIST* ch);	// LTS HORSERIDER
extern	int GetRiderGrade(CHARLIST* ch);	// LTS HORSERIDER
extern	int GetHorseGrade(CHARLIST* ch);	// LTS HORSERIDER

// �ٲ����� Ȯ���ؼ� �ٲ����� �����ش�.
void CheckAndSendChangeEquip( CHARLIST *ch, int send  )		// send�� 1�̸� ������ �����ش�.
{
	int cn = ch->GetServerID();
	if( !cn ) return;
	if( connections[cn].state == CONNECT_CHANGING_MAP ) return;

	CheckEquipItem( ch );		// 0720 YGI

	unsigned char equip[4];
	SetShape( equip, ch->GetServerID());

	int is_change = 0;

	for( int i=0; i<4; i++ )
		if( equip[i] != ch->accessory[i] ) 
		{
			is_change = true;
			break;
		}

	//011018 KHS ����
	short int mantle = ch->equip[ WT_NECK].item_no;
	if( mantle != ch->mantle )
	{
		ch->mantle = mantle;
		is_change = true;
	}

	short int Chair = ch->equip[ WT_SHOES].item_no;			// LTS SITDOWN BUG
	//CItem* t=ItemUnit(Chair);
	CItem_Etc* tempItem=(CItem_Etc*)ItemUnit(Chair);

	if(tempItem && tempItem->GetItemKind()==IK_CHAIR)
	{
		if (ch->bAlive==1)
		{
			ch->nCurrentAction=1;		
			ch->ChairNum=tempItem->Repair_Skill2;
			is_change = true;
		}
	}
	else
	{
		if (ch->nCurrentAction)
		{
			ch->nCurrentAction=0;		
			ch->ChairNum=0;
			is_change = true;
		}
	}


	short int HorseNo = ch->equip[ WT_SHOES].item_no;			// LTS HORSERIDER

	if(HorseNo>=8041&&HorseNo<=8043)
	{
		if (ch->bAlive==1)
		{
			ch->HorseNo=HorseNo-8040;
			ch->RiderNo=GetRider(ch);
			ch->HorseGrade=0;//GetHorseGrade(ch);//020508 LTS �뷮 ����
			ch->RiderGrade=0;//GetRiderGrade(ch);//020508 LTS �뷮 ����
		/*	ch->HorseNo=2;
			ch->RiderNo=1;
			ch->HorseGrade=1;
			ch->RiderGrade=1;*/
			is_change = true;
		}
	}
	else
	{
		if (ch->HorseNo>0)
		{
			ch->HorseNo=0;
			ch->RiderNo=0;
			ch->HorseGrade=0;
			ch->RiderGrade=0;
			is_change = true;
		}
	}

	// 011030 KHS JJING
	// ���� �����̴��� grade�� �ٸ��� �ٲ��־�� �Ѵ�. 
	int cur_jjing = ((RareMain*)&(ch->equip[ WT_ARMOR].attr[ IATTR_RARE_MAIN]))->grade;
	if( ch->jjing != cur_jjing )
	{
		ch->jjing = cur_jjing;
		is_change = true;
	}

	if( is_change || send )
	{
		t_packet packet;
		memcpy(ch->accessory, equip, 4);

		packet.h.header.type = CMD_CHANGE_EQUIP;
		packet.h.header.size = sizeof(t_server_change_equip);
		packet.u.server_change_equip.server_id= ch->GetServerID();
		memcpy(packet.u.server_change_equip.equip, equip, 4);
		packet.u.server_change_equip.mantle = ch->mantle;
		// 011030 KHS JJING
		packet.u.server_change_equip.jjing = ch->jjing;
		packet.u.server_change_equip.ChairNum=ch->ChairNum;		// LTS SITDOWN BUG
		packet.u.server_change_equip.Horse[0]=ch->HorseNo;		// LTS HORSERIDER
		packet.u.server_change_equip.Horse[1]=ch->RiderNo;		// LTS HORSERIDER
		packet.u.server_change_equip.Horse[2]=ch->HorseGrade;	// LTS HORSERIDER
		packet.u.server_change_equip.Horse[3]=ch->RiderGrade;	// LTS HORSERIDER		
		packet.u.server_change_equip.idWeaponItem = ch->equip[WT_WEAPON].item_no; // CSD-031001
		QueuePacket(connections, ch->GetServerID(), &packet, 1);
		CastMe2Other( ch->GetServerID(), &packet);
	}
}




void RecvRepPartyInfo( char *party_name, short int cn )		// ��Ƽ���� xy ��ǥ�� hp, hpmax���� �����ش�.
{
	int party_id = ExistHe( party_name );
	if( party_id == -1 ) return;
	
	CHARLIST *party = &connections[party_id].chrlst;
	if( !party ) return;

	t_packet packet;

	packet.h.header.type = CMD_REQ_PARTY_INFO;
		strcpy( packet.u.kein.rep_party_info.party_name, party_name);
		packet.u.kein.rep_party_info.x= party->X;
		packet.u.kein.rep_party_info.y= party->Y;
		packet.u.kein.rep_party_info.hp= party->Hp;
	packet.h.header.size = sizeof(k_rep_party_info);
	QueuePacket(connections, cn, &packet, 1);
	//CastMe2Other( cn, &packet);
}

void LoadPublicBbs( )
{
	char *text = NULL;
}

void SendLoginToClientPublicBbsOnlyOne( t_packet *p )
{
	p->h.header.type = CMD_LOGIN_TO_GAME_PUBLIC_BBS;
	QueuePacket(connections, p->u.kein.server_text_only_one.server_id, p, 1);
	return ;
}


void ReturnResultByName( char *name,  int cmd_type )		// �̸����� ���̵� ã�Ƽ� ���������� �������ش�.
{
	int cn = ExistHe( name );
	if( cn == -1 ) return;

	CallClient( cn, cmd_type );
}

void CheckMyMailOnStartGame( short int cn )
{	//< CSD-030324
	CHARLIST *ch = CheckServerId( cn );
	if( !ch ) return;
	t_packet packet;

	packet.h.header.type = CMD_START_GAME_MAIL;
		strcpy( packet.u.kein.start_game_mail.name, ch->Name);
		packet.u.kein.start_game_mail.server_id =  cn;
		packet.u.kein.start_game_mail.guild_code =  ch->GetGuildCode();
	packet.h.header.size = sizeof(k_start_game_mail);

	QueuePacket(connections, DB_DEMON , &packet, 1);
}	//> CSD-030324

void SendToLoginserver( short int cn )
{
	t_packet packet;

	packet.h.header.type = CMD_CALL_PUBLIC_BBS;
		packet.u.kein.server_id.server_id =  cn;
	packet.h.header.size = sizeof(k_server_id);
	QueuePacket(connections, DB_DEMON , &packet, 1);
}

// Modified by chan78 at 2000/11/28
void SendLoginServerHunterRegist( k_hunter_regist *p, int size, short int cn )		// 001212 YGI
{
	CHARLIST *ch = CheckServerId( cn );
	if( !ch ) return;

	if( p->money < 5000000 ) return;		// ���� ������..
	if( ch->Money < p->money ) return;

	if( SubtractMoney( p->money, ch ) != 1 ) return;
	
	t_packet packet;
	packet.h.header.type = CMD_HUNTER_REGIST;
		strcpy( packet.u.kein.hunter_regist2.regist_name, ch->Name);
		strcpy( packet.u.kein.hunter_regist2.target_name, p->name);
		packet.u.kein.hunter_regist2.money = p->money;
		strcpy( packet.u.kein.hunter_regist2.text, p->text);
	packet.h.header.size = sizeof( k_hunter_regist2 )-1000+strlen(p->text)+1;
	QueuePacket(connections, DB_DEMON , &packet, 1);	
}
void SendLoginServerReqHunterList( int id, short int cn )
{
	t_packet packet;
	
	packet.h.header.type = CMD_GET_HUNTER_LIST;
		packet.u.kein.game_login_hunter_list.server_id = cn;
		packet.u.kein.game_login_hunter_list.list_id = id;
	packet.h.header.size = sizeof( k_game_login_hunter_list );
	QueuePacket(connections, DB_DEMON , &packet, 1);
}

void RecvLoginServerAndSendHunterList( k_login_game_hunter_list *login_list )
{
	t_packet packet;
	
	packet.h.header.type = CMD_SEND_HUNTER_LIST;
		packet.u.kein.hunter_list = login_list->list;
	packet.h.header.size = sizeof( k_hunter_list )-1000+strlen( login_list->list.text );
	QueuePacket(connections, login_list->server_id, &packet, 1);
}

void KillWho(CHARLIST*death_ch, CHARLIST *attack_ch)
{	//< CSD-CN-031213
	if (death_ch->IsPlayer())
	{
		CBaseArena* pGame = death_ch->GetJoinArenaGame();

		if (pGame != NULL)
		{
			pGame->SendGameMessage(attack_ch, death_ch);
			
			switch (pGame->GetArenaType())
			{
			case AT_STONE:
			case AT_MONSTER:
				{
					CallMenu(MN_REVIVAL, death_ch->GetServerID());
					break;
				}
			}
		}

		if (attack_ch->IsPlayer())
		{
			SendPacketDefault(CMD_KILL_WHO, death_ch->Name, strlen(death_ch->Name), attack_ch->GetServerID());
		}
		// 030624 YGI
		if( g_EventFlagMgr.IsMoveMapWhenDie() )
		{
			extern bool MoveToVillage(int server_id);
			MoveToVillage(death_ch->GetServerID());
		
			t_packet packet;
			packet.h.header.size = sizeof(t_arena_game_message2);
			packet.h.header.type = CMD_ARENA_GAME_MESSAGE2;
			packet.u.arena.arena_game_message2.nTeamNumber1 = 1;
			packet.u.arena.arena_game_message2.nTeamNumber2 = 1;
			strcpy(packet.u.arena.arena_game_message2.szTeamMember1, attack_ch->Name);
			strcpy(packet.u.arena.arena_game_message2.szTeamMember2, death_ch->Name);
			g_pUserManager->SendPacket(&packet);
		}
	}
}	//> CSD-CN-031213

void RecvHunterList( int id, short int cn )
{ //< CSD-020819
	CHARLIST *ch = CheckServerId( cn );
	if (!ch)              return;
  if (!ch->IsPlayer())  return;

	t_packet packet;
	packet.h.header.type = CMD_KILL_HUNTER_LIST;
	packet.u.kein.kill_hunter_list_login.server_id = cn;
	packet.u.kein.kill_hunter_list_login.list_id= id;
  strcpy( packet.u.kein.kill_hunter_list_login.hunter_name , ch->Name );
	packet.h.header.size = sizeof( k_kill_hunter_list_login );
	QueuePacket(connections, DB_DEMON , &packet, 1);
} //> CSD-020819

// �������� ����� ����� ������ ���� ��ƾ
// ���������� �˷��ش�.
void RecvHunterListOk( k_send_kill_hunter_list *p )
{
	CHARLIST *ch = ::CheckServerId(p->server_id);
	if(!ch){return;}
	if(!ch->IsPlayer())  {return;}

	::AddMoney(p->money,ch);//021126 lsw// ���� �÷� �ش�.

	t_packet packet;				// �����ߴ�..
	packet.h.header.type = CMD_KILL_HUNTER_LIST_OK;
	packet.u.kein.kill_hunter_ok.money= p->money;
	packet.h.header.size = sizeof( k_kill_hunter_ok );
	QueuePacket(connections, p->server_id, &packet, 1);
} 

void ReturnDangerByHunter( short int target_id, char type)
{
	t_packet packet;				// �����ߴ�..
	packet.h.header.type = CMD_DANGER_OF_HUNTER;
		packet.u.kein.default_char= type;
	packet.h.header.size = sizeof( char );
	QueuePacket(connections, target_id, &packet, 1);
}

void RecvReqQuestIndex( char page, short int cn )
{
	t_packet packet;
	packet.h.header.type = CMD_REP_QUEST_INDEX;

	int start = page*14;
	int para = 0;
	for( int i=QUEST_START, ct=0; i<_SCRIPT_VARABLE_POINT_ ; i++ )
	{
		if( var[cn][i] ) 
		{
			if( start <= ct )
			{
				packet.u.kein.rep_quest_index.quest[para].index = i;
				packet.u.kein.rep_quest_index.quest[para].ret = var[cn][i];
				para++;
				if( para > 13 ) break;
			}
			ct++;
		}
	}
		packet.u.kein.rep_quest_index.max = para;

	packet.h.header.size = sizeof(k_rep_quest_index)-( (14-para)* sizeof(k_quest_unit) );
	QueuePacket(connections, cn, &packet, 1);
}

void SendLearnedItem( t_connection c[], short int cn )	
{
	t_packet packet;

	packet.h.header.type = CMD_SEND_LEARN_ITEM_INDEX;
	{
		packet.u.kein.server_learn_item.item_count = c[cn].chrlst.ItemMax;
		memcpy( packet.u.kein.server_learn_item.item, c[ cn].chrlst.Item,	sizeof( short int ) * c[cn].chrlst.ItemMax );
	}
	packet.h.header.size = sizeof( k_server_learn_item );
	QueuePacket( c, cn, &packet, 1);
}

void SendDeleteBox( DWORD box_id )
{
	t_packet packet;

	packet.h.header.type = CMD_DELETE_BOX_ITEM;
	{
		packet.u.kein.default_dword = box_id;
	}
	packet.h.header.size = sizeof( DWORD );
	QueuePacket( connections, DB_DEMON, &packet, 1);
}

bool IsConnectLogin(int idUser)
{	//< CSD-030515
	if (connections[idUser].dwAgentConnectionIndex == 0)
	{
		return false;
	}

	if (connections[idUser].state < CONNECT_LOGIN)
	{
		return false;
	}

	return true;
}	//> CSD-030515

bool IsConnectJoin(int idUser)
{	//< CSD-030515
	if (connections[idUser].dwAgentConnectionIndex == 0)
	{
		return false;
	}

	if (connections[idUser].state < CONNECT_JOIN)
	{
		return false;
	}

	return true;
}	//> CSD-030515

CHARLIST* CheckServerId(WORD id)
{	//< CSD-HK-030829
	if (id <= DB_DEMON || id >= DRAGON_MAX_CONNECTIONS)
	{
		return NULL;
	}

	if (connections[id].dwAgentConnectionIndex == 0)
	{
		return NULL;
	}

	if (connections[id].state < CONNECT_LOGIN)
	{
		return NULL;
	}

	return &connections[id].chrlst;
}	//> CSD-HK-030829

CHARLIST* CheckServerName( const char* szName )		// 030224 kyo �̸����� ã�´�. 
{	//< CSD-030320
	return g_pUserManager->GetCharList(szName);
}	//> CSD-030320

CHARLIST* GetCharListPtr(WORD idObject)
{
	if (idObject >= 10000)
	{
		idObject = idObject - 10000;
		
		if (idObject >= NPC_LIST_START && idObject < MAX_NPC_LIST)
		{
			return &NPCList[idObject];
		}
	}
	else
	{
		if (idObject >= DRAGON_CONNECTIONS_START && idObject < DRAGON_MAX_CONNECTIONS)
		{
			return &connections[idObject].chrlst;
		}
	}
	
	return NULL;
}

int SendServerEachItemAuto( CHARLIST *ch, ItemAttr *item )		// ����� ã�� �����ش�. ����� ������ 1�� ���� ������ 0�� ����
{
	int a, b, c;
	int ret;

	if( SearchInv( ch->inv, a, b, c ) )
	{
		POS pos;
		SetItemPos( INV, a, b, c, &pos );
		SendServerEachItem( &pos, item, ch->GetServerID());
		ch->inv[a][b][c] = *item;
		ret = 1;
	}
	else	// ���� ���� �߸��� 0�� ����
	{
		SendItemEventLog( item, ch->GetServerID(), 0, SILT_PUT_GROUND_MAX_INVEN, 1 ); //YGI acer
		DropItem( ch->X, ch->Y, item );
		ret = 0;
	}
	memset( item, 0, sizeof( ItemAttr ) );
	return ret;
}
void RecvReqManToManRefusal( short int target_id, short int cn )
{
	CHARLIST *me = CheckServerId( cn );
	if( !me ) return;

	RecvReqManToManRefusal( target_id, me );
}

void RecvReqManToManRefusal( short int target_id, CHARLIST *ch )
{
	if( ch->fight_flag != 2 ) return;
	ResultManToMan( ch->GetServerID(), target_id, 3 );
}

///////////////////////////////////////////////////////////////////////
void SendDefaultResult( int type, char ret, short int cn )
{
	t_packet p;
	p.h.header.type = type;
		p.u.kein.default_char = ret;
	p.h.header.size = sizeof( char );
	QueuePacket(connections, cn, &p, 1);
}



int ServerEachItemAuto( CHARLIST *ch, ItemAttr *item )		// ����� ã�� �������� �����Ѵ�. Ŭ���̾�Ʈ�� �������� �ʴ´�.
{
	int a, b, c;
	int ret;
	if( SearchInv( ch->inv, a, b, c ) )
	{
		ch->inv[a][b][c] = *item;
		ret = 1;
	}
	else	// ���� ���� �߸��� 0�� ����
	{
		DropItem( ch->X, ch->Y, item );
		ret = 0;
	}
	memset( item, 0, sizeof( ItemAttr ) );
	return ret;
}
void ResetManToManItemWhenCloseConnect( CHARLIST *you )
{
	RecvReqManToManRefusal( you->fight_id, you );
}

void RecvChangeScriptValue( t_packet *p, short int cn)
{
	int script_num = p->u.kein.script_change.script_num;
	int value = p->u.kein.script_change.value;

	if( script_num < 0 || script_num > _SCRIPT_VARABLE_POINT_  ) return;
	var[cn][script_num] = value;
}

bool IsSameNation( CHARLIST *party_ch, CHARLIST *ch )
{
	if( party_ch->name_status.nation == ch->name_status.nation ) return true;
	else return false;
}

void CheckBoxAndDownDuration( )
{
	for( int i=DRAGON_CONNECTIONS_START; i<DRAGON_MAX_CONNECTIONS-10; i+=10 )
	{
		int ct=i+rand()%10;
		if( connections[ct].dwAgentConnectionIndex && connections[ct].state >= CONNECT_JOIN )
		{
			for( int a=0; a<3; a++ )
				for( int b=0; b<3; b++ )
					for( int c=0; c<3; c++ )
					{
						ItemAttr &item = connections[ct].chrlst.inv[a][b][c];
						if( item.item_no )
						{
							POS pos;
							SetItemPos( INV, a, b, c, &pos );
							CItem *t = ItemUnit( item );
							if( t )
							{
								if( t->GetItemKind() == IK_BOX )
								{
									WORD d_curr, d_max;
									GetItemDuration( item, d_curr, d_max );
									if( d_curr )
									{
										d_curr--;
										if( d_curr > 100 ) 
											d_curr = d_max = 100;
										
										if( d_curr < 10 ) 
										{
											// 020428 YGI acer
											BreakItemByDuration( &item, &connections[ct].chrlst, pos );
										}
										else
										{
											item.attr[IATTR_DURATION] = MAKELONG( d_curr, d_max );
											SendChangeDurationByKein( &pos, item.attr[IATTR_DURATION], ct );
										}
									}
									// 020428 YGI acer
									else BreakItemByDuration( &item, &connections[ct].chrlst, pos );
								}
							}
						}
					}
		}
	}
}


void SendChangeDurationByKein( POS *pos, DWORD dur, int cn )
{
	t_packet p;
	p.h.header.type = CMD_CHANGE_DURATION;
		p.u.kein.change_dur.pos = *pos;
		p.u.kein.change_dur.dur = dur;
	p.h.header.size = sizeof( k_change_dur );
	QueuePacket(connections, cn, &p, 1);
}

void SendCheckName( short int cn, char *name )		// ��� �������� ĳ���� ���� ���� ��û ��ƾ
{
	t_packet p;

	p.h.header.type = CMD_EXIST_NAME;
	{
		p.u.kein.exist_name.server_id = cn;
		strcpy( p.u.kein.exist_name.name, name );
	}
	p.h.header.size = sizeof( k_exist_name ) - 20+strlen( name ) +1 ;
	QueuePacket(connections, DB_DEMON, &p, 1);
}

void SendCheckNameForNation( char *name, short int cn )
{
	t_packet p;
	CHARLIST *ch = CheckServerId( cn );
	if( !ch ) return;

	p.h.header.type = CMD_ISSAME_MY_NATION;
	{
		p.u.kein.exist_name_and_nation.server_id = cn ;
		p.u.kein.exist_name_and_nation.nation= ch->name_status.nation;
		strcpy( p.u.kein.exist_name_and_nation.name, name );
	}
	p.h.header.size = sizeof( k_exist_name_and_nation ) - 20+strlen( name ) +1 ;
	QueuePacket(connections, DB_DEMON, &p, 1);
}



//////////////////////////////////////////////////////////////////////
//
//		 ��ǥ �ý���
//

// ���� ����
int	g_CurrVote[6];		// ���� ��ǥ���ΰ�?	0:�ƴϴ�.	1:�ĺ� ��ϱⰣ	2:���� �Ⱓ		
int	g_VoteNumber[6];	// ��ǥ ��ȣ
int g_vote_reset;		// ��ǥ�� ���õ� ���� �ٽ� ���� �ð����� �����Ѵ�.
typedef struct tag_server_candidate
{
	k_candidate	data;
	int			score;
}k_server_candidate;

k_server_candidate g_Candidate[6][3];

int CanResistCandidate( CHARLIST *ch )
{	//< CSD-030806
	if (ch->GetLevel() < 74)
	{	// level�� �����ϴ�
		return -1;
	}
	
	if (ch->name_status.guild_master != 1)
	{	// ��� �����Ͱ� �ƴϴ�.
		return -2;
	}

	if (ch->nk[ch->name_status.nation] > 0)
	{	// �����̴�
		return -3;
	}

	return 1;
}	//> CSD-030806

void RecvRegistCandidate( short int cn )
{	//< CSD-030324
	CHARLIST *ch = CheckServerId( cn );
	if( !ch ) return;

	if( g_CurrVote[ch->name_status.nation] != 1 ) return;		// ���� ����� �ް� ���� �ʴ�.
	if( CanResistCandidate( ch ) < 0 ) return;		// �ڰ��� ����.

	t_packet p;
	p.h.header.type = CMD_REGIST_CANDIDATE;
	{
		strcpy( p.u.kein.regist_candidate.candidate.name, ch->Name );
		p.u.kein.regist_candidate.server_id	= cn;
		p.u.kein.regist_candidate.candidate.number		= g_VoteNumber[ch->name_status.nation];
		p.u.kein.regist_candidate.candidate.lv			= ch->GetLevel(); // CSD-030806 
		p.u.kein.regist_candidate.candidate.class_type	= ch->Class;
		p.u.kein.regist_candidate.candidate.gender		= ch->Gender;
		p.u.kein.regist_candidate.candidate.nation		= ch->name_status.nation;
		p.u.kein.regist_candidate.candidate.guild_code	= ch->GetGuildCode();
		p.u.kein.regist_candidate.candidate.face		= ch->Face;
	}
	p.h.header.size = sizeof( k_regist_candidate) - 20+strlen( ch->Name ) +1 ;
	QueuePacket(connections, DB_DEMON, &p, 1);	
}	//> CSD-030324

void RecvAndSendLoginCheckRegistCandidate( short int cn )
{
	CHARLIST *ch = CheckServerId( cn );
	if( !ch ) return;

	int ret = CanResistCandidate( ch );
	if( ret < 0 ) 
	{
		CallClient( cn, CMD_THIS_MAN_IS_CANDIDATE_NOT);
		return;
	}

	//////// ��ǥ���� ������ �ٽ� �о� ���� ���� ���� ����
	static DWORD old_time = g_curr_time;
	if( g_vote_reset > 200 ) g_vote_reset= 0;
	if( g_vote_reset==0 || g_curr_time - old_time > 3600 ) 
	{
		old_time = g_curr_time;
		CallClient( DB_DEMON, CMD_SET_VALUE_OF_VOTE );
		g_vote_reset++;
		CallClient( cn, CMD_RETRY );
		return;
	}
	g_vote_reset++;
	///////////////////////////////////////////////////////

	t_packet p;
	p.h.header.type = CMD_IS_THIS_MAN_CANDIDATE;
	{
		p.u.kein.is_this_man_candidate.server_id = cn;
		p.u.kein.is_this_man_candidate.vote_num = g_VoteNumber[ch->name_status.nation];
		strcpy( p.u.kein.is_this_man_candidate.name, ch->Name );
	}
	p.h.header.size = sizeof( k_is_this_man_candidate ) - 20+strlen( ch->Name ) +1 ;
	QueuePacket(connections, DB_DEMON, &p, 1);
}

void CheckVote( short int cn )
{
	CHARLIST *ch = CheckServerId( cn );
	if( !ch ) return;

	//////// ��ǥ���� ������ �ٽ� �о� ���� ���� ���� ����
	static DWORD old_time = g_curr_time;
	if( g_vote_reset > 400 ) g_vote_reset = 0;
	if( g_vote_reset==0 || g_curr_time - old_time > 3600 ) 
	{
		old_time = g_curr_time;
		CallClient( DB_DEMON, CMD_SET_VALUE_OF_VOTE );
		g_vote_reset++;
		CallClient( cn, CMD_RETRY );
		return;
	}
	g_vote_reset++;
	///////////////////////////////////////////////////////

	if( !ch->name_status.nation ) 
	{
		ReturnCheckVoteNot( cn, -2 );	// ���� ����.
		return;
	}
	if( g_CurrVote[ch->name_status.nation] != 2 ) 
	{
		ReturnCheckVoteNot( cn, -1 );	// ���� �Ⱓ�� �ƴϴ�.
		return;
	}
	if( ch->vote_num >= g_VoteNumber[ch->name_status.nation] )
	{
		ReturnCheckVoteNot( cn, -3 );// �̹� ���Ÿ� �ߴ�.
		return;
	}

	// DB-DEMON���� ��û
	t_packet p;
	p.h.header.type = CMD_CHECK_VOTE;
	{
		p.u.kein.check_vote.server_id = cn;
		p.u.kein.check_vote.vote_num = g_VoteNumber[ch->name_status.nation];
		p.u.kein.check_vote.nation = ch->name_status.nation;
		strcpy( p.u.kein.check_vote.login_id, connections[cn].id );
	}
	p.h.header.size = sizeof( k_check_vote )-20+strlen(connections[cn].id)+1;
	QueuePacket(connections, DB_DEMON , &p, 1);
}

int ReturnCheckVoteNot( short int client_id, int ret )
{
	CHARLIST *ch = CheckServerId( client_id );
	if( !ch ) return 0;

	if( ret == -3 )
	{
		ch->vote_num = g_VoteNumber[ch->name_status.nation];
	}

	t_packet p;
	p.h.header.type = CMD_CHECK_VOTE_NOT;
		p.u.kein.default_char = ret;
	p.h.header.size = sizeof( char );
	QueuePacket(connections, client_id, &p, 1);

	return 1;
}

void RecvAndSendCheckVote_Ok( t_packet *p, short int cn )
{
	short int server_id = p->u.kein.check_vote_ok.server_id;

	int count = p->u.kein.check_vote_ok.candidate_count;
	if( count < 0 || count >= 3 ) return;
	int nation = p->u.kein.check_vote_ok.candidate[0].nation;
	
	t_packet packet;
	packet.h.header.type = CMD_CHECK_VOTE_OK;
	{
		packet.u.kein.check_vote_client_ok.candidate_count = count;
		for( int i=0; i<count; i++ )
		{
			g_Candidate[nation][i].data = p->u.kein.check_vote_ok.candidate[i];
			packet.u.kein.check_vote_client_ok.candidate[i] = p->u.kein.check_vote_ok.candidate[i];
		}
		packet.h.header.size = sizeof( k_check_vote_client_ok ) - sizeof( k_candidate )*(3-count );
	}
	QueuePacket(connections, server_id, &packet, 1);
}

void RecvAndSendRegistCandidateResult( t_packet *p )
{
	short int server_id = p->u.kein.login2game_ear_message_result.send_server_id;

	t_packet packet;
	packet.h.header.type = p->h.header.type;
		packet.u.kein.default_char = p->u.kein.login2game_ear_message_result.ret;
	packet.h.header.size = sizeof( char );
	QueuePacket(connections, server_id, &packet, 1);
}

void RecvSetValueOfVote( t_packet *p )
{
	k_set_value_of_vote *target = &p->u.kein.set_value_of_vote;

	for( int i=0; i<6; i++ )
	{
		g_CurrVote[i] = target->is_voting[i];
		g_VoteNumber[i] = target->vote_num[i];
	}
}

void SendVoteGame2Login( t_packet *t, short int cn )
{
	CHARLIST *ch = CheckServerId( cn );
	if( !ch ) return;

	int nation = ch->name_status.nation;
	if( !nation ) return;		// ���� ����

	if( g_CurrVote[nation] < 0 ) return;
	if( g_VoteNumber[nation] < 0 ) return;

	int support = t->u.kein.default_char;

	t_packet p;
	p.h.header.type = CMD_VOTE_GAME2LOGIN;
	{
		p.u.kein.vote_game2login.server_id = cn;
		p.u.kein.vote_game2login.vote = g_VoteNumber[nation];
		p.u.kein.vote_game2login.support = support;
		p.u.kein.vote_game2login.nation = nation;
		strcpy( p.u.kein.vote_game2login.login_id, connections[cn].id );
	}
	p.h.header.size = sizeof( k_vote_game2login ) - 20+strlen( connections[cn].id ) +1 ;
	QueuePacket(connections, DB_DEMON, &p, 1);
}

void SendClientVoteScore( int nation, short int server_id )		// Ŭ���̾�Ʈ�� ���� �����ش�.
{
	int i =0;
	t_packet p;
	p.h.header.type = CMD_REP_VOTE_SCORE;
	{
		for( int x=0; x<3; x++ )
		{
			if( g_Candidate[nation][i].data.name[0] )
			{
				p.u.kein.client_rep_vote_score.name_score[i].score = g_Candidate[nation][i].score;
				strcpy( p.u.kein.client_rep_vote_score.name_score[i].name, g_Candidate[nation][i].data.name );
				i++;
			}
		}
		p.u.kein.client_rep_vote_score.max = i;
	}
	p.h.header.size = sizeof( k_client_rep_vote_score ) - sizeof( k_score_and_name )*(3-i);
	QueuePacket(connections, server_id, &p, 1);
}

void ReqVoteScore( short int cn )
{
	CHARLIST *ch = CheckServerId( cn );
	if( !ch ) return;

	int nation = ch->name_status.nation;
	if( !nation ) return;
	if( !g_CurrVote[nation] || !g_VoteNumber[nation] ) return;

	static int old_time = g_curr_time;

	int is_req = 0;		// ��񿡼� �о�;� �ϴ°�?
	if( g_curr_time - old_time > 360 ) 
	{
		is_req = 1;
		old_time = g_curr_time;
	}

	if( is_req || !g_Candidate[nation][0].data.name[0] || !g_Candidate[nation][0].score )
	{
		// ��񿡼� �о� �´�. ( ��û )
		t_packet p;
		p.h.header.type = CMD_REQ_VOTE_SCORE;
		{
			p.u.kein.req_vote_score.server_id = cn;
			p.u.kein.req_vote_score.vote_num = g_VoteNumber[nation];
			p.u.kein.req_vote_score.nation = nation;
		}
		p.h.header.size = sizeof( k_req_vote_score );
		QueuePacket(connections, DB_DEMON, &p, 1);		
	}
	else
	{
		// ���⼭ �׳� ���� �����ش�.
		SendClientVoteScore( nation, cn);
	}
}

void SendClientRepVoteScore( t_packet *p )
{
	k_score_and_name *target = p->u.kein.rep_vote_score.name_score;
	int server_id = p->u.kein.rep_vote_score.server_id;
	int max = p->u.kein.rep_vote_score.max;

	CHARLIST *ch = CheckServerId( server_id );
	if( !ch ) return;

	for( int i=0; i<max; i++ )
	{
		g_Candidate[ch->name_status.nation][i].score = target[i].score;
		strcpy( g_Candidate[ch->name_status.nation][i].data.name, target[i].name );
	}
	SendClientVoteScore( ch->name_status.nation, server_id );
}

////////////////////////////////////////////////////////////
// �����ֱ� ����
void RecvGetSkillInfo( t_packet *p, short int cn )
{//021113 lsw
	short int target_id = p->u.kein.default_short_int;
	CHARLIST *target_ch = CheckServerId( target_id );
	if( !target_ch ) return;

	const int iSkillMasterType = GMMgr.IsSkillMaster(target_ch);
	if(iSkillMasterType)//�⼺�� ��쵵 ������ �⼺�̴�
	{//�켱 ������ �⼺�� ���� �ö�´�
		::SendSkillInfo( target_id, 1000000, iSkillMasterType, cn );
		return;
	}
	
	if(GMMgr.IsMerchant(target_ch))//�����̸� ������ ���ιۿ� �ȵȴ�
	{
		::SendSkillInfo( target_id, 1000000, GM_QUEST_MERCHANT, cn );
		return;
	}

	if( target_ch->view_job > 0 )	// �̹� ���ǵ� ������ �ִٸ�
	{
		const int view_job			= target_ch->view_job;
		const int iSkillExpIndex	= g_GmMain[view_job].skill;
		const int iExp = target_ch->skillexp[iSkillExpIndex].skillexp;
		if(iExp) 
		{
			::SendSkillInfo( target_id, iExp, view_job, cn );
			return;
		}
	}
	else
	{
		int top_exp = 0;
		int top_jop = 0;
		for( int i=0; i<MAX_GM_MAIN; i++ )
		{
			const int iSkillExpIndex = g_GmMain[i].skill;
			if(	1== g_GmMain[i].job_level
			||	2== g_GmMain[i].job_level)
			{
				if( target_ch->skillexp[iSkillExpIndex].skillexp > top_exp )
				{
					top_exp = target_ch->skillexp[iSkillExpIndex].skillexp;
					top_jop =i;
				}
			}
		}
		target_ch->view_job = top_jop;
		::SendSkillInfo( target_id, top_exp, top_jop, cn );
	}
}

void SendSkillInfo( short int target_id, int top_exp, int top_job, short int cn ,const bool bCast)
{//021113 lsw
	t_packet p;
	int skill_lv = top_exp / 10000;
	p.h.header.type = CMD_GET_SKILL_INFO;
	{
		p.u.kein.get_skill_info.target_id	= target_id;
		p.u.kein.get_skill_info.job			= top_job;		// 1�� ���ؼ� ������ ����ϴµ��� 1�� ���� ����Ѵ�.
		p.u.kein.get_skill_info.call_lv		= skill_lv;
	}
	p.h.header.size = sizeof( k_get_skill_info );
	QueuePacket(connections, cn, &p, 1);
	if(bCast)
	{
		::CastMe2Other( cn, &p);
	}
}

void RecvClienNationMassage( t_packet *p, short int cn )		// ������ Ȯ���� �Ǹ� �޽����� �α��� ������ ������.
{
	char *text = p->u.kein.default_msg;
	CHARLIST *ch = CheckServerId( cn );
	if( !ch ) return;

	if( !ch->name_status.king ) return;

	t_packet packet;
	packet.h.header.type = CMD_SEND_ALL_MSG;
	{
		packet.u.kein.send_login_nation_msg.nation	= ch->name_status.nation;
		strcpy( packet.u.kein.send_login_nation_msg.msg, text );
	}
	packet.h.header.size = sizeof( k_send_login_nation_msg ) - MAX_MSG + strlen( text )+1;

	// Modified by chan78 at 2000/12/01 BroadCast It to MapServers.
	g_pServerTable->BroadCastToEveryServer( (char*)&packet, (sizeof(t_header)+packet.h.header.size), SERVER_TYPE_MAP );
}

extern void	SendPacket2NationClients(const int Nation,t_packet* p);	// CSD-030415

void RecvAndSendNationMassage( t_packet *p )
{	//< CSD-030415
	const int nation = p->u.kein.send_login_nation_msg.nation;
	if( nation <= 0 || nation >= 6 ) return;
	char *text = p->u.kein.send_login_nation_msg.msg;
	
	t_packet packet;
	packet.h.header.type = CMD_RECV_ALL_MSG;
	packet.h.header.size = strlen(text)+1;
	strcpy(packet.u.kein.default_msg, text);
	SendPacket2NationClients(nation, &packet);
}	//> CSD-030415

void RecvDemonYouAreKing( t_packet *p, int type )
{
	//Ȥ�� �̰��� ������ 5000�� ���� �Ȱ��� �ȴ�.
	int ret = ExistHe( p->u.kein.default_name );
	CHARLIST *ch = CheckServerId( ret );
	if( ch )
	{
		if( type == CMD_YOU_ARE_KING_LOG2CLIENT )
			ch->name_status.king = 1;
		else ch->name_status.king = 0;
		CallClient( ret, type );
	}
	else QueuePacket(connections, LOGIN_SERVER_CONNECT, p, 1);		// �α��� ������ ����
}

void RecvLoginYouAreKing( t_packet *p, int type )
{
	int ret = ExistHe( p->u.kein.default_name );
	CHARLIST *ch = CheckServerId( ret );
	if( ch )
	{
		if( type == CMD_YOU_ARE_KING_LOG2CLIENT )
			ch->name_status.king = 1;
		else ch->name_status.king = 0;
		CallClient( ret, type );
	}
}

void RecvGuildExit( t_packet *p, short int cn )		// ��� Ż�� ����
{	//< CSD-030326
	const int target_id = p->u.kein.default_short_int;
	// ��忡 ����� ĳ����
	CHARLIST* target = CheckServerId(target_id);
	
	if (target == NULL)
	{
		return;
	}
	// ��� ������
	CHARLIST* ch = CheckServerId(cn);

	if (ch == NULL)
	{
		return;
	}
	//  ���� ��忩�� �Ѵ�. ( ���� ���� ���� )
	if (ch->GetGuildCode() <= 0 || ch->name_status.guild_master != 1) 
	{
		return;	// ��� �����͸� ����Ҽ� �ִ� ���ɾ��.
	}

	const int nGuildCode = target->GetGuildCode();

	if (ch->GetGuildCode() != nGuildCode) 
	{
		return;
	}
	// ��� ��� ����Ʈ���� ����
	g_pGuildManager->DelMember(nGuildCode, target_id);
	// ��� ���� �ʱ�ȭ
	target->SetGuildCode(0); // CSD-030806
	target->name_status.guild_master = 0;
	// ��� ��� ����Ʈ ���̺����� ����
	SendSaveGuildMemberList(target->Name, nGuildCode, 0, 0);
	// Ż����ѳ�����
	CallClient(target_id, CMD_GUILD_EXIT_YOU);
	// ��� ����
	t_packet packet;					
	packet.h.header.type = CMD_GUILD_EXIT_WHO;
	packet.u.kein.default_short_int = target_id;
	packet.h.header.size = sizeof(short int);
	QueuePacket(connections, cn, &packet, 1); // ��� �����Ϳ���
	CastMe2Other(target_id, &packet); // �ֺ��������
}	//> CSD-030326

void RecvGuildJoin( t_packet *p, short int cn )
{	//< CSD-030324
	int target_id = p->u.kein.default_short_int;
	if( !target_id ) return;

	CHARLIST *target = CheckServerId( target_id );			// ��忡 ����� ĳ����
	if( !target ) return;

	CHARLIST *ch = CheckServerId( cn );						// ��� ������
	if( !ch ) return;

	if( !CheckGuildPower( GDP_JOINING, ch ) ) return;
	if( target->GetGuildCode() ) return;	// �ٸ� ��忡 ���ԵǾ� ������ �ȵȴ�.
	if( target->name_status.nation != ch->name_status.nation ) return;		// ���� ����� �Ѵ�.

	t_packet packet;					

	packet.h.header.type = CMD_GUILD_JOIN_SERVER;			// DB_DEMON ����...
		packet.u.kein.guild_join_server.target_id = target_id;
		packet.u.kein.guild_join_server.server_id = cn;	
		packet.u.kein.guild_join_server.guild_code = ch->GetGuildCode();
		strcpy( packet.u.kein.guild_join_server.id, connections[target_id].id );
	packet.h.header.size = sizeof( k_guild_join_server );
	QueuePacket(connections, DB_DEMON, &packet, 1);
}	//> CSD-030324

void RecvGuildJoinResult( t_packet *p )
{	//< CSD-030324
	int target_id = p->u.kein.guild_join_result.target_id;
	int server_id = p->u.kein.guild_join_result.server_id;

	CHARLIST *target = CheckServerId( target_id );
	CHARLIST *guild_master = CheckServerId( server_id );

	if( !target || !guild_master ) return;

	if( p->h.header.type == CMD_GUILD_JOIN_SERVER_OK )
	{
		// ��� ������ �ƴ�.

		t_packet packet;
		packet.h.header.type = CMD_GUILD_JOIN_CONFIRM;
			packet.u.kein.default_short_int = server_id;		// �渶�� ���̵� �����ش�.
		packet.h.header.size = sizeof( short int );
		QueuePacket(connections, target_id, &packet, 1);

		CallClient(server_id, CMD_GUILD_JOIN_CONFIRM_GUILMA );	// Ȯ�� ���̶�� ����� �渶���� �����ش�.


		//int guild_code = target->GetGuildCode() = guild_master->GetGuildCode();
		//target->name_status.guild_master = 0;

		/*	 ����ڿ��� ���� ���� ����.
		t_packet packet;
		packet.h.header.type = CMD_GUILD_JOIN_SERVER_OK;
			packet.u.kein.guild_join_ok.guild_code = guild_code;
			packet.u.kein.guild_join_ok.target_id = target_id;
		packet.h.header.size = sizeof( k_guild_join_ok );
		QueuePacket(connections, target_id, &packet, 1);
		CastMe2Other( target_id, &packet );
		*/
	}
	else		// �渶���Ը� �����ش�.		// ���� ����
	{
		t_packet packet;
		packet.h.header.type = CMD_GUILD_JOIN_SERVER_NOT;
			packet.u.kein.default_short_int = target_id;
		packet.h.header.size = sizeof( short int );
		QueuePacket(connections, server_id, &packet, 1);
	}
}	//> CSD-030324

void RecvGuildJoinConfirmOk( t_packet *p, short int cn )
{	//< CSD-030806
	int guild_id = p->u.kein.default_short_int;

	CHARLIST *target = CheckServerId( cn );
	CHARLIST *guild_master = CheckServerId( guild_id );

	if( !target || !guild_master ) return;

	if( !CheckGuildPower( GDP_JOINING, guild_master ) ) return;		// ��� �����Ϳ��� �Ѵ�.// 020620 YGI

	const int nGuildCode = guild_master->GetGuildCode();
	target->SetGuildCode(nGuildCode);	
	target->name_status.guild_master = 0;

	t_packet packet;
	packet.h.header.type = CMD_GUILD_JOIN_SERVER_OK;
	packet.u.kein.guild_join_ok.guild_code = nGuildCode;
	packet.u.kein.guild_join_ok.target_id = cn;
	packet.h.header.size = sizeof( k_guild_join_ok );
	QueuePacket(connections, cn, &packet, 1);
	CastMe2Other( cn, &packet );
	// ��� �������� ���� guild_member_list ���̺��� �����Ѵ�.
	SendSaveGuildMemberList( target->Name, nGuildCode, nGuildCode, 0 );
	g_pGuildManager->AddMember(nGuildCode, cn);
}	//> CSD-030806

void RecvGuildJoinConfirmNot( t_packet *p, short int cn )		// �渶���� �װ� �����ߴٴ� �� �˸���.
{
	int guild_id = p->u.kein.default_short_int;

	t_packet packet;
	packet.h.header.type = CMD_GUILD_JOIN_CONFIRM_NOT;
	packet.h.header.size = 0;
	QueuePacket(connections, guild_id, &packet, 1);
}

void RecvScanTarget( short int cn )
{	//< CSD-CN-031213
	CHARLIST *ch = CheckServerId( cn );
	if( !ch ) return;
	if (!ch->JoinLocalWar) return;				// 011217 LTS
	if( ch->scan_time )
	{
		if( (g_curr_time-ch->scan_time) < 20 ) return;		// �ð��� ���ݴ� ������ �Ѵ�.
	}
	ch->scan_time = g_curr_time;

	t_packet packet;
	packet.h.header.type = CMD_SCAN_TARGET;
	char &count = packet.u.kein.scan_target.count;
	count = 0;

	CUserManager::HASH_USER mpUser = g_pUserManager->GetUserSet();

	for (CUserManager::ITOR_USER i = mpUser.begin(); i != mpUser.end(); ++i)
	{
		const int idUser = i->second;
		CHARLIST* pUser = CheckServerId(idUser); 
		
		if (pUser != NULL)
		{
			if (pUser->name_status.nation != ch->name_status.nation)
			{
				packet.u.kein.scan_target.xy[count].x = pUser->X/TILE_SIZE*200/g_Map.file.wWidth;
				packet.u.kein.scan_target.xy[count].y = pUser->Y/TILE_SIZE*200/g_Map.file.wHeight;
				
				if (++count >= 50)
				{
					break;
				}
			}
		}
	}
	
	packet.h.header.size = sizeof( k_scan_target ) - sizeof( k_x_y )*(250-count);
	QueuePacket(connections, cn, &packet, 1);
}	//> CSD-CN-031213

void RecvGuildAllMsg( t_packet *p, short int cn )
{	//< CSD-030324
	char *msg = p->u.kein.default_msg;
	CHARLIST *ch = CheckServerId( cn );
	if( !ch ) return;
	
	if( !CheckGuildPower( GDP_BBS, ch ) ) return;

	t_packet packet;
	packet.h.header.type = CMD_GUILD_ALL_MSG_RECV;
		packet.u.kein.guild_msg.guild_code = ch->GetGuildCode();
		strcpy( packet.u.kein.guild_msg.msg, msg );
	packet.h.header.size = sizeof( k_guild_msg) - MAX_MSG + strlen( msg )+1;

	// Modified by chan78 at 2000/12/01 BroadCast It to MapServers.
	g_pServerTable->BroadCastToEveryServer( (char*)&packet, (sizeof(t_header)+packet.h.header.size), SERVER_TYPE_MAP );
	RecvGuildAllMsg_FromServer( &packet );
}	//> CSD-030324

void RecvGuildAllMsg_FromServer( t_packet *p )
{	//< CSD-030415
	char *msg = p->u.kein.guild_msg.msg;
	const int guild_code = p->u.kein.guild_msg.guild_code;

	t_packet packet;
	packet.h.header.type = CMD_GUILD_ALL_MSG_RECV;
	packet.h.header.size = strlen(msg) + 1;
	strcpy(packet.u.kein.default_msg, msg);
	g_pGuildManager->SendPacket(guild_code, &packet);
}	//> CSD-030415

void RecvChangeGuildEachLevel( t_packet *p, short int cn )
{	//< CSD-030324
	int each_level = p->u.kein.guild_each_level.each_level;
	int target_id = p->u.kein.guild_each_level.target_id;

	if( each_level <= 0 || each_level > 5 ) return;		// 1:�渶, 5: �Ϲ�ȸ��
	if( each_level == 1 ) return;		// �渶�� �ٲٴ� �Ŷ�� �̰� Ǯ���ش�.

	CHARLIST *target = CheckServerId( target_id );			// ����� �ٲ� ĳ����
	if( !target ) return;

	CHARLIST *ch = CheckServerId( cn );						// ��� ������
	if( !ch ) return;

//  ���� ��忩�� �Ѵ�. ( ���� ���� ���� )
	//if( !ch->GetGuildCode() || ch->name_status.guild_master != 1 ) return;	// ��� �����͸� ����Ҽ� �ִ� ���ɾ��.
	if( !CheckGuildPower( GDP_CHANGE_DEGREE, ch ) ) return;
	if( ch->GetGuildCode() != target->GetGuildCode() ) return;

	int each_level_to_real_level[] = { 0, 1, 2, 3, 4, 0 };		// 1�� �渶 5�� �Ϲ�
	target->name_status.guild_master = each_level_to_real_level[each_level];		// ���� �� ����

	int guild_code = target->GetGuildCode();
	SendSaveGuildMemberList( target->Name, guild_code, guild_code, each_level_to_real_level[each_level] );

	QueuePacket(connections, target_id, p, 1);		// �� �ڽŰ� �ֺ� ������� ��� �� ���� �״�� �����ش�.
	CastMe2Other( target_id, p );
}	//> CSD-030324

void SendBbsAll( char *name, int type )
{
	t_packet packet;
	packet.h.header.type = CMD_BBS_ALL;
		packet.u.kein.bbs_all.type = type;
		strcpy( packet.u.kein.bbs_all.name, name );
	packet.h.header.size = sizeof( k_bbs_all ) - 20+ strlen( name )+1;

	// Modified by chan78 at 2000/12/01 BroadCast It to MapServers.
	g_pServerTable->BroadCastToEveryServer( (char*)&packet, (sizeof(t_header)+packet.h.header.size) );
}

// �α��μ������� �޾Ƽ� ����� ��� ������� �ѷ��ٶ�..
void DirectSend(t_packet *p, int type)
{	//< CSD-CN-031213
	if (type)
	{
		p->h.header.type = type;
	}
	
	g_pUserManager->SendPacket(p);
}	//> CSD-CN-031213

CHARLIST* CheckKingByNation(int nation)
{	//< CSD-CN-031213
	CUserManager::HASH_USER mpUser = g_pUserManager->GetUserSet();

	for (CUserManager::ITOR_USER i = mpUser.begin(); i != mpUser.end(); ++i)
	{
		const int idUser = i->second;
		CHARLIST* pUser = CheckServerId(idUser);
		
		if (pUser != NULL && pUser->name_status.nation == nation)
		{
			if (pUser->name_status.king == 1)
			{
				return pUser;
			}
		}
	}

	return NULL;
}	//> CSD-CN-031213

// ��û���� �տ��� ������ ������.
void RecvReqStopWarGame2ClientSetPacket( short int nation, short int game_id, short int target_cn, short int send_cn )
{
	t_packet packet;
	packet.h.header.type = CMD_REQ_STOP_WAR_TO_CLIENT;
		packet.u.kein.send_client_req_stop_war.send_nation	= nation;
		packet.u.kein.send_client_req_stop_war.game_id = game_id;
		packet.u.kein.send_client_req_stop_war.server_id = send_cn;
	packet.h.header.size = sizeof( k_send_client_req_stop_war );
	QueuePacket(connections, target_cn, &packet, 1);
}

// �� ���� �ٸ� �տ��� ���� ��û�� �ؿ�����
void RecvReqStopWarGame2Login( t_packet *p, short int cn )
{
	int nation = p->u.kein.default_char;
	CHARLIST *ch = CheckServerId( cn );
	if( !ch ) return;
	
	if( ch->name_status.king != 1 ) return;
	if( ch->name_status.nation == nation ) return;
	if( nation < N_VYSEUS || nation > N_YILSE ) return;
	
	CHARLIST *target_king = CheckKingByNation( nation );		 // ����� ������ ���� ���� �ʿ� �ִٸ�
	if( target_king )
	{
		RecvReqStopWarGame2ClientSetPacket( ch->name_status.nation, DRAGON_PORT, target_king->GetServerID(), cn );		// �ٷ� ������ ������ ������ �����.
	}
	else
	{
		t_packet packet;
		packet.h.header.type = CMD_REQ_STOP_WAR_TO_LOGIN;
			packet.u.kein.send_login_req_stop_war.target_nation	= nation;
			packet.u.kein.send_login_req_stop_war.send_nation	= ch->name_status.nation;
			packet.u.kein.send_login_req_stop_war.server_id		= cn;
		packet.h.header.size = sizeof( k_send_login_req_stop_war );

		// Modified by chan78 at 2000/12/01 BroadCast It to MapServers.
		g_pServerTable->BroadCastToEveryServer( (char*)&packet, (sizeof(t_header)+packet.h.header.size), SERVER_TYPE_MAP );
	}
}

// �ʿ� ���� �ִ����� �˾ƺ��� ������ ���� ������ ������.
void RecvReqStopWarGame2Client( t_packet *p )
{
	int target_nation	= p->u.kein.send_game_server_req_stop_war.target_nation;
	int send_nation		= p->u.kein.send_game_server_req_stop_war.send_nation;
	short int server_id	= p->u.kein.send_game_server_req_stop_war.server_id;
	short int game_id	= p->u.kein.send_game_server_req_stop_war.game_id;

	CHARLIST *ch = CheckKingByNation( target_nation );
	if( !ch ) return;
	RecvReqStopWarGame2ClientSetPacket( send_nation, game_id, ch->GetServerID(), server_id );
}

void RecvRepStopWarRefusal( t_packet *p )
{
	int game_id = p->u.kein.rep_stop_war_login.game_id;		// gameserver_port;
	if( game_id == DRAGON_PORT )		// ���� ���� �������
	{
		int server_id = p->u.kein.rep_stop_war_login.server_id;
		CHARLIST *ch = CheckServerId( server_id );
		if( !ch ) return;
		CallClient( ch->GetServerID(), CMD_REP_STOP_WAR_REFUSAL_TO_CLIENT );		// ���� ���ߴٴ� ���� �˸���.
	}
	else {
		// Modified by chan78 at 2000/12/01 BroadCast It to MapServers.
		g_pServerTable->BroadCastToEveryServer( (char*)p, (sizeof(t_header)+p->h.header.size), SERVER_TYPE_MAP );
	}
}

void RecvRepStopWarAccept( t_packet *p, short int cn )				// �³��ߴ�.
{	
	int send_nation = p->u.kein.default_char;				// ó���� ������ ��û�ߴ� ����
	CHARLIST *ch = CheckServerId( cn );
	if( !ch ) return;
	if( ch->name_status.king != 1 ) return;
	int target_nation = ch->name_status.nation;

	SendNation2NationRelation( target_nation, send_nation, NATION_RELATION_FRIEND_ );
		/*

	t_packet packet;
	packet.h.header.type = CMD_REP_STOP_WAR_ACCEPT_TO_LOGIN;
		packet.u.kein.stop_war_all_msg.target_nation = target_nation;
		packet.u.kein.stop_war_all_msg.send_nation	= send_nation;
	packet.h.header.size = sizeof( k_stop_war_all_msg );
	QueuePacket(connections, LOGIN_SERVER_CONNECT, &packet, 1);*/
}

void RecvRepStopWarGame2Client( t_packet *p )		// ��ο��� ������ ����� �����ش�.
{	//< CSD-CN-031213
	const int nation1 = p->u.kein.stop_war_all_msg.send_nation ;
	const int nation2 = p->u.kein.stop_war_all_msg.target_nation;

	t_packet packet;
	packet.h.header.type = CMD_REP_STOP_WAR_ACCEPT_TO_CLIENT;
	packet.h.header.size = sizeof(char);

	CUserManager::HASH_USER mpUser = g_pUserManager->GetUserSet();

	for (CUserManager::ITOR_USER i = mpUser.begin(); i != mpUser.end(); ++i)
	{
		const int idUser = i->second;
		CHARLIST* pUser = CheckServerId(idUser);
		
		if (pUser != NULL)
		{
			if (pUser->name_status.nation == nation1)
			{
				packet.u.kein.default_char = nation2;
				QueuePacket(connections, idUser, &packet, 1);
			}
			
			if (pUser->name_status.nation == nation2)
			{
				packet.u.kein.default_char = nation1;
				QueuePacket(connections, idUser, &packet, 1);
			}
		}
	}
}	//> CSD-CN-031213

void RecvAndReqSalvation( short int cn )		// DB�� ��α��� ��û�Ѵ�.
{
	CHARLIST *ch = CheckServerId( cn );
	if( !ch ) return;

	t_packet packet;
	packet.h.header.type = CMD_REQ_SALVATION_MONEY;
		packet.u.kein.req_salvation_money.server_id = cn;
		strcpy( packet.u.kein.req_salvation_money.name, ch->Name );
	packet.h.header.size = sizeof( k_req_salvation_money ) - 20 + strlen( ch->Name )+1;
	QueuePacket(connections, DB_DEMON, &packet, 1);
}

void RecvAndSaveSalvation( t_packet *p, short int cn )
{
	CHARLIST *ch = CheckServerId( cn );
	if( !ch ) return;

	DWORD money = p->u.kein.default_dword;

	if( !money ) return;
//	ch->Money = GetMoneyByItem( ch );
	if( SubtractMoney( money, ch ) == -1 ) return;
	
	t_packet packet;
	packet.h.header.type = CMD_SALVATION_MONEY;
		packet.u.kein.db_salvation_money.server_id = cn;
		packet.u.kein.db_salvation_money.nation = ch->name_status.nation;
		packet.u.kein.db_salvation_money.money = money;
		strcpy( packet.u.kein.db_salvation_money.name, ch->Name );
	packet.h.header.size = sizeof( k_db_salvation_money ) - 20 + strlen( ch->Name )+1;
	QueuePacket(connections, DB_DEMON, &packet, 1);

	// 011020 YGI
	// ������ ����.. 
	SendNationMoney( NM_ADD, ch->name_status.nation, money); // ������ ��ȭ��Ų��.
	// fame ����
	char add_fame = g_Money_to_Fame.GetFame( money );
	SaveChangeFame(ch,ch->fame,ch->fame+add_fame,LF_DONATION);
	ch->fame += add_fame;
	SendDefaultResult( CMD_SALVATION_RESULT_ADD_FAME, add_fame, cn );
}

void Game2ClinetSalvation( t_packet *p )
{
	DWORD money = p->u.kein.db2game_salvation_money.money;
	short int cn = p->u.kein.db2game_salvation_money.server_id;

	t_packet packet;
	packet.h.header.type = CMD_DB2GAME_SALVATION;
		packet.u.kein.default_dword = money;
	packet.h.header.size = sizeof( DWORD );
	QueuePacket(connections, cn, &packet, 1);
}

void RecvDB2MapItem( t_packet *p )	// ��񿡼� �������� ����� �ֶ�� ������ ���� ó��
{
	short int item_no = p->u.kein.send_db2map_each_item.item_no;
	short int server_id = p->u.kein.send_db2map_each_item.server_id;

	CHARLIST *ch = CheckServerId( server_id );
	if( !ch ) return;

	ItemAttr item = GenerateItem( item_no );
	if( item.item_no != 0 )
	{
		SendItemEventLog( &item, server_id, SN_NOT_USER,SILT_MAKE_BY_DB, 2 ); //YGI acer
		SendServerEachItemAuto( ch, &item );
	}	
}

void RecvChangeMoney( t_packet *p, short int cn )
{
	CHARLIST *ch = CheckServerId( cn );
	if( !ch ) return;

	DWORD money = p->u.kein.change_money.money;

	DWORD old_money = 0;
	if(LocalMgr.IsChangeMoney())//030102 lsw
	{
		old_money = GetMoneyByOldItem( ch );
	}
	else
	{
		old_money = GetMoneyByItem( ch );
	}

	if( money > old_money ) return;		// �� �� ���� ���� �ʹ� ���� �ٲ۴�.
	
	DWORD new_money = money * 1;//020730 lsw
	if( new_money )
	{
		if(::GiveMoney(ch,new_money))//���� �Ǿ��°�
		{
			SubtractOldMoney( money, ch );
			t_packet packet;
			packet.h.header.type = CMD_CHANGE_MONEY_OK;
			packet.u.kein.default_dword = new_money;
			packet.h.header.size = sizeof( new_money );
			QueuePacket(connections, cn, &packet, 1);
		}	
	}
}	

void SendEatChcolate( char *name, DWORD limit, DWORD cn )
{
	t_packet packet;
	packet.h.header.type = CMD_EAT_CHOCOLATE;
		strcpy(packet.u.kein.eat_chocolate.name, name);
		packet.u.kein.eat_chocolate.server_id = cn ;
		packet.u.kein.eat_chocolate.limit = limit;
	packet.h.header.size = sizeof( k_eat_chocolate ) - 20 + strlen( name )+1;
	QueuePacket(connections, DB_DEMON, &packet, 1);
}


void RecvEatChocolateOk( t_packet *p )
{
	t_packet packet;
	packet.h.header.type = CMD_EAT_CHOCOLATE_OK;
		packet.u.kein.default_int = p->u.kein.eat_chocolate_ok.chocolate_count;
	packet.h.header.size = sizeof( int );
	QueuePacket(connections, p->u.kein.eat_chocolate_ok.server_id, &packet, 1);
}

void RecvPayRumor( short int cn )
{
	CHARLIST *ch = CheckServerId( cn );
	if( !ch ) return;

	int ret = SubtractMoney( 500, ch );
	if( ret != 1 ) 
	{
		CallClient( cn, CMD_PAY_RUMOR_NOT );
		return ;
	}

	t_packet packet;			// DB�� ��û
	packet.h.header.type = CMD_PAY_RUMOR;
		packet.u.kein.default_dword = cn;
	packet.h.header.size = sizeof( DWORD );
	QueuePacket(connections, DB_DEMON, &packet, 1);
}

void RecvAndSendRumor( t_packet *p )
{
	char *name = p->u.kein.login_check_chocolate.name;
	int count = p->u.kein.login_check_chocolate.count;
	int cn = p->u.kein.login_check_chocolate.server_id;


	t_packet packet;			// DB�� ��û
	packet.h.header.type = CMD_PAY_RUMOR_OK;
		strcpy( packet.u.kein.check_chocolate.name, name) ;
		packet.u.kein.check_chocolate.count = count;
	packet.h.header.size = sizeof( k_check_chocolate ) - 20 + strlen( name )+1;
	QueuePacket(connections, cn, &packet, 1);
}


void MovePc( short int cn, int x, int y)
{
	CHARLIST *ch = CheckServerId( cn );
	if( !ch ) return;

	ch->X = x*TILE_SIZE + 16;
	ch->Y = y*TILE_SIZE + 16;

	ch->MoveSx = x;
	ch->MoveSy = y;
	ch->OldMoveSx = x;
	ch->OldMoveSy = y;
	ch->MoveGox = x;
	ch->MoveGoy = y;

	ch->MoveLength = ch->MovePathCount = 0;

	t_server_magic_position	smp;

	//Ŭ���̾�Ʈ�� ���� ��Ŷ ����
	smp.nType =	PT_TELEPORT;
	smp.idTarget = WORD(cn);
	smp.nX =	x;
	smp.nY =	y;

	SendCMD_MAGIC_POSITION(&smp);
}

void SendOpenSChoolData( int cn, int item_no, int npc_id )
{
	CHARLIST *ch = CheckServerId( cn );
	if( !ch ) return;
	
	t_packet packet;			// DB�� ��û
	packet.h.header.type = CMD_OPENSCHOOL_DATA;
		strcpy( packet.u.kein.openschool_data.name, ch->Name);
		packet.u.kein.openschool_data.item_no = item_no;
		packet.u.kein.openschool_data.nation = ch->name_status.nation;
		packet.u.kein.openschool_data.server_id = cn;
		packet.u.kein.openschool_data.npc_id = npc_id;
	packet.h.header.size = sizeof( k_openschool_data ) - 20 + strlen( ch->Name )+1;
	QueuePacket(connections, DB_DEMON, &packet, 1);
}

void RecvOpenSchoolResult( t_packet *p )
{
	int server_id = p->u.kein.openschool_data_ok.server_id;
	int no = p->u.kein.openschool_data_ok.number;
	int item_no = p->u.kein.openschool_data_ok.item_no;
	int npc_id = p->u.kein.openschool_data_ok.npc_id;

	CHARLIST *ch = CheckServerId( server_id );
	if( !ch ) return;

	if( no == -1 ) // �ٽ� ����� �ش�.
	{
		POS pos;
		int a, b, c;
		if( SearchInv( ch->inv, a, b, c ) )
		{
			ItemAttr *item = &ch->inv[a][b][c];
			*item = GenerateItem( item_no );
			SetItemPos( INV, a, b, c, &pos );
			SendServerEachItem( &pos, item, server_id );
		}
	}

	t_packet packet;
	packet.h.header.type = CMD_OPENSCHOOL_DATA_OK;
		packet.u.kein.openschool_data_result.number = (char) no;
		packet.u.kein.openschool_data_result.npc_id = npc_id;
	packet.h.header.size = sizeof( k_openschool_data_result );
	QueuePacket(connections, server_id, &packet, 1);
}

void RecvGuildExitGlobal( t_packet *p, short int cn )
{	//< CSD-030324
	CHARLIST *ch = CheckServerId( cn );
	if( !ch ) return;

	if( !CheckGuildPower( GDP_EXIT_GUILD_HIM, ch ) ) return;

	char *name = p->u.kein.default_name;

	t_packet packet;
	packet.h.header.type = CMD_GUILD_EXIT_COMMAND_GLOBAL;
		strcpy( packet.u.kein.guild_exit_global.name, name );
		packet.u.kein.guild_exit_global.server_id = cn;
		packet.u.kein.guild_exit_global.guild_code = ch->GetGuildCode();
	packet.h.header.size = sizeof( k_guild_exit_global )-20+strlen( name )+1;
	QueuePacket(connections, DB_DEMON, &packet, 1);
}	//> CSD-030324

void RecvGuildExitGlobalFromDB( t_packet *p )
{	//< CSD-030326
	char *name = p->u.kein.default_name;
	int cn = ExistHe( name );
	CHARLIST *ch = CheckServerId( cn );
	if( !ch ) return;		// �׷��� ����.

	g_pGuildManager->DelMember(ch->GetGuildCode(), cn);

	ch->SetGuildCode(0); // CSD-030806
	ch->name_status.guild_master = 0;

	CallClient( cn, CMD_GUILD_EXIT_YOU );			// Ż����ѳ�����

	t_packet packet;								// �ֺ��������
	packet.h.header.type = CMD_GUILD_EXIT_WHO;
		packet.u.kein.default_short_int = cn;
		packet.h.header.size = sizeof( short int );
	CastMe2Other( cn, &packet );
}	//> CSD-030326

void SendEatCandy( char *name, DWORD limit, DWORD total_id )
{
	t_packet packet;
	packet.h.header.type = CMD_EAT_CANDY;
		strcpy(packet.u.kein.eat_candy.name, name);
		packet.u.kein.eat_candy.total_id = total_id;
		packet.u.kein.eat_candy.candy_limit = limit;
	packet.h.header.size = sizeof( k_eat_candy) - 20 + strlen( name )+1;
	QueuePacket(connections, DB_DEMON, &packet, 1);
}

// 010314 YGI
void RecvReqLoan( t_packet *p, short int cn )
{	//< CSD-030806
	CHARLIST* ch = CheckServerId(cn);
	
	if (ch == NULL)
	{
		return;
	}

	DWORD loan_money = p->u.kein.default_dword;			//	��û�� �ݾ�
	const int nLevel = ch->GetLevel();

	if ((loan_money + ch->LastLoan) > NPC_Lev_Ref[nLevel].bank_loan)
	{	// �䱸 �ݾ��� �ʹ� ũ��.
		return;	
	}

	if (GiveMoney(ch,loan_money))
	{
		ch->LastLoan += loan_money;
		RecvLoanInfo(cn);
	}
}	//> CSD-030806

void RecvLoanInfo( short int cn )
{	//< CSD-030806
	CHARLIST* ch = CheckServerId(cn);

	if (ch == NULL)
	{
		return;
	}

	const int nLevel = ch->GetLevel();
	t_packet packet;
	packet.h.header.type = CMD_REQ_LOAN_INFO;
	packet.u.kein.loan_info.last_loan = ch->LastLoan;
	packet.u.kein.loan_info.max_loan = NPC_Lev_Ref[nLevel].bank_loan;
	packet.h.header.size = sizeof( k_loan_info );
	QueuePacket(connections, cn, &packet, 1);
}	//> CSD-030806

void RecvRepayment( t_packet *p, short int cn )
{
	CHARLIST *ch = CheckServerId( cn );
	if( !ch ) return;

	DWORD money = p->u.kein.default_dword;
	if( ch->LastLoan < money ) money = ch->LastLoan;

//	ch->Money = GetMoneyByItem( ch );
	if( ch->Money < money ) money = ch->Money;

	SubtractMoney( money, ch );
	ch->LastLoan -= money;
	RecvLoanInfo( cn );
}

static int bbs_money[2][2] = { {100, 1000}, {1500, 15000} };
void RecvMerchantBBS( t_packet *p, short int cn ) // Ŭ���̾�Ʈ���� ������ �̸��� �־ ���� �� ��ο��� �ѷ��ش�.
{	//< CSD-CN-031213
	CHARLIST* ch = CheckServerId(cn);
	
	if (ch == NULL)
	{
		return;
	}

	if (ch->Money < bbs_money[1][0])
	{
		return;
	}

	SubtractMoney(bbs_money[1][0], ch);

	t_packet packet;
	packet.h.header.type = p->h.header.type;
	packet.h.header.size = sizeof(k_name_message) - 200 + p->h.header.size;
	strcpy(packet.u.kein.name_message.name, ch->Name);
	strcpy(packet.u.kein.name_message.message, p->u.data);
	g_pUserManager->SendPacket(&packet);
}	//> CSD-CN-031213

void RecvMerchantBBSAllMap( t_packet *p, short int cn )		// Ŭ���̾�Ʈ�� �޾Ƽ� Ÿ�԰� �̸��� �ְ� ��� ������ �ѷ��ش�.
{
	CHARLIST *ch = CheckServerId( cn );
	if( !ch ) return;

	if( ch->Money < bbs_money[1][1] ) return;
	SubtractMoney( bbs_money[1][1], ch );

	int nation = ch->name_status.nation;
	//g_pServerTable->BroadCastToEveryServer

	t_packet packet;
	packet.h.header.type = CMD_MERCHANT_BBS_MAP_TO_MAP;
		strcpy( packet.u.kein.name_message.name, ch->Name );
		strcpy( packet.u.kein.name_message.message, p->u.data );
	packet.h.header.size = sizeof( k_name_message ) - 200 + p->h.header.size;

	for( int i = 0 ;  i< MAX_MAP_ ; i ++ )
	{
		if( MapInfo[i].nation == nation ) g_pServerTable->SendRajaPacketToOtherMapServer( (WORD)MapInfo[i].port, (char *)&packet, packet.h.header.size+sizeof(t_header) );
	}
	
	packet.h.header.type = CMD_MERCHANT_BBS;
	g_pUserManager->SendPacket(&packet); // CSD-031213
}

void RecvMerchantItemBBS( t_packet *p, short int cn )		// ������ ����
{
	CHARLIST *ch = CheckServerId( cn );
	if(!ch ) return;

	if( ch->Money < bbs_money[0][0] ) return;
	SubtractMoney( bbs_money[0][0], ch );

	t_packet packet;
	packet.h.header.type = p->h.header.type;
	packet.h.header.size = sizeof( k_recv_merchant_item_bbs ) - 20+sizeof(ch->Name);
	strcpy( packet.u.kein.recv_merchant_item_bbs.szCharName, ch->Name );
	strcpy( packet.u.kein.recv_merchant_item_bbs.szItemName, p->u.kein.merchant_item_bbs.szItemName );
	packet.u.kein.recv_merchant_item_bbs.dwMoney = p->u.kein.merchant_item_bbs.dwMoney;
	packet.u.kein.recv_merchant_item_bbs.map_number = MapNumber;
	packet.u.kein.recv_merchant_item_bbs.pos_x = ch->X/TILE_SIZE;
	packet.u.kein.recv_merchant_item_bbs.pos_y = ch->Y/TILE_SIZE;

	g_pUserManager->SendPacket(&packet); // CSD-CN-031213
}

void RecvMerchantItemBBSAllMap( t_packet *p, short int cn )
{
	CHARLIST *ch = CheckServerId( cn );
	if( !ch ) return;

	if( ch->Money < bbs_money[0][1] ) return;
	SubtractMoney( bbs_money[0][1], ch );

	int nation = ch->name_status.nation;

	t_packet packet;
	packet.h.header.type = CMD_MERCHANT_ITEM_BBS_MAP_TO_MAP;

		strcpy( packet.u.kein.recv_merchant_item_bbs.szCharName, ch->Name );
		strcpy( packet.u.kein.recv_merchant_item_bbs.szItemName, p->u.kein.merchant_item_bbs.szItemName );
		packet.u.kein.recv_merchant_item_bbs.dwMoney = p->u.kein.merchant_item_bbs.dwMoney;
		packet.u.kein.recv_merchant_item_bbs.map_number = MapNumber;
		packet.u.kein.recv_merchant_item_bbs.pos_x = ch->X/TILE_SIZE;
		packet.u.kein.recv_merchant_item_bbs.pos_y = ch->Y/TILE_SIZE;
	packet.h.header.size = sizeof( k_recv_merchant_item_bbs )-20+sizeof(ch->Name);

	SendNationPacket( &packet, nation );	
	packet.h.header.type = CMD_MERCHANT_ITEM_BBS;		// �ڱ� ����
	g_pUserManager->SendPacket(&packet); // CSD-CN-031213
}

inline void SendNationPacket( t_packet *p, int nation )		// ���� ������ �ʼ������� ��� ������.
{
	for( int i = 0 ;  i< MAX_MAP_ ; i ++ )
	{
		if( MapNumber == i ) continue;

		if( MapInfo[i].nation == nation ) 
			g_pServerTable->SendRajaPacketToOtherMapServer( (WORD)MapInfo[i].port, (char *)p, p->h.header.size+sizeof(t_header) );
	}
}

void RecvPartyAllFree( short int cn )
{
	k_client_delete_party data;
	
	data.type = PARTY;
	for( int i=0; i<6; i++ )
	{
		data.ct = i;
		RecvDeleteParty( &data, connections, cn );		// ��� ���� ��Ų��.
	}
}

void InitJoinGameWho(CHARLIST *ch)	// ������ �� ������ ���Դ�.		// 010410 YGI
{	//< CSD-HK-030829
	const WORD idUser = ch->GetServerID();
	g_pUserManager->AddMember(idUser);
	g_pDualManager->AddMember(idUser);

	const int nGuildCode = ch->GetGuildCode();

	if (nGuildCode > 0)
	{
		g_pGuildManager->AddMember(nGuildCode, idUser);
	}
	
	if (g_pDungeonMap)
	{
		g_pDungeonMap->NewJoin(idUser);
	}
}	//> CSD-HK-030829

/////////////////////////////////////////////////////////////////////////////////////////////
// CAliveMapZoneAll
int CAliveMapZoneAll::CheckAlive( int ch_x, int ch_y, int map_number, int word_type )
{
	for( int i=0; i<m_Max; i++ )
	{
		if( m_Map[i].map_number != map_number ) continue;		
		if( m_Map[i].is_all ) return 100;
		if( ! TileMap[ch_x][ch_y].attr_inside ) // ���� �Ӽ��� �ִ� ���� �ƴ϶��
			continue;

		if( BoxAndDotCrash( m_Map[i].x,m_Map[i].y, m_Map[i].xl, m_Map[i].yl, ch_x, ch_y ) )
		{
			if( !m_Map[i].alive_word_type || word_type == m_Map[i].alive_word_type ) return 100; // 010612 YGI
			else return m_Map[i].alive_word_type;	// ��Ƴ��� ���̱� ������ ��� �ֹ��� Ʋ�Ǵ�. �׷� �ֹ��� ������ �ش�.
		}
	}

	return -1;		// �ش��ϴ� ���� �ƴϴ�.
}

/////////////////////////////////////////////////////////////////////////////////////////////


int UpdateLadderScore( CHARLIST *ch )
{
	if( !ch->refresh_ladder ) return 0;

	t_packet packet;
	packet.h.header.type = CMD_UPDATE_LADDER_SCORE;
	packet.h.header.size = sizeof( k_update_ladder_score ) - 20 + strlen( ch->Name )+1;
	packet.u.kein.update_ladder_score.score = ch->LadderScore;
	packet.u.kein.update_ladder_score.naiton = ch->name_status.nation;
	packet.u.kein.update_ladder_score.cls = ch->Class;
	packet.u.kein.update_ladder_score.btLevel = ch->GetLevel(); // CSD-030806
	strcpy( packet.u.kein.update_ladder_score.name, ch->Name );
	QueuePacket( connections, DB_DEMON, &packet,1 );
	return 1;
}

void RecvReqRankLadder( t_packet *p, short int cn )
{
	int page = p->u.kein.default_int;

	t_packet packet;
	packet.h.header.type = CMD_REQ_RANK_LADDER;
		packet.u.kein.req_rank_ladder_to_db.page = page;
		packet.u.kein.req_rank_ladder_to_db.server_id = cn;

	packet.h.header.size = sizeof( k_req_rank_ladder_to_db );
	QueuePacket( connections, DB_DEMON, &packet,1 );	
}

void RecvDirectClient( t_packet *p )
{
	int server_id = p->u.kein.direct_client.server_id;

	t_packet packet;
	packet.h.header.type = p->u.kein.direct_client.type;
	packet.h.header.size = p->h.header.size - sizeof( int );
	if( packet.h.header.size )
		memcpy( packet.u.kein.data, p->u.kein.direct_client.data, packet.h.header.size );

	QueuePacket( connections, server_id, &packet,1 );
}

// 030919 HK YGI
#define CMD_CHECK_SALVATION_NAME			10388
void RecvDirectDB( t_packet *p, short int cn )
{
	//k_send_db_direct_map ����ü�� k_send_db_direct_client ����ü�� ���°� �Ȱ��� ������ 

	int type = p->u.kein.send_db_direct_client.type;
	p->h.header.type = type;
	p->u.kein.send_db_direct_map.server_id = cn;

	// 030919 HK YGI
	switch (type)
	{	//< CSD-031210
	case CMD_CHECK_SALVATION_NAME :
		{
			CHARLIST* ch = CheckServerId(cn);
			
			if (ch != NULL)
			{	
				p->h.header.size = strlen(ch->Name) + 2;
				memcpy( p->u.kein.send_db_direct_map.data, ch->Name, 20 );
			}

			break;
		}
	//< 040331-YGI
	case CMD_REGIST_GUILD_MARK :
		{
			CHARLIST* ch = CheckServerId(cn);
			if( !ch ) 
			{
				MyLog( 0, "Wrong ch[%s], cn[%d] when CMD_REGIST_GUILD_MARK", connections[cn].name, cn );
				return;
			}
			if( !CheckGuildPower( GDP_CHANGE_MARK, ch ) )
			{
				MyLog( 0, "Wrong!! CMD_REGIST_GUILD_MARK : Name[%s], guild_code[%d], guild_degree[%d]", 
					ch->Name, ch->GetGuildCode(), ch->name_status.guild_master );
				return;
			}
			k_regist_guild_mark *pGuildData;
			pGuildData = (k_regist_guild_mark *)p->u.kein.send_db_direct_map.data;
			if( pGuildData->guild_code != ch->GetGuildCode() )
			{
				MyLog( 0, "Wrong!! CMD_REGIST_GUILD_MARK : Name[%s], guild_code[%d], guild_degree[%d], target_guild[%d]", 
					ch->Name, ch->GetGuildCode(), ch->name_status.guild_master, pGuildData->guild_code );
				return;
			}
			MyLog( 0, "Try change guild mark!! Name[%s], guild_code[%d], guild_degree[%d], target_guild[%d]", 
				ch->Name, ch->GetGuildCode(), ch->name_status.guild_master, pGuildData->guild_code );

		}//> 040331-YGI
	}	//> CSD-031210

	QueuePacket( connections, DB_DEMON, p,1 );
}

void RecvReqVisa( t_packet *p, short int cn )
{
	CHARLIST *ch = CheckServerId( cn );
	if( !ch ) return;

	int nation = p->u.kein.default_char;
	int new_nation_code;
	switch( nation )
	{
		case N_VYSEUS	:	new_nation_code = 0; break;
		case N_ZYPERN	:	new_nation_code = 1; break;
		case N_YILSE	:	new_nation_code = 2; break;
		default : return;		// ���� ���� �ȵǴ� ����
	}

	int visa_item[] = { NationInfo[nation].visa_va, NationInfo[nation].visa_zy, NationInfo[nation].visa_ill};
	int visa_mer_item[] = { NationInfo[nation].visa_mer_va, NationInfo[nation].visa_mer_zy, NationInfo[nation].visa_mer_ill };

	int visa_pri_item[] = { NationInfo[nation].visa_pri_va, NationInfo[nation].visa_pri_zy, NationInfo[nation].visa_pri_ill}; 
	int visa_mer_pri_item[] = { NationInfo[nation].visa_mer_pri_va, NationInfo[nation].visa_mer_pri_zy, NationInfo[nation].visa_mer_pri_ill, };

	t_packet packet;
	k_visa_item *send_visa_item = &packet.u.kein.visa_item;
	for( int i=0, ct=0; i<3; i++ )
	{	
		if( i == new_nation_code ) continue;
		send_visa_item->item_no[ct] = visa_item[i];
		send_visa_item->item_no[ct+2] = visa_mer_item[i];

		send_visa_item->item_price[ct] = visa_pri_item[i];
		send_visa_item->item_price[ct+2] = visa_mer_pri_item[i];
		ct++;
	}
	packet.h.header.type = CMD_REQ_VISA;
	packet.h.header.size = sizeof( k_visa_item );
	QueuePacket( connections, cn, &packet,1 );
}

void RecvBuyVisa( t_packet *p, short int cn )
{
	int visa_no = p->u.kein.visa_buy.item_no;
	CHARLIST *ch = CheckServerId( cn );
	if( !ch ) return;

	int nation = p->u.kein.visa_buy.nation;
	int visa_item[] = { NationInfo[nation].visa_va, NationInfo[nation].visa_zy, NationInfo[nation].visa_ill, 
						NationInfo[nation].visa_mer_va, NationInfo[nation].visa_mer_zy, NationInfo[nation].visa_mer_ill };

	int visa_pri_item[] = { NationInfo[nation].visa_pri_va, NationInfo[nation].visa_pri_zy, NationInfo[nation].visa_pri_ill, 
							NationInfo[nation].visa_mer_pri_va, NationInfo[nation].visa_mer_pri_zy, NationInfo[nation].visa_mer_pri_ill, };

	static int of_nation[] = { N_VYSEUS, N_ZYPERN, N_YILSE, N_VYSEUS, N_ZYPERN, N_YILSE };		// ������ ���� ����

	for( int i=0; i<6; i++ )
	{
		if( visa_item[i] == visa_no )
		{
			int ret = SubtractMoney( visa_pri_item[i], ch );
			if( ret == 1 )
			{
				ItemAttr item = GenerateItem( visa_item[i] );
				SendServerEachItemAuto( ch, &item );
				// ��� ���� ������ ++ ��Ų��.
				SendNationMoney( NM_ADD, of_nation[i], visa_pri_item[i] );		// ������ ��ȭ��Ų��.
				CallClient( cn, CMD_BUY_VISA_OK );
			}
			else
			{
				CallClient( cn, CMD_BUY_VISA_FAIL );
			}
			break;
		}
	}
}

void RecvExitGuildHimSelf( short int cn )		// ������ ��� Ż�� ���ɾ�		// 010613 YGI
{	//< CSD-030326
	CHARLIST *ch = CheckServerId( cn );
	
	if (ch == NULL)  
	{
		return;
	}

	const int nGuildCode = ch->GetGuildCode(); 
	
	if (nGuildCode <= 0) 
	{
		return;
	}

	if (ch->name_status.guild_master == 1) 
	{
		return;	// �渶�� ���
	}
	// ��带 Ż�� ��Ų��.
	ch->SetGuildCode(0); // CSD-030806 

	t_packet packet;
	packet.h.header.type = CMD_GUILD_EXIT_WHO;
	packet.u.kein.default_short_int = cn;
	packet.h.header.size = sizeof(short int);
	QueuePacket(connections, cn, &packet, 1);	
	CastMe2Other(cn, &packet);		// �޽����� ������.
	// ��� ��� ����Ʈ ���̺����� ����
	SendSaveGuildMemberList(ch->Name, nGuildCode, 0, 0); 
	// guild_list ���� �����Ѵ�.
	g_pGuildManager->DelMember(nGuildCode, cn);
}	//> CSD-030326

int GetItemMuchByMuchItem( CHARLIST *ch, int item_no )
{
	ItemAttr *pItem = (ItemAttr *)ch->inv;
	int count = 0;
	for( int i=0; i<72; i++ ) 
	{
		if( pItem[i].item_no == item_no ) count += pItem[i].attr[IATTR_MUCH];
	}
	return count;
}

void RecvReqistGuild( t_packet *p, short int cn )
{	//< CSD-030326 : ���� ��� ���
	CHARLIST *ch = CheckServerId( cn );
	if( !ch ) return;

	const int nGuildCode = ch->GetGuildCode();

	if (nGuildCode <= 0) 
	{
		return;	// ���� �ӽ� ��忡 ����ؾ� �Ѵ�.
	}
	
	CGuildInfo* pGuild = g_pGuildManager->GetGuildInfo(nGuildCode);

	if (pGuild == NULL)
	{
		return;
	}

	if (pGuild->first != 1)
	{
		return;		// �ӽ� ��尡 �ƴϴ�.
	}

	if( ch->name_status.guild_master != 1 ) return;		// �渶�� �ƴϴ�.
	//acer7
	if (GetItemMuchByMuchItem( ch, IN_RED_SIGNET ) < SIZE_GR_RS) 
	{
		return;		// �ñ׳��� �����ϴ�.
	}
	// �������� �����. ������ ���ϸ� ���ư������� ���� �մ�. --> ����
	SendDeleteMuchItemQuantity(ch, IN_RED_SIGNET, SIZE_GR_RS);
	p->u.kein.default_short_int = ch->GetGuildCode();		// ��� �ڵ� ��ŷ�� ���� ����..

	t_packet packet;
	packet.h.header.type = CMD_REGIST_GUILD;
	packet.u.kein.send_db_direct_map.server_id = cn;
	memcpy( packet.u.kein.send_db_direct_map.data, p->u.kein.data, p->h.header.size );
	packet.h.header.size = p->h.header.size + 2;
	QueuePacket(connections, DB_DEMON, &packet,1);
}	//> CSD-030326

void RecvReqistGuildImsi( t_packet *p, short int cn )
{	//< CSD-030326
	CHARLIST *ch = CheckServerId( cn );
	if (!ch) return;
	if (ch->GetGuildCode()) return; // �̹� ��尡 �ִ�.
	if (ch->GetLevel() < BUILD_GUILD_LEVEL) return; // CSD-030806
	if (GetItemMuchByMuchItem(ch, IN_BLUE_SIGNET ) < SIZE_GR_BS) return;		// �ñ׳��� �����ϴ�.

	t_packet packet;
	packet.h.header.type = CMD_REGIST_GUILD_IMSI;
	packet.u.kein.regist_guild_imsi_db.server_id = cn;
	packet.u.kein.regist_guild_imsi_db.nation = ch->name_status.nation;
	strcpy( packet.u.kein.regist_guild_imsi_db.guild_name, p->u.kein.regist_guild_imsi.guild_name );
	strcpy( packet.u.kein.regist_guild_imsi_db.guild_ma, ch->Name );
	strcpy( packet.u.kein.regist_guild_imsi_db.guild_explain, p->u.kein.regist_guild_imsi.guild_explain);
	packet.h.header.size = sizeof( k_regist_guild_imsi_db ) - 1024 + strlen( p->u.kein.regist_guild_imsi.guild_explain );

	QueuePacket( connections, DB_DEMON, &packet,1 );	
}	//> CSD-030326

void RecvReqistGuildImsiOk( t_packet *p )	// ��񿡼� �ӽ� ������� �����ߴٰ� ���ƿԴ�.
{	//< CSD-030326
	const int server_id = p->u.kein.regist_guild_imsi_ok.server_id;
	const int nGuildCode = p->u.kein.regist_guild_imsi_ok.guild_code;

	CHARLIST *ch = CheckServerId(server_id);
	
	if (ch == NULL) 
	{
		return;
	}

	if (ch->GetGuildCode() > 0)
	{
		return;
	}
	// ��� ���
	ch->SetGuildCode(nGuildCode); // CSD-030806
	// �渶 ���
	ch->name_status.guild_master = 1; 
	g_pGuildManager->AddMember(nGuildCode, server_id);
	// �������� �����. 
	SendDeleteMuchItemQuantity(ch, IN_BLUE_SIGNET, SIZE_GR_BS);
	
	CGuildInfo* pGuild = g_pGuildManager->GetGuildInfo(nGuildCode);

	if (pGuild == NULL)
	{
		return;
	}
	
	// �渶 ��Ͽ� ���� Ŭ���̾�Ʈ ��Ŷ ����
	t_packet packet;
	packet.h.header.type = CMD_REGIST_GUILD_IMSI_OK;
	packet.u.kein.send_regist_imsi_guild_info.server_id = server_id;
	packet.u.kein.send_regist_imsi_guild_info.guild_code = nGuildCode;
	packet.u.kein.send_regist_imsi_guild_info.make = pGuild->make;
	strcpy(packet.u.kein.send_regist_imsi_guild_info.guild_name, pGuild->guild_name);
	packet.h.header.size = sizeof(k_send_regist_imsi_guild_info) - 20 + strlen(pGuild->guild_name);
	QueuePacket(connections, server_id, &packet, 1);
	CastMe2Other(server_id, &packet);
}	//> CSD-030326

void RecvSetGuildInfo(t_packet* p)	// ��񿡼� ���ƿ� �ٲ� ������ �����Ѵ�.
{	//< CSD-030326
	k_set_guild_info *guild_info = &p->u.kein.set_guild_info;

	const int nGuildCode = guild_info->guild_code;

	if (g_pGuildManager->IsExist(nGuildCode))
	{	// �����ϴ� �����
		CGuildInfo* pGuild = g_pGuildManager->GetGuildInfo(nGuildCode);
		pGuild->on = 1;
		pGuild->first = guild_info->first;
		pGuild->active = guild_info->active;
		pGuild->mark_num = guild_info->mark_num;
		pGuild->make = guild_info->make;
		pGuild->nation = guild_info->nation;
		strcpy(pGuild->guild_name, guild_info->guild_name);
		
		for (int i = 0; i < 5; i++)
		{
			strcpy(pGuild->guild_degree[i], guild_info->guild_degree[i]);
		}
	}
	else
	{
		CGuildInfo* pGuild = CGuildInfo::CreateGuildInfo();
		pGuild->on = 1;
		pGuild->first = guild_info->first;
		pGuild->active = guild_info->active;
		pGuild->mark_num = guild_info->mark_num;
		pGuild->make = guild_info->make;
		pGuild->nation = guild_info->nation;
		strcpy(pGuild->guild_name, guild_info->guild_name);
	
		for (int i = 0; i < 5; i++)
		{
			strcpy(pGuild->guild_degree[i], guild_info->guild_degree[i]);
		}

		g_pGuildManager->AddGuild(nGuildCode, pGuild);
		return;
	}
}	//> CSD-030326

void RecvTest( t_packet *p, short int cn )
{
	t_packet packet;
	packet.h.header.type = 2001;
	packet.h.header.size = 0;
	QueuePacket( connections, DB_DEMON, &packet,1 );	
}

//acer7
bool ExitGuild(CHARLIST* ch)
{	//< CSD-030326
	const int nGuildCode = ch->GetGuildCode();
	
	if (nGuildCode > 0)
	{
		return false;
	}
	// ��� ��� ����Ʈ���� ����
	g_pGuildManager->DelMember(nGuildCode, ch->GetServerID());
	// ��� ��� ����Ʈ ���̺����� ����
	SendSaveGuildMemberList(ch->Name, nGuildCode, 0, 0);
	ch->SetGuildCode(0); // CSD-030806
	return true;
}	//> CSD-030326

void RecvGuildInfo( t_packet *p, short int cn )
{	//< CSD-030326
	CHARLIST *ch = CheckServerId( cn );
	if( !ch ) return;

	int guild_code = p->u.kein.req_guild_info1.guild_code;
	CGuildInfo* pGuild = g_pGuildManager->GetGuildInfo(guild_code);

	if (pGuild == NULL)
	{
		return;
	}

	const int make = p->u.kein.req_guild_info1.make;
	const int mark_num = p->u.kein.req_guild_info1.mark_num;
	const int info_num = p->u.kein.req_guild_info1.info_num;
	const int first = p->u.kein.req_guild_info1.first;

	// ���ؼ� ������ �ʿ䰡 ������ �����ش�.
	int is_new = 0;

	if (!pGuild->active)	// ������ ����̴�.
	{	// ������ ����
		if( ch->GetGuildCode() == guild_code )
		{	
			if (ExitGuild(ch))
			{	
				SendPacketDefault(CMD_GUILD_INFO_DELETED, &guild_code, sizeof(short int), cn);
			}	
		}
		return;
	}
	// ���� �ٲ� ����̴�.
	if (make != pGuild->make || pGuild->first != first)	
	{
		SendGuildName(guild_code, cn);
		is_new = 1;
	}
	// ������� ���̻� ������ �������� �ʴ´�.
	if (pGuild->first) 
	{
		return;						
	}
	// �̹����� ������Ʈ�� ������ �ִ�.
	if (is_new || mark_num != pGuild->mark_num)	
	{
		t_packet packet;
		packet.h.header.type = CMD_REQ_GUILD_IMAGE;
		packet.u.kein.regist_guild_imsi_ok.guild_code = guild_code;
		packet.u.kein.regist_guild_imsi_ok.server_id = cn;
		packet.h.header.size = sizeof( k_regist_guild_imsi_ok );
		QueuePacket( connections, DB_DEMON, &packet,1 );	
	}
	// ����� ������ �ٲ�Ŵ�.
	if (is_new || info_num != pGuild->info_num)
	{	// �ʼ����� �����ش�.
		t_packet packet;
		packet.h.header.type = CMD_GUILD_DEGREE_INFO;
		packet.u.kein.guild_degree_info.guild_code = guild_code;
		packet.u.kein.guild_degree_info.info_num = pGuild->info_num;
		memcpy(packet.u.kein.guild_degree_info.guild_degree, pGuild->guild_degree, sizeof(char)*5*20);

		packet.h.header.size = sizeof( k_guild_degree_info );
		QueuePacket( connections, cn, &packet,1 );
	}
}	//> CSD-030326

void SendGuildName(int guild_code, short int cn)
{	//< CSD-030326
	CGuildInfo* pGuild = g_pGuildManager->GetGuildInfo(guild_code);

	if (pGuild == NULL)
	{
		return;
	}

	t_packet packet;
	packet.h.header.type = CMD_GUILD_INFO_NAME;
	packet.u.kein.guild_info_name.guild_code = guild_code;
	packet.u.kein.guild_info_name.make = pGuild->make;
	packet.u.kein.guild_info_name.first = pGuild->first;
	strcpy(packet.u.kein.guild_info_name.guild_name, pGuild->guild_name);
	packet.h.header.size = sizeof(k_guild_info_name) - 20 + strlen(pGuild->guild_name);
	QueuePacket(connections, cn, &packet, 1);
}	//> CSD-030326

void RecvGuildNumberInfo( t_packet *p )
{	//< CSD-030326
	const int nGuildCode = p->u.kein.set_guild_number_info.guild_code;
	CGuildInfo* pGuild = g_pGuildManager->GetGuildInfo(nGuildCode);
	
	if (pGuild == NULL)
	{
		return;
	}

	const int type = p->u.kein.set_guild_number_info.type;
	const int number = p->u.kein.set_guild_number_info.number;
	
	switch (type)
	{
	case 0:		// �̹��� ��ȣ�� �ٲ���.
		{
			pGuild->mark_num = number;
			break;
		}
	case 1:		// ����ũ ��ȣ�� �ٲ���.
		{
			
			pGuild->make = number;
			break;
		}
	case 2:
		{
			pGuild->info_num = number;
			break;
		}
	}
}	//> CSD-030326

void RecvGuildDegreeInfo2( t_packet *p )
{	//< CSD-030326
	const int nGuildCode = p->u.kein.guild_degree_info.guild_code;
	CGuildInfo* pGuild = g_pGuildManager->GetGuildInfo(nGuildCode);

	if (pGuild == NULL)
	{
		return;
	}
	
	memcpy(pGuild->guild_degree, p->u.kein.guild_degree_info.guild_degree, sizeof(char)*5*20);

	const int info_num = p->u.kein.guild_degree_info.info_num;
	pGuild->info_num = info_num;
	// ��� �������� ���ο� ������ �����ش�.
	t_packet packet;
	packet.h.header.type = CMD_REFRESH_GUILD_CODE;
	packet.h.header.size = sizeof(short int);
	packet.u.kein.guild_instroduction_basic.guild_code = nGuildCode;
	g_pGuildManager->SendPacket(nGuildCode, &packet);
}	//> CSD-030326

void RecvGuildDegreeInfo( t_packet *p )
{	//< CSD-030326
	const int nGuildCode = p->u.kein.guild_degree_info.guild_code;
	CGuildInfo* pGuild = g_pGuildManager->GetGuildInfo(nGuildCode);

	if (pGuild == NULL)
	{
		return;
	}

	memcpy(pGuild->guild_degree, p->u.kein.guild_degree_info.guild_degree, sizeof(char)*5*20);
	const int info_num = p->u.kein.guild_degree_info.info_num;	
	pGuild->info_num = info_num;
	pGuild->first = 0;	// �ӽñ�尡 �ƴϴ�.
	
	// ��� �������� ���ο� ������ �����ش�.
	t_packet packet;
	packet.h.header.type = CMD_REGIST_GUILD_OK;
	packet.h.header.size = sizeof( k_guild_instroduction_basic );
	packet.u.kein.guild_instroduction_basic.guild_code = nGuildCode;
	strcpy(packet.u.kein.guild_instroduction_basic.guild_name, pGuild->guild_name);
	g_pGuildManager->SendPacket(nGuildCode, &packet);
}	//> CSD-030326

void RecvGuildIntroduction( short int cn )
{	//< CSD-030326
	CHARLIST *ch = CheckServerId( cn );
	if( !ch ) return;

	t_packet packet;

	packet.h.header.type = CMD_REQ_GUILD_INTRODUCTION;
	int count = 0;

	CGuildManager::HASH_GUILD mpGuild = g_pGuildManager->GetGuildSet();
	
	for (CGuildManager::ITOR_GUILD i = mpGuild.begin(); i != mpGuild.end(); ++i)
	{
		CGuildInfo* pGuild = i->second;

		if (pGuild->active && ch->name_status.nation == pGuild->nation)
		{
			packet.u.kein.guild_instroduction.guild[count].guild_code = i->first;
			strcpy(packet.u.kein.guild_instroduction.guild[count].guild_name, pGuild->guild_name);
			count++;
		}

		if( count == 100 )
		{
			packet.u.kein.guild_instroduction.count = count;
			packet.h.header.size = sizeof( short int ) + sizeof( k_guild_instroduction_basic )*count;
			QueuePacket( connections, cn, &packet,1 );
			packet.h.header.type = CMD_REQ_GUILD_INTRODUCTION_PLUS;
			count = 0;
		}
	}
	packet.u.kein.guild_instroduction.count = count;
	packet.h.header.size = sizeof( short int ) + sizeof( k_guild_instroduction_basic )*count;
	QueuePacket( connections, cn, &packet,1 );
}	//> CSD-030326

void SendSaveGuildMemberList( char *target_name, int old_guild_code, int guild_code, int degree )
{
	t_packet packet;
	packet.h.header.type = CMD_SAVE_GUILD_MEMBER_LIST;
		packet.u.kein.save_guild_member_list.guild_code = guild_code;
		packet.u.kein.save_guild_member_list.old_guild_code = old_guild_code;
		packet.u.kein.save_guild_member_list.degree = degree;
		strcpy( packet.u.kein.save_guild_member_list.name, target_name );
	packet.h.header.size = sizeof( k_save_guild_member_list );
	QueuePacket(connections, DB_DEMON, &packet, 1);
}

void TransDbGuildMailId( t_packet *p, short int cn )
{
	CHARLIST *ch = CheckServerId( cn );
	if( !ch ) return;

	DWORD mail_id = p->u.kein.default_dword;

	t_packet packet;
	packet.h.header.type = p->h.header.type;
		packet.u.kein.guild_mail_id.guild_mail_id= mail_id;
		strcpy( packet.u.kein.guild_mail_id.name, ch->Name );
	packet.h.header.size = sizeof( k_guild_mail_id );
	QueuePacket(connections, DB_DEMON, &packet, 1);
}

void TransDB_DeleteGuildMail( DWORD mail_id, int cn )
{	//< CSD-030324
	LPCHARLIST ch = CheckServerId( cn );
	if( !ch ) return;

	if( !ch->GetGuildCode() ) return;
	//if( !ch->name_status.guild_master ) return;

	t_packet packet;
	packet.h.header.type = CMD_RECV_MAIL_DELETE_GUILD;
		packet.u.kein.delete_guild_mail.server_id = cn;
		packet.u.kein.delete_guild_mail.guild_code = ch->GetGuildCode();
		packet.u.kein.delete_guild_mail.degree = ch->name_status.guild_master;
		packet.u.kein.delete_guild_mail.mail_id = mail_id;
	packet.h.header.size = sizeof( k_delete_guild_mail );
	QueuePacket(connections, DB_DEMON, &packet, 1);
}	//> CSD-030324

void RecvCheckGuild( t_packet *p, short int cn )
{	//< CSD-030324
	int guild_code = p->u.kein.default_short_int;
	CHARLIST *ch = CheckServerId( cn );
	if( !ch ) return;

	if( ch->GetGuildCode() == guild_code ) return;
	
	k_guild_mail_unit guild_info;
	guild_info.guild_code = ch->GetGuildCode();
	guild_info.degree = ch->name_status.guild_master;
	SendPacketDefault( CMD_CHECK_MY_GUILD, &guild_info, sizeof( k_guild_mail_unit ), cn );
}	//> CSD-030324

void RecvGuildChatMap( t_packet *p )
{	//< CSD-030326
	const int nGuildCode = p->u.kein.guild_chat_map.guild_code;
	const char* name =  p->u.kein.guild_chat_map.name;
	const char* msg = p->u.kein.guild_chat_map.msg;

	t_packet packet;
	packet.h.header.type = CMD_GUILD_CHAT;
	packet.h.header.size = sizeof(k_guild_chat) - 1024 + strlen(msg);
	strcpy(packet.u.kein.guild_chat.name, name);
	strcpy(packet.u.kein.guild_chat.msg, msg);
	g_pGuildManager->SendPacket(nGuildCode, &packet);
}	//> CSD-030326

void RecvGuildChat( t_packet *p, short int cn )
{	//< CSD-030324
	LPCHARLIST ch = CheckServerId( cn );
	if( !ch ) return;

	if( !ch->GetGuildCode() ) return;

	char *msg = (char *)p->u.data;
	t_packet packet;
	packet.h.header.type = CMD_GUILD_CHAT_MAP;
	packet.h.header.size = sizeof( k_guild_chat_map ) - 1024 + strlen(msg );
	packet.u.kein.guild_chat_map.guild_code = ch->GetGuildCode();
	strcpy( packet.u.kein.guild_chat_map.name, ch->Name );
	strcpy( packet.u.kein.guild_chat_map.msg, msg );

	g_pServerTable->BroadCastToEveryServer( (char*)&packet, (sizeof(t_header)+packet.h.header.size), SERVER_TYPE_MAP );
	RecvGuildChatMap( &packet );
}	//> CSD-030324

////////////////////////////////////////////////////////////////////////////////////
// ��� ��й� ��ƾ

inline int GetTotalAbility( LPCHARLIST ch )
{
	return ( ch->Str+ch->Con+ch->Dex+ch->Wis+ch->Int+ch->MoveP+
		ch->Char+ch->Endu+ch->Moral+ch->Luck+ch->wsps+ch->GetReservedPoint());
}

void AddAbility( CHARLIST *ch, int type, int add )
{
	short int *ability;
	switch( type )
	{
		case STR :  ability = &ch->Str; break;
		case DEX :  ability = &ch->Dex; break;
		case CON :  ability = &ch->Con; break;
		case WIS :  ability = &ch->Wis; break;
		case INT_ : ability = &ch->Int; break;
		case CHA :  ability = &ch->Char; break;
		case MOVP : ability = &ch->MoveP; break;
		case ENDU : ability = &ch->Endu; break;
		case MOR :  ability = &ch->Moral; break;
		case WSPS : ability = &ch->wsps; break;
		case LUCK : ability = &ch->Luck; break;
		default : return;
	}
	(*ability) += add;
	return;
}

void RecvReqResetAbility( short int cn )
{
	LPCHARLIST ch = CheckServerId( cn ); 
	if( !ch ) return;
	
	static int class_ability[5][5] = { 
								{ STR,	CON }, 
								{ DEX,	MOVP},
								{ DEX,	CON },
								{ INT_,	WSPS},
								{ WIS,	WSPS} 
	};

	static int basic_ability[5][11] = {
			{ 50, 10, 30, 10, 10, 10, 10, 10, 10, 10, 10 },
			{ 10, 40, 20, 10, 10, 10, 20, 10, 10, 10, 20 },
			{ 10, 50, 20, 10, 10, 10, 20, 10, 10, 10, 10 },
			{ 10, 10, 15, 10, 40, 10, 10, 20, 10, 25, 10 },
			{ 10, 10, 20, 35, 10, 10, 10, 20, 10, 25, 10 },
	};

	int total = GetTotalAbility( ch );
	int ability = 0;
	ability += ch->Str = basic_ability[ch->Class][0];
	ability += ch->Dex = basic_ability[ch->Class][1];
	ability += ch->Con = basic_ability[ch->Class][2];
	ability += ch->Wis = basic_ability[ch->Class][3];
	ability += ch->Int = basic_ability[ch->Class][4];
	ability += ch->Char = basic_ability[ch->Class][5];
	ability += ch->MoveP = basic_ability[ch->Class][6];
	ability += ch->Endu = basic_ability[ch->Class][7];
	ability += ch->Moral = basic_ability[ch->Class][8];
	ability += ch->wsps = basic_ability[ch->Class][9];
	ability += ch->Luck = basic_ability[ch->Class][10];

	//for( int i=0; i<11; i++ ) ability += basic_ability[ch->Class][i];

	AddAbility( ch, class_ability[ch->Class][0], 3 );
	AddAbility( ch, class_ability[ch->Class][1], 2 );
	ability+=5;
	ch->SetReservedPoint(total-ability);
	SendCharInfo( cn );
}

int SendCharInfo( short int cn )
{
	CHARLIST *ch = CheckServerId( cn );
	if( !ch ) return 0;

	t_packet packet;
	packet.h.header.type = CMD_REQ_RESET_ABILITY;
		packet.u.kein.reset_ability.Str  = ch->Str  ;
		packet.u.kein.reset_ability.Con  = ch->Con  ;
		packet.u.kein.reset_ability.Dex  = ch->Dex  ;
		packet.u.kein.reset_ability.Wis  = ch->Wis  ;
		packet.u.kein.reset_ability.Int  = ch->Int  ;
		packet.u.kein.reset_ability.MoveP= ch->MoveP;
		packet.u.kein.reset_ability.Char = ch->Char ;
		packet.u.kein.reset_ability.Endu = ch->Endu ;
		packet.u.kein.reset_ability.Moral= ch->Moral;
		packet.u.kein.reset_ability.Luck = ch->Luck ;
		packet.u.kein.reset_ability.wsps = ch->wsps ;
    packet.u.kein.reset_ability.reserved_point	= ch->GetReservedPoint();
	packet.h.header.size = sizeof( k_reset_ability );
	QueuePacket(connections, cn, &packet, 1);
	return 1;
}

////////////////////////////////////////////////////////////////////////////////////

void RecvDeleteGuild( t_packet *p, short int cn )
{	//< CSD-030324
	CHARLIST *ch = CheckServerId( cn ); 
	if( !ch ) return;
	if( ch->name_status.guild_master != 1 ) return;
	if( !ch->GetGuildCode() ) return;

	// 021008 YGI
	t_packet packet;
	packet.h.header.type = p->h.header.type;
	packet.u.kein.guild_instroduction_basic.guild_code = ch->GetGuildCode();
	strcpy( packet.u.kein.guild_instroduction_basic.guild_name, ch->Name );
	packet.h.header.size = sizeof( k_guild_instroduction_basic );
	QueuePacket(connections, DB_DEMON, &packet, 1);
}	//> CSD-030324

// ��尡 ���� �ƴٴ� ���� ��� �������� �˷� �ش�.
void RecvDeleteGuildOk( t_packet *p )
{	//< CSD-CN-031213
	const int nGuildCode = p->u.kein.default_short_int;
	g_pGuildManager->DelGuild(nGuildCode);

	t_packet packet;
	packet.h.header.type = CMD_GUILD_INFO_DELETED;
	packet.u.kein.default_short_int = nGuildCode;
	packet.h.header.size = sizeof(short int);
	g_pUserManager->SendPacket(&packet);
}	//> CSD-CN-031213

void RecvGetGhostQuestInfo( short int cn )
{
	t_packet packet;
	packet.h.header.type = CMD_GET_GHOST_QUEST_INFO;
	packet.u.data[0] =	var[cn][s_GhostLvDef];
	packet.u.data[1] =	var[cn][s_GhostEndCountDef];
	packet.h.header.size = sizeof( short int );
	QueuePacket(connections, cn, &packet, 1);
}

//////////////////////////////////////////////////////////////
// �׺�
int CGambleRateData::LoadData()
{
	char file[MAX_PATH];
	sprintf( file, "%s/data/GambleRateData.bin", GameServerDataPath );
	FILE *fp = fopen( file, "rb" );
	if( fp )
	{
		fread( &g_game_rate_data, sizeof( k_GambleRateDataBasic ), 1, fp );
		fclose(fp);
	}
	else
	{
		memcpy( rune2band, g_rune_band, sizeof( int )*8 );
		memcpy( lv2band, g_lv2band, sizeof( int )*10 );
		memcpy( choice_house, g_choice_house, sizeof( int )*10*5 );
		memcpy( house_band_class, g_house_band_class, sizeof( int )*5*11*6);
		memcpy( item2house_type, g_item2house_type, sizeof( int )*10*6);
		memcpy( give_gamble_item_rate, g_give_gamble_item_rate, sizeof( int )*16*2);
		memcpy( give_gamble_item_rate, g_give_gamble_item_rate, sizeof( int )*16*2);
		memcpy( map2band, g_map2band, sizeof( int ) * 6*2 );
		memcpy( create_today_item, g_create_today_item, sizeof( int ) * 8 );
		get_today_item = g_get_today_item;
	}
	return 1;
}

int CGambleRateData::SaveData()
{
	FILE *fp = fopen( "./output/GambleRateData.bin", "wb" );
	if( fp )
	{
		fwrite( &g_game_rate_data, sizeof( k_GambleRateDataBasic ), 1, fp );
		fclose(fp);
	}
	return 1;
}
/////////////////////////////////////////////////


int GetLevelBand( int lv, int map_count, int rune )		// �ڽ��� ������ �� �ʿ� ������ ���� ���� ��� ����
{
	int para = (lv-1)/10;
	if( para > 9 ) para = 9;
	int basic = g_game_rate_data.lv2band[para];

	para = 0;
	for( int i=0; i<6; i++ )
	{
		if( map_count < g_game_rate_data.map2band[i][0] )
		{
			para = g_game_rate_data.map2band[i][1];
			break;
		}
	}
	basic += para;
	basic += g_game_rate_data.rune2band[rune];	

	if( basic < 0 ) basic = 0;
	if( basic > 10 ) basic = 10;
	return basic;
}

// 1���� �迭�� �ѱ�� �� �Ѱ踦 �Ѱ� �ָ� ���� ���Ͽ� �� ���� ���� �ʴ� ���� �ִ�
// �� ÷�� ���� ��ȯ�Ѵ�.
inline int ChoiceParam( int *arg, int arg_count, int cmp_value )
{
	int count = 0;		// ���� ����
	for( int i=0; i<arg_count; i++ )
	{
		count += arg[i];
		if( cmp_value < count ) return i;
	}
	return 0;
}

void ProcGambleItem( int house_type, CHARLIST *ch )
{
	int lv = ch->GetLevel(); // CSD-030806
	int user_count = (g_pServerTable)?g_pServerTable->GetNumOfUsersInServerSet():0;

	//int house_max = sizeof(choice_house) / sizeof(int) * 5;

	// 1. house_type �� ���� �漱��
	int house_rand = rand()%1000;
	int house = ChoiceParam( g_game_rate_data.choice_house[house_type], 5, house_rand );

	// 2. ������ ���� ���� ���� ���� �������� ����Ű�� band����
	int lv_band = GetLevelBand( lv, user_count, (int)ch->select_rune );

	int cls_rand = rand()%100;
	int cls_param = ChoiceParam( g_game_rate_data.house_band_class[house][lv_band], 6, cls_rand );

	// house ������ cls_param ������ �̿��� ����
	const int item_no = g_GambleItem.GetRandItem( house, cls_param );

	if (item_no == -1)
	{	//< CSD-040223
		MyLog(LOG_NORMAL, "[Gamble Item] house(%d), cls_param(%d)", house, cls_param);
		return;
	}	//> CSD-040223
	
	ItemAttr item = MakeGambleItem( house, cls_param, item_no );
	SaveGambleLogFile( ch, house, cls_param, user_count, &item, house_type, lv_band );
	SendMessageGambleItem( ch, &item );

	SendItemEventLog( &item, ch->GetServerID(), SN_NOT_USER, SILT_GET_GAMBLE_ITEM, 2 ); //YGI acer
	MoveEmptyInv( &item, ch );
}

void RecvSearchDualCharacter( t_packet *p, short int cn )
{	//< CSD-030415
	CHARLIST *ch = CheckServerId( cn ); 
	if( !ch ) return;

	t_packet packet;
	char &count = packet.u.kein.scan_target.count;

	char type = 0;
	if	( !ch->IsDual() ) 
	{
		type = 1;		// ����� �ƴմϴ�.
		return;
	}
	if( ch->GetClassStep() == 1 )
	{
		if( !var[cn][s_GhostEndCountDef] ) 
		{
			type = 2;	// ����Ʈ �������� �ƴմϴ�.
			return;
		}
	}

	packet.h.header.type = CMD_SCAN_TARGET;	
	count = 0;

	const int nDualStep = ch->GetClassStep();
	CDualClassInfo* pDualClass = g_pDualManager->GetDualClassInfo(nDualStep);

	if (pDualClass == NULL)
	{
		return;
	}

	DWORD idMember = 0;
	CDualClassInfo::LIST_MEMBER ltMember = pDualClass->GetDualClassList();

	for (CDualClassInfo::ITOR_MEMBER i = ltMember.begin(); i != ltMember.end(); ++i)
	{	//< CSD-HK-030829
		const WORD idTarget = *i;
		CHARLIST* pTarget = CheckServerId(idTarget);

		if (pTarget == NULL)
		{
			continue;
		}

		if (cn == idTarget)
		{
			continue;
		}

		packet.u.kein.scan_target.xy[count].x = pTarget->X/TILE_SIZE*200/g_Map.file.wWidth;
		packet.u.kein.scan_target.xy[count].y = pTarget->Y/TILE_SIZE*200/g_Map.file.wHeight;
		
		if (++count >= 250)
		{
			break;
		}
	}	//> CSD-HK-030829
	
	packet.h.header.size = sizeof( k_scan_target ) - sizeof( k_x_y )*(250-count); ;
	QueuePacket(connections, cn, &packet, 1);
}	//> CSD-030415

// << 011019 ygi
void RecvChangeColor( t_packet *p, short int cn )
{
	CHARLIST *ch = CheckServerId( cn );
	if( !ch ) return;

	int type1 = p->u.kein.change_color.type1;		// ���� ���°� �ƴѰ�
	int type2 = p->u.kein.change_color.type2;		// �ٵ�Į������ �� Į�� ����
	int r = p->u.kein.change_color.r;
	int g = p->u.kein.change_color.g;
	int b = p->u.kein.change_color.b;

	if( type1 )		// ���� ���� �Ѵٸ�
	{	//< CSD-030806
		const int nLevel = ch->GetLevel();
		
		if (ch->Money < NPC_Lev_Ref[nLevel].nDyeingPrice)
		{
			return;
		}
		
		SubtractMoney(NPC_Lev_Ref[nLevel].nDyeingPrice, ch);
	}	//> CSD-030806
	if( type2 )		// �ʻ��� �ٲٱ�
	{
		ch->BodyR = r;
		ch->BodyG = g;
		ch->BodyB = b;
	}
	else
	{
		ch->ClothR = r;
		ch->ClothG = g;
		ch->ClothB = b;
	}

	t_packet packet;		// DB �� �����Ѵ�. 
	packet.h.header.type = CMD_CHANGE_COLOR;
		packet.u.kein.change_color_db.type = type2;		// ������ �� ���� ����
		packet.u.kein.change_color_db.r = r;
		packet.u.kein.change_color_db.g = g;
		packet.u.kein.change_color_db.b = b;
		strcpy( packet.u.kein.change_color_db.name, ch->Name );
	packet.h.header.size = sizeof( k_change_color_db );
	QueuePacket(connections, DB_DEMON, &packet, 1);

	// �������� �˷� �ش�.
	packet.h.header.type = CMD_CHANGE_COLOR;
		packet.u.kein.change_color_map2client.type = type2;		// ������ �� ���� ����
		packet.u.kein.change_color_map2client.r = r;
		packet.u.kein.change_color_map2client.g = g;
		packet.u.kein.change_color_map2client.b = b;
		packet.u.kein.change_color_map2client.server_id = cn;
	packet.h.header.size = sizeof( k_change_color_map2client );
	QueuePacket(connections, cn, &packet, 1);
	CastMe2Other( cn, &packet );
}

void RecvGetNationMoney( short int cn, int nation )
{
	CHARLIST *ch = CheckServerId( cn );
	if( !ch ) return;

	if( nation <= 0 )
	{
		nation = ch->name_status.nation;
	}

	t_packet packet;		// DB �� �����Ѵ�. 
	packet.h.header.type = CMD_REP_NATION_MONEY;
		packet.u.kein.nation_money.nation_type = nation;

		if( nation >= N_NATION_ALL )		// ��� ������ ������ ��û���� ��� 
		{
			packet.u.kein.nation_money.nation_money[0] = NationInfo[3].nation_money;
			packet.u.kein.nation_money.nation_money[1] = NationInfo[4].nation_money;
			packet.u.kein.nation_money.nation_money[2] = NationInfo[6].nation_money;
			packet.h.header.size = sizeof( k_nation_money );
		}
		else
		{
			packet.u.kein.nation_money.nation_money[0] = NationInfo[nation].nation_money;
			packet.h.header.size = sizeof( k_nation_money ) - sizeof(DWORD)*2;
		}

	QueuePacket(connections, cn, &packet, 1);
}

inline void SendDirectMap( t_packet *p, short int port )
{
	if( port == g_MapPort )
		HandleCommand( CN_FROM_OTHER_MAPSERVER_MSG, p );
	else 
		g_pServerTable->SendRajaPacketToOtherMapServer( (WORD)port, (char *)p, p->h.header.size+sizeof(t_header) );
}

int InitRefreshMenu()
{
	if( g_Money_to_Fame.LoadData() )
	{
		MyLog( LOG_NORMAL, "%s/data/money_fame.txt loding............ ok", GameServerDataPath  );
	}
	else
	{
		MyLog( LOG_NORMAL, "%s/data/money_fame.txt loding............ Fail !!!!!!", GameServerDataPath  );
	}
	g_game_rate_data.LoadData();
	RefreshTodayItem();
	LoadMapPotal();
	LoadNationItem(g_wday );
	LoadGuildHouse();
	g_EventMgr.LoadEvent();
	InitItemLog( GetManagementMapPort( MM_SAVE_LOG_FILE_MAP ) );

	// 021117 YGI
	g_LightVersion.LoadLightVersionMap();

	// 030624 YGI
	extern bool InitEventFlag();
	InitEventFlag();
	//< CSD-031229
	//ItemMgr.LoadHaveLimitItem();	// 031009 CI YGI
	//> CSD-031229
	return 1;
}

int InitOnlyStart_kein()		// ��ó������ �ε��� �͵�
{
	LoadDungeonMap();
	LoadDungeonMapAll();
	
	return 1;
}

// DB�� �־� �α� ���� ����Ÿ�� �ҽ����� ������ų��
void __stdcall MakeMapDataFile(DWORD dwValue)//020511 lsw	// VK_F7
{
	g_game_rate_data.SaveData();
	MyLog( LOG_NORMAL, " " );
	MyLog( LOG_NORMAL, "Make Server data files.... ok" );
	MyLog( LOG_NORMAL, " " );

	// 011130 YGI
	extern void LoadDRAGON_MAX_CONNECTIONS();
	LoadDRAGON_MAX_CONNECTIONS();

}

void RecvGambleItem( t_packet *p, short int cn )
{
	int count = p->u.kein.gamble_item.count;
	POS *pos =  p->u.kein.gamble_item.pos;

	if( !count ) return;

	CHARLIST *ch = CheckServerId( cn );
	if(!ch ) return;

	ItemAttr *item;
	CItem	*t;

	int gen1=0;			// �Ϲ� �׺� ������
	int gen2=0;			// �Ϲ� �׺� ������ 2
	int grade1=0;		// �׷��̵� �׺� 1
	int grade2=0;		// �׷��̵� �׺� 2
	int gen_item=0;		// �Ϲ� ������
	int grade_item=0;	// ���� ������
	int i;
	for( i=0; i<count; i++ )
	{
		item = GetItemByPOS( cn, pos[i] );
		if( !item ) continue;

		t = ItemUnit( *item );
		if( !t ) continue;

		if( item->attr[IATTR_RARE_MAIN] )	grade_item++;// �����..
		else if( t->GetItemKind() == IK_GRADE_GAMBLE ) grade1++;
		else if( t->GetItemKind() == IK_GENERAL_GAMBLE ) gen1++;
		else gen_item++;

		SendItemEventLog( item, cn, SN_NOT_USER, SILT_PUT_GAMBLE_ITEM, 4 ); //YGI acer
		DeleteItem( item );
	}

	ItemAttr today_item = g_TodayGambleItem.GetTodayItem();
	if( grade1 >= 5 && today_item.item_no )
	{
		if( WinOfRandFunction( g_game_rate_data.get_today_item ) )
		{
			SaveGambleLogFile( ch, 100, 100, 100, &today_item, 100, 100);
			SendMessageGambleItem( ch, &today_item );
			
			// ��ü �޽��� ������
			t_packet packet;
			packet.h.header.type = CMD_TODAY_ITEM_GET_WHO;
			packet.u.kein.today_gamble_item.item = today_item;
			packet.u.kein.today_gamble_item.nation = ch->name_status.nation;
			strcpy( packet.u.kein.today_gamble_item.name, ch->Name );
			packet.h.header.size = sizeof( k_today_gamble_item ) - 20 +sizeof( ch->Name );
			SendPacket2Maps( &packet );
 			g_pUserManager->SendPacket(&packet); // CSD-CN-031213

			SendItemEventLog( &today_item, cn, SN_NOT_USER, SILT_GET_TODAY_ITEM, 2 ); //YGI acer
			MoveEmptyInv( &today_item, ch );
			RecvDeleteTodayGambleItem();
			return;
		}
	}

	int house_type = -1;
	for( i=9; i>=0; i-- )
	{
		if(	( gen1 >= g_game_rate_data.item2house_type[i][0] ) &&
			( gen2 >= g_game_rate_data.item2house_type[i][1] ) &&
			( grade1 >= g_game_rate_data.item2house_type[i][2] ) &&
			( grade2 >= g_game_rate_data.item2house_type[i][3] ) &&
			( gen_item >= g_game_rate_data.item2house_type[i][4] ) &&
			( grade_item >= g_game_rate_data.item2house_type[i][5] ) )
		{
			house_type = i; 
			break;
		}
	}
	if( house_type == -1 )
	{
		char result = 1;		// �׺��� �������� ����.
		SendDefaultResult( CMD_GAMBLE_ITEM_RESULT, result, cn );
		return;
	}
	ProcGambleItem( house_type, ch );
}

ItemAttr MakeGambleItem( int house, int grade, int item_no )
{
	ItemAttr item;
	RareMain RareAttr;
	switch( house )
	{
		default :
			item = GenerateItem( item_no );
			break;
		case 4:	
			item = ItemMgr.GiveRareItem( item_no, 0, RARE_ABLE|RARE_ABLE_GAMBLE_ROOM_4, 2, grade, grade, RareAttr ,H_LV_HIGH_ITEM);//020725 lsw
			break;
		case 3:
			item = ItemMgr.GiveRareItem( item_no, 0, RARE_ABLE|RARE_ABLE_GAMBLE_ROOM_5, 1, grade, grade, RareAttr ,H_LV_HIGH_ITEM);//020725 lsw
			break;
	}

	return item;
}

void SaveGambleLogFile(CHARLIST *ch, int house, int class_type, int user_count, ItemAttr *item, int resource_type, int lv_band)
{	//< CSD-TW-030622
	string strPath;

	if (!g_pLogManager->GetLogPath(strPath))
	{
		return;
	}

	char file[MAX_PATH]= {0,};
	::sprintf( file, "%s/gamble_item%d.txt", strPath.c_str(), house );	// 030506 YGI
	FILE *fp = ::fopen( file, "at+" );
	if(!fp){return;}

	fprintf( fp, "---%02d.%02d %02d:%02d:%02d ---\n",  g_mon+1, g_day, g_hour, g_min, g_sec );
	fprintf( fp, "item_no[%d]	house:%d	room:%d		%s(lv:%d)\n", item->item_no, house, class_type, ch->Name, ch->GetLevel()); // CSD-030806
	fprintf( fp, "resource_type:%d	uesr:%d	lv_band%d\n\n", resource_type, user_count, lv_band );	

	fclose( fp );
}	//> CSD-TW-030622

void RecvGambleStart( short int cn )
{
	CHARLIST *ch = CheckServerId( cn );
	if( !ch ) return;

	ch->select_rune = rand()%8;	
	SendPacketDefault( CMD_GAMBLE_START, &ch->select_rune, sizeof( BYTE ), cn );
}

void SendMessageGambleItem( CHARLIST *ch, ItemAttr *item )
{
	t_packet packet;
	packet.h.header.type = CMD_GAMBLE_ITEM_RESULT;
		packet.u.kein.gamble_item_result.result_type = 2;		// � �޽��� ���� ��� Ÿ�� 
		packet.u.kein.gamble_item_result.item_no = item->item_no;
		strcpy( packet.u.kein.gamble_item_result.name, ch->Name );
	packet.h.header.size = sizeof( k_gamble_item_result );
	QueuePacket(connections, ch->GetServerID(), &packet, 1);
}

void RecvLogoutChar( t_packet *p, short int cn )
{
	char *name = p->u.kein.default_name;
	int who = ExistHe( name );
	CHARLIST *target = CheckServerId( who );
	if( target )
	{
		// ���� �α� �ƿ�
		SendPutMenuString( KM_FAIL, 87, who );		// ���� ���� ���ϴ� �������
		closeconnection( connections, who, -33 );	// ��ڿ� ���� ���� ����

		if( cn == CN_FROM_OTHER_MAPSERVER_MSG )		// �ٸ� �ʿ��� �°Ŷ��
		{
			int server_id = p->u.kein.focuse_logout_map.cn;
			int port = p->u.kein.focuse_logout_map.map_port;
			
			k_put_menu_string msg;
			msg.type = KM_INFO;
			msg.str_num = 88;		// ���� ���� �� �������
			
			SendDirectClientOtherMap( CMD_PUT_MENU_STRING, server_id, port, &msg, sizeof( k_put_menu_string ) );
		}
		else
		{
			SendPutMenuString( KM_OK, 88, cn );		// ���� ���� ���ϴ� �������
			//acer7
			CrackMgr.SendHackingUser( cn, USE_GM_TOOL, "Absolute LogOut by name" );
		}
		return;
	}
	else
	{
		if( cn == CN_FROM_OTHER_MAPSERVER_MSG )		// �ٸ� �ʿ��� �°Ŷ��
		{
			return;
		}
		else
		{
			CHARLIST *ch = CheckServerId( cn );
			if( !ch ) return;

			// �ٸ� ������ ������.
			t_packet packet;
			packet.h.header.type = p->h.header.type;
				strcpy( packet.u.kein.focuse_logout_map.name, name );
				packet.u.kein.focuse_logout_map.map_port=g_MapPort;
				packet.u.kein.focuse_logout_map.cn = cn;
			packet.h.header.size = sizeof( k_focuse_logout_map );
			g_pServerTable->BroadCastToEveryServer( (char*)&packet, (sizeof(t_header)+packet.h.header.size), SERVER_TYPE_MAP );

			//acer7
			CrackMgr.SendHackingUser( cn, USE_GM_TOOL, "Absolute LogOut by name" );
		}
	}
}

void SendPutMenuString( int type, int str_num, short int cn )
{
	t_packet packet;
	packet.h.header.type = CMD_PUT_MENU_STRING;
		packet.u.kein.put_menu_string.type = type;
		packet.u.kein.put_menu_string.str_num = str_num;
	packet.h.header.size = sizeof( k_put_menu_string);
	QueuePacket( connections, cn, &packet, 1 );
}

// 021107 YGI
void SendDirectClientOtherMap( int type, short int server_id, short int port, void *msg, int size )
{
	t_packet packet;
	packet.h.header.type = CMD_SEND_DIRECT_CLIENT_TO_MAP;
		packet.u.kein.direct_map2user.type = type;
		packet.u.kein.direct_map2user.server_id = server_id;
		if( msg )
		{
			memcpy( packet.u.kein.direct_map2user.data, msg, size );
		}		
	packet.h.header.size = size+4;
	if( port == GetOwnPort() )		// ���� ������ ���� ���
	{
		HandleCommand( CN_FROM_OTHER_MAPSERVER_MSG, &packet );
	}
	else
	{
		g_pServerTable->SendRajaPacketToOtherMapServer( (WORD)port, (char *)&packet, packet.h.header.size+sizeof(t_header) );	
	}
}

int RecvSEND_DIRECT_CLIENT_TO_MAP( t_packet *p )
{
	int type = p->u.kein.direct_map2user.type;
	int cn = p->u.kein.direct_map2user.server_id;
	void *msg = (void *)p->u.kein.direct_map2user.data;
	t_packet packet;
	packet.h.header.type = type;
	packet.h.header.size = p->h.header.size-4;
	memcpy( packet.u.data, msg, packet.h.header.size );
	QueuePacket( connections, cn, &packet, 1 );
	return 1;
}

void RecvReporterMode( t_packet *p, short int cn )
{
	CHARLIST *ch = CheckServerId( cn ); 
	if( !ch ) return;
	
	if( !ch->IsReporter() ) return;		// ���ڰ� �ƴϴ�.
	if( !ch->reporter_mode && ch->viewtype == VIEWTYPE_GHOST_ ) return;		// ���ɻ��¿��� ���ɻ��·� ������ �Ұ��
	ch->reporter_mode = 1 - ch->reporter_mode;

	if( ch->reporter_mode )		// ���ڴ� ����̸�
	{
		// �������� �����.. 
		ch->Hp = 1;
		ch->bAlive = DEAD_;
		ch->viewtype = VIEWTYPE_GHOST_;
		ch->InitCurse(); 
		SendViewTypeChange( cn, VIEWTYPE_GHOST_ );
		ch->SendCharInfoBasic( HP, 0 );////020704 lsw
	}
	else
	{
		// ���ɿ��� �������� ���� �ش�.
		SkillMgr.CharacterToAlive(ch,1);// ����ش�.//020501 lsw
	}
	SendPacketDefault( CMD_REPORTER_MODE, &ch->reporter_mode, sizeof( char ), cn );
}

// 011130 YGI
void LoadDRAGON_MAX_CONNECTIONS()
{
	int max = 0;
	max = GetPrivateProfileInt("Option","DRAGON_MAX_CONNECTIONS",0,MAP_SERVER_INI_);	
	if( !max ) return;
	if( max > DRAGON_MAX_CONNECTIONS_ )
	{	
		MyLog( LOG_IMPORTANT, "DRAGON_MAX_CONNECTIONS over %d", DRAGON_MAX_CONNECTIONS_ );
		return;
	}
	if( max < DRAGON_MAX_CONNECTIONS )
	{
		MyLog( LOG_IMPORTANT, "ERROR!!!  new DRAGON_MAX_CONNECTIONS: %d, current DRAGON_MAX_CONNECTIONS: %d", max, DRAGON_MAX_CONNECTIONS );
		return;
	}
	
	DRAGON_MAX_CONNECTIONS = max;
	MyLog( LOG_NORMAL, "DRAGON_MAX_CONNECTIONS change OK!!!  DRAGON_MAX_CONNECTIONS number = %d", DRAGON_MAX_CONNECTIONS );
	return;
}

void RecvSetTodayGambleItem( t_packet *p )
{
	ItemAttr *item = &p->u.kein.default_item;
	g_TodayGambleItem.SetTodayItem( item );
}

void RecvDeleteTodayGambleItem()
{
	g_TodayGambleItem.DeleteTodyaItem();
}

void RecvGetTodayGambleItem( short int cn )
{
	SendPacketDefault( CMD_GET_TODAY_GAMBLE_ITEM, 
		&g_TodayGambleItem.GetTodayItem(), sizeof(ItemAttr ), cn );
}

void RecvChangePartySelectExp( t_packet *p, short int cn )
{
	CHARLIST *ch = CheckServerId( cn );
	if( !ch ) return;

	ch->party_select = p->u.kein.default_char;
}

// 020111 YGI
int WinOfRandFunction( int count )
{
	// ����� 1 �� Ȯ�� ���
	if( !count ) return 0;
	if( rand()%count == count-1 ) return 1;
	return 0;
}
/*
void RecvGetPotalMap( short int cn )
{
	CHARLIST *ch = CheckServerId( cn );
	if( !ch ) return;

	int nation = ch->name_status.nation;
	int max = g_MapPotal[nation].GetCount();

	t_packet packet;
	packet.h.header.type = CMD_GET_POTAL_MAP;
	packet.h.header.size = sizeof( k_potal_map_unit	) * max+1;
		packet.u.kein.potal_map.count = max;
	
	if( !max ) return;
	for( int i=0; i<max; i++ )
	{
		//��Ŷ�� ������ ��´�.
		packet.u.kein.potal_map.map[i].map_number = g_MapPotal[nation].GetMapNumber( i );
		packet.u.kein.potal_map.map[i].map_money = g_MapPotal[nation].GetMapMoney( i );
	}

	QueuePacket( connections, cn, &packet, 1 );
}*/

void RecvGotoPotal( t_packet *p, short int cn )
{
	int para = p->u.kein.default_char;
	CHARLIST *ch = CheckServerId( cn );
	if( !ch ) return;

	int nation = MapInfo[MapNumber].nation;
	if( !nation ) nation = ch->name_status.nation;

	int money = g_MapPotal[nation].GetMapMoney( para );
	if( ch->Money < money ) return;// ���� ����

	int level = g_MapPotal[nation].GetMapLevel( para );
	if (ch->GetLevel() < level) return; // CSD-030806

	// �� �̵�
	int map_num = g_MapPotal[nation].GetMapNumber( para );
	char *map_name = MapInfo[map_num].mapfile;
	int x, y;
	g_MapPotal[nation].GetMapXY( para, x, y );

	char temp_name[30];
	strcpy( temp_name, MapName );
	CharUpper( temp_name );
	if( strcmp( map_name, temp_name) == 0 )
	{
		// ���� �������δ� �̵��� �� �� ����.
		SendPutMenuString( KM_FAIL, 128, cn );
		return;
	}

	SubtractMoney( money, ch );
	if( !MapMove( cn, map_name, x, y ) )
	{	// ���� ���� ��� 
		::AddMoney(money,ch);//021126 lsw
	}
}

void RecvSetMapType( t_packet *p, short int cn )
{
	int map_num;
	if( cn == CN_FROM_OTHER_MAPSERVER_MSG )		// �ʿ��� �Դ�.
	{
		map_num = p->u.kein.set_map_type.map_num;
		MapInfo[map_num].m_MoveType = p->u.kein.set_map_type.type;

		// 030811
		MyLog( 0, "Change MapMove Type [%s]'s type = %d", MapInfo[map_num].mapfile, MapInfo[map_num].m_MoveType );
	}
	else	// Ŭ���̾�Ʈ�� ��Ŷ�� ���� ��� 
	{
		// �� ���� ���� ���̸� ������ ���� �ʰ� �Ѵ�.
		if( !strcmp( MapName, "MA-IN" )
			|| !strcmp( MapName, "RENES_C" )
			|| !strcmp( MapName, "BARANTAN" ) )
			return;		// �߿� ���� ���̴�.

		int type = p->u.kein.default_char;
		int real_type = MMT_NORMAL;
		switch( type )
		{
			case 1 : // ����
				{
					real_type = MMT_NORMAL;
					
					// �̺�Ʈ ������ �˸���. // ���� ����	// �� ����
					t_packet packet;
					packet.h.header.type = CMD_PUT_MENU_STRING;
						packet.u.kein.put_menu_string.type = KM_BBS;
						packet.u.kein.put_menu_string.str_num = 117;
					packet.h.header.size = sizeof( k_put_menu_string);
					SendPacket2Maps( &packet );	// ��� �ʿ� �ѷ��ش�.
					g_pUserManager->SendPacket(&packet); // CSD-CN-031213
					break;
				}
			case 2 :
				{
					real_type = MMT_NOT_MOVE;
					
					// �̺�Ʈ ���۵����� �˸���.	// ���� �Ұ�
					t_packet packet;
					packet.h.header.type = CMD_PUT_MENU_STRING;
						packet.u.kein.put_menu_string.type = KM_BBS;
						packet.u.kein.put_menu_string.str_num = 118;
					packet.h.header.size = sizeof( k_put_menu_string);
					SendPacket2Maps( &packet );	// ��� �ʿ� �ѷ��ش�.
					g_pUserManager->SendPacket(&packet); // CSD-CN-031213
					break;
				}
			case 3:
				{
					real_type = MMT_NOT_MOVE;		// ���� �Ұ�
					// ��ڸ� �����, ��� �ñ��.
					UserAllMapMove( 1, "G_BATTLE", 111, 116 );
					break;
				}
			case 4:
				{
					real_type = MMT_NOT_MOVE;		// ���� �Ұ� ����
					break;
				}

			case 0 :
				break;
		}
		MapInfo[MapNumber].m_MoveType = real_type ;
		
		p->h.header.size = sizeof( k_set_map_type );
			p->u.kein.set_map_type.map_num = MapNumber;
			p->u.kein.set_map_type.type = real_type ;
		SendPacket2Maps( p );	// ��� �ʿ� �ѷ��ش�.
		// ������ ����� �˷��ش�.
		// �̺�Ʈ ������ �����߽��ϴ�. �������ʹ� �� ���� �̺�Ʈ ���� �ǰ� ���̵��� ���� ���մϴ�.
	}
}
void RecvCheckGuildChange( t_packet *p, short int cn )
{
	CHARLIST *ch = CheckServerId( cn ); 
	if( !ch) return;

	t_packet packet;
	packet.h.header.type = CMD_CHECK_CHANGE_GUILD_INFO;
	packet.h.header.size= sizeof( k_check_guild_change );


	int type = p->u.kein.default_char;
	packet.u.kein.check_guild_change.type = type;
	switch( type )
	{
		case 1 : // ��� ��ũ ����
			{	// �Ķ� �׳� 10��, ���� �ñ׳� 5��, �� 200000
				//< 040331-YGI 
				if( !CheckGuildPower( GDP_CHANGE_MARK, ch ) )
				{
					packet.u.kein.check_guild_change.ret = 0;		// ����
					QueuePacket( connections, cn, &packet, 1 );
					return;
				}
				//> 040331-YGI

				int ret = 0;
				if( ( GetItemMuchByMuchItem( ch, IN_RED_SIGNET) < 5 )		// ����
					|| ( GetItemMuchByMuchItem( ch, IN_BLUE_SIGNET) < 10 )		// �Ķ�
					|| ( ch->Money < 200000 ) )
				{
					packet.u.kein.check_guild_change.ret = 0;		// ����
					QueuePacket( connections, cn, &packet, 1 );
					return;
				}

				SendDeleteMuchItemQuantity( ch, IN_RED_SIGNET, 5 );
				SendDeleteMuchItemQuantity( ch, IN_BLUE_SIGNET, 10 );
				SubtractMoney( 200000, ch );
				
				packet.u.kein.check_guild_change.ret = 1;		// ����
				QueuePacket( connections, cn, &packet, 1 );
				break;
			}
		case 2 :	// ��� ��å ����
			{	// �Ķ� �ñ׳� 5��, ���� �ñ׳� 3��, �� 100000
				//< 040331-YGI
				if( !CheckGuildPower( GDP_CHANGE_DEGREE_INFO, ch ) )
				{
					packet.u.kein.check_guild_change.ret = 0;		// ����
					QueuePacket( connections, cn, &packet, 1 );
					return;
				}
				//> 040331-YGI
				int ret = 0;
				if( ( GetItemMuchByMuchItem( ch, IN_RED_SIGNET ) < 3 )		// ����
					|| ( GetItemMuchByMuchItem( ch, IN_BLUE_SIGNET) < 5 )		// �Ķ�
					|| ( ch->Money < 100000 ) )
				{
					packet.u.kein.check_guild_change.ret = 0;		// ����
					QueuePacket( connections, cn, &packet, 1 );
					return;
				}

				SendDeleteMuchItemQuantity( ch, IN_RED_SIGNET, 3 );
				SendDeleteMuchItemQuantity( ch, IN_BLUE_SIGNET, 5 );
				SubtractMoney( 100000, ch );
				
				packet.u.kein.check_guild_change.ret = 1;		// ����
				QueuePacket( connections, cn, &packet, 1 );
				break;
			}
	}
}


// 020701 YGI
extern void ScenarioServerReset( WORD port );
void DisconnectServer_kein( WORD port )
{
}

void ConnectServer_kein( WORD port )
{
	if( !port ) return;
	ScenarioServerReset( port );
}
