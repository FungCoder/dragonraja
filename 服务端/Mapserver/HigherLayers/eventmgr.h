#include "dungeon.h"


#ifndef __EVENTMGR_H__
#define __EVENTMGR_H__

/////////////////////////////////////////////////////////////////////////////
#define MAX_SCRIPT_ITEM	15
class CEventScriptItemBag		// npc�� ������ ������ ����, CEventMgr���� ����Ѵ�.
{
public :
	int m_nBagNumber;
	int m_aItemNo[MAX_SCRIPT_ITEM];
	int m_nCount;
public :
	CEventScriptItemBag()
	{
		for( int i=0; i<MAX_SCRIPT_ITEM; i++ )
		{
			m_aItemNo[i] = 0;
		}
		m_nCount = 0;
		m_nBagNumber = 0;
	}
	void Count()
	{
		m_nCount = 0;
		while( m_aItemNo[m_nCount] ) m_nCount++;
	}
	int GetItemNumber()
	{
		if( !m_nCount ) return 0;
		return m_aItemNo[rand()%m_nCount];
	}
};


///////////////////////////////////////////////////////////////////////////////

class CEventTime
{
public :
	int m_nEventNo;
	int m_nEventActive;

	time_t m_timeStart;
	time_t m_timeEnd;



public :
	CEventTime()
	{
		Remove();
	}
	~CEventTime()
	{
		Remove();
	}

	void Remove()
	{
		m_timeStart = 0;
		m_timeEnd = 0;
	}

	int CheckActive();
	int IsEventActive() { return m_nEventActive; }
	int SetTime( TIMESTAMP_STRUCT *pStart, TIMESTAMP_STRUCT *pEnd );
	virtual void RunEvent() { return; }
};

class CEventNpcItem	: public CEventTime		// �������� �߰��� ����߷� �ش�.
{
public :
	int m_nNpcNo;			// �������� ���� ���� ��ȣ
	int m_nPercent;			// �������� ���� �߸� Ȯ��
	int m_nMax;				// �� �̺�Ʈ ������ ( �� �̻� �������� ������ �ȵȴ�.
	int m_nItemNo;
	int m_nItemMutant;
	int m_nItemGrade;

	int m_nAmount;		// ������� ������ ������ ��

public :
	CEventNpcItem()
	{
		m_nNpcNo = 0;
		m_nPercent = 0;
		m_nMax = 0;
		m_nItemNo = 0;
		m_nItemMutant = 0;
		m_nItemGrade = 0;
		m_nAmount = 0;
	}

	int DropNpcItem( CHARLIST *pNpc );
};


class CEventNpcCreate : public CEventTime	// �̺�Ʈ�� npc�� ������Ų��.
{
public :
	int m_nMapX;
	int m_nMapY;
	int m_nNpcNo;
	int m_nNpcScriptNo;
	int m_nNpcMany;
	int m_nRegen;			// ������ �� Ƚ�� --> �ϴ� ���� �ϳ� ���ϳĸ� ���ܵд�.
	//int m_nRegenCount;		// ������� ������ ī��Ʈ
	int m_nRegenTime;

	int m_nFirst;
	bool m_bAlive;			// ��� �ִ��� �׾�����
	int m_nDeadTime;		// ���� ��� �ð�
	int m_nNpcIndex;		// ������ �ε���... �̺�Ʈ�� ������ �����ش�.

public :
	CEventNpcCreate()
	{
		m_nMapX = 0;
		m_nMapY = 0;
		m_nNpcNo = 0;
		m_nNpcScriptNo = 0;
		m_nNpcMany = 0;
		m_nRegen = 0;
		m_nRegenTime = 0;
		m_nDeadTime = 0;
		m_bAlive = 0;
		m_nFirst = 1;		// ��ó������ ����� �ų�?
		m_nNpcIndex = 0;
	}
	~CEventNpcCreate()
	{
		DeleteEventNpc();
	}

