
#ifndef		_QUEST_IN_MAP_
#define		_QUEST_IN_MAP_

#include <list>
#include <vector>
using namespace std;

enum SCRIPT
{
	SCRIPT_COUNTER_TYPE__NPCKILL = 0,
	SCRIPT_COUNTER_TYPE__ITEMCOUNT = 1,

	SCRIPT_COUNT_STOP = 0,
	SCRIPT_COUNT_START = 1,
	SCRIPT_COUNT_CHECK = 2,

	SCRIPT_END_SUC		=10,
	SCRIPT_END_FAL		=20,
};

class CRequital;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//CScriptTimer	Ÿ�̸Ӹ� ����Ѵ� .

class CScriptTimer
{
	/*
	enum
	{
		MAX_TIMER_TIME	= 5 * 3600 * 1000,		//5�ð��� �����̴�.
		MIN_SYNCH_TIME	= 2 * 1000,				//2�����ϴ�.
	};

  //����ü ����ȭ�� �ʿ䵵 ���� �Լ����� ���߿� �����ؼ� ����ȭ�� ���� 2���� �Լ��� �����Ҽ� �ְ� �ߴ�.
*/
	
public:
	CScriptTimer(int id, char * name, DWORD dwTime, DWORD dwSynch):m_ID(id), m_dwTimerTime(dwTime), m_dwSynchTime(dwSynch)
	{	//< CSD-030509
		strcpy( m_szName, name); 
		m_bStart = false;
		m_dwOld= 0;
		m_dwSynch=0;
		memset(m_szMapFile, 0, sizeof(m_szMapFile));
		m_nPosX = 0;
		m_nPosY = 0;
	}	//> CSD-030509

	CScriptTimer()
	{	//< CSD-030509
		m_ID=0; 
		m_dwTimerTime=0; 
		m_dwSynchTime=0; 
		m_dwSpendTime=0; 
		m_bStart=false; 
		m_dwOld=0; 
		m_dwSynch=0;
		memset(m_szMapFile, 0, sizeof(m_szMapFile));
		m_nPosX = 0;
		m_nPosY = 0;
	}	//> CSD-030509

	~CScriptTimer(){}

private:
	DWORD	m_dwTimerTime;		//�Ҵ�� Ÿ��
	DWORD	m_dwSpendTime;		//���� �ð�. ����ȭ�� ����
	bool	m_bStart;			//Ÿ�̸Ӱ� �õ��Ǿ��°�?
	DWORD	m_dwSynchTime;		//�󸶸��� �ð����� ����ȭ �ϴ°�? �ƴϸ� �� �ñ⸶�� � �Լ��� �����Ұ��ΰ�?

	void	(*m_func)();		//Ÿ�̸� ���鼭 ȣ���ϴ� �Լ�
	void	(*m_syncFunc)();	//Ÿ�̸� ���߿� ȣ���ϴ� �Լ�
	void	(*m_afterFunc)();	//Ÿ�̸� ������ ȣ���ϴ� �Լ�	<-���� ���� ���߿� Ƽ��� ������ �� �Լ��� ȣ���ϰ� �Ѵ�.

	void	AfterTimer();//char* szMap, int x, int y);	//Ÿ�̸Ӱ� ���� ���� �ٸ� �����Ѵ�?

	int		m_ID;				//cn���� �����Ѵ�.
	char	m_szName[31];			//������ ���� �̸��� �����Ѵ�.
	char    m_szMapFile[30]; // CSD-030509
	int     m_nPosX;		 // CSD-030509
	int     m_nPosY;		 // CSD-030509

public:
	DWORD	m_dwOld;
	DWORD	m_dwSynch;

	void	SetID( const int iID){m_ID = iID;};						//Ÿ�̸� id�Ҵ�
	void	SetName( const char* name );//{ strcpy(m_szName, name);};
	void	SetTimerTime( DWORD dwTime);				//Ÿ�̸��� �ð��� �����Ѵ�. 
	void	SetSynchroneTime( DWORD dwSyncTime );		//����ȭ �ϴ� �ð��� ����
	void	SetFunction( void(*func)(), void (*syncFunc)());
	void	SetAfterMap( const char* mapfile, const int& x, const int& y);	//Ÿ�̸� ������ ���۵� �Լ�
	void	ClearTimer();								//Ÿ�̸� ������ ���� 0����

