// ExchangeMgr.cpp: implementation of the CExchangeMgr class.
//
//////////////////////////////////////////////////////////////////////

#include "..\stdafx.h"
#include "CItem.h"
#include "Auction.h"
#include "DefaultHeader.h"
#include "DR_NETWORK.h"
#include "GMMGR.h"
#include "ExchangeMgr.h"
#include "GameplaySafety.h"

// 交易只能引用自己的有效物品槽，不能信任客户端传来的数组下标。
static ItemAttr* GetTradeItem(CHARLIST* player, const POS& pos)
{
    if (!player) return NULL;
    switch (pos.type)
    {
    case HAND: return &player->handheld;
    case INV:
        if (!IsTradeItemIndexValid(pos.p1, sizeof(player->inv) / sizeof(player->inv[0])) ||
            !IsTradeItemIndexValid(pos.p2, sizeof(player->inv[0]) / sizeof(player->inv[0][0])) ||
            !IsTradeItemIndexValid(pos.p3, sizeof(player->inv[0][0]) / sizeof(player->inv[0][0][0]))) return NULL;
        return &player->inv[pos.p1][pos.p2][pos.p3];
    case QUICK:
        if (!IsTradeItemIndexValid(pos.p3, sizeof(player->quick) / sizeof(player->quick[0]))) return NULL;
        return &player->quick[pos.p3];
    default: return NULL;
    }
}

