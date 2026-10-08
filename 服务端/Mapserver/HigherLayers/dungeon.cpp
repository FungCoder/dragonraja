#include "..\stdafx.h"

#include "defaultheader.h"
#include "Citem.h"
#include "scrp_exe.h"
#include "dungeon.h"
#include "ltssupport.h"
#include "skillmgr.h"
#include "AIDefine.h"
#include "UserManager.h"

/////////////////////////////////////////////////
// class CDungeonObject functions
bool CDungeonObject::IsDragonLoad()
{
	for( int i=0; i<m_nBossCount; i++ )
	{
		if( m_pBoss[i].IsDragonLoad() ) return 1;
	}
	return 0;
}

bool CDungeonObject::SetActive( CHARLIST *ch )		// ó�� ��Ƽ�� �� ��
{
	if (IsActive())
	{
		return false;
	}
	
	CDungeonMap* pDungeonMap = (CDungeonMap *)m_pMother;
	ActiveObject();

	if (ch)
	{
		Execute(ch->GetServerID());
	}
	
	// ó�� �ѹ��� �ؾ� �ϴ� �ൿ ó��
	CDungeonObject *pNewDOb;
	switch( m_nExecuteType )
	{
	case OET_NORMAL_TRAP :		// �ٸ� ���� ��Ƽ�� ���� ����
		{
			for( int i=0; i<pDungeonMap->m_nObjectCount; i++ )
			{
				pNewDOb = &pDungeonMap->m_pObject[i];
				if( pNewDOb->m_Index == m_Index+1 )
				{
					if( pNewDOb->m_nNeedType == ONT_NEED_TRAP2 )
						pNewDOb->SetActive( ch );
				}
				
				if( pNewDOb->m_nNeedType == ONT_NEED_TRAP )
				{
					pNewDOb->TryActive( ch );
				}
			}
			break;
		}
	case OET_POTAL_BOSS :
		g_pDungeonMap->InitPotalMember();
		CreateScenarioBoss();
		break;
	case OET_POTAL_DRAGON :		// �巡�� �ε� ����
		{
			if( pDungeonMap->IsDragonRoad() )
			{
				g_pDungeonMap->SetVote();
				CreateScenarioBoss( CScenarioBoss::NT_DRAGON_LOAD );
			}
			else
			{
				CreateScenarioBoss( CScenarioBoss::NT_NORMAL_DRAGON );
			}
			break;
		}
	default :
		return true;
	}
	
	pDungeonMap->SetHistory( ch, this, 0 );
	return true;
}

bool CDungeonObject::TryActive( CHARLIST *ch )
{
	if( IsActive() ) return false;
	bool ret = 0;
	CDungeonObject *pNewDOb;
	switch( m_nNeedType )
	{
	case ONT_NEED_ITEM :		// ������ �Ǻ��� �۵���Ű�� Ʈ���� ���
		{
			if( m_nActiveItemNo ) 
			{
				int a, b, c;
				if( SearchItemByInv( m_nActiveItemNo, ch, a, b, c ) )
				{
					ItemAttr *pItem = &ch->inv[a][b][c];
					POS pos;
					SetItemPos( INV, a, b, c, &pos );
					SendDeleteItem( pItem, &pos, ch );
					SetActive( ch );
					ret = 1;
				}
			}
			break;
		}
	case ONT_NEED_TRAP :
		{
			CDungeonMap* pDungeonMap = (CDungeonMap *)m_pMother;
			ret = 1;
			for( int i=0; i<pDungeonMap->m_nObjectCount; i++ )
			{
				pNewDOb = &pDungeonMap->m_pObject[i];
				if( pNewDOb->m_nExecuteType == OET_NORMAL_TRAP )
				{
					if( !pNewDOb->IsActive() )		// ���� ����
					{
						ret = 0;
						break;
					}
				}
			}
			if( ret ) SetActive( NULL );
			break;
		}
	case ONT_NEED_QUBE :
		{
			CDungeonMap* pDungeonMap = (CDungeonMap *)m_pMother;
			int end = 0;
			int a, b, c;
			while( !end && SearchItemByInv( SIN_TRAP_KEY_7 , ch, a, b, c ) )
			{
				ItemAttr *pItem = &ch->inv[a][b][c];
				POS pos;
				SetItemPos( INV, a, b, c, &pos );
				SendDeleteItem( pItem, &pos, ch );
				if( pDungeonMap->SetTrapKey( 7, ch ) ) end = 1;
			}
			while( !end && SearchItemByInv( SIN_TRAP_KEY_5 , ch, a, b, c ) )
			{
				ItemAttr *pItem = &ch->inv[a][b][c];
				POS pos;
				SetItemPos( INV, a, b, c, &pos );
				SendDeleteItem( pItem, &pos, ch );
				if( pDungeonMap->SetTrapKey( 5, ch ) ) end = 1;
			}
			while( !end && SearchItemByInv( SIN_TRAP_KEY_3 , ch, a, b, c ) )
			{
				ItemAttr *pItem = &ch->inv[a][b][c];
				POS pos;
				SetItemPos( INV, a, b, c, &pos );
				SendDeleteItem( pItem, &pos, ch );
				if( pDungeonMap->SetTrapKey( 3, ch ) ) end = 1;
			}
			break;
		}
	case ONT_NEED_MONSTER :
		{
			if( ch->IsPlayer() ) break;
			if( ch->npc_index == m_nActiveItemNo )
			{
				SetActive( NULL );
				ret = 1;
			}
			break;
		}
	}
	return ret;
}

int CDungeonObject::DeleteObjectImage()
{
	if( m_ObjectId != -1 )//������Ų ������Ʈ�� ������ ���� ��Ų��.
	{
		if( GetAttr2( ItemList[m_ObjectId].attr[IATTR_ATTR], IA2_SCENARIO_POTAL ) )
		{		
			RemoveItemList( m_ObjectId );
		}
		m_ObjectId = -1;
		return 1;
	}
	return 0;
}

void SendDeleteScenarioEventBox( int index )
{
	t_packet packet;
	packet.h.header.type = CMD_DELETE_MAP_TILE_EVENT;
	packet.h.header.size = sizeof( char );
	packet.u.kein.default_char = index;
	g_pUserManager->SendPacket(&packet); // CSD-CN-031213
}

void CDungeonObject::ActiveObject()
{
	m_bActive = true;
	m_dwOpenTime = g_curr_time;
	
	if( m_nActiveObjectImage )
	{
		DWORD attr = IA2_NOTMOVE | IA2_SCENARIO_POTAL | IA2_OPENED;
		if( m_nActiveObjectType )
		{
			attr |= IA2_POISONED;		// Ʈ������ ��´�.
		}
		int new_object_id = CreateMapObject( m_nActiveObjectImage, attr, m_nInputX, m_nInputY );
		DeleteObjectImage();
		m_ObjectId = new_object_id;
		if( m_eTileType1 && !m_eTileType2 )
		{
			::SendDeleteScenarioEventBox( m_Index );
		}
		else		// ���� �Ӽ��� ����� ���ٸ� �� ����� ���ο��� �˸���...
		{
			// 021108 YGI		new tile attr
			if( m_eTileType2 )
			{
				::SendTileAttr( m_Index, m_eTileType2, m_nEventNo2, m_nInputX, m_nInputY, m_nRange2, SEND_ALL );
			}
		}
	}
	else
	{
		if( DeleteObjectImage() )
		{
			// ����� �ִµ� ���� ����� ���ٸ� ���ο��� �� ����� �˷��ش�.
			::SendDeleteScenarioEventBox( m_Index );
		}
	}
}
bool CDungeonObject::IsThisLayerActive()
{
	CDungeonMap* pDungeonMap = (CDungeonMap *)m_pMother;
	if( pDungeonMap && pDungeonMap->IsActive() ) return true;
	
	return false;
}

int CDungeonObject::CreateNormalBoss()
{
	if( IsThisLayerActive() && IsNormalTrap() && !m_pBoss && m_nBoss )
	{
		m_pBoss = new CScenarioBoss[1];
		m_nBossCount = 1;
		int x = m_nInputX;
		int y = m_nInputY;
		if( m_nBossX )
		{
			x = m_nBossX;
			y = m_nBossY;
		}
		int ret = m_pBoss->CreateBoss( CScenarioBoss::NT_NORMAL_BOSS, m_nBoss, x, y );
		if( !ret )
		{
			SAFE_DELETE_ARRAY(m_pBoss);
			m_nBossCount = 0;
			return 0;
		}
		else
		{
			if( !m_pFollower && m_nFollower )
			{
				m_pFollower = new CScenarioBoss[m_nFollowerCount];
				for( int i=0; i<m_nFollowerCount; i++ )
				{
					m_pFollower[i].CreateBoss( CScenarioBoss::NT_FOLLOWER, m_nFollower, x+(rand()%5)-3, y+(rand()%5)-3 );
				}
			}
		}
	}
	return 1;
}