	bool IsAlive() { return m_bAlive; }
	int CreateNpc();
	bool IsCreateAble();
	int CheckKillMonster( CHARLIST *pNpc );
	int DeleteEventNpc();
	int CheckClearCreatedNpc();
};


class CEventObject: public CEventTime	// �̺�Ʈ ��
{
public :
	static enum 
	{
		ACTIVE_NUMBER_ = 0x01,
	};

public :
	char m_szObjectTOI[30];
	char m_szObjectB[30];

public :
	CEventObject()
	{
		m_szObjectTOI[0] = 0;
		m_szObjectB[0] = 0;
	}
	virtual void RunEvent();
};


class CEventSound: public CEventTime	// �̺�Ʈ ��
{
public :
	static enum 
	{
		ACTIVE_NUMBER_ = 0x02,
	};

public :
	int m_nSoundNo;
public :
	CEventSound()
	{
		m_nSoundNo = 0;
	}
};

class CEventMessage : public CEventTime
{
public :
	int m_bAll;				// ����
	int m_nNation;			// ���� ����
	int m_nTime;			// �޽��� �ֱ�
	char *m_lpszMessage;	// ���� �޼���
	int m_nMessageSize;
	DWORD m_nLastTime;		// ���������� �޼��� ���� �ð�

public :
	CEventMessage()
	{
		m_lpszMessage = 0;
		Reset();
	}
	~CEventMessage()
	{
		Reset();
	}
	int Reset()
	{
		SAFE_DELETE_ARRAY(m_lpszMessage);
		m_bAll = 0;
		m_nNation = 0;
		m_nTime = 0;
		m_nLastTime = 0;
		m_nMessageSize = 0;
		return 1;
	}
	void SetMessage( char *msg, int size )
	{
		if( !size ) return;
		SAFE_DELETE_ARRAY(m_lpszMessage);
		m_lpszMessage = new char[size+1];
		strcpy( m_lpszMessage, msg );
		m_nMessageSize = size;
	}
	int CheckEvent();
};


class CEventNpcScript : public CEventTime		// ��ũ��Ʈ�� ���� ������ �ޱ�
{
public :
	int m_nScriptNo;
	int m_nNumber;				// �� ������ �Ѱ����� �ƴ���..
	int m_nEventNpcItemBag;		// ������ ������ ���̺� ��ȣ

	int m_nMessageSize;			
	int m_nMessageSize2;

	char *m_lpszMessage;		// NPC�� �ؾ� �ϴ� ��
	char *m_lpszMessage2;		// �������� �� �ٶ� npc�� �ؾ� �� ��

public :
	CEventNpcScript()
	{
		m_lpszMessage = 0;
		m_lpszMessage2 = 0;
		Reset();
	}
	~CEventNpcScript() { Reset(); }
	int Reset()
	{
		SAFE_DELETE_ARRAY(m_lpszMessage);
		SAFE_DELETE_ARRAY(m_lpszMessage2);
		m_nMessageSize = 0;
		m_nMessageSize2 = 0;
		m_nScriptNo = 0;
		m_nNumber = 0;
		m_nEventNpcItemBag = 0;
		return 1;
	}
	void SetMessage( char *msg, int size )
	{
		if( !size ) return;
		SAFE_DELETE_ARRAY(m_lpszMessage);
		m_lpszMessage = new char[size+1];
		strcpy( m_lpszMessage, msg );
		m_nMessageSize = size;
	}
	void SetMessage2( char *msg, int size )
	{
		if( !size ) return;
		SAFE_DELETE_ARRAY(m_lpszMessage2);
		m_lpszMessage2 = new char[size+1];
		strcpy( m_lpszMessage2, msg );
		m_nMessageSize2 = size;
	}
	int CheckEvent( int npc_index, short int cn );		// �ش� ��ũ��Ʈ ��ȣ�� ������ üũ�Ѵ�.	
};