	DWORD	GetCurretTimerTime() const { return m_dwTimerTime;};			//���� ���� �ð��� �˾ƺ���.
	DWORD	GetCurretSynchTime() const { return m_dwSynchTime;};		//������ ����ȭ �ð��� �˾ƺ���.
	DWORD	GetSpendTime() const { return m_dwSpendTime;};
	int		GetConnectionID() const { return m_ID;};						//EventPc���� �ѱ��.
	char*	GetName() { return m_szName;};

	bool	IsRunning();				//�������ΰ�?
	bool	IsStart() const { return m_bStart;};

	void	SendSCRIPT_TIEMER( DWORD dwSpendTime , int iType);		//��ũ�a ���߱� ���� ��Ŷ�� ������.
	bool	ConfirmSyncTimer( DWORD dwTimer);						//��ũ ���⶧ Ÿ�̸Ӱ� �����ϰ� �����ߴ°�?

	bool	StartTimer( bool bOnlyServer = false);				//�ð��� �����Ǿ� ������ m_bStart = true�Ѵ�. �Ű������� true�̸� ������ ����.
	void	RunTimer();					//����ؼ� ����
	void	EndTimer( int iFlag);		//iflag ==0 ����, ==1 ����

	void	Log_UserInfo(const int nCn);	// 030617 kyo
};
//CScriptTimer
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//CScriptCounter
class CScriptCounter
{
public:
	CScriptCounter(){ClearCounter();};
	~CScriptCounter(){};

private:
	int		m_iSpeciesNum;		//ī�����Ҳ� ��ȣ
	int		m_iSpeciesType;		//��� ī�����ұ�?
	int		m_iSpeciesMuch;		//��� ī�����ұ�?
	bool	m_bStart;
	int		m_iCounter;			//���� ��γ�?
	int		m_ID;
	char	m_szName[31];

public:
	void	SetSpeciesNum	( const int& iNum )		{ m_iSpeciesNum = iNum; };
	void	SetSpeciesType	( const int& iType )	{ m_iSpeciesType= iType; };
	void	SetSpeciesMuch	( const int& iMuch )	{ m_iSpeciesMuch= iMuch; };
	void	SetConnectionID ( const int& iID )		{ m_ID = iID;};
	void	SetName			( const char* szName)	{ strcpy( m_szName, szName); };
	void	SetCounter		( const int& iCounter)	{ m_iCounter = iCounter;};	

	int		GetSpeciesNum() const { return m_iSpeciesNum; };
	int		GetSpeciesType() const { return m_iSpeciesType; };
	int		GetSpeciesMuch() const { return m_iSpeciesMuch; };
	int		GetCounter() const { return m_iCounter; };
	int		GetConnectionID() const { return m_ID; };
	char*	GetName() { return m_szName; };

	bool	IsStart() const { return m_bStart; };
	void	ClearCounter();

	void	SendSCRIPT_COUNTER( int iType);
	void	RecvSCRIPT_COUNTER();

	bool	StartCounter();
	void	RunCounter( int iNum );		//����ġ���� 
	void	EndCounter();		// �������� ��Ŷ�� ����.. ī���ʹ� Ÿ�̸Ӱ� ������, ��ũ��Ʈ�� ���϶� �� ��쿡�� �����
	void	AddCounter();		//npc����⿡�� ���Ǵ� ��. �����ø��� ��Ŷ����
};

//CScriptCounter
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CSpellMove
{
public:
	CSpellMove(){};
	CSpellMove(int id, string spells){ m_ID = id; m_szSpell = spells;};
	~CSpellMove(){};
	string		m_szSpell;
	int			m_ID;
	
	int			GetId() const {return m_ID;};
};

class CBossTimer
{