void CDungeonObject::CloseObject( int npc_create_type )
{
	m_bActive = false;
	if( m_nCloseObjectImage )
	{
		int new_id = CreateMapObject( m_nCloseObjectImage, IA2_NOTMOVE | IA2_SCENARIO_POTAL, m_nInputX, m_nInputY );
		DeleteObjectImage();
		m_ObjectId = new_id;
		if( m_eTileType2 && !m_eTileType1 )
		{
			::SendDeleteScenarioEventBox(m_Index);
		}
		else
		{
			// 021108 YGI		new tile attr
			if( m_eTileType1 )		// ���� �Ӽ��� ����ٸ�... �ϴ� ���ο��� ������.
			{
				::SendTileAttr( m_Index, m_eTileType1, m_nEventNo1, m_nInputX, m_nInputY, m_nRange1, SEND_ALL );
			}
		}
		
		if( npc_create_type )
		{
			CreateNormalBoss();
		}
	}
	else
	{
		if( DeleteObjectImage() )
		{
			// ����� �ִµ� ���� ����� ���ٸ� ���ο��� �� ����� �˷��ش�.
			::SendDeleteScenarioEventBox(m_Index);
		}
	}
	//  Ȥ�� �ؾ� ���϶� � ���� �ؾ� �� ��찡 �ִٸ� CheckCloseTime() ����
}

int CDungeonObject::Execute( short int cn )
{
	CHARLIST *ch = CheckServerId( cn );
	if( !ch ) return 0;
	
	
	CDungeonMap* pDungeonMap = (CDungeonMap *)m_pMother;
	
	int ret = 0;
	switch( m_nExecuteType )
	{
	case OET_NORMAL_TRAP : 
		break;
	case OET_POTAL_BOSS :
		DeleteScenarioItem( ch );
		ret = GotoUser( m_szOutputMap, m_nOutputX, m_nOutputY, cn );
		MyLog( 0, "Dungeon Scenario : move user by boss potal [%s]<%s:%d:%d>", ch->Name, m_szOutputMap, m_nOutputX, m_nOutputY );
		g_pDungeonMap->AddPotalMember( cn );
		break;
	case OET_POTAL_OUT_LAYER :
		{
			// ��� �ʿ�
			DeleteScenarioItem( ch );
			
			int going_time = g_curr_time - m_dwOpenTime;
			going_time = m_nTime - going_time;
			short int data = going_time/60;		// ���� �ð� (��)
			::SendScenarioMessage( SEND_ALL, SMT_LAYER_POTAL_MESSAGE, &data, sizeof( short ) );		// ��� �Ŀ� ���� ���� �����ϴ�.
			ret = GotoUser( m_szOutputMap, m_nOutputX, m_nOutputY, cn );
			MyLog( 0, "Dungeon Scenario : move user by layer potal [%s]<%s:%d:%d>", ch->Name, m_szOutputMap, m_nOutputX, m_nOutputY );
			break;
		}
	case OET_POTAL_OUT_DUNGEON :
		// ��� �ʿ�
		DeleteScenarioItem( ch );
		ret = GotoUser( m_szOutputMap, m_nOutputX, m_nOutputY, cn );
		MyLog( 0, "Dungeon Scenario : move user by last layer potal [%s]<%s:%d:%d>", ch->Name, m_szOutputMap, m_nOutputX, m_nOutputY );
		break;
	case OET_POTAL_DRAGON :
		{
			DeleteScenarioItem( ch );
			ret = GotoUser( m_szOutputMap, m_nOutputX, m_nOutputY, cn );
			if( pDungeonMap->IsDragonRoad() )
			{
				::SendScenarioMessage( cn, SMT_DRAGONROAD_MESSAGE1 );		// �巡�� �ε��� ���� �Բ� ���� �޴� ����.
				MyLog( 0, "Dungeon Scenario : move user by Red Dragon [%s]<%s:%d:%d>", ch->Name, m_szOutputMap, m_nOutputX, m_nOutputY );
			}
			else
			{
				MyLog( 0, "Dungeon Scenario : move user By Gold Dragon [%s]<%s:%d:%d>", ch->Name, m_szOutputMap, m_nOutputX, m_nOutputY );
			}
			break;
		}
	}
	return ret;
}

int CDungeonObject::CheckCloseTime()
{
	if( !m_bActive ) 
	{
		if( !m_pBoss )		// ���� ���Ѻ���
		{
			if( m_nResenTime != -1 )
			{
				m_nResenTime --;
				if( m_nResenTime == 0 )
				{
					CreateNormalBoss();		// �߰� ������ ���� ��Ű��..
					m_nResenTime = -1;
				}
			}
		}
		return 1;
	}
	
	int going_time = g_curr_time - m_dwOpenTime;
	
	CDungeonMap* pDungeonMap = (CDungeonMap *)m_pMother;
	if( going_time > m_nTime ) 
	{
		switch( m_nExecuteType )
		{
		case OET_POTAL_BOSS :
			g_pDungeonMap->GetOutDugeon( CDungeonMapMgr::GOT_END_BOSS_POTAL );
			break;
		case OET_NORMAL_TRAP : 
			break;
		case OET_POTAL_OUT_LAYER :
			{
				// ���� ���� active �����ش�.
				char data[4];
				data[0] = m_nLayer+1;
				data[1] = 1;
				WORD port = ::GetOwnPort();
				memcpy( &data[2], &port, 2 );
				
				SendRegistDungeonMgrServer( CDungeonBasic::TYPE_STATUS_REGIST, data, 4 );
				g_pDungeonMap->GetOutDugeon( CDungeonMapMgr::GOT_END_LAYER );		// �� �ð� ������.
				break;
			}
		case OET_POTAL_OUT_DUNGEON :
			break;
		case OET_POTAL_DRAGON :
			g_pDungeonMap->ScenarioFightProc( this );
			break;
		}
		if( m_nNeedType != ONT_NEED_DRAGON  )		// ONT_NEED_DRAGON �� �ð��� ���� close �Ǵ°� �ƴϴ�.
		{
			CloseObject();
			pDungeonMap->SetHistory( NULL, this, 0 );
			return 1;
		}
	}
	return 0;
}

int CDungeonObject::Start() // ��ó�� ����
{
	CloseObject( 0 );
	return 1;
}

CDungeonObject::~CDungeonObject()
{
	if(DeleteObjectImage())
	{
		t_packet packet;
		packet.h.header.type = CMD_DELETE_MAP_TILE_EVENT;
		packet.h.header.size = sizeof( char );
		packet.u.kein.default_char = m_Index;
		g_pUserManager->SendPacket(&packet); // CSD-CN-031213
	}
	SAFE_DELETE_ARRAY(m_pBoss);
	m_nBossCount = 0;
	
	SAFE_DELETE_ARRAY(m_pFollower);	
}

int CDungeonObject::KillBoss( CHARLIST *user, CHARLIST *boss )
{
	// �ó����� ���Ͱ� �׾��ٸ�... ��Ƽ�긦 �������.
	CDungeonMap* pDungeonMap = (CDungeonMap *)m_pMother;
	for( int i=0; i<pDungeonMap->m_nObjectCount; i++ )
	{
		if( pDungeonMap->m_pObject[i].m_nNeedType == ONT_NEED_MONSTER )
		{
			pDungeonMap->m_pObject[i].TryActive( boss );
		}
	}
	
	if (m_pBoss && m_pBoss->m_nNpcIndex == boss->GetServerID())
	{
		// �޽���
		switch( m_pBoss->GetBossType() )
		{
		case m_pBoss->BT_LAYER_BOSS :
			{
				k_invite_colossus_map data;
				data.port = boss->SprNo;
				strcpy( data.name, user?user->Name:"" );
				::SendScenarioMessage(SEND_ALL,  SMT_KILL_BOSS, &data, sizeof( k_invite_colossus_map ) );
				// 030211 YGI
				MyLog( 0, "[Dungeon] Kill LAYER_BOSS, sprno : [%d], killer:[%s]", boss->SprNo, user?user->Name:" " );
				break;
			}
		case m_pBoss->BT_DARAGON :
		case m_pBoss->BT_DARAGON_HEART :
			{
				k_invite_colossus_map data;
				data.port = boss->SprNo;
				strcpy( data.name, user?user->Name:"" );
				::SendScenarioMessage(SEND_ALL, SMT_KILL_BOSS, &data, sizeof( k_invite_colossus_map ) );
				
				// 030211 YGI
				MyLog( 0, "[Dungeon] Kill Dragon, sprno : [%d], killer:[%s]", boss->SprNo, user?user->Name:" " );
				
				if( pDungeonMap->IsDragonRoad() )
				{
					// �巡�� �ε尡 �׾���.
					SendScenarioMessage(SEND_ALL, SMT_DRAGONROAD_MESSAGE6 );
					// �� ���� ��� �巡�� �����̾�� ������...
				}
				
				pDungeonMap->SetDragonRoad();		// �������� �巡�� �ε����� ���� ����..
				break;
			}
		default : 
			{
				// ���� ����
				m_nResenTime = 3;		// 3��
				break;
			}
		}
		
		SAFE_DELETE_ARRAY(m_pBoss);
		m_pBoss = 0;
		m_nBossCount = 0;
		if( m_nBossItem )
		{
			ItemAttr item = ::GenerateItem( m_nBossItem );
			if( !item.item_no ) return 0;
			for( int i=0; i<3; i++ )		// 3�� ���� �ش�.
				DropItem( boss->X + (rand()%100)-50, boss->Y +(rand()%100)-50, &item);	
			return 1;
		}
	}
	return 0;
}