class CEventMoveMap	: public CEventTime		// �̺�Ʈ ������ �̵�
{
public :
	int m_bAll;
	int m_nSex;
	int m_nClass;
	int m_nNation;
	char m_szMap[20];
	int m_nMapX;
	int m_nMapY;
	int m_nMax;
	
public :
	int Reset()
	{
		m_bAll=0;
		m_nSex=0;
		m_nClass=0;
		m_nNation=0;
		m_nMapX=0;
		m_nMapY=0;
		m_nMax=0;
		m_szMap[0] = 0;
		return 1;
	}
	CEventMoveMap()
	{
		Reset();
	}
};

/////////////////////////////////////////////////////////////////
template <class T1> class TEventMgrBasic
{
public :
	T1 *m_pEvent;
	int m_nEventCount;
	TKeinList<int> m_listActive;
	int m_nActive;

public :
	TEventMgrBasic()
	{
		m_pEvent = 0;
		Reset();
	}
	~TEventMgrBasic()
	{
		Reset();
	}
	void Reset()
	{
		SAFE_DELETE_ARRAY(m_pEvent);
		m_pEvent = 0;
		m_nEventCount = 0;
		ClearActive();
	}
	void Set( int count )
	{
		Reset();
		m_pEvent = new T1[count];
		m_nEventCount = count;
	}
	void ClearActive()
	{
		m_nActive = 0;
		m_listActive.clear();
	}
	void CheckActiveEvent()
	{
		ClearActive();
		for( int i=0; i<m_nEventCount; i++ )
		{
			if( m_pEvent[i].CheckActive() )
			{
				m_nActive++;
				m_listActive.AddNode( i );
			}
		}
		if( IsActive() )
		{
			GetActiveEvent()->RunEvent();
		}
	}
	bool IsActive() { return m_nActive?true:false; }
	void Init( int index, T1 *data )
	{
		m_pEvent[index] = *data;
	}
	int GetNpcNo( int index ) { return m_pEvent[index].m_nNpcNo; }
	T1 *GetEventPoint( int index )
	{
		return &m_pEvent[index];
	}
	T1 *GetActiveEvent()		// ��ó�� ��Ƽ�� �� ����Ʈ�� ��� �´�.
	{
		if( !IsActive() ) return 0;
		m_listActive.SetFind();
		m_listActive.FindNext();
		m_listActive.GetFind();
		return GetEventPoint( m_listActive.GetFind() );
	}
};


/////////////////////////////////////////////////////

class CEventMgr
{
public :
	enum
	{
		ET_EVENT_ALL		= 0xFFFFFFFF,
		ET_EVENT_NPC_ITEM	= 0x00000001,
		ET_EVENT_NPC_CREATE = 0x00000002,
		ET_EVENT_OBJECT		= 0x00000004,
		ET_EVENT_SOUND		= 0x00000008,
		ET_EVENT_MESSAGE	= 0x00000010,
	};

public :
	TEventMgrBasic<CEventNpcItem> m_eventNpcItem;
	TEventMgrBasic<CEventNpcCreate> m_eventNpcCreate;
	TEventMgrBasic<CEventObject> m_eventObject;
	TEventMgrBasic<CEventSound> m_eventSound;
	TEventMgrBasic<CEventMessage> m_eventMessage;
	TEventMgrBasic<CEventNpcScript> m_eventNpcScript;
	TEventMgrBasic<CEventMoveMap> m_eventMoveMap;
	
	
	CEventScriptItemBag	*m_pItemBag;
	int m_nItemBagCount;
	

public :
	CEventMgr()
	{
		m_pItemBag = 0;
		SetItemBagCount( 0 );
	}
	
	int LoadNpcItem();
	int LoadNpcCreate();
	int LoadEventMap();
	int LoadEventSound();
	int LoadEventMessage();
	int LoadEventNpcScript();
	int LoadItemBag();
	int LoadEventMoveMap();

