#include "StdAfx.h"

#include "MAIN.H"
#include "NPC_Pattern.h"
#include "Map.h"
#include "Scrp_exe.h"
#include "monitor.h"
#include "MyLog.h"
#include "ChrLog.h"

#ifdef _GAME_SERVER_
#include "Hong_Area.h"
#include "Kein_Item.h"
#endif

#include "Citem.h"
#include "StepMigration.h"
#include "../Library/Shared/NoticeDatabase.h"

#define BLOCK_SIZE 6000

#define SIZE_OF_ITEMLIST	sizeof(ITEMLIST) * 1000
#define SIZE_OF_NPCLIST		sizeof(CHARLIST) * 500

HENV  hEnv			= NULL;
HDBC  hDBC			= NULL;
HENV  hEnv_TotalDB	= NULL;
HDBC  hDBC_TotalDB	= NULL;
HENV  hEnv_ChrLogDB	= NULL;
HDBC  hDBC_ChrLogDB	= NULL;
HENV  hEnv_NGCDB	= NULL;//030207 lsw
HDBC  hDBC_NGCDB	= NULL;
int FREE_LEVEL_ = 0; // CSD-040127

NPCLev_to_Exp NPC_Lev_Ref[Num_Of_NPC_Lev]; // CSD-TW-030606

extern int var[ DRAGON_MAX_CONNECTIONS][ _SCRIPT_VARABLE_POINT_ ];

extern int  calcAC( CHARLIST *ch );
extern int  calcDefenceRate( CHARLIST *ch );
extern int  calcAttackPowerMax( CHARLIST *ch );
extern int dexDefenseCor( CHARLIST *d ); // 0129 KHS
extern int GetTotalItemWeight( CHARLIST *ch );

extern int get_BinaryData_from_Chr_Info  (UCHAR *ws, UCHAR *ps, UCHAR *skill, UCHAR *skill_exp, UCHAR *tac_skillEXP, UCHAR *script_var, UCHAR *inv, UCHAR *equip, UCHAR *quick,UCHAR *party,UCHAR *relation,UCHAR *employment,UCHAR *itemindex, char *login_id, char *name);
extern int get_BinaryData_from_Chr_Info2 (UCHAR *bankitem,	 char *login_id, char *name);
extern int update_BinaryData_to_Chr_Info	(UCHAR *ws, UCHAR *pa,  UCHAR *akill, UCHAR *skill_exp, UCHAR *tac_skill_exp, UCHAR *script_var, UCHAR *inv,  UCHAR *equip, UCHAR *quick, UCHAR *party,UCHAR *relation,UCHAR *employment,UCHAR *itemindex,char *, char *) ;	//1219 YGI
extern int update_BinaryData_to_Chr_Info2	(UCHAR *bankitem, char *login_id, char *name); // 1225

extern int FREE_LVELE_; // CSD-040127

int CreateChar2_SQL(t_connection c[], int cn );

int UpdateCharID_SQL( int char_id );
int GetCharID_SQL( void );



int InsertBBS( char *name, char *title, char *contents );
int	GetTitleBBS( int count, char date[10][ FILENAME_MAX], char name[10][ FILENAME_MAX], char title[10][ FILENAME_MAX] );
int	GetContentsBBS( char *date, char *con );


char tempdate [10][ FILENAME_MAX];
char tempname [10][ FILENAME_MAX];
char temptitle[10][ FILENAME_MAX];
char tempbbs  [10][ FILENAME_MAX];



////////////////////////////////////////////////////////////////////////////////////////////////////////////////


typedef struct tag_Updating		// ���� Updateing�ϰ� �ִ� Character�� ���� Table�̴�. 
{								// GameServer�� Access Join�Ϸ��� Client�� ������, 
	char id[ ID_LENGTH];		// �� Table�� � ���� �ִٸ� ��ñ�ٷ��� �Ѵ�. 
}UPDATE, *LPUPDATE;

int		CurUpdateChar;
UPDATE  UpdateChar[ MAX_UPDATE_CHARACTER_];

void ConvertLoad(char* pData, int nSize = MAX_STEP)
{	//< CSD-TW-030620
	CStepMigration stepMigration;
	stepMigration.Load(pData);
}	//> CSD-TW-030620

void ConvertSave(char* pData, int nSize = MAX_STEP)
{	//< CSD-TW-030620
	CStepMigration stepMigration;
	stepMigration.Save(pData);
}	//> CSD-TW-030620

void ConvertQueryString(const char* szStr, char* szResult, const int iMaxString = MAX_STEP)
{// ' ���ڸ� ã�� '' �� �ٲ��ش�
//	if(!szStr){return;}

//	const int iLen = ::strlen(szStr);
	int i = 0, j = 0;
	for(; iMaxString > i; i++,j++)
	{
		::memcpy(szResult+j,szStr+i,sizeof(char));
		if(*(szStr+i) == '\'')
		{
			j++;
			::memcpy(szResult+j,szStr+i,sizeof(char));
		}
	}
	
	::memset(szResult+j,0,sizeof(char));
		
	return;//
}

void DisplaySQLError(SQLHDBC hDBC, HSTMT hStmt)
{
	
	UCHAR 	szSqlState[100];
	UCHAR	szErrorMsg[100];
	
	SQLINTEGER	fNativeError	= 0;
	SWORD		cbErrorMsg		= 100;
	RETCODE		rc;
	
	rc = SQLError(hEnv,
		hDBC,
		hStmt,
		szSqlState,
		&fNativeError,
		szErrorMsg,
		100,
		&cbErrorMsg
		);
	
	if (rc != SQL_NO_DATA_FOUND || rc != SQL_ERROR)
	{
		if (fNativeError != 0x1645)	// ignore change database to master context message
		{
			char Buffer[256];
			wsprintf(Buffer,"Error Message: %s SqlState: %s, fNativeError: %x",szErrorMsg, szSqlState,fNativeError);
			MessageBox(GetActiveWindow(),Buffer,"SQLError",MB_OK);
		}
	}
	else
	{	
		MessageBox(GetActiveWindow(),"SQLError() failed: , NO_DATA_FOUND OR SQL_ERROR","SQLError",MB_OK);
	}
	
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int SQLExecDirect_kein( HSTMT hstmt, UCHAR *szQuery, int sql_type )
{
/*
char query[100];
strncpy( query, (char *)szQuery, 99 );
query[99] = 0;

	MyLog(LOG_DEBUG, "Query : %s", query );*/
	return SQLExecDirect( hstmt, szQuery, sql_type );
}

// Helper: build MySQL ODBC connection string from a "DSN alias" (DragonRajaDB/TotalDB/ChrLogDB).
// Falls back to SQLConnect with the DSN name if no mapping is found in DBDemon.ini.
static void BuildMySqlConnString(LPSTR szAlias, LPSTR szUID, LPSTR szUPW, char* outStr, int outSize)
{
	// Map alias -> database name
	const char* szDb = NULL;
	if (stricmp(szAlias, "DragonRajaDB") == 0) szDb = "dragonraja";
	else if (stricmp(szAlias, "TotalDB") == 0) szDb = "dragonraja_total";
	else if (stricmp(szAlias, "ChrLogDB") == 0) szDb = "dragonraja_log";
	else if (stricmp(szAlias, "NgcDB") == 0) szDb = "ngcdb";
	else szDb = szAlias;

	// Try to read driver/server/port from INI [mysql_conn] section.
	// Format: [mysql_conn] driver=... server=... port=...
	char driver[128] = "MySQL ODBC 9.7 Unicode Driver";
	char server[64]  = "127.0.0.1";
	char port[16]    = "3306";
	char opt[64]     = "3";
	GetPrivateProfileString("mysql_conn", "driver", driver, driver, sizeof(driver), DB_DEMON_INI_);
	GetPrivateProfileString("mysql_conn", "server", server, server, sizeof(server), DB_DEMON_INI_);
	GetPrivateProfileString("mysql_conn", "port",   port,   port,   sizeof(port),   DB_DEMON_INI_);
	GetPrivateProfileString("mysql_conn", "option", opt,    opt,    sizeof(opt),    DB_DEMON_INI_);

	// Build the connection string
	_snprintf(outStr, outSize,
		"DRIVER={%s};SERVER=%s;PORT=%s;DATABASE=%s;UID=%s;PWD=%s;OPTION=%s;",
		driver, server, port, szDb, szUID, szUPW, opt);
}

int Init_SQL(LPSTR szDSN, LPSTR szUID, LPSTR szUPW)
{
	RETCODE		retCode;

	SQLAllocEnv(&hEnv);
	SQLAllocConnect( hEnv, &hDBC );

	// First try the new DSN-less MySQL connection string (more reliable across
	// 32/64-bit ODBC driver mismatches).
	{
		char szConnStr[1024] = {0,};
		BuildMySqlConnString(szDSN, szUID, szUPW, szConnStr, sizeof(szConnStr));
		SQLCHAR szOutConn[1024] = {0,};
		SQLSMALLINT cbOutConn = 0;
		retCode = SQLDriverConnect(hDBC, NULL,
			(SQLCHAR*)szConnStr, SQL_NTS,
			szOutConn, sizeof(szOutConn), &cbOutConn,
			SQL_DRIVER_NOPROMPT);
		if (retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
		{
			MyLog(LOG_NORMAL, "MySQL OK: %s (via DriverConnect)", szDSN);
			return(1);
		}

		UCHAR szSqlState[100] = {0,};
		UCHAR szErrorMsg[512] = {0,};
		SQLINTEGER fNativeError = 0;
		SWORD cbErrorMsg = 0;
		SQLError(hEnv, hDBC, SQL_NULL_HSTMT, szSqlState, &fNativeError, szErrorMsg, sizeof(szErrorMsg), &cbErrorMsg);
		MyLog(LOG_FATAL, "SQLDriverConnect FAILED alias='%s' UID='%s' - SQLState=%s NativeError=%d Msg=%s",
			szDSN, szUID, szSqlState, fNativeError, szErrorMsg);
	}

	// Fallback: original SQLConnect with DSN name
	retCode = SQLConnect(hDBC, (UCHAR *)szDSN, SQL_NTS, (UCHAR *)szUID, SQL_NTS, (UCHAR *)szUPW, SQL_NTS);

	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{
		return(1);
	}

	// Log detailed error info
	{
		UCHAR szSqlState[100] = {0,};
		UCHAR szErrorMsg[512] = {0,};
		SQLINTEGER fNativeError = 0;
		SWORD cbErrorMsg = 0;
		SQLError(hEnv, hDBC, SQL_NULL_HSTMT, szSqlState, &fNativeError, szErrorMsg, sizeof(szErrorMsg), &cbErrorMsg);
		MyLog(LOG_FATAL, "SQLConnect FAILED for DSN='%s' UID='%s' - SQLState=%s NativeError=%d Msg=%s",
			szDSN, szUID, szSqlState, fNativeError, szErrorMsg);
	}

	return(0);
}

int Release_SQL(void)
{	
	if(hDBC != NULL)
		SQLDisconnect(hDBC);
	
	SQLFreeConnect(hDBC);
	SQLFreeEnv(hEnv);
	
	return(1);
}

int Init_TotalDB_SQL(LPSTR szDSN, LPSTR szUID, LPSTR szUPW )
{
	RETCODE		retCode;

	SQLAllocEnv(&hEnv_TotalDB);
	SQLAllocConnect(hEnv_TotalDB, &hDBC_TotalDB);

	// First try DSN-less MySQL connection string
	{
		char szConnStr[1024] = {0,};
		BuildMySqlConnString(szDSN, szUID, szUPW, szConnStr, sizeof(szConnStr));
		SQLCHAR szOutConn[1024] = {0,};
		SQLSMALLINT cbOutConn = 0;
		retCode = SQLDriverConnect(hDBC_TotalDB, NULL,
			(SQLCHAR*)szConnStr, SQL_NTS,
			szOutConn, sizeof(szOutConn), &cbOutConn,
			SQL_DRIVER_NOPROMPT);
		if (retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
		{
			MyLog(LOG_NORMAL, "MySQL OK: %s (via DriverConnect)", szDSN);
			return(1);
		}
		UCHAR szSqlState[100] = {0,};
		UCHAR szErrorMsg[512] = {0,};
		SQLINTEGER fNativeError = 0;
		SWORD cbErrorMsg = 0;
		SQLError(hEnv_TotalDB, hDBC_TotalDB, SQL_NULL_HSTMT, szSqlState, &fNativeError, szErrorMsg, sizeof(szErrorMsg), &cbErrorMsg);
		MyLog(LOG_FATAL, "SQLDriverConnect FAILED alias='%s' UID='%s' - SQLState=%s NativeError=%d Msg=%s",
			szDSN, szUID, szSqlState, fNativeError, szErrorMsg);
	}

	// Fallback: original SQLConnect with DSN name
	retCode = SQLConnect(hDBC_TotalDB, (UCHAR *)szDSN, SQL_NTS, (UCHAR *)szUID, SQL_NTS, (UCHAR *)szUPW, SQL_NTS);

	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{
		return(1);
	}

	// Log detailed error info
	{
		UCHAR szSqlState[100] = {0,};
		UCHAR szErrorMsg[512] = {0,};
		SQLINTEGER fNativeError = 0;
		SWORD cbErrorMsg = 0;
		SQLError(hEnv_TotalDB, hDBC_TotalDB, SQL_NULL_HSTMT, szSqlState, &fNativeError, szErrorMsg, sizeof(szErrorMsg), &cbErrorMsg);
		MyLog(LOG_FATAL, "SQLConnect FAILED for DSN='%s' UID='%s' - SQLState=%s NativeError=%d Msg=%s",
			szDSN, szUID, szSqlState, fNativeError, szErrorMsg);
	}

	return(0);
}

int Release_TotalDB_SQL(void)
{	
	if(hDBC_TotalDB != NULL)	
		SQLDisconnect(hDBC_TotalDB);
	
	SQLFreeConnect(hDBC_TotalDB);
	SQLFreeEnv(hEnv_TotalDB);
	
	return(1);
}

int Init_NGCDB_SQL(LPSTR szDSN, LPSTR szUID, LPSTR szUPW )//030207 lsw
{		
	RETCODE		retCode;
	
	SQLAllocEnv(&hEnv_NGCDB);
	SQLAllocConnect(hEnv_NGCDB, &hDBC_NGCDB);
		
	retCode = SQLConnect(hDBC_NGCDB, (UCHAR *)szDSN, SQL_NTS, (UCHAR *)szUID, SQL_NTS, (UCHAR *)szUPW, SQL_NTS);
		
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{	
		return(1);
	}	
		
	return(0);
}

int Release_NGCDB_SQL(void)//030207 lsw
{	
	if(hDBC_NGCDB != NULL)	
		SQLDisconnect(hDBC_NGCDB);
	
	SQLFreeConnect(hDBC_NGCDB);
	SQLFreeEnv(hEnv_NGCDB);
	
	return(1);
}

int Init_ChrLogDB_SQL(LPSTR szDSN, LPSTR szUID, LPSTR szUPW )
{
	RETCODE		retCode;

	SQLAllocEnv(&hEnv_ChrLogDB);
	SQLAllocConnect(hEnv_ChrLogDB, &hDBC_ChrLogDB);

	// First try DSN-less MySQL connection string
	{
		char szConnStr[1024] = {0,};
		BuildMySqlConnString(szDSN, szUID, szUPW, szConnStr, sizeof(szConnStr));
		SQLCHAR szOutConn[1024] = {0,};
		SQLSMALLINT cbOutConn = 0;
		retCode = SQLDriverConnect(hDBC_ChrLogDB, NULL,
			(SQLCHAR*)szConnStr, SQL_NTS,
			szOutConn, sizeof(szOutConn), &cbOutConn,
			SQL_DRIVER_NOPROMPT);
		if (retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
		{
			MyLog(LOG_NORMAL, "MySQL OK: %s (via DriverConnect)", szDSN);
			return(1);
		}
		UCHAR szSqlState[100] = {0,};
		UCHAR szErrorMsg[512] = {0,};
		SQLINTEGER fNativeError = 0;
		SWORD cbErrorMsg = 0;
		SQLError(hEnv_ChrLogDB, hDBC_ChrLogDB, SQL_NULL_HSTMT, szSqlState, &fNativeError, szErrorMsg, sizeof(szErrorMsg), &cbErrorMsg);
		MyLog(LOG_FATAL, "SQLDriverConnect FAILED alias='%s' UID='%s' - SQLState=%s NativeError=%d Msg=%s",
			szDSN, szUID, szSqlState, fNativeError, szErrorMsg);
	}

	// Fallback: original SQLConnect with DSN name
	retCode = SQLConnect(hDBC_ChrLogDB, (UCHAR *)szDSN, SQL_NTS, (UCHAR *)szUID, SQL_NTS, (UCHAR *)szUPW, SQL_NTS);

	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{
		return(1);
	}

	// Log detailed error info
	{
		UCHAR szSqlState[100] = {0,};
		UCHAR szErrorMsg[512] = {0,};
		SQLINTEGER fNativeError = 0;
		SWORD cbErrorMsg = 0;
		SQLError(hEnv_ChrLogDB, hDBC_ChrLogDB, SQL_NULL_HSTMT, szSqlState, &fNativeError, szErrorMsg, sizeof(szErrorMsg), &cbErrorMsg);
		MyLog(LOG_FATAL, "SQLConnect FAILED for DSN='%s' UID='%s' - SQLState=%s NativeError=%d Msg=%s",
			szDSN, szUID, szSqlState, fNativeError, szErrorMsg);
	}

	return(0);
};


int Release_ChrLogDB_SQL(void)
{	
	if(hDBC_ChrLogDB != NULL)	SQLDisconnect(hDBC_ChrLogDB );
	
	SQLFreeConnect(hDBC_ChrLogDB);
	SQLFreeEnv(hEnv_ChrLogDB);
	
	return(1);
};	

int Querry_SQL(LPSTR szQuerry, HDBC hdbc)
{	//< CSD-030804
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	if( hdbc == NULL ) hdbc = hDBC;
	
	SQLAllocStmt(hdbc, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{
		SQLFreeStmt(hStmt, SQL_DROP);
		return 1;
	}
	
	SQLFreeStmt(hStmt, SQL_DROP);
	return -1;
}	//> CSD-030804

// chr_log_info ���̺����� ĳ�����̸��� ã�´�. 
int GetCharacterNameInID(LPSTR id, char name[4][20])
{
    memset(name, 0, sizeof(char) * 4 * 20);
    if (!id || !id[0] || strnlen_s(id, ID_LENGTH + 1) > ID_LENGTH)
        return -1;

    HSTMT statement = NULL;
    SQLLEN idLength = SQL_NTS;
    const auto executeForId = [&](const char* query) -> bool
    {
        if (statement)
        {
            SQLFreeStmt(statement, SQL_DROP);
            statement = NULL;
        }
        RETCODE result = SQLAllocStmt(hDBC, &statement);
        if (!SQL_SUCCEEDED(result) || !statement) return false;
        result = SQLPrepare(statement, (UCHAR*)query, SQL_NTS);
        if (!SQL_SUCCEEDED(result)) return false;
        result = SQLBindParameter(statement, 1, SQL_PARAM_INPUT, SQL_C_CHAR,
                                  SQL_VARCHAR, ID_LENGTH, 0, id,
                                  strlen(id) + 1, &idLength);
        if (!SQL_SUCCEEDED(result)) return false;
        return SQL_SUCCEEDED(SQLExecute(statement));
    };

    if (!executeForId(
            "SELECT name1, name2, name3, name4 "
            "FROM chr_log_info WHERE login_id = ? LIMIT 1"))
    {
        MyLog(LOG_FATAL, "[LOGIN] Failed to read character names for id='%s'", id);
        if (statement) SQLFreeStmt(statement, SQL_DROP);
        return -1;
    }

    const RETCODE fetched = SQLFetch(statement);
    if (fetched == SQL_NO_DATA)
    {
        // Keep the original behavior: an authenticated account gets a
        // main-database row before its first character is created.
        const bool inserted = executeForId(
            "INSERT INTO chr_log_info (login_id, vote, passwd) "
            "VALUES (?, 0, 'make by db_demon')");
        if (statement) SQLFreeStmt(statement, SQL_DROP);
        if (!inserted)
            MyLog(LOG_FATAL, "[LOGIN] Failed to create character slots for id='%s'", id);
        return inserted ? 1 : -1;
    }
    if (!SQL_SUCCEEDED(fetched))
    {
        MyLog(LOG_FATAL, "[LOGIN] Failed to fetch character names for id='%s'", id);
        SQLFreeStmt(statement, SQL_DROP);
        return -1;
    }

    for (int slot = 0; slot < 4; ++slot)
    {
        SQLLEN length = 0;
        const RETCODE result = SQLGetData(statement, slot + 1, SQL_C_CHAR,
                                           name[slot], sizeof(name[slot]), &length);
        if (!SQL_SUCCEEDED(result) ||
            (length != SQL_NULL_DATA &&
             (length < 0 || length >= static_cast<SQLLEN>(sizeof(name[slot])))))
        {
            MyLog(LOG_FATAL, "[LOGIN] Invalid character name in slot %d for id='%s'",
                  slot + 1, id);
            SQLFreeStmt(statement, SQL_DROP);
            return -1;
        }
        if (length == SQL_NULL_DATA) name[slot][0] = 0;
    }
    SQLFreeStmt(statement, SQL_DROP);
    return 1;
}


// GetCharacterNameInID( LPSTR id, char *name[4] )���� ã�� �̸��� ������  �⺻������ �̾Ƴ���. 
int GetCharInfo_SQL2( char *realid, char name[4][20], t_packet *packet)		// 1027 YGI
{				
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	SQLLEN		cbValue;
	char		szQuerry[255];
	int			i;
	char		id[ 20];
	int			Level, Job, Class, Gender;
	int			bodyr, bodyg, bodyb, clothr, clothg, clothb;
	DWORD		money;
	int			age;
	int 		acc_equip1, acc_equip2, acc_equip3, acc_equip4;
	int			nation[4];
	
	t_server_accept_login *tp;
	tp = &packet->u.server_accept_login;
				
	for( i = 0 ; i < 4 ; i ++)
	{
		if( name[i][0] == 0 )
		{
			tp->name[i][0] = 0;
			continue;
		}		
		
		sprintf(szQuerry, "select login_id, lev, job, class, gender, bodyr, bodyg, bodyb, "
			"clothr,clothg, clothb, money, age, "
			"acc_equip1, acc_equip2, acc_equip3, acc_equip4, nation "
			" from chr_info where name='%s'", name[i] );
		
		//    sprintf(szQuerry, "CALL up_get_char_info2( '%s')", name[i]);
		
		SQLAllocStmt(hDBC, &hStmt);
		retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
		if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
		{		
			retCode = SQLFetch(hStmt);
			// 010301 KHS   chr_log_info has name, but There isn't name in chr_info
			if( retCode == SQL_NO_DATA ) 
			{	
				tp->name[i][0] = 0;
			}	
			else
			{	
				//"select login_id, name, lev, job, class, gender, bodyr, bodyg, bodyb, "
				retCode = SQLGetData(hStmt, 1, SQL_C_CHAR,  &id,	   20, &cbValue);
				
				char tempid[ 20+1];
				strcpy( tempid, id );
				EatRearWhiteChar(	tempid);	CharUpper( tempid);
				if( strcmp( tempid, realid ) )	// Character�� ���������� �ƴϴ�..     
				{	
					RecvHackingUser( realid, name[i], 20009, " ", "Not His Char" );
					tp->name[i][0] = 0;
					SQLFreeStmt(hStmt, SQL_DROP);
					
					sprintf(szQuerry, "UPDATE chr_log_info SET name%d = NULL WHERE login_id = '%s' ", i+1, realid );	// ���� �̸��� ���� ���?�����?
					SQLAllocStmt(hDBC, &hStmt);
					retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
					SQLFreeStmt(hStmt, SQL_DROP);
					continue;
				}
				
				retCode = SQLGetData(hStmt, 2, SQL_C_ULONG, &Level,		0, &cbValue);
				retCode = SQLGetData(hStmt, 3, SQL_C_ULONG, &Job,		0, &cbValue);
				retCode = SQLGetData(hStmt, 4, SQL_C_ULONG, &Class,		0, &cbValue);
				retCode = SQLGetData(hStmt, 5, SQL_C_ULONG, &Gender,	0, &cbValue);
				retCode = SQLGetData(hStmt, 6, SQL_C_ULONG, &bodyr,		0, &cbValue);
				retCode = SQLGetData(hStmt, 7, SQL_C_ULONG, &bodyg,		0, &cbValue);
				retCode = SQLGetData(hStmt, 8, SQL_C_ULONG, &bodyb,		0, &cbValue);
				//"clothr,clothg, clothb, money, age, "
				retCode = SQLGetData(hStmt, 9, SQL_C_ULONG, &clothr,	0, &cbValue);
				retCode = SQLGetData(hStmt,10, SQL_C_ULONG, &clothg,	0, &cbValue);
				retCode = SQLGetData(hStmt,11, SQL_C_ULONG, &clothb,	0, &cbValue);
				retCode = SQLGetData(hStmt,12, SQL_C_ULONG, &money,		0, &cbValue);
				retCode = SQLGetData(hStmt,13, SQL_C_ULONG, &age,		0, &cbValue);
				//"acc_equip1, acc_equip2, acc_equip3, acc_equip4 "
				retCode = SQLGetData(hStmt,14, SQL_C_ULONG, &acc_equip1, 0, &cbValue );
				retCode = SQLGetData(hStmt,15, SQL_C_ULONG, &acc_equip2, 0, &cbValue );
				retCode = SQLGetData(hStmt,16, SQL_C_ULONG, &acc_equip3, 0, &cbValue );
				retCode = SQLGetData(hStmt,17, SQL_C_ULONG, &acc_equip4, 0, &cbValue );
				retCode = SQLGetData(hStmt,18, SQL_C_ULONG, &nation[i], 0, &cbValue );
				
				memcpy(tp->name[i], name[i], NM_LENGTH);
				tp->level[i]		= Level;
				tp->job[i]			= Job;
				tp->cla[i]			= Class;
				tp->gender[i]		= Gender;
				tp->bodyr[i]		= bodyr;
				tp->bodyg[i]		= bodyg;
				tp->bodyb[i]		= bodyb;
				tp->clothr[i]		= clothr;
				tp->clothg[i]		= clothg;
				tp->clothb[i]		= clothb;
				tp->money[i]		= money;
				tp->age[i]			= age;
				tp->acc_equip1[i]	= acc_equip1;
				tp->acc_equip2[i]	= acc_equip2;
				tp->acc_equip3[i]	= acc_equip3;
				tp->acc_equip4[i]	= acc_equip4;
			}		
		}			
		else		
		{			
			tp->name[i][0] = 0;
		}		
		SQLFreeStmt(hStmt, SQL_DROP);
	}		
	
	// ������ �ִ� ĳ������ ������ ���ؼ� 
	// �̻��� ���?��Ÿ ���?ó���Ѵ�.
	tp->nation = 0;			// 1027 YGI
	for( int j=0; j<4; j++ )
	{
		if( tp->name[j][0] )		// �̸��� ������ �ִ� ĳ���Ͱ�
		{
			CCharRank temp_status;
			memcpy( &temp_status, &nation[j], sizeof( DWORD ) );
			if( temp_status.nation == 0 )		// ĳ������ �Ѹ��̶� ���� 0�� ������ ������ ���� ���� �ٽ�
			{
				tp->nation = 0; 
				break;
			}
			if( tp->nation && ( tp->nation != temp_status.nation ) ) // ĳ������ ���� �ٸ� ���� ������ �ִٸ�		// 1028 YGI
			{
				tp->nation = 0;
				break;
			}
			tp->nation = temp_status.nation;
		}
	}
	return 1;
};			




int GetCharactersBasicInfoInID_SQL( LPSTR id, t_packet *packet )
{
	char name[ 4][ NM_LENGTH]={{0,},};
	if (GetCharacterNameInID(id, name) != 1)
		return -1;
	return GetCharInfo_SQL2(id, name, packet);
}




int GetCharInfo_SQL(LPSTR szUID, t_packet *packet)
{	
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	SQLLEN		cbValue;
	char		szQuerry[255];
	int			Num;
	char		Name[ID_LENGTH];
	int			Level, Job, Class, Gender;
	int			bodyr, bodyg, bodyb, clothr, clothg, clothb;
	DWORD		money;
	int			age;
	int 		acc_equip1, acc_equip2, acc_equip3, acc_equip4;
	
	t_server_accept_login *tp;
	tp = &packet->u.server_accept_login;
	
	Num = 0;
	//sprintf(szQuerry, "select name, lev, job, class, gender, bodyr, bodyg, bodyb, "
	//					  "clothr,clothg, clothb, money, age, "
	//					  "acc_equip1, acc_equip2, acc_equip3, acc_equip4 "
	//            " from chr_info where login_id='%s'", szUID);
	/////////////////////////////////////////////////////////////////////////////
	sprintf(szQuerry, "CALL up_get_char_info( '%s')", szUID);
	/////////////////////////////////////////////////////////////////////////////
	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{
		retCode = SQLFetch(hStmt);
		
		while(retCode == SQL_SUCCESS)
		{
			//"select name, lev, job, class, gender, bodyr, bodyg, bodyb, "
			retCode = SQLGetData(hStmt, 1, SQL_C_CHAR, Name, ID_LENGTH, &cbValue);
			retCode = SQLGetData(hStmt, 2, SQL_C_ULONG, &Level,	0, &cbValue);
			retCode = SQLGetData(hStmt, 3, SQL_C_ULONG, &Job,		0, &cbValue);
			retCode = SQLGetData(hStmt, 4, SQL_C_ULONG, &Class,	0, &cbValue);
			retCode = SQLGetData(hStmt, 5, SQL_C_ULONG, &Gender,	0, &cbValue);
			retCode = SQLGetData(hStmt, 6, SQL_C_ULONG, &bodyr,	0, &cbValue);
			retCode = SQLGetData(hStmt, 7, SQL_C_ULONG, &bodyg,	0, &cbValue);
			retCode = SQLGetData(hStmt, 8, SQL_C_ULONG, &bodyb,	0, &cbValue);
			//"clothr,clothg, clothb, money, age, "
			retCode = SQLGetData(hStmt, 9, SQL_C_ULONG, &clothr,	0, &cbValue);
			retCode = SQLGetData(hStmt,10, SQL_C_ULONG, &clothg,	0, &cbValue);
			retCode = SQLGetData(hStmt,11, SQL_C_ULONG, &clothb,	0, &cbValue);
			retCode = SQLGetData(hStmt,12, SQL_C_ULONG, &money,	0, &cbValue);
			retCode = SQLGetData(hStmt,13, SQL_C_ULONG, &age,		0, &cbValue);
			//"acc_equip1, acc_equip2, acc_equip3, acc_equip4 "
			retCode = SQLGetData(hStmt,14, SQL_C_ULONG, &acc_equip1, 0, &cbValue );
			retCode = SQLGetData(hStmt,15, SQL_C_ULONG, &acc_equip2, 0, &cbValue );
			retCode = SQLGetData(hStmt,16, SQL_C_ULONG, &acc_equip3, 0, &cbValue );
			retCode = SQLGetData(hStmt,17, SQL_C_ULONG, &acc_equip4, 0, &cbValue );
			
			//name, lev, job, class, gender, bodyr, bodyg, bodyb, clothr, clothg, clothb, money, age
			memcpy( &tp->name[Num][0], Name, ID_LENGTH);
			tp->level[Num]	= Level;
			tp->job[Num]		= Job;
			tp->cla[Num]		= Class;
			tp->gender[Num]	= Gender;
			
			tp->bodyr[Num]	= bodyr;
			tp->bodyg[Num]	= bodyg;
			tp->bodyb[Num]	= bodyb;
			tp->clothr[Num]	= clothr;
			tp->clothg[Num]	= clothg;
			tp->clothb[Num]	= clothb;
			
			tp->money[Num]	= money;
			tp->age[Num]		= age;
			tp->acc_equip1[Num]	= acc_equip1;
			tp->acc_equip2[Num]	= acc_equip2;
			tp->acc_equip3[Num]	= acc_equip3;
			tp->acc_equip4[Num]	= acc_equip4;
			
			Num ++;
			if(Num >= MAX_CHARACTEROFID )
				break;
			
			retCode = SQLFetch(hStmt);
		}
		
		SQLFreeStmt(hStmt, SQL_DROP);
		return(1);
	}
	
	SQLFreeStmt(hStmt, SQL_DROP);		// 0308 YGI
	return(-1);
};





#define READSQL( a, b, c )  retCode = SQLGetData(hStmt,  a, b, &c, 0, &cbValue);


#define SET_LIMIT( a, b, c )   { if( (a) > (c) ) (a) = (c);  if( (a) < (b) ) (a) = (b); }




int GetCharDB_SQL(t_connection c[], int cn)
{
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	SQLLEN		cbValue;
	char		szQuerry[2048];
	int			t;
	char		exp[ 12], money[12];
	CHARLIST	*ch = &c[cn].chrlst;
	
	sprintf(szQuerry, 
		"Select lev, exp, "
		"gender, face, spritvalue, class, spell, "
		"str, con, dex, wis, `int`, movep, `char`, endu, moral, "
		"money, hp, hpmax, mana, manamax, hungry, hungrymax, `condition`, "
		"sprno, x, y, mapname, sight, "
		"bodyr, bodyg, bodyb, clothr, clothg, clothb, "
		"Age, luck, wsps, nation, "
		"acc_equip1, acc_equip2, acc_equip3, acc_equip4, bAlive, "
		"openhouse, disease1, disease2, disease3, disease4, disease5, disease6, total_id, "
		"resist_poison, resist_stone, resist_magic, resist_fire, resist_ice, resist_elect, viewtype, job, "
		"social_status, fame, fame_pk, guildname "
		"From chr_info Where name='%s'", c[cn].name );
	
	//sprintf(szQuerry, "CALL up_get_char_db( '%s')", c[cn].name);
	strcpy( ch->Name, c[cn].name );
	EatRearWhiteChar( ch->Name );
	
	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{		
		retCode = SQLFetch(hStmt);
		
		if(retCode == SQL_SUCCESS)
		{	
			//				"Select lev, exp "
			retCode = SQLGetData(hStmt,  1, SQL_C_ULONG, &t, 0, &cbValue);         ch->Level		 = (short )t;
			retCode = SQLGetData(hStmt,  2, SQL_C_CHAR,  exp, 11, &cbValue);        ch->Exp			 = (DWORD ) atof( exp );
			//				"gender, face, spritvalue, class, spell, "				
			retCode = SQLGetData(hStmt,  3, SQL_C_ULONG, &t, 0, &cbValue);         ch->Gender		 = (short )t;
			retCode = SQLGetData(hStmt,  4, SQL_C_ULONG, &t, 0, &cbValue);         ch->Face		 = (short )t;
			retCode = SQLGetData(hStmt,  5, SQL_C_ULONG, &t, 0, &cbValue);         ch->nGuildCode	 = (short )t; // CSD-030324
			retCode = SQLGetData(hStmt,  6, SQL_C_ULONG, &t, 0, &cbValue);         ch->Class		 = (short )t;
			retCode = SQLGetData(hStmt,  7, SQL_C_ULONG, &t, 0, &cbValue);         ch->Spell		 = (short )t;
			//				"str, con, dex, wis, `int`, movep, `char`, endu, moral, "			
			retCode = SQLGetData(hStmt,  8, SQL_C_ULONG, &t, 0, &cbValue);         ch->Str			 = (short )t; //SET_LIMIT( ch->Str, 0, 100 );
			retCode = SQLGetData(hStmt,  9, SQL_C_ULONG, &t, 0, &cbValue);         ch->Con			 = (short )t; //SET_LIMIT( ch->Con, 0, 100 );
			retCode = SQLGetData(hStmt, 10, SQL_C_ULONG, &t, 0, &cbValue);         ch->Dex			 = (short )t; //SET_LIMIT( ch->Dex, 0, 100 );
			retCode = SQLGetData(hStmt, 11, SQL_C_ULONG, &t, 0, &cbValue);         ch->Wis			 = (short )t; //SET_LIMIT( ch->Wis, 0, 100 );
			retCode = SQLGetData(hStmt, 12, SQL_C_ULONG, &t, 0, &cbValue);         ch->Int			 = (short )t; //SET_LIMIT( ch->Int, 0, 100 );
			retCode = SQLGetData(hStmt, 13, SQL_C_ULONG, &t, 0, &cbValue);         ch->MoveP		 = (short )t; //SET_LIMIT( ch->MoveP, 0, 100 );
			retCode = SQLGetData(hStmt, 14, SQL_C_ULONG, &t, 0, &cbValue);         ch->Char		 = (short )t; //SET_LIMIT( ch->Char,  0, 100 );
			retCode = SQLGetData(hStmt, 15, SQL_C_ULONG, &t, 0, &cbValue);         ch->Endu		 = (short )t; //SET_LIMIT( ch->Endu,  0, 100 );
			retCode = SQLGetData(hStmt, 16, SQL_C_ULONG, &t, 0, &cbValue);         ch->Moral		 = (short )t; //SET_LIMIT( ch->Moral, 0, 100 );
			//				"money, hp, hpmax, mana, manamax, hungry, hungrymax, `condition`, "				
			//< CSD-011012
			retCode = SQLGetData(hStmt, 17, SQL_C_CHAR, money, 11, &cbValue);		  ch->Money = (DWORD ) atof( money );
			retCode = SQLGetData(hStmt, 18, SQL_C_LONG, &t, 0, &cbValue);         ch->Hp = t; 
			retCode = SQLGetData(hStmt, 19, SQL_C_ULONG, &t, 0, &cbValue);         ch->HpMax = t; SET_LIMIT(ch->Hp, 0, ch->HpMax);
			retCode = SQLGetData(hStmt, 20, SQL_C_ULONG, &t, 0, &cbValue);         ch->Mana = t;
			retCode = SQLGetData(hStmt, 21, SQL_C_ULONG, &t, 0, &cbValue);         ch->ManaMax = t; SET_LIMIT(ch->Mana, 0, ch->ManaMax);
			retCode = SQLGetData(hStmt, 22, SQL_C_ULONG, &t, 0, &cbValue);         ch->Hungry = t;
			retCode = SQLGetData(hStmt, 23, SQL_C_ULONG, &t, 0, &cbValue);         ch->HungryMax = t; SET_LIMIT(ch->Hungry, 0, ch->HungryMax);
			//> CSD-011012
			retCode = SQLGetData(hStmt, 24, SQL_C_ULONG, &t, 0, &cbValue);         ch->Condition	 = (BYTE  )t;
			
			if( ch->Condition == CON_DEATH )		//0206 YGI
			{
			}
			else ch->Condition = CON_NORMAL;		// �������°� �ƴ϶��?�⺻������ '�Ź���'���� ���´� ���ش�...
			
			//				"sprno, x, y, mapname, peacests, sight, "
			retCode = SQLGetData(hStmt, 25, SQL_C_ULONG, &t , 0, &cbValue); ch->SprNo		= (short )t;
			retCode = SQLGetData(hStmt, 26, SQL_C_ULONG, &t , 0, &cbValue); ch->X			= (short )t;
			retCode = SQLGetData(hStmt, 27, SQL_C_ULONG, &t , 0, &cbValue); ch->Y			= (short )t;
			retCode = SQLGetData(hStmt, 28, SQL_C_CHAR, ch->MapName , 20, &cbValue);
			retCode = SQLGetData(hStmt, 29, SQL_C_ULONG, &t, 0, &cbValue);	ch->Sight	= t;
			//				"bodyr, bodyg, bodyb, clothr, clothg, clothb, "
			retCode = SQLGetData(hStmt, 30, SQL_C_ULONG, &t, 0, &cbValue);	ch->BodyR			= (short )t;
			retCode = SQLGetData(hStmt, 31, SQL_C_ULONG, &t, 0, &cbValue);	ch->BodyG			= (short )t;
			retCode = SQLGetData(hStmt, 32, SQL_C_ULONG, &t, 0, &cbValue);	ch->BodyB			= (short )t;
			retCode = SQLGetData(hStmt, 33, SQL_C_ULONG, &t, 0, &cbValue);	ch->ClothR			= (short )t;
			retCode = SQLGetData(hStmt, 34, SQL_C_ULONG, &t, 0, &cbValue);	ch->ClothG			= (short )t;
			retCode = SQLGetData(hStmt, 35, SQL_C_ULONG, &t, 0, &cbValue);	ch->ClothB			= (short )t;
			//				"Age, luck, wsps, nation, "
			retCode = SQLGetData(hStmt, 36, SQL_C_ULONG, &t, 0, &cbValue);	ch->Age				= (short )t;	
			retCode = SQLGetData(hStmt, 37, SQL_C_ULONG, &t, 0, &cbValue);	ch->Luck			= (short )t; //SET_LIMIT( ch->Luck, 0, 100 );
			retCode = SQLGetData(hStmt, 38, SQL_C_ULONG, &t, 0, &cbValue);	ch->wsps			= (short )t; //SET_LIMIT( ch->wsps, 0, 100 );
			retCode = SQLGetData(hStmt, 39, SQL_C_ULONG, &t, 0, &cbValue);	memcpy( &ch->name_status, &t, sizeof( DWORD ) );	// 1004 YGI
			
			//				"acc_equip1, acc_equip2, acc_equip3, acc_equip4, bAlive "
			retCode = SQLGetData(hStmt, 40, SQL_C_ULONG, &t, 0, &cbValue);	ch->accessory[0]	= (unsigned char )t; 
			retCode = SQLGetData(hStmt, 41, SQL_C_ULONG, &t, 0, &cbValue);	ch->accessory[1]	= (unsigned char )t;
			retCode = SQLGetData(hStmt, 42, SQL_C_ULONG, &t, 0, &cbValue);	ch->accessory[2]	= (unsigned char )t;
			retCode = SQLGetData(hStmt, 43, SQL_C_ULONG, &t, 0, &cbValue);	ch->accessory[3]	= (unsigned char )t;
			retCode = SQLGetData(hStmt, 44, SQL_C_ULONG, &t, 0, &cbValue);	ch->bAlive			= (unsigned char )t;
			//"openhouse, disease1, disease2, disease3, di5ease4, disease5, disease6 ", 
			retCode = SQLGetData(hStmt, 45, SQL_C_ULONG, &t, 0, &cbValue);	ch->openhouse		= (short )t;
			retCode = SQLGetData(hStmt, 46, SQL_C_ULONG, &t, 0, &cbValue);	ch->disease[0]		= (char )t;
			retCode = SQLGetData(hStmt, 47, SQL_C_ULONG, &t, 0, &cbValue);	ch->disease[1]		= (char )t;
			retCode = SQLGetData(hStmt, 48, SQL_C_ULONG, &t, 0, &cbValue);	ch->disease[2]		= (char )t;
			retCode = SQLGetData(hStmt, 49, SQL_C_ULONG, &t, 0, &cbValue);	ch->disease[3]		= (char )t;
			retCode = SQLGetData(hStmt, 50, SQL_C_ULONG, &t, 0, &cbValue);	ch->disease[4]		= (char )t;
			retCode = SQLGetData(hStmt, 51, SQL_C_ULONG, &t, 0, &cbValue);	ch->disease[5]		= (char )t;
			retCode = SQLGetData(hStmt, 52, SQL_C_ULONG, &t, 0, &cbValue);	ch->total_id 		= t;
			//				"resis_poison, resis_stone,	resis_magic, resis_fire, resis_ice, resis_elect "
			
			retCode = SQLGetData(hStmt, 53, SQL_C_ULONG, &t, 0, &cbValue);	ch->nPoison	= t;
			retCode = SQLGetData(hStmt, 54, SQL_C_ULONG, &t, 0, &cbValue);	ch->nCurse	= t;
			retCode = SQLGetData(hStmt, 55, SQL_C_ULONG, &t, 0, &cbValue);	ch->nHoly	= t;
			retCode = SQLGetData(hStmt, 56, SQL_C_ULONG, &t, 0, &cbValue);	ch->nFire	= t;
			retCode = SQLGetData(hStmt, 57, SQL_C_ULONG, &t, 0, &cbValue);	ch->nIce	= t;
			retCode = SQLGetData(hStmt, 58, SQL_C_ULONG, &t, 0, &cbValue);	ch->nElect	= t;
			retCode = SQLGetData(hStmt, 59, SQL_C_ULONG, &t, 0, &cbValue);	ch->viewtype = (char )t;
			retCode = SQLGetData(hStmt, 60, SQL_C_ULONG, &t, 0, &cbValue);	ch->Job		= (short int )t;		// 0212 YGI
			//				"social_status, fame, fame_pk"
			retCode = SQLGetData(hStmt, 61, SQL_C_ULONG, &t, 0, &cbValue);	ch->social_status	= (short )t;			// 
			retCode = SQLGetData(hStmt, 62, SQL_C_ULONG, &t, 0, &cbValue);	ch->fame			= (short )t;
			retCode = SQLGetData(hStmt, 63, SQL_C_LONG, &t, 0, &cbValue);			memcpy(&ch->NWCharacter,&t,sizeof(DWORD));	// 010915 LTS
			//< CSD-010927
			retCode = SQLGetData(hStmt, 64, SQL_C_CHAR, ch->aStepInfo , 20, &cbValue); 
			ConvertLoad(ch->aStepInfo);
			//> CSD-010927
			SQLFreeStmt(hStmt, SQL_DROP);
			
			return(1);
		}
		else
		{
			SQLFreeStmt(hStmt, SQL_DROP);		// 0308 YGI
			return(-2);
		}
		}
		else
		{
			SQLFreeStmt(hStmt, SQL_DROP);		// 0308 YGI
			return(-1);
		}
};








//  ���� GetCharDB_SQL()��  Login Server������ ���Ǵ� �Լ��� Client�� �⺻������ ������ ������ �ַ� �ϰ� 
//  �̹� GetCharGameDB_SQL()�� ���� Server������ ���Ǵ� ������ �б����Ѱ��̴�. 
//  GameServer������ ���Լ��� ����Ѵ�? 
int GetCharGameDB_SQL(t_connection c[], int cn)
{
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	SQLLEN		cbValue;
	char		szQuerry[2048];
	DWORD		t;
	char		money[12];
	CHARLIST	*ch = &c[cn].chrlst;
	
	// 010406 YGI
	//sprintf(szQuerry,	"Select  bankmoney, LastLoan, "
	//				"nut1, nut2, nut3, killmon, killanimal, killpc, "
	//				"reserved_point, "
	//				"tactics, "
	//				"win_defeat, LadderScore  "
	//				"From chr_info Where name='%s'", c[cn].name );
	/////////////////////////////////////////////////////////////////////////////
	sprintf(szQuerry, "CALL up_get_char_game_db( '%s')", c[cn].name);
	/////////////////////////////////////////////////////////////////////////////
	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{
		retCode = SQLFetch(hStmt);
		
		if(retCode == SQL_SUCCESS)
		{
			// "Select  BankMoney, LastTotal, LastLoan, LastLoan_time, "
			retCode = SQLGetData(hStmt,  1, SQL_C_CHAR, money, 11, &cbValue);		ch->BankMoney		= (DWORD )::atof( money );
			
			//retCode = SQLGetData(hStmt,  2, SQL_C_ULONG, &t, 0, &cbValue);			ch->LastTotal		= t;	// 010314 YGI
			retCode = SQLGetData(hStmt,  2, SQL_C_ULONG, &t, 0, &cbValue);			ch->LastLoan		= t;	
			//retCode = SQLGetData(hStmt,  4, SQL_C_ULONG, &t, 0, &cbValue);			ch->LastLoan_time	= t;
			
			//"nut1, nut2, nut3, killmon, killanimal, killpc, ", 
			retCode = SQLGetData(hStmt,  3, SQL_C_ULONG, &t, 0, &cbValue);			ch->nk[ N_VYSEUS] = (short )t;	
			retCode = SQLGetData(hStmt,  4, SQL_C_ULONG, &t, 0, &cbValue);			ch->nk[ N_ZYPERN] = (short )t;	
			retCode = SQLGetData(hStmt,  5, SQL_C_ULONG, &t, 0, &cbValue);			ch->nk[ N_YILSE]  = (short )t;	
			retCode = SQLGetData(hStmt,  6, SQL_C_ULONG, &t, 0, &cbValue);			ch->killmon 		= t;	
			retCode = SQLGetData(hStmt,  7, SQL_C_ULONG, &t, 0, &cbValue);			ch->killanimal		= t;	
			retCode = SQLGetData(hStmt,  8, SQL_C_ULONG, &t, 0, &cbValue);			ch->killpc			= t;	
			//"reserved_point ", 
			retCode = SQLGetData(hStmt,  9, SQL_C_ULONG, &t, 0, &cbValue);			ch->reserved_point	= (short )t;	
			// 
			retCode = SQLGetData(hStmt, 10, SQL_C_ULONG, &t, 0, &cbValue);			ch->Tactics			= (unsigned char )t;	
			
			//"win_defeat, LadderScore  ",
			retCode = SQLGetData(hStmt, 11, SQL_C_ULONG, &t, 0, &cbValue);			memcpy( &ch->WinLoseScore, &t, sizeof( DWORD ) );
			retCode = SQLGetData(hStmt, 12, SQL_C_ULONG, &t, 0, &cbValue);			ch->LadderScore	= t;
			
			
			SQLFreeStmt(hStmt, SQL_DROP);
			
			//---------------------------------- //DB���� Data�� �����µ� ����Ұ���?������ ����Ѵ�? 
			//			ch->bAlive	= ALIVE_;
			//			ch->Condition = 0x00;
			//			if( ch->Hp <= 0 ) ch->Hp = 1;
			//-----------------------------------
			
			return(1);
		}
		else
		{
			SQLFreeStmt(hStmt, SQL_DROP);		// 0308 YGI
			return(-2);
		}
	}
	SQLFreeStmt(hStmt, SQL_DROP);		// 0308 YGI
	return(-1);
};


// �������?�ʴ´�. 0309 KHS 
int GetNameCount_SQL(LPSTR szUID)
{		
	HSTMT hStmt = NULL;
	RETCODE retCode;
	char szQuerry[255];
	int  Num;
	
	Num = 0;
	
	//sprintf(szQuerry, "select name from chr_info where login_id='%s'", szUID);
	/////////////////////////////////////////////////////////////////////////////
	sprintf(szQuerry, "CALL up_get_char_reg_name( '%s')", szUID);
	/////////////////////////////////////////////////////////////////////////////
	
	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{
		retCode = SQLFetch(hStmt);
		
		while(retCode == SQL_SUCCESS)
		{
			Num ++;
			
			retCode = SQLFetch(hStmt);
		}
		
		SQLFreeStmt(hStmt, SQL_DROP);
	}
	SQLFreeStmt(hStmt, SQL_DROP);		// 0308 YGI
	return(Num);
};

namespace {
bool NormalizeCharacterName(const char* rawName, char normalizedName[NM_LENGTH])
{
    if (!rawName) return false;

    // 名称来自固定 20 字节的旧协议字段；只接收带结�?NUL 的最�?19 字节值�?    if (!rawName[0] || !memchr(rawName, '\0', NM_LENGTH)) return false;

    memset(normalizedName, 0, NM_LENGTH);
    strcpy_s(normalizedName, NM_LENGTH, rawName);
    return true;
}

int CharacterNameExistsIn(const char* table, const char* name)
{
    if (!table || !name || !name[0] ||
        strnlen_s(name, NM_LENGTH + 1) > NM_LENGTH)
        return 1;

    char query[128] = {0};
    _snprintf(query, sizeof(query), "SELECT 1 FROM %s WHERE name=? LIMIT 1", table);
    HSTMT statement = NULL;
    if (!SQL_SUCCEEDED(SQLAllocStmt(hDBC, &statement))) return 1;
    RETCODE result = SQLPrepare(statement, (UCHAR*)query, SQL_NTS);
    SQLLEN nameLength = SQL_NTS;
    if (SQL_SUCCEEDED(result))
        result = SQLBindParameter(statement, 1, SQL_PARAM_INPUT, SQL_C_CHAR,
                                  SQL_VARCHAR, NM_LENGTH, 0, (SQLPOINTER)name,
                                  strlen(name) + 1, &nameLength);
    if (SQL_SUCCEEDED(result)) result = SQLExecute(statement);
    if (!SQL_SUCCEEDED(result))
    {
        displaySQLError(statement);
        SQLFreeStmt(statement, SQL_DROP);
        return 1;
    }
    result = SQLFetch(statement);
    SQLFreeStmt(statement, SQL_DROP);
    return SQL_SUCCEEDED(result) ? 1 : 0;
}
} // namespace

int CheckName2_SQL(LPSTR name)
{
    return CharacterNameExistsIn("chr_info2", name);
}

int CheckName_SQL(LPSTR name)
{
    if (CharacterNameExistsIn("chr_info", name)) return 1;
    return CharacterNameExistsIn("chr_info2", name);
}
int CreateChar_SQL(t_connection c[], int cn, t_packet *packet)
{
	HSTMT hStmt = NULL;
	RETCODE retCode;
	char szQuerry[2048];
	//	int Count;
	LPCHARLIST ch = &c[cn].chrlst;
	DWORD total_id;
	
	char normalizedName[NM_LENGTH] = {0};
	if (!NormalizeCharacterName(packet->u.client_create_char.name, normalizedName) ||
		CheckName_SQL(normalizedName) == 1 || 
		SearchStrWord(normalizedName) || 
		SearchStrStr(normalizedName, " ") ||
		IsBlockedId(normalizedName)
		) // �̸��� ������... // 0208 YGI
	{
		return(-100); // Invalid ID	// �̹� �ִ� ĳ����
	}
	memset(packet->u.client_create_char.name, 0, NM_LENGTH);
	memcpy(packet->u.client_create_char.name, normalizedName, strlen(normalizedName));
	
	
	// DB���� �ڵ������ȴ�. 
	//		Count = GetNameCount_SQL(c[cn].id);
	//		if(Count >= 4)
	//			return(-2); // Over
	
	CreateCharacter( c, cn, packet); // ���� ĳ������  ������ġ, �⺻��ġ, Item���� Setting�Ѵ�. NPC_Pattern.cpp
	
	total_id = GetCharID_SQL();
	if( total_id == 0 )  return -1; // Total ID�� �Ҵ����?���߾��?.
	
	ch->total_id = total_id;
	
	DWORD nation=0;// 1004 YGI
	memcpy( &nation, &ch->name_status,  sizeof( DWORD ) );
	ConvertSave(ch->aStepInfo); // CSD-TW-030620

	sprintf(szQuerry, 	
		"insert into chr_info (name, login_id, gender, face, age, spritvalue, total_id,"
		"clothr, clothg, clothb, bodyr, bodyg, bodyb, "
		"tactics, job, spell, str, con, dex, wis, `int`, movep, `char`, reserved_point, "
		"endu, moral, luck, wsps, nation,  x, y, sprno,"
		"race, `class`, hp, hpmax, mana, manamax, hungry, hungrymax, money, " 
		"nut1, nut2, nut3, "
		"acc_equip1, acc_equip2, acc_equip3, acc_equip4, "
		"sight, lev, exp, MapName, `condition`, "
		"resist_poison, resist_stone, resist_magic, resist_fire, resist_ice, resist_elect, "
		"guildname, bankmoney  "
		") values "
		"(?, ?, %d, %d, %d, %d, %d, "
		"%d, %d, %d, %d, %d, %d, "
		"%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, "
		"%d, %d, %d, %d, %d, %d, %d, %d, "
		"%d, %d, %d, %d, %d, %d, %d, %d, %d, " 
		"%d, %d, %d, " 
		"%d, %d, %d, %d, " // accessory..
		"%d, %d, %d, ?, %d, "
		"%d, %d, %d, %d, %d, %d, ?, 0"
		")",
		ch->Gender, 
		ch->Face, 
		ch->Age, 
		ch->nGuildCode, // CSD-030324
		ch->total_id,
		ch->ClothR,	ch->ClothG,	ch->ClothB,		// �� ���� R.G.B
		ch->BodyR,	ch->BodyG,	ch->BodyB,		// �� ���� R.G.B
		//"tactics, job, spell, str, con, dex, wis, int, movep, char, "
		ch->Tactics, 
		ch->Job, 
		ch->Spell, 
		ch->Str,
		ch->Con,
		ch->Dex,
		ch->Wis,
		ch->Int,
		ch->MoveP,
		ch->Char,
		ch->reserved_point,
		//"endu, moral, luck, wsps, nation,  x, y, sprno,"
		ch->Endu,
		ch->Moral,
		ch->Luck,
		ch->wsps,
		nation, // 1004 YGI
		ch->X,
		ch->Y,
		ch->Gender,
		//			"race, class, hp, hpmax, mana, manamax, hungry, hungrymax, money, " 
		ch->Race,
		ch->Class,
		ch->Hp,
		ch->HpMax,
		ch->Mana,
		ch->ManaMax,
		ch->Hungry,
		ch->HungryMax,
		ch->Money,
		//			"nut1, nut2, nut3, "
		ch->nk[ N_VYSEUS],
		ch->nk[ N_ZYPERN],
		ch->nk[ N_YILSE],
		
		//			"acc_equip1, acc_equip2, acc_equip3, acc_equip4, "
		ch->accessory[0],
		ch->accessory[1],
		ch->accessory[2],
		ch->accessory[3],
		//			"sight, lev, exp, MapName"
		ch->Sight,
		ch->Level,
		ch->Exp,
		ch->Condition,
		//			"resis_poison, resis_stone,	resis_magic, resis_fire, resis_ice, resis_elect "
		ch->nPoison,
		ch->nCurse,
		ch->nHoly,
		ch->nFire,
		ch->nIce,
		ch->nElect); // CSD-TW-030620
	// ĳ������ �Ϲ� Data�� chr_info�� �����ϰ� ( 1-1�ܰ� )
	SQLLEN textLengths[4] = {SQL_NTS, SQL_NTS, SQL_NTS, SQL_NTS};
	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLPrepare(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	if (SQL_SUCCEEDED(retCode))
		retCode = SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR,
			SQL_VARCHAR, NM_LENGTH, 0, ch->Name, strlen(ch->Name) + 1,
			&textLengths[0]);
	if (SQL_SUCCEEDED(retCode))
		retCode = SQLBindParameter(hStmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR,
			SQL_VARCHAR, ID_LENGTH, 0, c[cn].id, strlen(c[cn].id) + 1,
			&textLengths[1]);
	if (SQL_SUCCEEDED(retCode))
		retCode = SQLBindParameter(hStmt, 3, SQL_PARAM_INPUT, SQL_C_CHAR,
			SQL_VARCHAR, 20, 0, ch->MapName, strlen(ch->MapName) + 1,
			&textLengths[2]);
	if (SQL_SUCCEEDED(retCode))
		retCode = SQLBindParameter(hStmt, 4, SQL_PARAM_INPUT, SQL_C_CHAR,
			SQL_VARCHAR, 40, 0, ch->aStepInfo, strlen(ch->aStepInfo) + 1,
			&textLengths[3]);
	if (SQL_SUCCEEDED(retCode)) retCode = SQLExecute(hStmt);
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{
		//			SQLTransact(SQL_NULL_HENV, hDBC, SQL_COMMIT);
		SQLFreeStmt(hStmt, SQL_DROP);
		
		// ĳ������ Binary Data�� chr_info�� �����ϰ� ( 1-2�ܰ� )
		// 0218 YGI #######################
		//			EndsetPartyMenberForUpdate( ch );		// ��Ƽ���� �̸� ����	// 0219 Create������ �ʿ� ����.
		// ĳ������ Binary Data�� chr_info�� �����ϰ� ( 1-2�ܰ� )
		int ret = update_BinaryData_to_Chr_Info
			(	(UCHAR *)ch->Ws,   
			(UCHAR *)ch->Ps,     
			(UCHAR *)ch->Skill,  
			(UCHAR *)ch->skillexp,
			(UCHAR *)ch->tac_skillEXP,
			(UCHAR *)var[ cn], 						// 1219 YGI
			(UCHAR *)ch->inv,  
			(UCHAR *)ch->equip, 
			(UCHAR *)ch->quick, 
			(UCHAR *)ch->party_str, 
			(UCHAR *)ch->relation_str, 
			(UCHAR *)ch->employment_str, 
			(UCHAR *)ch->Item, 
			c[cn].id, ch->Name );
		//####################################
		if( ret == 1 ) 
		{
			// ĳ������ �Ϲ� Data�� chr_info2�� �����ϰ� ( 2-1�ܰ� )
			ret = CreateChar2_SQL(c, cn);
			if( ret == 1 ) // ����..
			{
				// ĳ������ �Ϲ� Data�� chr_info2�� �����ϰ� ( 2-1�ܰ� ) -- Character���� ��.
				ret = update_BinaryData_to_Chr_Info2(  (UCHAR *)ch->bank, c[cn].id, ch->Name );
				
				if( ret == 1 ) 
				{
					
					return 1; // ����...
				}
				return ret; // ����.
			}
			else  return ret; // ����.
		}
		else   return ret; // ����.
	}
	else
	{
		MyLog(LOG_FATAL, "[CREATE_CHAR] Failed to insert base character record");
		displaySQLError(hStmt, szQuerry);
		FILE *fp;
		fp = fopen( "QuerryError.txt","at+" );
		if( fp )
		{
			fprintf( fp, szQuerry );
			fclose(fp);
		}
		
		//			SQLTransact(SQL_NULL_HENV, hDBC, SQL_ROLLBACK);
		SQLFreeStmt(hStmt, SQL_DROP);
		return(-3); // unknown error
	}
	
	return 0;
};	




int delete_char_create_fail( LPSTR name )		// 0411_2 YGI		// �̰� �߸� ������ ���?�����ϴ� �Լ�
{	
	if (!name || !name[0] || strnlen_s(name, NM_LENGTH + 1) > NM_LENGTH)
		return 0;
	const char* queries[] = {
		"DELETE FROM chr_info WHERE name=?",
		"DELETE FROM chr_info2 WHERE name=?"
	};
	bool succeeded = true;
	for (int index = 0; index < 2; ++index)
	{
		HSTMT statement = NULL;
		SQLLEN nameLength = SQL_NTS;
		RETCODE result = SQLAllocStmt(hDBC, &statement);
		if (SQL_SUCCEEDED(result))
			result = SQLPrepare(statement, (UCHAR*)queries[index], SQL_NTS);
		if (SQL_SUCCEEDED(result))
			result = SQLBindParameter(statement, 1, SQL_PARAM_INPUT,
				SQL_C_CHAR, SQL_VARCHAR, NM_LENGTH, 0, name,
				strlen(name) + 1, &nameLength);
		if (SQL_SUCCEEDED(result)) result = SQLExecute(statement);
		if (!SQL_SUCCEEDED(result))
		{
			displaySQLError(statement);
			succeeded = false;
		}
		if (statement) SQLFreeStmt(statement, SQL_DROP);
	}
	return succeeded ? 1 : 0;
}	


/*****************************************************************************************
CreateChar_SQL()��  DB�� Chr_info��  Table�� ���ο� Character������ Insert�ϰ� 
CreateChar2_SQL()�� DB�� Chr_info2 Table�� Character������ Insert�Ѵ�.
Insert�Ǵ� field�� 

  name, login_id..
		
***********************************************************************************************/
int CreateChar2_SQL(t_connection c[], int cn)
{
    LPCHARLIST character = &c[cn].chrlst;
    if (!character->Name[0] || !c[cn].id[0]) return -20;

    HSTMT statement = NULL;
    SQLLEN nameLength = SQL_NTS;
    SQLLEN idLength = SQL_NTS;
    if (!SQL_SUCCEEDED(SQLAllocStmt(hDBC, &statement))) return -20;

    RETCODE result = SQLPrepare(statement,
        (UCHAR*)"INSERT INTO chr_info2 (name, login_id, `new`) VALUES (?, ?, 0)",
        SQL_NTS);
    if (SQL_SUCCEEDED(result))
        result = SQLBindParameter(statement, 1, SQL_PARAM_INPUT, SQL_C_CHAR,
                                  SQL_VARCHAR, NM_LENGTH, 0, character->Name,
                                  strlen(character->Name) + 1, &nameLength);
    if (SQL_SUCCEEDED(result))
        result = SQLBindParameter(statement, 2, SQL_PARAM_INPUT, SQL_C_CHAR,
                                  SQL_VARCHAR, ID_LENGTH, 0, c[cn].id,
                                  strlen(c[cn].id) + 1, &idLength);
    if (SQL_SUCCEEDED(result)) result = SQLExecute(statement);
    if (!SQL_SUCCEEDED(result))
    {
        displaySQLError(statement);
        SQLFreeStmt(statement, SQL_DROP);
        return -20;
    }
    SQLFreeStmt(statement, SQL_DROP);

    char names[4][NM_LENGTH] = {{0,},};
    if (GetCharacterNameInID(c[cn].id, names) != 1) return -30;

    int freeSlot = -1;
    for (int slot = 0; slot < 4; ++slot)
    {
        if (!names[slot][0])
        {
            freeSlot = slot;
            break;
        }
    }
    if (freeSlot < 0) return -2;

    char query[160] = {0};
    _snprintf(query, sizeof(query),
              "UPDATE chr_log_info SET name%d=? WHERE login_id=? "
              "AND (name%d IS NULL OR name%d='')",
              freeSlot + 1, freeSlot + 1, freeSlot + 1);
    statement = NULL;
    nameLength = SQL_NTS;
    idLength = SQL_NTS;
    if (!SQL_SUCCEEDED(SQLAllocStmt(hDBC, &statement))) return -10;
    result = SQLPrepare(statement, (UCHAR*)query, SQL_NTS);
    if (SQL_SUCCEEDED(result))
        result = SQLBindParameter(statement, 1, SQL_PARAM_INPUT, SQL_C_CHAR,
                                  SQL_VARCHAR, NM_LENGTH, 0, character->Name,
                                  strlen(character->Name) + 1, &nameLength);
    if (SQL_SUCCEEDED(result))
        result = SQLBindParameter(statement, 2, SQL_PARAM_INPUT, SQL_C_CHAR,
                                  SQL_VARCHAR, ID_LENGTH, 0, c[cn].id,
                                  strlen(c[cn].id) + 1, &idLength);
    if (SQL_SUCCEEDED(result)) result = SQLExecute(statement);

    SQLLEN affected = 0;
    const bool updated = SQL_SUCCEEDED(result) &&
                         SQL_SUCCEEDED(SQLRowCount(statement, &affected)) &&
                         affected == 1;
    if (!updated) displaySQLError(statement);
    SQLFreeStmt(statement, SQL_DROP);
    return updated ? 1 : -10;
}





int DeleteChar_SQL2( const char *szID, const char *szName, const char* szSecretKeyCode)		// 0302 YGI//020725 lsw
{
	HSTMT hStmt = NULL;
	RETCODE retCode;
	SQLLEN cbValue ;
	char szQuerry[255];
	char log_id[20] = {0, };
	char ori_log_id[20];
	char select_name[20];
	
	const int len = strlen( szName );
	if( len > 20 )
	{
		MyLog( LOG_NORMAL, "Error!! : Failed Character Deleted Name is Very Long ==> '%s'", szName);
		return 0;
	}
	// login_id�� ã��		
	//sprintf(szQuerry, "select login_id from chr_info where name='%s'", szName);		
	/////////////////////////////////////////////////////////////////////////////
	sprintf(szQuerry, "CALL up_get_login_id( '%s')", szName);
	/////////////////////////////////////////////////////////////////////////////
	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{				
		retCode = SQLFetch(hStmt);
		if(retCode == SQL_SUCCESS)
		{			
			retCode= SQLGetData(hStmt, 1, SQL_C_CHAR, log_id, 20, &cbValue) ;
		}			
	}				
	SQLFreeStmt(hStmt, SQL_DROP);
	
	// 010414 KHS
	if( !*log_id ) return 0;
	strcpy( ori_log_id, szID );
	EatRearWhiteChar(ori_log_id);	CharUpper( ori_log_id );
	EatRearWhiteChar(    log_id);	CharUpper(     log_id );
	if( strcmp( log_id, ori_log_id ) ) 
	{
		RecvHackingUser( log_id, szName, 20009, " ","Not his Char(Delete)" );	
		return 0;
	}
	// login_id�� �̿��Ͽ� 4���� �̸��� �о� �´�.
	//sprintf(szQuerry, "select name1, name2, name3, name4 from chr_log_info where login_id='%s'", log_id);		
	/////////////////////////////////////////////////////////////////////////////
	sprintf(szQuerry, "CALL up_get_char_all_name( '%s')", log_id);
	/////////////////////////////////////////////////////////////////////////////
	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{				
		retCode = SQLFetch(hStmt);
		if(retCode == SQL_SUCCESS)
		{			
			for( int i=0; i<4; i++ )
			{		
				retCode= SQLGetData(hStmt, i+1, SQL_C_CHAR, select_name, sizeof(char[20]), &cbValue) ;
				if( retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO )
				{	
					// 010414 KHS
					EatRearWhiteChar( select_name );
					
					if( !strcmp( select_name, szName ) ) //�̸��� �����Ƿ�
					{
						SQLFreeStmt(hStmt, SQL_DROP);//������ �� ���̴�
						if( IsSecretKeyCodeOk(log_id,szSecretKeyCode))
						{
							sprintf(szQuerry, "UPDATE chr_log_info SET name%d = NULL WHERE login_id = '%s' ", i+1, log_id );	// ���� �̸��� ���� ���?�����?
							SQLAllocStmt(hDBC, &hStmt);
							retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
							if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
							{
								//retCode = SQLFetch(hStmt);
								SQLFreeStmt(hStmt, SQL_DROP);
								return 1;
							}
							else
							{
								SQLFreeStmt(hStmt, SQL_DROP);		// 0308 YGI
								return 0;
							}
						}
						else
						{//Ű�ڵ尡 Ʋ�ȱ� �����̴� Ű�ڵ� üũ �Ҷ� SQL�� ���?Free������ ���� ���� ���� �ʰ� ���� �Ѵ�
							return 0;
						}
					}
				}	
			}		
		}			
	}
	SQLFreeStmt(hStmt, SQL_DROP);		// 0308 YGI
	return 0;
}


//  �������?�ϴ� Character Data�� �ű��? 
int MoveDeleteChar_SQL( const char *szName)//020725 lsw
{	
	HSTMT hStmt = NULL;
	RETCODE retCode;
	char szQuerry[255];
	
	DeleteBoxByDeleteCharacter( szName );		// 0817 YGI		// ������ �ִ� �ڽ��� �����?
	
	sprintf(szQuerry, "INSERT INTO Deleted_Chr_Info SELECT * FROM chr_info WHERE name='%s'", szName );
	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{
		retCode = SQLFetch(hStmt);
		SQLFreeStmt(hStmt, SQL_DROP);
	}
	else 
	{
		SQLFreeStmt(hStmt, SQL_DROP);		// 0308 YGI
		return -1;
	}
	
	sprintf(szQuerry, "INSERT INTO Deleted_Chr_Info2 SELECT * FROM chr_info2 WHERE name='%s'", szName );
	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{
		retCode = SQLFetch(hStmt);
		SQLFreeStmt(hStmt, SQL_DROP);
		return TRUE;
	}
	
	SQLFreeStmt(hStmt, SQL_DROP);		// 0308 YGI
	return -1;
}	

int DeleteChar_SQL( const char *szID, const char *szName ,const char *szSecretKeyCode)//020725 lsw
{	
	HSTMT hStmt = NULL;
	RETCODE retCode;
	char szQuerry[255];
	
	if( !DeleteChar_SQL2( szID, szName, szSecretKeyCode) ) return 0;		// 0302 YGI
	
	MoveDeleteChar_SQL( szName );
	SubtractFaith( szName );			// �����?���� �ڱ⸦ ������ �����?�žӽ��� ��´�?		// YGI 0405
	DeleteBoxByDeleteCharacter( szName );		// 0817 YGI		// ������ �ִ� �ڽ��� �����?
	
	// 021008 YGI		// ���?����Ʈ�� �ִ� ĳ���͸� �����?
	ChangeGuildMemberList( (char *)szName, 0, 0 );
	
	// 011214 YGI
	sprintf(szQuerry, "DELETE FROM friend_list WHERE name='%s'", szName );
	Querry_SQL( szQuerry );
	
	sprintf(szQuerry, "DELETE FROM chr_info WHERE name='%s'", szName );
	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{
		SQLFreeStmt(hStmt, SQL_DROP);
	}
	else 
	{
		SQLFreeStmt(hStmt, SQL_DROP);
	}
	
    sprintf(szQuerry, "DELETE FROM chr_info2 WHERE name='%s'", szName );
	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{
		SQLFreeStmt(hStmt, SQL_DROP);
		return TRUE;
	}
	
	SQLFreeStmt(hStmt, SQL_DROP);
	return -1;
}	



/////////////////////////////////////////////////////////////////////////////
//
// �Լ�����: ĳ������ binary(ũ�Ⱑ ū�迭���� binary�� �����ϰ� binary��
//				�о�´�?�� �о����?�Լ�.
//
// �Ķ����?	ws; ĳ���� ������ WS[200]�迭�� **, ps; PS[200]�迭�� **
//				inv; inv[3][3][8]�迭�� **
//				equip, quick; �ش� �迭�� **
//				(����: inv, equip, quick�� ItemAttr Structure�� �迭�̴�)
//
// ���ϰ�: update�� ���� ����; ����(1), ����(ret < 0)
//
/////////////////////////////////////////////////////////////////////////////

int get_BinaryData_from_Chr_Info(UCHAR *ws, UCHAR *ps, UCHAR *skill, UCHAR *skill_exp,  UCHAR *tac_skillEXP, UCHAR *script_var, UCHAR *inv, UCHAR *equip, UCHAR *quick, 
								 UCHAR *party,
								 UCHAR *relation,
								 UCHAR *employment,
								 UCHAR *itemindex,
								 char *login_id, char *name)
{
	SQLLEN  cbvalue = SQL_C_BINARY ;
	
	//BYTE tt[ 1152];
	
	SQLRETURN retcode;
	SQLHSTMT  hstmt;
	/* Prepare the SQL statement with parameter markers. */
	
	char query[256]= {0, } ;
	
	//sprintf(query, "SELECT ws, ps, inventory, FROM chr_info where login_id='%s' and name='%s' ",
	sprintf(query, "SELECT ws, ps, skill, skill_exp, tac_skillEXP, script_var, inventory, equip, quick, party, relation, employment, itemindex FROM chr_info where name='%s' ", name) ;
	
	//printf("\nSQL: %s", query) ;
	SQLAllocStmt(hDBC, &hstmt);
	retcode = SQLExecDirect(hstmt, (UCHAR *)query, SQL_NTS);
	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
		retcode = SQLFetch(hstmt);
		if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
			retcode= SQLGetData(hstmt, 1, SQL_C_BINARY, ws,			SIZE_OF_WS,				&cbvalue);
			retcode= SQLGetData(hstmt, 2, SQL_C_BINARY, ps,			SIZE_OF_PS,				&cbvalue);
			retcode= SQLGetData(hstmt, 3, SQL_C_BINARY, skill,			SIZE_OF_SKILL,			&cbvalue);
			retcode= SQLGetData(hstmt, 4, SQL_C_BINARY, skill_exp,		SIZE_OF_SKILL_EXP,		&cbvalue);
			retcode= SQLGetData(hstmt, 5, SQL_C_BINARY, tac_skillEXP,	SIZE_OF_TAC_SKILL_EXP,	&cbvalue);
			retcode= SQLGetData(hstmt, 6, SQL_C_BINARY, script_var,	SIZE_OF_SCRIPT_VAR,		&cbvalue);
			retcode= SQLGetData(hstmt, 7, SQL_C_BINARY, inv,			SIZE_OF_INV,			&cbvalue);
			retcode= SQLGetData(hstmt, 8, SQL_C_BINARY, equip,			SIZE_OF_EQUIP,			&cbvalue);
			retcode= SQLGetData(hstmt, 9, SQL_C_BINARY, quick,			SIZE_OF_QUICK,			&cbvalue);
			retcode= SQLGetData(hstmt,10, SQL_C_BINARY, party,			SIZE_OF_PARTY,			&cbvalue);
			retcode= SQLGetData(hstmt,11, SQL_C_BINARY, relation,		SIZE_OF_RELATION,		&cbvalue);
			retcode= SQLGetData(hstmt,12, SQL_C_BINARY, employment,	SIZE_OF_EMPLOYMENT,		&cbvalue);
			retcode= SQLGetData(hstmt,13, SQL_C_BINARY, itemindex,		SIZE_OF_ITEMINDEX,		&cbvalue);

			

//<soto-030428
			ItemAttr* Quick = (ItemAttr*)quick;
			if( Quick[5].item_no > 23 || Quick[5].item_no < 1 || Quick[5].attr[0] > 5 || Quick[5].attr[0] < 1)
			{
				memset( &(Quick[5]),0,sizeof(ItemAttr));
			}
//>soto-030428

		}
		else 
		{
			//printf("\n Fetch Error") ;
			SQLFreeStmt(hstmt, SQL_DROP);
			return -1 ;
		}
	}
	else {
		//printf("\n ExecDirect Error") ;
		SQLFreeStmt(hstmt, SQL_DROP);
		return -2 ;
	}
	
	
	if(retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
	{
		//		SQLTransact(SQL_NULL_HENV, hDBC, SQL_COMMIT);
		SQLFreeStmt(hstmt, SQL_DROP);
		
		
		return(1); // succeed
	}
	else
	{
		displaySQLError(hstmt) ;
		//		SQLTransact(SQL_NULL_HENV, hDBC, SQL_ROLLBACK);
		SQLFreeStmt(hstmt, SQL_DROP);
		return(-3); // unknown error
	}
	
}	



/////////////////////////////////////////////////////////////////////////////
//	
// �Լ�����: ĳ������ binary(ũ�Ⱑ ū�迭���� binary�� �����ϰ� binary���о�´�?�� �о����?�Լ�.
//	
//			---Bank�� ������ Item�����?�����´�.  ---
//			 ���� LogIn Server���� Client���� ������ �ʿ����?Data���?LogIn Server������ ȣ�����ʿ� ����. 
//	
// �Ķ����?	ItemAttr bankiem[5][3][6]
//				(����:  ItemAttr Structure�� �迭�̴�)
//	
// ���ϰ�:      ����(1), ����(ret < 0)
//	
/////////////////////////////////////////////////////////////////////////////


//-------------------------------------------------------------------------------------------------
int get_BinaryData_from_Chr_Info2(UCHAR *bankitem,	 char *login_id, char *name)
{	
	SQLLEN  cbvalue = SQL_C_BINARY ;
	SQLRETURN retcode;
	SQLHSTMT  hstmt;
	/* Prepare the SQL statement with parameter markers. */
	char query[256]= {0, } ;
	
	sprintf(query, "SELECT bankitem FROM chr_info2 where name='%s' ", name) ;
	
	//printf("\nSQL: %s", query) ;
	SQLAllocStmt(hDBC, &hstmt);
	retcode = SQLExecDirect(hstmt, (UCHAR *)query, SQL_NTS);
	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
		retcode = SQLFetch(hstmt);
		if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
			
			retcode= SQLGetData(hstmt, 1, SQL_C_BINARY, bankitem, SIZE_OF_BANKITEM, &cbvalue);
			
		}
		else 
		{
			//printf("\n Fetch Error") ;
			SQLFreeStmt(hstmt, SQL_DROP);
			return -1 ;
		}
	}
	else {
		//printf("\n ExecDirect Error") ;
		SQLFreeStmt(hstmt, SQL_DROP);
		return -2 ;
	}
	
	
	if(retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
	{
		//		SQLTransact(SQL_NULL_HENV, hDBC, SQL_COMMIT);
		SQLFreeStmt(hstmt, SQL_DROP);
		
		return(1); // succeed
	}
	else
	{
		displaySQLError(hstmt) ;
		//		SQLTransact(SQL_NULL_HENV, hDBC, SQL_ROLLBACK);
		SQLFreeStmt(hstmt, SQL_DROP);
		return(-3); // unknown error
	}
}






/////////////////////////////////////////////////////////////////////////////
//
// �Լ�����: ĳ������ binary(ũ�Ⱑ ū�迭���� binary�� �����ϰ� binary��
//				�о�´�?�� ����(update)�ϴ� �Լ�.
//
// �Ķ����?	ws; ĳ���� ������ WS[200]�迭�� *, ps; PS[200]�迭�� *
//				inv; inv[3][3][8]�迭�� *
//				equip, quick; �ش� �迭�� *
//				(����: inv, equip, quick�� ItemAttr Structure�� �迭�̴�)
//
// ���ϰ�: update�� ���� ����; ����(1), ����(ret < 0)
//
/////////////////////////////////////////////////////////////////////////////

namespace {
struct BinaryUpdateParameter
{
    UCHAR* data;
    SQLLEN size;
};

int ExecuteCharacterBinaryUpdate(const char* query,
                                 BinaryUpdateParameter* parameters,
                                 int parameterCount,
                                 const char* name)
{
    if (!query || !parameters || parameterCount <= 0 || parameterCount > 15 ||
        !name || !name[0] || strnlen_s(name, NM_LENGTH + 1) > NM_LENGTH)
        return -5;

    HSTMT statement = NULL;
    if (!SQL_SUCCEEDED(SQLAllocStmt(hDBC, &statement))) return -5;
    RETCODE result = SQLPrepare(statement, (UCHAR*)query, SQL_NTS);
    if (!SQL_SUCCEEDED(result))
    {
        displaySQLError(statement);
        SQLFreeStmt(statement, SQL_DROP);
        return -5;
    }

    SQLLEN lengths[16] = {0};
    for (int index = 0; index < parameterCount; ++index)
    {
        lengths[index] = parameters[index].size;
        result = SQLBindParameter(statement, index + 1, SQL_PARAM_INPUT,
                                  SQL_C_BINARY, SQL_LONGVARBINARY,
                                  parameters[index].size, 0,
                                  parameters[index].data,
                                  parameters[index].size, &lengths[index]);
        if (!SQL_SUCCEEDED(result))
        {
            displaySQLError(statement);
            SQLFreeStmt(statement, SQL_DROP);
            return -5;
        }
    }

    SQLLEN nameLength = SQL_NTS;
    result = SQLBindParameter(statement, parameterCount + 1, SQL_PARAM_INPUT,
                              SQL_C_CHAR, SQL_VARCHAR, NM_LENGTH, 0,
                              (SQLPOINTER)name, strlen(name) + 1, &nameLength);
    if (!SQL_SUCCEEDED(result))
    {
        displaySQLError(statement);
        SQLFreeStmt(statement, SQL_DROP);
        return -5;
    }

    result = SQLExecute(statement);
    SQLLEN affected = 0;
    if (!SQL_SUCCEEDED(result) ||
        !SQL_SUCCEEDED(SQLRowCount(statement, &affected)) || affected != 1)
    {
        displaySQLError(statement);
        SQLFreeStmt(statement, SQL_DROP);
        return -3;
    }
    SQLFreeStmt(statement, SQL_DROP);
    return 1;
}
} // namespace

int update_BinaryData_to_Chr_Info(
    UCHAR* ws, UCHAR* ps, UCHAR* skill, UCHAR* skill_exp,
    UCHAR* tac_skillEXP, UCHAR* script_var, UCHAR* inv, UCHAR* equip,
    UCHAR* quick, UCHAR* party, UCHAR* relation, UCHAR* employment,
    UCHAR* itemindex, char* login_id, char* name)
{
    (void)login_id;
    BinaryUpdateParameter parameters[] = {
        {ws, SIZE_OF_WS},
        {ps, SIZE_OF_PS},
        {skill, SIZE_OF_SKILL},
        {skill_exp, SIZE_OF_SKILL_EXP},
        {tac_skillEXP, SIZE_OF_TAC_SKILL_EXP},
        {script_var, SIZE_OF_SCRIPT_VAR},
        {inv, SIZE_OF_INV},
        {equip, SIZE_OF_EQUIP},
        {quick, SIZE_OF_QUICK},
        {party, SIZE_OF_PARTY},
        {relation, SIZE_OF_RELATION},
        {employment, SIZE_OF_EMPLOYMENT},
        {itemindex, SIZE_OF_ITEMINDEX}
    };
    return ExecuteCharacterBinaryUpdate(
        "UPDATE chr_info SET ws=?, ps=?, skill=?, skill_exp=?, "
        "tac_skillEXP=?, script_var=?, inventory=?, equip=?, quick=?, "
        "party=?, relation=?, employment=?, itemindex=? WHERE name=?",
        parameters, sizeof(parameters) / sizeof(parameters[0]), name);
}

int update_BinaryData_to_Chr_Info2(
    UCHAR* bankitem, char* login_id, char* name)
{
    (void)login_id;
    BinaryUpdateParameter parameters[] = {
        {bankitem, SIZE_OF_BANKITEM}
    };
    return ExecuteCharacterBinaryUpdate(
        "UPDATE chr_info2 SET bankitem=? WHERE name=?",
        parameters, sizeof(parameters) / sizeof(parameters[0]), name);
}





int GetMapName_SQL(t_connection c[], int cn)
{
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	SQLLEN		cbValue;
	char		szQuerry[1024];
	
	//sprintf(szQuerry, "Select mapname From chr_info Where name='%s'", c[cn].name);
	/////////////////////////////////////////////////////////////////////////////
	sprintf(szQuerry, "CALL up_get_map_name( '%s')", c[cn].name);
	////////////////////////////////////////////////////////////////////////////
	//	SQLAllocStmt(hDBC_TotalDB, &hStmt);  // 001211 KHS  
	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{
		retCode = SQLFetch(hStmt);
		
		if(retCode == SQL_SUCCESS)
		{
			retCode = SQLGetData(hStmt, 1, SQL_C_CHAR, c[cn].mapname, NM_LENGTH, &cbValue);
			EatRearWhiteChar( c[cn].mapname );
			SQLFreeStmt(hStmt, SQL_DROP);
			return(1);
		}
		else
		{
			SQLFreeStmt(hStmt, SQL_DROP);
			return(-2);
		}
	}
	
	SQLFreeStmt(hStmt, SQL_DROP);
	return(-1);
};




int GetConnectInfo_SQL(t_connection c[], int cn, t_packet *packet, char *mapname)
{
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	SQLLEN		cbValue;
	char		szQuerry[1024];
	char		IP[3][20] = {0,};
	int			PORT;
	int			cc = 0;
	char		tempmapname[ FILENAME_MAX];
	
	if(GetMapName_SQL(c, cn) == 1)
	{
AGAIN_:
	if( mapname == NULL )	strcpy( tempmapname, c[ cn].mapname );	// ���� �ִ� �ڸ��� ����
	else					strcpy( tempmapname, mapname );
	
	//sprintf(szQuerry, "Select IP0, IP1, IP2, port From MapName Where mapname='%s'", tempmapname ); //startmapposition[startmap][startposition] );//	c[cn].mapname);
	/////////////////////////////////////////////////////////////////////////////
	sprintf(szQuerry, "CALL up_get_connect_info( '%s')", tempmapname);
	/////////////////////////////////////////////////////////////////////////////
	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{
		retCode = SQLFetch(hStmt);
		
		if(retCode == SQL_SUCCESS)
		{
			retCode = SQLGetData(hStmt, 1, SQL_C_CHAR, IP[0], 18, &cbValue);
			retCode = SQLGetData(hStmt, 2, SQL_C_CHAR, IP[1], 18, &cbValue);
			retCode = SQLGetData(hStmt, 3, SQL_C_CHAR, IP[2], 18, &cbValue);
			retCode = SQLGetData(hStmt, 4, SQL_C_ULONG, &PORT, 0, &cbValue);
			
			memcpy(&packet->u.server_connect_info.ip[0], IP[0], 18);
			memcpy(&packet->u.server_connect_info.ip[1], IP[1], 18);
			memcpy(&packet->u.server_connect_info.ip[2], IP[2], 18);
			packet->u.server_connect_info.port = PORT;
			
			SQLFreeStmt(hStmt, SQL_DROP);
			return(1);
		}
		else
		{
			SQLFreeStmt(hStmt, SQL_DROP);
			if( cc == 0 ) strcpy( tempmapname, "ma-in" );
			cc++;
			if( cc > 1 ) return(-2);
			else goto AGAIN_;
		}
	}
	else
	{
		SQLFreeStmt(hStmt, SQL_DROP);
		return(-1);
	}
	}
	
	SQLFreeStmt(hStmt, SQL_DROP);
	return(-1);
};

int GetChangeMapInfo_SQL(t_connection c[], int cn, t_packet *packet)
{
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	SQLLEN		cbValue;
	char		szQuerry[1024];
	char		IP[3][18];
	int			PORT;
	
	//sprintf(szQuerry, "Select IP0, IP1, IP2, port  From MapName Where mapname='%s'", c[cn].mapname);
	/////////////////////////////////////////////////////////////////////////////
	sprintf(szQuerry, "CALL up_get_connect_info( '%s')", c[cn].mapname);
	/////////////////////////////////////////////////////////////////////////////
	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{
		retCode = SQLFetch(hStmt);
		
		if(retCode == SQL_SUCCESS)
		{
			retCode = SQLGetData(hStmt, 1, SQL_C_CHAR, IP[0], 18, &cbValue);
			retCode = SQLGetData(hStmt, 2, SQL_C_CHAR, IP[1], 18, &cbValue);
			retCode = SQLGetData(hStmt, 3, SQL_C_CHAR, IP[2], 18, &cbValue);
			retCode = SQLGetData(hStmt, 4, SQL_C_ULONG, &PORT, 0, &cbValue);
			
			memcpy(&packet->u.server_change_map.ip[0], IP[0], 18);
			memcpy(&packet->u.server_change_map.ip[1], IP[1], 18);
			memcpy(&packet->u.server_change_map.ip[2], IP[2], 18);
			packet->u.server_change_map.port = PORT;
			
			SQLFreeStmt(hStmt, SQL_DROP);
			return(1);
		}
		else
		{
			SQLFreeStmt(hStmt, SQL_DROP);
			return(-2);
		}
	}
	SQLFreeStmt(hStmt, SQL_DROP);
	return(-1);
};

int CheckUsedID_SQL( LPSTR szUID, WORD *wPort, DWORD *dwID, DWORD *dwServerSetNum )
{
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	SQLLEN		cbValue;
	int			Len;
	char		szQuerry[255];
	char		ID[ID_LENGTH];
	
	// 0403 ���� ���� LogIn/Out�� ���� �˰����� ������..........
	// ��а�?�������?�ʴ´�...................................
	
    if(szUID == NULL)	return(-2);
	
	Len = strlen(szUID);
	if(Len <= 0)		return(-2);
	
	//		return -1; // �������� �����?�����ϱ� ������ �ϴ� �����Ѵ�.  0420 KHS
	
	//sprintf(szQuerry, "select user_id, port, agent_id, server_set_num from logintable where user_id='%s'", szUID);
	/////////////////////////////////////////////////////////////////////////////
	sprintf(szQuerry, "CALL up_check_user_id( '%s')", szUID);
	/////////////////////////////////////////////////////////////////////////////			
	//1207 zhh
	SQLAllocStmt(hDBC_TotalDB, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{	
		retCode = SQLFetch(hStmt);
		
		if(retCode == SQL_SUCCESS)
		{
			memset(ID, 0, ID_LENGTH);
			
			// Modified by chan78 at 2001/01/31, ���� ����ó���� �����Ƿ� �׳�;;;
			retCode = SQLGetData(hStmt, 1, SQL_C_CHAR, ID, Len+1, &cbValue);
			retCode = SQLGetData(hStmt, 2, SQL_C_USHORT, wPort, 0, NULL );
			retCode = SQLGetData(hStmt, 3, SQL_C_ULONG, dwID, 0, NULL );
			retCode = SQLGetData(hStmt, 4, SQL_C_ULONG, dwServerSetNum, 0, NULL );
			
			SQLFreeStmt(hStmt, SQL_DROP);
			return (1);
			
		}
		else // Modified by chan78 at 2001/02/19
		{
			goto QueryFail;
		}
		
		SQLFreeStmt(hStmt, SQL_DROP);
		return(-1);
	}
	else // Modified by chan78 at 2001/02/19
	{
		goto QueryFail;
	}
	
	SQLFreeStmt(hStmt, SQL_DROP);
	return(-2);
	
QueryFail:
	
	if( retCode == SQL_ERROR )
	{
		MyLog( LOG_FATAL, "<< QUERY FAIL >> :: User '%s's SELECT QUERY FAILED", szUID );	
	}
	SQLFreeStmt(hStmt, SQL_DROP);
	return(-2);
};			

void SendDeleteUsedID( int server_id )
{	
	t_packet p;
	
	p.h.header.type = CMD_REQ_DELETE_USERID;
	{
		strcpy( p.u.gs_req_delete_userid.id,		connections[ server_id].id );
#ifdef _GAME_SERVER_
		strcpy( p.u.gs_req_delete_userid.mapname,	MapName );
#endif
	}
	p.h.header.size = sizeof( t_gs_req_delete_userid );
	
	QueuePacket( connections, DB_DEMON, &p, 1 );
}	



/****************************************************************************************/
// 10�� 4�� �߰�
/****************************************************************************************/
/*
typedef struct tagCHARLIST
{	
BOOL			bAlive;

  char			Name[20];
  
	short int		Level;
	DWORD			Exp;
	
	  short int		Gender;						// 0:���� 1 : ����
	  short int		Face;						
	  short int		SpritValue;					// ��ġ��
	  short int		Class;						// 0:���� 1:�ü� 2:���� 3:������ 4:������
	  short int		\Sprituality;	
	  short int		Class_Poisioning;	
	  short int		Class_Bombplay;		
	  short int		Class_Entrapment;	
	  short int		Class_Scrolling;	
	  short int		Class_Cunning1;	
	  short int		Class_Cunning2;	
	  short int		Class_Cunning3;	
	  short int		Class_Stealing;	
	  short int		Spell;
	  
		short int		Race;						// ����
		
		  //����ĳ���� �����Ҷ��� ��ġ..
		  short int		StrOld;				//  ��
		  short int		ConOld;				//  �ǰ�
		  short int		DexOld;				//	��ø��
		  short int		WisOld;				//	����
		  short int		IntOld;				//	����
		  short int		MovePOld;			//	�̵���
		  short int		CharOld;			//	�ŷ�
		  short int		EnduOld;			//  �����?
		  short int		HealthOld;			//	ü��
		  short int		MoralOld;			//	���?
		  short int		LuckOld;			//	���?
		  
			short int		Str;				//  ��
			short int		Con;				//  �ǰ�
			short int		Dex;				//	��ø��
			short int		Wis;				//	����
			short int		Int;				//	����
			short int		MoveP;				//	�̵���
			short int		Char;				//	�ŷ�
			short int		Endu;				//  �����?
			short int		Health;				//	ü��
			short int		Moral;				//	���?
			short int		Luck;				//	���?
			
			  short int		Job;
			  
				DWORD			Money;
				
				  short int		VitalPower;					// ������
				  short int		VitalMax;
				  short int		Mana;					// ������
				  short int		ManaMax;
				  short int		AttackPower;				// ���ݷ�
				  short int		AttackMax;
				  
					short int		Condition;
					
					  
						char			MapName[20];
						
						  unsigned char	Peacests;
						  short int		Sight;
						  
							short int		BodyR, BodyG, BodyB;		// �� ���� R.G.B
							short int		ClothR, ClothG, ClothB;		// �� ���� R.G.B
							
							  short int		Age;
							  
								short int		Hungry;
								short int		HungryMax;
								short int		Nut1;						// ���簡1
								short int		Nut2;
								short int		Nut3;
								
								  DWORD			Killmon;					// 6�Կ����� ų����
								  DWORD			Killanimal;					// ���� ų����
								  DWORD			KillNPC;					// ���?NPC ų����
								  
									short int		Resis_Poision;		//	��/���� ���׷�
									short int		Resis_Stone;		//	��ȭ ���� ���׷�
									short int		Resis_Magic;		//	�������׷�
									short int		Resis_Fire;			//	�� ���׷�
									short int		Resis_Ice;			//	�������׷�
									short int		Resis_Elect;		//	���������?
									short int		Wizardspell;		//	Ws
									short int		Ws[200];			//	����
									short int		Priestspell;		//	Ps
									short int		Ps[200];			//	�Ź�
									
									  
										// ���� ������ �ִ� ������..
										// �κ��丮
										ItemAttr	inv[3][3][8] ;
										// ����
										ItemAttr	equip[8] ;
										// �� 
										ItemAttr	quick[6] ;
										// �տ� ���?�ִ�
										ItemAttr	handheld ;
										//short int		Item[ 13][100];
										
										  //  Client�� ���� Data
										  
											short int		SprNo;
											short int		SprType;	
											
											  BYTE			accessory[4];
											  DIRECTION		Direction;
											  DWORD			IDWhoImAttacked;	// ���� ģ���� ������..
											  short int		nCurrentAction;
											  short int		Light;
											  
												DWORD			BankMoney;
												
												  short int		X, Y;
												  short int		MovePathCount;
												  short int		MoveLength;
												  unsigned char	MoveType;
												  short int		MoveSx;
												  short int		MoveSy;
												  unsigned char   MoveDirTbl[MAX_DIR_TABLE];
												  short int		MoveGox;
												  short int		MoveGoy;
												  
													
													  //------------------------   ���������� ����ϴ�?para...
													  
														
														  char			patterntype;
														  DWORD			WalkTime;
														  
															// ********************************* �߰��� �׸��?.
															// ��������. 
															short int		Dam_Min, Dam_Max ;
															short int		Ac ;
															short int		Def_Rate ;
															short int		Hit_Rate ;
															short int		Hp ;
															
															  short int		parrying ;
															  
																
																  int				base_hd ;
																  
																	
																	  char			skill[8] ;
																	  
																		// *************************************************
																		
																		  
																			} CHARLIST, *LPCHARLIST;
*/		

int updateCharacterStatus(t_connection c[], int char_id)
{		
	SQLRETURN retcode;
	SQLHSTMT  hstmt;
	CHARLIST *chr= &c[char_id].chrlst;
	char query[2048];
	
	if( chr->Level == 0 ) return	-1;
	if( chr->HpMax == 0 ) return	-1;
	if( chr->ManaMax == 0 ) return	-1;
	
	DWORD	temp_NWCharacter;									
	memcpy(&temp_NWCharacter,&chr->NWCharacter,sizeof(DWORD));
	
	sprintf(query, "UPDATE chr_info SET"
		" lev= %d, spritvalue=%d, social_status = %d, fame=%d, fame_pk=%d "
		" where name= '%s'"
		,chr->Level
		,chr->nGuildCode
		,chr->social_status
		,chr->fame
		,temp_NWCharacter		// 010915 LTS
		,c[char_id].name );
	
	SQLAllocStmt(hDBC, &hstmt);
	retcode = SQLExecDirect(hstmt, (UCHAR *)query, SQL_NTS);
	if(retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO) 
	{	
		displaySQLError(hstmt) ;
		//		SQLTransact(SQL_NULL_HENV, hDBC, SQL_ROLLBACK);
		SQLFreeStmt(hstmt, SQL_DROP);
		return -1 ;
	}	
	else {
		//		SQLTransact(SQL_NULL_HENV, hDBC, SQL_COMMIT);
		SQLFreeStmt(hstmt, SQL_DROP) ;
	}	
	
	DWORD nation;		//1004 YGI
	memcpy( &nation, &chr->name_status, sizeof( DWORD ) );
	
	sprintf(query, "UPDATE chr_info SET"
		" Str= %d, Con= %d, Dex= %d, Wis= %d, "
		" `Int`= %d, MoveP= %d, `Char`= %d, Endu= %d, Moral= %d, Luck= %d, wsps= %d, tactics= %d, nation= %d, Money= %d, Hp= %d, HpMax = %d, "
		" mana= %d, manamax= %d, `condition`= %d, mapname='%s', sight= %d, Age= %d, bAlive= %d "
		" where name= '%s'"
		, chr->Str, chr->Con, chr->Dex, chr->Wis,
		chr->Int, chr->MoveP, chr->Char, chr->Endu, chr->Moral, chr->Luck, chr->wsps, chr->Tactics, nation, chr->Money, chr->Hp, chr->HpMax,		// 1004 YGI
		chr->Mana, chr->ManaMax, chr->Condition, chr->MapName, chr->Sight, chr->Age, chr->bAlive
		, c[char_id].name ) ;
	SQLAllocStmt(hDBC, &hstmt) ;
	retcode = SQLExecDirect(hstmt, (UCHAR *)query, SQL_NTS);
	if(retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO) {
		displaySQLError(hstmt) ;
		//		SQLTransact(SQL_NULL_HENV, hDBC, SQL_ROLLBACK);
		SQLFreeStmt(hstmt, SQL_DROP);
		return -1 ;
	}	
	else {
		//		SQLTransact(SQL_NULL_HENV, hDBC, SQL_COMMIT);
		SQLFreeStmt(hstmt, SQL_DROP) ;
	}	
	
	
	sprintf(query, "UPDATE chr_info SET"
		" hungry= %d, hungrymax= %d, killmon= %d, "
		" killanimal= %d, killpc = %d, resist_poison= %d, resist_stone= %d, resist_magic= %d, resist_fire= %d, "
		" resist_ice= %d, resist_elect= %d, x=%d, y=%d, "
		" acc_equip1= %d, acc_equip2= %d, acc_equip3= %d, acc_equip4= %d "
		" where name= '%s'"
		,chr->Hungry, chr->HungryMax, chr->killmon,
		chr->killanimal, chr->killpc, 
		//< CSD-010907
		chr->nPoison, 
		chr->nCurse, 
		chr->nHoly, 
		chr->nFire,
		chr->nIce, 
		chr->nElect, 
		//> CSD-010907
		chr->X, chr->Y,
		chr->accessory[0], chr->accessory[1], chr->accessory[2], chr->accessory[3], 
		c[char_id].name) ;  
	
	SQLAllocStmt(hDBC, &hstmt) ;
	retcode = SQLExecDirect(hstmt, (UCHAR *)query, SQL_NTS);
	if(retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO) {
		displaySQLError(hstmt) ;
		//		SQLTransact(SQL_NULL_HENV, hDBC, SQL_ROLLBACK);
		SQLFreeStmt(hstmt, SQL_DROP);
		return -1 ;
	}	
	else {
		//		SQLTransact(SQL_NULL_HENV, hDBC, SQL_COMMIT);
		SQLFreeStmt(hstmt, SQL_DROP) ;
	}	
	
	
	// 010406 YGI
	DWORD win_defeat;
	memcpy( &win_defeat, &chr->WinLoseScore, sizeof(DWORD) );
	sprintf(query, "UPDATE chr_info SET"
		" openhouse = %d, reserved_point= %d,"
		" bankmoney = %u, LastLoan= %d, exp = %u,"
		" disease1=%d, disease2=%d, disease3=%d, disease4=%d, disease5=%d, disease6=%d, viewtype= %d,"
		" win_defeat = %d, ladderscore = %d "
		" where name= '%s'",
		
		chr->openhouse,
		chr->reserved_point,
		
		chr->BankMoney,
		chr->LastLoan,
		chr->Exp,
		
		chr->disease[0], 
		chr->disease[1], 
		chr->disease[2], 
		chr->disease[3], 
		chr->disease[4], 
		chr->disease[5], 
		
		chr->viewtype,
		win_defeat, 
		chr->LadderScore,
		
		c[char_id].name
		) ;  
	SQLAllocStmt(hDBC, &hstmt) ;
	retcode = SQLExecDirect(hstmt, (UCHAR *)query, SQL_NTS);
	if(retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO) {
		//printf("\n Update Character: Exec Direct Error ; %s", query) ;
		displaySQLError(hstmt) ;
		//		SQLTransact(SQL_NULL_HENV, hDBC, SQL_ROLLBACK);
		SQLFreeStmt(hstmt, SQL_DROP);
		return -1 ;
	}
	else {
		//		SQLTransact(SQL_NULL_HENV, hDBC, SQL_COMMIT);
		SQLFreeStmt(hstmt, SQL_DROP) ;
	}
	
	
	///////////////////////// 0218 YGI ////////////////////////
	
	EndsetPartyMenberForUpdate( chr );
	
	if( update_BinaryData_to_Chr_Info(	(UCHAR *)chr->Ws, 
		(UCHAR *)chr->Ps, 
		(UCHAR *)chr->Skill, 
		(UCHAR *)chr->skillexp, 
		(UCHAR *)chr->tac_skillEXP,
		(UCHAR *)var[ chr->server_id],
		(UCHAR *)chr->inv, 
		(UCHAR *)chr->equip, 
		(UCHAR *)chr->quick, 
		(UCHAR *)chr->party_str, 
		(UCHAR *)chr->relation_str, 
		(UCHAR *)chr->employment_str, 
		(UCHAR *)chr->Item, 
		c[char_id].id, c[char_id].name) < 0) {		//1219 YGI
		//printf("\n Update Character: Update BIN Error") ;
		return -2 ;
	}
	else 
	{
		if( update_BinaryData_to_Chr_Info2(  (UCHAR *)chr->bank, c[char_id].id, c[char_id].name) < 0) 
			return -2;
	}
	
	return 1 ;
}




void updateCharacterVeryImportantStatusToLoginServer( t_connection c[], int char_id )
{
	t_packet p;
	t_update_very_important_status *tp;
	LPCHARLIST ch = &c[ char_id].chrlst;
	
	
	if( ch->HpMax ==     0 )	return;
	if( ch->HungryMax == 0 )	return;
	if( ch->ManaMax ==   0 )	return;
	if( ch->updatable	== 0 )  return;
	
	p.h.header.type = CMD_UPDATE_VERY_IMPORTANT_STATUS;
	p.h.header.size = sizeof( t_update_very_important_status );
	tp = &p.u.update_very_important_status;
	
	strcpy( tp->name,	c[char_id].name );
	
	tp->Level				= ch->Level;
	
	tp->Str  				= ch->Str;
	tp->Con  				= ch->Con;
	tp->Dex  				= ch->Dex;
	tp->Wis  				= ch->Wis;
	tp->Int  				= ch->Int;
	tp->MoveP				= ch->MoveP;
	tp->Char 				= ch->Char;
	tp->Endu 				= ch->Endu;
	tp->Moral				= ch->Moral;
	tp->Luck 				= ch->Luck;
	tp->wsps 				= ch->wsps;
	
	tp->HpMax				= ch->HpMax ;
	tp->ManaMax				= ch->ManaMax;
	tp->HungryMax			= ch->HungryMax;
	
	tp->reserved_point		= ch->reserved_point;
	
	tp->Exp					= ch->Exp;
	
	QueuePacket( connections, DB_DEMON, &p, 1 );
}

void updateCharacterVeryImportant_TacticsSkillExp_ToLoginServer( t_connection c[], int char_id )
{
	t_packet p;
	t_update_very_important_tactics *tp;
	LPCHARLIST ch = &c[ char_id].chrlst;
	
	if( ch->HpMax ==		0 )	return;
	if( ch->HungryMax ==	0 )	return;
	if( ch->ManaMax ==		0 )	return;
	if( ch->updatable	==	0 ) return;
	
	p.h.header.type = CMD_UPDATE_VERY_IMPORTANT_TACTICS;
	p.h.header.size = sizeof( t_update_very_important_tactics );
	tp = &p.u.update_very_important_tactics;
	
	strcpy( tp->name,	c[char_id].name );
	memcpy( tp->tac_skillEXP, c[ char_id].chrlst.tac_skillEXP, SIZE_OF_TAC_SKILL_EXP );
	
	QueuePacket( connections, DB_DEMON, &p, 1 );
}						

int RecvUpdateCharacterVeryImportantStatus( t_update_very_important_status *tp )
{
	SQLRETURN retcode;
	SQLHSTMT  hstmt;
	char query[1024];
	
	sprintf(query, "UPDATE chr_info SET"
		" lev= %d, Str= %d, Con= %d, Dex= %d, Wis= %d, "
		" `Int`= %d, MoveP= %d, `Char`= %d, Endu= %d, Moral= %d, Luck= %d, wsps= %d, "
		" HpMax = %d, manamax= %d, hungrymax= %d, reserved_point= %d, exp = %u "
		" where name= '%s'",
		tp->Level,	
		
		tp->Str,	
		tp->Con,  	
		tp->Dex,  	
		tp->Wis,  	
		tp->Int,  	
		tp->MoveP,	  
		tp->Char, 	 
		tp->Endu, 	 
		tp->Moral,	  
		tp->Luck, 	 
		tp->wsps, 	 
		
		tp->HpMax, 
		tp->ManaMax,
		tp->HungryMax,
		
		tp->reserved_point,
		
		tp->Exp,
		
		tp->name );
	
	SQLAllocStmt(hDBC, &hstmt) ;
	retcode = SQLExecDirect(hstmt, (UCHAR *)query, SQL_NTS);
	if(retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO) {
		displaySQLError(hstmt) ;
		//		SQLTransact(SQL_NULL_HENV, hDBC, SQL_ROLLBACK);
		SQLFreeStmt(hstmt, SQL_DROP);
		return -1 ;
	}	
	else {
		//		SQLTransact(SQL_NULL_HENV, hDBC, SQL_COMMIT);
		SQLFreeStmt(hstmt, SQL_DROP) ;
	}	
	
	return 1;
	
}






void updateCharacterStatusToLoginServer( t_connection c[], int char_id)
{
	t_packet p;
	t_update_char_db *tp;
	LPCHARLIST ch = &c[ char_id].chrlst;
	
	
	p.h.header.type = CMD_UPDATE_CHAR_DB;
	tp = &p.u.update_char_db;
	
	if( ch->HpMax ==     0 )	return;
	if( ch->HungryMax == 0 )	return;
	if( ch->ManaMax ==   0 )	return;
	if( ch->updatable	== 0 )  return;
	
	strcpy( tp->id,		c[char_id].id );
	strcpy( tp->name,	c[char_id].name );
	
	tp->Level = ch->Level;
	tp->nGuildCode		= ch->nGuildCode; // CSD-030324
	tp->social_status	= ch->social_status;
	tp->fame			= ch->fame;
	tp->NWCharacter		= ch->NWCharacter;	// 010915 LTS
	
	
	tp->Str  				= ch->Str;
	tp->Con  				= ch->Con;
	tp->Dex  				= ch->Dex;
	tp->Wis  				= ch->Wis;
	tp->Int  				= ch->Int;
	tp->MoveP				= ch->MoveP;
	tp->Char 				= ch->Char;
	tp->Endu 				= ch->Endu;
	tp->Moral				= ch->Moral;
	tp->Luck 				= ch->Luck;
	tp->wsps 				= ch->wsps;
	
	tp->Tactics 		= ch->Tactics;				// ������ �������?(�����ֱ� ����)
	memcpy( &tp->nation, &ch->name_status, sizeof( DWORD ) );			// 1004 YGI
	tp->Money			= ch->Money;
	tp->Hp				= ch->Hp;
	tp->HpMax			= ch->HpMax ;
	tp->Mana			= ch->Mana;					// ������
	tp->ManaMax			= ch->ManaMax;
	tp->Condition		= ch->Condition;		// 0����->CON_NORMAL, 1Ȱ����->CON_ACTIVE, 2�����?>CON_SLUGGISH, 3���ο�->CON_SLOW, 4ȥ��->CON_DAZE, 5�ߵ�->CON_POISON, 6��ȭ->CON_STONE, 7����->CON_DEATH 
	
	memcpy( tp->MapName, MapName, 20 );
	tp->Sight = ch->Sight;
	tp->Age = ch->Age;
	tp->bAlive		= ch->bAlive;					// ĳ������ ���翩��(	REMOVE_:0 / ALIVE_:1 / DEAD_:2 / BUFE_:3)
	
	tp->Hungry = ch->Hungry;	
	tp->HungryMax = ch->HungryMax;
	tp->killmon = ch->killmon;						// 1�迭���� ų����
	tp->killanimal = ch->killanimal;				// ���� ų����
	tp->killpc = ch->killpc;						// ���?NPC ų����
	
	//< CSD-010907
	tp->nPoison = ch->nPoison;
	tp->nCurse = ch->nCurse;
	tp->nFire = ch->nFire;
	tp->nIce = ch->nIce;
	tp->nElect = ch->nElect;
	tp->nHoly = ch->nHoly;
	//> CSD-010907
	
	tp->X = ch->X;
	tp->Y = ch->Y;
	
	tp->accessory[0] = ch->accessory[0];
	tp->accessory[1] = ch->accessory[1];
	tp->accessory[2] = ch->accessory[2];
	tp->accessory[3] = ch->accessory[3];
	
	tp->openhouse		= ch->openhouse;
	
	tp->reserved_point  = ch->reserved_point ;
	tp->BankMoney		= ch->BankMoney;
	
	// 010406 YGI
	memcpy( &tp->win_defeat, &ch->WinLoseScore, sizeof( DWORD ) );
	tp->LadderScore = ch->LadderScore;
	//tp->LastTotal		= ch->LastTotal;
	tp->LastLoan		= ch->LastLoan;
	//tp->LastLoan_time	= ch->LastLoan_time;
	
	tp->Exp				= ch->Exp;
	tp->disease[0]		= ch->disease[0];					
	tp->disease[1]		= ch->disease[1];					
	tp->disease[2]		= ch->disease[2];					
	tp->disease[3]		= ch->disease[3];					
	tp->disease[4]		= ch->disease[4];					
	tp->disease[5]		= ch->disease[5];					
	tp->viewtype		= ch->viewtype;
	//< CSD-010927
	memcpy(tp->aStepInfo, ch->aStepInfo, sizeof(tp->aStepInfo));
	//> CSD-010927
	p.h.header.size = sizeof( t_update_char_db);
	
	QueuePacket( connections, DB_DEMON, &p, 1 );
}

void updateBinaryDataToLoaginServer( t_connection c[], int id )
{														
	t_packet p;										
	LPCHARLIST ch = &c[id].chrlst;			
	
	if( ch->HpMax ==     0 )	return;
	if( ch->HungryMax == 0 )	return;
	if( ch->ManaMax ==   0 )	return;
	if( ch->updatable	== 0 )  return;
	
	p.h.header.type = CMD_UPDATE_BINARY_DATA0;														
	memcpy(p.u.server_update_binary_data0.name, c[id].name, NM_LENGTH);
	memcpy(p.u.server_update_binary_data0.id, c[id].id,  ID_LENGTH );
	memcpy(p.u.server_update_binary_data0.Ws,				(UCHAR *)ch->Ws,			SIZE_OF_WS );
	memcpy(p.u.server_update_binary_data0.Ps,				(UCHAR *)ch->Ps,			SIZE_OF_PS );
	memcpy(p.u.server_update_binary_data0.Skill,			(UCHAR *)ch->Skill,			SIZE_OF_SKILL );
	memcpy(p.u.server_update_binary_data0.skillexp,		(UCHAR *)ch->skillexp,		SIZE_OF_SKILL_EXP );
	memcpy(p.u.server_update_binary_data0.tac_skillEXP,	(UCHAR *)ch->tac_skillEXP,	SIZE_OF_TAC_SKILL_EXP );
	p.h.header.size = sizeof( t_server_update_binary_data0 );
	QueuePacket( c, DB_DEMON, &p, 1 );
	
	
	p.h.header.type = CMD_UPDATE_SCRIPT_DATA;
	memcpy(p.u.server_update_script_data.name, c[id].name, NM_LENGTH);
	memcpy(p.u.server_update_script_data.id, c[id].id,  ID_LENGTH );
	memcpy(p.u.server_update_script_data.script_var,			(UCHAR *)var[ ch->server_id],			SIZE_OF_SCRIPT_VAR );
	p.h.header.size = sizeof( t_server_update_script_data );
	QueuePacket( c, DB_DEMON, &p, 1 );
	
	//////////////////////////// 0218 YGI //////////////////////
	EndsetPartyMenberForUpdate( ch );		
	
	p.h.header.type = CMD_UPDATE_BINARY_DATA1;	
	memcpy(p.u.server_update_binary_data1.name, c[id].name, NM_LENGTH);
	memcpy(p.u.server_update_binary_data1.id, c[id].id,  ID_LENGTH );
	memcpy(p.u.server_update_binary_data1.equip,			(UCHAR *)ch->equip,			SIZE_OF_EQUIP);
	memcpy(p.u.server_update_binary_data1.quick,			(UCHAR *)ch->quick,			SIZE_OF_QUICK);
	memcpy(p.u.server_update_binary_data1.party,			(UCHAR *)ch->party_str,			SIZE_OF_PARTY );
	memcpy(p.u.server_update_binary_data1.relation,			(UCHAR *)ch->relation_str,		SIZE_OF_RELATION);
	memcpy(p.u.server_update_binary_data1.employment,		(UCHAR *)ch->employment_str,	SIZE_OF_EMPLOYMENT);
	p.h.header.size = sizeof( t_server_update_binary_data1 );
	QueuePacket( c, DB_DEMON, &p, 1 );
	
	p.h.header.type = CMD_UPDATE_INV_DATA;		
	memcpy(p.u.server_update_inv_data.name, c[id].name, NM_LENGTH);
	memcpy(p.u.server_update_inv_data.id,   c[id].id,  ID_LENGTH );
	memcpy(p.u.server_update_inv_data.inv,			(UCHAR *)ch->inv,				SIZE_OF_INV );
	p.h.header.size = sizeof( t_server_update_inv_data );	
	QueuePacket( c, DB_DEMON, &p, 1 );
	
	p.h.header.type = CMD_UPDATE_ITEM_DATA;		
	memcpy(p.u.server_update_item_data.name, c[id].name, NM_LENGTH);
	memcpy(p.u.server_update_item_data.id,   c[id].id,  ID_LENGTH );
	memcpy(p.u.server_update_item_data.Item,			(UCHAR *)ch->Item,			SIZE_OF_ITEMINDEX );
	p.h.header.size = sizeof( t_server_update_item_data	);		
	QueuePacket( c, DB_DEMON, &p, 1 );
	
	p.h.header.type = CMD_UPDATE_BANKITEM_DATA;
	memcpy(p.u.server_update_bankitem_data.name,	c[id].name, NM_LENGTH);
	memcpy(p.u.server_update_bankitem_data.id,		c[id].id,  ID_LENGTH );
	
#ifdef _GAME_SERVER_
	strcpy(p.u.server_update_bankitem_data.mapname, MapName );
#endif
	
	memcpy(p.u.server_update_bankitem_data.bankitem,		(UCHAR *)ch->bank,		SIZE_OF_BANKITEM );
	p.h.header.size = sizeof( t_server_update_bankitem_data	);		
	QueuePacket( c, DB_DEMON, &p, 1 );
}	

//1029 zhh
int RecvUpdateSpecial( t_server_special_item *p)
{		
	int temp=1;	
	SQLRETURN retcode;
	SQLHSTMT  hstmt;
	char query[1024];
	
	sprintf(query, "UPDATE Special_item SET Now_Store = %d , S_Day = %d where No= '%d'" , p->Now_Store , p->S_Day ,p->No);
	
	SQLAllocStmt(hDBC, &hstmt) ;
	retcode = SQLExecDirect(hstmt, (UCHAR *)query, SQL_NTS);
	if(retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO) {
		displaySQLError(hstmt) ;
		//		SQLTransact(SQL_NULL_HENV, hDBC, SQL_ROLLBACK);
		SQLFreeStmt(hstmt, SQL_DROP);			
		temp=-1;
	}	
	else {
		//		SQLTransact(SQL_NULL_HENV, hDBC, SQL_COMMIT);
		SQLFreeStmt(hstmt, SQL_DROP) ;			
	}
	return temp;
}


//acer5-----------------------------------
int AddCRC( void *pSource, int size, int step )
{
	int count = 0;
	for( int i=0; i<size; i+=step )
	{
		count += ((char*)pSource)[i];
	}
	return count;
}
// --------------------------------------------

//  Updatae������ LoginServer�� ������.
int RecvUpdateCharDB( t_update_char_db *p )
{
	// acer5 -------------
	int c_check = AddCRC( p, sizeof( t_update_char_db)-4, 10 );
	if( p->check_crc != c_check ) 
	{
		MyLog( 0, "error crc check... update 'update_char_db' : %s", p->name );
		return 0;
	}
	
	SQLRETURN retcode;
	SQLHSTMT  hstmt;
	char query[2048];
	
	DWORD temp_NWCharacter;
	memcpy(&temp_NWCharacter,&p->NWCharacter,sizeof(DWORD));
	ConvertSave(p->aStepInfo); // CSD-TW-030620
	
	sprintf(query, "UPDATE chr_info "
		      "SET lev = %d, spritvalue = %d, social_status = %d, fame = %d, fame_pk = %d, guildname = '%s' "
			  "WHERE name= '%s'",
			  p->Level, 
			  p->nGuildCode,
			  p->social_status,
			  p->fame,
			  //p->fame_pk,	// 010915 LTS	//Fame_PK -> NWCharacter�� ��ü DB���� ������ NWCharacter�� ���� ���ϴ�?		
			  temp_NWCharacter,	// 010915 LTS
			  p->aStepInfo,	// CSD-TW-030620
			  p->name);
	//> CSD-030324
	SQLAllocStmt(hDBC, &hstmt) ;
	retcode = SQLExecDirect(hstmt, (UCHAR *)query, SQL_NTS);
	if(retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO) {
		displaySQLError(hstmt) ;
		//		SQLTransact(SQL_NULL_HENV, hDBC, SQL_ROLLBACK);
		SQLFreeStmt(hstmt, SQL_DROP);
		return -1 ;
	}	
	else {
		//		SQLTransact(SQL_NULL_HENV, hDBC, SQL_COMMIT);
		SQLFreeStmt(hstmt, SQL_DROP) ;
	}	
	
	sprintf(query, "UPDATE chr_info SET"
		" Str= %d, Con= %d, Dex= %d, Wis= %d, "
		" `Int`= %d, MoveP= %d, `Char`= %d, Endu= %d, Moral= %d, Luck= %d, wsps= %d, tactics= %d, nation= %d, Money= %d, Hp= %d, HpMax = %d, "
		" mana= %d, manamax= %d, `condition`= %d, mapname='%s', sight= %d, Age= %d, bAlive= %d "
		" where name= '%s'"
		, p->Str, p->Con, p->Dex, p->Wis,
		p->Int, p->MoveP, p->Char, p->Endu, p->Moral, p->Luck, p->wsps, p->Tactics, p->nation, p->Money, p->Hp, p->HpMax,
		p->Mana, p->ManaMax, p->Condition, p->MapName, p->Sight, p->Age, p->bAlive
		, p->name );
	SQLAllocStmt(hDBC, &hstmt) ;
	retcode = SQLExecDirect(hstmt, (UCHAR *)query, SQL_NTS);
	if(retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO) {
		displaySQLError(hstmt) ;
		//		SQLTransact(SQL_NULL_HENV, hDBC, SQL_ROLLBACK);
		SQLFreeStmt(hstmt, SQL_DROP);
		return -1 ;
	}	
	else {
		//		SQLTransact(SQL_NULL_HENV, hDBC, SQL_COMMIT);
		SQLFreeStmt(hstmt, SQL_DROP) ;
	}	
	
	sprintf(query, "UPDATE chr_info SET "
		" hungry= %d, hungrymax= %d, killmon= %d, "
		" killanimal= %d, killpc = %d, resist_poison= %d, resist_stone= %d, resist_magic= %d, resist_fire= %d, "
		" resist_ice= %d, resist_elect= %d, x=%d, y=%d, "
		" acc_equip1= %d, acc_equip2= %d, acc_equip3= %d, acc_equip4= %d "
		" where name= '%s' "
		,p->Hungry, p->HungryMax, p->killmon,
		p->killanimal, p->killpc, 
		//< CSD-010907
		p->nPoison, 
		p->nCurse, 
		p->nHoly, 
		p->nFire,
		p->nIce, 
		p->nElect, 
		//> CSD-010907
		p->X, p->Y,
		p->accessory[0], p->accessory[1], p->accessory[2], p->accessory[3],
		p->name );
	
	SQLAllocStmt(hDBC, &hstmt);
	retcode = SQLExecDirect(hstmt, (UCHAR *)query, SQL_NTS);
	if(retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO){
		displaySQLError(hstmt);
		//		SQLTransact(SQL_NULL_HENV, hDBC, SQL_ROLLBACK);
		SQLFreeStmt(hstmt, SQL_DROP);
		return -1 ;
	}
	else {
		//		SQLTransact(SQL_NULL_HENV, hDBC, SQL_COMMIT);
		SQLFreeStmt(hstmt, SQL_DROP) ;
	}
	
	// 010406 YGI
	// 010531 KHS  nut1,2,3 �� nk3,4,5 Update�߰�..
	sprintf(query, "UPDATE chr_info SET"
		" openhouse = %d, reserved_point= %d,"
		" bankmoney= %u, LastLoan= %d, exp = %u, "
		" disease1=%d, disease2=%d, disease3=%d, disease4=%d, disease5=%d, disease6=%d, viewtype= %d, "
		" win_defeat = %d, LadderScore = %d, nut1 = %d, nut2 = %d, nut3 = %d "
		" where name= '%s'",
		
		p->openhouse,
		p->reserved_point,
		
		p->BankMoney,
		p->LastLoan,
		p->Exp,
		
		p->disease[0], 
		p->disease[1], 
		p->disease[2], 
		p->disease[3], 
		p->disease[4], 
		p->disease[5], 
		
		p->viewtype,
		
		p->win_defeat,
		p->LadderScore,
		
		
		p->nk3, p->nk4, p->nk6,
		
		
		p->name );
	
	SQLAllocStmt(hDBC, &hstmt) ;
	retcode = SQLExecDirect(hstmt, (UCHAR *)query, SQL_NTS);
	if(retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO) {
		//printf("\n Update Character: Exec Direct Error ; %s", query) ;
		displaySQLError(hstmt) ;
		//		SQLTransact(SQL_NULL_HENV, hDBC, SQL_ROLLBACK);
		SQLFreeStmt(hstmt, SQL_DROP);
		return -1 ;
	}
	else {
		//		SQLTransact(SQL_NULL_HENV, hDBC, SQL_COMMIT);
		SQLFreeStmt(hstmt, SQL_DROP) ;
	}
	
	// 010504 YGI
	/*	if( p->LadderScore != 1000 )		// 010410 YGI
	{
	UpdateLadderScore( p->LadderScore, p->name );
	}*/
	
	
	return 1;
}	





int RecvUpdateTacticSkillExpData( t_update_very_important_tactics *p )
{													
	//-----------------						
	SQLLEN  cbdata = 200, cbvalue = SQL_DATA_AT_EXEC ;						
	SQLLEN	cbtacskillexplen;												
	SQLRETURN	retcode;									
	SQLHSTMT	hstmt;											
	SQLPOINTER	pParamData;								
	int			n=0;											
	char		aaa=20;										
	char		query[256]={0,};										
	
	sprintf(query, "UPDATE chr_info SET tac_skillEXP=? WHERE name='%s'", p->name ) ;
	
	//printf("\nSQL: %s", query) ;
	SQLAllocStmt(hDBC, &hstmt);
	
	retcode = SQLPrepare(hstmt, (UCHAR *)query, SQL_NTS);
	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
		
		retcode= SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_BINARY,SQL_LONGVARBINARY, SIZE_OF_TAC_SKILL_EXP,	0, p->tac_skillEXP,0, &cbtacskillexplen); 
		
		if(retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO)
		{			
			SQLFreeStmt(hstmt, SQL_DROP);
			//printf("\nBinding FAIL!!!") ;
			return -5 ;							
		}			
		
		cbtacskillexplen= SQL_LEN_DATA_AT_EXEC(0);
		
		retcode = SQLExecute(hstmt) ;
		
		while(retcode == SQL_NEED_DATA) 
		{      
			retcode = SQLParamData(hstmt, &pParamData) ;
			if(retcode == SQL_NEED_DATA) 
			{
				if(pParamData == p->tac_skillEXP)	SQLPutData(hstmt, p->tac_skillEXP, SIZE_OF_TAC_SKILL_EXP );
			}
			else break ;
		}
		
		retcode = SQLExecute(hstmt) ;
		
		if(retcode == SQL_NEED_DATA)
		{
			retcode= SQLCancel(hstmt) ;
			//			retcode= SQLTransact(SQL_NULL_HENV, hDBC, SQL_COMMIT);
			retcode= SQLFreeStmt(hstmt, SQL_DROP );
			
			if(retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO) {
				//printf("\n Transact commit Error") ;
			}
			//printf("\nTransact & free") ;
			return(1); // succeed
		}
		else
		{
			displaySQLError(hstmt) ;
			//			SQLTransact(SQL_NULL_HENV, hDBC, SQL_ROLLBACK);
			SQLFreeStmt(hstmt, SQL_DROP);
			return(-3); 
		}
	}
	SQLFreeStmt(hstmt, SQL_DROP);
	return 1;
}











///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int RecvUpdateBinaryData( t_server_update_binary_data0 *p )
{	
	// acer5 -------------
	int c_check = AddCRC( p, sizeof( t_server_update_binary_data0 )-4, 10 );
	if( p->check_crc != c_check ) 
	{
		MyLog( 0, "error crc check... update 't_server_update_binary_data0' : %s", p->name  );
		return 0;
	}
	// -------------------
	SQLLEN  cbdata = 200, cbvalue = SQL_DATA_AT_EXEC ;
	SQLLEN	cbwslen, cbpslen, cbskilllen, cbskillexplen,cbtacskillexplen;
	SQLRETURN	retcode;
	SQLHSTMT	hstmt;
	SQLPOINTER	pParamData;
	int			n=0;
	char		aaa=20;
	char		query[256]={0,};
	
	sprintf(query, "UPDATE chr_info SET ws=?, ps=?,skill=?, skill_exp=?, tac_skillEXP=? WHERE name='%s'", p->name) ;
	
	//printf("\nSQL: %s", query) ;
	SQLAllocStmt(hDBC, &hstmt);
	retcode = SQLPrepare(hstmt, (UCHAR *)query, SQL_NTS);
	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
		
		retcode= SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_BINARY,SQL_LONGVARBINARY, SIZE_OF_WS,			0, p->Ws,			0, &cbwslen); 
		retcode= SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT, SQL_C_BINARY,SQL_LONGVARBINARY, SIZE_OF_PS,			0, p->Ps,			0, &cbpslen); 
		retcode= SQLBindParameter(hstmt, 3, SQL_PARAM_INPUT, SQL_C_BINARY,SQL_LONGVARBINARY, SIZE_OF_SKILL,			0, p->Skill,		0, &cbskilllen); 
		retcode= SQLBindParameter(hstmt, 4, SQL_PARAM_INPUT, SQL_C_BINARY,SQL_LONGVARBINARY, SIZE_OF_SKILL_EXP,		0, p->skillexp,	0, &cbskillexplen); 
		retcode= SQLBindParameter(hstmt, 5, SQL_PARAM_INPUT, SQL_C_BINARY,SQL_LONGVARBINARY, SIZE_OF_TAC_SKILL_EXP,	0, p->tac_skillEXP,0, &cbtacskillexplen); 
		
		if(retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO)
		{
			SQLFreeStmt(hstmt, SQL_DROP);
			//printf("\nBinding FAIL!!!") ;
			return -5 ;
		}
		
		cbpslen			= SQL_LEN_DATA_AT_EXEC(0);
		cbwslen			= SQL_LEN_DATA_AT_EXEC(0);
		cbskilllen		= SQL_LEN_DATA_AT_EXEC(0);
		cbskillexplen	= SQL_LEN_DATA_AT_EXEC(0);
		cbtacskillexplen= SQL_LEN_DATA_AT_EXEC(0);
		
		retcode = SQLExecute(hstmt) ;
		
		while(retcode == SQL_NEED_DATA) 
		{      
			retcode = SQLParamData(hstmt, &pParamData) ;
			if(retcode == SQL_NEED_DATA) 
			{
				if(pParamData == p->Ws) 				SQLPutData(hstmt, p->Ws, SIZE_OF_WS);
				else if(pParamData == p->Ps) 			SQLPutData(hstmt, p->Ps, SIZE_OF_PS);      
				else if(pParamData == p->Skill) 		SQLPutData(hstmt, p->Skill, SIZE_OF_SKILL);      
				else if(pParamData == p->skillexp) 	SQLPutData(hstmt, p->skillexp, SIZE_OF_SKILL_EXP );      
				else if(pParamData == p->tac_skillEXP)	SQLPutData(hstmt, p->tac_skillEXP, SIZE_OF_TAC_SKILL_EXP );      
				
			}
			else break ;
		}
		
		retcode = SQLExecute(hstmt) ;
		
		if(retcode == SQL_NEED_DATA)
		{
			retcode= SQLCancel(hstmt) ;
			//			retcode= SQLTransact(SQL_NULL_HENV, hDBC, SQL_COMMIT);
			retcode= SQLFreeStmt(hstmt, SQL_DROP );
			
			if(retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO) {
				//printf("\n Transact commit Error") ;
			}
			//printf("\nTransact & free") ;
			return(1); // succeed
		}
		else
		{
			displaySQLError(hstmt) ;
			//			SQLTransact(SQL_NULL_HENV, hDBC, SQL_ROLLBACK);
			SQLFreeStmt(hstmt, SQL_DROP);
			return(-3); 
		}
	}
	SQLFreeStmt(hstmt, SQL_DROP);
	return 1;
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int RecvUpdateBinaryData1( t_server_update_binary_data1 *p )
{
	// acer5 -------------
	int c_check = AddCRC( p, sizeof( t_server_update_binary_data1 )-4, 10 );
	if( p->check_crc != c_check ) 
	{
		MyLog( 0, "error crc check... update 't_server_update_binary_data1 ' : %s", p->name );
		return 0;
	}
	// -------------------
	
	SQLLEN  cbdata = 200, cbvalue = SQL_DATA_AT_EXEC ;
	SQLLEN	cbquick, cbequip, cbparty, cbrelation, cbemployment;
	SQLRETURN	retcode;
	SQLHSTMT	hstmt;
	SQLPOINTER	pParamData;
	int			n=0;
	char		aaa=20;
	char		query[256]={0,};
	
	sprintf(query, "UPDATE chr_info SET equip=?, quick=?, party=?, relation=?, employment=?  WHERE name='%s'", p->name) ;
	
	//printf("\nSQL: %s", query) ;
	SQLAllocStmt(hDBC, &hstmt);
	retcode = SQLPrepare(hstmt, (UCHAR *)query, SQL_NTS);
	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
		
		retcode= SQLBindParameter(hstmt,  1, SQL_PARAM_INPUT, SQL_C_BINARY,SQL_LONGVARBINARY, SIZE_OF_EQUIP,		0, p->equip,		0, &cbequip); 
		retcode= SQLBindParameter(hstmt,  2, SQL_PARAM_INPUT, SQL_C_BINARY,SQL_LONGVARBINARY, SIZE_OF_QUICK,		0, p->quick,		0, &cbquick); 
		retcode= SQLBindParameter(hstmt,  3, SQL_PARAM_INPUT, SQL_C_BINARY,SQL_LONGVARBINARY, SIZE_OF_PARTY,		0, p->party,		0, &cbparty     ); 
		retcode= SQLBindParameter(hstmt,  4, SQL_PARAM_INPUT, SQL_C_BINARY,SQL_LONGVARBINARY, SIZE_OF_RELATION,		0, p->relation,		0, &cbrelation  ); 
		retcode= SQLBindParameter(hstmt,  5, SQL_PARAM_INPUT, SQL_C_BINARY,SQL_LONGVARBINARY, SIZE_OF_EMPLOYMENT,	0, p->employment,	0, &cbemployment); 
		
		
		if(retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO)
		{
			SQLFreeStmt(hstmt, SQL_DROP);
			printf("\nBinding FAIL!!!") ;
			return -5 ;
		}
		
		cbequip			= SQL_LEN_DATA_AT_EXEC(0);
		cbquick			= SQL_LEN_DATA_AT_EXEC(0);
		cbparty			= SQL_LEN_DATA_AT_EXEC(0);
		cbrelation		= SQL_LEN_DATA_AT_EXEC(0);
		cbemployment	= SQL_LEN_DATA_AT_EXEC(0);
		
		retcode = SQLExecute(hstmt) ;
		
		while(retcode == SQL_NEED_DATA) 
		{      
			retcode = SQLParamData(hstmt, &pParamData) ;
			if(retcode == SQL_NEED_DATA) 
			{
				if(pParamData == p->equip) 		SQLPutData(hstmt, p->equip,			SIZE_OF_EQUIP);
				else if(pParamData == p->quick) 		SQLPutData(hstmt, p->quick,			SIZE_OF_QUICK );
				else if(pParamData == p->party ) 		SQLPutData(hstmt, p->party,			SIZE_OF_PARTY );
				else if(pParamData == p->relation) 	SQLPutData(hstmt, p->relation,		SIZE_OF_RELATION);
				else if(pParamData == p->employment ) 	SQLPutData(hstmt, p->employment,	SIZE_OF_EMPLOYMENT );
			}
			else break ;
		}
		
		retcode = SQLExecute(hstmt) ;
		
		if(retcode == SQL_NEED_DATA)
		{
			retcode= SQLCancel(hstmt) ;
			//			retcode= SQLTransact(SQL_NULL_HENV, hDBC, SQL_COMMIT);
			retcode= SQLFreeStmt(hstmt, SQL_DROP );
			
			if(retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO) {
				printf("\n Transact commit Error") ;
			}
			//printf("\nTransact & free") ;
			return(1); // succeed
		}
		else
		{
			displaySQLError(hstmt) ;
			//			SQLTransact(SQL_NULL_HENV, hDBC, SQL_ROLLBACK);
			SQLFreeStmt(hstmt, SQL_DROP);
			return(-3); 
		}
	}
	
	SQLFreeStmt(hstmt, SQL_DROP);
	return 1;
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



int  RecvUpdateScriptData( t_server_update_script_data *p )
{
	// acer5 -------------
	int c_check = AddCRC( p, sizeof( t_server_update_script_data )-4, 10 );
	if( p->check_crc != c_check ) 
	{
		MyLog( 0, "error crc check... update 't_server_update_script_data ' : %s", p->name  );
		return 0;
	}
	// -------------------
	
	SQLLEN  cbdata = 200, cbvalue = SQL_DATA_AT_EXEC ;
	SQLLEN	cbscriptvarlen;
	SQLRETURN	retcode;
	SQLHSTMT	hstmt;
	SQLPOINTER	pParamData;
	int			n=0;
	char		aaa=20;
	char		query[256]={0,};
	
	sprintf(query, "UPDATE chr_info SET script_var=? WHERE name='%s'", p->name) ;
	
	//printf("\nSQL: %s", query) ;
	SQLAllocStmt(hDBC, &hstmt);
	retcode = SQLPrepare(hstmt, (UCHAR *)query, SQL_NTS);
	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
		
		retcode= SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_BINARY,SQL_LONGVARBINARY, SIZE_OF_SCRIPT_VAR,	0, p->script_var,	0, &cbscriptvarlen); 
		
		if(retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO)
		{
			SQLFreeStmt(hstmt, SQL_DROP);
			//printf("\nBinding FAIL!!!") ;
			return -5 ;
		}
		
		cbscriptvarlen	= SQL_LEN_DATA_AT_EXEC(0);
		
		retcode = SQLExecute(hstmt) ;
		
		while(retcode == SQL_NEED_DATA) 
		{      
			retcode = SQLParamData(hstmt, &pParamData) ;
			if(retcode == SQL_NEED_DATA) 
			{
				if(pParamData == p->script_var ) 	SQLPutData(hstmt, p->script_var, SIZE_OF_SCRIPT_VAR );
			}
			else break ;
		}
		
		cbscriptvarlen	= SQL_LEN_DATA_AT_EXEC(0);
		
		retcode = SQLExecute(hstmt) ;
		
		if(retcode == SQL_NEED_DATA)
		{
			retcode= SQLCancel(hstmt) ;
			//			retcode= SQLTransact(SQL_NULL_HENV, hDBC, SQL_COMMIT);
			retcode= SQLFreeStmt(hstmt, SQL_DROP );
			
			if(retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO) {
				//printf("\n Transact commit Error") ;
			}
			//printf("\nTransact & free") ;
			return(1); // succeed
		}
		else
		{
			displaySQLError(hstmt) ;
			//			SQLTransact(SQL_NULL_HENV, hDBC, SQL_ROLLBACK);
			SQLFreeStmt(hstmt, SQL_DROP);
			return(-3); 
		}
	}
	
	SQLFreeStmt(hstmt, SQL_DROP);
	return 1;
}



int RecvUpdateInvData( t_server_update_inv_data *p )
{
	// acer5 -------------
	int c_check = AddCRC( p, sizeof( t_server_update_inv_data )-4, 10 );
	if( p->check_crc != c_check ) 
	{
		MyLog( 0, "error crc check... update 't_server_update_inv_data' : %s", p->name );
		return 0;
	}
	// -------------------
	
	SQLLEN  cbdata = 200, cbvalue = SQL_DATA_AT_EXEC ;
	SQLLEN	cbinvlen;
	SQLRETURN	retcode;
	SQLHSTMT	hstmt;
	SQLPOINTER	pParamData;
	int			n=0;
	char		aaa=20;
	char		query[256]={0,};
	
	sprintf(query, "UPDATE chr_info SET inventory=?  WHERE name='%s'", p->name) ;
	
	//printf("\nSQL: %s", query) ;
	SQLAllocStmt(hDBC, &hstmt);
	retcode = SQLPrepare(hstmt, (UCHAR *)query, SQL_NTS);
	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
		
		retcode= SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_BINARY,SQL_LONGVARBINARY, SIZE_OF_INV,			0, p->inv,			0, &cbinvlen); 
		
		
		if(retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO)
		{
			SQLFreeStmt(hstmt, SQL_DROP);
			printf("\nBinding FAIL!!!") ;
			return -5 ;
		}
		
		cbinvlen		= SQL_LEN_DATA_AT_EXEC(0);
		
		retcode = SQLExecute(hstmt) ;
		
		while(retcode == SQL_NEED_DATA) 
		{      
			retcode = SQLParamData(hstmt, &pParamData) ;
			if(retcode == SQL_NEED_DATA) 
			{
				if(pParamData == p->inv) 			SQLPutData(hstmt, p->inv, SIZE_OF_INV);
			}
			else break ;
		}
		
		retcode = SQLExecute(hstmt) ;
		
		if(retcode == SQL_NEED_DATA)
		{
			retcode= SQLCancel(hstmt) ;
			//			retcode= SQLTransact(SQL_NULL_HENV, hDBC, SQL_COMMIT);
			retcode= SQLFreeStmt(hstmt, SQL_DROP );
			
			
			if(retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO) {
				printf("\n Transact commit Error") ;
			}
			//printf("\nTransact & free") ;
			return(1); // succeed
		}
		else
		{
			displaySQLError(hstmt) ;
			//			SQLTransact(SQL_NULL_HENV, hDBC, SQL_ROLLBACK);
			SQLFreeStmt(hstmt, SQL_DROP);
			return(-3); 
		}
	}
	SQLFreeStmt(hstmt, SQL_DROP);
	return 1;
}


int RecvUpdateItemData( t_server_update_item_data *p )
{
	// acer5 -------------
	int c_check = AddCRC( p, sizeof( t_server_update_item_data )-4, 10 );
	if( p->check_crc != c_check ) 
	{
		MyLog( 0, "error crc check... update 't_server_update_item_data' : %s", p->name  );
		return 0;
	}
	// -------------------
	
	SQLLEN  cbdata = 200, cbvalue = SQL_DATA_AT_EXEC ;
	SQLLEN	cbitemindex;
	SQLRETURN	retcode;
	SQLHSTMT	hstmt;
	SQLPOINTER	pParamData;
	int			n=0;
	char		aaa=20;
	char		query[256]={0,};
	
	sprintf(query, "UPDATE chr_info SET itemindex=?  WHERE name='%s'", p->name) ;
	
	//printf("\nSQL: %s", query) ;
	SQLAllocStmt(hDBC, &hstmt);
	retcode = SQLPrepare(hstmt, (UCHAR *)query, SQL_NTS);
	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
		
		retcode= SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_BINARY,SQL_LONGVARBINARY, SIZE_OF_ITEMINDEX,	0, p->Item,	0, &cbitemindex); 
		
		
		if(retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO)
		{
			SQLFreeStmt(hstmt, SQL_DROP);
			printf("\nBinding FAIL!!!") ;
			return -5 ;
		}
		
		cbitemindex		= SQL_LEN_DATA_AT_EXEC(0);
		
		retcode = SQLExecute(hstmt) ;
		
		while(retcode == SQL_NEED_DATA) 
		{      
			retcode = SQLParamData(hstmt, &pParamData) ;
			if(retcode == SQL_NEED_DATA) 
			{
				if(pParamData == p->Item ) 	SQLPutData(hstmt, p->Item,	SIZE_OF_ITEMINDEX );
				
			}
			else break ;
		}
		
		retcode = SQLExecute(hstmt) ;
		
		if(retcode == SQL_NEED_DATA)
		{
			retcode= SQLCancel(hstmt) ;
			//			retcode= SQLTransact(SQL_NULL_HENV, hDBC, SQL_COMMIT);
			retcode= SQLFreeStmt(hstmt, SQL_DROP );
			
			if(retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO) {
				printf("\n Transact commit Error") ;
			}
			//printf("\nTransact & free") ;
			return(1); // succeed
		}
		else
		{
			displaySQLError(hstmt) ;
			//			SQLTransact(SQL_NULL_HENV, hDBC, SQL_ROLLBACK);
			SQLFreeStmt(hstmt, SQL_DROP);
			return(-3); 
		}
	}
	SQLFreeStmt(hstmt, SQL_DROP);
	return 1;
}	





int RecvUpdateBankItemData( t_connection c[], int cn, t_server_update_bankitem_data  *p  )
{		
	// acer5 -------------
	int c_check = AddCRC( p, sizeof( t_server_update_bankitem_data )-4, 10 );
	if( p->check_crc != c_check ) 
	{
		MyLog( 0, "error crc check... update 't_server_update_bankitem_data' : %s", p->name );
		return 0;
	}
	// -------------------
	
	SQLLEN  cbdata = 200, cbvalue = SQL_DATA_AT_EXEC ;
	SQLLEN	cbbankitemlen;
	SQLRETURN	retcode;
	SQLHSTMT	hstmt;
	SQLPOINTER	pParamData;
	int			n=0;
	char		aaa=20;
	char		query[256]={0,};
	
	sprintf(query, "UPDATE chr_info2 SET bankitem=? WHERE name='%s'", p->name) ;
	
	SQLAllocStmt(hDBC, &hstmt);
	retcode = SQLPrepare(hstmt, (UCHAR *)query, SQL_NTS);
	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) 
	{	
		retcode= SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_BINARY,SQL_LONGVARBINARY, SIZE_OF_BANKITEM,			0, p->bankitem,			0, &cbbankitemlen );
		
		if(retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO)
		{
			SQLFreeStmt(hstmt, SQL_DROP);
			printf("\nBinding FAIL!!!") ;
			return -5 ;
		}
		
		cbbankitemlen	= SQL_LEN_DATA_AT_EXEC(0);
		
		retcode = SQLExecute(hstmt) ;
		while(retcode == SQL_NEED_DATA) 
		{      
			retcode = SQLParamData(hstmt, &pParamData) ;
			if(retcode == SQL_NEED_DATA) 
			{
				if(pParamData == p->bankitem ) 	
					int ret = SQLPutData(hstmt, p->bankitem , SIZE_OF_BANKITEM );
			}
			else break ;
		}
		
		retcode = SQLExecute(hstmt) ;
		
		if(retcode == SQL_NEED_DATA)
		{
			retcode= SQLCancel(hstmt) ;
			retcode= SQLFreeStmt(hstmt, SQL_DROP );
			
			
			if(retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO) {
				printf("\n Transact commit Error") ;
			}
			
			return(1); // succeed
		}	
		else
		{	
			SQLFreeStmt(hstmt, SQL_DROP);
			return(-3); 
		}
	}
	
	SQLFreeStmt(hstmt, SQL_DROP);
	return 0;
}




















/*
3	name	char	20	0	0	0		0			0
2	login_id	char	20	0	0	0		0			0
0	gender	tinyint	1	3	0	1		0			0
0	sprno	tinyint	1	3	0	1		0			0
0	race	tinyint	1	3	0	1		0			0
0	spritvalue	tinyint	1	3	0	1		0			0
0	face	tinyint	1	3	0	1		0			0
0	class	tinyint	1	3	0	1		0			0
0	class_sprituality	tinyint	1	3	0	1		0			0
0	class_poisoning	tinyint	1	3	0	1		0			0
0	class_bombplay	tinyint	1	3	0	1		0			0
0	class_entrapment	tinyint	1	3	0	1		0			0
0	class_scrolling	tinyint	1	3	0	1		0			0
0	class_cunning1	tinyint	1	3	0	1		0			0
0	class_cunning2	tinyint	1	3	0	1		0			0
0	class_cunning3	tinyint	1	3	0	1		0			0
0	class_stealing	tinyint	1	3	0	1		0			0
0	job	tinyint	1	3	0	1		0			0
0	spell	tinyint	1	3	0	1		0			0
0	clothr	smallint	2	5	0	1		0			0
0	clothg	smallint	2	5	0	1		0			0
0	clothb	smallint	2	5	0	1		0			0
0	bodyr	smallint	2	5	0	1		0			0
0	bodyg	smallint	2	5	0	1		0			0
0	bodyb	smallint	2	5	0	1		0			0
0	age	smallint	2	5	0	1		0			0
0	x	smallint	2	5	0	1		0			0
0	y	smallint	2	5	0	1		0			0
0	lev	smallint	2	5	0	1		0			0
0	exp	money	8	19	4	1		0			0
0	attackpow	smallint	2	5	0	1		0			0
0	attackmax	smallint	2	5	0	1		0			0
0	mana	smallint	2	5	0	1		0			0
0	manamax	smallint	2	5	0	1		0			0
0	vitalpow	smallint	2	5	0	1		0			0
0	vitalmax	smallint	2	5	0	1		0			0
0	hungry	smallint	2	5	0	1		0			0
0	hungrymax	smallint	2	5	0	1		0			0
0	guildname	char	20	0	0	1		0			0
0	nut1	smallint	2	5	0	1		0			0
0	nut2	smallint	2	5	0	1		0			0
0	nut3	smallint	2	5	0	1		0			0
0	killmon1	int	4	10	0	1		0			0
0	killmon2	int	4	10	0	1		0			0
0	killmon3	int	4	10	0	1		0			0
0	killmon4	int	4	10	0	1		0			0
0	killmon5	int	4	10	0	1		0			0
0	killmon6	int	4	10	0	1		0			0
0	killanimal	int	4	10	0	1		0			0
0	killnpc	int	4	10	0	1		0			0
0	money	decimal	9	18	0	1		0			0
0	strold	smallint	2	5	0	1		0			0
0	conold	smallint	2	5	0	1		0			0
0	dexold	smallint	2	5	0	1		0			0
0	wisold	smallint	2	5	0	1		0			0
0	intold	smallint	2	5	0	1		0			0
0	movepold	smallint	2	5	0	1		0			0
0	charold	smallint	2	5	0	1		0			0
0	enduold	smallint	2	5	0	1		0			0
0	healthold	smallint	2	5	0	1		0			0
0	moralold	smallint	2	5	0	1		0			0
0	luckold	smallint	2	5	0	1		0			0
0	str	smallint	2	5	0	1		0			0
0	con	smallint	2	5	0	1		0			0
0	dex	smallint	2	5	0	1		0			0
0	wis	smallint	2	5	0	1		0			0
0	int	smallint	2	5	0	1		0			0
0	movep	smallint	2	5	0	1		0			0
0	char	smallint	2	5	0	1		0			0
0	endu	smallint	2	5	0	1		0			0
0	health	smallint	2	5	0	1		0			0
0	moral	smallint	2	5	0	1		0			0
0	luck	smallint	2	5	0	1		0			0
0	condition	smallint	2	5	0	1		0			0
0	resist_poison	smallint	2	5	0	1		0			0
0	resist_stone	smallint	2	5	0	1		0			0
0	resist_magic	smallint	2	5	0	1		0			0
0	resist_fire	smallint	2	5	0	1		0			0
0	resist_ice	smallint	2	5	0	1		0			0
0	resist_elect	smallint	2	5	0	1		0			0
0	wizardspell	smallint	2	5	0	1		0			0
0	ws	binary	400	0	0	1		0			0
0	priestspell	char	10	0	0	1		0			0
0	ps	binary	400	0	0	1		0			0
0	bankmoney	money	8	19	4	1		0			0
0	acc_equip1	tinyint	1	3	0	1		0			0
0	acc_equip2	tinyint	1	3	0	1		0			0
0	acc_equip3	tinyint	1	3	0	1		0			0
0	acc_equip4	tinyint	1	3	0	1		0			0
0	acc_equip5	tinyint	1	3	0	1		0			0
0	acc_equip6	tinyint	1	3	0	1		0			0
0	sight	smallint	2	5	0	1		0			0
0	inventory	binary	1152	0	0	1		0			0
0	tactics	smallint	2	5	0	1		0			0
0	MapName	char	20	0	0	1		0			0
0	peacests	tinyint	1	3	0	1		0			0
0	quick	binary	96	0	0	1		0			0
0	equip	binary	128	0	0	1		0			0
*/

// 030923 HK YGI
void displaySQLError(SQLHSTMT hStmt, char *szQuery)
{
	SQLCHAR err[80] = {0, } ;
	SQLINTEGER nep = 0 ;
	SQLCHAR msg[255] = { 0, } ;
	SQLSMALLINT num = 0 ;
	
	int retcode = SQLGetDiagRec(SQL_HANDLE_STMT, hStmt, 1, err, &nep, msg, 254, &num) ;
	
	if(retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
	{
		MyLog(0, "SQL Error MSG: %s; %s", err, msg);
	}
	
	if( szQuery )
	{
		char query[100];
		strncpy( query, szQuery, 99 );
		query[99] = 0;
		
		MyLog(2, "query Error : %s", query );
	}
}



extern int AddBoxItem_SQL(int id);
extern int DeleteBoxItem_SQL(int id);
extern int PutBoxItem_SQL(int id, int pos, ItemAttr *I);
extern int GetBoxItem_SQL(int id, int pos, ItemAttr *I);

int AddBoxItem_SQL(int id)
{
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	char		szQuerry[255];
	
	sprintf(szQuerry, "insert into box_item(id) values (%d)", id);
	
	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{
		//			SQLTransact(SQL_NULL_HENV, hDBC, SQL_COMMIT);
		SQLFreeStmt(hStmt, SQL_DROP);
		return(1); // succeed
	}
	else
	{
		//			SQLTransact(SQL_NULL_HENV, hDBC, SQL_ROLLBACK);
		SQLFreeStmt(hStmt, SQL_DROP);
		return(-3); // unknown error
	}
};

int DeleteBoxItem_SQL(int id)
{
	HSTMT hStmt = NULL;
	RETCODE retCode;
	char szQuerry[255];
	
    sprintf(szQuerry, "delete from box_item where id=%d", id);
	
	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{
		retCode = SQLFetch(hStmt);
		SQLFreeStmt(hStmt, SQL_DROP);
		return(1);
	}
	SQLFreeStmt(hStmt, SQL_DROP);
	return -1;
};

int PutBoxItem_SQL(int id, int pos, ItemAttr *I)			//0308 YGI
{
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	char		szQuerry[255];
	
	if(pos < 0 || pos > 14)	return(-1);			// 0204 YGI
	pos ++;
	
    //sprintf(szQuerry, "select id from box_item where id = %d", id );
    /////////////////////////////////////////////////////////////////////////////
    sprintf(szQuerry, "CALL up_check_box_item_id( %d)", id);
    /////////////////////////////////////////////////////////////////////////////
	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{
		retCode = SQLFetch(hStmt);
		if( retCode == SQL_NO_DATA )
		{
			SQLFreeStmt(hStmt, SQL_DROP) ;	
			AddBoxItem_SQL(id);			
		}
		else SQLFreeStmt(hStmt, SQL_DROP) ;
	}
	else return 0;
	
	sprintf(szQuerry, "update box_item set item_%02d=%d, "
		"attr_%02d_1=%d, attr_%02d_2=%d, attr_%02d_3=%d, attr_%02d_4=%d, attr_%02d_5=%d, attr_%02d_6=%d "
		"where id=%d", 
		pos, I->item_no, 
		pos, I->attr[0], pos, I->attr[1], pos, I->attr[2], pos, I->attr[3], pos, I->attr[4], pos, I->attr[5], id);
	
	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{
		//			SQLTransact(SQL_NULL_HENV, hDBC, SQL_COMMIT );
		SQLFreeStmt(hStmt, SQL_DROP);
		return(1);
	}
	else 
	{
		///			SQLTransact(SQL_NULL_HENV, hDBC, SQL_ROLLBACK);
		SQLFreeStmt(hStmt, SQL_DROP);
		return(-2);
	}
};

int GetBoxItem_SQL(int id, int pos, ItemAttr *I)			// 0612 YGI
{
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	char		szQuerry[255];
	DWORD		t;
	
	if(pos < 0 || pos > 14)	return(-1);
	pos ++;
	
	//sprintf(szQuerry, "select item_%02d, "
	//    "attr_%02d_1, attr_%02d_2, attr_%02d_3, attr_%02d_4, attr_%02d_5, attr_%02d_6 "
	//    "from box_item where id=%d", 
	//    pos, pos, pos, pos, pos, pos, pos, id);
	/////////////////////////////////////////////////////////////////////////////
	sprintf(szQuerry, "CALL up_get_box_item_info( '%02d', %d)", pos, id);
	/////////////////////////////////////////////////////////////////////////////
	
	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{
		retCode = SQLFetch(hStmt);
		retCode = SQLGetData(hStmt, 1, SQL_C_SSHORT, &I->item_no, 0, NULL);
		retCode = SQLGetData(hStmt, 2, SQL_C_ULONG, &t, 0, NULL); I->attr[0] = t;
		retCode = SQLGetData(hStmt, 3, SQL_C_ULONG, &t, 0, NULL); I->attr[1] = t;
		retCode = SQLGetData(hStmt, 4, SQL_C_ULONG, &t, 0, NULL); I->attr[2] = t;
		retCode = SQLGetData(hStmt, 5, SQL_C_ULONG, &t, 0, NULL); I->attr[3] = t;
		retCode = SQLGetData(hStmt, 6, SQL_C_ULONG, &t, 0, NULL); I->attr[4] = t;
		retCode = SQLGetData(hStmt, 7, SQL_C_ULONG, &t, 0, NULL); I->attr[5] = t;
		SQLFreeStmt(hStmt, SQL_DROP);
		
		if(retCode == SQL_SUCCESS)
			return(1);
		else 
			return(-2);
	}
	SQLFreeStmt(hStmt, SQL_DROP);
	return(-3);
};

/*
int PutBoxItem(int id, int x, int y, UCHAR **I)
{
int			pos;
SQLLEN		cbValue;
SQLLEN	cbItemLen;
SQLPOINTER	pParamData;
SQLRETURN	retCode;
SQLHSTMT	hstmt;
char		szQuerry[255];

		pos = (x * 5) + y;
		if(pos < 0 || pos > 14)
		return(-1);
		
		  sprintf(szQuerry, "update box_item set item_%02d=? where id=%d", pos+1, id);
		  
			SQLAllocStmt(hDBC, &hstmt);
			retCode = SQLPrepare(hstmt, (UCHAR *)szQuerry, SQL_NTS);
			if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO) 
			{
			retCode = SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_BINARY, SQL_LONGVARBINARY, 26+1, 0, I, 0, &cbValue); 
			if(retCode != SQL_SUCCESS && retCode != SQL_SUCCESS_WITH_INFO)
			{
			SQLFreeStmt(hstmt, SQL_DROP);
			return(-2);
			}
			
			  cbItemLen = SQL_LEN_DATA_AT_EXEC(0);
			  
				retCode = SQLExecute(hstmt);
				
				  while(retCode == SQL_NEED_DATA) 
				  {      
				  retCode = SQLParamData(hstmt, &pParamData);
				  if(retCode == SQL_NEED_DATA) 
				  {
				  if(pParamData == I) 
				  SQLPutData(hstmt, I, 26);
				  }
				  else 
				  break;
				  }
				  
					retCode = SQLExecute(hstmt);
					
					  if(retCode == SQL_NEED_DATA)
					  {
					  retCode = SQLCancel(hstmt);
					  retCode = SQLFreeStmt(hstmt, SQL_RESET_PARAMS);
					  //				retCode = SQLTransact(SQL_NULL_HENV, hDBC, SQL_COMMIT);
					  
						return(1);
						}
						else
						{
						//				SQLTransact(SQL_NULL_HENV, hDBC, SQL_ROLLBACK);
						SQLFreeStmt(hstmt, SQL_DROP);
						
						  return(-4); 
						  }
						  }
						  
							return(-5);
							};
							
							  int GetBoxItem(int id, int x, int y, UCHAR **I)
							  {
							  int			pos;
							  SQLLEN		cbValue;
							  HSTMT		hStmt = NULL;
							  RETCODE		retCode;
							  char		szQuerry[255];
							  
								pos = (x * 5) + y;
								if(pos < 0 || pos > 14)
								return(-1);
								
								  sprintf(szQuerry, "select item_%02d from box_item where id=%d", pos+1, id);
								  
									SQLAllocStmt(hDBC, &hStmt);
									retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
									if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
									{
									retCode = SQLFetch(hStmt);
									retCode = SQLGetData(hStmt, 1, SQL_C_BINARY, I, 26, &cbValue);
									SQLFreeStmt(hStmt, SQL_DROP);
									
									  if(retCode == SQL_SUCCESS)
									  return(1);
									  else 
									  return(-2);
									  }
									  else 
									  return(-3);
									  };
									  */
									  
									  int PutBoxItemId_SQL( int box_id )
									  {
										  HSTMT		hStmt = NULL;
										  RETCODE		retCode;
										  char		szQuerry[255];
										  
										  sprintf(szQuerry, "update BoxItemIDCount set BoxItemIDCount=%d", box_id);
										  SQLAllocStmt(hDBC, &hStmt);
										  retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
										  if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
										  {
											  //		SQLTransact(SQL_NULL_HENV, hDBC, SQL_COMMIT );
											  SQLFreeStmt(hStmt, SQL_DROP);
											  return(1);
										  }
										  else 
										  {
											  //		SQLTransact(SQL_NULL_HENV, hDBC, SQL_ROLLBACK);
											  SQLFreeStmt(hStmt, SQL_DROP);
											  return(-2);
										  }
									  };
									  
									  
									  int GetBoxItemId_SQL( )
									  {
										  SQLLEN		cbValue;
										  HSTMT		hStmt = NULL;
										  RETCODE		retCode;
										  char		szQuerry[255];
										  
										  int Box_id;
										  
										  //sprintf(szQuerry, "select BoxItemIDCount from BoxItemIDCount");
										  /////////////////////////////////////////////////////////////////////////////
										  sprintf(szQuerry, "CALL up_get_box_item_id_count");
										  /////////////////////////////////////////////////////////////////////////////
										  SQLAllocStmt(hDBC, &hStmt);
										  retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
										  if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
										  {
											  retCode = SQLFetch(hStmt);
											  retCode = SQLGetData(hStmt, 1, SQL_C_ULONG, &Box_id, 0, &cbValue);
											  SQLFreeStmt(hStmt, SQL_DROP);
											  
											  if(retCode == SQL_SUCCESS)
											  {
												  Box_id++;
												  PutBoxItemId_SQL( Box_id );
												  return(Box_id);
											  }
											  
											  SQLFreeStmt(hStmt, SQL_DROP);
											  return(-2);
										  }
										  SQLFreeStmt(hStmt, SQL_DROP);
										  return(-3);
									  };
									  
									  
									  
									  
									  
									  
									  
									  
									  
									  
									  
									  
									  
									  
									  /////////////////////  PARTY, RELATION, EMPLOYMENT 
									  /*typedef struct tagCharacterParty
									  {
									  int		m_Id;
									  char	m_Name[ 31];
									  int		m_ImageNo;
									  int		m_Lv;
									  int		m_Str;
									  int		m_Class;
									  }CharacterParty, *LpCharacterParty;
									  */
									  
									  
									  //����ý�����?�κ����� Party, Relation, Employment�� Data���о�´�? 
									  
									  int GetPartyChar_SQL( char *name, int *Level, int *Face, int *Str, int *Class, int *Gender )		// 0115
									  {
										  HSTMT		hStmt = NULL;
										  RETCODE		retCode;
										  SQLLEN		cbValue;
										  char		szQuerry[1024];
										  DWORD		t;
										  
										  //sprintf(szQuerry, "Select  face, lev, str, class, gender "
										  //"From chr_info Where name='%s'", name );
										  /////////////////////////////////////////////////////////////////////////////
										  sprintf(szQuerry, "CALL up_get_party_char( '%s')", name);
										  /////////////////////////////////////////////////////////////////////////////
										  SQLAllocStmt(hDBC, &hStmt);
										  retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
										  if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
										  {
											  retCode = SQLFetch(hStmt);
											  
											  if(retCode == SQL_SUCCESS)
											  {
												  // "Select  BankMoney, LastTotal, LastLoan, LastLoan_time,"
												  retCode = SQLGetData(hStmt,  1, SQL_C_ULONG, &t, 0, &cbValue);		*Face	= (short )t;
												  retCode = SQLGetData(hStmt,  2, SQL_C_ULONG, &t, 0, &cbValue);		*Level	= (short )t;
												  retCode = SQLGetData(hStmt,  3, SQL_C_ULONG, &t, 0, &cbValue);		*Str	= (short )t;
												  retCode = SQLGetData(hStmt,  4, SQL_C_ULONG, &t, 0, &cbValue);		*Class	= (short )t;
												  retCode = SQLGetData(hStmt,  5, SQL_C_ULONG, &t, 0, &cbValue);		*Gender	= (short )t;
												  SQLFreeStmt(hStmt, SQL_DROP);
												  return(1);
											  }
											  
											  SQLFreeStmt(hStmt, SQL_DROP);
											  return(-2);
										  }
										  SQLFreeStmt(hStmt, SQL_DROP);
										  return(-1);
									  };
									  
									  
									  
									  
									  
									  
									  
									  
									  
									  
									  
									  
									  //-----------------------------------------
									  
									  int UpdateTotalMapConnections( char *mapname, int no )
									  {
										  SQLRETURN retcode;
										  SQLHSTMT  hstmt;
										  char query[128];
										  
										  sprintf(query, "UPDATE MapName SET HowMany=%d  where MapName= '%s'", no, mapname );
										  SQLAllocStmt(hDBC, &hstmt) ;
										  retcode = SQLExecDirect(hstmt, (UCHAR *)query, SQL_NTS);
										  if(retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO) 
										  {
											  displaySQLError(hstmt) ;
											  //		SQLTransact(SQL_NULL_HENV, hDBC, SQL_ROLLBACK);
											  SQLFreeStmt(hstmt, SQL_DROP);
											  return -1 ;
										  }	
										  else {
											  //		SQLTransact(SQL_NULL_HENV, hDBC, SQL_COMMIT);
											  SQLFreeStmt(hstmt, SQL_DROP) ;
										  }	
										  
										  return 1;
									  }
									  
									  int TotalConnections( void )
									  {
										  SQLRETURN retCode;
										  SQLHSTMT  hStmt;
										  char szQuerry[128];
										  SQLLEN		cbValue;
										  int totalno = 0;
										  int no;
										  
										  //sprintf( szQuerry, "SELECT HowMany FROM MapName" );
										  /////////////////////////////////////////////////////////////////////////////
										  sprintf(szQuerry, "CALL up_get_how_many");
										  /////////////////////////////////////////////////////////////////////////////
										  
										  SQLAllocStmt(hDBC, &hStmt);
										  retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
										  if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
										  {
											  retCode = SQLFetch(hStmt);
											  while( retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
											  {
												  retCode = SQLGetData(hStmt, 1, SQL_C_LONG, &no,	0, &cbValue);
												  if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
												  {
													  totalno += no;
													  retCode = SQLFetch(hStmt);
												  }
												  else 
												  {
													  SQLFreeStmt(hStmt, SQL_DROP);
													  return 0;
												  }
											  }
										  }
										  SQLFreeStmt(hStmt, SQL_DROP);
										  return totalno;
									  }
									  
									  
									  //---------------------------------------------
									  
									  //  DB�� Character�̸��� �ִ��� ������ �˾ƺ���.
									  // ������ 
									  
									  
									  
									  
									  void RecvIsThereCharName( int cn, char *name )
									  {
										  t_packet p;
										  char normalizedName[NM_LENGTH] = {0};
										  const bool validName = NormalizeCharacterName(name, normalizedName);
										  const bool exists = validName && CheckName_SQL(normalizedName) == 1;
										  const bool blocked = validName &&
											  (SearchStrWord(normalizedName) || SearchStrStr(normalizedName, " "));
										  if(!validName || exists || blocked) // �̸��� ������... // 0208 YGI
										  {
											  MyLog(LOG_NORMAL, "[CREATE_CHAR] Name rejected: valid=%d exists=%d blocked=%d",
												  validName, exists, blocked);
											  p.h.header.type = CMD_THEREIS_CHARNAME;
											  p.h.header.size = 0;
											  QueuePacket( connections, cn, &p, 1 );
										  }
										  else	// �̸��� ������..
										  {
											  MyLog(LOG_NORMAL, "[CREATE_CHAR] Name available");
											  p.h.header.type = CMD_THEREIS_NO_CHARNAME;
											  p.h.header.size = 0;
											  QueuePacket( connections, cn, &p, 1 );
										  }
									  }	
									  
									  DiseaseTable	disease_tbl[6];
									  // ���� ���̺� ~~~		0104
									  int GetDisease_SQL( )
									  {	
										  HSTMT		hStmt = NULL;
										  RETCODE		retCode;
										  SQLLEN		cbValue;
										  char		szQuerry[2047];
										  DWORD		t;
										  //char		name[31];
										  
										  for( int a=0; a<6; a++ )
										  {
										  /*		sprintf(szQuerry, "Select disease_number, han_name, name, lv, nut1_1, nut2_1, nut3_1, probability_1, andor_1, checktime_1, contact_char, talk_char, fuck_char,"
										  "contact_obj,eat_obj,nut1_2,nut2_2,nut3_2,probability_2,checktime_2,andor_2,dec_str,dec_dex,dec_con,dec_wis,dec_int,dec_movp,dec_char,"
										  "dec_endu,dec_mor,dec_luck,dec_ws,dec_ps,recoverable,life,health,mp,dp,max_,damage,att_rate,depen_rate,ac,mad,poison,stony,sluggish,active,"
										  "paralysis,active_time,maintain_time,natural_recovery,nut1_3,nut2_3,nut3_3,probability_3,checktime_3,andor_3,price_heal,price_prevent"
										  "From DiseaseMain Where disease_number=%d ", a+1);
											  */
											  //sprintf(szQuerry, "Select *"
											  //"From DiseaseMain Where disease_number=%d ", a+1);
											  /////////////////////////////////////////////////////////////////////////////
											  sprintf(szQuerry, "CALL up_get_disease_main( %d)", a+1);
											  /////////////////////////////////////////////////////////////////////////////
											  SQLAllocStmt(hDBC, &hStmt);
											  retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
											  if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
											  {
												  retCode = SQLFetch(hStmt);
												  if(retCode == SQL_SUCCESS)
												  {
													  int cn = 1;
													  retCode = SQLGetData(hStmt,  cn++, SQL_C_ULONG, &t, 0, &cbValue);	disease_tbl[a].disease_number	= (short )t;
													  retCode = SQLGetData(hStmt,  cn++, SQL_C_CHAR,	 disease_tbl[a].han_name, 31, &cbValue);		
													  retCode = SQLGetData(hStmt,  cn++, SQL_C_CHAR,  disease_tbl[a].name, 31, &cbValue);			
													  retCode = SQLGetData(hStmt,  cn++, SQL_C_ULONG, &t, 0, &cbValue);	disease_tbl[a].lv		= (short )t;
													  retCode = SQLGetData(hStmt,  cn++, SQL_C_ULONG, &t, 0, &cbValue);	disease_tbl[a].nut1_1		= (short )t;
													  retCode = SQLGetData(hStmt,  cn++, SQL_C_ULONG, &t, 0, &cbValue);	disease_tbl[a].nut2_1		= (short )t;
													  retCode = SQLGetData(hStmt,  cn++, SQL_C_ULONG, &t, 0, &cbValue);	disease_tbl[a].nut3_1		= (short )t;
													  retCode = SQLGetData(hStmt,  cn++, SQL_C_ULONG, &t, 0, &cbValue);	disease_tbl[a].probability_1= (short )t;
													  retCode = SQLGetData(hStmt,  cn++, SQL_C_ULONG, &t, 0, &cbValue);	disease_tbl[a].andor_1		= (short )t;
													  retCode = SQLGetData(hStmt,  cn++, SQL_C_ULONG, &t, 0, &cbValue);	disease_tbl[a].checktime_1	= (short )t;
													  retCode = SQLGetData(hStmt,  cn++, SQL_C_ULONG, &t, 0, &cbValue);	disease_tbl[a].contact_char	= (short )t;
													  retCode = SQLGetData(hStmt,  cn++, SQL_C_ULONG, &t, 0, &cbValue);	disease_tbl[a].talk_char	= (short )t;
													  retCode = SQLGetData(hStmt,  cn++, SQL_C_ULONG, &t, 0, &cbValue);	disease_tbl[a].fuck_char	= (short )t;
													  retCode = SQLGetData(hStmt,  cn++, SQL_C_ULONG, &t, 0, &cbValue);	disease_tbl[a].contact_obj	= (short )t;
													  retCode = SQLGetData(hStmt,  cn++, SQL_C_ULONG, &t, 0, &cbValue);	disease_tbl[a].eat_obj		= (short )t;
													  retCode = SQLGetData(hStmt,  cn++, SQL_C_ULONG, &t, 0, &cbValue);	disease_tbl[a].nut1_2		= (short )t;
													  retCode = SQLGetData(hStmt,  cn++, SQL_C_ULONG, &t, 0, &cbValue);	disease_tbl[a].nut2_2		= (short )t;
													  retCode = SQLGetData(hStmt,  cn++, SQL_C_ULONG, &t, 0, &cbValue);	disease_tbl[a].nut3_2		= (short )t;
													  retCode = SQLGetData(hStmt,  cn++, SQL_C_ULONG, &t, 0, &cbValue);	disease_tbl[a].probability_2= (short )t;
													  retCode = SQLGetData(hStmt,  cn++, SQL_C_ULONG, &t, 0, &cbValue);	disease_tbl[a].checktime_2	= (short )t;
													  retCode = SQLGetData(hStmt,  cn++, SQL_C_ULONG, &t, 0, &cbValue);	disease_tbl[a].andor_2		= (short )t;
													  retCode = SQLGetData(hStmt,  cn++, SQL_C_ULONG, &t, 0, &cbValue);	disease_tbl[a].dec_str		= (short )t;
													  retCode = SQLGetData(hStmt,  cn++, SQL_C_ULONG, &t, 0, &cbValue);	disease_tbl[a].dec_dex		= (short )t;
													  retCode = SQLGetData(hStmt,  cn++, SQL_C_ULONG, &t, 0, &cbValue);	disease_tbl[a].dec_con		= (short )t;
													  retCode = SQLGetData(hStmt,  cn++, SQL_C_ULONG, &t, 0, &cbValue);	disease_tbl[a].dec_wis		= (short )t;
													  retCode = SQLGetData(hStmt,  cn++, SQL_C_ULONG, &t, 0, &cbValue);	disease_tbl[a].dec_int		= (short )t;
													  retCode = SQLGetData(hStmt,  cn++, SQL_C_ULONG, &t, 0, &cbValue);	disease_tbl[a].dec_movp		= (short )t;
													  retCode = SQLGetData(hStmt,  cn++, SQL_C_ULONG, &t, 0, &cbValue);	disease_tbl[a].dec_char		= (short )t;
													  retCode = SQLGetData(hStmt,  cn++, SQL_C_ULONG, &t, 0, &cbValue);	disease_tbl[a].dec_endu		= (short )t;
													  retCode = SQLGetData(hStmt,  cn++, SQL_C_ULONG, &t, 0, &cbValue);	disease_tbl[a].dec_mor		= (short )t;
													  retCode = SQLGetData(hStmt,  cn++, SQL_C_ULONG, &t, 0, &cbValue);	disease_tbl[a].dec_luck		= (short )t;
													  retCode = SQLGetData(hStmt,  cn++, SQL_C_ULONG, &t, 0, &cbValue);	disease_tbl[a].dec_ws		= (short )t;
													  retCode = SQLGetData(hStmt,  cn++, SQL_C_ULONG, &t, 0, &cbValue);	disease_tbl[a].dec_ps		= (short )t;
													  retCode = SQLGetData(hStmt,  cn++, SQL_C_ULONG, &t, 0, &cbValue);	disease_tbl[a].recoverable	= (short )t;
													  retCode = SQLGetData(hStmt,  cn++, SQL_C_ULONG, &t, 0, &cbValue);	disease_tbl[a].life			= (short )t;
													  retCode = SQLGetData(hStmt,  cn++, SQL_C_ULONG, &t, 0, &cbValue);	disease_tbl[a].health		= (short )t;
													  retCode = SQLGetData(hStmt,  cn++, SQL_C_ULONG, &t, 0, &cbValue);	disease_tbl[a].mp			= (short )t;
													  retCode = SQLGetData(hStmt,  cn++, SQL_C_ULONG, &t, 0, &cbValue);	disease_tbl[a].dp			= (short )t;
													  retCode = SQLGetData(hStmt,  cn++, SQL_C_ULONG, &t, 0, &cbValue);	disease_tbl[a].max_			= (short )t;
													  retCode = SQLGetData(hStmt,  cn++, SQL_C_ULONG, &t, 0, &cbValue);	disease_tbl[a].damage		= (short )t;
													  retCode = SQLGetData(hStmt,  cn++, SQL_C_ULONG, &t, 0, &cbValue);	disease_tbl[a].att_rate		= (short )t;
													  retCode = SQLGetData(hStmt,  cn++, SQL_C_ULONG, &t, 0, &cbValue);	disease_tbl[a].depen_rate	= (short )t;
													  retCode = SQLGetData(hStmt,  cn++, SQL_C_ULONG, &t, 0, &cbValue);	disease_tbl[a].ac			= (short )t;
													  retCode = SQLGetData(hStmt,  cn++, SQL_C_ULONG, &t, 0, &cbValue);	disease_tbl[a].mad			= (short )t;
													  retCode = SQLGetData(hStmt,  cn++, SQL_C_ULONG, &t, 0, &cbValue);	disease_tbl[a].poison		= (short )t;
													  retCode = SQLGetData(hStmt,  cn++, SQL_C_ULONG, &t, 0, &cbValue);	disease_tbl[a].stony		= (short )t;
													  retCode = SQLGetData(hStmt,  cn++, SQL_C_ULONG, &t, 0, &cbValue);	disease_tbl[a].sluggish		= (short )t;
													  retCode = SQLGetData(hStmt,  cn++, SQL_C_ULONG, &t, 0, &cbValue);	disease_tbl[a].active		= (short )t;
													  retCode = SQLGetData(hStmt,  cn++, SQL_C_ULONG, &t, 0, &cbValue);	disease_tbl[a].paralysis	= (short )t;
													  retCode = SQLGetData(hStmt,  cn++, SQL_C_ULONG, &t, 0, &cbValue);	disease_tbl[a].active_time	= (short )t;
													  retCode = SQLGetData(hStmt,  cn++, SQL_C_ULONG, &t, 0, &cbValue);	disease_tbl[a].maintain_time	= (short )t;
													  retCode = SQLGetData(hStmt,  cn++, SQL_C_ULONG, &t, 0, &cbValue);	disease_tbl[a].natural_recovery	= (short )t;
													  retCode = SQLGetData(hStmt,  cn++, SQL_C_ULONG, &t, 0, &cbValue);	disease_tbl[a].nut1_3			= (short )t;
													  retCode = SQLGetData(hStmt,  cn++, SQL_C_ULONG, &t, 0, &cbValue);	disease_tbl[a].nut2_3			= (short )t;
													  retCode = SQLGetData(hStmt,  cn++, SQL_C_ULONG, &t, 0, &cbValue);	disease_tbl[a].nut3_3			= (short )t;
													  retCode = SQLGetData(hStmt,  cn++, SQL_C_ULONG, &t, 0, &cbValue);	disease_tbl[a].probability_3	= (short )t;
													  retCode = SQLGetData(hStmt,  cn++, SQL_C_ULONG, &t, 0, &cbValue);	disease_tbl[a].checktime_3		= (short )t;
													  retCode = SQLGetData(hStmt,  cn++, SQL_C_ULONG, &t, 0, &cbValue);	disease_tbl[a].andor_3			= (short )t;
													  retCode = SQLGetData(hStmt,  cn++, SQL_C_ULONG, &t, 0, &cbValue);	disease_tbl[a].price_heal		= (short )t;
													  retCode = SQLGetData(hStmt,  cn++, SQL_C_ULONG, &t, 0, &cbValue);	disease_tbl[a].price_prevent	= (short )t;
													  
													  
													  SQLFreeStmt(hStmt, SQL_DROP);
													  
													  //				 return(1);
												  }
												  else
												  {
													  SQLFreeStmt(hStmt, SQL_DROP);
													  return(-2);
												  }
											  }
											  else
											  {
												  SQLFreeStmt(hStmt, SQL_DROP);
												  return(-1);
											  }
										  }
										  
										  FILE *fp;
										  
										  fp = fopen( "./Output/disease_tbl.bin", "wb" );
										  if( fp )
										  {
											  fwrite( disease_tbl, 6, sizeof(DiseaseTable), fp );
											  fclose(fp);
										  }
										  
										  return 1;
}

int UpdateCharID_SQL( int char_id )
{
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	char		szQuerry[ 128];
	
	sprintf(szQuerry, "update CharIDCount set CharIDCount=%d", char_id);
	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{
		SQLFreeStmt(hStmt, SQL_DROP);
		return(1);
	}
	else 
	{
		//		SQLTransact(SQL_NULL_HENV, hDBC, SQL_ROLLBACK);
		SQLFreeStmt(hStmt, SQL_DROP);
		return(-2);
	}
};

int GetCharID_SQL( void  )
{
	HSTMT statement = NULL;
	SQLLEN length = 0;
	SQLLEN affected = 0;
	DWORD charId = 0;

	// LAST_INSERT_ID(expr) is connection-local and makes the increment atomic.
	if (!SQL_SUCCEEDED(SQLAllocStmt(hDBC, &statement))) return 0;
	RETCODE result = SQLExecDirect(statement,
		(UCHAR*)"UPDATE CharIDCount SET CharIDCount = LAST_INSERT_ID("
		"GREATEST(CharIDCount, (SELECT COALESCE(MAX(total_id), 0) FROM chr_info)) + 1) "
		"LIMIT 1", SQL_NTS);
	if (!SQL_SUCCEEDED(result) || !SQL_SUCCEEDED(SQLRowCount(statement, &affected)))
	{
		displaySQLError(statement);
		SQLFreeStmt(statement, SQL_DROP);
		return 0;
	}
	SQLFreeStmt(statement, SQL_CLOSE);
	if (affected == 0)
	{
		result = SQLExecDirect(statement,
			(UCHAR*)"INSERT INTO CharIDCount (CharIDCount) "
			"SELECT COALESCE(MAX(total_id), 0) + 1 FROM chr_info", SQL_NTS);
		if (!SQL_SUCCEEDED(result))
		{
			displaySQLError(statement);
			SQLFreeStmt(statement, SQL_DROP);
			return 0;
		}
		SQLFreeStmt(statement, SQL_CLOSE);
		result = SQLExecDirect(statement,
			(UCHAR*)"SELECT CharIDCount FROM CharIDCount LIMIT 1", SQL_NTS);
	}
	else if (affected == 1)
	{
		result = SQLExecDirect(statement, (UCHAR*)"SELECT LAST_INSERT_ID()", SQL_NTS);
	}
	else
	{
		MyLog(LOG_FATAL, "[CREATE_CHAR] CharIDCount contains multiple rows");
		SQLFreeStmt(statement, SQL_DROP);
		return 0;
	}

	if (SQL_SUCCEEDED(result) && SQL_SUCCEEDED(SQLFetch(statement)) &&
		SQL_SUCCEEDED(SQLGetData(statement, 1, SQL_C_ULONG, &charId,
		                         sizeof(charId), &length)) &&
		length != SQL_NULL_DATA && charId != 0)
	{
		SQLFreeStmt(statement, SQL_DROP);
		return charId;
	}
	displaySQLError(statement);
	SQLFreeStmt(statement, SQL_DROP);
	return 0;
};




///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////

// BBS���� �ҽ�...



//#define MAX_DATA  5000
//	char enter[ MAX_DATA];
//	FILE *fp = fopen( "t.txt", "rt" );
//	DWORD filelen = filelength( fileno(fp));
//	fread(	enter, filelen, 1, fp );
//	fclose(fp);



int InsertBBS( char *name, char *title, char *contents )
{
	HSTMT hStmt = NULL;
	RETCODE retCode;
	char szQuerry[128];
	
	sprintf(szQuerry,	"insert into bbs ( date, name, title ) "
		"values ('%d/%d/%d %d:%d:%d', '%s', '%s' )", 
		g_mon+1, g_day, g_year - 2000, g_hour, g_min, g_sec, name, title );
	
	// ĳ������ �Ϲ� Data�� chr_info�� �����ϰ� ( 1-1�ܰ� )
	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{
		//		SQLTransact(SQL_NULL_HENV, hDBC, SQL_COMMIT);
		SQLFreeStmt(hStmt, SQL_DROP);
	}
	else
	{
		//		SQLTransact(SQL_NULL_HENV, hDBC, SQL_ROLLBACK);
		SQLFreeStmt(hStmt, SQL_DROP);
		return(0); // unknown error
	}
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	SQLLEN  cbdata = 200, cbvalue = SQL_DATA_AT_EXEC ;
	SQLLEN	cbcontentslen;
	SQLRETURN	retcode;
	SQLHSTMT	hstmt;
	SQLPOINTER	pParamData;
	int			n=0;
	char		aaa=20;
	DWORD		filelen;
	
	filelen = strlen( contents );
	
	sprintf(szQuerry, "UPDATE bbs SET con=? WHERE date='%d/%d/%d %d:%d:%d'", g_mon+1, g_day, g_year - 2000, g_hour, g_min, g_sec );
	
	//printf("\nSQL: %s", query);
	SQLAllocStmt(hDBC, &hstmt);
	retcode = SQLPrepare(hstmt, (UCHAR *)szQuerry, SQL_NTS);
	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) 
	{
		retcode= SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR,SQL_LONGVARCHAR, filelen,	0, contents,			0, &cbcontentslen); 
		
		if(retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO)
		{
			SQLFreeStmt(hstmt, SQL_DROP);
			printf("\nBinding FAIL!!!") ;
			return -5 ;
		}
		
		cbcontentslen		= SQL_LEN_DATA_AT_EXEC(0);
		retcode = SQLExecute(hstmt) ;
		
		while(retcode == SQL_NEED_DATA) 
		{      
			retcode = SQLParamData(hstmt, &pParamData);
			if(retcode == SQL_NEED_DATA) 
			{
				if(pParamData == contents ) 	SQLPutData(hstmt, contents, filelen );
			}
			else break ;
		}
		
		retcode = SQLExecute(hstmt) ;
		
		if(retcode == SQL_NEED_DATA)
		{
			retcode= SQLCancel(hstmt) ;
			retcode= SQLFreeStmt(hstmt, SQL_RESET_PARAMS);
			//			retcode= SQLTransact(SQL_NULL_HENV, hDBC, SQL_COMMIT);
			if(retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO) {
				printf("\n Transact commit Error") ;
			}
			//printf("\nTransact & free") ;
			return(1); // succeed
		}
		else
		{
			displaySQLError(hstmt) ;
			//			SQLTransact(SQL_NULL_HENV, hDBC, SQL_ROLLBACK);
			SQLFreeStmt(hstmt, SQL_DROP);
			return(-3); 
		}
	}	
	//////////////////////////////////////////////////////////////////////////////////////////////////////
	return 0;
}		


// 0305 YGI
int GetTitleBBS( int count, int ct[10], TIMESTAMP_STRUCT date[10], char name[10][ FILENAME_MAX], char title[10][ FILENAME_MAX] )
{		
	SQLLEN		cbValue;
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	char		szQuerry[128];
	DWORD		bbs_count=0;		// ���������� �� ����
	int max=10;		
	
	//sprintf( szQuerry, "SELECT no FROM bbs ORDER BY no DESC" );
	/////////////////////////////////////////////////////////////////////////////
	sprintf(szQuerry, "CALL  up_get_bbs_no"); 
	/////////////////////////////////////////////////////////////////////////////
	
	SQLAllocStmt(hDBC, &hStmt);
	
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{	
		retCode = SQLFetch( hStmt );
		retCode = SQLGetData(hStmt, 1, SQL_C_ULONG,  &bbs_count, 0, &cbValue);
		SQLFreeStmt(hStmt, SQL_DROP);
		
		bbs_count -= count*10;
		if( bbs_count > 0 && bbs_count < 10 )
		{
			max = bbs_count;
		}
		else if( bbs_count < 0 ) return 0;
		
		//sprintf( szQuerry, "select no, name, title, date from bbs order by no DESC" );
		/////////////////////////////////////////////////////////////////////////////
		sprintf(szQuerry, "CALL up_get_bbs_info");
		/////////////////////////////////////////////////////////////////////////////
		SQLAllocStmt(hDBC, &hStmt);
		retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
		if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
		{	
			for( int i = 0 ; i < max ; i ++)
			{
				retCode = SQLFetch( hStmt );
				retCode = SQLGetData(hStmt, 1, SQL_C_ULONG,		&ct[i],	0, &cbValue);
				retCode = SQLGetData(hStmt, 2, SQL_C_CHAR,		&name[i],  FILENAME_MAX, &cbValue);
				retCode = SQLGetData(hStmt, 3, SQL_C_CHAR,		&title[i], FILENAME_MAX, &cbValue);
				retCode = SQLGetData(hStmt, 4, SQL_C_TIMESTAMP, &date[i],  sizeof( TIMESTAMP_STRUCT ), &cbValue);
			}
		}
		
		if(retCode == SQL_SUCCESS)
		{
			SQLFreeStmt(hStmt, SQL_DROP);
			return ( max );			// ������ �������� ������ �������ش�.
		}
		else 
		{
			SQLFreeStmt(hStmt, SQL_DROP);		// 0308 YGI
			return( 0 );
		}
	}	
	SQLFreeStmt(hStmt, SQL_DROP);		// 0308 YGI
	return( 0 );
}		



int GetContentsBBS( char *date, char *con )
{
	SQLLEN		cbValue;
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	char		szQuerry[128];
	
	//sprintf( szQuerry, "select con from bbs where date = '%s'", date );
	/////////////////////////////////////////////////////////////////////////////
	sprintf(szQuerry, "CALL up_get_bbs_con( '%s')", date);
	/////////////////////////////////////////////////////////////////////////////			
	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{	
		//if( count == 0 )	
		retCode = SQLFetch( hStmt );
		retCode = SQLGetData(hStmt, 1, SQL_C_CHAR,		con, 0, &cbValue);
		
		if(retCode == SQL_SUCCESS)
		{
			SQLFreeStmt(hStmt, SQL_DROP);
			return (1);
		}
		else 
		{
			SQLFreeStmt(hStmt, SQL_DROP);		// 0308 YGI
			return( 0 );
		}
	}
	SQLFreeStmt(hStmt, SQL_DROP);		// 0308 YGI
	return 1;
}	





//1016 YGI		
int get_BinaryData_Party( UCHAR **party, const char *name)
{	
	SQLLEN  cbvalue = SQL_C_BINARY ;
	SQLRETURN retcode;
	SQLHSTMT  hstmt;
	/* Prepare the SQL statement with parameter markers. */
	
	char query[256]= {0, } ;
	
	//sprintf(query, "SELECT ws, ps, inventory, FROM chr_info where name='%s' ",
	sprintf(query, "SELECT party FROM chr_info where name='%s' ",name) ;
	
	SQLAllocStmt(hDBC, &hstmt);
	retcode = SQLExecDirect(hstmt, (UCHAR *)query, SQL_NTS);
	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
		retcode = SQLFetch(hstmt);
		if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
			retcode= SQLGetData(hstmt,1, SQL_C_BINARY, party,			SIZE_OF_PARTY,			&cbvalue);
		}
		else 
		{
			printf("\n Fetch Error") ;
			SQLFreeStmt(hstmt, SQL_DROP);
			return -1 ;
		}
	}
	else {
		printf("\n ExecDirect Error") ;
		SQLFreeStmt(hstmt, SQL_DROP);
		return -2 ;
	}
	
	
	if(retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
	{
		//		SQLTransact(SQL_NULL_HENV, hDBC, SQL_COMMIT);
		SQLFreeStmt(hstmt, SQL_DROP);
		return(1); // succeed
	}
	else
	{
		displaySQLError(hstmt) ;
		//		SQLTransact(SQL_NULL_HENV, hDBC, SQL_ROLLBACK);
		SQLFreeStmt(hstmt, SQL_DROP);
		return(-3); // unknown error
	}
	
}	


//////////////////////// 0130 YGI //////////////////////
int CheckBetaId_SQL(LPSTR szUNM)
{	
	char		szQuerry[255];
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	
	//sprintf(szQuerry, "select id from beta_id where id='%s'", szUNM);
	/////////////////////////////////////////////////////////////////////////////
	sprintf(szQuerry, "CALL up_check_beta_id( '%s')", szUNM);
	/////////////////////////////////////////////////////////////////////////////
	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{
		retCode = SQLFetch(hStmt);
		SQLFreeStmt(hStmt, SQL_DROP);
		
		if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
			return(1);
		else	return(0);
	}
	
	return(0); // Invalid id
};	
////////////////////////////////////////////////////////////

void CheckBetaIdTest( int cn, char *id )
{	
	t_packet packet;
	if( CheckBetaId_SQL( id ) ) 
	{
		packet.h.header.type = CMD_CHECK_BETA_TEST_OK;
	}
	else
	{
		packet.h.header.type = CMD_CHECK_BETA_TEST_NOT;
	}
	packet.h.header.size = sizeof(0);
	QueuePacket(connections, cn, &packet, 1);
}	




///////////////////////////// 0131 YGI ///////////////////////////
int CheckNoName_SQL(LPSTR szUNM)
{
	char		szQuerry[255];
	
	sprintf(szQuerry, "select noid from noid where noid='%s'", szUNM);
	if(Querry_SQL(szQuerry) == 1)
		return(1); // OK
	else
		return(0); // Invalid Name
};






//////////////////////////////////////////////////////////////// 0208 YGI
//
// ���ڿ��� Ư�� ���ڿ��� ������ ���ڰ� �ִ����� ã�� �޼ҵ� 
//
// return : ���� ���?true, ���� ���?false�� ����
//
///////////////////////////////////////////////////////////////
bool SearchStrStr( const char *szSource, const char *szWhite )
{
	int len = strlen( szWhite );
	
	for( int i=0; i<len; i++ )
	{
		if( strchr( szSource, szWhite[i] ) ) return true; 
	}
	
	return false;
}





//// 0209 YGI ////////////

#define NO_NAME_MAX_	1000

char no_name[NO_NAME_MAX_][20];
int no_name_count;

/////////////////////////////////////////////////////////////////
//
//		0209 YGI	���ڿ����� ���� ���� ���ڿ� �ִ��� ������ �Ǵ�.
//
/////////////////////////////////////////////////////////////
// 010301 KHS
bool SearchStrWord( const char *szSource )
{
	char tempsource[ FILENAME_MAX];
	
	strcpy( tempsource, szSource );
	CharUpper( tempsource );
	
	for( int i=0; i<no_name_count; i++ )
	{
		if(no_name[i][0] && strstr(tempsource, no_name[i])) return true;
	}
	
	return false;
}


SkillMain SkillTbl[ MAX_SKILLMAIN];

int InitSkillTable( void )
{
	if( MAX_SKILLMAIN <= 0) return -4; 
	
	HSTMT hStmt= NULL ;
	RETCODE ret ;
	char query_stmt[80]= {0, } ;
	SQLLEN cbValue ;
	int c;
	
	SQLAllocStmt(hDBC, &hStmt);
	
	//strcpy(query_stmt, "select * from SkillMain") ;
	/////////////////////////////////////////////////////////////////////////////
	strcpy(query_stmt, "CALL up_get_skill_main");
	/////////////////////////////////////////////////////////////////////////////
	ret= SQLExecDirect(hStmt, (UCHAR *)query_stmt, SQL_NTS) ;
	
	if(ret != SQL_SUCCESS_WITH_INFO && ret != SQL_SUCCESS) {
		printf("\nSkill Init : ExecDirect Error ") ;
		SQLFreeStmt(hStmt, SQL_DROP);
		return -1 ;
	}
	
	//SQLNumResultCols(hStmt, &nCols) ;
	ret= SQLFetch(hStmt) ;
	if(ret != SQL_SUCCESS_WITH_INFO && ret != SQL_SUCCESS) {
		LPVOID lpMsgBuf;
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS,    NULL,
			GetLastError(),
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
			(LPTSTR) &lpMsgBuf,    0,    NULL );// Process any inserts in lpMsgBuf.
		// ...// Display the string.
		printf("\nLast Error: %s", (LPCTSTR)lpMsgBuf) ;
		// Free the buffer.
		LocalFree( lpMsgBuf );
		printf("\nNPC init: Fetch Error ") ;
		return -1 ;
	}
	
	int tno;
	
	c = 1;
	while(c < MAX_SKILLMAIN && ret == SQL_SUCCESS)
	{
		ret = SQLGetData(hStmt,  1, SQL_C_LONG,	&tno,			 0, &cbValue);
		SkillTbl[tno].NO = tno;
		ret = SQLGetData(hStmt,  2, SQL_C_CHAR,		SkillTbl[tno].Skill_Minute,		30, &cbValue);
		ret = SQLGetData(hStmt,  3, SQL_C_LONG,	&SkillTbl[tno].Class_Warrior,	0, &cbValue);
		ret = SQLGetData(hStmt,  4, SQL_C_LONG,	&SkillTbl[tno].Class_Thief	,	0, &cbValue);
		ret = SQLGetData(hStmt,  5, SQL_C_LONG,	&SkillTbl[tno].Class_Archery,	0, &cbValue);
		ret = SQLGetData(hStmt,  6, SQL_C_LONG,	&SkillTbl[tno].Class_Wizard	,	0, &cbValue);
		ret = SQLGetData(hStmt,  7, SQL_C_LONG,	&SkillTbl[tno].Class_Priest	,	0, &cbValue);
		ret = SQLGetData(hStmt,  8, SQL_C_LONG,	&SkillTbl[tno].Select_Farmer,			 0, &cbValue);
		ret = SQLGetData(hStmt,  9, SQL_C_LONG,	&SkillTbl[tno].Select_miner	,			 0, &cbValue);
		ret = SQLGetData(hStmt, 10, SQL_C_LONG,	&SkillTbl[tno].Select_fisher,			 0, &cbValue);
		ret = SQLGetData(hStmt, 11, SQL_C_LONG,	&SkillTbl[tno].Select_Lumberjack,		 0, &cbValue);
		ret = SQLGetData(hStmt, 12, SQL_C_LONG,	&SkillTbl[tno].Select_Butcher,			 0, &cbValue);
		ret = SQLGetData(hStmt, 13, SQL_C_LONG,	&SkillTbl[tno].Select_Herbdigger,		 0, &cbValue);
		ret = SQLGetData(hStmt, 14, SQL_C_LONG,	&SkillTbl[tno].Select_herdman,			 0, &cbValue);
		ret = SQLGetData(hStmt, 15, SQL_C_LONG,	&SkillTbl[tno].Select_Carpenter	,		 0, &cbValue);
		ret = SQLGetData(hStmt, 16, SQL_C_LONG,	&SkillTbl[tno].Select_Blacksmith,		 0, &cbValue);
		ret = SQLGetData(hStmt, 17, SQL_C_LONG,	&SkillTbl[tno].Select_Cooking,			 0, &cbValue);
		ret = SQLGetData(hStmt, 18, SQL_C_LONG,	&SkillTbl[tno].Select_Tailoring	,		 0, &cbValue);
		ret = SQLGetData(hStmt, 19, SQL_C_LONG,	&SkillTbl[tno].Select_Bowcraft_Fletcher, 0, &cbValue);
		ret = SQLGetData(hStmt, 20, SQL_C_LONG,	&SkillTbl[tno].Select_Alchemy	,			 0, &cbValue);
		ret = SQLGetData(hStmt, 21, SQL_C_LONG,	&SkillTbl[tno].Select_Candlemaker	,			 0, &cbValue);
		ret = SQLGetData(hStmt, 22, SQL_C_LONG,	&SkillTbl[tno].Select_Merchant	,			 0, &cbValue);
		ret = SQLGetData(hStmt, 23, SQL_C_LONG,	&SkillTbl[tno].Howto	,			 0, &cbValue);   
		ret = SQLGetData(hStmt, 24, SQL_C_LONG,	&SkillTbl[tno].mouse_change	,			 0, &cbValue);
		ret = SQLGetData(hStmt, 25, SQL_C_LONG,	&SkillTbl[tno].Interface1	,			 0, &cbValue);
		ret = SQLGetData(hStmt, 26, SQL_C_LONG,	&SkillTbl[tno].Interface2	,			 0, &cbValue);
		ret = SQLGetData(hStmt, 27, SQL_C_LONG,	&SkillTbl[tno].Success_interface	,			 0, &cbValue);
		ret = SQLGetData(hStmt, 28, SQL_C_LONG,	&SkillTbl[tno].Fail_Interface	,			 0, &cbValue);
		ret = SQLGetData(hStmt, 29, SQL_C_LONG,	&SkillTbl[tno].Work_target_self	,			 0, &cbValue);
		ret = SQLGetData(hStmt, 30, SQL_C_LONG,	&SkillTbl[tno].Work_target_othercharacter,			 0, &cbValue);
		ret = SQLGetData(hStmt, 31, SQL_C_LONG,	&SkillTbl[tno].Work_target_object	,			 0, &cbValue);
		ret = SQLGetData(hStmt, 32, SQL_C_LONG,	&SkillTbl[tno].Work_target_Item	,			 0, &cbValue);
		ret = SQLGetData(hStmt, 33, SQL_C_LONG,	&SkillTbl[tno].Work_target_earth	,			 0, &cbValue);
		ret = SQLGetData(hStmt, 34, SQL_C_LONG,	&SkillTbl[tno].Work_target_water	,			 0, &cbValue);
		ret = SQLGetData(hStmt, 35, SQL_C_LONG,	&SkillTbl[tno].Work_target_forest	,			 0, &cbValue);
		ret = SQLGetData(hStmt, 36, SQL_C_LONG,	&SkillTbl[tno].Work_target_interface	,			 0, &cbValue);
		ret = SQLGetData(hStmt, 37, SQL_C_LONG,	&SkillTbl[tno].Tool	,			 0, &cbValue);
		ret = SQLGetData(hStmt, 38, SQL_C_LONG,	&SkillTbl[tno].Object_Workshop	,			 0, &cbValue);
		ret = SQLGetData(hStmt, 39, SQL_C_LONG,	&SkillTbl[tno].Object_target	,			 0, &cbValue);
		ret = SQLGetData(hStmt, 40, SQL_C_LONG,	&SkillTbl[tno].Obj_Attribute	,			 0, &cbValue);
		ret = SQLGetData(hStmt, 41, SQL_C_LONG,	&SkillTbl[tno].Resource1	,			 0, &cbValue);
		ret = SQLGetData(hStmt, 42, SQL_C_LONG,	&SkillTbl[tno].Resource1_quantity	,			 0, &cbValue);
		ret = SQLGetData(hStmt, 43, SQL_C_LONG,	&SkillTbl[tno].Resourece2	,			 0, &cbValue);
		ret = SQLGetData(hStmt, 44, SQL_C_LONG,	&SkillTbl[tno].Resource2_quantity	,			 0, &cbValue);
		ret = SQLGetData(hStmt, 45, SQL_C_LONG,	&SkillTbl[tno].Resource3	,			 0, &cbValue);
		ret = SQLGetData(hStmt, 46, SQL_C_LONG,	&SkillTbl[tno].Resource3_quantity	,			 0, &cbValue);
		ret = SQLGetData(hStmt, 47, SQL_C_LONG,	&SkillTbl[tno].Produce_type	,			 0, &cbValue);
		ret = SQLGetData(hStmt, 48, SQL_C_LONG,	&SkillTbl[tno].Produce_Fix	,			 0, &cbValue);
		ret = SQLGetData(hStmt, 49, SQL_C_LONG,	&SkillTbl[tno].Ability_Fix	,			 0, &cbValue);
		ret = SQLGetData(hStmt, 50, SQL_C_LONG,	&SkillTbl[tno].Time_judge	,			 0, &cbValue);
		ret = SQLGetData(hStmt, 51, SQL_C_LONG,	&SkillTbl[tno].Time_Maintain	,			 0, &cbValue);
		ret = SQLGetData(hStmt, 52, SQL_C_LONG,	&SkillTbl[tno].Ability_min	,			 0, &cbValue);
		ret = SQLGetData(hStmt, 53, SQL_C_LONG,	&SkillTbl[tno].Ability_max	,			 0, &cbValue);
		ret = SQLGetData(hStmt, 54, SQL_C_LONG,	&SkillTbl[tno].Plus_element	,			 0, &cbValue);
		ret = SQLGetData(hStmt, 55, SQL_C_LONG,	&SkillTbl[tno].Plus_type	,			 0, &cbValue);
		ret = SQLGetData(hStmt, 56, SQL_C_LONG,	&SkillTbl[tno].Plus_Rate	,			 0, &cbValue);
		ret = SQLGetData(hStmt, 57, SQL_C_LONG,	&SkillTbl[tno].Plus_min	,			 0, &cbValue);
		ret = SQLGetData(hStmt, 58, SQL_C_LONG,	&SkillTbl[tno].SP_demand	,			 0, &cbValue);
		ret = SQLGetData(hStmt, 59, SQL_C_LONG,	&SkillTbl[tno].Success	,			 0, &cbValue);
		ret = SQLGetData(hStmt, 60, SQL_C_LONG,	&SkillTbl[tno].effectCharobj_status	,			 0, &cbValue);
		ret = SQLGetData(hStmt, 61, SQL_C_LONG,	&SkillTbl[tno].Change_enermy_status	,			 0, &cbValue);
		ret = SQLGetData(hStmt, 62, SQL_C_LONG,	&SkillTbl[tno].Change_enermy_status2	,			 0, &cbValue);
		ret = SQLGetData(hStmt, 63, SQL_C_LONG,	&SkillTbl[tno].charvisible	,			 0, &cbValue);
		ret = SQLGetData(hStmt, 64, SQL_C_LONG,	&SkillTbl[tno].Effectobj_status	,			 0, &cbValue);
		ret = SQLGetData(hStmt, 65, SQL_C_LONG,	&SkillTbl[tno].Changeobj_attribute	,			 0, &cbValue);
		ret = SQLGetData(hStmt, 66, SQL_C_LONG,	&SkillTbl[tno].changeobj_visible	,			 0, &cbValue);
		ret = SQLGetData(hStmt, 67, SQL_C_LONG,	&SkillTbl[tno].Skill2job	,			 0, &cbValue);
		ret = SQLGetData(hStmt, 68, SQL_C_LONG,	&SkillTbl[tno].effectChar_hide	,			 0, &cbValue);
		ret = SQLGetData(hStmt, 69, SQL_C_LONG,	&SkillTbl[tno].Increase_mov	,			 0, &cbValue);
		ret = SQLGetData(hStmt, 70, SQL_C_LONG,	&SkillTbl[tno].Increse_Life	,			 0, &cbValue);
		ret = SQLGetData(hStmt, 71, SQL_C_LONG,	&SkillTbl[tno].Increse_mana	,			 0, &cbValue);
		ret = SQLGetData(hStmt, 72, SQL_C_LONG,	&SkillTbl[tno].Increse_health	,			 0, &cbValue);
		ret = SQLGetData(hStmt, 73, SQL_C_LONG,	&SkillTbl[tno].Change_mor	,			 0, &cbValue);
		ret = SQLGetData(hStmt, 74, SQL_C_LONG,	&SkillTbl[tno].Obj_attribute1	,			 0, &cbValue);
		ret = SQLGetData(hStmt, 75, SQL_C_LONG,	&SkillTbl[tno].ChangeObj_attribute1	,			 0, &cbValue);
		ret = SQLGetData(hStmt, 76, SQL_C_LONG,	&SkillTbl[tno].Display_LV	,			 0, &cbValue);
		ret = SQLGetData(hStmt, 77, SQL_C_LONG,	&SkillTbl[tno].DisPlay_Wis	,			 0, &cbValue);
		ret = SQLGetData(hStmt, 78, SQL_C_LONG,	&SkillTbl[tno].Display_Int	,			 0, &cbValue);
		ret = SQLGetData(hStmt, 79, SQL_C_LONG,	&SkillTbl[tno].DisPlay_Life	,			 0, &cbValue);
		ret = SQLGetData(hStmt, 80, SQL_C_LONG,	&SkillTbl[tno].DisPlay_Health	,			 0, &cbValue);
		ret = SQLGetData(hStmt, 81, SQL_C_LONG,	&SkillTbl[tno].DisPlay_Damage	,			 0, &cbValue);
		ret = SQLGetData(hStmt, 82, SQL_C_LONG,	&SkillTbl[tno].DisPlay_AC	,			 0, &cbValue);
		ret = SQLGetData(hStmt, 83, SQL_C_LONG,	&SkillTbl[tno].DisPlay_inventory	,			 0, &cbValue);
		ret = SQLGetData(hStmt, 84, SQL_C_LONG,	&SkillTbl[tno].Display_Foot	,			 0, &cbValue);
		ret = SQLGetData(hStmt, 85, SQL_C_LONG,	&SkillTbl[tno].DisPlay_Item_Changeall	,			 0, &cbValue);
		ret = SQLGetData(hStmt, 86, SQL_C_LONG,	&SkillTbl[tno].DisPlay_royalty	,			 0, &cbValue);
		ret = SQLGetData(hStmt, 87, SQL_C_LONG,	&SkillTbl[tno].DisPlay_Item_Status	,			 0, &cbValue);
		ret = SQLGetData(hStmt, 88, SQL_C_LONG,	&SkillTbl[tno].DisPlay_name	,			 0, &cbValue);
		ret = SQLGetData(hStmt, 89, SQL_C_LONG,	&SkillTbl[tno].DisPlay_item_Price	,			 0, &cbValue);
		ret = SQLGetData(hStmt, 90, SQL_C_LONG,	&SkillTbl[tno].effect	,			 0, &cbValue);
		ret = SQLGetData(hStmt, 91, SQL_C_LONG,	&SkillTbl[tno].effect_wave	,			 0, &cbValue);
		ret = SQLGetData(hStmt, 92, SQL_C_LONG,	&SkillTbl[tno].effect_race	,			 0, &cbValue);
		ret = SQLGetData(hStmt, 93, SQL_C_LONG,	&SkillTbl[tno].Success_range	,			 0, &cbValue);
		ret = SQLGetData(hStmt, 94, SQL_C_LONG,	&SkillTbl[tno].Damage	,			 0, &cbValue);
		ret = SQLGetData(hStmt, 95, SQL_C_LONG,	&SkillTbl[tno].Status_self	,			 0, &cbValue);
		ret = SQLGetData(hStmt, 96, SQL_C_LONG,	&SkillTbl[tno].Effect1	,			 0, &cbValue);
		ret = SQLGetData(hStmt, 97, SQL_C_LONG,	&SkillTbl[tno].Effect_wave1	,			 0, &cbValue);
		ret = SQLGetData(hStmt, 98, SQL_C_LONG,	&SkillTbl[tno].Enemy_status	,			 0, &cbValue);
		ret = SQLGetData(hStmt, 99, SQL_C_LONG,	&SkillTbl[tno].Range	,			 0, &cbValue);
		ret = SQLGetData(hStmt,100, SQL_C_LONG,	&SkillTbl[tno].Enermy_alram	,			 0, &cbValue);
		ret = SQLGetData(hStmt,101, SQL_C_LONG,	&SkillTbl[tno].map	,			 0, &cbValue);
		ret = SQLGetData(hStmt,102, SQL_C_LONG,	&SkillTbl[tno].Curser1	,			 0, &cbValue);
		ret = SQLGetData(hStmt,103, SQL_C_LONG,	&SkillTbl[tno].Curser2	,			 0, &cbValue);
		ret = SQLGetData(hStmt,104, SQL_C_LONG,	&SkillTbl[tno].Curser3	,			 0, &cbValue);
		ret = SQLGetData(hStmt,105, SQL_C_LONG,	&SkillTbl[tno].point	,			 0, &cbValue);
		
		ret = SQLGetData(hStmt,106, SQL_C_LONG,	&SkillTbl[tno].inclusive ,			 0, &cbValue);
		ret = SQLGetData(hStmt,107, SQL_C_LONG,	&SkillTbl[tno].money ,				 0, &cbValue);
		ret = SQLGetData(hStmt,108, SQL_C_LONG,	&SkillTbl[tno].need_exp,			 0, &cbValue);
		ret = SQLGetData(hStmt,109, SQL_C_LONG,	&SkillTbl[tno].MotherSkillType,		 0, &cbValue);
		ret = SQLGetData(hStmt,110, SQL_C_LONG,	&SkillTbl[tno].MotherSkillExp,		 0, &cbValue);
		
		if(ret != SQL_SUCCESS_WITH_INFO && ret != SQL_SUCCESS) { printf("\nError!!! (%d)", ret) ; return -1 ;}
		c++ ;
		ret= SQLFetch(hStmt) ;
	}	
	
	SQLFreeStmt(hStmt, SQL_DROP) ;
	
	FILE *fp;
	int  i;
	char * tt = (char *)SkillTbl;
	char crc = 0;
	for( i = 0 ; i < MAX_SKILLMAIN * sizeof(SkillMain) ; i ++ )
	{	
		*tt += ( 200 + i );
		crc  += ((*tt + *tt ) - i);
		tt ++;
	}	
	
	fp = fopen( "./Output/skilltable.bin", "wb" );
	if( fp )
	{	
		fwrite( &crc, 1,1, fp );
		fwrite( SkillTbl, MAX_SKILLMAIN, sizeof(SkillMain), fp );
		fclose(fp);
	}	
	
	tt = (char *)SkillTbl;
	for( i = 0 ; i < MAX_SKILLMAIN * sizeof(SkillMain) ; i ++ )
	{	
		*tt -= ( 200 + i );
		tt ++;
	}	
	
	MyLog(LOG_NORMAL, "    .Skill Info   %3d data Loaded !", c) ;
	
	return c ;			
}							






int CurrentTotalUserInMap( void )
{
	SQLRETURN retCode;
	SQLHSTMT  hStmt;
	char szQuerry[128];
	SQLLEN		cbValue;
	int totalno = 0;
	int no;
	char mapname[ NM_LENGTH + 1];
	int c;
	
	
	static int oldhour;
	
	if( oldhour == g_hour ) return 0;
	oldhour = g_hour;
	
	
	FILE *fp = fopen( "�����ڼ�.txt", "at+" );
	if( fp == NULL ) return 0;
	
	fprintf( fp, " %2d.%2d %02d:%02d | ", g_mon + 1, g_day, g_hour, g_min );
	
	//sprintf( szQuerry, "SELECT MapName, HowMany FROM MapName" );
	/////////////////////////////////////////////////////////////////////////////
	sprintf(szQuerry, "CALL up_get_map_name_info");
	/////////////////////////////////////////////////////////////////////////////
	c = 0;
	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{
		retCode = SQLFetch(hStmt);
		while( retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
		{
			memset( mapname, 0, NM_LENGTH+1);
			retCode = SQLGetData(hStmt, 1, SQL_C_CHAR, mapname, NM_LENGTH, &cbValue);
			retCode = SQLGetData(hStmt, 2, SQL_C_LONG, &no,	0, &cbValue);
			if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
			{
				totalno += no;
				retCode = SQLFetch(hStmt);
				EatRearWhiteChar( mapname );
				fprintf( fp, "[%13s:%4d]  ", mapname, no );
				c++;
				if( c >= 7 ) goto NEXT_;
			}
			else 
			{
				SQLFreeStmt(hStmt, SQL_DROP);
				fclose(fp);
				return 0;
			}
		}
		
NEXT_:
		
		SQLFreeStmt(hStmt, SQL_DROP);
	}
	
	fprintf( fp, ": ConcurrentUser : %5d \n", totalno );
	
	fclose(fp);
	return totalno;
}

int GetBelieveAndFaith_SQL( int &believe, int &faith, char *name )		// 0405 YGI
{	
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	SQLLEN		cbValue;
	char		szQuerry[255];
	
	//sprintf(szQuerry, "select believe_god , faith from chr_info2 where name='%s'", name );		// 0414 YGI
	/////////////////////////////////////////////////////////////////////////////
	sprintf(szQuerry, "CALL up_get_believe_and_faith( '%s')", name);
	/////////////////////////////////////////////////////////////////////////////
	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{	
		retCode = SQLFetch(hStmt);
		
		retCode = SQLGetData(hStmt, 1, SQL_C_ULONG, &believe,	0, &cbValue);
		retCode = SQLGetData(hStmt, 2, SQL_C_ULONG, &faith,		0, &cbValue);
	}		
	else 
	{
		SQLFreeStmt(hStmt, SQL_DROP);
		return 0;
	}
	SQLFreeStmt(hStmt, SQL_DROP);
	return 1;
}

int UpdateFaith_SQL( short int faith, char *name )		// 0405 YGI
{	
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	char		szQuerry[255];
	
	sprintf(szQuerry, "Update chr_info2 SET faith = %d where name='%s'", faith, name );
	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	SQLFreeStmt(hStmt, SQL_DROP);
	
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO) return 1;
	else return 0;
}

int UpdateEvaName( const char *my_name, char *eva_name )		// ������ ���?�̸��� ���?������Ʈ �Ѵ�. // 0405 YGI//020725 lsw
{
	return 0;											// 010414 YGI
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	char		szQuerry[255];
	
	sprintf(szQuerry, "Update chr_info2 SET evangelist = '%s' where name='%s'", eva_name, my_name );		// 010414 YGI
	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	SQLFreeStmt(hStmt, SQL_DROP);
	
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO) return 1;
	else return 0;
}

int GetEvangelist( const char *my_name, char *eva_name )		// ���� ������ ���?�̸� ��������//020725 lsw
{
	return 0;											// 010414 YGI
	SQLLEN		cbValue;
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	char		szQuerry[128];
	
	//sprintf( szQuerry, "select evangelist from chr_info2 where name = '%s'", my_name);						// 010414 YGI
	/////////////////////////////////////////////////////////////////////////////
	sprintf(szQuerry, "CALL up_get_evange_list( '%s')", my_name);
	/////////////////////////////////////////////////////////////////////////////			
	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{	
		retCode = SQLFetch( hStmt );
		retCode = SQLGetData(hStmt, 1, SQL_C_CHAR,	eva_name, 0, &cbValue);
		
		if(retCode == SQL_SUCCESS)
		{
			SQLFreeStmt(hStmt, SQL_DROP);
			return (1);
		}
		else 
		{
			SQLFreeStmt(hStmt, SQL_DROP);		// 0308 YGI
			return( 0 );
		}
	}
	SQLFreeStmt(hStmt, SQL_DROP);		// 0308 YGI
	return 1;
}

void SubtractFaith( const char *szName )		// ������ �� ���?�ڱ⸦ ������ �����?�žӽ��� ��´�?//020725 lsw
{
	char eva_name[20] = {0, };
	if( GetEvangelist( szName, eva_name ) )//020725 lsw
	{
		if( eva_name[0] )
		{
			int believe, faith;
			GetBelieveAndFaith_SQL( believe, faith, eva_name );
			faith -= 10;
			if( faith < 0 ) faith = 0;
			UpdateFaith_SQL( (short int )faith, eva_name );
		}
	}
}


//// 0405 YGI
int GetGodFood_SQL( int &god_food, DWORD &god_food_date, char *name )
{
	SQLLEN		cbValue;
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	char		szQuerry[128];
	char		date[10];
	
	//sprintf( szQuerry, "SELECT god_food, god_food_date FROM chr_info2 WHERE name = '%s'", name);
	/////////////////////////////////////////////////////////////////////////////
	sprintf(szQuerry, "CALL up_get_god_food( '%s')", name);
	/////////////////////////////////////////////////////////////////////////////	
	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{	
		retCode = SQLFetch( hStmt );
		retCode = SQLGetData(hStmt, 1, SQL_C_ULONG,	&god_food, 0, &cbValue);
		retCode = SQLGetData(hStmt, 2, SQL_C_CHAR,	date, 9, &cbValue); god_food_date = (DWORD) atof( date );
		
		if(retCode == SQL_SUCCESS)
		{
			SQLFreeStmt(hStmt, SQL_DROP);
			return (1);
		}
		else 
		{
			SQLFreeStmt(hStmt, SQL_DROP);		
			return( 0 );
		}
	}
	SQLFreeStmt(hStmt, SQL_DROP);	
	return 1;
}



bool CheckIsRealName( char *id, char *charname )
{
	SQLLEN		cbValue;
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	char		szQuerry[128];
	char		realid[ 20];
	
	
	sprintf( szQuerry, "SELECT login_id FROM chr_info WHERE name = '%s'", charname );
	
	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{
		retCode = SQLFetch( hStmt );
		retCode = SQLGetData(hStmt, 1, SQL_C_CHAR,	realid, 20, &cbValue);	
		EatRearWhiteChar( realid );
		CharUpper( realid );
		
		char tempid[20];
		strcpy( tempid, id );
		EatRearWhiteChar( tempid );
		CharUpper( tempid );
		if( strcmp( tempid, realid ) == 0 ) goto SUCCESS_;
		
		SQLFreeStmt(hStmt, SQL_DROP);
		
		RecvHackingUser( id, charname, 20009, " ","Not his Char" );
		
		return ( false );
	}		
	
SUCCESS_:	
	
	SQLFreeStmt(hStmt, SQL_DROP);
	return true;
}			


int GetCharNew( int &is_new, char *name )		// new char
{			
	SQLLEN		cbValue;
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	char		szQuerry[128];
	
	//sprintf( szQuerry, "SELECT new FROM chr_info2 WHERE name = '%s'", name);
	/////////////////////////////////////////////////////////////////////////////
	sprintf(szQuerry, "CALL up_get_new( '%s')", name);
	/////////////////////////////////////////////////////////////////////////////			
	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{	
		retCode = SQLFetch( hStmt );
		retCode = SQLGetData(hStmt, 1, SQL_C_ULONG,	&is_new, 0, &cbValue);
		
		if(retCode == SQL_SUCCESS)
		{
			SQLFreeStmt(hStmt, SQL_DROP);
			return (1);
		}
		else 
		{
			SQLFreeStmt(hStmt, SQL_DROP);
			return( 0 );
		}
	}
	SQLFreeStmt(hStmt, SQL_DROP);	
	return 1;
}

int UpdateCharNew( char *name )
{
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	char		szQuerry[255];
	
	sprintf(szQuerry, "Update chr_info2 SET new = 0 where name='%s'", name );
	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	SQLFreeStmt(hStmt, SQL_DROP);
	
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO) return 1;
	else return 0;
}



char *GodIndex2Name[] = { "EDELBLHOY", "TEFFERY", "LETTY", };	// 0410 YGI		���������? ���丮, �׶�������, ���ǳ�, ��Ƽ, ��ī�׽�, ȭ����
int GetGodMeetingTime( TIMESTAMP_STRUCT &day, TIMESTAMP_STRUCT &month, TIMESTAMP_STRUCT &year, int god_index )
{		
	SQLLEN		cbValue;
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	char		szQuerry[128];
	DWORD		bbs_count=0;
	
	// char *index2name[] = { "EDELBLHOY", };			// 0410 YGI
	//sprintf( szQuerry, "SELECT day_time, month_time, year_time FROM god_meeting_time WHERE god_name = '%s'", GodIndex2Name[god_index-1] );
	/////////////////////////////////////////////////////////////////////////////
	sprintf(szQuerry, "CALL up_get_god_meeting_time( '%s')", GodIndex2Name[god_index-1]);
	/////////////////////////////////////////////////////////////////////////////
	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{	
		retCode = SQLFetch( hStmt );
		
		retCode = SQLGetData(hStmt, 1, SQL_C_TIMESTAMP,	&day,	sizeof( TIMESTAMP_STRUCT ), &cbValue);
		retCode = SQLGetData(hStmt, 2, SQL_C_TIMESTAMP,	&month, sizeof( TIMESTAMP_STRUCT ), &cbValue);
		retCode = SQLGetData(hStmt, 3, SQL_C_TIMESTAMP,	&year,  sizeof( TIMESTAMP_STRUCT ), &cbValue);
	}
	
	if(retCode == SQL_SUCCESS)
	{
		SQLFreeStmt(hStmt, SQL_DROP);
		return ( 1 );
	}
	else 
	{
		SQLFreeStmt(hStmt, SQL_DROP);
		return( 0 );
	}
}

char *GetMeetingMessage( int god_type, int text_type, int text_num )			// 0410 YGI
{		
	SQLLEN		cbValue;
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	char		szQuerry[128];
	int			len = 0;
	char		*msg=NULL;
	
	char		text_field[20];
	
	switch( text_type )
	{
	case 300:		sprintf( text_field, "year_text%d", text_num ); break;
	case 200:		sprintf( text_field, "month_text%d", text_num ); break;
	case 100:		sprintf( text_field, "day_text%d", text_num ); break;
	default : return NULL;
	}
	
	//sprintf( szQuerry, "SELECT DataLength(%s), %s FROM god_meeting_time WHERE god_name = '%s'", text_field,text_field, GodIndex2Name[god_type-1] );
	/////////////////////////////////////////////////////////////////////////////
	sprintf(szQuerry, "CALL up_get_meeting_message_info( '%s', '%s')", text_field, 
		GodIndex2Name[god_type-1]);
	/////////////////////////////////////////////////////////////////////////////
	SQLAllocStmt(hDBC, &hStmt);
	
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{	
		retCode = SQLFetch( hStmt );
		retCode = SQLGetData(hStmt, 1, SQL_C_ULONG, &len, 0, &cbValue);
		if( !len ) 
		{
			SQLFreeStmt(hStmt, SQL_DROP);
			return msg;
		}
		msg = new char[len+1];
		memset( msg, 0, len+1 );
		retCode = SQLGetData(hStmt, 2, SQL_C_CHAR, msg, len+1, &cbValue);
		SQLFreeStmt(hStmt, SQL_DROP);
		
		return msg;
	}
	SQLFreeStmt(hStmt, SQL_DROP);
	return msg;
}		

int UpdateCharStatusByKein( t_connection c[], short int cn )		// 0410 YGI
{
	CHARLIST *ch = &c[cn].chrlst;
	if( !ch ) return 0; 
	
	k_char_update_data p;
	
	p.believe_god		= ch->believe_god;
	p.faith				= ch->faith;
	p.god_cast_level	= ch->god_cast_level;
	sprintf( p.name, "%s", ch->Name );
	
	return ( UpdateCharStatusByKein( &p ) );
}

int UpdateCharStatusByKein( k_char_update_data *ch )		// 0410 YGI
{
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	char		szQuerry[255];
	
	
	sprintf(szQuerry, "UPDATE chr_info2 SET god_cast_level=%d, believe_god=%d, faith=%d WHERE name='%s'", 
		ch->god_cast_level, ch->believe_god, ch->faith, ch->name );
	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	SQLFreeStmt(hStmt, SQL_DROP);
	
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO) return 1;
	else return 0;
}


static char		szmailQuerry[MAIL_BUF];		// 0507 YGI
int GetCharDataStatusByKein( k_get_char_info *ch, char *name )		// 0410 YGI
{		
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	SQLLEN		cbValue;
	DWORD		t;
	
	//sprintf(szmailQuerry, 
	//			"SELECT god_cast_level, believe_god, faith "		// 0414 YGI
	//      "FROM chr_info2 WHERE name='%s'", name );
	/////////////////////////////////////////////////////////////////////////////
	sprintf(szmailQuerry, "CALL up_get_char_data_status( '%s')", name);
	/////////////////////////////////////////////////////////////////////////////			
	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szmailQuerry, SQL_NTS);
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{		
		retCode = SQLFetch(hStmt);
		if(retCode == SQL_SUCCESS)
		{	
			retCode = SQLGetData(hStmt, 1, SQL_C_ULONG, &t, 0, &cbValue);			ch->god_cast_level	= (short int) t;
			retCode = SQLGetData(hStmt, 2, SQL_C_ULONG, &t, 0, &cbValue);			ch->believe_god		= (char) t;
			retCode = SQLGetData(hStmt, 3, SQL_C_ULONG, &t, 0, &cbValue);			ch->faith			= (short int) t;
			
			SQLFreeStmt(hStmt, SQL_DROP);
			return(1);
		}
	}	
	
	SQLFreeStmt(hStmt, SQL_DROP);
	return(0);
}			

//---------------------------------------------------------------------------------------------------------			
// ��  �� :
// ��  �� : 
// ��  �� : 
//---------------------------------------------------------------------------------------------------------
int AccessBoxData( int box_id )			// 0420 YGI
{			
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	char		szQuerry[255];
				
	sprintf(szQuerry, "UPDATE box_item SET access_time = NOW() WHERE id = %d", box_id);
	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{		
		//		SQLTransact(SQL_NULL_HENV, hDBC, SQL_COMMIT );
		SQLFreeStmt(hStmt, SQL_DROP);
		return(1);
	}			
	else		
	{			
		//		SQLTransact(SQL_NULL_HENV, hDBC, SQL_ROLLBACK);
		SQLFreeStmt(hStmt, SQL_DROP);
		return(-2);
	}		
}			


//----------------------------------------------------------------------
// 020808 YGI ----------------------------------------------------
int GetMailTitle( char *name, MAIL mail[], int start_mail_id )
{
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	SQLLEN		cbValue;
	char		szQuerry[255];
	
	TIMESTAMP_STRUCT	date;
	
	
	int i=1;
	
	if( start_mail_id )
	{
		sprintf(szQuerry, "SELECT mail_id, send_name, mail_title, mail_time from mail WHERE recv_name = '%s' AND mail_id < %d ORDER BY mail_id DESC LIMIT 12", name, start_mail_id );
	}
	else 
	{
		sprintf(szQuerry, "CALL up_get_mail_title( '%s')", name);
	}
	
	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect_kein(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{
		int count = 0;
		retCode = SQLFetch(hStmt);
		while( retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
		{
			retCode = SQLGetData(hStmt, 1, SQL_C_LONG, &mail[count].mail_id,	0, &cbValue);
			retCode = SQLGetData(hStmt, 2, SQL_C_CHAR, mail[count].send_name,	20, &cbValue);
			retCode = SQLGetData(hStmt, 3, SQL_C_CHAR, mail[count].mail_title,	60, &cbValue);
			retCode = SQLGetData(hStmt, 4, SQL_C_TIMESTAMP, &date,  sizeof( TIMESTAMP_STRUCT ), &cbValue);
			
			EatRearWhiteChar( mail[count].send_name );
			EatRearWhiteChar( mail[count].mail_title );
			
			mail[count].year = date.year;
			mail[count].month = date.month;
			mail[count].day = date.day;
			
			if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
			{
				count ++;
				retCode = SQLFetch(hStmt);
			}
			else 
			{
				SQLFreeStmt(hStmt, SQL_DROP);
				return -1;
			}
		}
		
		SQLFreeStmt(hStmt, SQL_DROP);
		return( count );
	}
	else 
	{
		displaySQLError(hStmt, szQuerry);
		SQLFreeStmt(hStmt, SQL_DROP);
		return(-1);
	}
}
//----------------------------------------------------------------------

int GetMailBody( int type, DWORD mail_id, char *body )
{
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	SQLLEN		cbValue;
	char		szQuerry[255];
	
	if( type == 2 )	// ���?����
	{
		sprintf(szQuerry, "SELECT mail_body FROM guild_mail WHERE mail_id = %u ", mail_id );
	}
	else sprintf(szQuerry, "CALL up_get_mail_body( %u)", mail_id);
	
	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{
		int count = 0;
		retCode = SQLFetch(hStmt);
		if( retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO )
		{
			retCode = SQLGetData(hStmt, 1, SQL_C_CHAR, body, 1000, &cbValue);
			SQLFreeStmt(hStmt, SQL_DROP);
			CheckMail( mail_id );		// 0811 YGI
			return 1;
		}
		
		SQLFreeStmt(hStmt, SQL_DROP);
		return( 0 );
	}
	else 
	{
		//		SQLTransact(SQL_NULL_HENV, hDBC, SQL_ROLLBACK);
		SQLFreeStmt(hStmt, SQL_DROP);
		return( 0 );
	}
}

bool UpdateLogintablebyChangeMap( char *user_id, char *map_name )		// �ʰ� ��ȯ�� ���� ���̺� ����   //0521 YGI
{		
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	char		szQuerry[512] = {0,};
	
	sprintf(szQuerry, "UPDATE dragonraja_total.logintable SET servername='%s' WHERE user_id = '%s' ", map_name, user_id);
	if (!SQL_SUCCEEDED(::SQLAllocStmt(hDBC, &hStmt))) return false;
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	if (!SQL_SUCCEEDED(retCode)) displaySQLError(hStmt);
	::SQLFreeStmt(hStmt, SQL_DROP);
	
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{	
		return (true);
	}	
	
	return (false);
}		





////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	���ڵ� ������ �����´�.
//	÷�� : ���̺� �̸�( table ), key���� �ɸ� �ʵ��? field ) NULL�ϰ��?* ���� ó��, �������� �����? RowCount )
//	���ϰ� : SQL���� ���������� ���� �����? ����, ���� )�� �������ش�.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 021206
int GetRowLineOfSQL( const char *table, const char *field, int *RowCount, const char *condition, HDBC hdbc )
{
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	SQLLEN		cbValue;
	
	if( !table ) return -100;
	if( !field ) return -200;
	
	if( !hdbc ) hdbc = hDBC;
	
	if( !condition )
	{
		//sprintf(szmailQuerry, "select count( %s ) FROM %s", field, table);
		sprintf(szmailQuerry, "CALL up_get_record_count1( '%s', '%s')", field, table);
	}
	else 
	{
		sprintf(szmailQuerry, "select count( %s ) FROM %s WHERE %s", field, table, condition );
		//sprintf(szmailQuerry, "CALL up_get_record_count2( '%s', '%s', '%s')", field, table, condition);
	}
	
	SQLAllocStmt(hdbc, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szmailQuerry, SQL_NTS);
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{
		retCode = SQLFetch( hStmt );
		retCode = SQLGetData(hStmt, 1, SQL_C_ULONG, RowCount, 0, &cbValue);
		//021107 YGI
		SQLFreeStmt(hStmt, SQL_DROP);
		return 1;
	}
	else 
	{
		//		SQLTransact(SQL_NULL_HENV, hDBC, SQL_ROLLBACK);
		SQLFreeStmt(hStmt, SQL_DROP);
		return( retCode );
	}
}


///	0705 YGI
int GetRecentMapName( char *char_name, char *map_name )
{
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	SQLLEN		cbValue;
	char		szQuerry[2048];
	
	//sprintf(szQuerry, "SELECT mapname FROM chr_info WHERE name='%s'", char_name );
	/////////////////////////////////////////////////////////////////////////////
	sprintf(szQuerry, "CALL up_get_map_name( '%s')", char_name);
	/////////////////////////////////////////////////////////////////////////////	  		
	
	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{		
		retCode = SQLFetch(hStmt);
		
		if(retCode == SQL_SUCCESS)
		{	
			retCode = SQLGetData(hStmt, 1, SQL_C_CHAR, map_name, 20, &cbValue);
			EatRearWhiteChar( map_name );
			SQLFreeStmt(hStmt, SQL_DROP);
			
			return(1);
		}
		else
		{
			SQLFreeStmt(hStmt, SQL_DROP);		// 0308 YGI
			return(-2);
		}
	}
	else
	{
		SQLFreeStmt(hStmt, SQL_DROP);		// 0308 YGI
		return(-1);
	}
};
void GetPublicBbs(char** text)
{
    if (!text) return;
    delete[] *text;
    *text = NULL;
    HSTMT statement = NULL;
    if (!SQLOK(SQLAllocStmt(hDBC, &statement))) return;
    if (SQLOK(SQLExecDirect(statement, (SQLCHAR*)"CALL up_get_public_bbs", SQL_NTS))) {
        const RETCODE fetched = SQLFetch(statement);
        if (SQLOK(fetched)) {
            std::string gbk;
            if (ReadDatabaseNoticeGbk(statement, 2, gbk)) {
                if (!gbk.empty()) {
                    *text = new char[gbk.size() + 1];
                    memcpy(*text, gbk.c_str(), gbk.size() + 1);
                }
            } else {
                MyLog(LOG_FATAL, "Public notice Unicode read, CP936 conversion or size check failed");
            }
        } else if (fetched != SQL_NO_DATA) {
            MyLog(LOG_FATAL, "Public notice fetch failed");
        }
    } else {
        MyLog(LOG_FATAL, "Public notice query failed");
    }
    SQLFreeStmt(statement, SQL_DROP);
}
int InsertHunterList( char *target_name, char *wanted_name, DWORD money, char *profile )		// 0811 YGI
{
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	
	int lv, face, str, cls, gender;
	int ret = GetPartyChar_SQL( target_name, &lv, &face, &str, &cls, &gender );
	if( ret != 1 ) return 0;
	
	sprintf(szmailQuerry, "INSERT INTO hunter_list (target_name, regist_name, target_gender, target_face, money, profile )" 
		" VALUES ('%s', '%s', '%d', '%d', '%d', '%s')",target_name, wanted_name, gender, face, money, profile );
	
	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szmailQuerry, SQL_NTS);
	SQLFreeStmt(hStmt, SQL_DROP);
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO) return 1;
	
	return 0;
}

int GetHunterList( int id, k_hunter_list* pList, int type )			// 0811 YGI
{
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	SQLLEN		cbValue;
	char		szQuerry[255];
	DWORD		t;
	
	memset( pList, 0, sizeof( k_hunter_list ) );
	
	if( type )
	{
		if( id )	// id ���� ���� �ٷ� ������ �ϳ�
		{
			//sprintf(szQuerry, "SELECT TOP 1 * FROM hunter_list WHERE id < %d ORDER BY id DESC", id);
			/////////////////////////////////////////////////////////////////////////////
			sprintf(szQuerry, "CALL up_get_hunter_list1( %d)", id);
			/////////////////////////////////////////////////////////////////////////////
		}
		else
		{
			//sprintf(szQuerry, "SELECT TOP 1 * FROM hunter_list ORDER BY id DESC");		// �� ó��
			/////////////////////////////////////////////////////////////////////////////
			sprintf(szQuerry, "CALL up_get_hunter_list2");
			/////////////////////////////////////////////////////////////////////////////
		}
	}
	else
	{
		//sprintf(szQuerry, "SELECT * FROM hunter_list WHERE id = %d ", id);
		/////////////////////////////////////////////////////////////////////////////
		sprintf(szQuerry, "CALL up_get_hunter_list3( %d)", id);
		/////////////////////////////////////////////////////////////////////////////
	}
	
	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)		// 0811 YGI
	{
		SQLFetch(hStmt);
		SQLGetData(hStmt, 1, SQL_C_ULONG, &pList->index,	0, &cbValue);
		SQLGetData(hStmt, 2, SQL_C_CHAR, pList->target_name, 20, &cbValue);
		SQLGetData(hStmt, 3, SQL_C_CHAR, pList->regist_name, 20, &cbValue);
		SQLGetData(hStmt, 4, SQL_C_ULONG, &pList->money,	0, &cbValue);
		SQLGetData(hStmt, 5, SQL_C_CHAR, pList->text, 1000, &cbValue);
		SQLGetData(hStmt, 6, SQL_C_ULONG, &t,	0, &cbValue); pList->gender = (char)t;
		SQLGetData(hStmt, 7, SQL_C_ULONG, &t,	0, &cbValue); pList->face = (short int)t;
		
		EatRearWhiteChar( pList->target_name );
		EatRearWhiteChar( pList->target_name );
		EatRearWhiteChar( pList->text );
		
		SQLFreeStmt(hStmt, SQL_DROP);
		return 1;
	}
	
	SQLFreeStmt(hStmt, SQL_DROP);
	return 0;
}

int DeleteHunterList( int list_id )
{
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	char		szQuerry[255];
	
	sprintf(szQuerry, "DELETE FROM hunter_list WHERE id=%d", list_id );
	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	SQLFreeStmt(hStmt, SQL_DROP);
	
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO) return 1;
	else return 0;
}

int CheckMail( DWORD  mail_id )			// 0811 YGI
{
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	char		szQuerry[255];
	
	sprintf(szQuerry, "UPDATE mail SET read_check=1 WHERE mail_id = %u ", mail_id );
	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{
		SQLFreeStmt(hStmt, SQL_DROP);
		return 1;
	}
	else 
	{
		//		SQLTransact(SQL_NULL_HENV, hDBC, SQL_ROLLBACK);
		SQLFreeStmt(hStmt, SQL_DROP);
		return( 0 );
	}
}

int DeleteBox( int box_id )		// 0817 YGI
{
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	char		szQuerry[255];
	
	sprintf(szQuerry, "DELETE FROM box_item WHERE id=%d", box_id );
	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	SQLFreeStmt(hStmt, SQL_DROP);
	
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO) return 1;
	else return 0;
}

int SearchBoxAndDelete( UCHAR **inv, UCHAR **bank, const char *szName )//020725 lsw
{
	SQLLEN  cbvalue = SQL_C_BINARY ;
	SQLRETURN retcode;
	SQLHSTMT  hstmt;
	
	char query[256];
	sprintf(query, "SELECT inventory FROM chr_info where name='%s' ", szName) ;
	SQLAllocStmt(hDBC, &hstmt);
	retcode = SQLExecDirect(hstmt, (UCHAR *)query, SQL_NTS);
	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
		retcode = SQLFetch(hstmt);
		if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
			retcode= SQLGetData(hstmt, 1, SQL_C_BINARY, inv,			SIZE_OF_INV,			&cbvalue);
			int ret = 0;
			if(retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)	ret = 1;
			else	ret =0;
			
			//			SQLTransact(SQL_NULL_HENV, hDBC, SQL_COMMIT);
			SQLFreeStmt(hstmt, SQL_DROP);
			if( !ret ) return -1;		// ���� ����
		}
		else 
		{
			printf("\n Fetch Error") ;
			SQLFreeStmt(hstmt, SQL_DROP);
			return -1 ;
		}
	}
	else {
		printf("\n ExecDirect Error") ;
		SQLFreeStmt(hstmt, SQL_DROP);
		return -2 ;
	}
	
	sprintf(query, "SELECT bankitem FROM chr_info2 where name='%s' ", szName) ;
	SQLAllocStmt(hDBC, &hstmt);
	retcode = SQLExecDirect(hstmt, (UCHAR *)query, SQL_NTS);
	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
		retcode = SQLFetch(hstmt);
		if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
			retcode= SQLGetData(hstmt, 1, SQL_C_BINARY, bank,			SIZE_OF_BANKITEM,			&cbvalue);
			//				SQLTransact(SQL_NULL_HENV, hDBC, SQL_COMMIT);
			SQLFreeStmt(hstmt, SQL_DROP);
			return 1;
		}
		else 
		{
			//				SQLTransact(SQL_NULL_HENV, hDBC, SQL_COMMIT);
			SQLFreeStmt(hstmt, SQL_DROP);
			return(-1); // succeed
		}
	}
	else
	{
		//	SQLTransact(SQL_NULL_HENV, hDBC, SQL_ROLLBACK);
		SQLFreeStmt(hstmt, SQL_DROP);
		return(-3); // unknown error
	}
}




//////////////////////////////////////////////////////////////////////////
int UpdateResetJob_SQL( int job, char *name )		// 0909 YGI
{	
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	char		szQuerry[255];
	
	sprintf(szQuerry, "Update chr_info SET job = %d where name='%s'", job, name );
	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	SQLFreeStmt(hStmt, SQL_DROP);
	
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO) return 1;
	else return 0;
}

int get_Skill_BinaryData(UCHAR **skill_exp, UCHAR **script_var, char *name)
{
	SQLLEN  cbvalue = SQL_C_BINARY ;
	SQLRETURN retcode;
	SQLHSTMT  hstmt;
	char query[256];
	sprintf(query, "SELECT skill_exp, script_var FROM chr_info where name='%s' ", name) ;
	
	SQLAllocStmt(hDBC, &hstmt);
	retcode = SQLExecDirect(hstmt, (UCHAR *)query, SQL_NTS);
	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
		retcode = SQLFetch(hstmt);
		if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
			retcode= SQLGetData(hstmt, 1, SQL_C_BINARY, skill_exp,		SIZE_OF_SKILL_EXP,		&cbvalue);
			retcode= SQLGetData(hstmt, 2, SQL_C_BINARY, script_var,		SIZE_OF_SCRIPT_VAR,		&cbvalue);
		}
		else 
		{
			printf("\n Fetch Error") ;
			SQLFreeStmt(hstmt, SQL_DROP);
			return -1 ;
		}
	}
	else {
		printf("\n ExecDirect Error") ;
		SQLFreeStmt(hstmt, SQL_DROP);
		return -2 ;
	}
	
	if(retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
	{
		//		SQLTransact(SQL_NULL_HENV, hDBC, SQL_COMMIT);
		SQLFreeStmt(hstmt, SQL_DROP);
		
		return(1); // succeed
	}
	else
	{
		displaySQLError(hstmt) ;
		//		SQLTransact(SQL_NULL_HENV, hDBC, SQL_ROLLBACK);
		SQLFreeStmt(hstmt, SQL_DROP);
		return(-3); // unknown error
	}
}	

int Update_Skill_BinaryData(UCHAR **skill, UCHAR **skill_exp, UCHAR **script_var, char *name)
{
	SQLLEN  cbdata = 200, cbvalue = SQL_DATA_AT_EXEC ;
	SQLLEN	cbskilllen, cbskillexplen, cbscriptvarlen;
	SQLRETURN	retcode;
	SQLHSTMT	hstmt;
	SQLPOINTER	pParamData;
	int			n=0;
	char		aaa=20;
	char		query[256]={0,};
	
	sprintf(query, "UPDATE chr_info SET skill=?, skill_exp=? WHERE name='%s'", name) ;
	sprintf(query, "UPDATE chr_info SET skill=?, skill_exp=?, script_var=? WHERE name='%s'", name) ;
	
	//printf("\nSQL: %s", query) ;
	SQLAllocStmt(hDBC, &hstmt);
	retcode = SQLPrepare(hstmt, (UCHAR *)query, SQL_NTS);
	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
		
		retcode= SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_BINARY,SQL_LONGVARBINARY, SIZE_OF_SKILL,			0, skill,		0, &cbskilllen); 
		retcode= SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT, SQL_C_BINARY,SQL_LONGVARBINARY, SIZE_OF_SKILL_EXP,		0, skill_exp,	0, &cbskillexplen); 
		retcode= SQLBindParameter(hstmt, 3, SQL_PARAM_INPUT, SQL_C_BINARY,SQL_LONGVARBINARY, SIZE_OF_SCRIPT_VAR,	0, script_var,	0, &cbscriptvarlen); 
		
		if(retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO)
		{
			SQLFreeStmt(hstmt, SQL_DROP);
			printf("\nBinding FAIL!!!") ;
			return -5 ;
		}
		
		cbskilllen		= SQL_LEN_DATA_AT_EXEC(0);
		cbskillexplen	= SQL_LEN_DATA_AT_EXEC(0);
		cbscriptvarlen	= SQL_LEN_DATA_AT_EXEC(0);
		retcode = SQLExecute(hstmt) ;
		
		while(retcode == SQL_NEED_DATA) 
		{      
			retcode = SQLParamData(hstmt, &pParamData) ;
			if(retcode == SQL_NEED_DATA) 
			{
				if(pParamData == skill) 		SQLPutData(hstmt, skill, SIZE_OF_SKILL);      
				else if(pParamData == skill_exp) 	SQLPutData(hstmt, skill_exp, SIZE_OF_SKILL_EXP );
				else if(pParamData == script_var ) 	SQLPutData(hstmt, script_var, SIZE_OF_SCRIPT_VAR );
			}
			else break ;
		}
		
		retcode = SQLExecute(hstmt) ;
		
		if(retcode == SQL_NEED_DATA)
		{
			retcode= SQLCancel(hstmt) ;
			//			retcode= SQLTransact(SQL_NULL_HENV, hDBC, SQL_COMMIT);
			retcode= SQLFreeStmt(hstmt, SQL_DROP );
			if(retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO) {
				printf("\n Transact commit Error") ;
			}
			//printf("\nTransact & free") ;
			return(1); // succeed
		}
		else
		{
			displaySQLError(hstmt) ;
			//			SQLTransact(SQL_NULL_HENV, hDBC, SQL_ROLLBACK);
			SQLFreeStmt(hstmt, SQL_DROP);
			return(-3); 
		}
	}
	
	SQLFreeStmt(hstmt, SQL_DROP);
	return 1;
}

int UpdateSelectNation( int nation, char *name )		// 1027 YGI
{
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	char		szQuerry[255];
	
	
	CCharRank guild;
	DWORD name_status;
	int ret = GetNationByName( name, &name_status );
	if( !ret ) return 0;		// ������ ����
	
	memcpy( &guild, &name_status, sizeof( DWORD ) );
	guild.nation = nation;
	memcpy( &name_status, &guild, sizeof( DWORD ) );
	
	sprintf(szQuerry, "UPDATE chr_info SET nation = %d WHERE name ='%s'", name_status, name );
	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	SQLFreeStmt(hStmt, SQL_DROP);
	
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO) return 1;
	else return 0;
}

bool GetGuildCode(const char* szName, int& rGuildCode)
{	//< CSD-030804
	char szQuerry[MAX_PATH];
	::sprintf(szQuerry, "SELECT spritvalue FROM chr_info WHERE name='%s'", szName);
	
	HSTMT hStmt = NULL;
	SQLAllocStmt(hDBC, &hStmt);

	RETCODE	retCode = ::SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);

	if (retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{		
		retCode = ::SQLFetch(hStmt);
		
		if (retCode == SQL_SUCCESS)
		{
			SQLLEN cbValue;
			int nCode = 0;

			retCode = ::SQLGetData(hStmt, 1, SQL_C_SLONG, &nCode, 0, &cbValue); 
			rGuildCode = nCode;
			::SQLFreeStmt(hStmt, SQL_DROP);
			return true;
		}
	}
	
	rGuildCode = 0;
	::SQLFreeStmt(hStmt, SQL_DROP);
	return false;
}	//> CSD-030804

// 030602 YGI
bool SetGuildCode(char * pName, int iGuildCode)
{	//< CSD-030723
	char query[256];
	sprintf(query, "UPDATE chr_info SET spritvalue = %d WHERE name = '%s'", iGuildCode, pName);
	Querry_SQL(query);
	return 1;
}	//> CSD-030723


int GetNationByName( char *name, DWORD *name_status ) // 1027 YGI �������� ������ ���°��� �ƴ϶� ���ӽ������ͽ� �� �����´�.
{
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	SQLLEN		cbValue;
	char		szQuerry[2048];
	DWORD		temp;
	
	//sprintf(szQuerry, "SELECT nation FROM chr_info WHERE name='%s'", name );
	/////////////////////////////////////////////////////////////////////////////
	sprintf(szQuerry, "CALL up_get_nation( '%s')", name);
	/////////////////////////////////////////////////////////////////////////////
	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{		
		retCode = SQLFetch(hStmt);
		if(retCode == SQL_SUCCESS)
		{	
			retCode = SQLGetData(hStmt, 1, SQL_C_ULONG, &temp, 0, &cbValue); *name_status = temp;
			SQLFreeStmt(hStmt, SQL_DROP);
			return( 1 );
		}
	}
	
	SQLFreeStmt(hStmt, SQL_DROP);
	return( 0 );
}

int GetNationById( char *login_id )	// 1027 YGI---		// �̰� ��¥ ���� ���� �����´�.
{
	char name[4][20]={0,};
	GetCharacterNameInID( login_id, name );
	for( int i=0; i<4; i++ )
	{
		if( name[i][0] )
		{
			DWORD status;
			int ret = GetNationByName( name[i], &status );
			if( !ret ) continue;
			
			CCharRank name_status;
			memcpy( &name_status, &status, sizeof(DWORD ) );
			return (int) name_status.nation;
		}
	}
	return 0;
}

int GetMapInfo( t_mapinfo aMap[] )		// 1004 YGI
{
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	SQLLEN		cbValue;
	char		szQuerry[255];
	
	DWORD		value;
	
	//sprintf(szQuerry, "SELECT * FROM map_info ORDER BY number");
	/////////////////////////////////////////////////////////////////////////////
	sprintf(szQuerry, "CALL up_get_map_info");
	/////////////////////////////////////////////////////////////////////////////
	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
				
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{			
		int count = 0;
		retCode = SQLFetch(hStmt);
		while( retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
		{		
			retCode = SQLGetData(hStmt, 1, SQL_C_LONG, &value,	0, &cbValue); count = (char )value;
			retCode = SQLGetData(hStmt, 2, SQL_C_CHAR, aMap[count].mapfile,	30, &cbValue);
			retCode = SQLGetData(hStmt, 3, SQL_C_CHAR, aMap[count].mapname,	30, &cbValue);
			retCode = SQLGetData(hStmt, 4, SQL_C_LONG, &value,	0, &cbValue); aMap[count].floor = (char )value;
			retCode = SQLGetData(hStmt, 5, SQL_C_LONG, &value,	0, &cbValue); aMap[count].x= (char )value;
			retCode = SQLGetData(hStmt, 6, SQL_C_LONG, &value,	0, &cbValue); aMap[count].y= (char )value;
			retCode = SQLGetData(hStmt, 7, SQL_C_LONG, &value,	0, &cbValue); aMap[count].daylightcontrol= (char)value;
			retCode = SQLGetData(hStmt, 8, SQL_C_LONG, &value,	0, &cbValue); aMap[count].rain= (char )value;
			retCode = SQLGetData(hStmt, 9, SQL_C_LONG, &value,	0, &cbValue); aMap[count].nation= (char )value;
			retCode = SQLGetData(hStmt, 10, SQL_C_LONG, &value,	0, &cbValue); aMap[count].forrookie = value?true:false;
			retCode = SQLGetData(hStmt, 11, SQL_C_LONG, &value,	0, &cbValue); aMap[count].port      = value;
			
			EatRearWhiteChar(aMap[count].mapfile); strupr( aMap[count].mapfile );		// 010414 YGI
			EatRearWhiteChar(aMap[count].mapname);
			
			if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
			{	
				retCode = SQLFetch(hStmt);
			}	
			else
			{	
				SQLFreeStmt(hStmt, SQL_DROP);
				return 0;
			}
		}
		SQLFreeStmt(hStmt, SQL_DROP);
		return 1;
	}
	else 
	{
		//		SQLTransact(SQL_NULL_HENV, hDBC, SQL_ROLLBACK);
		SQLFreeStmt(hStmt, SQL_DROP);
		return 0;
	}
}


int MapPort( char *mapname )
{
	int i;
	
	strupr( mapname );
	
	for( i = 0 ;  i< MAX_MAP_ ; i ++ )
	{	
		if( strcmp( MapInfo[i].mapfile, mapname ) == 0 ) return MapInfo[i].port;
	}	
	
	return 0;
}		




int GetVoteById_SQL( char *szUID )		// 1004 YGI
{		
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	SQLLEN		cbValue;
	int			Len;
	char		szQuerry[255];
	int			vote;
	
	if(szUID == NULL) return(-3);
	
	Len = strlen(szUID);
	if(Len <= 0) return(-3);
	
	//sprintf(szQuerry, "SELECT vote FROM chr_log_info WHERE login_id='%s'", szUID);
	/////////////////////////////////////////////////////////////////////////////
	sprintf(szQuerry, "CALL up_get_vote( '%s')", szUID);
	/////////////////////////////////////////////////////////////////////////////			
	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{	
		retCode = SQLFetch(hStmt);
		
		while(retCode == SQL_SUCCESS)
		{
			retCode = SQLGetData(hStmt, 1, SQL_C_ULONG, &vote,	0, &cbValue);
			retCode = SQLFetch(hStmt);
		}
		SQLFreeStmt(hStmt, SQL_DROP);
		return(vote);
	}	
	else 
	{	
		//		SQLTransact(SQL_NULL_HENV, hDBC, SQL_ROLLBACK);
		SQLFreeStmt(hStmt, SQL_DROP);
		return(-2);
	}	
}		

int SetVote_SQL( char *id, int vote_num )
{		
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	char		szQuerry[255];
	
	sprintf(szQuerry, "UPDATE chr_log_info SET vote =%d WHERE login_id = '%s' ", vote_num, id );
	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{	
		//		SQLTransact(SQL_NULL_HENV, hDBC, SQL_COMMIT );
		SQLFreeStmt(hStmt, SQL_DROP);
		return(1);
	}	
	else 
	{	
		//		SQLTransact(SQL_NULL_HENV, hDBC, SQL_ROLLBACK);
		SQLFreeStmt(hStmt, SQL_DROP);
		return(-2);
	}	
}		

//int GetScoreVote( char *name, int vote_num )
int GetScoreVote( int candidate, int vote_num, int nation )		// 001031_2 YGI
{		
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	SQLLEN		cbValue;
	char		szQuerry[255];
	int			vote_score = -1;
	
	//sprintf(szQuerry, "SELECT score FROM vote WHERE ct =%d AND number = %d AND nation = %d", candidate, vote_num, nation);
	/////////////////////////////////////////////////////////////////////////////
	sprintf(szQuerry, "CALL up_get_vote_score( %d, %d, %d)", candidate, vote_num, nation);
	/////////////////////////////////////////////////////////////////////////////			
	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{	
		retCode = SQLFetch(hStmt);
		
		while(retCode == SQL_SUCCESS)
		{
			retCode = SQLGetData(hStmt, 1, SQL_C_ULONG, &vote_score, 0, &cbValue);
			retCode = SQLFetch(hStmt);
		}
		SQLFreeStmt(hStmt, SQL_DROP);
		return(vote_score);
	}	
	else 
	{	
		//		SQLTransact(SQL_NULL_HENV, hDBC, SQL_ROLLBACK);
		SQLFreeStmt(hStmt, SQL_DROP);
		return(-2);
	}	
}		


//int SetPlusScoreVote( char *name, int vote_num )		// ��ǥ�� �����ش�.
int SetPlusScoreVote( int candidate, int vote_num, int nation )		// ��ǥ�� �����ش�.		// 001031_2 YGI
{
	int vote_score = GetScoreVote( candidate, vote_num, nation );
	if( vote_score < 0 ) return vote_score;
	
	vote_score++;		// ��ǥ�ߵ�.
	
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	char		szQuerry[255];
	
	sprintf(szQuerry, "UPDATE vote SET score =%d WHERE ct = %d AND number = %d AND nation = %d", vote_score, candidate, vote_num, nation );
	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{
		//		SQLTransact(SQL_NULL_HENV, hDBC, SQL_COMMIT );
		SQLFreeStmt(hStmt, SQL_DROP);
		return(1);
	}
	else 
	{
		//		SQLTransact(SQL_NULL_HENV, hDBC, SQL_ROLLBACK);
		SQLFreeStmt(hStmt, SQL_DROP);
		return(-2);
	}
}

//////////////////////////////////////////////////////////////////////////////////////
// vote ���̺� ���� ��
// 1: ���ڵ� ����( �ĺ����?)	- ������ �ĺ��� �ִ��� üũ�ؼ� �ĺ���ȣ�� �ο��Ѵ�.
// 2: �ĺ� ���� ��������		- ���? ����, ����, ����, Ŭ������ �����´�
// 3: ���� ������?- ���� ��ȣ�� ������ �ִ� ���?�ĺ����� ���� ��ǥ���� �����´�

int RegistCandidate_SQL( k_candidate *p )		// �ĺ� ���?
{
	int nRowCount;		// ����ΰ�?
	
	char condition[255];
	sprintf( condition, "number = %d AND name = '%s'", p->number, p->name );
	int ret = GetRowLineOfSQL( "vote", "name", &nRowCount, condition );		// �̹� ��ϵǾ�?�ִ°�?
	if( ret ) return -3;
	
	sprintf( condition, "number = %d AND nation = %d", p->number, p->nation );
	ret = GetRowLineOfSQL( "vote", "name", &nRowCount, condition );
	
	if( nRowCount >= 3 ) return -1;		// ���̻� �ĺ� �����?�Ҽ� ����. // ���� ���ǿ� ���� �ٲ����ʿ䰡 �ִٸ� ���⼭
	
	int score = 0;
	int ct = nRowCount+1;				// �ĺ� ��ȣ
	
	char		szQuerry[255];
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	
	sprintf(szQuerry, "INSERT INTO vote (name, number, ct, score, lv, class, guild_code, gender, nation, face)"
		" values ('%s', %d, %d, %d, %d, %d, %d, %d, %d, %d)", 
		p->name, p->number, ct, score, p->lv, p->class_type, p->guild_code, p->gender, p->nation, p->face );
	
	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	SQLFreeStmt(hStmt, SQL_DROP);
	
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO) return 1;
	else
	{
		SQLFreeStmt(hStmt, SQL_DROP);
		displaySQLError(hStmt) ;
	}
	return -2;		// sql ������ ���� �ߴ�.
}

int GetCandidate_SQL( const char *name, short int vote_num, k_candidate *p )		// �ĺ� ���� ��������
{
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	SQLLEN		cbValue;
	char		szQuerry[255];
	DWORD		value;
	
	//sprintf(szQuerry, "SELECT lv, class, guild_code, gender, nation, face FROM vote WHERE name ='%s' && number = %d", name, vote_num );
	/////////////////////////////////////////////////////////////////////////////
	sprintf(szQuerry, "CALL up_get_candidate( '%s', %d)", name, vote_num);
	/////////////////////////////////////////////////////////////////////////////
	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{
		retCode = SQLFetch(hStmt);
		
		strcpy( p->name, name );
		p->number = vote_num;
		
		retCode = SQLGetData(hStmt, 1, SQL_C_ULONG, &value,	0, &cbValue); p->lv = value;
		retCode = SQLGetData(hStmt, 2, SQL_C_ULONG, &value, 0, &cbValue); p->class_type = value;
		retCode = SQLGetData(hStmt, 3, SQL_C_ULONG, &value,	0, &cbValue); p->guild_code = value;
		retCode = SQLGetData(hStmt, 4, SQL_C_ULONG, &value, 0, &cbValue); p->gender = value;
		retCode = SQLGetData(hStmt, 5, SQL_C_ULONG, &value,	0, &cbValue); p->nation = value;
		retCode = SQLGetData(hStmt, 5, SQL_C_ULONG, &value,	0, &cbValue); p->face = value;
		
		SQLFreeStmt(hStmt, SQL_DROP);
		return 1;
	}
	else 
	{
		//		SQLTransact(SQL_NULL_HENV, hDBC, SQL_ROLLBACK);
		SQLFreeStmt(hStmt, SQL_DROP);
		return(-2);
	}
}

int GetCandidate_SQL( short int vote_num, short int nation, k_candidate p[]  )		// �ĺ� ���� ��������
{
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	SQLLEN		cbValue;
	char		szQuerry[255];
	DWORD		value;
	
	//sprintf(szQuerry, "SELECT name, lv, class, guild_code, gender, nation, face FROM vote WHERE number = %d AND nation = %d",  vote_num, nation );
	/////////////////////////////////////////////////////////////////////////////
	sprintf(szQuerry, "CALL up_get_candidate2( %d, %d)", vote_num, nation);
	/////////////////////////////////////////////////////////////////////////////
	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{
		int ct = 0;
		retCode = SQLFetch(hStmt);
		while( retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO )
		{
			retCode = SQLGetData(hStmt, 1, SQL_C_CHAR, p[ct].name,	20, &cbValue);
			EatRearWhiteChar(p[ct].name);
			p[ct].number = vote_num;
			
			retCode = SQLGetData(hStmt, 2, SQL_C_ULONG, &value,	0, &cbValue); p[ct].lv = value;
			retCode = SQLGetData(hStmt, 3, SQL_C_ULONG, &value, 0, &cbValue); p[ct].class_type = value;
			retCode = SQLGetData(hStmt, 4, SQL_C_ULONG, &value,	0, &cbValue); p[ct].guild_code = value;
			retCode = SQLGetData(hStmt, 5, SQL_C_ULONG, &value, 0, &cbValue); p[ct].gender = value;
			retCode = SQLGetData(hStmt, 6, SQL_C_ULONG, &value,	0, &cbValue); p[ct].nation = value;
			retCode = SQLGetData(hStmt, 7, SQL_C_ULONG, &value,	0, &cbValue); p[ct].face = value;
			ct++;
			if( ct >= 3 ) break;
			retCode = SQLFetch(hStmt);
		}
		
		SQLFreeStmt(hStmt, SQL_DROP);
		return ct;
	}
	else 
	{
		//		SQLTransact(SQL_NULL_HENV, hDBC, SQL_ROLLBACK);
		SQLFreeStmt(hStmt, SQL_DROP);
		return(-2);
	}
}

int GetCurrentVoting( int vote_num, int nation, k_rep_vote_score *target )		// ���� ���� ���� ��Ȳ
{
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	SQLLEN		cbValue;
	char		szQuerry[255];
	
	//sprintf(szQuerry, "SELECT score, name FROM vote WHERE number =%d AND nation = %d", vote_num, nation );
	/////////////////////////////////////////////////////////////////////////////
	sprintf(szQuerry, "CALL up_get_current_vote( %d, %d)", vote_num, nation);
	/////////////////////////////////////////////////////////////////////////////
	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{
		retCode = SQLFetch(hStmt);
		int ct = 0;
		while(retCode == SQL_SUCCESS)
		{
			if( ct >= 3 ) break;;
			retCode = SQLGetData(hStmt, 1, SQL_C_ULONG, &target->name_score[ct].score,	0, &cbValue);
			retCode = SQLGetData(hStmt, 2, SQL_C_CHAR, target->name_score[ct].name,	20, &cbValue);
			EatRearWhiteChar( target->name_score[ct].name);
			retCode = SQLFetch(hStmt);
			ct++;
		}
		SQLFreeStmt(hStmt, SQL_DROP);
		return(ct);
	}
	else 
	{
		//		SQLTransact(SQL_NULL_HENV, hDBC, SQL_ROLLBACK);
		SQLFreeStmt(hStmt, SQL_DROP);
		return(-2);
	}
}


int GetVoteInfo( k_set_value_of_vote *target )		// ���� ��ȣ�� ���� ���� ����
{
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	SQLLEN		cbValue;
	char		szQuerry[255];
	DWORD		value;
	
	//sprintf(szQuerry, "SELECT vote_num, is_voting FROM vote_info" );
	/////////////////////////////////////////////////////////////////////////////
	sprintf(szQuerry, "CALL up_get_vote_info");
	/////////////////////////////////////////////////////////////////////////////
	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{
		retCode = SQLFetch(hStmt);
		int cn = 0;
		while(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
		{
			retCode = SQLGetData(hStmt, 1, SQL_C_ULONG, &value,	0, &cbValue); target->vote_num[cn] = (char)value;
			retCode = SQLGetData(hStmt, 2, SQL_C_ULONG, &value, 0, &cbValue); target->is_voting[cn] = (char)value;
			retCode = SQLFetch(hStmt);
			cn++;
			if( cn >= 6 ) break;
		}
		
		SQLFreeStmt(hStmt, SQL_DROP);
		return 1;
	}
	else 
	{
		//		SQLTransact(SQL_NULL_HENV, hDBC, SQL_ROLLBACK);
		SQLFreeStmt(hStmt, SQL_DROP);
		return(-2);
	}
}

/////////////////////////////////////////////////////////////
//
//	���� �Ϸ� ��ƾ
//

// 1: �� ���� ǥ�� ���� ���?�̸��� �����´�.
int GetVoteNumberOne( int nation, int vote_num, char *name )
{
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	SQLLEN		cbValue;
	char		szQuerry[255];
	
	//sprintf(szQuerry, "SELECT name FROM vote WHERE number =%d AND nation = %d ORDER BY score DESC", vote_num, nation );
	/////////////////////////////////////////////////////////////////////////////
	sprintf(szQuerry, "CALL up_get_vote_name( %d, %d)", vote_num, nation);
	/////////////////////////////////////////////////////////////////////////////
	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{
		retCode = SQLFetch(hStmt);
		retCode = SQLGetData(hStmt, 1, SQL_C_CHAR, name,	20, &cbValue);
		EatRearWhiteChar( name );
		SQLFreeStmt(hStmt, SQL_DROP);
		return(1);
	}
	else 
	{
		//		SQLTransact(SQL_NULL_HENV, hDBC, SQL_ROLLBACK);
		SQLFreeStmt(hStmt, SQL_DROP);
		return(-2);
	}
}

// 2: ���� �� ���� ������ �ִ����� Ȯ��( ����.. ���̵� �������µ� ������ ���� Ȯ���� �����ϱ� �׳� ���� ������ ���?������.
int IsLogingByName_SQL( char *name )
{
	//	GetRowLineOfSQL( "LoginTable", "name", 
	return 1;
}

// 3 : �ϴ� ���� ������ �����Ŀ� �׳� ĳ���Ϳ� ������Ʈ�� �ϴ� �Ѵ�.
// 3-1 : name_status �����ͼ� ������Ʈ�Ѵ�.
int UpdateYouAreKing( char *name , int king )		// 1027 YGI
{
	DWORD before;
	int ret = GetNationByName( name, &before );
	if( !ret ) return 0; 
	
	CCharRank after;
	memcpy( &after, &before, sizeof( DWORD ) );
	after.king = king;		// ������ �����?�ְų� ���ӽ�Ų��.
	
	memcpy( &before, &after, sizeof( DWORD ) );
	
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	char		szQuerry[255];
	
	sprintf(szQuerry, "UPDATE chr_info SET nation = %d WHERE name = '%s'", before, name );
	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{
		//		SQLTransact(SQL_NULL_HENV, hDBC, SQL_COMMIT );
		SQLFreeStmt(hStmt, SQL_DROP);
		return(1);
	}
	else 
	{
		//		SQLTransact(SQL_NULL_HENV, hDBC, SQL_ROLLBACK);
		SQLFreeStmt(hStmt, SQL_DROP);
		return(-2);
	}
}

// 4: ��Ȳ�� 0���� �����ؼ� ������ �Ѵ�.
int SetVoteEndFlag( int count, int type )
{
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	char		szQuerry[255];
	
	sprintf(szQuerry, "UPDATE vote_info SET is_voting = 0, king = NULL, king_start = NULL WHERE ct = %d", count );	// ���� ���� ���·� ->���÷� ���� ���´�.
	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{
		//		SQLTransact(SQL_NULL_HENV, hDBC, SQL_COMMIT );
		SQLFreeStmt(hStmt, SQL_DROP);
		return(1);
	}
	else 
	{
		//		SQLTransact(SQL_NULL_HENV, hDBC, SQL_ROLLBACK);
		SQLFreeStmt(hStmt, SQL_DROP);
		return(-2);
	}
}

// 5: ���?���̺��� �����Ѵ�. ( �յ�Ϲ�?�ĺ� ���� )
int CheckDbTable(int nation, char *name )
{
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	char		szQuerry[255];
	
	sprintf(szQuerry, "UPDATE vote_info SET king='%s', king_start = NOW() WHERE ct = %d", name, nation );// ���� ��Ͻ���?���´�.	
	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	
	//	SQLTransact(SQL_NULL_HENV, hDBC, SQL_COMMIT );
	SQLFreeStmt(hStmt, SQL_DROP);
	
	sprintf(szQuerry, "TRUNCATE TABLE vote_regist" );	// �ĺ� ���?���̺��� ����������.
	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{
		//		SQLTransact(SQL_NULL_HENV, hDBC, SQL_COMMIT );
		SQLFreeStmt(hStmt, SQL_DROP);
		return(1);
	}
	else 
	{
		//		SQLTransact(SQL_NULL_HENV, hDBC, SQL_ROLLBACK);
		SQLFreeStmt(hStmt, SQL_DROP);
		return(-2);
	}
}

//////////////////////////////////////////////////////
// �տ��� ������ ���� ��ƾ
int GetKingOfNation_SQL( int nation, char *name )		// �� ������ �� �̸� ��������
{
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	SQLLEN		cbValue;
	char		szQuerry[255];
	
	//sprintf(szQuerry, "SELECT king FROM vote_info WHERE ct =%d", nation );
	/////////////////////////////////////////////////////////////////////////////
	sprintf(szQuerry, "CALL up_get_nation_king( %d)", nation);
	/////////////////////////////////////////////////////////////////////////////
	
	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{
		retCode = SQLFetch(hStmt);
		retCode = SQLGetData(hStmt, 1, SQL_C_CHAR, name,	20, &cbValue);
		EatRearWhiteChar( name );
		SQLFreeStmt(hStmt, SQL_DROP);
		return(1);
	}
	else 
	{
		//		SQLTransact(SQL_NULL_HENV, hDBC, SQL_ROLLBACK);
		SQLFreeStmt(hStmt, SQL_DROP);
		return(-2);
	}
}

int InsertGmRegist( k_gm_regist_db *p )		// 1027 YGI
{//021108 lsw
	char		szQuerry[255];
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	/*
	char job_name[][20] = 
	{ 
	"���?,		"����",		"���?,		"������",	"������", 
	"�ɸ���",	"���?,		"��������", "�丮��",	"��ܻ�?, 
	"Ȱ���ۻ�", "���ݼ���", "������",	"����",		"�������̱⼺",
	"�����?, "Ȱ���ۻ��?, "��ܻ�⼺"
	};
	*/
	sprintf(szQuerry, "INSERT INTO gm_table (name, job, job_name )"
		//	" values ('%s', %d, '%s')", p->name, p->gm_list, job_name[p->gm_list] );
		" values ('%s', %d, 'GmQuest')", p->name, p->gm_list);
	
	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	SQLFreeStmt(hStmt, SQL_DROP);
	
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO) return 1;
	return -2;
}


// 001214 YGI -- ������ ��..
void SQLerror( char *querry )
{	
	//	printf( " SQL querry error! : [%s] '\n", querry );
}



int GetSalvation( char * name, DWORD &money )		// �����?�� �˾ƺ���
{
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	SQLLEN		cbValue;
	char		szQuerry[255];
	
	//sprintf(szQuerry, "SELECT salvation FROM chr_info2 WHERE name = '%s'", name);
	/////////////////////////////////////////////////////////////////////////////
	sprintf(szQuerry, "CALL up_get_salvation( '%s')", name);
	/////////////////////////////////////////////////////////////////////////////
	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{
		retCode = SQLFetch(hStmt);
		retCode = SQLGetData(hStmt, 1, SQL_C_ULONG, &money,	0, &cbValue);
		SQLFreeStmt(hStmt, SQL_DROP);
		return(1);
	}
	else 
	{
		//		SQLTransact(SQL_NULL_HENV, hDBC, SQL_ROLLBACK);
		SQLFreeStmt(hStmt, SQL_DROP);
		SQLerror( szQuerry );		
		return(-2);
	}
}

// 001221 YGI
int PlusMoneyRedClose( double &TotalMoney, DWORD plus_money )
{
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	SQLLEN		cbValue;
	char		szQuerry[255];
	LONGLONG	temp_money;
	
	SQLAllocStmt(hDBC, &hStmt);
	//retCode = SQLExecDirect(hStmt, (UCHAR *)"SELECT money FROM redcross", SQL_NTS);
	/////////////////////////////////////////////////////////////////////////////
	retCode = SQLExecDirect(hStmt, (UCHAR *)"CALL up_get_money", SQL_NTS);
	/////////////////////////////////////////////////////////////////////////////
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{
		retCode = SQLFetch(hStmt);
		retCode = SQLGetData(hStmt, 1, SQL_C_UBIGINT, &temp_money, 0, &cbValue);
		TotalMoney = temp_money;
		TotalMoney += plus_money;
		
		SQLFreeStmt(hStmt, SQL_DROP);
		SQLAllocStmt(hDBC, &hStmt);
		
		sprintf(szQuerry, "UPDATE redcross Set money = %.0f", TotalMoney );
		retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
		if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
		{
			retCode = 1;
		}
		
		SQLFreeStmt(hStmt, SQL_DROP);
		return(retCode);
	}
	else 
	{
		//		SQLTransact(SQL_NULL_HENV, hDBC, SQL_ROLLBACK);
		SQLFreeStmt(hStmt, SQL_DROP);
		SQLerror( szQuerry );		
		return(-2);
	}
}

// 010300 KHS
int LoadNoName_SQL()		// 010117 YGI
{
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	SQLLEN		cbValue;
	char		szQuerry[255];
	
	//sprintf(szQuerry, "SELECT noid FROM noid" );
	/////////////////////////////////////////////////////////////////////////////
	sprintf(szQuerry, "CALL up_get_noid");
	/////////////////////////////////////////////////////////////////////////////
	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	if( retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{
		retCode = SQLFetch(hStmt);
		no_name_count = 0;
		while( retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
		{
			char blockedName[NM_LENGTH] = {0};
			retCode = SQLGetData(hStmt, 1, SQL_C_CHAR, blockedName,
				sizeof(blockedName), &cbValue);
			if ((retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO) &&
				cbValue != SQL_NULL_DATA && blockedName[0])
			{
				CharUpper(blockedName);
				memcpy(no_name[no_name_count], blockedName, NM_LENGTH);
				no_name_count++;
				if( no_name_count >= NO_NAME_MAX_ ) break;
			}
			
			retCode = SQLFetch(hStmt);
		}
		SQLFreeStmt(hStmt, SQL_DROP);
		return(1);
	}
	
	SQLFreeStmt(hStmt, SQL_DROP);
	return(-1);
}








int SetChocolateCount( char * name, int &count )		// 010212 YGI
{
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	SQLLEN		cbValue;
	char		szQuerry[255];
	
	//sprintf(szQuerry, "SELECT ct FROM chocolate WHERE name = '%s'", name);
	/////////////////////////////////////////////////////////////////////////////
	sprintf(szQuerry, "CALL up_get_chocolate_count( '%s')", name);
	/////////////////////////////////////////////////////////////////////////////
	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{
		retCode = SQLFetch(hStmt);
		if( retCode == SQL_NO_DATA )		// ó���ϰ��?
		{	
			SQLFreeStmt(hStmt, SQL_DROP);
			SQLAllocStmt(hDBC, &hStmt);
			count = 1;
			sprintf(szQuerry, "INSERT INTO chocolate ( ct, name ) VALUES ( %d, '%s' )", count, name);
			retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
			SQLFreeStmt(hStmt, SQL_DROP);
			return(1);
		}
		
		retCode = SQLGetData(hStmt, 1, SQL_C_ULONG, &count,	0, &cbValue);
		SQLFreeStmt(hStmt, SQL_DROP);
		
		count++;
		SQLAllocStmt(hDBC, &hStmt);
		sprintf(szQuerry, "UPDATE chocolate SET ct = ct+1 WHERE name = '%s'", name);
		retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
		SQLFreeStmt(hStmt, SQL_DROP);
		return(1);
	}
	else
	{
		//		SQLTransact(SQL_NULL_HENV, hDBC, SQL_ROLLBACK);
		SQLFreeStmt(hStmt, SQL_DROP);
		SQLerror( szQuerry );		
		return(-2);
	}
}

int GetTopGivenChocolate( char * name, int &count )		// 010212 YGI // ���� ���� ���� ������ �ִ°�
{
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	SQLLEN		cbValue;
	char		szQuerry[255];
	
	sprintf(szQuerry, "SELECT name, score FROM colossus_rank ORDER BY score DESC LIMIT 1" );
	/////////////////////////////////////////////////////////////////////////////
	//sprintf(szQuerry, "EXEC up_get_top_given_chocolate");
	/////////////////////////////////////////////////////////////////////////////
	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{
		retCode = SQLFetch(hStmt);
		retCode = SQLGetData(hStmt, 1, SQL_C_CHAR, name, 20, &cbValue);
		EatRearWhiteChar( name );
		retCode = SQLGetData(hStmt, 2, SQL_C_ULONG, &count,	0, &cbValue);
		SQLFreeStmt(hStmt, SQL_DROP);
		return(1);
	}
	else 
	{
		//		SQLTransact(SQL_NULL_HENV, hDBC, SQL_ROLLBACK);
		SQLFreeStmt(hStmt, SQL_DROP);
		SQLerror( szQuerry );		
		return(-2);
	}
}
// 010303 YGI
////////////////////////////////////////////////////
//
//	���� �̺�Ʈ�� ���� ���̺� ���� openschool
//
int SaveOpenSchoolItem( char *name, int nation, int item_no, int &number )
{
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	SQLLEN		cbValue;
	char		szQuerry[255];
	
	number = 0;
	
	//sprintf(szQuerry, "SELECT name FROM openschool WHERE name = '%s'", name);
	/////////////////////////////////////////////////////////////////////////////
	sprintf(szQuerry, "CALL up_check_openschool_name( '%s')", name);
	/////////////////////////////////////////////////////////////////////////////
	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{
		retCode = SQLFetch(hStmt);
		if( retCode != SQL_NO_DATA ) 
		{
			//			SQLTransact(SQL_NULL_HENV, hDBC, SQL_ROLLBACK);
			SQLFreeStmt(hStmt, SQL_DROP);
			return -1;	// �̹� ��ϵ�?���̴�.
		}
	}
	else
	{
		//		SQLTransact(SQL_NULL_HENV, hDBC, SQL_ROLLBACK);
		SQLFreeStmt(hStmt, SQL_DROP);
		return(-2);
	}
	
	SQLFreeStmt(hStmt, SQL_DROP);
	SQLAllocStmt(hDBC, &hStmt);
	
	//sprintf(szQuerry, "SELECT count( name ) FROM openschool WHERE nation = %d", nation);
	/////////////////////////////////////////////////////////////////////////////
	sprintf(szQuerry, "CALL up_get_openschool_name_count( %d)", nation);
	/////////////////////////////////////////////////////////////////////////////
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{
		retCode = SQLFetch(hStmt);
		if( retCode == SQL_SUCCESS )
		{
			SQLGetData(hStmt, 1, SQL_C_ULONG, &number, 0, &cbValue);
		}
		
		SQLFreeStmt(hStmt, SQL_DROP);
		SQLAllocStmt(hDBC, &hStmt);
		sprintf(szQuerry, "INSERT INTO openschool ( name, nation, item ) values ('%s', %d, %d )", name, nation, item_no );
		number++;
		
		retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
		SQLFreeStmt(hStmt, SQL_DROP);
		return(1);
	}
	else
	{
		//		SQLTransact(SQL_NULL_HENV, hDBC, SQL_ROLLBACK);
		SQLFreeStmt(hStmt, SQL_DROP);
		return(-2);
	}
}


int SetNationByName( DWORD status, char *name )		// 1027 YGI
{
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	char		szQuerry[255];
	
	sprintf(szQuerry, "UPDATE chr_info SET nation = %d WHERE name ='%s'", status, name );
	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	SQLFreeStmt(hStmt, SQL_DROP);
	
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO) return 1;
	else return 0;
}


// 010310 YGI
int GetChocolateEatName( char *name, DWORD total_id )		// total_id�� ���� �� ã��
{
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	SQLLEN		cbValue;
	char		szQuerry[255];
	
	//sprintf(szQuerry, "SELECT name FROM chocolate_info WHERE total_id = %u", total_id );
	/////////////////////////////////////////////////////////////////////////////
	sprintf(szQuerry, "CALL up_get_chocolate_eatname( %u)", total_id);
	/////////////////////////////////////////////////////////////////////////////
	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{
		retCode = SQLFetch(hStmt);
		retCode = SQLGetData(hStmt, 1, SQL_C_CHAR, name, 20, &cbValue);
		EatRearWhiteChar( name );
		SQLFreeStmt(hStmt, SQL_DROP);
		return(1);
	}
	else 
	{
		//		SQLTransact(SQL_NULL_HENV, hDBC, SQL_ROLLBACK);
		SQLFreeStmt(hStmt, SQL_DROP);
		SQLerror( szQuerry );		
		return(-2);
	}
}

int GetNameByTotalId( char *name, DWORD total_id )
{
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	SQLLEN		cbValue;
	char		szQuerry[255];
	
	//sprintf(szQuerry, "SELECT name FROM chr_info WHERE total_id = %u", total_id );
	/////////////////////////////////////////////////////////////////////////////
	sprintf(szQuerry, "CALL up_get_name_by_total_id( %u)", total_id);
	/////////////////////////////////////////////////////////////////////////////
	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{
		retCode = SQLFetch(hStmt);
		retCode = SQLGetData(hStmt, 1, SQL_C_CHAR, name, 20, &cbValue);
		EatRearWhiteChar( name );
		SQLFreeStmt(hStmt, SQL_DROP);
		return(1);
	}
	else 
	{
		//		SQLTransact(SQL_NULL_HENV, hDBC, SQL_ROLLBACK);
		SQLFreeStmt(hStmt, SQL_DROP);
		SQLerror( szQuerry );		
		return(-2);
	}
}	

int SaveWhiteDayEvent( char *name, char *name2 )
{	
	char		szQuerry[255];
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	
	sprintf(szQuerry, "INSERT INTO savewhitedayevent ( man_name, woman_name )"
		" values ('%s', '%s' )", name, name2 );
	
	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	SQLFreeStmt(hStmt, SQL_DROP);
	
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO) return 1;
	return -2;
}	


int CheckLogInTable( int server_set_num )
{	
	char	szQuerry[255];
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	
	sprintf( szQuerry, "select count(*) from logintable where server_set_num = %d ", server_set_num );
	
	SQLAllocStmt(hDBC_TotalDB, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{
		retCode = SQLFetch(hStmt);
	}
	SQLFreeStmt(hStmt, SQL_DROP);
	
	if( retCode == SQL_SUCCESS )
	{
		sprintf( szQuerry, "There are IDs in ServerSet '%d' LoginTable \n\n     Do you want to CLEAR all of IDs in 'LogInTable' ?           ", server_set_num );
		if( MessageBox( NULL,	szQuerry, "Notification", MB_YESNO ) != IDYES )	return 0;
	}
	else return 0;
	
	sprintf( szQuerry, "delete from logintable where server_set_num = %d", server_set_num );
	SQLAllocStmt(hDBC_TotalDB, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	SQLFreeStmt(hStmt, SQL_DROP);
	
	return 1;
}		

// 010322 KHS
int CheckWorstDBTable( void )
{	
	char	szQuerry[255];
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	SQLLEN		cbValue;
	int number=0;
	
	// ������ �����?�ϸ� Mail�� ���� ������ ����. 
	return 1;
	
	
	SQLAllocStmt(hDBC, &hStmt);
	/////////////////////////////////////////////////////////////////////////////
	//retCode = SQLExecDirect(hStmt, (UCHAR *)"select count (*) from mail", SQL_NTS);
	retCode = SQLExecDirect(hStmt, (UCHAR *)"CALL up_get_mail_count", SQL_NTS);
	/////////////////////////////////////////////////////////////////////////////
	
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{	
		retCode = SQLFetch(hStmt);
		SQLGetData(hStmt, 1, SQL_C_ULONG, &number, 0, &cbValue);
	}	
	SQLFreeStmt(hStmt, SQL_DROP);
	
	MyLog( LOG_NORMAL, "Count of [DragonRajaDB].mail : %d ", number );
	
	if( number > 30000 )
	{	
		sprintf( szQuerry, "There are %d mails in [DragonRajaDB].mail \n It can make Servers wrong situation. \n\n Quit the progress?", number  );
		if( MessageBox( NULL, szQuerry, "DB Demon", MB_YESNO ) == IDYES )
		{
			return 0;
		}
	}	
	
	return 1;
}


// 010511 YGI
int UpdateLadderScore( DWORD ladderscore, char *name, int nation, int cls, int lv )
{
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	SQLLEN		cbValue;
	char		szQuerry[255];
	int			score;
	
	//sprintf(szQuerry, "SELECT score FROM colossus_rank WHERE name = '%s'", name);
	/////////////////////////////////////////////////////////////////////////////
	sprintf(szQuerry, "CALL up_get_colossus_rank_score( '%s')", name);
	/////////////////////////////////////////////////////////////////////////////
	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{
		retCode = SQLFetch(hStmt);
		if( retCode == SQL_NO_DATA )		// ó���ϰ��?
		{	
			SQLFreeStmt(hStmt, SQL_DROP);
			SQLAllocStmt(hDBC, &hStmt);
			sprintf(szQuerry, "INSERT INTO colossus_rank ( score, name, nation, class, lv ) VALUES ( %d, '%s', %d, %d, %d )", ladderscore, name, nation, cls, lv );
			retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
			SQLFreeStmt(hStmt, SQL_DROP);
			return(1);
		}
		else
		{
			int cn = 0;
			if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
			{
				SQLGetData(hStmt, 1, SQL_C_ULONG, &score, 0, &cbValue);
			}
			
			if( score != ladderscore )
			{
				SQLFreeStmt(hStmt, SQL_DROP);
				SQLAllocStmt(hDBC, &hStmt);
				//acer7
				sprintf(szQuerry, "UPDATE colossus_rank SET score = %d, lv = %d, nation = %d WHERE name = '%s'",ladderscore, lv, nation, name );
				retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
			}
			SQLFreeStmt(hStmt, SQL_DROP);
			return(1);
		}
	}
	else
	{
		//		SQLTransact(SQL_NULL_HENV, hDBC, SQL_ROLLBACK);
		SQLFreeStmt(hStmt, SQL_DROP);
		SQLerror( szQuerry );		
		return(-2);
	}
}	

// 020716 YGI
void RefreshColossus_rank_read_table()
{
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	char		szQuerry[255];
	
	MyLog( 0, "RefreshColossus_rank_read_table... START" );
	
	sprintf(szQuerry, "TRUNCATE TABLE colossus_rank_read" );
	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	SQLFreeStmt(hStmt, SQL_DROP);
	
	MyLog( 0, "RefreshColossus_rank_read_table... Delete OK" );
	
	sprintf(szQuerry, "INSERT INTO colossus_rank_read ( score, name, nation, class, lv ) SELECT * FROM colossus_rank ORDER BY score DESC" );
	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	SQLFreeStmt(hStmt, SQL_DROP);
	
	MyLog( 0, "RefreshColossus_rank_read_table... Insert OK" );	
}

int GetLadderScoreRank( int rank, k_db_get_ladder_score_rank *rank_data )
{
	static DWORD time;
	DWORD curr = GetTickCount();
	if( curr - time > 60000 ) 
	{
		time = curr;
		RefreshColossus_rank_read_table();
	}
	
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	SQLLEN		cbValue;
	char		szQuerry[255];
	int			t;
	
	memset( rank_data, 0, sizeof( k_db_get_ladder_score_rank ) );
	sprintf(szQuerry, "SELECT * FROM colossus_rank_read WHERE cn >= %d order by cn LIMIT 11", rank );		// 020716 YGI
	/////////////////////////////////////////////////////////////////////////////
	//sprintf(szQuerry, "CALL up_get_colossus_rank_read_info( %d)", rank);
	/////////////////////////////////////////////////////////////////////////////
	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{
		retCode = SQLFetch(hStmt);
		if( retCode == SQL_NO_DATA )		// ó���ϰ��?
		{	
			SQLFreeStmt(hStmt, SQL_DROP);
			if( rank == 1 )		// ���� ���� ���� ���?
			{
				RefreshColossus_rank_read_table();
				return 2;		// �ѹ� �� ��������ش�? ( ��ʹ�?���ѿ� ���� ������ �ֱ� ������ �� �ѹ��� ��..
			}
			return -1;		// �̰� ���̴�..
		}
		else
		{
			int cn = 0;
			while(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
			{
				SQLGetData(hStmt, 1, SQL_C_ULONG, &t, 0, &cbValue); rank_data->unit[cn].rank = t;
				SQLGetData(hStmt, 2, SQL_C_ULONG, &t, 0, &cbValue); rank_data->unit[cn].score = t;
				SQLGetData(hStmt, 3, SQL_C_CHAR, &rank_data->unit[cn].name, 20, &cbValue);
				EatRearWhiteChar( rank_data->unit[cn].name );		// 010514 YGI
				SQLGetData(hStmt, 4, SQL_C_ULONG, &t, 0, &cbValue); rank_data->unit[cn].nation = t;
				SQLGetData(hStmt, 5, SQL_C_ULONG, &t, 0, &cbValue); rank_data->unit[cn].cls = t;
				SQLGetData(hStmt, 6, SQL_C_ULONG, &t, 0, &cbValue); rank_data->unit[cn].btLevel = t; // CSD-030806
				cn++;
				retCode = SQLFetch(hStmt);
				if( cn == 10 )
				{
					if( retCode == SQL_NO_DATA )  rank_data->is_end = 1;
					break;
				}
			}
			if( cn != 10 ) rank_data->is_end = 1;
			
			SQLFreeStmt(hStmt, SQL_DROP);
			return(1);
		}
	}
	else
	{
		//		SQLTransact(SQL_NULL_HENV, hDBC, SQL_ROLLBACK);
		SQLFreeStmt(hStmt, SQL_DROP);
		SQLerror( szQuerry );		
		return(-2);
	}
}	

int GetRankLadderByName( char *name )
{
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	SQLLEN		cbValue;
	char		szQuerry[255];
	int			t;
	
	/////////////////////////////////////////////////////////////////////////////
	//sprintf(szQuerry, "SELECT cn FROM colossus_rank_read WHERE name = '%s' ", name );
	sprintf(szQuerry, "CALL up_get_colossus_rank_read_cn( '%s')", name);
	/////////////////////////////////////////////////////////////////////////////
	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{
		retCode = SQLFetch(hStmt);
		if( retCode == SQL_NO_DATA )		// ���� ĳ���̴�.
		{	
			SQLFreeStmt(hStmt, SQL_DROP);
			return -1;
		}
		else
		{
			SQLGetData(hStmt, 1, SQL_C_ULONG, &t, 0, &cbValue);
			SQLFreeStmt(hStmt, SQL_DROP);
			return(t);
		}
	}
	else
	{
		//		SQLTransact(SQL_NULL_HENV, hDBC, SQL_ROLLBACK);
		SQLFreeStmt(hStmt, SQL_DROP);
		SQLerror( szQuerry );		
		return(-2);
	}
}	

// 010530 YGI
int SaveNationInfo_SQL( k_save_nation_info *p )
{
	int nation = p->nation;
	
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	char		szQuerry[1024];
	
	sprintf(szQuerry, "UPDATE nation_info SET "
		" nation_money = %.0f, king_support = %d, friend_va = %d, friend_zy = %d, friend_ill = %d, "
		" item_01=%d, item_02=%d, item_03=%d, item_04=%d, item_05=%d, item_06=%d, item_07=%d, item_08=%d, item_09=%d, item_10=%d, "
		" item_11=%d, item_12=%d, item_13=%d, item_14=%d, item_15=%d, item_16=%d, item_17=%d, item_18=%d, item_19=%d, item_20=%d "
		" WHERE nation_code = %d ", 
		p->nation_money, p->king_support, p->friend_va, p->friend_zy, p->friend_ill, 
		p->nation_item[0], p->nation_item[1], p->nation_item[2], p->nation_item[3], p->nation_item[4], 
		p->nation_item[5], p->nation_item[6], p->nation_item[7], p->nation_item[8], p->nation_item[9], 
		p->nation_item[10], p->nation_item[11], p->nation_item[12], p->nation_item[13], p->nation_item[14], 
		p->nation_item[15], p->nation_item[16], p->nation_item[17], p->nation_item[18], p->nation_item[19], 
		nation );
	
	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{
		SQLFreeStmt(hStmt, SQL_DROP);
		return(1);
	}
	else 
	{
		//		SQLTransact(SQL_NULL_HENV, hDBC, SQL_ROLLBACK);
		SQLFreeStmt(hStmt, SQL_DROP);
		return(-2);
	}
}

bool IsFreeLevel( char *szUID )
{
	char name[ 4][ NM_LENGTH]={{0,},};
	HSTMT		hStmt = NULL;
	SQLLEN		cbValue;
	RETCODE		retCode;
	char		szQuerry[256];
	int			i;
	int			lev = 0;
	bool		must_pay = false;
	
	// 4���� �̸��� �����´�. 
	if( GetCharacterNameInID( szUID, name ) == -1 ) 
	{
		return false;
	}
	
	for( i = 0 ; i < 4; i ++)
	{
		if( name[i][0] == 0 ) 
		{
			continue;
		}
		lev = 0;
		
		sprintf(szQuerry, "Select lev from chr_info where name = '%s'", name[i] );
		SQLAllocStmt(hDBC, &hStmt);
		retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
		if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
		{
			retCode = SQLFetch(hStmt);
			if( retCode == SQL_SUCCESS)
			{
				retCode = SQLGetData(hStmt, 1, SQL_C_ULONG, &lev,		0, &cbValue);
			}
			SQLFreeStmt(hStmt, SQL_DROP);
		}
		if( lev > FREE_LEVEL_ ) break;
	}
	
	if( i == 4 ) {return true;}  // Free Level..
	
	return false; 
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////
// ���?�ý��� ����
int UpdateGuildMark( int guild_code, int size, UCHAR *mark)
{
	SQLLEN	clen = 0;
	SQLRETURN	retcode;
	SQLHSTMT	hstmt;
	SQLPOINTER	pParamData;
	char		query[256]={0,};
	
	sprintf(query, "UPDATE guild_list SET mark = ? WHERE code =%d", guild_code );
	
	SQLAllocStmt(hDBC, &hstmt);
	retcode = SQLPrepare(hstmt, (UCHAR *)query, SQL_NTS);
	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) 
	{
		//retcode = SQLBindParameter( hstmt, 1, SQL_PARAM_INPUT, SQL_C_BINARY,SQL_LONGVARBINARY, 0, 0, (SQLPOINTER)mark, 0, &clen);
		retcode= SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_BINARY,SQL_LONGVARBINARY, size, 0, mark, 0, &clen );
		
		if(retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO)
		{
			SQLFreeStmt(hstmt, SQL_DROP);
			return -5 ;
		}
		clen = SQL_LEN_DATA_AT_EXEC(0);
		retcode = SQLExecute(hstmt) ;
		while(retcode == SQL_NEED_DATA) 
		{      
			retcode = SQLParamData(hstmt, &pParamData) ;
			if(retcode == SQL_NEED_DATA) 
			{
				if(pParamData == mark ) SQLPutData(hstmt, mark, size );
			}
			else break ;
		}
		
		retcode = SQLExecute(hstmt) ;
		
		if(retcode == SQL_NEED_DATA)
		{
			retcode= SQLCancel(hstmt) ;
			retcode= SQLFreeStmt(hstmt, SQL_DROP );
			return(1); // succeed
		}
		else
		{
			displaySQLError(hstmt) ;
			retcode= SQLCancel(hstmt) ;
			SQLFreeStmt(hstmt, SQL_DROP);
			return(-3); 
		}
	}
	
	SQLFreeStmt(hstmt, SQL_DROP);
	return 1;
}

int GetGuildMarkImage( int guild_code, char *image, int size, int &mark_num )
{	
	SQLLEN  cbvalue = SQL_C_BINARY ;
	SQLRETURN retcode;
	SQLHSTMT  hstmt;
	char query[256]= {0, } ;
	
	sprintf(query, "SELECT mark_num, mark FROM guild_list WHERE code = %d", guild_code );
	
	SQLAllocStmt(hDBC, &hstmt);
	retcode = SQLExecDirect(hstmt, (UCHAR *)query, SQL_NTS);
	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
		retcode = SQLFetch(hstmt);
		if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
			retcode= SQLGetData(hstmt, 1, SQL_C_ULONG, &mark_num, 0, &cbvalue);
			retcode= SQLGetData(hstmt, 2, SQL_C_BINARY, image, size, &cbvalue);
			SQLFreeStmt(hstmt, SQL_DROP);
			return 1;
		}
		else 
		{
			SQLFreeStmt(hstmt, SQL_DROP);
			return -1 ;
		}
	}
	else {
		SQLFreeStmt(hstmt, SQL_DROP);
		return -2 ;
	}
	
	if(retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
	{
		//		SQLTransact(SQL_NULL_HENV, hDBC, SQL_COMMIT);
		SQLFreeStmt(hstmt, SQL_DROP);
		
		return(1); // succeed
	}
	else
	{
		displaySQLError(hstmt) ;
		//		SQLTransact(SQL_NULL_HENV, hDBC, SQL_ROLLBACK);
		SQLFreeStmt(hstmt, SQL_DROP);
		return(-3); // unknown error
	}
}

int GetGuildImageSize( int guild_code )		// �̹����� ũ�⸦ �����´�.
{		
	SQLLEN		cbValue;
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	char		szQuerry[128];
	int			len = 0;
	
	sprintf( szQuerry, "SELECT LENGTH( mark ) FROM guild_list WHERE code = %d ", guild_code );
	SQLAllocStmt(hDBC, &hStmt);
	
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{	
		retCode = SQLFetch( hStmt );
		retCode = SQLGetData(hStmt, 1, SQL_C_ULONG, &len, 0, &cbValue);
		if( !len ) 
		{
			SQLFreeStmt(hStmt, SQL_DROP);
			return 0;
		}
		SQLFreeStmt(hStmt, SQL_DROP);
		return len;
	}
	SQLFreeStmt(hStmt, SQL_DROP);
	return -1;
}

const int MAX_GUILD = SHRT_MAX; // CSD-030423

int GetEmptyGuildCode( int &guild_code )
{	//< CSD-030423
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	SQLLEN		cbValue;
	char		szQuerry[255];
	
	guild_code = 0;
	int last_code = 0;

	sprintf(szQuerry, "SELECT code FROM guild_list WHERE active = 0 ORDER BY code LIMIT 1" );
	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{
		retCode = SQLFetch(hStmt);
		if( retCode != SQL_NO_DATA )
		{
			SQLGetData(hStmt, 1, SQL_C_ULONG, &last_code,0, &cbValue);
			SQLFreeStmt(hStmt, SQL_DROP);
			guild_code = last_code;
			return 1;
		}
	}

	SQLFreeStmt(hStmt, SQL_DROP);

	sprintf(szQuerry, "SELECT code FROM guild_list ORDER BY code DESC LIMIT 1 " );		// ���� ���?�����?���?��
	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{
		retCode = SQLFetch(hStmt);
		last_code = 0;
		if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO || retCode == SQL_NO_DATA )
		{
			retCode = SQLGetData(hStmt, 1, SQL_C_ULONG, &last_code,0, &cbValue);
			SQLFreeStmt(hStmt, SQL_DROP);

			if (last_code < MAX_GUILD)
			{
				guild_code = last_code + 1;
				return 1;
			}
		}
	}
	SQLFreeStmt(hStmt, SQL_DROP);
	return -1;
}	//> CSD-030423

// �ӽ� ���?���?�����ְ� �� ������ �����´�.
int SaveGuildRegistImsi( int guild_code, k_regist_guild_imsi_db *guild, k_set_guild_info *guild_info )
{
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	SQLLEN		cbValue;
	char		szQuerry[2048];
	int			active;
	
	sprintf(szQuerry, "SELECT active, make FROM guild_list WHERE code = %d ", guild_code );
	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{
		retCode = SQLFetch(hStmt);
		
		int make = 0;
		if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
		{
			retCode = SQLGetData(hStmt, 1, SQL_C_ULONG, &active,0, &cbValue);
			retCode = SQLGetData(hStmt, 2, SQL_C_ULONG, &make,0, &cbValue);
			
			SQLFreeStmt(hStmt, SQL_DROP);
			
			if( active ) 
			{
				return -100;// Ȱ������ ����̴�?
			}
			SQLAllocStmt(hDBC, &hStmt);
			
			sprintf( szQuerry, "DELETE FROM guild_list WHERE code = %d", guild_code );
			retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
		}		
		
		SQLFreeStmt(hStmt, SQL_DROP);
		SQLAllocStmt(hDBC, &hStmt);
		sprintf(szQuerry, "INSERT INTO guild_list ( "
			" code, active, name, guild_lv, make, master, money, nation, friendship, "
			" first, `explain`, mark_num, create_time ) "
			" VALUES ( %d, 1, '%s', 1, %d, '%s', 0, %d, 0, "
			" 1, '%s', 1, NOW() ) ", 
			guild_code, guild->guild_name, make+1, guild->guild_ma, guild->nation, 
			guild->guild_explain );
		
		
		retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
		SQLFreeStmt(hStmt, SQL_DROP);
		
		guild_info->active = 1;		// ��ϵǾ����Ƿ�?Ȱ�����̴�.
		guild_info->first = 1;		// �ӽ��̴�.
		guild_info->guild_code = guild_code;
		guild_info->make = make+1;
		guild_info->mark_num = 1;
		guild_info->nation = guild->nation;
		strcpy( guild_info->guild_name, guild->guild_name );
		return(1);
	}
	else
	{
		//		SQLTransact(SQL_NULL_HENV, hDBC, SQL_ROLLBACK);
		SQLFreeStmt(hStmt, SQL_DROP);
		SQLerror( szQuerry );		
		return(-2);
	}
}

int GetLoginIdByName( char *name, char *login_id )
{
	char		szQuerry[255];
	HSTMT		hStmt = NULL;
	SQLLEN		cbValue;
	RETCODE		retCode;
	
	//sprintf(szQuerry, "select login_id from chr_info where name='%s'", szUNM);
	/////////////////////////////////////////////////////////////////////////////
	sprintf(szQuerry, "CALL up_get_login_id( '%s')", name );
	/////////////////////////////////////////////////////////////////////////////
	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{
		retCode = SQLFetch(hStmt);
		retCode = SQLGetData(hStmt, 1, SQL_C_CHAR, login_id, ID_LENGTH, &cbValue);
		
		if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
		{
			SQLFreeStmt(hStmt, SQL_DROP);
			return 	1;
		}
	}
	SQLFreeStmt(hStmt, SQL_DROP);
	return(0); // Invalid Name
};

int GetDataBasic2ByName( k_char_data_basic2 *data, char *name )
{
	char		szQuerry[255];
	HSTMT		hStmt = NULL;
	SQLLEN		cbValue;
	RETCODE		retCode;
	DWORD t;
	CCharRank name_stats;
	memset( data, 0, sizeof( k_char_data_basic2 ) );
	
	sprintf(szQuerry, "SELECT lev, nation FROM chr_info WHERE name='%s'", name);
	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{
		retCode = SQLFetch(hStmt);
		retCode = SQLGetData(hStmt, 1, SQL_C_ULONG, &t, 0, &cbValue); data->lv = (char)t;
		retCode = SQLGetData(hStmt, 2, SQL_C_ULONG, &t, 0, &cbValue); memcpy( &name_stats, &t, sizeof( DWORD ) );
		data->nation = name_stats.nation;
		
		if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
		{
			SQLFreeStmt(hStmt, SQL_DROP);
			return 	1;
		}
	}
	SQLFreeStmt(hStmt, SQL_DROP);
	return(0); // Invalid Name
}

int GetGuildExplain( int guild_code, int &len, char *explain )
{
	SQLLEN		cbValue;
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	char		szQuerry[128];
	
	sprintf( szQuerry, "SELECT LENGTH( `explain` ), `explain` FROM guild_list WHERE code = %d", guild_code );
	SQLAllocStmt(hDBC, &hStmt);
	
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{	
		retCode = SQLFetch( hStmt );
		retCode = SQLGetData(hStmt, 1, SQL_C_ULONG, &len, 0, &cbValue);
		if( !len ) 
		{
			SQLFreeStmt(hStmt, SQL_DROP);
			return -1;
		}
		retCode = SQLGetData(hStmt, 2, SQL_C_CHAR, explain, len+1, &cbValue);
		SQLFreeStmt(hStmt, SQL_DROP);
		return 1;
	}
	SQLFreeStmt(hStmt, SQL_DROP);
	return -2;
}

int IncreaseNumberOfGuildList( int guild_code, int type, BYTE &number  )		// mark_num, info_num, make_number �� 1 ���������ش�.
{
	char		szQuerry[255];
	HSTMT		hStmt = NULL;
	SQLLEN		cbValue;
	RETCODE		retCode;
	
	DWORD t=0;
	
	switch( type )
	{
	case 0 :		// �̹��� ��ȣ +1
		sprintf(szQuerry, "SELECT mark_num FROM guild_list WHERE code = '%d'", guild_code );
		break;
		
	case 1: 
		sprintf(szQuerry, "SELECT make FROM guild_list WHERE code = '%d'", guild_code );
		break;
		
	case 2: 
		sprintf(szQuerry, "SELECT info_num FROM guild_list WHERE code = '%d'", guild_code );
		break;
	}
	
	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{
		retCode = SQLFetch(hStmt);
		retCode = SQLGetData(hStmt, 1, SQL_C_ULONG, &t, 0, &cbValue); number = (BYTE)t;
	}
	else
	{
		SQLFreeStmt(hStmt, SQL_DROP);
		return 0;		
	}
	
	SQLFreeStmt(hStmt, SQL_DROP);
	number++;
	
	switch( type )
	{
	case 0 :		// �̹��� ��ȣ +1
		sprintf(szQuerry, "UPDATE guild_list SET mark_num = %d WHERE code = '%d'", number, guild_code );
		break;
		
	case 1:			// ���?��ȣ +1 
		sprintf(szQuerry, "UPDATE guild_list SET make = %d WHERE code = '%d'", number, guild_code );
		break;
		
	case 2:			// ���?���� ��ȣ +1
		sprintf(szQuerry, "UPDATE guild_list SET info_num = %d WHERE code = '%d'", number, guild_code );
		break;
	}
	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{
		//		SQLTransact(SQL_NULL_HENV, hDBC, SQL_COMMIT );
		SQLFreeStmt(hStmt, SQL_DROP);
		return(1);
	}
	else 
	{
		//		SQLTransact(SQL_NULL_HENV, hDBC, SQL_ROLLBACK);
		SQLFreeStmt(hStmt, SQL_DROP);
		return(-2);
	}	
}

int SaveGuildRegist( k_guild_regist *guild, int &info_num )
{
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	SQLLEN		cbValue;
	char		szQuerry[255];
	DWORD		t;
	
	sprintf(szQuerry, "SELECT first, info_num FROM guild_list WHERE code = %d ", guild->guild_code );
	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{
		retCode = SQLFetch(hStmt);
		
		info_num = 0;
		t = 0;
		retCode = SQLGetData(hStmt, 1, SQL_C_ULONG, &t,0, &cbValue);
		int first = t;
		t = 0;
		retCode = SQLGetData(hStmt, 2, SQL_C_ULONG, &t,0, &cbValue);
		info_num = t;
		if( first != 1 ) 
		{
			SQLFreeStmt(hStmt, SQL_DROP);
			return -100;// �ӽ� ���?�ƴϳ�..
		}
		
		info_num++;
		
		SQLFreeStmt(hStmt, SQL_DROP);
		SQLAllocStmt(hDBC, &hStmt);
		sprintf(szQuerry, "UPDATE guild_list set "
			" first=%d, sub_master='%s', info_num = %d, "
			" degree0='%s', degree1='%s',degree2='%s',degree3='%s',degree4='%s' "
			" WHERE code = %d",
			0, guild->sub_name, info_num, guild->guild_degree[0], guild->guild_degree[1], 
			guild->guild_degree[2], guild->guild_degree[3], guild->guild_degree[4],
			guild->guild_code );
		
		retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
		SQLFreeStmt(hStmt, SQL_DROP);
		
		return(1);
	}
	else
	{
		//		SQLTransact(SQL_NULL_HENV, hDBC, SQL_ROLLBACK);
		SQLFreeStmt(hStmt, SQL_DROP);
		return(-2);
	}
}

int SaveChangeGuildDegree( k_guild_change_degree *guild, int &info_num )
{
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	SQLLEN		cbValue;
	char		szQuerry[255];
	
	sprintf(szQuerry, "SELECT info_num FROM guild_list WHERE code = %d ", guild->guild_code );
	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{
		retCode = SQLFetch(hStmt);
		
		info_num = 0;
		retCode = SQLGetData(hStmt, 1, SQL_C_ULONG, &info_num,0, &cbValue);
		
		info_num++;
		
		SQLFreeStmt(hStmt, SQL_DROP);
		SQLAllocStmt(hDBC, &hStmt);
		sprintf(szQuerry, "UPDATE guild_list set "
			" info_num = %d, "
			" degree0='%s', degree1='%s',degree2='%s',degree3='%s',degree4='%s' "
			" WHERE code = %d",
			info_num, guild->guild_degree[0], guild->guild_degree[1], 
			guild->guild_degree[2], guild->guild_degree[3], guild->guild_degree[4],
			guild->guild_code );
		
		retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
		SQLFreeStmt(hStmt, SQL_DROP);
		
		return(1);
	}
	else
	{
		//		SQLTransact(SQL_NULL_HENV, hDBC, SQL_ROLLBACK);
		SQLFreeStmt(hStmt, SQL_DROP);
		return(-2);
	}
}

int ChangeGuildMemberList( char *char_name, int guild_code, int guild_degree )
{
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	char		szQuerry[255];
	
	if( !guild_code )		// ���� ���Ѷ�
	{
		sprintf(szQuerry, "DELETE FROM guild_member_list WHERE name = '%s' ", char_name );
		SQLAllocStmt(hDBC, &hStmt);
		SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
		SQLFreeStmt(hStmt, SQL_DROP);
		return 1;
	}
	
	sprintf(szQuerry, "SELECT * FROM guild_member_list WHERE name = '%s' ", char_name );
	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{
		retCode = SQLFetch(hStmt);
		
		int make = 0;
		if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
		{
			SQLFreeStmt(hStmt, SQL_DROP);
			SQLAllocStmt(hDBC, &hStmt);
			sprintf( szQuerry, "UPDATE guild_member_list SET guild_code = %d, guild_degree = %d WHERE name = '%s'", 
				guild_code, guild_degree, char_name );
			retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
			SQLFreeStmt(hStmt, SQL_DROP);
			return 1;
		}		
		
		SQLFreeStmt(hStmt, SQL_DROP);
		SQLAllocStmt(hDBC, &hStmt);
		sprintf(szQuerry, "INSERT INTO guild_member_list ( "
			" name, guild_code, guild_degree, mail ) "
			" VALUES ( '%s', %d, %d, 0 ) ", 
			char_name, guild_code, guild_degree );
		
		retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
		SQLFreeStmt(hStmt, SQL_DROP);
		return(1);
	}
	else
	{
		//		SQLTransact(SQL_NULL_HENV, hDBC, SQL_ROLLBACK);
		SQLFreeStmt(hStmt, SQL_DROP);
		SQLerror( szQuerry );		
		return(-2);
	}
}

// �� ���?��ȣ�� ����?�������?���?���� ��ȣ ��������
int GetGuildMakeNumber( int guild_code, int &make )
{
	SQLLEN		cbValue;
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	char		szQuerry[128];
	
	make = 0;
	sprintf( szQuerry, "SELECT make FROM guild_list WHERE code = %d", guild_code );
	SQLAllocStmt(hDBC, &hStmt);
	
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{	
		retCode = SQLFetch( hStmt );
		retCode = SQLGetData(hStmt, 1, SQL_C_ULONG, &make, 0, &cbValue);
		
		SQLFreeStmt(hStmt, SQL_DROP);
		return 1;
	}
	SQLFreeStmt(hStmt, SQL_DROP);
	return -2;
}

int PutGuildMail( int guild_code, char *title, char *body, char *ch_name, int degree )
{		
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	
	sprintf(szmailQuerry, "INSERT INTO guild_mail ( send_name, guild_code, degree, mail_title, mail_body ) VALUES "
		"( '%s', %d, %d, '%s', '%s' ) ", ch_name, guild_code, degree, title, body );
	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szmailQuerry, SQL_NTS);
	
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{
		SQLFreeStmt(hStmt, SQL_DROP);
		return(1);
	}
	else
	{
		//		SQLTransact(SQL_NULL_HENV, hDBC, SQL_ROLLBACK);
		SQLFreeStmt(hStmt, SQL_DROP);
		return(0);
	}
}

int GetGuildMailTitle( int guild_code, k_guild_mail mail[] )
{
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	SQLLEN		cbValue;
	char		szQuerry[255];
	
	TIMESTAMP_STRUCT	date;
	
	sprintf(szQuerry, "SELECT mail_id, send_name, degree, mail_title, mail_time FROM guild_mail WHERE guild_code = %d ORDER BY mail_id DESC LIMIT 12 ", guild_code );
	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{
		int count = 0;
		retCode = SQLFetch(hStmt);
		while( retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
		{
			retCode = SQLGetData(hStmt, 1, SQL_C_LONG, &mail[count].mail_id, 0, &cbValue);
			retCode = SQLGetData(hStmt, 2, SQL_C_CHAR, mail[count].send_name, 20, &cbValue);
			EatRearWhiteChar( mail[count].send_name );
			retCode = SQLGetData(hStmt, 3, SQL_C_LONG, &mail[count].degree,	0, &cbValue);
			retCode = SQLGetData(hStmt, 4, SQL_C_CHAR, mail[count].mail_title, 60, &cbValue);
			retCode = SQLGetData(hStmt, 5, SQL_C_TIMESTAMP, &date,  sizeof( TIMESTAMP_STRUCT ), &cbValue);
			EatRearWhiteChar( mail[count].mail_title );
			
			mail[count].year = date.year;
			mail[count].month = date.month;
			mail[count].day = date.day;
			
			if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
			{
				count ++;
				retCode = SQLFetch(hStmt);
			}
			else 
			{
				SQLFreeStmt(hStmt, SQL_DROP);
				return -1;
			}
		}
		
		SQLFreeStmt(hStmt, SQL_DROP);
		return( count );
	}
	else 
	{
		SQLFreeStmt(hStmt, SQL_DROP);
		return(-1);
	}
}

int GetGuildMailBody( DWORD guild_mail, DWORD mail_id, char *body )
{
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	SQLLEN		cbValue;
	char		szQuerry[255];
	
	int i=1;
	
	sprintf(szQuerry, "SELECT mail_body FROM guild_mail WHERE mail_id = %u ", mail_id );
	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{
		int count = 0;
		retCode = SQLFetch(hStmt);
		if( retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO )
		{
			retCode = SQLGetData(hStmt, 1, SQL_C_CHAR, body, 1000, &cbValue);
			SQLFreeStmt(hStmt, SQL_DROP);
			CheckMail( mail_id );		// 0811 YGI
			return 1;
		}
		
		SQLFreeStmt(hStmt, SQL_DROP);
		return( 0 );
	}
	else 
	{
		//		SQLTransact(SQL_NULL_HENV, hDBC, SQL_ROLLBACK);
		SQLFreeStmt(hStmt, SQL_DROP);
		return( 0 );
	}
}

int GetGuildMailId( char *name, int &mail_id )
{
	SQLLEN		cbValue;
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	char		szQuerry[128];
	
	mail_id = 0;
	sprintf( szQuerry, "SELECT mail FROM guild_member_list WHERE name = '%s'", name );
	SQLAllocStmt(hDBC, &hStmt);
	
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{	
		retCode = SQLFetch( hStmt );
		retCode = SQLGetData(hStmt, 1, SQL_C_ULONG, &mail_id, 0, &cbValue);
		
		SQLFreeStmt(hStmt, SQL_DROP);
		return 1;
	}
	SQLFreeStmt(hStmt, SQL_DROP);
	return -2;
}

int CheckGuildMail( int guild_code, int &mail_id )
{
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	SQLLEN		cbValue;
	char		szQuerry[255];
	
	mail_id = 0;
	sprintf(szQuerry, "SELECT mail_id FROM guild_mail WHERE guild_code = %d ORDER BY mail_id DESC LIMIT 1 ", guild_code );
	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
				
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{			
		retCode = SQLFetch(hStmt);
		retCode = SQLGetData(hStmt, 1, SQL_C_LONG, &mail_id, 0, &cbValue);
		if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
		{	
			SQLFreeStmt(hStmt, SQL_DROP);
			return 1;
		}	
	}
	
	//	SQLTransact(SQL_NULL_HENV, hDBC, SQL_ROLLBACK);
	SQLFreeStmt(hStmt, SQL_DROP);
	return 0;
}

int UpdateGuildMailId( char *name, DWORD mail_id )
{
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	char		szQuerry[255];
	
	sprintf(szQuerry, "UPDATE guild_member_list SET mail = %u WHERE name ='%s'", mail_id, name );
	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	SQLFreeStmt(hStmt, SQL_DROP);
	
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO) return 1;
	else return 0;
}

int GetGuildMailInfo( DWORD mail_id, int &conv_guild_code, int &conv_degree )
{
	SQLLEN		cbValue;
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	char		szQuerry[128];
	
	conv_guild_code = 0;
	conv_degree = 0;
	sprintf( szQuerry, "SELECT guild_code, degree FROM guild_mail WHERE mail_id = %u", mail_id );
	SQLAllocStmt(hDBC, &hStmt);
	
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{	
		retCode = SQLFetch( hStmt );
		retCode = SQLGetData(hStmt, 1, SQL_C_ULONG, &conv_guild_code, 0, &cbValue);
		retCode = SQLGetData(hStmt, 2, SQL_C_ULONG, &conv_degree, 0, &cbValue);
		
		SQLFreeStmt(hStmt, SQL_DROP);
		return 1;
	}
	SQLFreeStmt(hStmt, SQL_DROP);
	return -2;
}

int DeleteGuildMail( DWORD mail_id )
{
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	char		szQuerry[255];
	
	int i=1;
	
	sprintf(szQuerry, "DELETE FROM guild_mail WHERE mail_id = %u ", mail_id );
	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{
		SQLFreeStmt(hStmt, SQL_DROP);
		return( 1 );
	}
	else 
	{
		//		SQLTransact(SQL_NULL_HENV, hDBC, SQL_ROLLBACK);
		SQLFreeStmt(hStmt, SQL_DROP);
		return( 0 );
	}
}

int GetGuildMasterAndsubMaster( int guild_code, char *master, char *sub_master )
{
	SQLLEN		cbValue;
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	char		szQuerry[128];
	
	sprintf( szQuerry, "SELECT master, sub_master FROM guild_list WHERE code = %d", guild_code );
	SQLAllocStmt(hDBC, &hStmt);
	
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{	
		master[0] = 0;
		sub_master[0] = 0;
		
		retCode = SQLFetch( hStmt );
		retCode = SQLGetData(hStmt, 1, SQL_C_CHAR, master, 20, &cbValue);
		EatRearWhiteChar( master );
		retCode = SQLGetData(hStmt, 2, SQL_C_CHAR, sub_master, 20, &cbValue);
		EatRearWhiteChar( sub_master );
		
		SQLFreeStmt(hStmt, SQL_DROP);
		return 1;
	}
	SQLFreeStmt(hStmt, SQL_DROP);
	return -2;
}

int IsExistGiuildMember( char *name )		// �������?���� ���� �ִ°�?
{
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	char		szQuerry[512];
	
	sprintf(szQuerry, "SELECT guild_code FROM guild_member_list WHERE name = '%s' ", name );
	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{
		retCode = SQLFetch(hStmt);
		SQLFreeStmt(hStmt, SQL_DROP);
		if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
		{
			return 1;		// �� ���?���� ���� �ִ�.
		}
		else return 0;		// ���?���� ���� �ʴ�.
	}
	else
	{
		SQLFreeStmt(hStmt, SQL_DROP);
		SQLerror( szQuerry );
		return(-2);
	}
	return -1;
}

int GetIsFirstGuild( int nGuildCode, int &first )
{
	SQLLEN		cbValue;
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	char		szQuerry[512];
	
	sprintf(szQuerry, "SELECT first FROM guild_list WHERE code = %d ", nGuildCode );
	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{
		retCode = SQLFetch(hStmt);
		retCode = SQLGetData(hStmt, 1, SQL_C_ULONG, &first,0, &cbValue);
		SQLFreeStmt(hStmt, SQL_DROP);
		if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)	
			return 1;
		else return 0;
	}
	SQLFreeStmt(hStmt, SQL_DROP);
	return -1;
}

struct k_name
{
	char name[20];
};
int CheckGuildMemberForRegistGuild( int nGuildCode )
{
	SQLLEN		cbValue;
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	char		szQuerry[512];
	char		name[20];
	
	int first = 0;
	int ret = GetIsFirstGuild( nGuildCode, first );
	if( ret != 1 ) return -100;		// ������ ������ �ְų� ���� ����.
	if( first != 1 ) return -200;		// �ӽ� ���?�ƴϴ�.
	
	int nRowCount = 0;
	sprintf( szQuerry, "guild_code = %d", nGuildCode );
	ret = GetRowLineOfSQL( "guild_member_list", "name", &nRowCount, szQuerry );
	if( nRowCount < MIN_GUILD_COUNT ) return -400; // ���?���� �����ϴ�. 
	
	
	sprintf(szQuerry, "SELECT name FROM guild_member_list WHERE guild_code = %d ", nGuildCode );
	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{
		k_name *name_data;
		name_data = new k_name[nRowCount];
		
		retCode = SQLFetch(hStmt);
		int count = 0;		// ������ 30�̻��� ���?üũ
		while(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
		{
			retCode = SQLGetData(hStmt, 1, SQL_C_CHAR, name, 20, &cbValue);
			strcpy( name_data[count].name, name );
			count ++;
			retCode = SQLFetch(hStmt);
		}
		SQLFreeStmt(hStmt, SQL_DROP);
		
		count = 0;
		k_char_data_basic2 sCharData;
		for( int i=0; i<nRowCount; i++ )
		{
			ret = GetDataBasic2ByName( &sCharData, name_data[i].name );
			if( ret && sCharData.lv >= 30 ) count ++;
		}
		
		delete[] name_data;
		return count;
	}
	else
	{
		SQLFreeStmt(hStmt, SQL_DROP);
		SQLerror( szQuerry );
		return(-2);
	}
	return -1;
}

int DeleteGuild( int guild_code )
{
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	char		szQuerry[255];
	
	sprintf(szQuerry, "UPDATE guild_list SET active = 0 WHERE code = %d", guild_code );
	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	SQLFreeStmt(hStmt, SQL_DROP);
	
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO) return 1;
	else return 0;
}

int DeleteGuildMemberList( int guild_code )
{
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	char		szQuerry[255];
	
	sprintf(szQuerry, "DELETE FROM guild_member_list WHERE guild_code = %d", guild_code );
	
	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	SQLFreeStmt(hStmt, SQL_DROP);
	
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO) return 1;
	else return 0;	
}

int DeleteImsiGuild( int *guild_code, int &guild_code_count )
{
	SQLLEN		cbValue;
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	char		szQuerry[255];
	int ret = -1;
	
	// ���ڰ� 20���� ���� Ȱ������ ���?
	sprintf(szQuerry, "SELECT code FROM guild_list WHERE first = 1  AND active = 1 AND ( DATE_SUB(NOW(), INTERVAL 20 DAY) > create_time )" );
	
	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{
		guild_code_count = 0;
		retCode = SQLFetch(hStmt);
		while(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
		{
			retCode = SQLGetData(hStmt, 1, SQL_C_LONG, &guild_code[guild_code_count++], 0, &cbValue);
			retCode = SQLFetch(hStmt);
			ret = 1;
		}
	}
	SQLFreeStmt(hStmt, SQL_DROP);
	return ret;
}

int GetEventItemNumber( int *item_no, char *szID)
{	//< CSD-030723
	char	szQuerry[255];
	HSTMT		hStmt;
	RETCODE		retCode;
	SQLLEN		cbValue;
	
	sprintf( szQuerry, "SELECT event1, event2, event3, event4, event5, event6, event7, event8, event9, event10 "
		               "FROM chr_log_info WHERE login_id = '%s' ", szID );//020823 lsw
	
	SQLAllocStmt(hDBC_TotalDB, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{
		retCode = SQLFetch(hStmt);
		retCode = SQLGetData(hStmt, 1, SQL_C_LONG, &item_no[0], 0, &cbValue);
		retCode = SQLGetData(hStmt, 2, SQL_C_LONG, &item_no[1], 0, &cbValue);
		retCode = SQLGetData(hStmt, 3, SQL_C_LONG, &item_no[2], 0, &cbValue);
		retCode = SQLGetData(hStmt, 4, SQL_C_LONG, &item_no[3], 0, &cbValue);
		retCode = SQLGetData(hStmt, 5, SQL_C_LONG, &item_no[4], 0, &cbValue);
		retCode = SQLGetData(hStmt, 6, SQL_C_LONG, &item_no[5], 0, &cbValue);
		retCode = SQLGetData(hStmt, 7, SQL_C_LONG, &item_no[6], 0, &cbValue);
		retCode = SQLGetData(hStmt, 8, SQL_C_LONG, &item_no[7], 0, &cbValue);
		retCode = SQLGetData(hStmt, 9, SQL_C_LONG, &item_no[8], 0, &cbValue);
		retCode = SQLGetData(hStmt, 10, SQL_C_LONG, &item_no[9], 0, &cbValue);

		SQLFreeStmt(hStmt, SQL_DROP);
		return 1;
	}
	else
	{
		MyLog(0,"GetEventItemNumber() Faild. LoginID = %s - by soto",szID);//soto-cn031205
		SQLFreeStmt(hStmt, SQL_DROP);
		return 0;
	}

	return 0;
}	//> CSD-030723

int DeleteEventItemNumber( char *szID, int count )//020822 lsw
{
	char	szQuerry[255];
	HSTMT		hStmt;
	RETCODE		retCode;
	sprintf( szQuerry, "UPDATE chr_log_info SET event%d = 0 WHERE login_id = '%s' ", count ,szID );
	
	SQLAllocStmt(hDBC_TotalDB, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	SQLFreeStmt(hStmt, SQL_DROP);
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{
		return 1;
	}
	return 0;
}

int GetCharOfGuildMemberList( int guild_code, k_get_guild_memeber_list *data )
{
	SQLLEN		cbValue;
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	char		szQuerry[255];
	int			count=0;
	
	sprintf(szQuerry, "SELECT count( name ) FROM guild_member_list WHERE guild_code = %d AND guild_degree = %d", guild_code, data->degree );
	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{
		retCode = SQLFetch(hStmt);
		retCode = SQLGetData(hStmt, 1, SQL_C_LONG, &count, 0, &cbValue);
		if( count > 100 ) count = 100;
		data->count = count;
		SQLFreeStmt(hStmt, SQL_DROP);
	}
	else
	{
		SQLFreeStmt(hStmt, SQL_DROP);
		return -1;
	}
	if( !count ) return 0;
	
	sprintf(szQuerry, "SELECT name FROM guild_member_list WHERE guild_code = %d AND guild_degree = %d", guild_code, data->degree );
	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{
		retCode = SQLFetch(hStmt);
		count = 0;
		while(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
		{
			retCode = SQLGetData(hStmt, 1, SQL_C_CHAR, data->name[count].name, 20, &cbValue);
			retCode = SQLFetch(hStmt);
			count++;
			if( count >= 100 ) break;
		}
	}
	SQLFreeStmt(hStmt, SQL_DROP);
	return 1;
}

int GetGuildNotice( int guild_code, char *text )
{
	char		szQuerry[255];
	HSTMT		hStmt;
	RETCODE		retCode;
	SQLLEN		cbValue;
	int ret = 0;
	
	sprintf( szQuerry, "SELECT notice FROM guild_list WHERE code = %d ", guild_code );
	
	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{
		retCode = SQLFetch(hStmt);
		retCode = SQLGetData(hStmt, 1, SQL_C_CHAR, text, 1000, &cbValue);
		EatRearWhiteChar( text );
		SQLFreeStmt(hStmt, SQL_DROP);
		ret = 1;
	}
	else 
	{
		ret = -1;
		SQLFreeStmt(hStmt, SQL_DROP);
	}
	return ret;
}

int RegistFriend_SQL( char *name1, char *name2 )
{
	char		szQuerry[1024];
	HSTMT		hStmt;
	RETCODE		retCode;
	SQLLEN		cbValue;
	int ret = 0;
	
	sprintf( szQuerry, "SELECT friend1, friend2, friend3, friend4, friend5, "
		" friend6, friend7, friend8, friend9, friend10, "
		" friend11, friend12, friend13, friend14, friend15, "
		" friend16, friend17, friend18, friend19, friend20, "
		" friend21, friend22, friend23, friend24, friend25, "
		" friend26, friend27, friend28, friend29, friend30 "
		" FROM friend_list WHERE name = '%s' ", name1 );
	
	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{
		int cn = 1;
		char get_name[20];
		
		retCode = SQLFetch(hStmt);
		if( retCode == SQL_NO_DATA )
		{
			SQLFreeStmt(hStmt, SQL_DROP);
			sprintf( szQuerry, "INSERT INTO friend_list ( name, friend1 ) values ( '%s', '%s' )",name1, name2 );
			Querry_SQL( szQuerry );
			return 1;
		}
		while( retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
		{
			ZeroMemory( get_name, 20 );
			retCode = SQLGetData(hStmt, cn, SQL_C_CHAR, get_name, 20, &cbValue);
			EatRearWhiteChar( get_name );
			if( !get_name[0] )		// NULL�� ���?
			{
				SQLFreeStmt(hStmt, SQL_DROP);
				sprintf( szQuerry, " UPDATE friend_list SET friend%d = '%s' WHERE name = '%s'", cn, name2, name1 );
				Querry_SQL( szQuerry );
				return 1;
			}
			else
			{
				if( strcmp( get_name, name2 ) == 0 )
				{
					SQLFreeStmt(hStmt, SQL_DROP);
					return 200;
				}
			}
			cn++;
			if( cn > 30 ) break;
		}
		SQLFreeStmt(hStmt, SQL_DROP);
		return 100;
	}
	else 
	{
		ret = -1;
		SQLFreeStmt(hStmt, SQL_DROP);
	}
	return ret;
}

int GetFriend_SQL( char *name, char data_name[][20] )
{
	char		szQuerry[1024];
	HSTMT		hStmt;
	RETCODE		retCode;
	SQLLEN		cbValue;
	int ret = 0;
	
	sprintf( szQuerry, "SELECT friend1, friend2, friend3, friend4, friend5, "
		" friend6, friend7, friend8, friend9, friend10, "
		" friend11, friend12, friend13, friend14, friend15, "
		" friend16, friend17, friend18, friend19, friend20, "
		" friend21, friend22, friend23, friend24, friend25, "
		" friend26, friend27, friend28, friend29, friend30 "
		" FROM friend_list WHERE name = '%s' ", name );
	
	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{
		int cn = 1;
		char get_name[20];
		
		retCode = SQLFetch(hStmt);
		while(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
		{
			ZeroMemory( get_name, 20 );
			retCode = SQLGetData(hStmt, cn, SQL_C_CHAR, get_name, 20, &cbValue);
			EatRearWhiteChar( get_name );
			strcpy( data_name[cn-1], get_name );
			//retCode = SQLFetch(hStmt);
			cn++;
			if( cn > 30 ) break;
		}
		SQLFreeStmt(hStmt, SQL_DROP);
		return 1;
	}
	else 
	{
		ret = -1;
		SQLFreeStmt(hStmt, SQL_DROP);
	}
	return ret;
}

int SaveEventItem( char *name, int item_no, int count )
{
	char		szQuerry[1024];
	HSTMT		hStmt;
	RETCODE		retCode;
	
	char table_name[20];
	
	sprintf( table_name, "event_%d", item_no );
	
	sprintf( szQuerry,
	" CREATE TABLE %s (	\
								  `name` char (20) NOT NULL ,	\
								  `count` int NOT NULL,			\
								  `date` datetime NULL			\
								  ) ", table_name );
	
	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	SQLFreeStmt(hStmt, SQL_DROP);
	
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{
		sprintf( szQuerry,
		"ALTER TABLE %s ADD PRIMARY KEY (`name`, `count`)", table_name );

		SQLAllocStmt(hDBC_ChrLogDB, &hStmt);
		retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
		SQLFreeStmt(hStmt, SQL_DROP);
		MyLog( LOG_NORMAL, "'%s' Table successfully Created ", table_name );
	}
	
	sprintf( szQuerry, "SELECT name FROM %s WHERE name = '%s' ", table_name, name );
	
	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{
		retCode = SQLFetch(hStmt);
		SQLFreeStmt(hStmt, SQL_DROP);
		if( retCode == SQL_NO_DATA )
		{			
			sprintf( szQuerry, "INSERT INTO %s ( name, count, date ) values ( '%s', %d, NOW() )", table_name, name, count );
			return Querry_SQL( szQuerry );
		}
		if( retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
		{
			sprintf( szQuerry, "UPDATE %s SET count = count+%d, date = NOW() WHERE name = '%s'", table_name, count, name );
			return Querry_SQL( szQuerry );
		}
		return -2;
	}
	else 
	{
		SQLFreeStmt(hStmt, SQL_DROP);
		return -1;
	}	
}

int GetGuildItemIndexAndCount( int guild_code, int &max, DWORD *index )
{
	char		szQuerry[1024];
	HSTMT		hStmt;
	RETCODE		retCode;
	SQLLEN		cbValue;
	
	sprintf( szQuerry, "SELECT count( * ) FROM guild_item WHERE guild_code = %d", guild_code );
	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{
		max = 0;
		SQLFetch(hStmt);
		SQLGetData(hStmt, 1, SQL_C_LONG, &max, 0, &cbValue);
		SQLFreeStmt(hStmt, SQL_DROP);
		if( !max ) return -100;	// �������� �ϳ��� ����.
		if( max > 150 ) max = 150;
		
		sprintf( szQuerry, "SELECT cn FROM guild_item WHERE guild_code = %d ORDER BY cn LIMIT 150", guild_code );
		SQLAllocStmt(hDBC, &hStmt);
		retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
		if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
		{
			retCode  = SQLFetch(hStmt);
			int count = 0;
			while( retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
			{
				retCode = SQLGetData(hStmt, 1, SQL_C_LONG, &index[count++], 0, &cbValue);
				retCode  = SQLFetch(hStmt);
			}
			SQLFreeStmt(hStmt, SQL_DROP);
			return 1;
		}
	}
	SQLFreeStmt(hStmt, SQL_DROP);
	return -1;
}

int GetGuildItem( int guild_code, int cn, ItemAttr *item, int &count, DWORD *index )
{
	char		szQuerry[1024];
	HSTMT		hStmt;
	RETCODE		retCode;
	SQLLEN		cbValue;
	
	sprintf( szQuerry, "SELECT item_no, attr1, attr2, attr3, attr4, attr5, attr6, cn "
		"FROM guild_item WHERE guild_code = %d AND cn >= %d ORDER BY cn LIMIT 15", guild_code, cn );
	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{
		count = 0;
		retCode = SQLFetch(hStmt);
		while( retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
		{
			int item_no = 0;
			SQLGetData(hStmt, 1, SQL_C_LONG, &item_no, 0, &cbValue);
			item[count].item_no = item_no;
			SQLGetData(hStmt, 2, SQL_C_LONG, &item[count].attr[0], 0, &cbValue);
			SQLGetData(hStmt, 3, SQL_C_LONG, &item[count].attr[1], 0, &cbValue);
			SQLGetData(hStmt, 4, SQL_C_LONG, &item[count].attr[2], 0, &cbValue);
			SQLGetData(hStmt, 5, SQL_C_LONG, &item[count].attr[3], 0, &cbValue);
			SQLGetData(hStmt, 6, SQL_C_LONG, &item[count].attr[4], 0, &cbValue);
			SQLGetData(hStmt, 7, SQL_C_LONG, &item[count].attr[5], 0, &cbValue);
			SQLGetData(hStmt, 8, SQL_C_LONG, &index[count], 0, &cbValue);
			count ++;
			retCode = SQLFetch(hStmt);
		}
		SQLFreeStmt(hStmt, SQL_DROP);
		return 1;
	}
	SQLFreeStmt(hStmt, SQL_DROP);
	return -1;
}

int InsertGuildItem( int guild_code, ItemAttr *pItem, char *name )
{
	char		szQuerry[1024];
	sprintf( szQuerry, "INSERT into guild_item ( item_no, attr1, attr2, attr3, attr4, attr5, attr6, "
		" name, guild_code ) VALUES ( %d, %d, %d, %d, %d, %d, %d, '%s', %d ) ", 
		pItem->item_no, pItem->attr[0], pItem->attr[1], pItem->attr[2], pItem->attr[3], pItem->attr[4], 
		pItem->attr[5], name, guild_code );
	
	return Querry_SQL( szQuerry );
}

int GetGuildItemOne( int guild_code, DWORD index, ItemAttr *pItem )		// ���� ������ �ϳ� ��������
{
	char		szQuerry[1024];
	HSTMT		hStmt;
	RETCODE		retCode;
	SQLLEN		cbValue;
	
	sprintf( szQuerry, "SELECT item_no, attr1, attr2, attr3, attr4, attr5, attr6 "
		"FROM guild_item WHERE guild_code = %d AND cn = %d", guild_code, index );
	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{
		retCode = SQLFetch(hStmt);
		if( retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
		{
			int item_no = 0;
			SQLGetData(hStmt, 1, SQL_C_LONG, &item_no, 0, &cbValue);
			pItem->item_no = item_no;
			SQLGetData(hStmt, 2, SQL_C_LONG, &pItem->attr[0], 0, &cbValue);
			SQLGetData(hStmt, 3, SQL_C_LONG, &pItem->attr[1], 0, &cbValue);
			SQLGetData(hStmt, 4, SQL_C_LONG, &pItem->attr[2], 0, &cbValue);
			SQLGetData(hStmt, 5, SQL_C_LONG, &pItem->attr[3], 0, &cbValue);
			SQLGetData(hStmt, 6, SQL_C_LONG, &pItem->attr[4], 0, &cbValue);
			SQLGetData(hStmt, 7, SQL_C_LONG, &pItem->attr[5], 0, &cbValue);
			SQLFreeStmt(hStmt, SQL_DROP);
			
			sprintf( szQuerry, "DELETE FROM guild_item WHERE cn = %d", index );
			Querry_SQL( szQuerry );
			return 1;
		}
		else
		{
			SQLFreeStmt(hStmt, SQL_DROP);
			return 100;		// ����.
		}
	}
	SQLFreeStmt(hStmt, SQL_DROP);
	return -1;
}

int GetGuildItemOne( int guild_code, int item_no, ItemAttr *pItem, char *name )		// ���� ������ �ϳ� ��������
{
	char		szQuerry[1024];
	HSTMT		hStmt;
	RETCODE		retCode;
	SQLLEN		cbValue;
	
	sprintf( szQuerry, "SELECT item_no, attr1, attr2, attr3, attr4, attr5, attr6, name, cn "
		"FROM guild_item WHERE guild_code = %d AND item_no = %d LIMIT 1", guild_code, item_no );
	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{
		retCode = SQLFetch(hStmt);
		if( retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
		{
			int item_no = 0;
			int index = 0;
			SQLGetData(hStmt, 1, SQL_C_LONG, &item_no, 0, &cbValue);
			pItem->item_no = item_no;
			SQLGetData(hStmt, 2, SQL_C_LONG, &pItem->attr[0], 0, &cbValue);
			SQLGetData(hStmt, 3, SQL_C_LONG, &pItem->attr[1], 0, &cbValue);
			SQLGetData(hStmt, 4, SQL_C_LONG, &pItem->attr[2], 0, &cbValue);
			SQLGetData(hStmt, 5, SQL_C_LONG, &pItem->attr[3], 0, &cbValue);
			SQLGetData(hStmt, 6, SQL_C_LONG, &pItem->attr[4], 0, &cbValue);
			SQLGetData(hStmt, 7, SQL_C_LONG, &pItem->attr[5], 0, &cbValue);
			SQLGetData(hStmt, 8, SQL_C_CHAR, name, 20, &cbValue);
			SQLGetData(hStmt, 9, SQL_C_LONG, &index, 0, &cbValue);
			EatRearWhiteChar( name );
			SQLFreeStmt(hStmt, SQL_DROP);
			sprintf( szQuerry, "DELETE FROM guild_item WHERE cn = %d", index );
			Querry_SQL( szQuerry );
			return 1;
		}
		else
		{
			SQLFreeStmt(hStmt, SQL_DROP);
			return 100;		// ����.
		}
	}
	SQLFreeStmt(hStmt, SQL_DROP);
	return -1;
}


// ���?������ ������
int GetGuildItemPay( int guild_code, DWORD &box_money )
{
	char		szQuerry[255];
	HSTMT		hStmt;
	RETCODE		retCode;
	SQLLEN		cbValue;
	
	sprintf( szQuerry, "SELECT box_money FROM guild_list WHERE code = %d ", guild_code );
	
	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{
		retCode = SQLFetch(hStmt);
		retCode = SQLGetData(hStmt, 1, SQL_C_ULONG, &box_money, 0, &cbValue);
		SQLFreeStmt(hStmt, SQL_DROP);
		return 1;
	}
	else 
	{
		SQLFreeStmt(hStmt, SQL_DROP);
		return -1;
	}
}

int GetLastGuildCode( int &last_guild_code )
{	//< CSD-030324
	char		szQuerry[255];
	HSTMT		hStmt;
	RETCODE		retCode;
	SQLLEN		cbValue;

	last_guild_code = 0;

	sprintf( szQuerry, "SELECT code FROM guild_list ORDER BY code DESC LIMIT 1" );

	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{
		retCode = SQLFetch(hStmt);
		retCode = SQLGetData(hStmt, 1, SQL_C_ULONG, &last_guild_code, 0, &cbValue);
		SQLFreeStmt(hStmt, SQL_DROP);
		return 1;
	}
	else 
	{
		SQLFreeStmt(hStmt, SQL_DROP);
		return -1;
	}
}	//> CSD-030324

int GetActiveByGuildList( int guild_code, int &active )
{
	char		szQuerry[255];
	HSTMT		hStmt;
	RETCODE		retCode;
	SQLLEN		cbValue;
	
	active = 0;
	
	sprintf( szQuerry, "SELECT active FROM guild_list WHERE code = %d ", guild_code );
	
	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{
		retCode = SQLFetch(hStmt);
		retCode = SQLGetData(hStmt, 1, SQL_C_ULONG, &active, 0, &cbValue);
		SQLFreeStmt(hStmt, SQL_DROP);
		return 1;
	}
	else 
	{
		SQLFreeStmt(hStmt, SQL_DROP);
		return -1;
	}
}

int CheckGuildMasterLastConnect( int guild_code )
{
	// ���?�������� �ֱ� ���� �ð��� üũ�Ѵ�.
	char		szQuerry[255];
	HSTMT		hStmt;
	RETCODE		retCode;
	SQLLEN		cbValue;
	
	//sprintf( szQuerry, "select * from guild_list where code = %d AND (master_last_connect < getdate()-20)", guild_code );
	int RowCount = 0;
	sprintf( szQuerry, "code = %d AND (master_last_connect < DATE_SUB(NOW(), INTERVAL 15 DAY))", guild_code  );
	GetRowLineOfSQL( "guild_list", "name", &RowCount, szQuerry );
	if( RowCount > 0 )		// ��ü�� �����ϴ�.
		return 1;
	
	TIMESTAMP_STRUCT temp_date ={0,};
	sprintf( szQuerry, "SELECT master_last_connect FROM guild_list WHERE code = %d", guild_code );
	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{
		SQLFetch(hStmt);
		SQLGetData(hStmt, 1, SQL_C_TIMESTAMP, &temp_date,  sizeof( TIMESTAMP_STRUCT ), &cbValue);
		SQLFreeStmt(hStmt, SQL_DROP);
		if( temp_date.year == 0 )	// �ð��� null �� ���?10�� �� ���ڷ� ����
		{
			sprintf( szQuerry, "UPDATE guild_list SET master_last_connect = DATE_SUB(NOW(), INTERVAL 11 DAY)  WHERE code = %d", guild_code );
			Querry_SQL( szQuerry );
			return 2;	// ��ü �Ұ��� : ���?2
		}
		return 3;		// ��ü �Ұ��� : ���?1
	}
	else 
	{
		SQLFreeStmt(hStmt, SQL_DROP);
		return -1;		// ��ü �Ұ���
	}
}

int GetGuildHouseInfo( k_guild_house_info *data, int &count, int nation )
{
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	SQLLEN		cbValue;
	char		szQuerry[512];
	TIMESTAMP_STRUCT temp_time;
	
	sprintf(szQuerry, "SELECT id, map_name, lv, guild_code, DATE_SUB(NOW(), INTERVAL 0 SECOND), red, blue FROM guild_house WHERE nation = %d", nation );
	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	if( !SQLOK(retCode) )
	{
		SQLFreeStmt(hStmt, SQL_DROP);
		return( retCode );
	}
	retCode = SQLFetch( hStmt );
	count = 0;
	while( SQLOK( retCode ) )
	{
		int temp_int = 0;
		retCode = SQLGetData(hStmt, 1, SQL_C_ULONG, &temp_int, 0, &cbValue); data->data[count].id = temp_int;
		retCode = SQLGetData(hStmt, 2, SQL_C_CHAR, data->data[count].map_name, 20, &cbValue);
		EatRearWhiteChar( data->data[count].map_name );
		retCode = SQLGetData(hStmt, 3, SQL_C_ULONG, &temp_int, 0, &cbValue); data->data[count].lv = temp_int;
		retCode = SQLGetData(hStmt, 4, SQL_C_ULONG, &temp_int, 0, &cbValue); data->data[count].guild_code = temp_int;
		temp_time.year = 0;
		SQLGetData(hStmt, 5, SQL_C_TIMESTAMP, &temp_time,  sizeof( TIMESTAMP_STRUCT ), &cbValue);
		data->data[count].time = 0;
		if( temp_time.year >= 1900 )
		{
			data->data[count].time += (temp_time.year-1900 )*365;
			data->data[count].time += (temp_time.month-1)*30;
			data->data[count].time += temp_time.day;
		}
		retCode = SQLGetData(hStmt, 6, SQL_C_ULONG, &temp_int, 0, &cbValue); data->data[count].red= temp_int;
		retCode = SQLGetData(hStmt, 7, SQL_C_ULONG, &temp_int, 0, &cbValue); data->data[count].blue= temp_int;
		
		count++;
		retCode = SQLFetch( hStmt );
	}
	
	//SQLTransact(SQL_NULL_HENV, hDBC, SQL_ROLLBACK);
	SQLFreeStmt(hStmt, SQL_DROP);
	return( 1 );
}

int GetGuildHouseInfoById( k_change_guild_house_info_db2map *data, int house_id )
{
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	SQLLEN		cbValue;
	char		szQuerry[512];
	TIMESTAMP_STRUCT temp_time;
	
	sprintf(szQuerry, "SELECT guild_code, date FROM guild_house WHERE id = %d", house_id );
	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	if( !SQLOK(retCode) )
	{
		SQLFreeStmt(hStmt, SQL_DROP);
		return( retCode );
	}
	
	retCode = SQLFetch( hStmt );
	if( SQLOK( retCode ) )
	{
		int temp_int = 0;
		retCode = SQLGetData(hStmt, 1, SQL_C_ULONG, &temp_int, 0, &cbValue); data->guild_code = temp_int;
		retCode = SQLGetData(hStmt, 2, SQL_C_TIMESTAMP, &temp_time,  sizeof( TIMESTAMP_STRUCT ), &cbValue);
		data->year = temp_time.year;
		data->month = temp_time.month;
		data->day= temp_time.day;
	}
	
	SQLFreeStmt(hStmt, SQL_DROP);
	return( 1 );
}

int GetCheckGuildHouse( char *pHouseId, int &count )
{
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	SQLLEN		cbValue;
	char		szQuerry[512];
	int temp_int;
	
	sprintf(szQuerry, "SELECT id FROM guild_house WHERE NOW() > date" );	//������ ����Ͽ�?
	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	if( !SQLOK(retCode) )
	{
		SQLFreeStmt(hStmt, SQL_DROP);
		return( retCode );
	}
	retCode = SQLFetch( hStmt );
	count = 0;
	while( SQLOK( retCode ) )
	{
		retCode = SQLGetData(hStmt, 1, SQL_C_ULONG, &temp_int, 0, &cbValue); pHouseId[count] = temp_int;
		count++;
		retCode = SQLFetch( hStmt );
	}
	
	//SQLTransact(SQL_NULL_HENV, hDBC, SQL_ROLLBACK);
	SQLFreeStmt(hStmt, SQL_DROP);
	return( 1 );
}

int GetSkillPoint(const char* pName)
{ //< CSD-020724
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	SQLLEN		cbValue;
	char		szQuerry[512];
	
	sprintf(szQuerry, "SELECT point FROM battle_skill_point WHERE name = '%s' ", pName);
	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	if( !SQLOK(retCode) )
	{
		SQLFreeStmt(hStmt, SQL_DROP);
		return( retCode );
	}
	retCode = SQLFetch( hStmt );
	
	int nPoint = 0;
	
	while( SQLOK( retCode ) )
	{
		retCode = SQLGetData(hStmt, 1, SQL_C_ULONG, &nPoint, 0, &cbValue); 
		retCode = SQLFetch( hStmt );
	}
	
	//SQLTransact(SQL_NULL_HENV, hDBC, SQL_ROLLBACK);
	SQLFreeStmt(hStmt, SQL_DROP);
	return nPoint;
} //> CSD-020724

int ClearSkillPoint(const char* pName)
{ //< CSD-020724
	char	szQuerry[255];
	sprintf( szQuerry, "UPDATE battle_skill_point SET used = 1 WHERE name = '%s' ", pName);
	return Querry_SQL(szQuerry);
} //> CSD-020724



extern int GetServerSetNum();
int SaveItemLog( k_item_log *pData )
{
	if( !hDBC_ChrLogDB ) return -100;		// ���� �Ǿ� ���� ���� �����̴�.
	
	//acer7
	MyLog( 0, " ItemLog : %s->%s, item_no(%d), type(%d)",pData->name1, pData->name2, pData->item.item_no, pData->type );
	
	char szQuerry[1024];
	sprintf(szQuerry, "INSERT INTO log_item_event VALUES ( %d, %d, %d, %d, %d, %d, %d, "
		"%d, %d, '%s', '%s', %d, %d, NOW() ) ", 
		pData->item.item_no, pData->item.attr[0],
		pData->item.attr[1],pData->item.attr[2],
		pData->item.attr[3],pData->item.attr[4],
		pData->item.attr[5],		pData->port,
		pData->type, pData->name1, pData->name2, pData->lv,
		GetServerSetNum() );
	//MyLog(0,  szQuerry ); 
	return SQLOK( Querry_SQL( szQuerry, hDBC_ChrLogDB ) );
}


// 020826 YGI
int CheckSalvationNameBasic2( char *name )
{
/* 24 �ð��� ���������� ������ ���?
int nRowCount = 0;
char condition[256];
sprintf( condition, "name = '%s' AND date > DATE_SUB(NOW(), INTERVAL 1 DAY)", name );
GetRowLineOfSQL( "salvation_name", "name", &nRowCount, condition );
return nRowCount?1:0;
	*/
	HSTMT		hStmt = NULL;
	RETCODE		retCode = 0;
	SQLLEN		cbValue = 0;
	char		szQuerry[512]={0,};
	TIMESTAMP_STRUCT temp_time, today;
	
	sprintf(szQuerry, "SELECT date, NOW() FROM salvation_name WHERE name = '%s'", name );
	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	if( !SQLOK(retCode) )
	{
		::SQLFreeStmt(hStmt, SQL_DROP);
		return -1;
	}
	retCode = ::SQLFetch( hStmt );
	if( retCode == SQL_NO_DATA )		// ���?�����ϴ�.
	{
		::SQLFreeStmt(hStmt, SQL_DROP);
		return 1;
	}
	
	if( SQLOK( retCode ) )
	{
		::SQLGetData(hStmt, 1, SQL_C_TIMESTAMP, &temp_time,  sizeof( TIMESTAMP_STRUCT ), &cbValue);
		::SQLGetData(hStmt, 2, SQL_C_TIMESTAMP, &today,  sizeof( TIMESTAMP_STRUCT ), &cbValue);
		::SQLFreeStmt(hStmt, SQL_DROP);
		if( temp_time.year != today.year || temp_time.month != today.month || temp_time.day != today.day ) 
		{
			return 1;
		}
		else return 0;
	}
	
	::SQLFreeStmt(hStmt, SQL_DROP);
	return( -1 );
}

//021030 YGI
// �̺�Ʈ ���̺��� ���� �ؼ� �������� ������ �ִ����� üũ�Ѵ�.
// �ش��������?������ event_startitem_recv_id �� üũ�ϰ� �������� ������ �����Ѵ�.
int CheckEventITem( CHARLIST *ch )
{
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	SQLLEN		cbValue;
	int i = 0;		// for ��
	int ct = 0;		// ����Ÿ ������ �� �ε�����
	int count = 0;	// �������� �ִ��� Ȯ���ϴ� ��ƾ���� ���?
	char condition[255] = {0,};	// Ȯ�ο� ����
	int ret = 0;
	
	int nRowCount = 0;
	::GetRowLineOfSQL( "Event_StartItem", "*", &nRowCount, "event_start_date <= NOW() AND event_end_date >= NOW()" );
	if( !nRowCount ) {return 0;}		// �ش� �̺�Ʈ�� ����.
	
	vector<k_startitem_basic> vtSI;
	vtSI.reserve(nRowCount);
	
	const char szQuerry[] = "SELECT event_no, `all`, sex, class, nation, item1_no, item1_mutant, item1_grade, "
		" item2_no, item2_mutant, item2_grade, table_name  FROM Event_StartItem WHERE event_start_date <= NOW() AND event_end_date >= NOW()";
	
	::SQLAllocStmt(hDBC, &hStmt);
	retCode = ::SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	if( !SQLOK(retCode) )
	{
		::SQLFreeStmt(hStmt, SQL_DROP);
		return 0;
	}
	
	// ����Ÿ �������� ��ƾ
	retCode = SQLFetch( hStmt );
	while( SQLOK( retCode ) )
	{
		if( retCode == SQL_NO_DATA ) break;

		k_startitem_basic *pData = &vtSI[ct];		// ���� int �̱� ������ 
		int i = 0;
		retCode = ::SQLGetData(hStmt, ++i, SQL_C_ULONG, &pData->m_nEventNo,		0, &cbValue); 
		retCode = ::SQLGetData(hStmt, ++i, SQL_C_ULONG, &pData->m_bAll,			0, &cbValue); 
		retCode = ::SQLGetData(hStmt, ++i, SQL_C_ULONG, &pData->m_nSex,			0, &cbValue); 
		retCode = ::SQLGetData(hStmt, ++i, SQL_C_ULONG, &pData->m_nClass,		0, &cbValue); 
		retCode = ::SQLGetData(hStmt, ++i, SQL_C_ULONG, &pData->m_nNation,		0, &cbValue); 
		retCode = ::SQLGetData(hStmt, ++i, SQL_C_ULONG, &pData->m_nItem1No,		0, &cbValue); 
		retCode = ::SQLGetData(hStmt, ++i, SQL_C_ULONG, &pData->m_nItem1Mutant, 0, &cbValue); 
		retCode = ::SQLGetData(hStmt, ++i, SQL_C_ULONG, &pData->m_nItem1Grade,	0, &cbValue); 
		retCode = ::SQLGetData(hStmt, ++i, SQL_C_ULONG, &pData->m_nItem2No,		0, &cbValue); 
		retCode = ::SQLGetData(hStmt, ++i, SQL_C_ULONG, &pData->m_nItem2Mutant, 0, &cbValue); 
		retCode = ::SQLGetData(hStmt, ++i, SQL_C_ULONG, &pData->m_nItem2Grade,	0, &cbValue); 
		// 030616 YGI
		retCode = ::SQLGetData(hStmt, ++i, SQL_C_CHAR, &pData->m_szTable,	20, &cbValue); 
		
		retCode = SQLFetch( hStmt );
		ct++;
	}
	::SQLFreeStmt(hStmt, SQL_DROP);
	
	// ���� �������� �ִ��� Ȯ�� ��ƾ
	for( i=0; i<nRowCount; i++ )
	{
		if( vtSI[i].m_nEventNo )
		{
			::sprintf( condition, "event_no = %d AND login_id = '%s'", vtSI[i].m_nEventNo, connections[ch->server_id].id );
			::GetRowLineOfSQL( "Event_StartItem_Recv_ID", "*", &count, condition );
			if( !count )
			{
				ret += ::AddEventItem( ch, &vtSI[i] );
			}	
		}
	}
	
	return ret;
}

int AddEventItem( CHARLIST *ch, k_startitem_basic* pData )
{
	char query[1024];
	if( !pData->m_nEventNo ) return 0;
	if( !pData->m_bAll )
	{
		switch( pData->m_nSex )
		{
		case 1 : if( ch->Gender != 1 ) return 0; break;		// ����
		case 2 : if( ch->Gender != 0 ) return 0; break;		// ����
		}
		switch( pData->m_nClass )
		{
		case 1: if( ch->Class != WARRIOR ) return 0; break;
		case 2: if( ch->Class != ARCHER	 ) return 0; break;
		case 3: if( ch->Class != THIEF   ) return 0; break;
		case 4: if( ch->Class != PRIEST  ) return 0; break;
		case 5: if( ch->Class != WIZARD  ) return 0; break;
		}
		switch( pData->m_nNation )
		{
		case 1 : if( ch->name_status.nation != N_VYSEUS ) return 0; break;
		case 2 : if( ch->name_status.nation != N_ZYPERN ) return 0; break;
		case 3 : if( ch->name_status.nation != N_YILSE ) return 0; break;
		}

		// 030616 YGI
		if( pData->m_szTable[0] )
		{
			int nRowCount = 0;
			char szQuerry[512];
			sprintf( szQuerry, "login_id = '%s'", connections[ch->server_id].id );
			GetRowLineOfSQL( pData->m_szTable, "*", &nRowCount, szQuerry );
			
			if( nRowCount <= 0 ) return 0;
		}
	}
	
	int add_item = 0;
	
	int ret;
	if( pData->m_nItem1No )
	{
		ret = MakeAndSetItem( ch, pData->m_nItem1No, pData->m_nItem1Mutant, pData->m_nItem1Grade );
		if( ret < 0 )
		{
			MyLog( 0, "Error Make Event_StartItem1 [%s] : [%s]", (ret==0)?"Fail rare":"no enough inventory", ch->Name );
		}
		else
		{
			// �Է�
			sprintf( query, "INSERT INTO event_startitem_recv_id ( event_no, recv_date, login_id, character_name, item_no, item_mutant, item_grade ) VALUES "
				" ( %d, NOW(), '%s', '%s', %d, %d, %d ) ", pData->m_nEventNo, connections[ch->server_id].id, ch->Name,
				pData->m_nItem1No, pData->m_nItem1Mutant, pData->m_nItem1Grade );
			Querry_SQL( query );
			add_item = 1;
		}
	}
	if( pData->m_nItem2No )
	{
		ret = MakeAndSetItem( ch, pData->m_nItem2No, pData->m_nItem2Mutant, pData->m_nItem2Grade );
		if( ret < 0 )
		{
			MyLog( 0, "Error Make Event_StartItem2 [%s] : [%s]", (ret==0)?"Fail rare":"no enough inventory", ch->Name );
		}
		else
		{
			// �Է�
			sprintf( query, "INSERT INTO event_startitem_recv_id ( event_no, recv_date, login_id, character_name, item_no, item_mutant, item_grade ) VALUES "
				" ( %d, NOW(), '%s', '%s', %d, %d, %d ) ", pData->m_nEventNo, connections[ch->server_id].id, ch->Name,
				pData->m_nItem2No, pData->m_nItem2Mutant, pData->m_nItem2Grade );
			Querry_SQL( query );
			add_item = 1;
		}
	}
	return add_item;
}

extern bool MakeRareAttr(	unsigned int &attr3 ,const int iNowGrade,
						 const int iSokSung1,const int iSokSung2,const int iSokSung3,
						 const int iHighLevel,const bool bIsDynamicRare);

extern int MakeDefaultRareItem( ItemAttr *item, const int iItemNo, const int iSok1, const int iGrade, const int iTryMulti);

int MakeAndSetItem( CHARLIST *ch, int item_no, int item_mutant, int item_grade )
{
	ItemAttr *item = SearchInv( ch );
	if( item )
	{
		return MakeDefaultRareItem( item, item_no, item_mutant, item_grade, 0 );
	}
	return -1;		// �������?����.
}

// �̺�Ʈ ���������� Ȯ���Ѵ�.  return ���� 0 �� 1�θ� ���� �Ѵ�.
int CheckEventObject( char *map_name, k_event_object_sound *data )
{
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	SQLLEN		cbValue;
	char szQuerry[512];
	
	int nRowCount = 0;
	sprintf( szQuerry, "event_start_date <= NOW() AND event_end_date >= NOW() AND map = '%s'", map_name );
	GetRowLineOfSQL( "Event_Object", "*", &nRowCount, szQuerry );
	if( !nRowCount ) return 0;		// �ش� �̺�Ʈ�� ����.
	
	sprintf( szQuerry, "SELECT object_toi, object_b FROM event_object "
		"WHERE event_start_date <= NOW() AND event_end_date >= NOW() AND map = '%s'"
		"ORDER BY event_no DESC", map_name );
	
	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	if( !SQLOK(retCode) )
	{
		SQLFreeStmt(hStmt, SQL_DROP);
		return 0;
	}
	
	// ����Ÿ �������� ��ƾ
	retCode = SQLFetch( hStmt );
	if( SQLOK( retCode ) )
	{
		retCode = SQLGetData(hStmt, 1, SQL_C_CHAR, data->object_toi, 30, &cbValue);
		EatRearWhiteChar( data->object_toi );
		retCode = SQLGetData(hStmt, 2, SQL_C_CHAR, data->object_b, 30, &cbValue);
		EatRearWhiteChar( data->object_b );
	}
	else 
	{
		SQLFreeStmt(hStmt, SQL_DROP);
		return 0;
	}
	data->active = 1;
	SQLFreeStmt(hStmt, SQL_DROP);
	return 1;
}

// �̺�Ʈ ���尡 ���������� Ȯ���Ѵ�.  return ���� 0 �� 2�θ� ���� �Ѵ�.
int CheckEventSound( char *map_name, k_event_object_sound *data )
{
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	SQLLEN		cbValue;
	char szQuerry[512];
	
	int nRowCount = 0;
	sprintf( szQuerry, "event_start_date <= NOW() AND event_end_date >= NOW() AND ( `all` <> 0 OR map = '%s' )", map_name );
	GetRowLineOfSQL( "Event_sound", "*", &nRowCount, szQuerry );
	if( !nRowCount ) return 0;		// �ش� �̺�Ʈ�� ����.
	
	sprintf( szQuerry, "SELECT sound FROM event_sound "
		"WHERE event_start_date <= NOW() AND event_end_date >= NOW() AND ( `all` <> 0 OR map = '%s' )"
		" ORDER BY event_no DESC ", map_name );
	
	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	if( !SQLOK(retCode) )
	{
		SQLFreeStmt(hStmt, SQL_DROP);
		return 0;
	}
	
	// ����Ÿ �������� ��ƾ
	int temp = 0;
	retCode = SQLFetch( hStmt );
	if( SQLOK( retCode ) )
	{
		retCode = SQLGetData(hStmt, 1, SQL_C_LONG, &temp, 0, &cbValue); data->sound_no = temp;
	}
	else
	{
		SQLFreeStmt(hStmt, SQL_DROP);
		return 0;
	}
	
	data->active += 2;
	SQLFreeStmt(hStmt, SQL_DROP);
	return 2;
}

//021107 YGI
int GetEventMoveMapName( k_event_move_map_req *pData, k_event_move_map_rep *pMapData )
{
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	SQLLEN		cbValue;
	char szQuerry[512];
	
	static int sex2sex[] = { 2, 1 };
	static int class2class[] = { 1, 3, 2, 5, 4 };
	static int nation2nation[] = { 0, 0, 0, 1, 2, 0, 3 };
	
	sprintf( szQuerry, "SELECT map, event_no, `all`, sex, class, nation FROM event_mapmove WHERE "
		"event_start_date <= NOW() AND event_end_date >= NOW() "
		"ORDER BY event_no DESC" );
	
	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	if( !SQLOK(retCode) )
	{
		SQLFreeStmt(hStmt, SQL_DROP);
		return 0;
	}
	
	int tAll, tSex, tClass, tNation;
	tAll = tSex = tClass = tNation = 0;
	// ����Ÿ �������� ��ƾ
	retCode = SQLFetch( hStmt );
	
	if( SQLOK( retCode ) )
	{
		retCode = SQLGetData(hStmt, 1, SQL_C_CHAR, pMapData->map_name, 20, &cbValue);
		EatRearWhiteChar( pMapData->map_name );
		retCode = SQLGetData(hStmt, 2, SQL_C_LONG, &pMapData->event_no, 0, &cbValue);
		
		retCode = SQLGetData(hStmt, 3, SQL_C_LONG, &tAll, 0, &cbValue);
		retCode = SQLGetData(hStmt, 4, SQL_C_LONG, &tSex, 0, &cbValue);
		retCode = SQLGetData(hStmt, 5, SQL_C_LONG, &tClass, 0, &cbValue);
		retCode = SQLGetData(hStmt, 6, SQL_C_LONG, &tNation, 0, &cbValue);
		
		SQLFreeStmt(hStmt, SQL_DROP);
		
		if( tAll ) return 1;		// ����
		if( tSex )
		{
			if( tSex != sex2sex[pData->sex] ) return 0;
		}
		if( tClass )
		{
			if( tClass != class2class[pData->class_num] ) return 0;
		}
		if( tNation )
		{
			if( tNation != nation2nation[pData->nation] ) return 0;
		}
		return 1;
	}
	
	SQLFreeStmt(hStmt, SQL_DROP);
	return 0;
}

// 030221 YGI
// �ܺο��� ���� ���ϸ� �޴� ���?�̸��� �����´�. ������ �̸��� �����?
int GetRecvMailName( int *pRowCount, char pszRecvName[40][20])
{
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	SQLLEN		cbValue;
	char szQuerry[512];
	
	*pRowCount = 0;
	GetRowLineOfSQL( "recv_mail_name", "name", pRowCount, NULL, hDBC );
	if( *pRowCount <= 0 ) {return 0;}
	if( *pRowCount > 40 ) {*pRowCount = 40;}
	
	::strcpy( szQuerry, "SELECT `name`, ct FROM recv_mail_name ORDER BY ct LIMIT 40" );
	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	if( !SQLOK(retCode) )
	{
		SQLFreeStmt(hStmt, SQL_DROP);
		return 0;
	}
	
	char tempname[20] = {0,};
	
	retCode = ::SQLFetch( hStmt );
	int iCount = 0;
	int delete_count = 0;
	while( SQLOK( retCode ) )
	{
		::SQLGetData(hStmt, 1, SQL_C_CHAR, tempname, 20, &cbValue);
		EatRearWhiteChar( tempname );
		strcpy( pszRecvName[iCount++], tempname );
		
		::SQLGetData(hStmt, 2, SQL_C_LONG, &delete_count, 0, &cbValue);
		retCode = SQLFetch( hStmt );
		
	}
	::SQLFreeStmt(hStmt, SQL_DROP);

	sprintf( szQuerry, "DELETE FROM recv_mail_name WHERE ct <= %d", delete_count );
	Querry_SQL( szQuerry );

	return 1;
}

// NPC������ ����ġ ���� ������ ���?�ִ� ���̺��� DB���� �о����?�Լ�
int LoadNPCLevTable()
{	//< CSD-030306
	if(Num_Of_NPC_Lev <= 0) 
	{
		return -4 ; // No item's available
	}
	
	HSTMT hStmt= NULL ;
	RETCODE ret ;
	char query_stmt[80]= {0, } ;
	SQLLEN cbValue ;
	int c= 1 ;
	SQLAllocStmt(hDBC, &hStmt);
	
	strcpy(query_stmt, "select * from NPC_Lv2Exp order by Lv") ;
	ret= SQLExecDirect(hStmt, (UCHAR *)query_stmt, SQL_NTS) ;

	if(ret != SQL_SUCCESS_WITH_INFO && ret != SQL_SUCCESS) 
	{
		printf("NPC_Lv2Exp: ExecDirect Error ");
		return -1 ;
	}

	//SQLNumResultCols(hStmt, &nCols) ;
	
	ret= SQLFetch(hStmt) ;
	if(ret != SQL_SUCCESS_WITH_INFO && ret != SQL_SUCCESS) 
	{
		printf("NPC_Lv2Exp:: Fetch Error" ) ;
		return -1 ;
	}
		
	while(c < Num_Of_NPC_Lev && ret == SQL_SUCCESS)
	{	//< CSD-TW-030606
		ret = SQLGetData(hStmt,  2, SQL_C_SLONG, &NPC_Lev_Ref[c].nMinExp, 0, &cbValue);
		ret = SQLGetData(hStmt,  3, SQL_C_SLONG, &NPC_Lev_Ref[c].nMaxExp, 0, &cbValue);
		ret = SQLGetData(hStmt,  4, SQL_C_SLONG, &NPC_Lev_Ref[c].nNeedExp, 0, &cbValue);
		ret = SQLGetData(hStmt,  5, SQL_C_SLONG, &NPC_Lev_Ref[c].nTrainingExp, 0, &cbValue);
		ret = SQLGetData(hStmt,  6, SQL_C_FLOAT, &NPC_Lev_Ref[c].fExpReduction, 0, &cbValue);
		ret = SQLGetData(hStmt,  7, SQL_C_FLOAT, &NPC_Lev_Ref[c].fNKReduction, 0, &cbValue);
		ret = SQLGetData(hStmt,  8, SQL_C_SLONG, &NPC_Lev_Ref[c].nTacRate, 0, &cbValue);
		ret = SQLGetData(hStmt,  9, SQL_C_SLONG, &NPC_Lev_Ref[c].nMaxTactic, 0, &cbValue);
		ret = SQLGetData(hStmt, 10, SQL_C_SLONG, &NPC_Lev_Ref[c].nDyeingPrice, 0, &cbValue);
		ret = SQLGetData(hStmt, 11, SQL_C_SLONG, &NPC_Lev_Ref[c].nLessRate, 0, &cbValue);
		ret = SQLGetData(hStmt, 12, SQL_C_SLONG, &NPC_Lev_Ref[c].nGreatRate, 0, &cbValue);
		ret = SQLGetData(hStmt, 13, SQL_C_SLONG, &NPC_Lev_Ref[c].nCspMax, 0, &cbValue);
		ret = SQLGetData(hStmt, 14, SQL_C_SLONG, &NPC_Lev_Ref[c].nBaseCP, 0, &cbValue);
		ret = SQLGetData(hStmt, 15, SQL_C_SLONG, &NPC_Lev_Ref[c].bank_loan, 0, &cbValue);
		ret = SQLGetData(hStmt, 16, SQL_C_SLONG, &NPC_Lev_Ref[c].nStep, 0, &cbValue);
		ret = SQLGetData(hStmt, 17, SQL_C_SLONG, &NPC_Lev_Ref[c].nWarriorLife, 0, &cbValue);
		ret = SQLGetData(hStmt, 18, SQL_C_SLONG, &NPC_Lev_Ref[c].nThiefLife, 0, &cbValue);
		ret = SQLGetData(hStmt, 19, SQL_C_SLONG, &NPC_Lev_Ref[c].nArcherLife, 0, &cbValue); 
		ret = SQLGetData(hStmt, 20, SQL_C_SLONG, &NPC_Lev_Ref[c].nWizardLife, 0, &cbValue); 
		ret = SQLGetData(hStmt, 21, SQL_C_SLONG, &NPC_Lev_Ref[c].nPriestLife, 0, &cbValue); 
		ret = SQLGetData(hStmt, 22, SQL_C_SLONG, &NPC_Lev_Ref[c].nWarriorExpRate, 0, &cbValue);
		ret = SQLGetData(hStmt, 23, SQL_C_SLONG, &NPC_Lev_Ref[c].nThiefExpRate, 0, &cbValue);
		ret = SQLGetData(hStmt, 24, SQL_C_SLONG, &NPC_Lev_Ref[c].nArcherExpRate, 0, &cbValue); 
		ret = SQLGetData(hStmt, 25, SQL_C_SLONG, &NPC_Lev_Ref[c].nWizardExpRate, 0, &cbValue); 
		ret = SQLGetData(hStmt, 26, SQL_C_SLONG, &NPC_Lev_Ref[c].nPriestExpRate, 0, &cbValue); 
		//> CSD-TW-030606
		if(ret != SQL_SUCCESS_WITH_INFO && ret != SQL_SUCCESS) 
		{
			printf("NPC_Lv2Exp: Error!!! (%d)", ret);
			return -1 ;
		}
		
		c++;

		ret= SQLFetch(hStmt) ;
	}

	SQLFreeStmt(hStmt, SQL_DROP);
	return c;
}
// 040105 YGI ------------
int GetTreasureAttr( int &mapnumber, int &x, int &y )
{
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	SQLLEN		cbValue;
	char		szQuerry[512];
	
	sprintf(szQuerry, "SELECT `index`, mapname, x, y FROM event_treasure_xy WHERE `use` = 0 ORDER BY RAND() LIMIT 1");
	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	if( !SQLOK(retCode) )
	{
		SQLFreeStmt(hStmt, SQL_DROP);
		return( -1 );
	}
	
	retCode = SQLFetch( hStmt );
	mapnumber = x = y = 0;
	char map_name[20];
	int index = 0;
	if( SQLOK( retCode ) )
	{
		SQLGetData(hStmt, 1, SQL_C_ULONG, &index, 0, &cbValue); 
		SQLGetData(hStmt, 2, SQL_C_CHAR, map_name, 20, &cbValue); 
		EatRearWhiteChar( map_name );
		SQLGetData(hStmt, 3, SQL_C_ULONG, &x, 0, &cbValue); 
		SQLGetData(hStmt, 4, SQL_C_ULONG, &y, 0, &cbValue);
	}

	mapnumber = GetMapNameByNumber( map_name );
	SQLFreeStmt(hStmt, SQL_DROP);
	if( mapnumber<0 ) return -1;

	// ����Ѱ�?üũ����..
	sprintf( szQuerry, "UPDATE event_treasure_xy SET `use`=1 WHERE `index` = %d", index );
	Querry_SQL( szQuerry );
	return 1;
}

int GetMapNameByNumber( char *name )
{
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	SQLLEN		cbValue;
	char		szQuerry[1024];
	
	sprintf(szQuerry, "SELECT number FROM map_info WHERE mapfile = '%s'", name);
	SQLAllocStmt(hDBC, &hStmt);
	int number = 0;
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{
		retCode = SQLFetch(hStmt);
		if(retCode == SQL_SUCCESS)
		{
			retCode = SQLGetData(hStmt, 1, SQL_C_LONG, &number, 0, &cbValue);
			SQLFreeStmt(hStmt, SQL_DROP);
			return( number );
		}
		else
		{
			SQLFreeStmt(hStmt, SQL_DROP);
			return(-2);
		}
	}
	
	SQLFreeStmt(hStmt, SQL_DROP);
	return(-1);
}

int GetConvertDayTime2Age(const int nAge) // 030929 kyo
{// convert 20030301 -> current age
 // format yyyymmdd
	if( nAge <= 0)
	{
		return 0;
	}

	int rAge=0;
	/* using mmdd
	rAge = g_year - nAge/10000 - 1;
	if( nAge%10000 <= ((g_mon+1)*100 + g_day) )
	{
		rAge++;
	}
	*/
	rAge = g_year - nAge/10000 ; // not use mmdd
	return rAge;
}


int GetUserAge(const char* szID) // 030929 kyo
{	
	if (LocalMgr.IsAbleNation(CHINA|TAIWAN|HONGKONG))
	{	//< CSD-040120
		return 0;	
	}	//> CSD-040120

	HSTMT		hStmt=NULL;
	SQLLEN		cbValue=0;
	char		szQuerry[255];
	sprintf( szQuerry, "SELECT d_age FROM chr_log_info WHERE login_id = '%s' ", szID );//020823 lsw
	
	SQLAllocStmt(hDBC_TotalDB, &hStmt);
	RETCODE		retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	
	int nAge =0;
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{
		retCode = SQLFetch(hStmt);
		retCode = SQLGetData(hStmt, 1, SQL_C_LONG, &nAge, 0, &cbValue);		
	}
	else
	{		
		MyLog( LOG_NORMAL, "Failed Get User Age chr_log_info in TotalDB (User Name -->'%s')", szID );		
	}

	SQLFreeStmt(hStmt, SQL_DROP);		

	return GetConvertDayTime2Age(nAge);
}

bool IsLimitedTime() // 030929 kyo
{
	int nStart  = ::GetPrivateProfileInt( "Thai Limited", "StartTime", 22, DB_DEMON_INI_ );
	int nEnd	= ::GetPrivateProfileInt( "Thai Limited", "EndTime", 6, DB_DEMON_INI_ );
	if( g_hour >= nStart || g_hour < nEnd ) 
	{
		return true;
	}
	return false;
}