int CDungeonObject::CreateScenarioBoss( int boss_type )
{
	switch( boss_type )
	{
	case CScenarioBoss::NT_DRAGON_LOAD :
	case CScenarioBoss::NT_NORMAL_DRAGON :
		{
			SAFE_DELETE_ARRAY(m_pBoss);
			m_pBoss = new CScenarioBoss[2];
			m_nBossCount = 2;
			int ret =0 ;
#ifdef _KEIN_TEST_
			ret += m_pBoss[0].CreateBoss( CScenarioBoss::NT_DRAGON_HEART, 0, 317, 312 );
			ret += m_pBoss[1].CreateBoss( boss_type, 0, 320, 310 );
#else
			ret += m_pBoss[0].CreateBoss( CScenarioBoss::NT_DRAGON_HEART, 0, 489, 75 );
			ret += m_pBoss[1].CreateBoss( boss_type, 0, 492, 73 );
#endif
			
			if( ret != 2 ) 
			{
				SAFE_DELETE_ARRAY(m_pBoss);
				m_nBossCount = 0;
			}
			else
			{
				char data = 0;
				if( boss_type == CScenarioBoss::NT_DRAGON_LOAD )
				{
					data = 1;
				}
				::SendScenarioMessage(SEND_ALL,  SMT_CREATE_DRAGON, &data, 1 );
			}
			break;
		}
	case CScenarioBoss::NT_NORMAL_BOSS :
		{
			if( !m_nBoss ) return 0;
			SAFE_DELETE_ARRAY(m_pBoss);
			m_pBoss = new CScenarioBoss[1];
			m_nBossCount = 1;
			int ret = m_pBoss->CreateBoss( CScenarioBoss::NT_NORMAL_BOSS, m_nBoss, m_nBossX, m_nBossY );
			if( ret != 1 ) 
			{
				SAFE_DELETE_ARRAY(m_pBoss);
				m_nBossCount = 0;
			}
			else
			{
				POINTS xy;
				xy.x = m_nBossX;
				xy.y = m_nBossY;
				::SendScenarioMessage(SEND_ALL,  SMT_CREATE_BOSS, (char *)&xy, sizeof( POINTS ) );
				
				if( !m_pFollower && m_nFollower )
				{
					m_pFollower = new CScenarioBoss[m_nFollowerCount];
					for( int i=0; i<m_nFollowerCount; i++ )
					{
						m_pFollower[i].CreateBoss( CScenarioBoss::NT_FOLLOWER, m_nFollower, m_nBossX+(rand()%5)-3, m_nBossY+(rand()%5)-3 );
					}
				}
			}
			break;
		}
	}
	return 1;
}

int CDungeonObject::DeletBoss()
{
	if( !m_pBoss ) return 0;
	int ret = 0;
	for( int i=0; i<m_nBossCount; i++ )
	{
		ret += m_pBoss[i].DeleteBoss();
	}
	return ( m_nBossCount == ret )?1:0;
}

void CDungeonObject::SendObjectTileAttr( short int cn )
{
	if( IsActive() )
	{
		if( m_eTileType2 )
		{
			::SendTileAttr( m_Index, m_eTileType2, m_nEventNo2, m_nInputX, m_nInputY, m_nRange2, cn );
		}
	}
	else
	{
		if( m_eTileType1 )
		{
			::SendTileAttr( m_Index, m_eTileType1, m_nEventNo1, m_nInputX, m_nInputY, m_nRange1, cn );
		}
	}
}


///////////////////////////////////////////////////////////////////////////

// �ʿ��� ������ �ƴѰ� ������ų��....
int CreateMapObject( int object_no, DWORD attr, int x, int y )
{
	ItemAttr object;
	object.item_no = 25000+object_no;
	object.attr[0] =  1;
	object.attr[1] =  1;
	if( object_no == 1 ) attr |= IA2_HIDE;		// �Ⱥ����ش�.
	object.attr[IATTR_ATTR] = attr;	
	object.attr[3] =  1;
	object.attr[4] =  1;
	object.attr[5] =  1;
	
	int id = AddItemList( object.item_no, object.attr, 0, x*TILE_SIZE+TILE_SIZE/2, y*TILE_SIZE+TILE_SIZE/2, 0, 0 );
	return id;
}

// ���߿� ���ﶧ�� ����ؼ� index�� ���� �ش�. 
void SendTileAttr( int index, int type, int number, int x, int y, int range, short int cn )
{
	t_packet packet;
	packet.h.header.type = CMD_SETTING_MAP_TILE_EVENT;
	packet.h.header.size = sizeof( k_setting_map_tile );
	packet.u.kein.setting_map_tile.index = index;
	packet.u.kein.setting_map_tile.type = type;
	packet.u.kein.setting_map_tile.number = number;
	packet.u.kein.setting_map_tile.x = x;
	packet.u.kein.setting_map_tile.y = y;
	packet.u.kein.setting_map_tile.range = range;
	if( cn == SEND_ALL )
	{
		g_pUserManager->SendPacket(&packet); // CSD-CN-031213
	}
	else
	{
		QueuePacket( connections, cn, &packet, 1 );
	}
}


////////////////////////////////////////////////////////////////
// class CDungeonMap functions
int CDungeonMap::NewJoin( short int cn )		// ��ó�� �� �ʿ� ���� ������� �����ٲ�..
{
	for( int i=0; i<m_nObjectCount; i++ )
	{
		m_pObject[i].SendObjectTileAttr( cn );		// Ÿ�� �Ӽ��� �����ش�.
	}
	return 1;
}

int CDungeonMap::SetActive()
{
	static int dungeon_key[] = { 21, 23, 25, 27, 29, 31, 33, 35, 37, 39, 41, 43, 45, 47, 49 };
	static int dungeon_key_count = sizeof( dungeon_key )/sizeof( int );
	if( m_nDungeonType == DT_PASSWORD )
	{
		int param = rand();
		m_nTrapCount = dungeon_key[param%dungeon_key_count];
		ResetKey();
	}
	
	m_bActive = true;
	
	for( int i=0; i<m_nObjectCount; i++ )
	{
		m_pObject[i].CloseObject(1);		// ���͸� ������Ų��.
	}
	
	char layer = m_nLayer;
	::SendScenarioMessage(SEND_ALL,  SMT_START_LAYER, &layer, 1 );
	
	return 1;
}
int CDungeonMap::SetClose()
{
	for( int i=0; i<m_nObjectCount; i++ )
	{
		m_pObject[i].CloseObject( 0 );
	}
	
	m_bActive = false;
	return 1;
}

// �޽��� ���
int CDungeonMap::SetHistory( CHARLIST *ch, CDungeonObject *pDO, int type )
{
	CDungeonHistory *pData;
	pData = new CDungeonHistory;
	
	int layer = pDO->m_nLayer;
	int msg_type;
	
	switch( pDO->m_nExecuteType )
	{
	case pDO->OET_NORMAL_TRAP		: msg_type = CDungeonHistory::NORMAL_TRAP_OPEN  ; break;
	case pDO->OET_POTAL_BOSS		: msg_type = CDungeonHistory::MIDDLE_BOSS_OPEN  ; break;
	case pDO->OET_POTAL_OUT_LAYER	: msg_type = CDungeonHistory::LAYER_POTAL_OPEN  ; break;
	case pDO->OET_POTAL_OUT_DUNGEON : msg_type = CDungeonHistory::DUNGEON_POTAL_OPEN; break;
	}
	
	msg_type+= pDO->IsActive()? 0:1;
	pData->SetData( msg_type, layer, ch?ch->Name:NULL );
	
	if( type )	// ��ü ������ ������ �޽���
	{
		::SendDungeonHistory( pData );
	}
	else		// �ڱ� �ʸ� �����ִ� �޽���
	{
		CDungeonMap *pDungeonMap = g_pDungeonMap->GetDungeonMapByLayer( layer );
		if( pDungeonMap ) pDungeonMap->m_History.AddNodeLimit( pData, 15 );
		// 030211 YGI
		MyLog( 0, "[Scenario message] Local, type:%d, layer:%d, name:[%s]", msg_type, layer, ch?ch->Name:" " );
	}
	return 1;
}

// ������Ʈ�� ���õ� �̺�Ʈ Ÿ���� ����� ���
bool CDungeonMap::CheckObject( int cn, int event_no )
{
	CHARLIST *ch = CheckServerId( cn );
	if( !ch ) return false;
	
	if( !event_no ) return false;
	
	CDungeonObject *pDOb;
	int call_menu = 0;
	
	int result = 0;
	for( int i=0; i<m_nObjectCount; i++ )
	{
		if( m_pObject[i].IsActive() )
		{
			if( m_pObject[i].m_nEventNo2 == event_no )
			{
				
				m_pObject[i].Execute( cn );;
				pDOb = &m_pObject[i];
				if( m_pObject[i].m_nExecuteType == pDOb->OET_NORMAL_TRAP ) { call_menu = 1; }
				result = 1;
			}
		}
		else
		{
			if( m_pObject[i].m_nEventNo1 == event_no )
			{
				m_pObject[i].TryActive( ch );
				if( m_pObject[i].m_nExecuteType == pDOb->OET_NORMAL_TRAP ) { call_menu = 1; }
				result = 1;
			}
		}
	}
	
	if( call_menu )
	{
		int menunum = 0;
		if( m_nDungeonType == DT_PASSWORD )
		{
			menunum = 137;		// ť�� ����
		}
		else
		{
			menunum = 136;		// trap info
		}
		::SendCallSmallMenu( cn, 0, menunum );
	}
	return result?true:false;
}