static bool ValidateNormalTrade(CHARLIST* caster, CHARLIST* target)
{
    return ValidateTradeOffers(MAX_EXCHANGE_ITEM, [&](int i) {
        const K_ITEM& offer = caster->give[i];
        if (!offer.item_attr.item_no) return !target->recv[i].item_attr.item_no;
        const ItemAttr* current = GetTradeItem(caster, offer.item_pos);
        return current && !memcmp(current, &offer.item_attr, sizeof(ItemAttr)) &&
            EqualPosByPos(&caster->give[i].item_pos, &target->recv[i].item_pos) &&
            !memcmp(&offer.item_attr, &target->recv[i].item_attr, sizeof(ItemAttr));
    });
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CExchangeMgr::CExchangeMgr()
{

}

CExchangeMgr::~CExchangeMgr()
{

}


// 011120 YGI
int RecvItemExchange( K_ITEM *k_item, int you_id, int cn , const int iExchangeType)//�ŷ��� �ϰ� �ʹٴ� �䱸
{
	//////////////////// exist check //////////////
	CHARLIST *ch	= ::CheckServerId( cn );
	CHARLIST *you	= ::CheckServerId( you_id );
	if( !ch || !you ) {return 0;}

	if( ch->IsDead() || you->IsDead() )//�׾��ٸ�.
	{
		return 0;
	}

	if (!CanSaveUserData(ch, 6) || !CanSaveUserData(you, 6))
	{	//< CSD-040212
		return 0;
	}	//> CSD-040212

	if(!ch->IsTradeOn() || !you->IsTradeOn())
	{
		if(!ch->IsTradeOn()){::OutMessage(ch,2,920);}//��ȯ �ź� ���� Ȯ���� �������� �ŷ� �Ͻʽÿ�.
		else				{::OutMessage(ch,2,921);}//��밡 ��ȯ�� �ź�

		if(!you->IsTradeOn())	{::OutMessage(you,2,920);}//��ȯ �ź� ���� Ȯ���� �������� �ŷ� �Ͻʽÿ�.
		else					{::OutMessage(you,2,921);}//��밡 ��ȯ�� �ź�
		
		return 0;
	}

	if( EXCHANGE_STATE_READY != you->GetExchangeState() )		// �׻���� �������� �ŷ��� �ϰ� ������
	{
		SendExchangeItemResult( cn, 0 ); 
		return 0;
	}
	if( EXCHANGE_STATE_READY != ch->GetExchangeState() )	// ���� �������� �ŷ��ϰ� �����鼭 
	{
		SendExchangeItemResult( cn, 1 ); // �ٸ� ����ϰ� �ŷ��ϰ� �ִٴ� ��~
		return 0;
	}
	if( ch->fight_flag > 0 )
	{
		SendExchangeItemResult( cn, 5 ); // 1:1 ���� ���̴�.
		return 0;
	}
	if( you->fight_flag > 0 )
	{
		SendExchangeItemResult( cn, 5 ); // 1:1 ���� ���̴�.
		return 0;
	}
	if( !you->IsPlayer())
	{
		SendExchangeItemResult( cn, 3 );		// �ΰ��� �Ƴ�~
		return 0;
	}
	if(!connections[you_id].dwAgentConnectionIndex || connections[you_id].state < CONNECT_JOIN) // �� ����� ���� �������� �ƴϴ�.	
	{
		SendExchangeItemResult( cn, 4 );
		return 0;
	}

	if( EXCHANGE_STATE_READY != ch->GetExchangeState() 
	&&	you_id == ch->GetExchangeTargetId())	// �� ����� �ŷ����϶� ó�� �޴��� �÷� ���� �ſ� ���� ó�� �ʿ�
	{		
		return 0;
	}

	//////////// ��ȯ ���� ���� ��ƾ ////////////////
	ch->ClearExchangeInfo();//���� ���� Ŭ���� ��Ű�� �� �����̴ϱ� ������ ����
	you->ClearExchangeInfo();

	switch(iExchangeType)
	{
	case EXCHANGE_TYPE_NORMAL:
		{	//��ȯ�� ó�� ������ ���� ������ ��� Ŭ���� ��Ŵ
			::memset(k_item,0,sizeof(K_ITEM));
			::memset(ch->give,0,sizeof(K_ITEM)*MAX_EXCHANGE_ITEM);
			::memset(ch->recv,0,sizeof(K_ITEM)*MAX_EXCHANGE_ITEM);
			::memset(you->give,0,sizeof(K_ITEM)*MAX_EXCHANGE_ITEM);
			::memset(you->recv,0,sizeof(K_ITEM)*MAX_EXCHANGE_ITEM);
		}break;
	case EXCHANGE_TYPE_MERCHANT_DIRECT:
		{//�˻簡 ����. ���������� ȣ�� �ϱ� �����̴�
			ch->give[0] = *k_item;
			you->recv[0] = *k_item;
		}break;//021121 lsw
	default:
		{
			return 0;
		}break;
	}

	ch->SetExchangeTarget(you_id,you->Name);
	you->SetExchangeTarget(cn,ch->Name);

	ch->SetExchangeType(iExchangeType);//�ŷ�Ÿ�� ����
	you->SetExchangeType(iExchangeType);

	ch->SetExchangeState(EXCHANGE_STATE_EXCHANGE_NOW_SIGN_NO);//���� �ŷ��� ����
	you->SetExchangeState(EXCHANGE_STATE_EXCHANGE_NOW_SIGN_NO);
	
	::SendExchangeItemOk( you_id, cn, k_item ,iExchangeType);// ���� // �޴��� ����.
	::SendExchangeItemReq( you_id, cn, k_item ,iExchangeType);
	return 1;
}

void SendExchangeItemOk( int you_id, int cn, const K_ITEM *item , const int iExchangeType)	
{	// ������ ��ȯ �޴��� �����...
	t_packet packet;
	
	packet.h.header.type = CMD_EXCHANGE_ITEM_READY;
	packet.u.kein.exchange_item_start.item = *item;
	packet.u.kein.exchange_item_start.you_id = you_id;
	packet.u.kein.exchange_item_start.iExchangeType = iExchangeType;
	packet.h.header.size = sizeof( k_exchange_item_start );

	::QueuePacket(connections, cn, &packet, 1);
}

void SendExchangeItemReq( int you_id, int cn, const K_ITEM *item , const int iExchangeType)	
{	// cn�� �ʿ��� �������� ��ȯ���ڰ� ��û�ߴ�.
	t_packet packet;

	packet.h.header.type = CMD_EXCHANGE_ITEM_REQ;
	packet.u.kein.exchange_item_start.item = *item;
	packet.u.kein.exchange_item_start.you_id = cn;
	packet.u.kein.exchange_item_start.iExchangeType = iExchangeType;
	packet.h.header.size = sizeof( k_exchange_item_start );

	::QueuePacket(connections, you_id, &packet, 1);
}

void SendExchangeItemResult( int cn, int result )
{
	t_packet packet;
	packet.h.header.type = CMD_EXCHANGE_ITEM_RESULT;
	packet.u.kein.try_item_trade.result = (char) result;
	packet.h.header.size = sizeof(k_server_money);

	::QueuePacket(connections, cn, &packet, 1);
}

void RecvExchangeStateSign( const short int you, const short int me , const bool bIsOkSign)
{//��ȯ OK ������ �Ծ�
	CHARLIST *me_ch = ::CheckServerId(me);
	CHARLIST *you_ch = ::CheckServerId(you);
	if (!IsActiveTrade(me_ch, you_ch, EXCHANGE_STATE_READY)) return;

	if( !me_ch || !you_ch )			 //  �׷� �� ����~
	{
		::SendExchangeCancel( you, me );
		return;
	}

	if( EXCHANGE_STATE_READY != me_ch->GetExchangeState() 
	&&	EXCHANGE_STATE_READY != you_ch->GetExchangeState() 
	&& 	me_ch->GetExchangeTargetId() == you 
	&& 	you_ch->GetExchangeTargetId() == me )
	{
		int iSendPacketType = 0;
		if(bIsOkSign)
		{
			iSendPacketType = CMD_EXCHANGE_OK_SIGN;
			me_ch->SetExchangeState(EXCHANGE_STATE_EXCHANGE_NOW_SIGN_YES);//Change SignState
			if(EXCHANGE_STATE_EXCHANGE_NOW_SIGN_YES == you_ch->GetExchangeState())
			{// 2 Guys All Yes Sign
				RecvExchange( you, me );//ItemExchange
			}
		}
		else
		{
			iSendPacketType = CMD_EXCHANGE_CANCEL_SIGN;
			me_ch->SetExchangeState(EXCHANGE_STATE_EXCHANGE_NOW_SIGN_NO);//Change SignState
		}
	
		t_packet packet;// ���� �ŷ��� �ϰ� �ִ� ���̾� �Ѵ�.
		packet.h.header.type = iSendPacketType;
		packet.h.header.size = sizeof( 0 );
		::QueuePacket(connections, you, &packet, 1);
	}
	else 
	{
		::SendExchangeCancel( you, me );
	}
	return;
}

void ExchangeCancel(CHARLIST *ch)//SendExchangeCancel() �� ��Ʈ �Դϴ�. �������� �ʰ� ���� �ϼ���
{
	if(!ch){return;}
	switch(ch->GetExchangeType())
	{
	case EXCHANGE_TYPE_MERCHANT_DIRECT:	//���ΰ� ���� �ŷ�
		{//��� �ɶ� IS_END_BUYING �� Ǯ�� ����
			if(	AUCTION_DB == ch->give[0].item_pos.type)//�Ǹ� ��ǰ��
			{
				// 030929 HK YGI
				int iIndex = MAKELONG( ch->give[0].item_pos.p1, ch->give[0].item_pos.p2 );
				Auction.SendCMD_MERCHANT_ITEM_BUY_COMFORM_RESULT(
					ch,iIndex,ch->give[0].item_attr,false);
			}
			if(	AUCTION_DB == ch->recv[0].item_pos.type)
			{
				// 030929 HK YGI
				int iIndex = MAKELONG( ch->recv[0].item_pos.p1, ch->recv[0].item_pos.p2 );
				Auction.SendCMD_MERCHANT_ITEM_BUY_COMFORM_RESULT(
					ch,iIndex,ch->recv[0].item_attr,false);
			}

		}break;
	default:
		{
		}break;
	}

	ch->ClearExchangeInfo();

	t_packet packet;
	packet.h.header.type = CMD_EXCHANGE_CANCEL;
	packet.h.header.size = sizeof( 0 );
	::QueuePacket(connections, ch->GetServerID(), &packet, 1);
}

void SendExchangeCancel( short int you, short int me)
{//iExchangeType�� ���ΰ� �ŷ��϶� iIndex���� �ְ�
	CHARLIST *pChMe		= ::CheckServerId(me);
	CHARLIST *pChYou	= ::CheckServerId(you);

	if( pChMe )		// ���̵� 0 �̸� �ȵȴ�.
	{
		::ExchangeCancel(pChMe); 
	}
	if( pChYou )
	{	
		::ExchangeCancel(pChYou);
	}
}

// 010223 YGI
void RecvExchangeEach( K_ITEM *k_item, int you_id, int cn )		// �÷� ���� ������ ���ǵ�
{
	CHARLIST *me_ch	= ::CheckServerId( cn );
	CHARLIST *you_ch = ::CheckServerId( you_id );

    if (!k_item || !IsActiveTrade(me_ch, you_ch, EXCHANGE_STATE_READY)) return;
    if (k_item->item_pos.type != EQUIP && !GetTradeItem(me_ch, k_item->item_pos)) return;

	if( !me_ch || !you_ch )			 //  �׷� �� ����~
	{
		::SendExchangeCancel( you_id, cn );
		return;
	}

	switch(k_item->item_pos.type)
	{
	case EQUIP:
		{
			if( k_item->item_pos.type == EQUIP )
			{
				int a, b, c;
				if( ::SearchInv( me_ch->inv, a, b, c ) )
				{
					::SetItemPos( INV, a, b, c, &k_item->item_pos );
				}
				else return;
			}
		}break;
	case GROUND:
	case BOX:
	case OTHER_CH:
	case MAKE:
	case BANK:
		{
			return;
		}break;
	default:
		{
		}break;
	}

	const ItemAttr *target= ::GetItemByPOS( cn, k_item->item_pos );		// ������ �� �������� ������ �ִ���?

	if( !target
	||	!target->item_no 
	|| ::memcmp( target,&k_item->item_attr,sizeof(ITEMATTR) ) )
	{
		return;
	}

	if (!GetTradeItem(me_ch, k_item->item_pos)) return;
	k_item->item_attr = *target;

	int i;
	for( i=0; i<MAX_EXCHANGE_ITEM ; i++ )
	{
		if( ::EqualPosByPos(&me_ch->give[i].item_pos, &k_item->item_pos) ) 
		{
			return;// ���� ���� �������� �ö� �Դ�.
		}
	}

	if(AUCTION_DB == me_ch->give[0].item_pos.type)
	{
		return;
	}//0���� ���ǰ�� ������ ������ ���ø���
	
	for( i=0; i<MAX_EXCHANGE_ITEM; i++ )
	{
		if( !me_ch->give[i].item_attr.item_no )
		{
			me_ch->give[i] = *k_item;
			you_ch->recv[i] = *k_item;
			break;
		}
	}
	
	if (i == MAX_EXCHANGE_ITEM) return;
    // 报价改变后，双方必须重新确认；复用现有的取消确认通知。
    RecvExchangeStateSign(you_id, cn, false);
    RecvExchangeStateSign(cn, you_id, false);
	t_packet packet;
	packet.h.header.type = CMD_EXCHANGE_ITEM_EACH;
	packet.u.kein.transfer_item.item = *k_item;//030108 lsw
	packet.h.header.size = sizeof( k_transfer_item );
	::QueuePacket(connections, you_id, &packet, 1);
}

void RecvExchange_Sub(CHARLIST *pCaster,CHARLIST *pTarget,const int i)
{
	if (!pCaster || !pTarget || !IsTradeItemIndexValid(i, MAX_EXCHANGE_ITEM)) return;
	const int iCasterCn = pCaster->GetServerID();
	const int iTargetCn = pTarget->GetServerID();
	if( pCaster->give[i].item_attr.item_no )
	{
		ItemAttr *del_item = ::GetItemByPOS( iCasterCn, pCaster->give[i].item_pos );
		
		if(	AUCTION_DB == pCaster->give[0].item_pos.type)//�Ǹ� ��ǰ��
		{//��ȯ ���� ���ǿ� �ִ°� ���� �������� �÷���
			// 030929 HK YGI
			int iIndex = MAKELONG(pCaster->give[0].item_pos.p1, pCaster->give[0].item_pos.p2 );
			Auction.SendCMD_MERCHANT_ITEM_BUY_COMFORM_RESULT(
				pCaster, iIndex,pCaster->give[0].item_attr,true);
		}

		if( !del_item ) return;

		::SendItemEventLog( del_item, iCasterCn, iTargetCn, SILT_PUT_OTHER_CH, 1 ); //YGI acer
		::SendItemEventLog( del_item, iTargetCn, iCasterCn, SILT_GET_OTHER_CH, 1 ); //YGI acer

		int a, b, c;
		int ret = SearchInv( pTarget->inv, a, b, c );
		if( !ret ) 
		{
			::SendItemEventLog( del_item , iTargetCn, iCasterCn, SILT_PUT_GROUND_MAX_INVEN, 1 ); //YGI acer
			::DropItem( pTarget->X, pTarget->Y, del_item );
		}
		else 
		{
			ItemAttr *recv_item = &pTarget->inv[a][b][c];
			*recv_item = *del_item;
			POS pos;
			::SetItemPos( INV, a, b, c, &pos );
			::SendServerEachItem( &pos, recv_item , iTargetCn );
		}
		::memset( del_item, 0, sizeof( ItemAttr ) );
		::SendServerEachItem( &pCaster->give[i].item_pos, del_item, iCasterCn );
	}
}

void RecvExchange( short int you, short int me )
{
	CHARLIST *me_ch = ::CheckServerId( me );
	CHARLIST *you_ch = ::CheckServerId( you );

	if( !me_ch || !you_ch )			 //  �׷� �� ����~
	{
		SendExchangeCancel( you, me );
		return;
	}
	const int iExchangeType = me_ch->GetExchangeType();
	if (!IsConfirmedTrade(me_ch, you_ch, EXCHANGE_STATE_EXCHANGE_NOW_SIGN_YES)) return;

	if(iExchangeType != you_ch->GetExchangeType())
	{//��ȯ Ÿ���� �ٸ���
		return;
	}
	
	switch(iExchangeType)
	{
	case EXCHANGE_TYPE_NORMAL:
		{
			if (!ValidateNormalTrade(me_ch, you_ch) || !ValidateNormalTrade(you_ch, me_ch))
			{
				SendExchangeCancel(you, me);
				return;
			}
			if( EXCHANGE_STATE_READY != me_ch->GetExchangeState() 
			&&	EXCHANGE_STATE_READY != you_ch->GetExchangeState() 
			&& 	me_ch->GetExchangeTargetId() == you 
			&&	you_ch->GetExchangeTargetId() == me )
			{
				for( int i=0; i<MAX_EXCHANGE_ITEM; i++ )
				{
					RecvExchange_Sub(me_ch,you_ch,i);
					RecvExchange_Sub(you_ch,me_ch,i);
				}
				me_ch->ClearExchangeInfo();
				you_ch->ClearExchangeInfo();
			}
		}break;
	case EXCHANGE_TYPE_MERCHANT_DIRECT:
		{
			if( EXCHANGE_STATE_READY != me_ch->GetExchangeState() 
			&&	EXCHANGE_STATE_READY != you_ch->GetExchangeState() 
			&& 	me_ch->GetExchangeTargetId() == you 
			&&	you_ch->GetExchangeTargetId() == me )
			{
				for( int i=0; i<MAX_EXCHANGE_ITEM; i++ )
				{
					RecvExchange_Sub(me_ch,you_ch,i);
					RecvExchange_Sub(you_ch,me_ch,i);
				}
				me_ch->ClearExchangeInfo();
				you_ch->ClearExchangeInfo();
			}
		}break;
	}
	t_packet packet;
	packet.h.header.type = CMD_EXCHANGE_ITEM;
	packet.h.header.size = sizeof( 0 );
	::QueuePacket(connections, me, &packet, 1);
	::QueuePacket(connections, you, &packet, 1);

	me_ch->Money = ::GetMoneyByItem( me_ch );
	you_ch->Money = ::GetMoneyByItem( you_ch );
}

void RecvExchangeItemDel( POS pos, short int you_id, short int me )
{
	CHARLIST *me_ch	= ::CheckServerId(me);
	CHARLIST *you_ch = ::CheckServerId(you_id);
	if (!IsActiveTrade(me_ch, you_ch, EXCHANGE_STATE_READY)) return;

	if( !me_ch || !you_ch )			 //  �׷� �� ����~
	{
		::SendExchangeCancel( you_id, me );
		return;
	}

	RecvExchangeStateSign( you_id, me , false);
	RecvExchangeStateSign( me, you_id , false);
	
	if(AUCTION_DB == me_ch->give[0].item_pos.type)//0���� ���ǰ�� ������ �������� ��� �� �� ����(��� �� �� ����)
	{
		return;
	}

	int i;
	for( i=0; i<MAX_EXCHANGE_ITEM; i++ )
	{
		if( IsEqualPosByPos( me_ch->give[i].item_pos, pos ) )
		{
			::memset( &me_ch->give[i], 0, sizeof( K_ITEM ) );
		}
	}

	
	for( i=0; i<MAX_EXCHANGE_ITEM; i++ )
	{
		if( IsEqualPosByPos( you_ch->recv[i].item_pos, pos ) )
		{
			K_ITEM targetItem = you_ch->recv[i];
			::SendExchangeItemDel( you_id, targetItem );
			::memset( &you_ch->recv[i], 0, sizeof( K_ITEM ) );
		}
	}
	
	
}

void SendExchangeItemDel( const short int you, const K_ITEM item )		// ��������� �Ѿ �������� ������ ������..
{
	t_packet packet;
	packet.h.header.type = CMD_EXCHANGE_ITEM_DEL;
	{
		packet.u.kein.transfer_item.item = item;
	}
	packet.h.header.size = sizeof( k_transfer_item );
	QueuePacket(connections, you, &packet, 1);	
}


void RecvReqManToMan( short int target_id, short int cn )
{
	CHARLIST *target= ::CheckServerId(target_id);
	CHARLIST *ch	= ::CheckServerId( cn );
	if(!target || !ch){return;}

	if (target->GetLevel() > ch->GetLevel() + ENABLE_DUEL_LEVEL)
	{	//< CSD-030806
		::SendReqManToManResult(3, cn);
		return;
	}	//> CSD-030806

	if( EXCHANGE_STATE_READY != target->GetExchangeState() 
	||	EXCHANGE_STATE_READY != ch->GetExchangeState() ) // ���� �ŷ����̴�
	{
		::SendReqManToManResult(4, cn);
		return;
	}

	if( target->fight_flag || ch->fight_flag )
	{
		::SendReqManToManResult(4, cn);
		return;
	}
	::SendReqManToManResult(1, cn) ;		// ��û ����	//�������� ���� �ʽ��ϴ�.//021030 lsw

	// ���� �ý��� �÷��� ����
	target->fight_flag	= 2;		// ���� �޴��� ���ִ� ���� �÷��� set
	ch->fight_flag		= 2;		// ���� �޴��� ���ִ� ���� �÷��� set

	t_packet packet;				// 1:1 ��û
	packet.h.header.type = CMD_REQ_MAN_TO_MAN;
	packet.u.kein.req_man_to_man.id = cn;
	packet.h.header.size = sizeof( k_req_man_to_man );
	::QueuePacket(connections, target_id, &packet, 1);
}

////////////////////////////////////////////////////////
void SendReqManToManResult( char ret, short int cn )
{
	t_packet packet;				// 1:1 ��û
	packet.h.header.type = CMD_REQ_MAN_TO_MAN_RESULT;
	packet.u.kein.default_char= ret;
	packet.h.header.size = sizeof( char );
	QueuePacket(connections, cn, &packet, 1);
}

void RecvManToManConsent( short int target_id, short int cn )
{
	CHARLIST *ch = CheckServerId( cn );
	CHARLIST *target = CheckServerId( target_id );
	if( !ch || !target ) return;

	t_packet packet;				// 1:1 ����
	packet.h.header.type = CMD_REQ_MAN_TO_MAN_CONSENT;
	packet.h.header.size = sizeof( short int );
	packet.u.kein.default_short_int = target_id;
	QueuePacket(connections, cn, &packet, 1);

	packet.u.kein.default_short_int = cn;
	QueuePacket(connections, target_id, &packet, 1);
}

void ResultManToMan( short int cn, short int target_id, int flag )		// 1:1 ���� ����� ������ ��� ������ ����~
{
	CHARLIST *me = CheckServerId( cn );
	if( !me || me->fight_flag == 0 )
	{
		return;
	}
	
	CHARLIST *you = CheckServerId( target_id );
	if( !you || you->fight_flag == 0 )
	{
		return;
	}

	switch( flag )//���ʿ��� �������̽��� ������ //030116 lsw
	{
	case 3:					// �����		// ���� �������� ���ڰ� ��������.
		{
			::CallClient( target_id, CMD_REQ_MAN_TO_MAN_REFUSAL );
		}break;
	}

	me->fight_id = 0;
	me->fight_flag = 0;
	you->fight_id = 0;
	you->fight_flag = 0;
}

void RecvCMD_TRADE_MODE_CHANGE(const int iCn, t_packet *p)//030127 lsw //Trade On/Off ����
{
	CHARLIST *pCh = ::CheckServerId(iCn);
	const bool bIsTradeOn = p->u.Hwoa.rang.TradeModeChange.bIsTradeOn;
	pCh->SetTradeMode(bIsTradeOn);

	::QueuePacket(connections, iCn, p, 1);
}