	typedef list<int>::iterator	ITOR_INT;
public:
	enum{
		TYPE_BOSS_ONLY = 0,
		TYPE_BOSS_GROUP = 1,
		TYPE_BOSS_OUT	=2,
	};

	CBossTimer();
	~CBossTimer();
	CScriptTimer m_cBossTimer;
	list<int>	m_lUserCn;
	int			m_nBossID,m_iBossSprNo;
	DWORD		m_dwBossTimer;
	int			m_nFlag;	// ������ �Ѹ����� �Ҳ��� ��ü���Ҳ���. 031029 kyo 
	
	bool AddUser(const int nCn);
	bool DeleteUser(const int nCn);
	int GetUserSize() const {return m_lUserCn.size();}
	void RemoveBossNpc();		//���� ����
	
};

class CQuestInMap
{
	typedef list<CScriptTimer>           TIMERLIST;
	typedef list<CScriptTimer>::iterator TIMERITER;

	typedef list<CScriptCounter>			COUNTERLIST;
	typedef list<CScriptCounter>::iterator	COUNTERITER;

	typedef list<table_quest_in_map>		TB_QUESTINMAP;
	typedef list<table_quest_info_bystep>	TB_QUESTINFO;

	typedef list<table_requital_list >	TB_REQUITALLIST;
	typedef list<table_requital_list >::iterator REQUITALLISTITER;
//<! BBD 040329
	typedef list<table_requital_Item >	TB_REQUITALITEM;
	typedef list<table_requital_Item >::iterator REQUITALITEMITER;
//> BBD 040329

	typedef list<CSpellMove>::iterator	SPELLITER;

	//bosstimer����
	typedef map<int, CBossTimer*>				MAP_BOSSTIMER;
	typedef map<int, CBossTimer*>::iterator		ITOR_BOSS;
	typedef map<int, CBossTimer*>::value_type	PAIR_BOSS;

friend class CScriptTimer;
public:
	CQuestInMap();
	~CQuestInMap();

protected:
	//table
	TB_QUESTINMAP			m_tQuestInMap;
	TB_QUESTINFO			m_tQuestInfo;

public:
	
	// 021014 //�ֹ��Է����� ���̵��� ���
	list<CSpellMove>	m_szClientSpell;				//������ ģ �ֹ��� SendChatData�� �Ѿ�� ::CheckSpellMapMoveArea���� ����ȴ�.
	int			CheckSpell( const int cn, const char* szSpell);	//������ ģ �ֹ��� ��ũ��Ʈ �ֹ��� ���Ѵ�.
	SPELLITER	FindSpellUser( const int cn );				//���� ã��
	void		DeleteSpellUser( const int cn );			//���� ���̱�..
	string		GetSpellWord( const int cn );		//������ �ֹ� ã��

	void	IniteAllTable(){ m_tQuestInMap.clear();m_tQuestInfo.clear();};		//��� ���̺� �����

	int		LoadQuestInfoByStep();	//quest_info_byStep ���̺� load
	int		LoadQuestInMap();		//quest_in_map ���̺� load

	//SetTable
	void	SetQuestInMap(	table_quest_in_map &tp ){			m_tQuestInMap.push_back( tp );};
	void	SetQuestInfoByMap( table_quest_info_bystep &tp){	m_tQuestInfo.push_back( tp );};

	//������ �����´�. 
	list<int> GetQuestInMap( int iMapnumber);		//�ش� �ʿ� �ִ� ����Ʈ�� �Ѱ��ش�.
	list<int> GetRoundUser( const int cn );			//�ش� cn������ ������� ����Ʈ�� �����´�.��
	int		GetQuestInfo( int iQuestNum, int iStep );	//����Ʈ��ȣ�� ���ܰ����� ������ �˾Ƴ���.
		