CDungeonObject *CDungeonMap::GetObjectByEventNo( int event_no )
{
	if( !event_no ) return NULL;
	for( int i=0; i<m_nObjectCount; i++ )
	{
		if( m_pObject[i].IsActive() )
		{
			if( m_pObject[i].m_nEventNo2 == event_no )
			{
				return &m_pObject[i];
			}
		}
		else
		{
			if( m_pObject[i].m_nEventNo1 == event_no )
			{
				return &m_pObject[i];
			}
		}
	}
	return NULL;
}

// ������Ʈ �����ٶ� event�� �����ش�.
int CDungeonMap::SendEventWhenAddItem( int item_index, short int cn )
{
	for( int i=0; i<m_nObjectCount; i++ )
	{
		if( m_pObject[i].m_ObjectId == -1 ) continue;
		if( m_pObject[i].m_ObjectId == item_index )
		{
			m_pObject[i].SendObjectTileAttr( cn );
		}
	}
	return 1;
}

int CDungeonMap::SetInt( int *pTemp, int max )
{
	int *pData = &m_nLayer;
	
	for( int i=0; i<max; i++ )
	{
		pData[i] = pTemp[i];
	}
	return 1;
}

int CDungeonMap::Load( int layer )
{
	int count=0;
	char condition[512];
	sprintf( condition, "map_name = '%s' AND layer = %d ", MapName, layer );
	::GetRowLineOfSQL( "scenario_trap", "id", &count, condition );
	if( count <= 0 ) return 0;
	
	SetDungeonObject( count );
	LoadDungeonTrap( m_pObject, layer );
	return 1;
}

void CDungeonMap::Proc()
{
	if( m_bActive )
	{
		m_nMessageCount --;
		if( m_nMessageCount <= 0 )
		{
			m_nMessageCount = MESSAGE_COUNT;
			if( m_nDungeonType == DT_PASSWORD )
			{
				// ��ȣ�� ������ �ش�.
				char data = m_nTrapCount;
				if( m_nMessageOption )
					::SendScenarioMessage(SEND_ALL,  3, &data, 1 );
			}
			else
			{
				//char data;
				// m_nLayer
				//SendScenarioMessage(SEND_ALL,  6, &data, 1 );
			}
		}
	}
	
	for( int i=0; i<m_nObjectCount; i++ )
	{
		m_pObject[i].CheckCloseTime();
	}
}
int CDungeonMap::GetTrapCount( int &count, int &active_count , int &active_time ) 
{
	count = 0;
	active_count = 0;
	active_time  = 99999;
	for( int i=0; i<m_nObjectCount; i++ )
	{
		if( m_pObject[i].m_nExecuteType == CDungeonObject::OET_NORMAL_TRAP ) 
		{
			count++;
			if( m_pObject[i].m_bActive )
			{
				active_count++;
				int going_time = g_curr_time - m_pObject[i].m_dwOpenTime;
				int wait_time = (m_pObject[i].m_nTime - going_time)/60.0 + 0.5;
				if( active_time > wait_time ) active_time = wait_time;
			}
		}
	}
	if( active_time == 99999 ) active_time = 0;
	return 1;
}

int CDungeonMap::SetTrapKey( int count, CHARLIST *ch )
{
	int total =0;
	if( m_nDungeonType != DT_PASSWORD ) return 0;
	int i;
	for( i=0; i<7; i++ )
	{
		if( !m_pKey[i] ) break;
		total += m_pKey[i];
	}
	if( i == 7 ) return 0;
	
	m_pKey[i] = count;
	total += count;
	if( i == 6 )
	{
		if( m_nTrapCount == total )
		{
			// ��Ƽ�� ��Ų��.
			// ���տ� �����߽��ϴ�.
			::SendScenarioMessage(SEND_ALL,  4, ch->Name, strlen(ch->Name)+1 );
			for( int a=0; a<m_nObjectCount; a++ )
			{
				if( m_pObject[a].m_nNeedType == CDungeonObject::ONT_NEED_QUBE 
					|| m_pObject[a].m_nNeedType == CDungeonObject::ONT_NEED_QUBE2 )
				{
					m_pObject[a].SetActive( ch );
				}
			}
			m_nMessageOption = 0;
			return 1;
		}
		else
		{
			// ���տ� Ʋ�ȱ� ������ ó������ �ٽ� �����մϴ�.
			::SendScenarioMessage(SEND_ALL,  5, ch->Name, strlen(ch->Name)+1 );
			ResetKey();
			return 0;
		}
	}
	return 0;
}

int CDungeonMap::KillBoss( CHARLIST *user, CHARLIST *boss )
{
	for( int i=0; i<m_nObjectCount; i++ )
	{
		m_pObject[i].KillBoss( user, boss );
	}
	return 1;
}

void CDungeonMap::OpenDragonRoadObject()
{
	for( int i=0; i<m_nObjectCount; i++ )
	{
		if( m_pObject[i].m_nNeedType == CDungeonObject::ONT_NEED_DRAGON )
		{
			m_pObject[i].SetActive( NULL );
		}
	}
}
void CDungeonMap::CloseDragonRoadObject()
{
	for( int i=0; i<m_nObjectCount; i++ )
	{
		if( m_pObject[i].m_nNeedType == CDungeonObject::ONT_NEED_DRAGON )
		{
			m_pObject[i].CloseObject();
		}
	}
}

int CDungeonMap::DeleteDragon()		// �巡���� ã�� ���δ�.
{
	for( int i=0; i<m_nObjectCount; i++ )
	{
		if( m_pObject[i].m_pBoss && m_pObject[i].IsDragonLoad() )
		{
			m_pObject[i].DeletBoss();
			return 1;
		}
	}
	return 0;
}


///////////////////////////////////////////////
void CDungeonMapMgr::GetOutDugeon( int type )
{
	// ox_select	// ������ �̿��Ѵ�.
	static char temp[DRAGON_MAX_CONNECTIONS_];
	ZeroMemory( temp, DRAGON_MAX_CONNECTIONS_ );
	
	switch( type )
	{
	case GOT_END_BOSS_POTAL :
		{
			if( !pPotalList ) return;
			pPotalList->SetFind();
			while( pPotalList->FindNext() )
			{
				temp[pPotalList->GetFind()] = 1;
			}
			DeletePotalMember();
			break;
		}
		
	}
	
	// 030211 YGI -------------
	static char why_out[5][40]	= // �� �ð����
	{
		" ",
			"END_BOSS_POTAL",
			"END_LAYER",
			"END_SCENARIO",
			"END_VOTE"
	};
	// ------------------------
	
	
	CHARLIST *ch;
	for( int i = DRAGON_CONNECTIONS_START; i < DRAGON_MAX_CONNECTIONS; i++ )
	{
		ch = CheckServerId( i );
		if( !ch ) continue;
		if( temp[i] ) continue;
		::GotoUser( GetDungeonMap()->m_szOutMap, GetDungeonMap()->m_nOutX, GetDungeonMap()->m_nOutY, i );
		MyLog( 0, "Dungeon Scenario : move user by '%s' [%s]<%s:%d:%d>", why_out[type], ch->Name, GetDungeonMap()->m_szOutMap, GetDungeonMap()->m_nOutX, GetDungeonMap()->m_nOutY );
	}
}

int CDungeonMapMgr::DropDragonItem()
{
	if( !pVoteFalse && !pVoteTrue ) return 0;
	SAFE_DELETE(g_pDragonItem);
	g_pDragonItem = new CScenarioDragonItem;
	g_pDragonItem->DropDragonItem( pVoteTrue, pVoteFalse );
	return 1;
}