	int CheckNpcItem( CHARLIST *pNpc );
	int CheckActiveEvent( int type );
	int IsActive( int type );
	int CheckCloseEvent( int type );
	int Proc();
	int LoadEvent()
	{
		LoadNpcItem();
		LoadNpcCreate();
		LoadEventMap();
		LoadEventSound();
		LoadEventMessage();
		LoadEventNpcScript();
		LoadItemBag();
		LoadEventMoveMap();
		return 1;
	}
	void *GetActiveEvent( int type );
	int CheckScriptNo( int item_index, short int cn );

	// ������ ���濡 ���õ� �Լ���
	void SetItemBagCount( int count )
	{
		if( m_pItemBag ) delete [] m_pItemBag;
		if( !count )
		{
			m_pItemBag = 0;
			m_nItemBagCount = 0;
			return;
		}
		m_pItemBag = new CEventScriptItemBag[count];
		m_nItemBagCount = count;
	}	
	CEventScriptItemBag	*GetBagPoint( int bag_number )
	{
		for( int i=0; i<m_nItemBagCount; i++ )
		{
			if( m_pItemBag[i].m_nBagNumber == bag_number ) return &m_pItemBag[i];
		}
		return NULL;
	}
	CEventScriptItemBag	*GetBagPointByIndex( int index ) { return &m_pItemBag[index]; }
	CEventNpcScript *GetEventNpcScript( int script_no );
	CEventNpcScript *GetEventNpcScriptByEventNo( int event_no );
	CEventMoveMap *GetEventMoveMapPoint( int event_no );

};





/////////////////////////////////////////////////////////////////////////

// 021117 YGI
class CLightVersion
{
public :
	// �� �̵� ����	// �ʰ����� ������ �ű��.
	string *m_szlpMap;
	int m_nCount;

public :
	CLightVersion()
	{
		m_nCount = 0;
		m_szlpMap = 0;
	}
	~CLightVersion()
	{
		Clear();
	}
	void Clear()
	{
		m_nCount =0;
		SAFE_DELETE_ARRAY(m_szlpMap);
	}		
	int LoadLightVersionMap();
	bool IsAbleMapMove( CHARLIST *ch, const char *mapname );
	bool IsLightVersion( CHARLIST *ch );
};

///////////////////////////////////////////////////////////////////////////
// 030624 YGI
class CEventFlagMgr		// �̺�Ʈ ���� �÷��׵�
{
public : 
	bool m_bMoveMapWhenDie;		// ���� ��� �ٱ����� �ñ��.
	bool m_bSaveLogAboutSaveUserData;	// 030919 HK YGI

public :
	CEventFlagMgr()
	{
		m_bMoveMapWhenDie = 0;
		m_bSaveLogAboutSaveUserData = 0;	// 030919 HK YGI
	}
	bool IsMoveMapWhenDie() const
	{
		return m_bMoveMapWhenDie;
	}
	void SetMoveMapWhenDie( bool flag )
	{
		m_bMoveMapWhenDie = flag;
	}
	// 030919 HK YGI
	void InitSaveLogFlag();
	bool IsSaveLogWhenSaveUserData() 
	{
		return m_bSaveLogAboutSaveUserData;
	}
};
extern CEventFlagMgr g_EventFlagMgr;



extern CLightVersion g_LightVersion;

extern int MakeAndSetItem( ItemAttr *item, int item_no, int item_mutant, int item_grade );
extern CEventMgr	g_EventMgr;
extern void SendSaveAmountEventNpcItem( int event_no, int amount );
extern int SendEventNpcScriptItem( short int cn, int npc_index, int bag_num, char *message, int size );

extern bool CheckContinueLine( char check );