	//��Ŷ�� ������
	void	SendQuestInfo(	t_quest_in_map *tp, int cn);		
	void	SendShowQuestInfo( t_quest_in_map *tp, int cn );

//script�Լ�
	void	ShowStateMsg( const int& cn, const char* szMsg, const int& R, const int& G, const int& B );	//�޽����� ���������� ����Ѵ�.
	void	ShowStateMsg( const int& cn, const int& iMsgNum, const int& R, const int& G, const int& B );	//�̻��

//BossTimerCheck
	MAP_BOSSTIMER m_mBossTimer;
	int		m_iBossEntree;	//���� ���Դ��� �˰� �ʹٸ� ���߿� list�� �ٲ㼭 cn�� �ִ´�.
	DWORD	StartBossTimer( const int& iCn, const int& iBoss, const int& iX, const int& iY, const DWORD& dwTimer, const int iEntree, const int nFlag);	//bossŸ�̸Ӹ� �����Ѵ�. ������ �����Ѵ�. 
	bool	DeleteBossTimer(int nKey);		//bossŸ�̸Ӹ� ���δ�. 
	void	DeleteBossNpc(int nKey);		//Boss�� ������ ���δ�.
	void	RunBossTimer();		//bossŸ�̸Ӹ� ������. 
	
	DWORD	GetCurrentBossTimer(int nKey);	//���� �����ִ� �ð��� ���ϵȴ�.
	int		GetBossTimerEntree(int nKey);	//���� �濡 �� �ִ� �������� ���ϵȴ�. 
	void	DecreaseBossTimerUser(const int nKey, const int nCn);	
	void	CheckBossTimerBoss( const int nKey);		//���� �����濡 ��������� 0�̸� ������ ���̰� ���� �ݴ´�. 

//Timer�� ���õ� ����
	list<CScriptTimer> m_cTimer;
	void	RunQuestTimer();		//����Ʈ,�ó������� ���õ� ��� Ÿ�̸Ӹ� ������. 	
	bool	StartScriptTimer(const int iServerID, const char* szName, const DWORD &dwTimer, const DWORD &dwSync, const char* mapfile, const int& x, const int& y);		//��ũ��Ʈ �Լ� �ʱ�ȭ
	void	RunScriptTimer();												//very_long_term���� �ǳ� ���� ��
	void	EndScriptTimer( const int iID );
	bool	DeleteScriptTimer( const int iServerID, const char *szName );	//�ش� �̸��� id�� Ÿ�̸Ӹ� ����
	void	DeleteScriptTimer();											//Ÿ�̸Ӱ� ���� ���� ���� ��Ų��.
	bool	CheckID2Name( const int& iID, const char* szName );				//�Ƶڶ� �̸��� �� ����Ʈ�� ��ġ���� �ʴ´�.
	void	RevcScriptTimerPacket( t_script_timer *timer, int iType, int cn );				//script timer�� ���õ� ��� ��Ŷ�� �޴´�.
	list<CScriptTimer>::iterator FindUserInScriptTimer( const int& iId, const char* name);	//�ش� ������ ��ũ��Ʈ Ÿ�̸Ӹ� ����ϰ� �ִ°�?

//Counter�� ���õ� ����.
	list<CScriptCounter> m_cCounter;
	bool	StartScriptCounter( const int iServerID, const char* szName, const int iNum, const int iType, const int iMuch, const int iFlag);
	void	RunScriptCounter( const int cn, const char* szName, int iNum, int iType);
	void	EndScriptCounter( const int iServerId, const char* szName);
	bool	CheckScriptCounter( const int& iServerId, const char* szName, const int iNum, const int iType, const int iCounter );
	void	DeleteScriptCounter();											//ī���Ͱ� ���� ������...�����??
	bool	DeleteScriptCounter( const int iServerId, const char* szName);
	void	AddScriptCounter(const int iServerId, const char* szName);		//�ش� ������ ī���͸� �����ϰ� �� �������� ��Ŷ ������.
	void	RevcScriptCounterSync( t_script_counter *counter,  int cn );	//030108 Ŭ���̾�Ʈ ī������ ���ڸ� �޴´�.
	list<CScriptCounter>::iterator FindUserInScriptCounter( const int& iId, const char* name, const int iType);	//�ش� ������ �ش�Ÿ���� ī���͸� �����ϰ� �ִ°�?
	
//Npc�� ���̸� �ް� �Ǵ� ���� requital_list�� ����Ǿ� �ִ� ���� �ش�. 
	CRequital *m_cRequital;	
	int		LoadRequitalTable();												//���̺� �ε��ؼ� ���� �ֱ�
	void	DeleteRequeitalTable();												//����
	void	GiveRequitalAfterKillNpc( const CHARLIST *a, const CHARLIST *d);	//�׿���. 
	void	RequitalRoundUser( const int& cn, table_requital_list& table);		//���ΰ� ��ó���鿡�� �����ش�.
	void	RequitalKillUser( int cn, table_requital_list &table );				//���γѸ� �����ش�.
	void	GiveRequitalByCn( const int cn , table_requital_list& table);		//cn���� ������ �ش�. 
	list<table_requital_list>::iterator FindRequitalNpc( const int iNpcIndex);	//�ش� npc�� ���̺��� �����ϴ°�?

//spellmapmove ������
	void SendSpellMapMove( const int&cn, const char* szSpell, const char* szMap, const int&  x, const int& y);
	void SendSpellMapMove_Suc(const char* szMap, const int& cn);
	void SendSpellMapMove_Fail(const int& cn);
	void RecvSpellMapMove( const t_script_spellmapmove *spell, const int& cn );

//<! BBD 040329
	int		LoadRequitalItemTable();
	bool Requital_About_SealStone( const int cn, int nSardAmount, int nLeafAmount, int nRef_index);
//> BBD 040329
};