int CDungeonMapMgr::EndUserFightByDragon()
{
	// count1, count2�� ������ ���������� �ð��� �ٵǼ� ����, ������ ������ ��
	// ���� �����ش�.
	
	if( !pVoteFalse || !pVoteTrue ) return 0;
	
	int count1 = pVoteFalse->GetCount();
	int count2 = pVoteTrue->GetCount();
	CDungeonMap* pDungeonMap = GetActiveDungeonMap();
	
	pDungeonMap->OpenDragonRoadObject();		// ���� ���� �ִ� ������Ʈ�� ���ش�.
	
	POINTS data;
	data.x = count2;
	data.y = count1;
	if( !count1 && count2 )		// ������ �̱���
	{
		pDungeonMap->DeleteDragon();
		::SendScenarioMessage(SEND_ALL,  SMT_DRAGONROAD_MESSAGE2 , &data, sizeof( POINTS ));
	}
	else if( count1 && count2 )	// �ð� ���� �� ���� ���
	{
		SendScenarioMessage(SEND_ALL,  SMT_DRAGONROAD_MESSAGE5 );		// �̳�� �� ���̰ڴ�.
	}
	else if( count1 && !count2 )	// �ݴ밡 �̱���
	{
		::SendScenarioMessage(SEND_ALL,  SMT_DRAGONROAD_MESSAGE3 , &data, sizeof( POINTS ));		
	}
	return 1;
}
int CDungeonMapMgr::ScenarioFightProc( CDungeonObject *pDO )
{
	CDungeonMap* pDungeonMap = GetActiveDungeonMap();
	if( !pVoteTrue ) // �׳� �巡�� �϶�..
	{
		pDungeonMap->OpenDragonRoadObject(); 
		return 0;
	}
	
	int count1 = pVoteFalse->GetCount();
	int count2 = pVoteTrue->GetCount();
	
	POINTS data;
	data.x = count2;
	data.y = count1;
	if( !count1 && count2 )		// ������ �ִ� ��� 
	{
		pDungeonMap->OpenDragonRoadObject();		// ���� ���� �ִ� ������Ʈ�� ���ش�.
		::SendScenarioMessage(SEND_ALL,  SMT_DRAGONROAD_MESSAGE2 , &data, sizeof( POINTS ));
		// �������� �Ѹ��� �巡�� �ε带 ���ش�.
		pDO->DeletBoss();
	}
	else if( count1 && count2 )	// ������ ������� 
	{	//< CSD-030225
		POINT MovePoint[3];
		for( int i=0; i<3; i++ )
		{
			MovePoint[i].x = pDungeonMap->m_nOutX;
			MovePoint[i].y = pDungeonMap->m_nOutY;
		}
		int *team1;
		int *team2;
		
		team1= new int[count1];
		team2= new int[count2];
		
		TKeinList<int> *target = pVoteFalse;
		int ct = 0;
		target->SetFind();
		while( target->FindNext() )
		{
			team1[ct++] = target->GetFind();
		}
		
		target = pVoteTrue;
		ct = 0;
		target->SetFind();
		while( target->FindNext() )
		{
			team2[ct++] = target->GetFind();
		}
		
		g_pDragonLordWar.ActiveWar(20*60,ScenarioDragonEndWar,30,ScenarioDragonMsgFunc,
			MovePoint,count1,team1,count2,team2);
		SAFE_DELETE_ARRAY(team1);
		SAFE_DELETE_ARRAY(team2);
		
		::SendScenarioMessage(SEND_ALL,  SMT_DRAGONROAD_MESSAGE4, &data, sizeof( POINTS ));
	}	//> CSD-030225
	else if( count1 && !count2 )	// ��� �ݴ��� ���
	{
		// �巡�� �ε��ϰ� �ο��.
		pDungeonMap->OpenDragonRoadObject();		// ���� ���� �ִ� ������Ʈ�� ���ش�.
		::SendScenarioMessage(SEND_ALL,  SMT_DRAGONROAD_MESSAGE3 , &data, sizeof( POINTS ));
	}
	else
	{
		// �巡�� �ε带 ���ش�.
		pDO->DeletBoss();
	}
	return 1;
}

///////////////////////////////////////////////
int CDungeonHistory::SetData( int type, int layer, char *name, const char *mapname )
{
	m_nType = type;
	
	struct tm *now;
	time_t nowTime;
	
	time( &nowTime );
	now = localtime(&nowTime);
	
	m_nMonth = now->tm_mon + 1;
	m_nDay = now->tm_mday;
	m_nHour = now->tm_hour;
	m_nMin= now->tm_min;
	m_nLayer = layer;
	strcpy( m_szName, name?name:"" );
	
	strcpy( m_szDungeon, mapname?mapname:MapName );
	return 1;
}

int CDungeonMgr::Proc()
{
	for( int i=0; i<m_nCount; i++ )
	{
		CDungeonBasic* pTemp = GetDungeon(i);
		if(pTemp)
		{
			pTemp->Proc();
		}
	}
	return 1;
}

int CDungeonBasic::SendScenarioStatus()
{
	t_packet packet;
	packet.h.header.type = CMD_SCENARIO_COMMAND;
	packet.u.data[0] = TYPE_STATUS;
	memcpy( &packet.u.data[1], m_pActive, sizeof( int ) );
	packet.h.header.size = 5;
	
	if( CheckMapPort( m_nDungeonMapPort1 ) )
	{
		RecvScenarioCommand( &packet );
	}
	else
	{
		::SendPacket2Map( &packet, m_nDungeonMapPort1 );
	}
	
	if( CheckMapPort( m_nDungeonMapPort2 ) )
	{
		RecvScenarioCommand( &packet );
	}
	else
	{
		::SendPacket2Map( &packet, m_nDungeonMapPort2 );
	}
	return 1;
}

int CDungeonBasic::SaveDungeonStatus( )
{
	//return 1;
	char query[256];
	if( m_nActive )
	{
		if(LocalMgr.IsFreeBeta())//030102 lsw
		{
			sprintf( query, "Update scenario_dungeon set start_time = wait_time - 2 where id = %d", m_nId );
		}
		else
		{
			sprintf( query, "Update scenario_dungeon set start_time = wait_time - 60 where id = %d", m_nId );
		}
	}
	else
	{
		sprintf( query, "Update scenario_dungeon set start_time = %d where id = %d", m_nStartTime, m_nId );
	}
	::Querry_SQL( query );
	return 1;
}

int CDungeonBasic::Proc()
{	//< CSD-030324
	if( m_nActive )
	{
		m_nStartTime++;
		
		if (IsCloseDungeon(m_nIndex))
		{
			// ��� ����
			CDungeonHistory data;
			data.SetData( data.DUNGEON_CLOSE, 1, NULL, ::GetMapName2(m_nDungeonMapPort1) );
			::SendDungeonHistory(&data);
			// close ��Ų��. // ��� �ñ�
			m_nStartTime = 0;
			SetActiveFlag(0);
			
			t_packet packet;
			packet.h.header.type = CMD_SCENARIO_MESSAGE_MAP;
			packet.h.header.size = sizeof( k_scenario_message );
			packet.u.kein.scenario_message.type = 1;
			packet.u.kein.scenario_message.port = m_nDungeonMapPort1;
			::SendPacket2Maps( &packet );
			RecvScenarioMessageMap( &packet, -1 );
		}
	}
	else
	{
		m_nStartTime++;
	
		for (int i = 0; i < GetStartUpInfoCount(); ++i)
		{
			if (IsOpenDungeon(i))
			{	// open ��Ų��.
				CDungeonHistory data;
				data.SetData(data.DUNGEON_OPEN, 1, NULL, ::GetMapName2(m_nDungeonMapPort1));
				::SendDungeonHistory( &data );
			
				m_nStartTime = 0;
				SetActiveFlag(1, 1, 1);
			
				t_packet packet;
				packet.h.header.type = CMD_SCENARIO_MESSAGE_MAP;
				packet.h.header.size = sizeof( k_scenario_message );
				packet.u.kein.scenario_message.type = 2;
				packet.u.kein.scenario_message.port = m_nDungeonMapPort1;
				::SendPacket2Maps(&packet);
				RecvScenarioMessageMap(&packet, -1);

				m_nIndex = i;
				break;
			}
		}
	}

	return 1;
}	//> CSD-030324

/////////////////////////////////////////////////
CDungeonMapMgr *g_pDungeonMap = NULL;	// �ش� �ʿ� �ִ� ������( 1��, 2�� )�� ���� Ŭ����
CDungeonMgr *g_pDungeonMgr = NULL;		// ��� ����( �ƺ� �̱�,.. )�� ���� Ŭ����
////////////////////////////////////////////////////////
// ������Ʈ�� �����ٶ� Ȯ���ؼ� eventŸ�ϵ� ���� �����ش�.
void CheckDungeonObject( int id, short int cn )
{
	if( g_pDungeonMap ) g_pDungeonMap->SendEventWhenAddItem( id, cn );
}

bool DungeonEventTileProc( int event_no, short int cn )
{
	if( event_no < 20 ) return 0;
	CHARLIST *ch = CheckServerId( cn );
	if( !ch ) return 0;
	
	if( !g_pDungeonMap ) return 0;
	return g_pDungeonMap->CheckObject( cn, event_no );
}

int SendDungeonHistory( CDungeonHistory *pHistory )
{
	if( g_pDungeonMgr )
	{
		CDungeonHistory *pData = new CDungeonHistory;
		*pData = *pHistory;
		g_pDungeonMgr->m_History.AddNodeLimit( pData, 5 );
		
		// 030211 YGI
		MyLog( 0, "[Scenario message] Manage, type:%d, layer:%d, name:[%s]", 
			pHistory->m_nType, pHistory->m_nLayer, pHistory->m_szName?pHistory->m_szName:" " );
	}
	else	// ���� ������ ������.
	{
		t_packet packet;
		packet.h.header.type = CMD_SEND_DUNGEON_HISTORY;
		packet.h.header.size = sizeof( k_send_dungeon_history );
		packet.u.kein.send_dungeon_history.type = pHistory->m_nType;
		packet.u.kein.send_dungeon_history.layer = pHistory->m_nLayer;
		
		strcpy( packet.u.kein.send_dungeon_history.map, pHistory->m_szDungeon );
		// 030211 YGI
		strcpy( packet.u.kein.send_dungeon_history.name, pHistory->m_szName?pHistory->m_szName:" " );
		SendPacket2Map( &packet, MN_SCENARIO_MAP );
	}
	
	return 1;
}

int RecvDungeonHistory( t_packet *p )
{
	int type = p->u.kein.send_dungeon_history.type;
	int layer = p->u.kein.send_dungeon_history.layer ;
	char *name = p->u.kein.send_dungeon_history.name ;
	char *map =  p->u.kein.send_dungeon_history.map ;
	if( g_pDungeonMgr )
	{
		CDungeonHistory *pData = new CDungeonHistory;
		pData->SetData( type, layer, name, map );
		g_pDungeonMgr->m_History.AddNodeLimit( pData, 5 );
		// 030211 YGI
		MyLog( 0, "[Scenario message] Manage, type:%d, layer:%d, name:[%s]", type, layer, name );
	}
	return 1;
}