///////////////////////////////////////////////////////////////////////////
// 040105 YGI ����
class CEventFindTreasure
{
	POINTS *m_pXY;		// ���� ��ǥ
	int m_nXYCount;		// ����

public :
	CEventFindTreasure()
	{
		m_pXY = 0;
		m_nXYCount = 0;
	}
	~CEventFindTreasure()
	{
		Clear();
	}
	bool Clear()
	{
		if( m_pXY ) delete [] m_pXY;
		m_pXY = 0;
		m_nXYCount = 0;
		return true;
	}
	bool SetBuffer( int size )
	{
		if( size < 0 ) return false;
		m_pXY = new POINTS[size];
		m_nXYCount = size;
		return true;
	}
	int SetPoint( int index, int x, int y )
	{
		if( index > m_nXYCount ) return false;
		m_pXY[index].x = x;
		m_pXY[index].y = y;
		return true;
	}
	int IsEvent()		// �̺�Ʈ ���ΰ�?
	{
		return m_nXYCount;
	}
	int CheckXY( int x, int y )		// ������ �������ΰ�?
	{
		if( !IsEvent() ) return -1;
		for( int i=0; i<m_nXYCount; i++ )
		{
			if( m_pXY[i].x==x && m_pXY[i].y==y )
			{
				return i;
			}
		}
		return -1;
	}
};

class CTreasureGuardTBL		// �����ؾ� �ϴ� ����
{
public :
	int *m_pNpcNo;
	int *m_pNpcCount;
	int m_nCount;
public : 
	CTreasureGuardTBL()
	{
		m_pNpcNo = 0;
		m_pNpcCount = 0;
		m_nCount = 0;
	}
	~CTreasureGuardTBL()
	{
		Reset();
	}
	void Reset()
	{
		if( m_pNpcNo ) delete [] m_pNpcNo;
		if( m_pNpcCount ) delete [] m_pNpcCount;
		m_pNpcNo = 0;
		m_pNpcCount = 0;
		m_nCount = 0;
	}
	void SetCount( int count )
	{
		m_nCount = count;
		m_pNpcNo = new int[count];
		m_pNpcCount = new int[count];
		memset( m_pNpcNo, 0, sizeof(int)*count);
		memset( m_pNpcCount, 0, sizeof(int)*count);
	}
	void Set( int index, int npc_no, int count )
	{
		if( index >= m_nCount ) return;
		if( index < 0 ) return;
		m_pNpcNo[index] = npc_no;
		m_pNpcCount[index] = count;
	}
};

enum TREASURE_MAP_ITEM
{
	LOW_LEVEL_MAP = 10276, // �ʱ� ����
	MIDDLE_LEVEL_MAP = 10277,
	HIGH_LEVEL_MAP = 10278,
	UNKNOWN_MAP = 10279,
	TREASURE_BOX = 10280,	// ��������
};

class CTreasureGuardMgr
{
public :
	typedef map< int, POINTS > MAP_GUARD;	// points.x --> ���� ���, points.y --> �� ���� ��
	typedef MAP_GUARD::iterator ITOR_MAP_GUARD;

	MAP_GUARD m_Data;

public :
	CTreasureGuardMgr() { }
	~CTreasureGuardMgr()
	{
		m_Data.clear();
	}
	bool IsEvent( int index )		// �̺�Ʈ ���� ���ΰ�?
	{
		ITOR_MAP_GUARD itor = m_Data.find(index);
		if (itor != m_Data.end())
		{
			return true;
		}
		return false;
	}
	int AddNew( int index, POINTS data )
	{
		m_Data.insert( MAP_GUARD::value_type( index, data ) );
		return 1;
	}
	int CreateGuard( int index, int treasure_class, int x, int y, int cn );
	int KilledTreasureGuard( CHARLIST *user, CHARLIST *boss );
};



extern CTreasureGuardTBL g_TreasureGuardTBL[5];
extern CTreasureGuardMgr g_TreasureGuardMgr;


extern CEventFindTreasure g_EventFindTreasure;
extern CEventFlagMgr g_EventFlagMgr;
extern int IsTreasureMapItem( int item_no );

#endif