template <class T>
class CTable
{
private:
	list<T>		m_tTable;
	char		m_szTableName[32];
	char		m_szOrderType[32];
	//int			m_iFildeSize;
	vector<t_field_type>	m_tFieldType;
/*
#define	SQL_UNKNOWN_TYPE	0
#define SQL_CHAR            1
#define SQL_NUMERIC         2
#define SQL_DECIMAL         3
#define SQL_INTEGER         4
#define SQL_SMALLINT        5
#define SQL_FLOAT           6
#define SQL_REAL            7
#define SQL_DOUBLE          8
*/


public:
	CTable(){};
	~CTable(){};
	void	InitTable(const char* szTableName, const char* szOrder, const int iFieldNum);
	void	InsertFieldType(t_field_type type){	m_FieldType.push_back(type);	};
	//int		GetFieldType(){	m_FieldType.pop_front();};

	int	LoadTable();
	void	SetTable( const T t){ m_tTable.push_back(t); };
	T		GetTable() { return m_tTable.pop_back();};
};


class CRequital	//�����ֱ� Ŭ����
{	
	typedef list<table_requital_list >	TB_REQUITALLIST;
	typedef list<table_requital_list >::iterator REQUITALLISTITER;
//<! BBD 040329
	typedef list<table_requital_Item >	TB_REQUITALITEM;
	typedef list<table_requital_Item >::iterator REQUITALITEMITER;
//> BBD 040329
protected:
	TB_REQUITALLIST			m_tRequital;
	TB_REQUITALITEM			m_tRequitalItem;	// BBD 040329

public:
	CRequital(){};
	~CRequital(){};

	void	InitTable(){};
	int		LoadRequitalTable();
	void	SetRequitalTable( table_requital_list &table){ m_tRequital.push_back( table); };

	void	GiveRequital2User( const int cn, table_requital_list t);		//cn���� ������ �ش�. 

	//<! BBD 040329
	int		LoadRequitalItemTable();
	void	SetRequitalItemTable( table_requital_Item &table){ m_tRequitalItem.push_back( table); };
	REQUITALITEMITER GetRequital_ItemIttr( const int index);
	//> BBD 040329
	
	REQUITALLISTITER	GetRequital_KillNpc( const int index);
	REQUITALLISTITER	RequitalEnd() { return m_tRequital.end(); }
	REQUITALITEMITER	RequitalItemEnd() { return m_tRequitalItem.end(); }
};

extern CQuestInMap g_QuestInMap;

#endif //_QUEST_IN_MAP_