void RecvGetScenarioInfo( t_packet *p, short int cn )
{
	CHARLIST *ch = CheckServerId( cn );
	if( !ch ) return;
	
	if( g_pDungeonMap )//���� �������̸� �����丮�� �����ش�.
	{
		CDungeonMap *pDMap = g_pDungeonMap->GetActiveDungeonMap();
		if( pDMap )
		{
			t_packet packet;
			packet.h.header.type = CMD_GET_SCENARIO_INFO;
			k_get_scenario_info &scenario_info = packet.u.kein.get_scenario_info;
			scenario_info.count = pDMap->m_History.GetCount();
			if( !scenario_info.count ) return;
			
			for( int i=0; i<=scenario_info.count; i++ )
			{
				CDungeonHistory *pHistory = pDMap->m_History.GetData(i);
				if( !pHistory ) continue;
				scenario_info.data[i].day = pHistory->m_nDay;
				scenario_info.data[i].SHour= pHistory->m_nHour;
				scenario_info.data[i].SMin = pHistory->m_nMin;
				scenario_info.data[i].type = pHistory->m_nType;
				scenario_info.data[i].layer = pHistory->m_nLayer;
				strcpy( scenario_info.data[i].dungeon_map, pHistory->m_szDungeon );
			}
			packet.h.header.size = 1+sizeof( k_get_scenario_info_basic )*(scenario_info.count);
			QueuePacket( connections, cn, &packet, 1 );
		}
		return;
	}
	else
	{//���� �������� �ƴϸ� �����Ŵ��� ������ ��û ��Ŷ�� ����
		t_packet packet;
		packet.h.header.type = CMD_GET_SCENARIO_INFO_MAP;
		packet.h.header.size = sizeof( k_direct_map_req );
		packet.u.kein.direct_map_req.port = GetOwnPort();
		packet.u.kein.direct_map_req.server_id = cn;

		SendPacket2Map( &packet, MN_SCENARIO_MAP );
	}
}

void RecvGetScenarioInfoMap( t_packet *p, int type )//�ٸ��ʿ��� ��û�� �����̶�� �������������� �˷��ָ� �ȴ�.
{
	if( !g_pDungeonMgr ){return;}//�Ŵ����� �ƴϸ� ���� 
	
	const int port = p->u.kein.direct_map_req.port;
	const int server_id = p->u.kein.direct_map_req.server_id;
	
	k_get_scenario_info scenario_info = {0,};
	const int iCount = g_pDungeonMgr->GetCount();
	int i, j;
	for(i=0, j=0 ; iCount > j ; j++)
	{
		if(MAX_START_UP_INFO <= i){break;}
		CDungeonBasic *pTemp  = g_pDungeonMgr->GetDungeon(j);
		if(!pTemp){continue;}

		const int iSUIct = MAX_START_UP_INFO;//pTemp->GetStartUpInfoCount();
		for(int k=0; iSUIct>k;k++)
		{
			if(MAX_START_UP_INFO <= i){break;}
			START_UP_INFO *pSUI= pTemp->GetStartUpInfo(k);
			if(!pSUI){continue;}

			if(pSUI->nApplyDay < g_wday){continue;}//���������� �Ⱥ�����

			k_get_scenario_info_basic *pSUIB = &scenario_info.data[i];
			i++;
			pSUIB->day		= pSUI->nApplyDay;
			pSUIB->SHour	= pSUI->nStartHour;
			pSUIB->SMin		= pSUI->nStartMinute;
			pSUIB->EHour	= pSUI->nEndHour;
			pSUIB->EMin		= pSUI->nEndMinute;
			pSUIB->layer	= j;//���� �ó����� Ÿ��
			pSUIB->type		= CDungeonHistory::DUNGEON_WHEN_OPEN_CLOSE_NOTICE;
			::strcpy( pSUIB->dungeon_map, ::GetMapName2(pTemp->m_nDungeonMapPort1));
			break;//������ �Ѱ��� ������
		}
	}
	scenario_info.count = i;
	int size = sizeof(char) + sizeof( k_get_scenario_info_basic )*i;
	if( !type )//�ٸ��ʿ��� ��û�� ������ ���
	{
		SendDirectClientOtherMap( CMD_GET_SCENARIO_INFO, server_id, port, &scenario_info, size );
	}
	else//�������ο��� �θ� ���
	{
		SendPacketDefault( CMD_GET_SCENARIO_INFO, &scenario_info, size, server_id );
	}
}

void DugeonMgrProc()
{
	static int old_time = 0;
	if( g_curr_time - old_time < 60 ) return;
	
	old_time = g_curr_time;
	if( g_pDungeonMgr ) g_pDungeonMgr->Proc();
	if( g_pDungeonMap ) g_pDungeonMap->Proc();
}

void RecvScenarioCommand( t_packet *p )
{
	int type = p->u.data[0];
	switch( type )
	{
	case CDungeonBasic::TYPE_STATUS :
		{
			if( !g_pDungeonMap ) return;
			g_pDungeonMap->SetSatus( &p->u.data[1] );
			break;
		}
	case CDungeonBasic::TYPE_STATUS_REGIST :
		{
			if( !g_pDungeonMgr ) return;
			int layer = p->u.data[1];
			int active = p->u.data[2];
			WORD port;
			memcpy( &port, &p->u.data[3], sizeof( WORD ) );
			CDungeonBasic* pDungeon = g_pDungeonMgr->GetDungeonByPort(port);
			if( !pDungeon ) return;
			pDungeon->SetActiveFlag( layer, active, 1 );
			break;
		}
	}
}

void SendRegistDungeonMgrServer( int type, void *msg, int size )
{
	t_packet packet;
	packet.h.header.type = CMD_SCENARIO_COMMAND;
	packet.h.header.size = size+1;
	packet.u.data[0] = type;
	memcpy( &packet.u.data[1], msg, size );
	
	if( CheckMapPort( MN_SCENARIO_MAP ) )
		RecvScenarioCommand( &packet );
	else 
		::SendPacket2Map( &packet, MN_SCENARIO_MAP ); 
}

void SendCheckGotoScenarioDungeon( short int cn, char *map_name, int x, int y )
{
	CHARLIST *ch = CheckServerId( cn );
	if( !ch ) return;
	
	t_packet packet;
	packet.h.header.type = CMD_CHECK_GOTO_SCENARIO_DUNGEON;
	
	strcpy( packet.u.kein.check_goto_scenario_dungeon_map.name, ch->Name );
	strcpy( packet.u.kein.check_goto_scenario_dungeon_map.dungeon, map_name );
	packet.u.kein.check_goto_scenario_dungeon_map.cn = cn;
	packet.u.kein.check_goto_scenario_dungeon_map.x = x;
	packet.u.kein.check_goto_scenario_dungeon_map.y = y;
	packet.u.kein.check_goto_scenario_dungeon_map.map_port = GetOwnPort();
	
	packet.h.header.size = sizeof( k_check_goto_scenario_dungeon_map );
	
	
	if( CheckMapPort(MN_SCENARIO_MAP) )
	{
		RecvCheckGotoScenarioDungeon( &packet, -1 );
	}
	else
	{
		SendPacket2Map( &packet, MN_SCENARIO_MAP );
	}
}

void RecvCheckGotoScenarioDungeon( t_packet *p, short int cn )
{
	if( !g_pDungeonMgr ) return;
	
	int map_port = p->u.kein.check_goto_scenario_dungeon_map.map_port;
	char *dungeon_name = p->u.kein.check_goto_scenario_dungeon_map.dungeon;
	
	int port = GetMapPortByName( dungeon_name );
	if( !port ) return;
	if( g_pDungeonMgr->CheckActive( port ) )
	{
		p->h.header.type = CMD_CHECK_GOTO_SCENARIO_DUNGEON_RESULT;
		if( CheckMapPort( map_port ) )
		{
			RecvCheckGotoScenarioDungeonResult( p, -1 );
		}
		else 
		{
			SendPacket2Map( p, map_port );
		}
	}
	else
	{
		// ���н� �޽���
	}
}

void RecvCheckGotoScenarioDungeonResult( t_packet *p, short int cn )
{
	int server_id = p->u.kein.check_goto_scenario_dungeon_map.cn;
	char *dungeon_name = p->u.kein.check_goto_scenario_dungeon_map.dungeon;
	char *name = p->u.kein.check_goto_scenario_dungeon_map.name;
	int x = p->u.kein.check_goto_scenario_dungeon_map.x;
	int y = p->u.kein.check_goto_scenario_dungeon_map.y;
	CHARLIST *ch = CheckServerId( server_id );
	if( !ch ) return;
	
	if( strcmp( ch->Name, name ) !=  0 ) return;		// �ٸ� ���̴�.
	MapMove( server_id, dungeon_name, x, y );
}

void RecvScenarioMessageMap( t_packet *p, short int cn )
{
	p->h.header.type = CMD_SCENARIO_MESSAGE;
	g_pUserManager->SendPacket(p); // CSD-CN-031213
	
	// 030211 YGI
	if( p->u.kein.scenario_message.type == 1 )
	{
		MyLog( 0, "################### [%d]DUNGEON_CLOSE #####################", p->u.kein.scenario_message.port );
	}
	else if( p->u.kein.scenario_message.type == 2 )
	{
		MyLog( 0, "################### [%d]DUNGEON_OPEN #####################", p->u.kein.scenario_message.port );
	}
}

void RecvScenarioDetailInfo( t_packet *p, short int cn )
{
	CHARLIST *ch = CheckServerId( cn ); 
	if( !ch ) return;
	
	//int event_no = p->u.kein.default_char;
	if( !g_pDungeonMap ) return;
	
	int count, active_count, active_time;
	g_pDungeonMap->GetTrapCount( count, active_count, active_time );
	
	t_packet packet;
	if( !count )
	{
		packet.h.header.type = p->h.header.type;
		packet.u.kein.scenario_detail_info.type = 0;
		packet.h.header.size = 1;
		QueuePacket( connections, cn, &packet, 1 );
		return;
	}
	
	packet.h.header.type = p->h.header.type;
	packet.u.kein.scenario_detail_info.type = 1;
	packet.u.kein.scenario_detail_info.total_trap = count;
	packet.u.kein.scenario_detail_info.active_trap = active_count;
	packet.u.kein.scenario_detail_info.time = active_time;
	packet.h.header.size = sizeof( k_scenario_detail_info );
	QueuePacket( connections, cn, &packet, 1 );
	
	packet.h.header.type = CMD_TRAP_INFO_MAP;
	packet.u.kein.direct_map_req.server_id = cn;
	packet.u.kein.direct_map_req.port = GetOwnPort();
	packet.h.header.size = sizeof( k_direct_map_req );
	if( CheckMapPort(MN_SCENARIO_MAP) )
	{
		RecvScenarioDetailInfoMap( &packet, -1 );
	}
	else
	{
		SendPacket2Map( &packet, MN_SCENARIO_MAP );
	}
}

void RecvScenarioDetailInfoMap( t_packet *p, short int cn )
{
	if( !g_pDungeonMgr ) return;
	int server_id = p->u.kein.direct_map_req.server_id;
	int port = p->u.kein.direct_map_req.port;
	CDungeonBasic *pDungeon = g_pDungeonMgr->GetDungeonByPort( port );
	if( !pDungeon ) return;
	if( !pDungeon->m_nActive ) return;
	
	k_scenario_detail_info scenario_detail_info;
	scenario_detail_info.type = 2;
	scenario_detail_info.time = pDungeon->GetRemainTime();//pDungeon->m_nContinueTime - pDungeon->m_nStartTime;	// 031110 YGI
	int size = size = sizeof(char) + sizeof(int);
	if( CheckMapPort(port) )
	{
		t_packet packet;
		packet.h.header.type = CMD_TRAP_INFO;
		packet.h.header.size = size;
		memcpy( packet.u.data, &scenario_detail_info, size );
		QueuePacket( connections, server_id, &packet, 1 ); 
	}
	else
	{
		SendDirectClientOtherMap( CMD_TRAP_INFO, server_id, port, &scenario_detail_info, size );
	}
}

// �ʼ����� ���� ������ ��, �� ���� ���� ��Ƽ�� �Ǿ� �ִ� ��Ȳ�̸�
void ScenarioServerNewStart( WORD port )
{
	if( !g_pDungeonMgr ) {return;}
	
	CDungeonBasic *pDungeon = g_pDungeonMgr->GetDungeonByPort( port );
	if( !pDungeon ) {return;}
	//if( !pDungeon->m_nActive ) return;
	
	int temp[MAX_CDUNGEONBASIC_INT]; // CSD-030723
	LoadDungeonMapEach( port, temp );
	pDungeon->SetActiveFlag( 0 );
	pDungeon->SetInt( temp );
}

void ScenarioServerClear( WORD port )
{
	if( !g_pDungeonMgr ) return;
	
	CDungeonBasic *pDungeon = g_pDungeonMgr->GetDungeonByPort( port );
	if( !pDungeon ) return;
	if( !pDungeon->m_nActive ) return;
	
	pDungeon->m_nStartTime = pDungeon->m_nWaitTime - 60;
	pDungeon->SetActiveFlag( 0 );
	pDungeon->SaveDungeonStatus();
}

void ScenarioServerReset( WORD port )		// ���� ���� ���� ���� �Ǹ� �� ���� ��Ƽ�긦 �ٲ��ش�.
{
	if( !g_pDungeonMgr ) return;
	CDungeonBasic *pDungeon = g_pDungeonMgr->GetDungeonByPort( port );
	if( !pDungeon ) return;
	if( !pDungeon->m_nActive ) return;
	
	pDungeon->m_nStartTime = pDungeon->m_nWaitTime - 2;
	pDungeon->SetActiveFlag( 0 );
}

void RecvScenarioTrapKeyInfo( t_packet *p, short int cn )
{
	CHARLIST *ch = CheckServerId( cn );
	if( !ch ) return;
	
	if( !g_pDungeonMap ) return;
	
	CDungeonMap* pDM = g_pDungeonMap->GetActiveDungeonMap();
	if( !pDM ) return;
	t_packet packet;
	packet.h.header.type = p->h.header.type;
	packet.u.kein.scenario_trap_key.sum = pDM->m_nTrapCount;
	for( int i=0; i<7; i++ )
	{
		packet.u.kein.scenario_trap_key.qube[i] = pDM->m_pKey[i];
	}
	packet.h.header.size = sizeof( k_scenario_trap_key );
	QueuePacket( connections, cn, &packet, 1 );
}

void SendScenarioMessage( short int cn, int type, void *data, int size )
{
	t_packet packet;
	packet.h.header.type = CMD_SCENARIO_MESSAGE;
	packet.u.kein.data[0] = type;		// 
	if( data )
	{
		memcpy( &packet.u.kein.data[1], data, size );
	}
	packet.h.header.size = size+1;
	if( cn == SEND_ALL )
	{
		g_pUserManager->SendPacket(&packet); // CSD-CN-031213
	}
	else
	{
		QueuePacket( connections, cn, &packet, 1 );
	}
}

// �ó����� �� ������ ���� �����
void DeleteScenarioItem( CHARLIST *ch )
{
	//	return ;		// test
	for( int a=0; a<3; a++ ) 
	{	for( int b=0; b<3; b++ ) 
	{	for( int c=0; c<8; c++ )
	{
		ItemAttr *item = &ch->inv[a][b][c];
		if( !item->item_no ) continue;
		CItem *t = ItemUnit( item->item_no );
		if( !t ) continue;
		if( ( t->GetItemKind() == IK_ABYSS_MAZE )
			|| ( t->GetItemKind() == IK_UNDEAD_DUNGEON )
			|| ( t->GetItemKind() == IK_GREAT_MAZE ) )
		{
			POS pos;
			SetItemPos( INV, a, b, c, &pos );
			SendDeleteItem( item, &pos, ch );
		}
	}
	}
	}
	
	ItemAttr *item = &ch->handheld;
	if( item->item_no )
	{
		CItem *t = ItemUnit( item->item_no );
		if( t )
		{
			if( ( t->GetItemKind() == IK_ABYSS_MAZE )
				|| ( t->GetItemKind() == IK_UNDEAD_DUNGEON )
				|| ( t->GetItemKind() == IK_GREAT_MAZE ) )
			{
				POS pos;
				SetItemPos( HAND, &pos );
				SendDeleteItem( item, &pos, ch );
			}
		}
	}
}



//////////////////////////////////////////////////////
// class CScenarioBoss member functions
//021030 YGI
int CScenarioBoss::DeleteBoss()
{
	int ret = 0;
	if( m_nNpcIndex ) 
	{
		CHARLIST *npc = &NPCList[m_nNpcIndex];
		if( npc->npc_index == m_nSprNum )
		{
			if( npc->bAlive == ALIVE_ )
			{
				::DeleteNpc( m_nNpcIndex );
				ret = 1;
			}
			
			if ( g_pDungeonMap )
				if( m_nBossType == BT_DARAGON ) g_pDungeonMap->DropDragonItem();
		}
	}
	Clear();
	return ret;
	
}
int CScenarioBoss::CreateBoss( int npc_type, int npc_num, int x, int y )
{
	const int id = ::GetDeleteAbleNPC();//030211 lsw
	if( id >= 0 )
	{
		::DeleteNpc( id );
	}
	else
	{//���� �� �ִ� ���Ͱ� ���ٸ�
		::MyLog(0,"Critical Error Can't Get Normal Monster(CreateBoss())");
		return 0;
	}
	
	int spr = 0;
	switch( npc_type )
	{
	default : return 0;
	case NT_DRAGON_LOAD :
		{
			m_nSprNum = DRAGON_LORD_INDEX_NO;
			NPC_Create( id, m_nSprNum, x, y, 0, 0, GT_SCENARIO_MONSTER );
			m_nBossType = BT_DARAGON;
			break;
		}
	case NT_NORMAL_DRAGON : 
		{
			m_nSprNum = DRAGON_INDEX_NO;
			NPC_Create( id, m_nSprNum, x, y, 0, 0, GT_SCENARIO_MONSTER );
			m_nBossType = BT_DARAGON;
			break;
		}
	case NT_DRAGON_HEART :
		{
			m_nSprNum = DRAGON_HEART_INDEX_NO;
			NPC_Create( id, m_nSprNum, x, y, 0, 0, GT_SCENARIO_MONSTER );
			m_nBossType = BT_DARAGON_HEART;
			break;
		}
	case NT_NORMAL_BOSS :
		{
			m_nSprNum = npc_num;
			NPC_Create( id, m_nSprNum, x, y, 0, 0, GT_SCENARIO_MONSTER );
			break;
		}
	case NT_FOLLOWER:
		{
			if( TileMap[x][ y].attr_dont )
			{
				for( int i=x-5; i<x+5; i++ )
				{
					for( int j=y-5; j<y+5; y++ )
					{
						if( !TileMap[x][ y].attr_dont ) 
						{
							x=i, y =j;
							break;
						}
					}
				}
			}
			if( TileMap[x][ y].attr_dont ) return 0;
			m_nSprNum = npc_num;
			NPC_Create( id, m_nSprNum, x, y, 0, 0, GT_SCENARIO_MONSTER );
			break;
		}
	}
	m_nNpcType = npc_type;
	m_nNpcIndex = id;
	
	return 1;
}

//////////////////////////////////////////////////////

int KilledScenarioBoss( CHARLIST *user, CHARLIST *npc )		// �ó��󸮿� ���Ͱ� �׾���.
{
	if( !g_pDungeonMap ) return 0;
	return g_pDungeonMap->KilledScenarioBoss( user, npc );
}

void RecvDragonVote( t_packet *p, short int cn )
{
	CHARLIST *ch = CheckServerId( cn );
	if( !ch ) return;
	if( !g_pDungeonMap ) return;
	
	int true_false = p->u.data[0];		// ���� �ݴ�
	if( !g_pDungeonMap->pVoteTrue ) return;
	int *pData;
	pData = new int;
	*pData = cn;
	if( true_false ) g_pDungeonMap->pVoteTrue->AddNode( pData );
	else g_pDungeonMap->pVoteFalse->AddNode( pData );
} 

// ���� ����
void ScenarioDragonEndWar()
{
	g_pDungeonMap->EndUserFightByDragon();
}
void ScenarioDragonMsgFunc()
{
	//MyLog( 0, "message " );
}

CScenarioDragonItem	*g_pDragonItem = NULL;
int CScenarioDragonItem::MakeDragonItem( int total )
{
	SAFE_DELETE_ARRAY(m_pTotalItem);
	
	m_pTotalItem = new ItemAttr[total];
	m_nTotalCount = total;
	
	ItemAttr *pItem;
	for( int i=0; i<m_nTotalCount; i++ )
	{
		pItem = &m_pTotalItem[i];
		SkillMgr.MakeItemByItemControl( pItem, DRAGON_ITEMCONTROL_NUMBER );
	}
	return 1;
}
/////////////////////////////////////////////
// �巡���� ���� �� ȣ��
// ������ ĳ���� ����Ʈ�� �ݴ��� ĳ���� ����Ʈ�� ���ڷ� �޴´�.
//
// ���� ���� �� * 2 + 2�� �� �� ��ŭ �������� �����.
int CScenarioDragonItem::DropDragonItem( TKeinList<int> *pLink1, TKeinList<int> *pLink2 )
{
	int max_ch = 0;
	max_ch += pLink1?pLink1->GetCount():0;
	max_ch += pLink2?pLink2->GetCount():0;
	
	if( !max_ch ) return 0;
	
	int item_count_max =  max_ch*2 + 2;
	MakeDragonItem( item_count_max );
	short int *pServerIDBuf;
	pServerIDBuf = new short int[max_ch];
	
	
	int man_count = 0;		// ��� ī��Ʈ �Ѵ�.( ���̵� ���ۿ� ��� ���� )
	int item_count = 0;		// ������ ������ ���� ī��Ʈ�Ѵ�.
	CHARLIST *ch;
	
	// �ѻ�� �տ� �ϳ����� �� ������ �������� ���� �߸���.
	TKeinList<int> *pTarget;
	for( int i=0; i<2; i++ )
	{
		if( i == 0 )
		{
			pTarget = pLink1;
		}
		else
		{
			pTarget = pLink2;
		}
		pTarget ->SetFind();
		while( pTarget->FindNext() )
		{
			pServerIDBuf[man_count] = pTarget->GetFind();
			ch = CheckServerId( pServerIDBuf[man_count] );
			if( !ch ) continue;
			
			int rand_item_count = rand()%2+1;
			for( int i=0; i<rand_item_count; i++ )
			{
				DropItem( ch->X + (rand()%40) - 40, ch->Y + (rand()%40) - 40, &m_pTotalItem[item_count++] );
			}
			man_count++;
		}
	}
	
	// ���� �������� �������� ����� �̾Ƽ� �Ѹ���.
	for( ; item_count<item_count_max; item_count++ )
	{
		ch = CheckServerId( pServerIDBuf[rand()%man_count] );
		if( !ch ) continue;
		DropItem( ch->X + (rand()%40) - 40, ch->Y + (rand()%40) - 40, &m_pTotalItem[item_count] );
	}
	
	SAFE_DELETE_ARRAY(pServerIDBuf);
	return 1;
}

//DropItem(	ch->X + (rand()%40) - 40, ch->Y + (rand()%40) - 40, &item);

void SendGetScenarioTime(int port, int cn, k_client_scenario_time_info* pPacket)
{	//< CSD-030723
	if( !g_pDungeonMgr ) return ;
	
	CDungeonBasic *pTemp = g_pDungeonMgr->GetDungeon(pPacket->idDungeon);
		
	if (pTemp == NULL)
	{
		return;
	}

	t_packet packet;
	packet.h.header.type = CMD_SCENARIO_TIME_INFO;
	packet.h.header.size = sizeof(k_server_scenario_time_info);

	CDungeonBasic::VECTOR_START_UP& rList = pTemp->GetStartUpList();

	for (CDungeonBasic::ITOR_START_UP i = rList.begin(); i != rList.end(); ++i)
	{	
		packet.u.kein.server_scenario_time_info.idStartUp = i->idIndex;
		packet.u.kein.server_scenario_time_info.idDungeon = pPacket->idDungeon;
		packet.u.kein.server_scenario_time_info.nApplyDay = i->nApplyDay;
		packet.u.kein.server_scenario_time_info.nStartHour = i->nStartHour;
		packet.u.kein.server_scenario_time_info.nStartMinute = i->nStartMinute;
		packet.u.kein.server_scenario_time_info.nEndHour = i->nEndHour;
		packet.u.kein.server_scenario_time_info.nEndMinute = i->nEndMinute;
	
		if( port == GetOwnPort() )
		{
			QueuePacket( connections, cn, &packet, 1 );
		}
		else
		{
			SendDirectClientOtherMap(CMD_SCENARIO_TIME_INFO, cn, port, 
				                     &packet.u.kein.server_scenario_time_info, 
									 sizeof(k_server_scenario_time_info));
		}
	}
}	//> CSD-030723

void RecvGetScenarioTime( t_packet *p, short int cn )
{	//< CSD-030306
	if( cn == CN_FROM_OTHER_MAPSERVER_MSG )
	{
		int port = p->u.kein.client_scenario_time_info.nPort;
		int server_id = p->u.kein.client_scenario_time_info.nServer_Id;
		SendGetScenarioTime(port, server_id, &p->u.kein.client_scenario_time_info);	// �ش���� ���ؼ� �����ش�.
	}
	else
	{
		if( g_pDungeonMgr )
		{
			SendGetScenarioTime(GetOwnPort(), cn, &p->u.kein.client_scenario_time_info); // �ٷ� �����ش�.
		}
		else
		{
			// ���⼭ ��û�ϴ� ������ ������ ��Ƽ� ���� �ش�.
			p->u.kein.client_scenario_time_info.nPort = GetOwnPort();
			p->u.kein.client_scenario_time_info.nServer_Id = cn;

			SendPacket2Map(p, MN_SCENARIO_MAP);
		}
	}
}	//> CSD-030306

void RecvChangeScenarioTime( t_packet *p, short int cn )
{	//< CSD-030306
	if( cn == CN_FROM_OTHER_MAPSERVER_MSG )
	{
		if (!g_pDungeonMgr) 
		{
			return;
		}

		CDungeonBasic* pTemp = g_pDungeonMgr->GetDungeon(p->u.kein.scenario_time_change.idDungeon);

		if (pTemp == NULL)
		{
			return;
		}

		pTemp->SetStartUpInfo(p->u.kein.scenario_time_change.idStartUp,
							  p->u.kein.scenario_time_change.nApplyDay,
							  p->u.kein.scenario_time_change.nStartHour,
							  p->u.kein.scenario_time_change.nStartMinute,
							  p->u.kein.scenario_time_change.nEndHour,
							  p->u.kein.scenario_time_change.nEndMinute);
	}
	else
	{
		if( g_pDungeonMgr )
		{
			RecvChangeScenarioTime( p, CN_FROM_OTHER_MAPSERVER_MSG );
		}
		else
		{
			SendPacket2Map( p, MN_SCENARIO_MAP);
		}
	}
}	//> CSD-030306

bool IsActiveDugeon()
{	//< CSD-030428
	if( !g_pDungeonMap ) return false;
	if( g_pDungeonMap->GetActiveDungeonMap() ) return true;
	
	return false;
}	//> CSD-030428